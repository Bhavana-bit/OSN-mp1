#include "sham.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/select.h>

FILE *log_file = NULL;

void open_log(const char *role)
{
    char fname[64];
    snprintf(fname, sizeof(fname), "%s_log.txt", role);
    log_file = fopen(fname, "w");
    if (!log_file)
    {
        perror("fopen log_file");
        exit(1);
    }
}

void log_event(const char *fmt, ...)
{
    if (!log_file)
    {
        return;
    }
    struct timeval tv;
    gettimeofday(&tv, NULL);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&tv.tv_sec));
    fprintf(log_file, "[%s.%06ld] [LOG] ", buf, (long)tv.tv_usec);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(log_file, fmt, ap);
    va_end(ap);
    fprintf(log_file, "\n");
    fflush(log_file);
}

// --- Handshake ---
static int do_handshake(int sock, struct sockaddr_in *cli, socklen_t *clen)
{
    uint8_t buf[1500];
    ssize_t n;
    n = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)cli, clen);
    struct sham_header *h = (struct sham_header *)buf;
    sham_header_to_host(h);
    log_event("RCV SYN SEQ=%u", h->seq_num);
    uint32_t client_isn = h->seq_num;
    uint32_t server_isn = rand() % 100000;
    struct sham_header reply = {.seq_num = server_isn, .ack_num = client_isn + 1, .flags = SHAM_FLAG_SYN | SHAM_FLAG_ACK, .window_size = 8192};
    struct sham_header net = reply;
    sham_header_to_network(&net);
    sendto(sock, &net, sizeof(net), 0, (struct sockaddr *)cli, *clen);
    log_event("SND SYN-ACK SEQ=%u ACK=%u", reply.seq_num, reply.ack_num);

    n = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)cli, clen);
    h = (struct sham_header *)buf;
    sham_header_to_host(h);
    if (h->flags & SHAM_FLAG_ACK)
    {
        log_event("RCV ACK FOR SYN");
    }
    return server_isn + 1;
}

// --- Chat Mode ---
static void run_chat_mode(int sock, struct sockaddr_in *cli, socklen_t clen, uint32_t seq)
{
    fd_set readfds;
    char msg[1024];
    uint8_t buf[1500];
    printf("Chat started! Type messages. Type '/quit' to exit.\n");
    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sock, &readfds);
        int maxfd = sock > STDIN_FILENO ? sock : STDIN_FILENO;
        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0)
        {
            perror("select");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            if (!fgets(msg, sizeof(msg), stdin))
            {
                break;
            }
            if (strncmp(msg, "/quit", 5) == 0)
            {
                struct sham_header fin = {.seq_num = seq, .ack_num = 0, .flags = SHAM_FLAG_FIN, .window_size = 8192};
                struct sham_header netf = fin;
                sham_header_to_network(&netf);
                sendto(sock, &netf, sizeof(netf), 0, (struct sockaddr *)cli, clen);
                log_event("SND FIN SEQ=%u", seq);
                break;
            }
            size_t len = strlen(msg);
            struct sham_header h = {.seq_num = seq, .ack_num = 0, .flags = 0, .window_size = 8192};
            struct sham_header net = h;
            sham_header_to_network(&net);
            memcpy(buf, &net, sizeof(net));
            memcpy(buf + sizeof(net), msg, len);
            sendto(sock, buf, sizeof(net) + len, 0, (struct sockaddr *)cli, clen);
            log_event("SND CHAT SEQ=%u LEN=%zu", seq, len);
            seq += len;
        }

        if (FD_ISSET(sock, &readfds))
        {
            ssize_t n = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)cli, &clen);
            if (n <= 0)
            {
                continue;
            }
            struct sham_header *h = (struct sham_header *)buf;
            sham_header_to_host(h);
            if (h->flags & SHAM_FLAG_FIN)
            {
                log_event("RCV FIN SEQ=%u", h->seq_num);
                printf("Client ended chat.\n");
                break;
            }
            size_t payload_len = n - sizeof(*h);
            if (payload_len > 0)
            {
                buf[n] = 0;
                printf("Client: %s", (char *)(buf + sizeof(*h)));
            }
        }
    }
}

// --- File Mode ---
static void receive_file(int sock, struct sockaddr_in *cli, socklen_t clen, const char *outfile)
{
    FILE *out = fopen(outfile, "wb");
    if (!out)
    {
        perror("fopen");
        exit(1);
    }
    uint8_t buf[1500];
    ssize_t n;
    while (1)
    {
        n = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)cli, &clen);
        if (n <= 0)
        {
            break;
        }
        struct sham_header *h = (struct sham_header *)buf;
        sham_header_to_host(h);
        size_t payload_len = n - sizeof(*h);
        if (h->flags & SHAM_FLAG_FIN)
        {
            log_event("RCV FIN SEQ=%u", h->seq_num);
            break;
        }
        else if (payload_len > 0)
        {
            fwrite(buf + sizeof(*h), 1, payload_len, out);
            log_event("RCV DATA SEQ=%u LEN=%zu", h->seq_num, payload_len);
        }
    }
    fclose(out);
    printf("File received successfully.\n");
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage:\n  %s <port> --chat\n  %s <port> --file <outputfile>\n", argv[0], argv[0]);
        return 1;
    }
    srand(time(NULL) ^ getpid());
    open_log("server");

    int port = atoi(argv[1]);
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr, cli;
    socklen_t clen = sizeof(cli);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(sock, (struct sockaddr *)&addr, sizeof(addr));

    uint32_t seq = do_handshake(sock, &cli, &clen);

    if (strcmp(argv[2], "--chat") == 0)
    {
        run_chat_mode(sock, &cli, clen, seq);
    }
    else if (strcmp(argv[2], "--file") == 0 && argc >= 4)
    {
        receive_file(sock, &cli, clen, argv[3]);
    }
    else
    {
        fprintf(stderr, "Invalid arguments.\n");
        return 1;
    }

    if (log_file)
    {
        fclose(log_file);
    }
    close(sock);
    return 0;
}

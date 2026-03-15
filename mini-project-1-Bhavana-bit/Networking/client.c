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
static int do_handshake(int sock, struct sockaddr_in *srv, socklen_t slen)
{
    uint32_t client_isn = rand() % 100000;
    struct sham_header syn = {.seq_num = client_isn, .ack_num = 0, .flags = SHAM_FLAG_SYN, .window_size = 8192};
    struct sham_header net = syn;
    sham_header_to_network(&net);
    sendto(sock, &net, sizeof(net), 0, (struct sockaddr *)srv, slen);
    log_event("SND SYN SEQ=%u", client_isn);

    uint8_t buf[1500];
    ssize_t n = recvfrom(sock, buf, sizeof(buf), 0, NULL, NULL);
    struct sham_header *h = (struct sham_header *)buf;
    sham_header_to_host(h);
    if (!(h->flags & SHAM_FLAG_SYN) || !(h->flags & SHAM_FLAG_ACK))
    {
        fprintf(stderr, "Expected SYN-ACK\n");
        exit(1);
    }
    log_event("RCV SYN-ACK SEQ=%u ACK=%u", h->seq_num, h->ack_num);

    struct sham_header ack = {.seq_num = client_isn + 1, .ack_num = h->seq_num + 1, .flags = SHAM_FLAG_ACK, .window_size = 8192};
    struct sham_header net2 = ack;
    sham_header_to_network(&net2);
    sendto(sock, &net2, sizeof(net2), 0, (struct sockaddr *)srv, slen);
    log_event("SND ACK FOR SYN");
    return client_isn + 1;
}

// --- Chat Mode ---
static void run_chat_mode(int sock, struct sockaddr_in *srv, socklen_t slen, uint32_t seq)
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
                sendto(sock, &netf, sizeof(netf), 0, (struct sockaddr *)srv, slen);
                log_event("SND FIN SEQ=%u", seq);
                break;
            }
            size_t len = strlen(msg);
            struct sham_header h = {.seq_num = seq, .ack_num = 0, .flags = 0, .window_size = 8192};
            struct sham_header net = h;
            sham_header_to_network(&net);
            memcpy(buf, &net, sizeof(net));
            memcpy(buf + sizeof(net), msg, len);
            sendto(sock, buf, sizeof(net) + len, 0, (struct sockaddr *)srv, slen);
            log_event("SND CHAT SEQ=%u LEN=%zu", seq, len);
            seq += len;
        }

        if (FD_ISSET(sock, &readfds))
        {
            ssize_t n = recvfrom(sock, buf, sizeof(buf), 0, NULL, NULL);
            if (n <= 0)
            {
                continue;
            }
            struct sham_header *h = (struct sham_header *)buf;
            sham_header_to_host(h);
            if (h->flags & SHAM_FLAG_FIN)
            {
                log_event("RCV FIN SEQ=%u", h->seq_num);
                printf("Server ended chat.\n");
                break;
            }
            size_t payload_len = n - sizeof(*h);
            if (payload_len > 0)
            {
                buf[n] = 0;
                printf("Server: %s", (char *)(buf + sizeof(*h)));
            }
        }
    }
}

// --- File Mode ---
static void send_file(int sock, struct sockaddr_in *srv, socklen_t slen, uint32_t seq, const char *file)
{
    FILE *in = fopen(file, "rb");
    if (!in)
    {
        perror("fopen");
        exit(1);
    }
    uint8_t buf[1500];
    size_t r;
    while ((r = fread(buf + sizeof(struct sham_header), 1, 1024, in)) > 0)
    {
        struct sham_header h = {.seq_num = seq, .ack_num = 0, .flags = 0, .window_size = 8192};
        struct sham_header net = h;
        sham_header_to_network(&net);
        memcpy(buf, &net, sizeof(net));
        sendto(sock, buf, sizeof(net) + r, 0, (struct sockaddr *)srv, slen);
        log_event("SND DATA SEQ=%u LEN=%zu", seq, r);
        seq += r;
    }
    fclose(in);
    struct sham_header fin = {.seq_num = seq, .ack_num = 0, .flags = SHAM_FLAG_FIN, .window_size = 8192};
    struct sham_header netf = fin;
    sham_header_to_network(&netf);
    sendto(sock, &netf, sizeof(netf), 0, (struct sockaddr *)srv, slen);
    log_event("SND FIN SEQ=%u", seq);
    printf("File sent successfully.\n");
}

int main(int argc, char **argv)
{
    if (argc < 4)
    {
        fprintf(stderr, "Usage:\n  %s <server_ip> <port> --chat\n  %s <server_ip> <port> --file <inputfile> <outputfile>\n", argv[0], argv[0]);
        return 1;
    }
    srand(time(NULL) ^ getpid());
    open_log("client");

    char *server_ip = argv[1];
    int port = atoi(argv[2]);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in srv;
    socklen_t slen = sizeof(srv);
    srv.sin_family = AF_INET;
    srv.sin_port = htons(port);
    inet_aton(server_ip, &srv.sin_addr);

    uint32_t seq = do_handshake(sock, &srv, slen);

    if (strcmp(argv[3], "--chat") == 0)
    {
        run_chat_mode(sock, &srv, slen, seq);
    }
    else if (strcmp(argv[3], "--file") == 0 && argc >= 6)
    {
        send_file(sock, &srv, slen, seq, argv[4]);
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

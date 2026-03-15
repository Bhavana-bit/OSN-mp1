#ifndef SHAM_H
#define SHAM_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

/* Flags */
#define SHAM_FLAG_SYN 0x1
#define SHAM_FLAG_ACK 0x2
#define SHAM_FLAG_FIN 0x4

#define SHAM_DATA_SIZE 1024

/* Force packed layout so header is exactly sizeof(...) */
#pragma pack(push,1)
struct sham_header {
    uint32_t seq_num;     /* byte sequence number (first byte in this segment) */
    uint32_t ack_num;     /* next expected sequence (cumulative ack) */
    uint16_t flags;       /* SYN, ACK, FIN */
    uint16_t window_size; /* advertised window in bytes */
};
#pragma pack(pop)

/* Convert fields to network order before sending */
static inline void sham_header_to_network(struct sham_header *h) {
    h->seq_num = htonl(h->seq_num);
    h->ack_num = htonl(h->ack_num);
    h->flags = htons(h->flags);
    h->window_size = htons(h->window_size);
}

/* Convert fields to host order after receiving */
static inline void sham_header_to_host(struct sham_header *h) {
    h->seq_num = ntohl(h->seq_num);
    h->ack_num = ntohl(h->ack_num);
    h->flags = ntohs(h->flags);
    h->window_size = ntohs(h->window_size);
}

#endif
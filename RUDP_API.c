#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>

#define MAX_DATA_SIZE 1024

typedef struct {
    uint16_t length;
    uint16_t checksum;
    uint8_t flags;
    char data[MAX_DATA_SIZE];
} RUDP_Packet;

unsigned short int calculate_checksum(void *data, unsigned int bytes) {
    unsigned short int *data_pointer = (unsigned short int *)data;
    unsigned int total_sum = 0;

    // Main summing loop
    while (bytes > 1) {
        total_sum += *data_pointer++;
        bytes -= 2;
    }

    // Add left-over byte, if any
    if (bytes > 0)
        total_sum += *((unsigned char *)data_pointer);

    // Fold 32-bit sum to 16 bits
    while (total_sum >> 16)
        total_sum = (total_sum & 0xFFFF) + (total_sum >> 16);

    return (~((unsigned short int)total_sum));
}

int rudp_socket() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        perror("Failed to create UDP socket");
        return -1;
    }
    // Additional initialization code for RUDP protocol parameters
    return sockfd;
}

int rudp_send(int sockfd, const void *buf, size_t len, int flags) {
    RUDP_Packet packet;
    memset(&packet, 0, sizeof(packet));
    packet.length = (uint16_t)len;
    memcpy(packet.data, buf, len);
    packet.checksum = calculate_checksum(packet.data, packet.length);

    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(12345);
    dest_addr.sin_addr.s_addr = inet_addr("192.168.1.100");

    if (sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("Failed to send packet");
        return -1;
    }

    return 0;
}

int rudp_close(int sockfd) {
    return close(sockfd);
}
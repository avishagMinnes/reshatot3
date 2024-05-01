#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "RUDP_API.h"

typedef struct {
    PacketType type;
    unsigned int seq_num;
    char data[1024];
} RUDPPacket;

int handle_handshake(int sockfd, struct sockaddr_in *client_addr, socklen_t *client_len) {
    RUDPPacket packet;
    if (recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)client_addr, client_len) < 0) {
        perror("Failed to receive handshake");
        return -1;
    }

    if (packet.type == HANDSHAKE_INIT) {
        packet.type = HANDSHAKE_ACK;
        if (sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)client_addr, *client_len) < 0) {
            perror("Failed to send handshake ACK");
            return -1;
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    int sockfd = rudp_socket();
    if (sockfd < 0) {
        perror("Failed to create UDP socket");
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to bind socket");
        rudp_close(sockfd);
        return 1;
    }

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    if (handle_handshake(sockfd, &client_addr, &client_len) != 0) {
        fprintf(stderr, "Handshake failed, terminating.\n");
        rudp_close(sockfd);
        return 1;
    }

    char buffer[1024];
    while (1) {
        ssize_t received = rudp_recv(sockfd, buffer, sizeof(buffer), 0, &client_addr);
        if (received < 0) {
            perror("Failed to receive data");
            continue;
        }

        buffer[received] = '\0'; // Null-terminate string
        printf("Received: %s\n", buffer);

        // Check for a termination condition
        if (strcmp(buffer, "exit") == 0) {
            printf("Exit signal received, shutting down.\n");
            break;
        }
    }

    rudp_close(sockfd);
    return 0;
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "RUDP_API.h"

int handle_handshake(int sockfd, struct sockaddr_in *client_addr, socklen_t *client_len) {
    RUDPPacket packet;
    if (recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)client_addr, client_len) < 0) {
        perror("Handshake receive failed");
        return -1;
    }
    // Check if it's a handshake init packet
    if (packet.type == HANDSHAKE_INIT) {
        packet.type = HANDSHAKE_ACK; // Prepare ACK packet
        if (sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)client_addr, *client_len) < 0) {
            perror("Handshake ACK failed");
            return -1;
        }
        return 0; // Handshake successful
    }
    return -1; // Unexpected packet type
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int sockfd, port = atoi(argv[1]);
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Create UDP socket
    int sockfd = rudp_socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Failed to create UDP socket");
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    server_addr.sin_port = htons(port); // Host to network short

     if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    // Handle handshake
    if (handle_handshake(sockfd, &client_addr, &client_len) < 0) {
        fprintf(stderr, "Handshake failed\n");
        return 1;
    }

    char buffer[1024]; // Buffer to store received data
    struct timeval start, end;
    long mtime, seconds, useconds;
    double total_bytes_received = 0;
    double total_time = 0;
    int total_received_messages = 0;

    while (1) {
        gettimeofday(&start, NULL);
        ssize_t received = rudp_recv(sockfd, buffer, sizeof(buffer), 0, port);
        gettimeofday(&end, NULL);

        if (received < 0) {
            perror("Failed to receive data");
            continue;
        }

        seconds  = end.tv_sec  - start.tv_sec;
        useconds = end.tv_usec - start.tv_usec;
        mtime = ((seconds) * 1000 + useconds / 1000.0) + 0.5;

        if (received > 0) {
            total_bytes_received += received;
            total_time += mtime;
            total_received_messages++;
            buffer[received] = '\0'; // Null-terminate string
            printf("Received: %s in %ld ms.\n", buffer, mtime);
        }

        // Check for a termination condition
        if (strcmp(buffer, "exit") == 0) {
            printf("Exit signal received, shutting down.\n");
            break;
        }
    }

    // Calculate and display the average time and bandwidth
    if (total_received_messages > 0) {
        double average_time = total_time / total_received_messages;
        double average_bandwidth = (total_bytes_received / total_time) * 1000; // Bytes per second
        printf("Average reception time: %.2f ms\n", average_time);
        printf("Average bandwidth: %.2f Bytes/s\n", average_bandwidth);
    }

    // Close the UDP socket
    rudp_close(sockfd);

    return 0;
}



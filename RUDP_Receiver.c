#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "RUDP_API.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]); // Convert port number from string to integer

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

    // if (rudp_bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    //     perror("Failed to bind socket");
    //     rudp_close(sockfd);
    //     return 1;
    // }

    char buffer[1024]; // Buffer to store received data
    struct timeval start, end;
    long mtime, seconds, useconds;
    double total_bytes_received = 0;
    double total_time = 0;
    int total_received_messages = 0;

    while (1) {
        gettimeofday(&start, NULL);
        ssize_t received = rudp_recv(sockfd, buffer, sizeof(buffer), 0);
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

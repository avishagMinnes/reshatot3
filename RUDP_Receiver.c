#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // For close()
#include <sys/socket.h>
#include <netinet/in.h> // For the AF_INET (Address Family)
#include "RUDP_API.c"

// Assuming these are part of your RUDP library
int rudp_socket(int domain, int type, int protocol);
int rudp_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
ssize_t rudp_recv(int sockfd, void *buf, size_t len, int flags);
int rudp_close(int sockfd);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]); // Convert port number from string to integer

    // Create socket
    int sockfd = rudp_socket(AF_INET, SOCK_DGRAM, 0); // SOCK_DGRAM for UDP
    if (sockfd < 0) {
        perror("Failed to create socket");
        return 1;
    }

    // Bind socket to a specific port
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    server_addr.sin_port = htons(port); // Host to network short

    if (rudp_bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to bind socket");
        rudp_close(sockfd);
        return 1;
    }

    // Receive data
    char buffer[1024]; // Buffer to store received data
    while (1) {
        ssize_t received = rudp_recv(sockfd, buffer, sizeof(buffer), 0);
        if (received < 0) {
            perror("Failed to receive data");
            break;
        }

        buffer[received] = '\0'; // Null-terminate string
        printf("Received: %s\n", buffer);

        // Check for a termination condition
        if (strcmp(buffer, "exit") == 0) {
            printf("Exit signal received, shutting down.\n");
            break;
        }
    }

    // Close the RUDP socket
    rudp_close(sockfd);

    return 0;
}

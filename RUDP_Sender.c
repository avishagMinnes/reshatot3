#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // For close()

// Include your RUDP headers here
// Assume rudp_socket(), rudp_send(), and rudp_close() are defined elsewhere

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <IP> <Port>\n", argv[0]);
        return 1;
    }

    const char* ip = argv[1];
    int port = atoi(argv[2]);

    // Initialize RUDP socket
    int sockfd = rudp_socket();
    if (sockfd < 0) {
        fprintf(stderr, "Failed to initialize RUDP socket\n");
        return 1;
    }

    // Normally here you would open and read from a file, but for simplicity:
    const char* message = "Hello, this is a test message using RUDP.";
    size_t message_len = strlen(message);

    // Use rudp_send() to send the data
    if (rudp_send(sockfd, ip, port, message, message_len, 0) < 0) {
        fprintf(stderr, "Failed to send data\n");
        rudp_close(sockfd);
        return 1;
    }

    // Close RUDP socket
    rudp_close(sockfd);

    printf("Data sent successfully.\n");
    return 0;
}
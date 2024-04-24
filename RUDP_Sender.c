#include "RUDP_API.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP> <PORT>\n", argv[0]);
        return 1;
    }

    const char* ip = argv[1];
    int port = atoi(argv[2]);
    char buffer[1024];
    FILE *file;

    // Create UDP socket
    int sockfd = rudp_socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Failed to create UDP socket");
        return 1;
    }

    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    dest_addr.sin_addr.s_addr = inet_addr(ip);

    char decision[10];  // Buffer for user decision

    do {
        printf("Enter the filename to send: ");
        scanf("%1023s", buffer);

        file = fopen(buffer, "rb");
        if (file == NULL) {
            perror("Failed to open file");
            continue;
        }

        // Read and send the file
        while (!feof(file)) {
            size_t read_bytes = fread(buffer, 1, sizeof(buffer), file);
            if (read_bytes > 0) {
                if (rudp_send(sockfd, &dest_addr, buffer, read_bytes, 0) < 0) {
                    perror("Failed to send data");
                }
            }
        }

        fclose(file);

        // Ask user whether to send the file again
        printf("Send the file again? (yes/no): ");
        scanf("%9s", decision);

    } while (strncmp(decision, "yes", 3) == 0);

    // Send an exit message to the receiver
    strcpy(buffer, "exit");
    rudp_send(sockfd, &dest_addr, buffer, strlen(buffer), 0);

    // Close the UDP socket
    rudp_close(sockfd);

    return 0;
}

#include "RUDP_API.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int initiate_handshake(int sockfd, struct sockaddr_in *dest_addr) {
    RUDP_Packet packet = { .type = HANDSHAKE_INIT, .seq_num = 0 };
    if (sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)dest_addr, sizeof(struct sockaddr_in)) < 0) {
        perror("Handshake send failed");
        return -1;
    }

    // Wait for handshake ACK
    struct sockaddr_in from;
    socklen_t fromlen = sizeof(from);
    if (recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&from, &fromlen) < 0 || packet.type != HANDSHAKE_ACK) {
        perror("Handshake ACK failed");
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    printf("0\n");
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP> <PORT>\n", argv[0]);
        return 1;
    }

    const char* ip = argv[1];
    int port = atoi(argv[2]);
    char buffer[1024];
    FILE *file;

    printf("1\n");
    int sockfd = rudp_socket();
    if (sockfd < 0) {
        perror("Failed to create UDP socket");
        return 1;
    }

    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    dest_addr.sin_addr.s_addr = inet_addr(ip);

    printf("2\n");
    if (initiate_handshake(sockfd, &dest_addr) != 0) {
        fprintf(stderr, "Handshake failed, terminating.\n");
        return 1;
    }

	char decision[5];
    do {
        printf("3\n");
        printf("Enter the filename to send: ");
        fgets(buffer, 1024, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';
        printf("file: %s\n", buffer);
        file = fopen(buffer, "rb");
        if (file == NULL) {
            perror("Failed to open file");
            continue;
        }

        printf("4\n");
        while (!feof(file)) {
            size_t read_bytes = fread(buffer, 1, sizeof(buffer), file);
            if (ferror(file)) {
                perror("Read error");
                break;
            }
            if (read_bytes > 0) {
                if (rudp_send(sockfd, buffer, read_bytes, 0, ip, port) < 0) {
                    perror("Failed to send data");
                }
            }
        }

        fclose(file);
	
        printf("5\n");
        printf("Send the file again? (yes/no): ");
        fgets(decision, sizeof(decision), stdin);
        decision[strcspn(decision, "\n")] = 0;  // Remove trailing newline

    } while (strncmp(decision, "yes", 3) == 0);

    printf("6\n");
    strcpy(buffer, "exit");
    rudp_send(sockfd, buffer, strlen(buffer), 0, ip, port);
    rudp_close(sockfd);

    return 0;
}

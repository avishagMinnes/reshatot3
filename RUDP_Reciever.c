#include "RUDP_API.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <PORT>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    char buffer[1024];
    FILE *file;
    struct timeval start_time, end_time;

    // Create UDP socket
    int sockfd = rudp_socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Failed to create UDP socket");
        return 1;
    }

    struct sockaddr_in my_addr;
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    my_addr.sin_port = htons(port);

    if (rudp_bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
        perror("Failed to bind socket");
        rudp_close(sockfd);
        return 1;
    }

    double total_time = 0;
    double total_bandwidth = 0;
    int count = 0;

    printf("Ready to receive files...\n");

    while (1) {
        gettimeofday(&start_time, NULL);
        size_t total_received = 0;

        // Open a temporary file to save received data
        file = fopen("received_file", "wb");
        if (file == NULL) {
            perror("Failed to open file for writing");
            continue;
        }

        while (1) {
            ssize_t received = rudp_recv(sockfd, buffer, sizeof(buffer), 0);
            if (received < 0) {
                perror("Failed to receive data");
                break;
            }

            if (strncmp(buffer, "exit", 4) == 0) {
                fclose(file);
                goto exit_program;
            }

            fwrite(buffer, 1, received, file);
            total_received += received;

            // Check if the sender indicates the end of file transmission
            if (received < sizeof(buffer)) {
                break;  // Typically, a smaller read indicates the last packet
            }
        }

        fclose(file);
        gettimeofday(&end_time, NULL);

        double time_taken = (end_time.tv_sec - start_time.tv_sec) * 1000.0;      // sec to ms
        time_taken += (end_time.tv_usec - start_time.tv_usec) / 1000.0;   // us to ms
        double bandwidth = total_received / time_taken * 1000 / 1024;     // KBps

        printf("Received file, Time: %.2f ms, Bandwidth: %.2f KBps\n", time_taken, bandwidth);

        total_time += time_taken;
        total_bandwidth += bandwidth;
        count++;

        // Wait for Sender response
        // This simplistic approach assumes the sender will either resend immediately or send an exit message
    }

exit_program:
    if (count > 0) {
        printf("Average Time: %.2f ms, Average Bandwidth: %.2f KBps\n", total_time / count, total_bandwidth / count);
    }

    rudp_close(sockfd);
    return 0;
}
}

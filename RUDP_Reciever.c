#include "RUDP_API.c"
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
    char buffer[1024];  // Increase if expecting larger packets

    // Create a UDP socket
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

    struct timeval start_time, end_time;
    double total_time = 0, total_bandwidth = 0;
    int count = 0;

    printf("Receiver is ready to receive files...\n");

    while (1) {
        gettimeofday(&start_time, NULL);
        ssize_t received = rudp_recv(sockfd, buffer, sizeof(buffer), 0);
        if (received < 0) {
            perror("Receive failed");
            continue;
        }

        buffer[received] = '\0';  // Null-terminate received data
        if (strcmp(buffer, "exit") == 0) {
            break;
        }

        gettimeofday(&end_time, NULL);

        double time_taken = (end_time.tv_sec - start_time.tv_sec) * 1000.0;
        time_taken += (end_time.tv_usec - start_time.tv_usec) / 1000.0;
        double bandwidth = received / time_taken * 1000 / 1024;  // KBps

        printf("Received: %s\nTime: %.2f ms, Bandwidth: %.2f KBps\n", buffer, time_taken, bandwidth);
        
        total_time += time_taken;
        total_bandwidth += bandwidth;
        count++;
    }

    if (count > 0) {
        printf("Average Time: %.2f ms, Average Bandwidth: %.2f KBps\n", total_time / count, total_bandwidth / count);
    }

    rudp_close(sockfd);
    return 0;
}

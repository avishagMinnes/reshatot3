#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

#define MAX_DATA_SIZE 1024

typedef enum {
    HANDSHAKE_INIT, HANDSHAKE_ACK, DATA, TERMINATE
} PacketType;

typedef struct {
    PacketType type;
    unsigned int seq_num;
    uint16_t length;
    uint16_t checksum;
    uint8_t flags;
    char data[MAX_DATA_SIZE];
} RUDP_Packet;


// Allocates a new structure for the RUDP socket (contains basic information about the socket itself). Also creates a UDP socket as a baseline for the RUDP.
// isServer means that this socket acts like a server. If set to server socket, it also binds the socket to a specific port.
int rudp_socket();

// Receives data from the other side and put it into the buffer. Returns the number of received bytes on success, 0 if got FIN packet (disconnect), and -1 on error.
// Fails if called when the socket is disconnected.
int rudp_recv(int sockfd, void *buf, size_t len, int flags, int port);

// Sends data stores in buffer to the other side. Returns the number of sent bytes on success, 0 if got FIN packet (disconnect), and -1 on error. Fails if called when the socket is disconnected.
int rudp_send(int sockfd, const void *buf, short len, int flags, const char *ip, int port);

// This function releases all the memory allocation and resources of the socket.
int rudp_close(int sockfd);

//this function calculates the checksum
unsigned short int calculate_checksum(void *data, unsigned int bytes);


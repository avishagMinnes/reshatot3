CC=gcc
CFLAGS=-Wall -g
OBJFILES=RUDP_Sender.o RUDP_Receiver.o RUDP_API.o
TARGET=runRUDP

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES)

RUDP_Sender.o: RUDP_Sender.c RUDP_API.c
	$(CC) $(CFLAGS) -c RUDP_Sender.c

RUDP_Receiver.o: RUDP_Receiver.c RUDP_API.c
	$(CC) $(CFLAGS) -c RUDP_Receiver.c

RUDP_API.o: RUDP_API.c RUDP_API.c
	$(CC) $(CFLAGS) -c RUDP_API.c

clean:
	rm -f $(OBJFILES) $(TARGET)

.PHONY: all clean

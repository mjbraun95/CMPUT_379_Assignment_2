CC = gcc
CFLAGS = -Wall -Wextra

	
all: gs_client client server a2p2

debug: CFLAGS += -g
debug: clean a2p2.c

gs_client: gs_client.c
	$(CC) $(CFLAGS) -o gs_client gs_client.c

client: client.c
	$(CC) $(CFLAGS) -o client client.c

server: server.c
	$(CC) $(CFLAGS) -o server server.c

a2p2: a2p2.c
	$(CC) $(CFLAGS) -o a2p2 a2p2.c

clean:
	rm -f gs_client server a2p2

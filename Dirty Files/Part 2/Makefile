CC = gcc
DB = gdb
CFLAGS = -Wall -Wextra
GDBFLAGS = -g -Wall -Wextra


	
all: gs_client client server a2p2

# debug:
# 	gdb ${GDBFLAGS} -o all

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

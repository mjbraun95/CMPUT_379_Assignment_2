#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXCLIENTS 3
#define FIFO_NAME "myfifo"

// Define packet types
enum packet_type {
    PUT,
    GET,
    DELETE,
    GTIME,
    TIME,
    OK,
    ERROR
};

// Define packet struct
struct packet {
    enum packet_type type;
    char message[100];
};

// Function to spawn a new client process
void spawn_client(int id, char *inputFile) {
    pid_t pid;
    if ((pid = fork()) < 0) {
        fprintf(stderr, "Error: fork failed\n");
        exit(1);
    } else if (pid == 0) {
        // child process
        char idStr[10];
        snprintf(idStr, 10, "%d", id);
        // execlp("./gs_client", "./gs_client", "localhost", idStr, inputFile, NULL);
        execlp("./client", "./client", "localhost", idStr, inputFile, NULL);
        fprintf(stderr, "Error: exec failed\n");
        exit(1);
    }
}

// Function to run the server
void run_server() {
    execlp("./server", "./server", NULL);
    fprintf(stderr, "Error: exec failed\n");
    exit(1);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s -s | -c idNumber inputFile\n", argv[0]);
        exit(1);
    }

    if (strcmp(argv[1], "-s") == 0) {
        // Run as server
        run_server();
    } else if (strcmp(argv[1], "-c") == 0) {
        // Run as client
        if (argc < 4) {
            fprintf(stderr, "Usage: %s -s | -c idNumber inputFile\n", argv[0]);
            exit(1);
        }
        int id = atoi(argv[2]);
        char *inputFile = argv[3];
        spawn_client(id, inputFile);
    } else {
        fprintf(stderr, "Usage: %s -s | -c idNumber inputFile\n", argv[0]);
        exit(1);
    }
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/select.h>

#define LINE_BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    // Check # of arguments isn't correct
    if (argc != 4) {
        printf("Usage: %s (number of lines) (input file) (delay(ms))\n", argv[0]);
        exit(EXIT_FAILURE);
    } else {
        int lines_per_iteration = atoi(argv[1]);
        FILE* input_fp = fopen(argv[2], "r");
        int delay_ms = atoi(argv[3]);
        // Check for null file
        if (input_fp == NULL) {
            perror("Error opening input file.");
            exit(EXIT_FAILURE);
        } else {

            // Input is accepted, starts reading file contents
            char line_buffer[LINE_BUFFER_SIZE];
            int lines_printed = 0;
            printf("a2p1 starts: (nLine=%s, inFile='%s', delay=%s)...\n", argv[1], argv[2], argv[3]);
            while (1) {
                if (fgets(line_buffer, LINE_BUFFER_SIZE, input_fp) != NULL) {
                    lines_printed++;
                }
                if (lines_printed % lines_per_iteration == 0) {
                    // nLines reached
                    printf("\n*** Entering a delay period of %d msec\n", delay_ms);

                    char* command = getpass("");
                    fflush(stdout);
                    sleep(delay_ms/1000);

                    if (fgets(command, sizeof(command), stdin) != NULL) {
                        printf("command: %s", command);
                    };
                }

            }
            
        }

    }
}
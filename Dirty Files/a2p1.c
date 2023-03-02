#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/select.h>

#define MAXBUF 1024

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s (number of lines) (input file) (delay(ms))\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int nLine = atoi(argv[1]);
    FILE *input_file = fopen(argv[2], "r");
    int delay = atoi(argv[3]);

    if (input_file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    printf("Starting the program with (nLine=%d, inputFile='%s', delay=%d)...\n",
            nLine, argv[2], delay);

    char input_buffer[MAXBUF];
    int line_count = 0;

    while (1) {
        if (fgets(input_buffer, MAXBUF, input_file) == NULL) {
            break;
        }

        line_count++;
        printf("[%04d]: %s", line_count, input_buffer);

        if (line_count % nLine == 0) {
            printf("\nEntering a delay period of %d msec...\n", delay);
            usleep(delay * 1000);

            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(STDIN_FILENO, &read_fds);

            struct timeval timeout;
            // Set to zero-length
            timeout.tv_sec = 0;
            timeout.tv_usec = 0;


            int select_res = select(STDIN_FILENO + 1, &read_fds, NULL, NULL, &timeout);
            // Check successful read
            if (select_res == -1) {
                perror("select");
                exit(EXIT_FAILURE);
            // Check that there's 1 fd ready for I/O
            } else if (select_res == 1) {
                if (fgets(input_buffer, MAXBUF, stdin) != NULL) {
                    input_buffer[strcspn(input_buffer, "\n")] = '\0';
                    printf("User command: %s\n", input_buffer);
                    if (strcmp(input_buffer, "quit") == 0) {
                        break;
                    } else {
                        char pipe_cmd[MAXBUF + 10]; // add space for " 2>&1"
                        snprintf(pipe_cmd, MAXBUF + 10, "%s 2>&1", input_buffer);

                        FILE *pipe_file = popen(pipe_cmd, "r");
                        if (pipe_file == NULL) {
                            perror("popen");
                            exit(EXIT_FAILURE);
                        } else {
                            char output_buffer[MAXBUF];
                            while (fgets(output_buffer, MAXBUF, pipe_file) != NULL) {
                                printf("%s", output_buffer);
                            }
                            pclose(pipe_file);
                        }
                    }
                } else {
                    printf("User command: \n");
                }
            }

            printf("Delay period ended.\n");
        }
    }

    fclose(input_file);
    return EXIT_SUCCESS;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <limits.h>

#define MAX_CMD_LEN 1024

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s nLine inputFile delay\n", argv[0]);
        exit(1);
    }

    int nLine = atoi(argv[1]);
    FILE *inputFile = fopen(argv[2], "r");
    int delay = atoi(argv[3]);

    if (inputFile == NULL) {
        perror("fopen");
        exit(1);
    }

    printf("a2p1 starts: (nLine= %d, inFile='%s', delay= %d)\n", nLine, argv[2], delay);

    char line[LINE_MAX];
    int lineCount = 0;
    int delayCount = 0;

    while (fgets(line, sizeof(line), inputFile) != NULL) {
        lineCount++;
        printf("[%04d]: %s", lineCount, line);

        if (lineCount % nLine == 0) {
            printf("\n*** Entering a delay period of %d msec\n\n", delay);
            usleep(delay * 1000);

            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(STDIN_FILENO, &fds);

            if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &(struct timeval){0}) == 1) {
                if (fgets(line, sizeof(line), stdin) != NULL) {
                    line[strcspn(line, "\n")] = 0; //TODO?: Remove
                    printf("User command: %s\n", line);
                    if (strcmp(line, "quit") == 0) {
                        exit(0);
                    } else {
                        char cmd[MAX_CMD_LEN];
                        snprintf(cmd, MAX_CMD_LEN, "%s 2>&1", line);
                        FILE *pipe = popen(cmd, "r");
                        if (pipe == NULL) {
                            perror("popen");
                        } else {
                            char buffer[MAX_CMD_LEN];
                            while (fgets(buffer, MAX_CMD_LEN, pipe) != NULL) {
                                printf("%s", buffer);
                            }
                            pclose(pipe);
                        }
                    }
                } else {
                    printf("User command: \n"); // print this when the user doesn't input anything
                }
            }

printf("\n*** Delay period ended ***\n");
        }
    }

    fclose(inputFile);
    return 0;
}

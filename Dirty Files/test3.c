#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/select.h>
#include <pthread.h>
#include <time.h>

using namespace std;

// Global variables
int nLine; // Number of lines to display
string inputFile; // Name of input file
int delay; // Delay period in milliseconds
int cmd_fd[2]; // Pipe for passing commands from signal handler to main thread
pthread_t cmd_thread; // Thread for handling commands during delay period
int timer_fd; // File descriptor for timer event
struct itimerspec timer_spec; // Specification for timer event

// Signal handler for SIGALRM
void sigalrm_handler(int signum) {
    // Send a message to the main thread to handle a command
    write(cmd_fd[1], "cmd", 3);
}

// Thread function for handling commands during delay period
void* cmd_thread_func(void* arg) {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    while (true) {
        int ret = select(STDIN_FILENO + 1, &fds, NULL, NULL, NULL);
        if (ret < 0) {
            perror("select");
            break;
        }
        else if (ret == 0) {
            continue;
        }
        else {
            string cmd;
            getline(cin, cmd);
            if (cmd == "quit") {
                // Send a message to the main thread to quit
                write(cmd_fd[1], "quit", 4);
                break;
            }
            else {
                // Send the command to the shell for execution
                FILE* fp = popen(cmd.c_str(), "r");
                if (fp != NULL) {
                    char buf[4096];
                    while (fgets(buf, sizeof(buf), fp) != NULL) {
                        cout << buf;
                    }
                    pclose(fp);
                }
            }
        }
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " nLine inputFile delay" << endl;
        return 1;
    }

    // Parse command-line arguments
    nLine = atoi(argv[1]);
    inputFile = argv[2];
    delay = atoi(argv[3]);

    // Open the input file
    ifstream fin(inputFile.c_str());
    if (!fin.is_open()) {
        cerr << "Failed to open input file: " << inputFile << endl;
        return 1;
    }

    // Set up the pipe for passing commands from signal handler to main thread
    if (pipe(cmd_fd) < 0) {
        perror("pipe");
        return 1;
    }

    // Create the timer event
    timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (timer_fd < 0) {
        perror("timerfd_create");
        return 1;
    }
    timer_spec.it_interval.tv_sec = delay / 1000;
    timer_spec.it_interval.tv_nsec = (delay % 1000) * 1000000;
    timer_spec.it_value.tv_sec = timer_spec.it_interval.tv_sec;
    timer_spec.it_value.tv_nsec = timer_spec.it_interval.tv_nsec;
    if (timerfd_settime(timer_fd, 0, &timer_spec, NULL) < 0) {
        perror("timerfd_settime");
        return 1;
    }

    // Set up the signal handler for SIG

/*
# ------------------------------------------------------------
# server.c - a simple concurrent server using AF_STREAM sockets
#     Compilation:
#     	Linux 		        gcc server.c -o server
#
#     Usage:		server
#     Features:		- use of setrlimit() to limit the CPU time
#			  note: time spent while sleeping, or waiting
#			        for input is not user CPU time
#			- use of I/O multiplexing to read from the
#			  managing socket, as well as data sockets 
#
# [U. of Alberta, CMPUT379, E.S. Elmallah]
# ------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>		/* required by setrlimit() */
#include <sys/resource.h>	/* required by setrlimit() */

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>
#include <fcntl.h>

#define MAXBUF		1024
#define	CPU_LIMIT	120	/* secs */
#define CLIENT_LIMIT	3
#define MYPORT		2222
#define FIFO_NAME "myfifo"
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
//
// int main(int argc, char* argv[]) {
//     int idNumber, delayTime, server_fd, client_fd;
//     char command[MAXBUF], objectName[MAXBUF], fifo_name[MAXBUF];

//     // Create the FIFOs
//     for (int i = 0; i <= CLIENT_LIMIT; i++) {
        // sprintf(fifo_name, "fifo-%d-%d", i, (i + 1) % (CLIENT_LIMIT + 1));
        // mkfifo(fifo_name, 0666);
//         sprintf(fifo_name, "fifo-%d-%d", (i + 1) % (CLIENT_LIMIT + 1), i);
//         mkfifo(fifo_name, 0666);
//     }

//     if (strcmp(argv[1], "-s") == 0) {
//         // Server mode
//         // Open the FIFOs
//         int fd[CLIENT_LIMIT + 1];
//         for (int i = 0; i <= CLIENT_LIMIT; i++) {
//             sprintf(fifo_name, "fifo-%d-%d", i, (i + 1) % (CLIENT_LIMIT + 1));
//             fd[i] = open(fifo_name, O_RDWR);
//         }

        
//     } else if (strcmp(argv[1], "-c") == 0 && argc == 4) {
//         // Client mode
//         idNumber = atoi(argv[2]);
//         char* inputFile = argv[3];

//         // Open the FIFOs
//         sprintf(fifo_name, "fifo-%d-0", idNumber);
//         client_fd = open(fifo_name, O_WRONLY);
//         sprintf(fifo_name, "fifo-0-%d", idNumber);
//         server_fd = open(fifo_name, O_RDONLY);

//         // Open the input file
//         FILE* ifs = fopen(inputFile, "r");
//         if (!ifs) {
//             printf("Error: could not open input file %s\n", inputFile);
//             return 1;
//         }

//         // Parse the input file
//         char line[MAXBUF * 3];
//         while (fgets(line, MAXBUF * 3, ifs)) {
//             // Skip empty lines and comments
//             if (line[0] == '#' || line[0] == '\n') {
//                 continue;
//             }
            
//             // Parse the line
//             sscanf(line, "%d %s %s %d", &idNumber, command, objectName, &delayTime);
//             if (strcmp(command, "put") == 0 || strcmp(command, "get") == 0 || strcmp(command, "delete") == 0) {
//                 // Send the command to the server
//                 sprintf(line, "%d %s %s", idNumber, command, objectName);
//                 write(client_fd, line, MAXBUF);
                
//                 // Wait for the server's response
//                 read(server_fd, line, MAXBUF);
//                 printf("Client %d received response: %s\n", idNumber, line);
//             } else if (strcmp(command, "gtime") == 0) {
//                 // Send the command to the server
//                 sprintf(line, "%d %s", idNumber, command);
//                 write(client_fd, line, MAXBUF);
                
//                 // Wait for the server's response
//                 read(server_fd, line, MAXBUF);
//                 printf("Client %d received response: %s\n", idNumber, line);
//             } else if (strcmp(command, "delay") == 0) {
//                 // Delay processing of subsequent lines
//                 usleep(delayTime * 1000);
//             } else if (strcmp(command, "quit") == 0) {
//                 // Terminate the client
//                 break;
//             } else {
//                 printf("Error: unrecognized command '%s'\n", command);
//                 break;
//             }
//         }
        
//         // Close the input file and FIFOs
//         fclose(ifs);
//         close(client_fd);
//         close(server_fd);
//     } else {
//         printf("Usage: %s -s | -c idNumber inputFile\n", argv[0]);
//         return 1;
//     }
    
//     return 0;
// }


int main (int argc, char **argv) {
    
    int			i, number_of_fds_to_poll, len, port, number_of_fds_with_events, timeout, managing_socket, from_length;
    int			newsock[CLIENT_LIMIT+1];
    char		buf[MAXBUF];
    struct rlimit	cpuLimit;
    struct pollfd	pfd[CLIENT_LIMIT+1];
    struct sockaddr_in	socket_address_input, from;
    FILE		*sfp[CLIENT_LIMIT+1];

    /* set a cpu limit */

    cpuLimit.rlim_cur= cpuLimit.rlim_max= CPU_LIMIT;

    if (setrlimit (RLIMIT_CPU, &cpuLimit) < 0 ) {
    	fprintf (stderr, "%s: setrlimit \n", argv[0]);
	    exit (1);
    }    	
    getrlimit (RLIMIT_CPU, &cpuLimit);
    printf ("cpuLimit: current (soft)= %lu, maximum (hard)= %lu \n",
    		cpuLimit.rlim_cur, cpuLimit.rlim_max);
    
    /* create a managing socket */

    managing_socket= socket (AF_INET/*IPV4*/, SOCK_STREAM/*Stream socket*/, 0);
    if (managing_socket < 0) {
        fprintf (stderr, "%s: socket \n", argv[0]);
        exit (1);
    } 

    /* bind the managing socket to a name */

    socket_address_input.sin_family= AF_INET;
    // socket_address_input.sin_addr.s_addr= htonl(INADDR_ANY);
    socket_address_input.sin_addr.s_addr= inet_addr("127.0.0.1");
    socket_address_input.sin_port= htons(MYPORT);

    if (bind (managing_socket, (struct sockaddr *) &socket_address_input, sizeof socket_address_input) < 0) {
        fprintf (stderr, "%s: bind \n", argv[0]);
        exit (1);
    }	
    
    // Create the FIFOs
    for (int i = 0; i <= CLIENT_LIMIT; i++) {
        sprintf(FIFO_NAME, "fifo-%d-%d", i, (i + 1) % (CLIENT_LIMIT + 1));
        mkfifo(FIFO_NAME, 0666);
        sprintf(FIFO_NAME, "fifo-%d-%d", (i + 1) % (CLIENT_LIMIT + 1), i);
        mkfifo(FIFO_NAME, 0666);
    }
    int fifo_fd;
    /* Open the FIFO for READING */
    fifo_fd = open(FIFO_NAME, O_RDONLY);
    if (fifo_fd == -1) {
        perror("Error opening FIFO for writing");
        return -1;
    }

    /* indicate how many connection requests can be queued */
    
    listen (managing_socket, 5);
    
    /* prepare for nonblocking I/O polling from the master socket  */

    timeout=  0;
    pfd[0].fd= managing_socket;
    pfd[0].events= POLLIN;
    pfd[0].revents= 0;

    number_of_fds_to_poll= 1;			/* number_of_fds_to_poll descriptors to poll */
    while (1) {
        for (int i=0; i<CLIENT_LIMIT; i++) {
            printf("pfd[%d]: %d, events: %d\n", i, pfd[i].fd, pfd[i].events);
        }
        printf("\n");
        number_of_fds_with_events= poll (pfd, number_of_fds_to_poll, timeout);

        /* check if there is data to be read from the master socket + not at client limit yet */
        if ( (number_of_fds_to_poll < CLIENT_LIMIT) && (pfd[0].revents & POLLIN)) {
            /* accept a new connection */
            from_length= sizeof (from);
            newsock[number_of_fds_to_poll]= accept (managing_socket, (struct sockaddr *) &from, &from_length); 

            /* we may also want to perform STREAM I/O */

            if ((sfp[number_of_fds_to_poll]= fdopen(newsock[number_of_fds_to_poll], "r")) < 0) {
            fprintf (stderr, "%s: fdopen \n", argv[0]);
            exit (1);
            }    	    
            
            pfd[number_of_fds_to_poll].fd= newsock[number_of_fds_to_poll];
            pfd[number_of_fds_to_poll].events= POLLIN;
            pfd[number_of_fds_to_poll].revents= 0;
            number_of_fds_to_poll++;
        }	    

        for (i= 1; i < number_of_fds_to_poll; i++) {
            if (pfd[i].revents & POLLIN) {	/* check data socket */
                if (fgets(buf, MAXBUF, sfp[i]) != NULL) 
                    printf("%s", buf);
            }	    	
        }
    }

    close(managing_socket);
    for (i= 1; i < number_of_fds_to_poll; i++) close(newsock[i]); 
    printf("\n");
    return 0;
}

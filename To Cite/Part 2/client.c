/*
# ------------------------------------------------------------
# client.c - a client for the simple concurrent server 'server.c'
#     Compilation:
#     	Linux		        gcc client.c -o client
#     Usage:		client  remotehost
#     Features:		- use of setrlimit() to limit the CPU time
#			  note: time spent while sleeping, or waiting for input
#			        is not user CPU time
#			- use of htons()
#
# [U. of Alberta, CMPUT379, E.S. Elmallah]
# ------------------------------------------------------------
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>		/* required by setrlimit() */
#include <sys/resource.h>	/* required by setrlimit() */

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>	/* defines struct hostent, struct servent,  */
			/* and prototypes for obtaining host and    */
			/* service information                      */
#include <poll.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAXBUF		80
#define	CPU_LIMIT	100
#define MYPORT		2222
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

/* Open the FIFO for writing and send the contents of the file */
int send_file(char *filename) {
    int fifo_fd;
    int file_fd;
    struct packet pkt;
    ssize_t num_read;
    char buf[100];

    /* Open the FIFO for writing */
    fifo_fd = open(FIFO_NAME, O_WRONLY);
    if (fifo_fd == -1) {
        perror("Error opening FIFO for writing");
        return -1;
    }

    /* Open the file to be sent */
    file_fd = open(filename, O_RDONLY);
    if (file_fd == -1) {
        perror("Error opening file for reading");
        return -1;
    }

    /* Send a PUT packet with the filename */
    pkt.type = PUT;
    strncpy(pkt.message, filename, sizeof(pkt.message));
    if (write(fifo_fd, &pkt, sizeof(pkt)) == -1) {
        perror("Error writing to FIFO");
        return -1;
    }

    /* Send the contents of the file */
    while ((num_read = read(file_fd, buf, sizeof(buf))) > 0) {
        pkt.type = PUT;
        memcpy(pkt.message, buf, num_read);
        if (write(fifo_fd, &pkt, sizeof(pkt)) == -1) {
            perror("Error writing to FIFO");
            return -1;
        }
    }

    /* Send an EOF packet to signal the end of the file */
    pkt.type = PUT;
    if (write(fifo_fd, &pkt, sizeof(pkt)) == -1) {
        perror("Error writing to FIFO");
        return -1;
    }

    /* Close the file and FIFO */
    close(file_fd);
    close(fifo_fd);

    return 0;
}

int main (int argc, char **argv) {
    
    int			pid, i, s;
    char		buf[MAXBUF], pidstr[MAXBUF];
    struct rlimit	cpuLimit;
    struct sockaddr_in	server;
    struct hostent	*hp;			/* host entity    */ 
    
    FILE		*sfp;
    char* filename;
    /* check for at least one argument */

    if (argc < 3) {
    	fprintf (stderr, "Usage: %s hostname filename\n", argv[0]);
    	fprintf (stderr, "       e.g., %s gpu.srv.ualberta.ca a2p2-ex1.dat\n",  argv[0]);
    	exit (1);
    }    	

    /* set a cpu limit */
    filename = argv[2];
    cpuLimit.rlim_cur= cpuLimit.rlim_max= CPU_LIMIT;

    if (setrlimit (RLIMIT_CPU, &cpuLimit) < 0 ) {
    	fprintf (stderr, "%s: setrlimit \n", argv[0]);
	exit (1);
    }    	
    getrlimit (RLIMIT_CPU, &cpuLimit);
    printf ("cpuLimit: current (soft)= %lu, maximum (hard)= %lu \n",
    		cpuLimit.rlim_cur, cpuLimit.rlim_max);
    
    /* lookup the specified host */

    hp= gethostbyname(argv[1]);
    if (hp == (struct hostent *) NULL) {
	fprintf (stderr, "%s: gethostbyname ('%s') \n", argv[0], argv[1]);
	exit (1);
    }

    /* put the host's address, and type into a socket structure;    */ 
    /* first, clear the structure, then fill in with the IP address */
    /* of the foreign host, and the port number at which the remote */
    /* server listens						    */
        
    memset ((char *) &server, 0, sizeof server);
    memcpy ((char *) &server.sin_addr, hp->h_addr_list[0], hp->h_length);
    server.sin_family= hp->h_addrtype;
    server.sin_port= htons(MYPORT);
    
    /* create a socket, and initiate a connection */

    s= socket(hp->h_addrtype, SOCK_STREAM, 0);
    if (s < 0) {
	fprintf (stderr, "%s: socket \n", argv[0]);
	exit (1);
    } 

    /* if the socket is unbound at the time of connect(); the system */ 
    /* automatically selects and binds a name to the socket          */
    
    if (connect(s, (struct sockaddr *) &server, sizeof server) < 0) {
	fprintf (stderr, "%s: connect\n", argv[0]);
	exit (1);
    }

    /* we may also want to perform STREAM I/O on the socket */

    if ((sfp= fdopen(s, "r")) < 0) {
    	fprintf (stderr, "%s: converting s to FILE* \n", argv[0]);
    	exit (1);
    }    	    

    sprintf(pidstr, "%d: ", getpid()); 		/* get client pid */

    /* read from stdin, and write into socket */
    
    while (! feof(stdin)) {

	if (fgets(buf, MAXBUF, stdin) != NULL) {
	    printf("%s: %s", argv[0], buf);
        send_file(filename);
	    write(s, pidstr, strlen(pidstr));
	    write(s, buf, strlen(buf));

	    /* slow down clients to allow viewing interleaved input */
	    /* from multiple clients at the sever side     	    */

	    for (i=0; i < 4e6; i++);
	}
    }
    
    printf("\n"); close(s); return 0;
}

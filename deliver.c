/*
** deliver.c -- a datagram client.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>

#define MAXBUFLEN 100 // Max buffer length

int main(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;

    if (argc != 3) {
        fprintf(stderr,"usage: talker hostname message\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    char buf[100];
	char *serveraddr = argv[1];
	char *serverport = argv[2];
	char message[MAXBUFLEN];
	char *command = malloc(4*sizeof(char));
	char *filename = malloc(100*sizeof(char));
	char *ready = "Yes";	// Used to check if we received "yes" from server

	struct timeval stop_timer, start_timer;

    if ((rv = getaddrinfo(serveraddr, serverport, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("deliver: socket");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        return 2;
    }

	printf("Enter ftp <filename>:\n");
	fgets(message, MAXBUFLEN, stdin);

	// separate ftp and <filename> into 2 separate variables command and filename, respectively
	int i = 0;
	int j = 0;

	while (message[i] != 32) {
		command[i] = message[i];
		i++;
	}
	i++;
	while (message[i] != '\n') {
		filename[j] = message[i];
		i++;
		j++;
	}

	// Check existence of <filename> on host
	if (access(filename, F_OK) != -1) {
		gettimeofday(&start_timer, NULL);	// start RTT timer
		// send ftp to server
		if ((numbytes = sendto(sockfd, command, strlen(command), 0,
         		p->ai_addr, p->ai_addrlen)) == -1) {
        	perror("talker: sendto");
        	exit(1);
		}
	} else {
		printf("Oh no\n");
	}

	// Wait for server to reply
	socklen_t len = sizeof p->ai_addr;
	
	if (numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, p->ai_addr, &len) == -1) {
        perror("recvfrom");
        exit(1);
    }

	gettimeofday(&stop_timer, NULL);	// stop RTT timer

	if (strcmp(buf, ready) == 0) {
		printf("A file transfer can start.\n");
	} else {
		// exit
	}
	
	printf("RTT = %luus\n", stop_timer.tv_usec - start_timer.tv_usec);

    freeaddrinfo(servinfo);
	close(sockfd);
	
    return 0;
}

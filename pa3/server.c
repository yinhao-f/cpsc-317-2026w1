/* server.c
 * Handles the creation of a server socket and data sending.
 * Author  : Jonatan Schroeder
 * Modified: Nov 6, 2021
 *
 * Modified by: Norm Hutchinson
 * Modified: Mar 5, 2022
 *
 * Notes: You will find useful examples in Beej's Guide to Network
 * Programming (http://beej.us/guide/bgnet/).
 */

#include "server.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <signal.h>
#include <pthread.h>

/* TODO: Fill in the server code. You are required to listen on all interfaces for connections. For each connection,
 * invoke the handler on a new thread. */
void run_server(const char *port, void (*handler)(void *)) {
    // TODO: Implement this function
    int sockfd, newfd;
    struct sockaddr_in addr;
    socklen_t addrlen;
    const int BACKLOG = 10;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(port));
    addr.sin_addr = (struct in_addr){INADDR_ANY};

    addrlen = sizeof(addr);

    bind(sockfd, (struct sockaddr *)&addr, addrlen);

    while (1) {
	listen(sockfd, BACKLOG);
	newfd = accept(sockfd, NULL, NULL);

	// Single-thread
	//handler(&newfd);

	// Multi-threading
	pthread_t thread;
	pthread_create(&thread, NULL, (void *) handler, &newfd);
    }
}

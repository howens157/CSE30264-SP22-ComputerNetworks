/* proj1server.c 
 * 2/1/2022
 * Hayden Owens, Lauren Korbel, Riley Griffith
 * CSE30264 - Computer Networks
 *
 * This code implements a simple server that will accept a connection from a client 
 * requesting a file transfer, and transfer that file over a network connection
 *
 * Usage:
 *      ./proj1server PORT
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <json-glib/json-glib.h>
#include <json-glib/json-gobject.h>

#define BACKLOG 10   // how many pending connections queue will hold
#define MAXDATASIZE 1024

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    char *PORT;

    if(argc != 2){
        printf("Problem with command line arguments\n");
        printf("Format is: ./server [PORT]\n");
        exit(EXIT_FAILURE);
    }

    PORT = argv[1];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }


    //Variable declarations
    FILE *fdr; 
    int numread, numBytes;
    uint32_t fileSize;
    uint16_t filenamelen;
    char bufToSend[MAXDATASIZE];

    //while(1) {  // main accept() loop
    printf("server: waiting for connections...\n");
    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
        perror("accept");
        //continue;
    }

    inet_ntop(their_addr.ss_family,
        get_in_addr((struct sockaddr *)&their_addr),
        s, sizeof s);
    printf("server: got connection from %s\n", s);

    //create a buffer to store the filename and receive it from the client
    char bufToRecv[MAXDATASIZE];
    memset(bufToRecv, 0, sizeof(bufToRecv));
    recv(new_fd, bufToRecv, MAXDATASIZE, 0);
    if(numBytes < 0) {
        perror("Error receiving bufToRecv\n");
        //continue;
    }
    printf("Received request to transfer: %s\n", bufToRecv);


    close(new_fd);
    //}

    return 0;
}

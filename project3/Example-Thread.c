
#include <pthread.h>
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

/* Global Defines */
#define BUFFER_MAX 1000

/* Global Variables */
char g_bKeepLooping = 1;
pthread_mutex_t 	g_BigLock;

struct ClientInfo 
{
	pthread_t threadClient;
	char szIdentifier[100];
	int  socketClient;
};


void * Thread_Client (void * pData)
{
	struct ClientInfo * pClient;
	struct ClientInfo   threadClient;
	
	char szBuffer[BUFFER_MAX];
	int	 numBytes;
	
	/* Typecast to what we need */
	pClient = (ClientInfo *) pData;
	
	/* Copy it over to a local instance */
	threadClient = *pClient;
	

	while(g_bKeepLooping)
	{
		if ((numBytes = recv(pClient->socketClient, szBuffer, MAXDATASIZE-1, 0)) == -1) {
			perror("recv");
			exit(1);
		}

		szBuffer[numBytes] = '\0';

		// Debug / show what we got
		printf("Received a message of %d bytes from Client %s\n", numBytes, pClient->szIdentifier);
		printf("   Message: %s\n", szBuffer);
		
		// Do something with it
						
		
		
		// This is a pretty good time to lock a mutex
		pthread_mutex_lock(&g_BigLock);
		
		// Do something dangerous here that impacts shared information
		
		// Echo back the same message
		if (send(pClient->socketClient, szBuffer, numBytes, 0) == -1)
		{
			perror("send");		
		}
				
		// This is a pretty good time to unlock a mutex
		pthread_mutex_unlock(&g_BigLock);
	}
	
	return NULL;
}



void Server_Lobby (uint16_t nLobbyPort)
{
	// Adapting this from Beej's Guide
	
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
	
	struct ClientInfo theClientInfo;
	int       nClientCount = 0;	

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, nLobbyPort, &hints, &servinfo)) != 0) {
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

    printf("server: waiting for connections...\n");

    while(g_bKeepLooping) 
	{  
        sin_size = sizeof their_addr;		
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		
        if (new_fd == -1) 
		{
            perror("accept");
            continue;
        }

		/* Simple bit of code but this can be helpful to detect successful
		   connections 
		 */
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);

		// Print out this client information 
		sprintf(theClientInfo.szIdentifier, "%s-%d", s, nClientCount);
		theClientInfo.socketClient = new_fd;
		nClientCount++;

		/* From OS: Three Easy Pieces 
		 *   https://pages.cs.wisc.edu/~remzi/OSTEP/threads-api.pdf */
		pthread_create(&(theClientInfo.threadClient), NULL, Thread_Client, &theClientInfo);
		
		// Bail out when the third client connects after sleeping a bit
		if(nClientCount == 3)
		{
			g_bKeepLooping = 0;
			sleep(15);
		}		
    }


}

int main(int argc, char *argv[]) 
{
	pthread_mutex_init(&g_BigLock, NULL);
	
	Server_Lobby(41000);

	printf("Sleeping before exiting\n");
	sleep(15);
	
	printf("And we are done\n");
	return 0;
}	


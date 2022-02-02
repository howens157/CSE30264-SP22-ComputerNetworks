#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <arpa/inet.h>

//#define PORT "3490" // the port client will be connecting to 
#define PORT "41150" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

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
    char* port = "0";
    char* ip_addr = "0";
    char *file;
    if (argc == 4) {
        char* ip_addr = argv[1];
        port = argv[2];
        file = argv[3];
        printf("%s\n", ip_addr);
        printf("%s\n", port);
        printf("%s\n", file);

    } else {
        fprintf(stderr, "usage: ./client ip_addr port file\n");
        exit(1);
    }
    int sockfd, numBytes;  
    
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
		
		struct in_addr addr;
		if (!inet_aton(ip_addr, &addr)) {
			fprintf(stderr, "invalid ip addr\n");
			exit(1);
		}

    if ((rv = getaddrinfo(ip_addr, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }


    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

  
    //New Stuff 
    uint16_t filenamelen = (uint16_t)strlen(file);
    filenamelen = htons(filenamelen);
    filenamelen = ntohs(filenamelen);
    printf("%d\n", filenamelen);
    numBytes = send(sockfd, &filenamelen, sizeof(filenamelen), 0);
    if(numBytes < 0) {
        perror("Error sending length of filename\n");
    }
    numBytes = send(sockfd, file, strlen(file), 0);
    if(numBytes < 0) {
        perror("Error sending filename\n");
    }


    uint32_t fileSize;
    numBytes = recv(sockfd, &fileSize, sizeof(fileSize), 0);
    if(numBytes < 0) {
        perror("Error sending file size\n");
    }
    fileSize = ntohl(fileSize);
    printf("Got file size: %d\n", fileSize);

    FILE *fdw = fopen("target.test", "w");

    struct timeval tvalBefore, tvalAfter;

    gettimeofday(&tvalBefore, NULL);

    char bufToRead[MAXDATASIZE];
    int numReadTotal = 0;
    int numRead;
    while(numReadTotal < fileSize) {
        numRead = recv(sockfd, bufToRead, sizeof(bufToRead), 0);
        if(numRead == 0) {
            perror("Connection Closed by Server\n");
            break;
        }
        else if(numRead < 0) {
            perror("recv error\n");
            continue;
        }
        numReadTotal += numRead;
        printf("%d read, total: %d\n", numRead, numReadTotal);
        fwrite(bufToRead, numRead, 1, fdw);
    }
    gettimeofday(&tvalAfter, NULL);
    printf("%d %d\n", (int)tvalAfter.tv_sec - (int)tvalBefore.tv_sec, (int)tvalAfter.tv_usec - (int)tvalBefore.tv_usec);
    double txTime = (tvalAfter.tv_sec - tvalBefore.tv_sec) + (tvalAfter.tv_usec - tvalBefore.tv_usec)/1000000.0;
    double mbRead = numReadTotal/1048576.0;
    double speed = mbRead/txTime;
    printf("Transferred %lf Megabytes in %lf seconds for %lf Mb/s\n", mbRead, txTime, speed);
    fclose(fdw);
    close(sockfd);

    return 0;
}

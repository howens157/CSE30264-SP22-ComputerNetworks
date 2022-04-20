/* mycalserver.c 
 * 3/25/2022
 * Hayden Owens, Lauren Korbel, Riley Griffith
 * CSE30264 - Computer Networks
 *
 * This code implements a calendar server that manages calendars stored as lists of JSON
 * data for each event
 *
 * Usage:
 *      ./mycalserver
 *      ./mycalserver -mt
 *          -use mt flag to allow multiple connections at once
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
#include <stdbool.h>
#include <json-glib/json-glib.h>
#include <json-glib/json-gobject.h>

/* Global Defines */
#define BUFFER_MAX 1000
#define BACKLOG 10   // how many pending connections queue will hold
#define MAXDATASIZE 1024

/* Global Variables */
char g_bKeepLooping = 1;
bool lobbyFull;
pthread_mutex_t g_BigLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int numPlayers;
char* lobPort;
char* gamePort;
int numRounds;
char *dictFile;
bool debug;

char answer[100];
int answer_length;

int sockfd;
int new_fd;

struct ClientInfo 
{
	pthread_t threadClient;
	char szIdentifier[100];
	int  socketClient;
};

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

void sigint_handler(int s)
{
    printf("\nShutting down safely\n");
    close(new_fd);
    close(sockfd);
    exit(0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void * Thread_Client (void * pData)
{
	struct ClientInfo * pClient;
	struct ClientInfo   threadClient;
	
	char szBuffer[BUFFER_MAX];
	int	 numBytes;
	
	/* Typecast to what we need */
	pClient = (struct ClientInfo *) pData;
	
	/* Copy it over to a local instance */
	threadClient = *pClient;

    if ((numBytes = recv(pClient->socketClient, szBuffer, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    szBuffer[numBytes] = '\0';

    // Debug / show what we got
    printf("Received a message of %d bytes from Client %s\n", numBytes, pClient->szIdentifier);
    printf("   Message: %s\n", szBuffer);
    
    // Do something with it
    g_autoptr(JsonParser) cmdParser = json_parser_new();
    g_autoptr(GError) error = NULL;
    json_parser_load_from_data(cmdParser, szBuffer, numBytes, &error);
    if(error != NULL)
    {
        g_critical("Unable to parse command: %s", error->message);
        exit(1);
    }
    g_autoptr(JsonNode) cmdRoot = json_parser_get_root(cmdParser);

    //Read calenderName and action values
    g_autoptr(JsonReader) cmdReader = json_reader_new(cmdRoot);
    json_reader_read_member(cmdReader, "MessageType");
    const char* msgType = json_reader_get_string_value(cmdReader);
    json_reader_end_member(cmdReader);
    json_reader_read_member(cmdReader, "Data");
    json_reader_read_member(cmdReader, "Name");
    const char* name = json_reader_get_string_value(cmdReader);
    json_reader_end_member(cmdReader);
    json_reader_read_member(cmdReader, "Client");
    const char* client = json_reader_get_string_value(cmdReader);
    json_reader_end_member(cmdReader);
    json_reader_end_member(cmdReader);				
    
    printf("%s %s %s\n", msgType, name, client);

    g_autoptr(JsonBuilder) builder = json_builder_new();
    json_builder_begin_object(builder);
    json_builder_set_member_name(builder, "MessageType");
    json_builder_add_string_value(builder, "JoinResult");
    json_builder_set_member_name(builder, "Data");
    json_builder_begin_object(builder);
    json_builder_set_member_name(builder, "Name");
    json_builder_add_string_value(builder, name);
    json_builder_set_member_name(builder, "Result");
    json_builder_add_string_value(builder, "Yes");
    json_builder_end_object(builder);
    json_builder_end_object(builder);

    g_autoptr(JsonNode) root = json_builder_get_root(builder);
    g_autoptr(JsonGenerator) g = json_generator_new();
    json_generator_set_root(g, root);
    const char *joinResp = json_generator_to_data(g, NULL);

    printf("%s\n", joinResp);

    uint16_t len = strlen(joinResp);
    len = htons(len);
    send(pClient->socketClient, joinResp, len, 0);
    // This is a pretty good time to lock a mutex
    pthread_mutex_lock(&g_BigLock);
    while(!lobbyFull){
        pthread_cond_wait(&cond, &g_BigLock);
    }
    // Do something dangerous here that impacts shared information
    
            
    // This is a pretty good time to unlock a mutex
    pthread_mutex_unlock(&g_BigLock);
	
    builder = json_builder_new();
    json_builder_begin_object(builder);
    json_builder_set_member_name(builder, "MessageType");
    json_builder_add_string_value(builder, "StartInstance");
    json_builder_set_member_name(builder, "Data");
    json_builder_begin_object(builder);
    json_builder_set_member_name(builder, "Server");
    json_builder_add_string_value(builder, "localhost");
    json_builder_set_member_name(builder, "Port");
    json_builder_add_string_value(builder, gamePort);
    json_builder_set_member_name(builder, "Nonce");
    json_builder_add_string_value(builder, "TEMP");
    json_builder_end_object(builder);
    json_builder_end_object(builder);

    root = json_builder_get_root(builder);
    g = json_generator_new();
    json_generator_set_root(g, root);
    const char *startMsg = json_generator_to_data(g, NULL);

    printf("%s\n", startMsg);

    len = strlen(startMsg);
    len = htons(len);
    send(pClient->socketClient, startMsg, len, 0);

	return NULL;
}

void Server_Lobby (char* nLobbyPort, int numClients)
{
    lobbyFull = false;
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
	int nClientCount = 0;	

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, nLobbyPort, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return;
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
		
        printf("%d of %d clients\n", nClientCount, numClients);
		// Bail out when the third client connects after sleeping a bit
		if(nClientCount == numClients)
		{
            pthread_mutex_lock(&g_BigLock);
            lobbyFull = true;
			g_bKeepLooping = 0;
			sleep(15);
            pthread_cond_broadcast(&cond);
            pthread_mutex_unlock(&g_BigLock);
		}		
    }
}

void choose_answer() {
	FILE* fp = fopen(dictFile, "r");
	if (!fp) {
		fprintf(stderr, "error: could not open dictionary\n");
	}
	int count = 0;
	for (char c = getc(fp); c != EOF; c = getc(fp)) {
		if (c == '\n') {
			count += 1;
		}
	}
	printf("count: %d\n", count);
	// get random int from 1 to count
	int rand_num = rand() % count;
	rand_num += 1;
	printf("rand num: %d\n", rand_num);

	// get rand word from dictionary
	fclose(fp);
	fp = fopen(dictFile, "r");
	int i = 0;
	char line[256];
	while(fgets(line, sizeof(line), fp)) {
		i++;
		if (i == rand_num) {
			strcpy(answer, line);
			break;
		}
	}
	printf("answer: %s\n", answer);
}

char* print_guess_result(char* guess) {
	printf("answer: %s\n", answer);
	//printf("Your guess: ");
	char result [100];
	for (int i = 0; i < strlen(guess); i++) {
		int letter_printed = 0;
		if (!strncmp(&guess[i], &answer[i], 1)) {
			// correct letter, correct spot, print in green
			result[i] = 'g';
		} else {
			for (int j = 0; j < strlen(answer); j++) {
				if (!strncmp(&guess[i], &answer[j], 1)) {
					// correct letter, incorrect spot, print in yellow
					result[i] = 'y';
					letter_printed = 1;
					break;
				}
			}
			if (!letter_printed) {
				result[i] = 'b';
			}
		}
	}
	printf("result: %s\n", result);
	return (char*) result;
}

int main(int argc, char *argv[])
{
		// seed random number
		time_t t;
		srand((unsigned) time(&t));
		
		int sockfd, new_fd;  // listen on sockfd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    signal(SIGINT, sigint_handler);

    numPlayers = 1; //change to 2
    lobPort = "41100";
    gamePort = "41101";
    numRounds = 3;
    dictFile = "defaultDict.txt";
    debug = false;

    if(argc > 12) {
        printf("Improper usage\n");
        exit(1);
    }

    //Process 
    int i;
    for(i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "-np") == 0)
        {
            numPlayers = atoi(argv[i+1]);
            i++;
        }
        else if(strcmp(argv[i], "-lp") == 0)
        {
            lobPort = argv[i+1];
            i++;
        }
        else if(strcmp(argv[i], "-pp") == 0)
        {
            gamePort = argv[i+1];
            i++;
        }
        else if(strcmp(argv[i], "-nr") == 0)
        {
            numRounds = atoi(argv[i+1]);
            i++;
        }
        else if(strcmp(argv[i], "-d") == 0)
        {   
            dictFile = argv[i+1];
            i++;
        }
        else if(strcmp(argv[i], "-dbg") == 0)
        {
            debug = true;
        }
    }

    printf("Number of players: %d\n", numPlayers);
    printf("Lobby Port: %s\n", lobPort);
    printf("Game Port: %s\n", gamePort);
    printf("Number of rounds: %d\n", numRounds);
    printf("Dictionary: %s\n", dictFile);
    printf("Should debug: %s\n", (debug) ? "True" : "False");
	
	Server_Lobby(lobPort, numPlayers);

	printf("Sleeping before exiting\n");
	sleep(15);
	
	printf("And we are done\n");
	return 0;

    return 0;
}

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
char gameKeepLooping = 1;
bool lobbyFull;
bool gameFull;
pthread_mutex_t g_BigLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int numPlayers;
char* lobPort;
char* gamePort;
int numRounds;
char *dictFile;
bool debug;
struct GameClientInfo **PlayersArr;


struct ClientInfo 
{
	pthread_t threadClient;
	char szIdentifier[100];
	int  socketClient;
};

struct GameClientInfo 
{
	pthread_t threadClient;
	char szIdentifier[100];
	int  socketClient;
    char *nonce;
    char *name;
    int playerNum;
    int score;
};

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

// void sigint_handler(int s)
// {
//     printf("\nShutting down safely\n");
//     close(new_fd);
//     close(sockfd);
//     exit(0);
// }

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void * Game_Client (void * pData)
{
	struct GameClientInfo * pClient;
	struct GameClientInfo   threadClient;
	
	char szBuffer[BUFFER_MAX];
	int	 numBytes;
	
	/* Typecast to what we need */
	pClient = (struct GameClientInfo *) pData;
	
	/* Copy it over to a local instance */
	threadClient = *pClient;

    //wait for game to be ready (what about chat here?)
    pthread_mutex_lock(&g_BigLock);
    while(!gameFull){
        pthread_cond_wait(&cond, &g_BigLock);
    }
    pthread_mutex_unlock(&g_BigLock);

    //send StartGame message
    g_autoptr(JsonBuilder) builder = json_builder_new();
    json_builder_begin_object(builder);
    json_builder_set_member_name(builder, "MessageType");
    json_builder_add_string_value(builder, "StartGame");
    json_builder_set_member_name(builder, "Data");
    json_builder_begin_object(builder);
    json_builder_set_member_name(builder, "Rounds");
    json_builder_add_int_value(builder, numRounds);
    json_builder_set_member_name(builder, "PlayerInfo");
    json_builder_begin_array(builder);
    int i;
    for(i = 0; i < numPlayers; i++)
    {
        json_builder_begin_object(builder);
        json_builder_set_member_name(builder, "Name");
        json_builder_add_string_value(builder, PlayersArr[i]->name);
        json_builder_set_member_name(builder, "Number");
        json_builder_add_int_value(builder, PlayersArr[i]->playerNum);
        json_builder_end_object(builder);
    }
    json_builder_end_object(builder);
    json_builder_end_object(builder);

    g_autoptr(JsonNode) root = json_builder_get_root(builder);
    g_autoptr(JsonGenerator) g = json_generator_new();
    json_generator_set_root(g, root);
    const char *startGame = json_generator_to_data(g, NULL);

    printf("%s\n", startGame);

    uint16_t len = strlen(startGame);
    send(pClient->socketClient, startGame, len, 0);

    //start game loop
    int currRound;
    char *startRound;
    for(currRound = 1; currRound <= numRounds; currRound++)
    {
        //send StartRound message
        builder = json_builder_new();
        json_builder_begin_object(builder);
        json_builder_set_member_name(builder, "MessageType");
        json_builder_add_string_value(builder, "StartRound");
        json_builder_set_member_name(builder, "Data");
        json_builder_begin_object(builder);
        json_builder_set_member_name(builder, "Round");
        json_builder_add_int_value(builder, currRound);
        json_builder_set_member_name(builder, "RoundsRemaining");
        json_builder_add_int_value(builder, numRounds-currRound-1);
        json_builder_set_member_name(builder, "PlayerInfo");
        json_builder_begin_array(builder);
        int i;
        for(i = 0; i < numPlayers; i++)
        {
            json_builder_begin_object(builder);
            json_builder_set_member_name(builder, "Name");
            json_builder_add_string_value(builder, PlayersArr[i]->name);
            json_builder_set_member_name(builder, "Number");
            json_builder_add_int_value(builder, PlayersArr[i]->playerNum);
            json_builder_set_member_name(builder, "Score");
            json_builder_add_int_value(builder, PlayersArr[i]->score);
            json_builder_end_object(builder);
        }
        json_builder_end_object(builder);
        json_builder_end_object(builder);

        g_autoptr(JsonNode) root = json_builder_get_root(builder);
        g_autoptr(JsonGenerator) g = json_generator_new();
        json_generator_set_root(g, root);
        char *startRound = json_generator_to_data(g, NULL);

        printf("%s\n", startRound);

        uint16_t len = strlen(startRound);
        send(pClient->socketClient, startRound, len, 0);

        //initialize current round variables (new word, word length, numGuesses)
        int numGuesses = 0;

        //loop until someone gets it right
        while(1)
        {
            //Send PromptForGuess Message

            //receive Guess message

            //set receipt time in PlayersArr

            //send GuessResponse message

            //check client's guess
            break;
            
        }
        
    }
  
    return;
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
    send(pClient->socketClient, joinResp, len, 0);
    sleep(1);
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
    send(pClient->socketClient, startMsg, len, 0);

	return NULL;
}

void Game_Lobby ()
{
    gameFull = false;
	// Adapting this from Beej's Guide
	
    int sockfd, new_fd;  // listen on sockfd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    PlayersArr = (struct GameClientInfo **)(malloc(numPlayers*sizeof(struct GameClientInfo *)));
	
	int nClientCount = 0;	

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, gamePort, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("game server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("game server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "game server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    printf("game server: waiting for connections...\n");

    while(gameKeepLooping) 
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

        PlayersArr[nClientCount] = (struct GameClientInfo *)(malloc(sizeof(struct GameClientInfo)));

		// Print out this client information 
		sprintf(PlayersArr[nClientCount]->szIdentifier, "%s-%d", s, nClientCount);
		PlayersArr[nClientCount]->socketClient = new_fd;
        PlayersArr[nClientCount]->playerNum = nClientCount + 1;

        // receive joinInstance message
        char szBuffer[BUFFER_MAX];
        int	 numBytes;

        if ((numBytes = recv(PlayersArr[nClientCount]->socketClient, szBuffer, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            exit(1);
        }

        szBuffer[numBytes] = '\0';

        // Debug / show what we got
        printf("Received a message of %d bytes from Client %s\n", numBytes, PlayersArr[nClientCount]->szIdentifier);
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

        g_autoptr(JsonReader) cmdReader = json_reader_new(cmdRoot);
        json_reader_read_member(cmdReader, "MessageType");
        const char* msgType = json_reader_get_string_value(cmdReader);
        json_reader_end_member(cmdReader);
        json_reader_read_member(cmdReader, "Data");
        json_reader_read_member(cmdReader, "Name");
        char* name = (char*)json_reader_get_string_value(cmdReader);
        json_reader_end_member(cmdReader);
        json_reader_read_member(cmdReader, "Nonce");
        char* nonce = (char*)json_reader_get_string_value(cmdReader);
        json_reader_end_member(cmdReader);
        json_reader_end_member(cmdReader);				
        
        //TODO check that nonce is one of the approved nonces 
        PlayersArr[nClientCount]->nonce = nonce;
        PlayersArr[nClientCount]->name = name;

        printf("%s %s %s\n", msgType, name, nonce);

        g_autoptr(JsonBuilder) builder = json_builder_new();
        json_builder_begin_object(builder);
        json_builder_set_member_name(builder, "MessageType");
        json_builder_add_string_value(builder, "JoinInstanceResult");
        json_builder_set_member_name(builder, "Data");
        json_builder_begin_object(builder);
        json_builder_set_member_name(builder, "Name");
        json_builder_add_string_value(builder, name);
        json_builder_set_member_name(builder, "Number");
        json_builder_add_int_value(builder, nClientCount+1);
        json_builder_set_member_name(builder, "Result");
        json_builder_add_string_value(builder, "Yes");
        json_builder_end_object(builder);
        json_builder_end_object(builder);

        g_autoptr(JsonNode) root = json_builder_get_root(builder);
        g_autoptr(JsonGenerator) g = json_generator_new();
        json_generator_set_root(g, root);
        const char *joinInstRes = json_generator_to_data(g, NULL);

        printf("%s\n", joinInstRes);

        uint16_t len = strlen(joinInstRes);
        send(PlayersArr[nClientCount]->socketClient, joinInstRes, len, 0);

		/* From OS: Three Easy Pieces 
		 *   https://pages.cs.wisc.edu/~remzi/OSTEP/threads-api.pdf */
		pthread_create(&(PlayersArr[nClientCount]->threadClient), NULL, Game_Client, &PlayersArr[nClientCount]);
		
        nClientCount++;
        printf("%d of %d clients\n", nClientCount, numPlayers);
		// Bail out when the third client connects after sleeping a bit
		if(nClientCount == numPlayers)
		{
            pthread_mutex_lock(&g_BigLock);
            gameFull = true;
			gameKeepLooping = 0;
			sleep(5);
            pthread_cond_broadcast(&cond);
            pthread_mutex_unlock(&g_BigLock);
		}		
    }

    printf("Game lobby closing\n");
    close(sockfd);
    return;
}

void Server_Lobby ()
{
    lobbyFull = false;
	// Adapting this from Beej's Guide
	
    int sockfd, new_fd;  // listen on sockfd, new connection on new_fd
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

    if ((rv = getaddrinfo(NULL, lobPort, &hints, &servinfo)) != 0) {
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
		
        printf("%d of %d clients\n", nClientCount, numPlayers);
		// Bail out when the third client connects after sleeping a bit
		if(nClientCount == numPlayers)
		{
            pthread_mutex_lock(&g_BigLock);
            lobbyFull = true;
			g_bKeepLooping = 0;
			sleep(15);
            pthread_cond_broadcast(&cond);
            pthread_mutex_unlock(&g_BigLock);
		}		
    }
    int cpid = fork();
    if(cpid == 0)
    {
        //start a game
        Game_Lobby();
    }

    wait(NULL); //only one child process, so just wait for any child process

    printf("Gather Lobby closing\n");
    close(sockfd);
    //in the lobby, just return
    return;
}

int main(int argc, char *argv[])
{
    // signal(SIGINT, sigint_handler);

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
	
	Server_Lobby();

	printf("Sleeping before exiting\n");
	sleep(15);
	
	printf("And we are done\n");
	return 0;

    return 0;
}

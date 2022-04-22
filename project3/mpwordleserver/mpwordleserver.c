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
#include "SocketHelper.h"

#define MAXDATASIZE 1024
#define BUFFER_MAX 1000
#define DEFAULT_LISTEN_BACKLOG      10

int numPlayers; //change to 2
char *lobPort;
char *gamePort;
int numRounds;
char *dictFile;
bool debug;

int createSocket_TCP_Listen (char * pszServer, char * pszPort)
{
    return createSocket_TCP_Listen_real (pszServer, pszPort, DEFAULT_LISTEN_BACKLOG);
}

int createSocket_TCP_Listen_real (char * pszServer, char * pszPort, int nBacklog)
{
    // This code is adapted from Beej's Network Programming guide which has a fantastic
    //  primer on writing network code.  This is the server variant of the code which
    //  is drawn from here:
    //
    //  https://beej.us/guide/bgnet/examples/server.c
    //

    int     serverSocket;       // This is what we will create and return
    struct addrinfo hints, *servinfo, *p;
    int yes=1;
    
    int rv;

	
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    // Retrieve the address information pointer to browse what choices might be
    //  available
    if ((rv = getaddrinfo(pszServer, pszPort, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 0;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((serverSocket = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            // Since we did get it created, we should close up shop
            close(serverSocket);
            perror("setsockopt");
            continue;
        }

        if (bind(serverSocket, p->ai_addr, p->ai_addrlen) == -1) {
            // Since we did get it created, we should close up shop
            close(serverSocket);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        return 0;
    }

    if (listen(serverSocket, nBacklog) == -1) {
        perror("listen");
        // Since we did get it created, we should close up shop
        close(serverSocket);
        return 0;
    }

    return serverSocket;
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void Game_Instance()
{
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];

    int nClientCount = 0;
    printf("game server: waiting for connections...\n");
    int serverFD =  createSocket_TCP_Listen(NULL, gamePort);
    int clientFD = accept(serverFD, (struct sockaddr *)&their_addr, &sin_size);

    inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
    printf("server: got connection from %s\n", s);

    // receive joinInstance message
    char szBuffer[BUFFER_MAX];
    int	 numBytes;

    if ((numBytes = recv(clientFD, szBuffer, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    szBuffer[numBytes] = '\0';
    char szIdentifier[100];

    nClientCount++;

    sprintf(szIdentifier, "%s-%d", s, nClientCount);

    // Debug / show what we got
    printf("Received a message of %d bytes from Client %s\n", numBytes, szIdentifier);
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
    char* clientNonce = nonce;
    char* clientName = name;

    printf("%s %s %s\n", msgType, clientName, clientNonce);

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
    send(clientFD, joinInstRes, len, 0);

    sleep(5);
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
	
	Game_Instance();
	
	printf("And we are done\n");

    return 0;
}
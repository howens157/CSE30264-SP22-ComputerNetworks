//   mpwordleserver.c 
//   4/27/2022
//   Hayden Owens, Lauren Korbel, Riley Griffith
//   CSE30264 - Computer Networks

//   This code implements a multiplayer version of the game wordle, supporting 
//   multiple players using threads, multiple games using multiprocessing, and
//   a custom number of rounds and players per game.

//   Usage:
//        ./mpwordleserver/mpwordleserver -np X -lp X -pp X -nr X -d DFile -dbg

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
#include <sys/time.h>

//constants
#define MAXDATASIZE 1024
#define BUFFER_MAX 1000
#define DEFAULT_LISTEN_BACKLOG      10

//globabl vars for game logic
int numPlayers; //change to 2
char *lobPort;
int gamePort;
int numRounds;
int numGuesses = 3;
char *dictFile;
bool debug;
char answer [256];
int playersWaiting = 0;
bool answerChosen = false;
int playersGuessed = 0;
bool someoneWon = false;
bool winnerChosen = false;
char *theWinner;

//array of player info structs so every client thread can access the result and scores of other players
struct gametArgs **players;

struct gametArgs
{
    pthread_t threadClient;
    int socket;
    int myNum;
    char name[256];
    char result[256];
    double score;
    bool iWon;
    double lastReceipt;
};

//array of info for waiting players
struct servertArgs
{
    pthread_t threadClient;
    int socket;
    int number;
    int nextPort;
};

int queueWaiting = 0;
struct servertArgs **queue;

//array of nonce name pairs for checking validity of clients who try to join the game
struct secStruct {
    char *name;
    int nonce;
};

struct secStruct **nonces;

//initialize global lock and condition variable
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

//choose a random word from the dictionary file and set the global answer to it
void choose_answer() {
	printf("using dictionary %s\n", dictFile);
	FILE* fp = fopen(dictFile, "r");
	if (!fp) {
		fprintf(stderr, "error: could not open dictionary\n");
		return;
	}
	int count = 0;
	for (char c = getc(fp); c != EOF; c = getc(fp)) {
		if (c == '\n') {
			count += 1;
		}
	}
	// get random int from 1 to count
	int rand_num = rand() % count;
	rand_num += 1;

	// get rand word from dictionary
	fclose(fp);
	fp = fopen(dictFile, "r");
	if (fp < 0) {
		fprintf(stderr, "unable to open\n");
		return;
	}
	int i = 0;
	char line[256];
	while(fgets(line, sizeof(line), fp)) {
		i++;
		if (i == rand_num) {
			strcpy(answer, line);
			break;
		}
	}
    for(i = 0; i < 256; i++)
    {
        if(answer[i] >= 'a' && answer[i] <= 'z')
        {
            answer[i] -= 32;
        }
        if(answer[i] == '\n')
        {
            answer[i] = '\0';
            break;
        }
    }
}

//takes a guess and a blank result char* and sets result to the BYG string representing the
//result of the guess
void check_guess_result(char* guess, char *result) {
	for (int i = 0; i < strlen(guess); i++) {
		int letter_printed = 0;
		if (!strncmp(&guess[i], &answer[i], 1)) {
			// correct letter, correct spot, print in green
			result[i] = 'G';
		} else {
			for (int j = 0; j < strlen(answer); j++) {
				if (!strncmp(&guess[i], &answer[j], 1)) {
					// correct letter, incorrect spot, print in yellow
					result[i] = 'Y';
					letter_printed = 1;
					break;
				}
			}
			if (!letter_printed) {
				result[i] = 'B';
			}
		}
	}
	return;
}

//taken from provided socket helper file
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

//take from provided socket helper file
int createSocket_TCP_Listen (char * pszServer, char * pszPort)
{
    return createSocket_TCP_Listen_real (pszServer, pszPort, DEFAULT_LISTEN_BACKLOG);
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

//send a chat msg to every player in the players array
void processChat(char *msg, int msglen)
{
    int i;
    for(i = 0; i < numPlayers; i++)
    {
        send(players[i]->socket, msg, msglen, 0);
    }
}

//send a chat msg to every player in the queue array
void processChatLobby(char *msg, int msglen)
{
    int i;
    for(i = 0; i < numPlayers; i++)
    {
        if(queue[i] != NULL)
            send(queue[i]->socket, msg, msglen, 0);
    }
}

//function for client threads to run during the game
void * Game_Instance(void *args)
{
    //deconstruct the provided args
    struct gametArgs * myArgs;
    struct gametArgs threadArgs;

    myArgs = (struct gametArgs *) args;
    threadArgs = *myArgs;

    int clientFD = threadArgs.socket;
    int myNum = threadArgs.myNum;

    // receive joinInstance message
    char szBuffer[BUFFER_MAX];
    int	 numBytes;

    if ((numBytes = recv(clientFD, szBuffer, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    szBuffer[numBytes] = '\0';

    if(debug)
    {
        printf("Received a message of %d bytes from Client\n", numBytes);
        printf("   Message: %s\n", szBuffer);
    }
    
    // Parse the received command
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
    char* clientName = (char*)json_reader_get_string_value(cmdReader);
    json_reader_end_member(cmdReader);
    json_reader_read_member(cmdReader, "Nonce");
    int nonce = (int)json_reader_get_int_value(cmdReader);
    json_reader_end_member(cmdReader);
    json_reader_end_member(cmdReader);				
    
    //check that nonce is one of the approved nonces 
    int clientNonce = nonce;
    bool playerApproved = false;
    int i;
    for(i = 0; i < numPlayers; i++)
    {
        if(nonces[i]->nonce == clientNonce && strcmp(clientName, nonces[i]->name) == 0)
        {
            playerApproved = true;
        }
    }

    //if the player was not approved send them a JoinInstanceResult msg with a Result of No and return
    if(!playerApproved)
    {
        g_autoptr(JsonBuilder) builder = json_builder_new();
        json_builder_begin_object(builder);
        json_builder_set_member_name(builder, "MessageType");
        json_builder_add_string_value(builder, "JoinInstanceResult");
        json_builder_set_member_name(builder, "Data");
        json_builder_begin_object(builder);
        json_builder_set_member_name(builder, "Name");
        json_builder_add_string_value(builder, clientName);
        json_builder_set_member_name(builder, "Number");
        json_builder_add_int_value(builder, myNum);
        json_builder_set_member_name(builder, "Result");
        json_builder_add_string_value(builder, "No");
        json_builder_end_object(builder);
        json_builder_end_object(builder);

        g_autoptr(JsonNode) root = json_builder_get_root(builder);
        g_autoptr(JsonGenerator) g = json_generator_new();
        json_generator_set_root(g, root);
        const char *joinInstRes = json_generator_to_data(g, NULL);

        if(debug)
        {
            printf("Sending message to Client\n");
            printf("   Message: %s\n", joinInstRes);
        }

        uint16_t len = strlen(joinInstRes);
        send(clientFD, joinInstRes, len, 0);
        sleep(1);
        return NULL; 
    }

    //initialize the name, score, and iWon fields in this clients spot in the players aray
    strcpy((players[myNum-1]->name), clientName); 
    players[myNum-1]->score = 0.0;
    players[myNum-1]->iWon = false;

    //If the player was approved, send them a JoinInstanceResult
    g_autoptr(JsonBuilder) builder = json_builder_new();
    json_builder_begin_object(builder);
    json_builder_set_member_name(builder, "MessageType");
    json_builder_add_string_value(builder, "JoinInstanceResult");
    json_builder_set_member_name(builder, "Data");
    json_builder_begin_object(builder);
    json_builder_set_member_name(builder, "Name");
    json_builder_add_string_value(builder, clientName);
    json_builder_set_member_name(builder, "Number");
    json_builder_add_int_value(builder, myNum);
    json_builder_set_member_name(builder, "Result");
    json_builder_add_string_value(builder, "Yes");
    json_builder_end_object(builder);
    json_builder_end_object(builder);

    g_autoptr(JsonNode) root = json_builder_get_root(builder);
    g_autoptr(JsonGenerator) g = json_generator_new();
    json_generator_set_root(g, root);
    const char *joinInstRes = json_generator_to_data(g, NULL);

    if(debug)
    {
        printf("Sending message to Client\n");
        printf("   Message: %s\n", joinInstRes);
    }

    uint16_t len = strlen(joinInstRes);
    send(clientFD, joinInstRes, len, 0);
    sleep(1);

    int currRound;
    // Synchronization point if we have more than one player
    // All players need to join

    pthread_mutex_lock(&lock);
    playersWaiting++;
    pthread_cond_broadcast(&cond);
    while(playersWaiting < numPlayers)
    {
        pthread_cond_wait(&cond, &lock);
    }
    pthread_mutex_unlock(&lock);


    //send StartGame message
    builder = json_builder_new();
    json_builder_begin_object(builder);
    json_builder_set_member_name(builder, "MessageType");
    json_builder_add_string_value(builder, "StartGame");
    json_builder_set_member_name(builder, "Data");
    json_builder_begin_object(builder);
    json_builder_set_member_name(builder, "Rounds");
    json_builder_add_int_value(builder, numRounds);
    json_builder_set_member_name(builder, "PlayerInfo");
    json_builder_begin_array(builder);

    for(i = 0; i < numPlayers; i++)
    {
        json_builder_begin_object(builder);
        json_builder_set_member_name(builder, "Name");
        json_builder_add_string_value(builder, players[i]->name);
        json_builder_set_member_name(builder, "Number");
        json_builder_add_int_value(builder, players[i]->myNum);
        json_builder_end_object(builder);
    }
    json_builder_end_array(builder);
    json_builder_end_object(builder);
    json_builder_end_object(builder);

    root = json_builder_get_root(builder);
    g = json_generator_new();
    json_generator_set_root(g, root);
    const char *startGame = json_generator_to_data(g, NULL);

    if(debug)
    {
        printf("Sending message to Client\n");
        printf("   Message: %s\n", startGame);
    }

    len = strlen(startGame);
    send(clientFD, startGame, len, 0);

    sleep(1);

    //loop for numRounds times
    for(currRound = 1; currRound <= numRounds; currRound++)
    {
        //initialize game variables
        players[myNum-1]->iWon = false;
        memset(players[myNum-1]->result, 0, 256);
        //player 1 is tasked with doing gameWide actions such as choosing an answer
        if(myNum == 1)
        {
            choose_answer();
            pthread_mutex_lock(&lock);
            someoneWon = false;
            answerChosen = true;
            pthread_cond_broadcast(&cond);
            pthread_mutex_unlock(&lock);
        }

        //wait until an answer is chosen
        pthread_mutex_lock(&lock);
        while(!answerChosen)
        {
            pthread_cond_wait(&cond, &lock);
        }
        pthread_mutex_unlock(&lock);

        int answerLen = strlen(answer);
        if(debug)
        {
            printf("This round's word is %s, len: %d\n", answer, answerLen);
        }

        //send StartRound message
        builder = json_builder_new();
        json_builder_begin_object(builder);
        json_builder_set_member_name(builder, "MessageType");
        json_builder_add_string_value(builder, "StartRound");
        json_builder_set_member_name(builder, "Data");
        json_builder_begin_object(builder);
        json_builder_set_member_name(builder, "WordLength");
        json_builder_add_int_value(builder, answerLen);
        json_builder_set_member_name(builder, "Round");
        json_builder_add_int_value(builder, currRound);
        json_builder_set_member_name(builder, "RoundsRemaining");
        json_builder_add_int_value(builder, numRounds-currRound+1);
        json_builder_set_member_name(builder, "PlayerInfo");
        json_builder_begin_array(builder);
        for(i = 0; i < numPlayers; i++)
        {
            json_builder_begin_object(builder);
            json_builder_set_member_name(builder, "Name");
            json_builder_add_string_value(builder, players[i]->name);
            json_builder_set_member_name(builder, "Number");
            json_builder_add_int_value(builder, players[i]->myNum);
            json_builder_set_member_name(builder, "Score");
            json_builder_add_double_value(builder, players[i]->score);
            json_builder_end_object(builder);
        }
        json_builder_end_array(builder);
        json_builder_end_object(builder);
        json_builder_end_object(builder);

        root = json_builder_get_root(builder);
        g = json_generator_new();
        json_generator_set_root(g, root);
        char *startRound = json_generator_to_data(g, NULL);

        if(debug)
        {
            printf("Sending message to Client\n");
            printf("   Message: %s\n", startRound);
        }

        len = strlen(startRound);
        send(clientFD, startRound, len, 0);
        sleep(1);

        //loop for numGuesses guesses
        int currGuess;
        for(currGuess = 1; currGuess <= numGuesses; currGuess++)
        {
            //Send PromptForGuess Message
            builder = json_builder_new();
            json_builder_begin_object(builder);
            json_builder_set_member_name(builder, "MessageType");
            json_builder_add_string_value(builder, "PromptForGuess");
            json_builder_set_member_name(builder, "Data");
            json_builder_begin_object(builder);
            json_builder_set_member_name(builder, "WordLength");
            json_builder_add_int_value(builder, answerLen);
            json_builder_set_member_name(builder, "Name");
            json_builder_add_string_value(builder, clientName);
            json_builder_set_member_name(builder, "GuessNumber");
            json_builder_add_int_value(builder, currGuess);
            json_builder_end_object(builder);
            json_builder_end_object(builder);

            root = json_builder_get_root(builder);
            g = json_generator_new();
            json_generator_set_root(g, root);
            char *promptGuess = json_generator_to_data(g, NULL);

            if(debug)
            {
                printf("Sending message to Client\n");
                printf("   Message: %s\n", joinInstRes);
            }

            len = strlen(promptGuess);
            send(clientFD, promptGuess, len, 0);

            memset(szBuffer, 0, BUFFER_MAX);
            int	 numBytes;

            //loop until a non-chat message is received
            while(1)
            {
                if ((numBytes = recv(clientFD, szBuffer, MAXDATASIZE-1, 0)) == -1) {
                    perror("recv");
                    exit(1);
                }

                szBuffer[numBytes] = '\0';

                // Debug
                if(debug)
                {
                    printf("Received a message of %d bytes from Client\n", numBytes);
                    printf("   Message: %s\n", szBuffer);
                }

                // Parse the received message
                cmdParser = json_parser_new();
                error = NULL;
                json_parser_load_from_data(cmdParser, szBuffer, numBytes, &error);
                if(error != NULL)
                {
                    g_critical("Unable to parse command: %s", error->message);
                    exit(1);
                }
                cmdRoot = json_parser_get_root(cmdParser);
                cmdReader = json_reader_new(cmdRoot);
                json_reader_read_member(cmdReader, "MessageType");
                msgType = json_reader_get_string_value(cmdReader);
                json_reader_end_member(cmdReader);

                //if it was a chat message, just forward it along
                if(strcmp(msgType, "Chat") == 0)
                {
                    processChat(szBuffer, numBytes);
                }
                //otherwise break to parse it as a Guess
                else
                    break;
            }

            //Parse the received message as a Guess message
            cmdParser = json_parser_new();
            error = NULL;
            json_parser_load_from_data(cmdParser, szBuffer, numBytes, &error);
            if(error != NULL)
            {
                g_critical("Unable to parse command: %s", error->message);
                exit(1);
            }
            cmdRoot = json_parser_get_root(cmdParser);
            cmdReader = json_reader_new(cmdRoot);
            json_reader_read_member(cmdReader, "MessageType");
            msgType = json_reader_get_string_value(cmdReader);
            json_reader_end_member(cmdReader);
            json_reader_read_member(cmdReader, "Data");
            json_reader_read_member(cmdReader, "Guess");
            char *guess = (char*)json_reader_get_string_value(cmdReader);
            json_reader_end_member(cmdReader);
            json_reader_end_member(cmdReader);			

            //set receipt time in PlayersArr
            struct timeval receipt;
            gettimeofday(&receipt, NULL);
            players[myNum-1]->lastReceipt = (receipt.tv_sec) + (receipt.tv_usec)/1000000.0;


            //send GuessResponse message
            builder = json_builder_new();
            json_builder_begin_object(builder);
            json_builder_set_member_name(builder, "MessageType");
            json_builder_add_string_value(builder, "GuessReponse");
            json_builder_set_member_name(builder, "Data");
            json_builder_begin_object(builder);
            json_builder_set_member_name(builder, "Name");
            json_builder_add_string_value(builder, clientName);
            json_builder_set_member_name(builder, "Guess");
            json_builder_add_string_value(builder, guess);
            json_builder_set_member_name(builder, "Accepted");
            json_builder_add_string_value(builder, "Yes");
            json_builder_end_object(builder);
            json_builder_end_object(builder);

            root = json_builder_get_root(builder);
            g = json_generator_new();
            json_generator_set_root(g, root);
            char *guessResponse = json_generator_to_data(g, NULL);

            if(debug)
            {
                printf("Sending message to Client\n");
                printf("   Message: %s\n", guessResponse);
            }

            len = strlen(guessResponse);
            send(clientFD, guessResponse, len, 0);

            sleep(1);
            
            //check client's guess and set the client's spot in the players array
            if (!strncmp(answer, guess, answerLen)) {
                pthread_mutex_lock(&lock);
                someoneWon = true;
                pthread_mutex_unlock(&lock);
                players[myNum-1]->iWon = true;
                players[myNum-1]->score += 20.0*answerLen/((double)(currGuess));
            }
            check_guess_result(guess, (players[myNum-1]->result));

            //loop until a non-chat message is received, client will send message 
            //with the name mpwordle when the user has indicated that they would like to move
            //on from chatting
            while(1)
            {
                if ((numBytes = recv(clientFD, szBuffer, MAXDATASIZE-1, 0)) == -1) {
                    perror("recv");
                    exit(1);
                }

                szBuffer[numBytes] = '\0';

                // Debug / show what we got
                if(debug)
                {
                    printf("Received a message of %d bytes from Client\n", numBytes);
                    printf("   Message: %s\n", szBuffer);
                }
                // Parse the message
                cmdParser = json_parser_new();
                error = NULL;
                json_parser_load_from_data(cmdParser, szBuffer, numBytes, &error);
                if(error != NULL)
                {
                    g_critical("Unable to parse command: %s", error->message);
                    exit(1);
                }
                cmdRoot = json_parser_get_root(cmdParser);
                cmdReader = json_reader_new(cmdRoot);
                json_reader_read_member(cmdReader, "MessageType");
                msgType = json_reader_get_string_value(cmdReader);
                json_reader_end_member(cmdReader);
                json_reader_read_member(cmdReader, "Data");
                json_reader_read_member(cmdReader, "Name");
                char *name = (char*)json_reader_get_string_value(cmdReader);
                json_reader_end_member(cmdReader);
                json_reader_end_member(cmdReader);

                //if the message is from mpwordle, break to continue
                if(strcmp(name, "mpwordle") == 0)
                {
                    break;
                }
                //otherwise, forward it along to the other players
                if(strcmp(msgType, "Chat") == 0)
                {
                    processChat(szBuffer, numBytes);
                }
            }

            pthread_mutex_lock(&lock);
            playersGuessed++;
            pthread_cond_broadcast(&cond);
            pthread_mutex_unlock(&lock);

            // Synchronization point if we have more than one player
                // All players need to have sent a guess
            pthread_mutex_lock(&lock);
            while(playersGuessed < numPlayers)
            {
                pthread_cond_wait(&cond, &lock);
            }
            pthread_mutex_unlock(&lock);
            

            //send GuessResult
            builder = json_builder_new();
            json_builder_begin_object(builder);
            json_builder_set_member_name(builder, "MessageType");
            json_builder_add_string_value(builder, "GuessResult");
            json_builder_set_member_name(builder, "Data");
            json_builder_begin_object(builder);
            json_builder_set_member_name(builder, "Winner");
            if (someoneWon) {
                json_builder_add_string_value(builder, "Yes");
            } else {
                json_builder_add_string_value(builder, "No");
            }
            json_builder_set_member_name(builder, "PlayerInfo");
            json_builder_begin_array(builder);
            for(i = 0; i < numPlayers; i++)
            {
                json_builder_begin_object(builder);
                json_builder_set_member_name(builder, "Name");
                json_builder_add_string_value(builder, players[i]->name);
                json_builder_set_member_name(builder, "Number");
                json_builder_add_int_value(builder, players[i]->myNum);
                json_builder_set_member_name(builder, "Correct");
                if (players[i]->iWon) {
                    json_builder_add_string_value(builder, "Yes");
                } else {
                    json_builder_add_string_value(builder, "No");
                }
                json_builder_set_member_name(builder, "ReceiptTime");
                json_builder_add_double_value(builder, players[i]->lastReceipt);
                json_builder_set_member_name(builder, "Result");
                json_builder_add_string_value(builder, players[i]->result);
                json_builder_end_object(builder);
            }
            json_builder_end_array(builder);
            json_builder_end_object(builder);
            json_builder_end_object(builder);

            root = json_builder_get_root(builder);
            g = json_generator_new();
            json_generator_set_root(g, root);
            char *GuessResult = json_generator_to_data(g, NULL);

            if(debug)
            {
                printf("Sending message to Client\n");
                printf("   Message: %s\n", GuessResult);
            }

            len = strlen(GuessResult);
            send(clientFD, GuessResult, len, 0);
            sleep(1); 

            //reset playersGuessed for new guess. Player 1's thread is tasked with handling game wide tasks
            if(myNum == 1)
            {
                pthread_mutex_lock(&lock);
                playersGuessed = 0;
                pthread_cond_broadcast(&cond);
                pthread_mutex_unlock(&lock);
            }

            pthread_mutex_lock(&lock);
            while(playersGuessed != 0)
            {
                pthread_cond_wait(&cond, &lock);
            }
            pthread_mutex_unlock(&lock);

            // Decision point – was the guess successful or are there more rounds of guessing allowed?
            if(someoneWon)
            {
                if(players[myNum-1]->iWon == false)
                {
                    int numLettersRight = 0;
                    for(i = 0; i < answerLen; i++)
                    {
                        if(players[myNum-1]->result[i] == 'G')
                            numLettersRight++;
                    }
                    players[myNum-1]->score += numLettersRight*10.0/((double)(currGuess));
                }
                break;
            }
        }
        //reset answerChosen at the end of a round
        if(myNum == 1)
        {
            pthread_mutex_lock(&lock);
            answerChosen = false;
            pthread_cond_broadcast(&cond);
            pthread_mutex_unlock(&lock);
        }

        pthread_mutex_lock(&lock);
        while(answerChosen)
        {
            pthread_cond_wait(&cond, &lock);
        }
        pthread_mutex_unlock(&lock);

        //send EndRound
        builder = json_builder_new();
        json_builder_begin_object(builder);
        json_builder_set_member_name(builder, "MessageType");
        json_builder_add_string_value(builder, "EndRound");
        json_builder_set_member_name(builder, "Data");
        json_builder_begin_object(builder);
        json_builder_set_member_name(builder, "RoundsRemaining");
        json_builder_add_int_value(builder, numRounds-currRound+1);
        json_builder_set_member_name(builder, "PlayerInfo");
        json_builder_begin_array(builder);
        for(i = 0; i < numPlayers; i++)
        {
            json_builder_begin_object(builder);
            json_builder_set_member_name(builder, "Name");
            json_builder_add_string_value(builder, players[i]->name);
            json_builder_set_member_name(builder, "Number");
            json_builder_add_int_value(builder, players[i]->myNum);
            json_builder_set_member_name(builder, "ScoreEarned");
            json_builder_add_int_value(builder, players[i]->score);
            json_builder_set_member_name(builder, "Winner");
            if(players[i]->iWon)
                json_builder_add_string_value(builder, "Yes");
            else
                json_builder_add_string_value(builder, "No");
            json_builder_end_object(builder);
        }
        json_builder_end_array(builder);
        json_builder_end_object(builder);
        json_builder_end_object(builder);

        root = json_builder_get_root(builder);
        g = json_generator_new();
        json_generator_set_root(g, root);
        char *EndRound = json_generator_to_data(g, NULL);

        if(debug)
        {
            printf("Sending message to Client\n");
            printf("   Message: %s\n", EndRound);
        }

        len = strlen(EndRound);
        send(clientFD, EndRound, len, 0);
        sleep(1); 
    }

    //have player 1's thread choose the winner and let the others wait
    if(myNum == 1)
    {
        pthread_mutex_lock(&lock);
        int winnerIndex = 0;
        int winnerScore = players[0]->score;
        for(i = 1; i < numPlayers; i++)
        {
            if(players[i]->score > winnerScore)
            {
                winnerScore = players[i]->score;
                winnerIndex = i;
            }
        }
        theWinner = (players[winnerIndex]->name);
        winnerChosen = true;
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&lock);
    }

    pthread_mutex_lock(&lock);
    while(!winnerChosen)
    {
        pthread_cond_wait(&cond, &lock);
    }
    pthread_mutex_unlock(&lock);

    //send EndGame message
    builder = json_builder_new();
    json_builder_begin_object(builder);
    json_builder_set_member_name(builder, "MessageType");
    json_builder_add_string_value(builder, "EndGame");
    json_builder_set_member_name(builder, "Data");
    json_builder_begin_object(builder);
    json_builder_set_member_name(builder, "WinnerName");
    json_builder_add_string_value(builder, theWinner);
    json_builder_set_member_name(builder, "PlayerInfo");
    json_builder_begin_array(builder);
    for(i = 0; i < numPlayers; i++)
    {
        json_builder_begin_object(builder);
        json_builder_set_member_name(builder, "Name");
        json_builder_add_string_value(builder, players[i]->name);
        json_builder_set_member_name(builder, "Number");
        json_builder_add_int_value(builder, players[i]->myNum);
        json_builder_set_member_name(builder, "ScoreEarned");
        json_builder_add_int_value(builder, players[i]->score);
        json_builder_end_object(builder);
    }
    json_builder_end_array(builder);
    json_builder_end_object(builder);
    json_builder_end_object(builder);

    root = json_builder_get_root(builder);
    g = json_generator_new();
    json_generator_set_root(g, root);
    char *EndGame = json_generator_to_data(g, NULL);

    if(debug)
    {
        printf("Sending message to Client\n");
        printf("   Message: %s\n", EndGame);
    }

    len = strlen(EndGame);
    send(clientFD, EndGame, len, 0);

    sleep(5);

    return NULL;
}

//Gather players for the newly created Game
void Game_Lobby(int portNum)
{
    //initialize the players array
    players = (struct gametArgs**)(malloc(numPlayers*sizeof(struct gametArgs*)));
    char myPort[100];
    sprintf(myPort, "%d", portNum);
    int serverFD =  createSocket_TCP_Listen(NULL, myPort);
    int nClientCount = 0;

    //accept numPlayers connections
    int i;
    for(i = 0; i < numPlayers; i++)
    {
        struct sockaddr_storage their_addr;
        socklen_t sin_size;
        char s[INET6_ADDRSTRLEN];

        
        printf("game server: waiting for connections on port %s...\n", myPort);
        
        sin_size = sizeof their_addr;
        int clientFD = accept(serverFD, (struct sockaddr *)&their_addr, &sin_size);
        if (clientFD == -1) 
        {
            perror("accept");
            return;
        }

        inet_ntop(their_addr.ss_family,
                get_in_addr((struct sockaddr *)&their_addr),
                s, sizeof s);
        printf("server: got connection from %s\n", s);
        //initialize players array variables
        nClientCount++;
        players[nClientCount-1] = (struct gametArgs*)(malloc(sizeof(struct gametArgs)));
        players[nClientCount-1]->socket = clientFD;
        players[nClientCount-1]->myNum = nClientCount;

        //spawn a new Game_Instance thread
        pthread_create(&(players[nClientCount-1]->threadClient), NULL, Game_Instance, (players[nClientCount-1]));
    }
    
    //join the threads for all players
    for(i = 0; i < numPlayers; i++)
    {
        pthread_join(players[i]->threadClient, NULL);
    }

}

//thread function for a client waiting in the lobby
void * Server_Instance(void * args)
{
    //deconstruct args
    struct servertArgs * myArgs;
    struct servertArgs threadArgs;

    myArgs = (struct servertArgs *) args;
    threadArgs = *myArgs;

    int clientFD = threadArgs.socket;
    int myNum = threadArgs.number;
    char nextPort[100];
    sprintf(nextPort, "%d", threadArgs.nextPort);

    // receive join message
    char szBuffer[BUFFER_MAX];
    int	 numBytes;

    if ((numBytes = recv(clientFD, szBuffer, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    szBuffer[numBytes] = '\0';

    if(debug)
    {
        printf("Received a message of %d bytes from Client\n", numBytes);
        printf("   Message: %s\n", szBuffer);
    }
    // Parse the join message
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
    json_reader_read_member(cmdReader, "Data");
    json_reader_read_member(cmdReader, "Name");
    char* name = (char*)json_reader_get_string_value(cmdReader);
    json_reader_end_member(cmdReader);
    json_reader_end_member(cmdReader);	

    //send joinresult
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
    const char *joinRes = json_generator_to_data(g, NULL);

    if(debug)
    {
        printf("Sending message to Client\n");
        printf("   Message: %s\n", joinRes);
    }

    uint16_t len = strlen(joinRes);
    send(clientFD, joinRes, len, 0);
    sleep(1);

    //loop until a non-chat message is received, client will send message 
    //with the name mpwordle when the user has indicated that they would like to move
    //on from chatting
    while(1)
    {
        if ((numBytes = recv(clientFD, szBuffer, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            exit(1);
        }

        szBuffer[numBytes] = '\0';

        // Debug 
        if(debug)
        {
            printf("Received a message of %d bytes from Client\n", numBytes);
            printf("   Message: %s\n", szBuffer);
        }
        // Parse the message
        cmdParser = json_parser_new();
        error = NULL;
        json_parser_load_from_data(cmdParser, szBuffer, numBytes, &error);
        if(error != NULL)
        {
            g_critical("Unable to parse command: %s", error->message);
            exit(1);
        }
        cmdRoot = json_parser_get_root(cmdParser);
        cmdReader = json_reader_new(cmdRoot);
        json_reader_read_member(cmdReader, "MessageType");
        char *msgType = (char*)json_reader_get_string_value(cmdReader);
        json_reader_end_member(cmdReader);
        json_reader_read_member(cmdReader, "Data");
        json_reader_read_member(cmdReader, "Name");
        char *name = (char*)json_reader_get_string_value(cmdReader);
        json_reader_end_member(cmdReader);
        json_reader_end_member(cmdReader);

        //if the message is from mpwordle, continue to the next stage
        if(strcmp(name, "mpwordle") == 0)
        {
            break;
        }
        //otherwise, forward it along to the other players waiting in the lobby
        if(strcmp(msgType, "Chat") == 0)
        {
            processChatLobby(szBuffer, numBytes);
        }
    }

    //wait for the lobby to be full
    pthread_mutex_lock(&lock);
    queueWaiting++;
    pthread_cond_broadcast(&cond);
    while(queueWaiting < numPlayers)
    {
        pthread_cond_wait(&cond, &lock);
    }
    pthread_mutex_unlock(&lock);

    //create nonces and store the nonce,name pair in the nonces array
    int nonce = rand() % 1000000;
    nonces[myNum-1] = (struct secStruct*)(malloc(sizeof(struct secStruct)));
    nonces[myNum-1]->name = name;
    nonces[myNum-1]->nonce = nonce;

    //send startInstance
    builder = json_builder_new();
    json_builder_begin_object(builder);
    json_builder_set_member_name(builder, "MessageType");
    json_builder_add_string_value(builder, "StartInstance");
    json_builder_set_member_name(builder, "Data");
    json_builder_begin_object(builder);
    json_builder_set_member_name(builder, "Server");
    json_builder_add_string_value(builder, "127.0.0.1");
    json_builder_set_member_name(builder, "Port");
    json_builder_add_string_value(builder, nextPort);
    json_builder_set_member_name(builder, "Nonce");
    json_builder_add_int_value(builder, nonce);
    json_builder_end_object(builder);
    json_builder_end_object(builder);

    root = json_builder_get_root(builder);
    g = json_generator_new();
    json_generator_set_root(g, root);
    const char *startInstance = json_generator_to_data(g, NULL);

    if(debug)
    {
        printf("Sending message to Client\n");
        printf("   Message: %s\n", startInstance);
    }

    len = strlen(startInstance);
    send(clientFD, startInstance, len, 0);
    sleep(1);
    return NULL;
}

//Gather players in the Lobby until there are enough for a game, then fork to create the new game,
//and continue gathering for the next game
void Server_Lobby()
{
    //create a socket to listen on
    int serverFD =  createSocket_TCP_Listen(NULL, lobPort);
    while(1)
    {
        //reset queue and nonces
        queue = (struct servertArgs**)(malloc(numPlayers*sizeof(struct servertArgs*)));
        int nClientCount = 0;
        nonces = (struct secStruct**)(malloc(numPlayers*sizeof(struct secStruct*)));
        int i;
        //initialize the queue to NULL
        for(i = 0; i < numPlayers; i++)
        {
            queue[i] = NULL;
        }
        //accept numPlayers connections
        for(i = 0; i < numPlayers; i++)
        {
            struct sockaddr_storage their_addr;
            socklen_t sin_size;
            char s[INET6_ADDRSTRLEN];
            
            printf("Server lobby: waiting for connections on port %s...\n", lobPort);
            
            sin_size = sizeof their_addr;
            int clientFD = accept(serverFD, (struct sockaddr *)&their_addr, &sin_size);
            if (clientFD == -1) 
            {
                perror("accept");
                return;
            }

            inet_ntop(their_addr.ss_family,
                    get_in_addr((struct sockaddr *)&their_addr),
                    s, sizeof s);
            printf("server: got connection from %s\n", s);
            //set variables in the global queue array
            nClientCount++;
            queue[nClientCount-1] = (struct servertArgs*)(malloc(sizeof(struct servertArgs)));
            queue[nClientCount-1]->socket = clientFD;
            queue[nClientCount-1]->number = nClientCount;
            queue[nClientCount-1]->nextPort = gamePort;

            //spawn a new Server_Instance thread for the connection
            pthread_create(&(queue[nClientCount-1]->threadClient), NULL, Server_Instance, (queue[nClientCount-1]));
        }
        
        //join all threads in the queue
        for(i = 0; i < numPlayers; i++)
        {
            pthread_join(queue[i]->threadClient, NULL);
        }

        //fork to create a new process
        int cpid = fork();

        //in the child, run Game_Lobby to receive the connections and start a new game
        if(cpid == 0)
        {
            Game_Lobby(gamePort);
            return;
        }

        //in the parent, free the allocated memory, reset queueWaiting, and increment the port 
        //for the next game to run on
        for(i = 0; i < numPlayers; i++)
        {
            free(nonces[i]);
            free(queue[i]);
        }
        free(nonces);
        free(queue);
        queueWaiting = 0;
        gamePort++;
    }


}

int main(int argc, char *argv[])
{
    // seed random number
    srand(time(0));

    //default for all args
    numPlayers = 2; 
    lobPort = "41100";
    gamePort = 41101;
    numRounds = 1;
    dictFile = "mpwordleserver/defaultDict.txt";
    debug = false;

    if(argc > 12) {
        printf("Improper usage\n");
        exit(1);
    }

    //Process the args
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
            gamePort = atoi(argv[i+1]);
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
    printf("Game Port: %d\n", gamePort);
    printf("Number of rounds: %d\n", numRounds);
    printf("Dictionary: %s\n", dictFile);
    printf("Should debug: %s\n", (debug) ? "True" : "False");
	
    //Start the lobby
	Server_Lobby();

    return 0;
}

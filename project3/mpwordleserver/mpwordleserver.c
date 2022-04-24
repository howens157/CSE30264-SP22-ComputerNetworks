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
//#include "SocketHelper.h"

#define MAXDATASIZE 1024
#define BUFFER_MAX 1000
#define DEFAULT_LISTEN_BACKLOG      10

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

struct gametArgs **players;

struct gametArgs
{
    pthread_t threadClient;
    int socket;
    int myNum;
    char name[256];
    char result[256];
    int score;
    bool iWon;
    double lastReceipt;
};

struct servertArgs
{
    pthread_t threadClient;
    int socket;
    int number;
    int nextPort;
};

int queueWaiting = 0;
struct servertArgs **queue;

struct secStruct {
    char *name;
    int nonce;
};

struct secStruct **nonces;


pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void choose_answer() {
	printf("trying to open %s\n", dictFile);
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
	printf("count: %d\n", count);
	// get random int from 1 to count
	int rand_num = rand() % count;
	rand_num += 1;
	printf("rand num: %d\n", rand_num);

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
	printf("answer: %s\n", answer);
}

void check_guess_result(char* guess, char *result) {
	printf("answer: %s\n", answer);
	//printf("Your guess: ");
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
	printf("result: %s\n", result);
	return;
}

/*int createSocket_TCP_Listen (char * pszServer, char * pszPort)
{
    return createSocket_TCP_Listen_real (pszServer, pszPort, DEFAULT_LISTEN_BACKLOG);
}*/

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

void * Game_Instance(void *args)
{
    struct gametArgs * myArgs;
    struct gametArgs threadArgs;

    myArgs = (struct gametArgs *) args;
    threadArgs = *myArgs;

    int clientFD = threadArgs.socket;
    int myNum = threadArgs.myNum;

    printf("New player: socket: %d, number: %d\n", clientFD, myNum);
    // receive joinInstance message
    char szBuffer[BUFFER_MAX];
    int	 numBytes;

    printf("Receiving from %d\n", clientFD);
    if ((numBytes = recv(clientFD, szBuffer, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    szBuffer[numBytes] = '\0';

    printf("Received a message of %d bytes from Client\n", numBytes);
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
    char* clientName = (char*)json_reader_get_string_value(cmdReader);
    json_reader_end_member(cmdReader);
    json_reader_read_member(cmdReader, "Nonce");
    int nonce = (int)json_reader_get_int_value(cmdReader);
    json_reader_end_member(cmdReader);
    json_reader_end_member(cmdReader);				
    
    //TODO check that nonce is one of the approved nonces 
    int clientNonce = nonce;
    bool playerApproved = false;
    int i;
    for(i = 0; i < numPlayers; i++)
    {
        if(nonces[i]->nonce == clientNonce && strcmp(clientName, nonces[i]->name))
        {
            playerApproved = true;
        }
    }

    if(!playerApproved)
    {
        return NULL; //TODO: change lobby to be a while loop
    }

    strcpy((players[myNum-1]->name), clientName); 
    printf("Player %d's name is %s\n", myNum, players[myNum-1]->name);
    players[myNum-1]->score = 0;
    players[myNum-1]->iWon = false;

    printf("%s %s %d\n", msgType, clientName, clientNonce);

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

    printf("%s\n", joinInstRes);

    uint16_t len = strlen(joinInstRes);
    send(clientFD, joinInstRes, len, 0);
    sleep(1);

    int currRound;
    // Synchronization point if we have more than one player
    // All players need to join
    // Fill in that code later

    printf("Player %d waiting for players to join\n", myNum);
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

    printf("StartGame msg: %s\n", startGame);

    len = strlen(startGame);
    send(clientFD, startGame, len, 0);

    sleep(1);

    for(currRound = 1; currRound <= numRounds; currRound++)
    {
        players[myNum-1]->iWon = false;
        memset(players[myNum-1]->result, 0, 256);
        //initialize game variables
        if(myNum == 1)
        {
            choose_answer();
            pthread_mutex_lock(&lock);
            someoneWon = false;
            answerChosen = true;
            pthread_cond_broadcast(&cond);
            pthread_mutex_unlock(&lock);
        }

        pthread_mutex_lock(&lock);
        while(!answerChosen)
        {
            pthread_cond_wait(&cond, &lock);
        }
        pthread_mutex_unlock(&lock);

        //answer = "HELLO";
        int answerLen = strlen(answer);
        printf("This round's word is %s, len: %d\n", answer, answerLen);

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
            json_builder_add_int_value(builder, players[i]->score);
            json_builder_end_object(builder);
        }
        json_builder_end_array(builder);
        json_builder_end_object(builder);
        json_builder_end_object(builder);

        root = json_builder_get_root(builder);
        g = json_generator_new();
        json_generator_set_root(g, root);
        char *startRound = json_generator_to_data(g, NULL);

        printf("%s\n", startRound);

        len = strlen(startRound);
        send(clientFD, startRound, len, 0);
        sleep(1);

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

            printf("%s\n", promptGuess);

            len = strlen(promptGuess);
            send(clientFD, promptGuess, len, 0);

            // receive Guess message
            memset(szBuffer, 0, BUFFER_MAX);
            int	 numBytes;

            printf("Receiving from %d\n", clientFD);
            if ((numBytes = recv(clientFD, szBuffer, MAXDATASIZE-1, 0)) == -1) {
                perror("recv");
                exit(1);
            }

            szBuffer[numBytes] = '\0';

            // Debug / show what we got
            printf("Received a message of %d bytes from Client\n", numBytes);
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
            msgType = json_reader_get_string_value(cmdReader);
            json_reader_end_member(cmdReader);
            json_reader_read_member(cmdReader, "Data");
            // maybe process name
            // json_reader_read_member(cmdReader, "Name");
            // char* name = (char*)json_reader_get_string_value(cmdReader);
            // json_reader_end_member(cmdReader);
            json_reader_read_member(cmdReader, "Guess");
            char *guess = (char*)json_reader_get_string_value(cmdReader);
            json_reader_end_member(cmdReader);
            json_reader_end_member(cmdReader);		

            printf("%s guessed %s\n", clientName, guess);		

            //set receipt time in PlayersArr
            struct timeval receipt;
            gettimeofday(&receipt, NULL);
            players[myNum-1]->lastReceipt = (receipt.tv_sec) + (receipt.tv_usec)/1000000.0;
            printf("got guess at %lf\n", players[myNum-1]->lastReceipt);


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

            printf("%s\n", guessResponse);

            len = strlen(guessResponse);
            send(clientFD, guessResponse, len, 0);

            sleep(1);
            
            //check client's guess
            if (!strncmp(answer, guess, answerLen)) {
                pthread_mutex_lock(&lock);
                someoneWon = true;
                pthread_mutex_unlock(&lock);
                players[myNum-1]->iWon = true;
                players[myNum-1]->score += answerLen;
                printf("we have a winner\n");
            }
            check_guess_result(guess, (players[myNum-1]->result));

            pthread_mutex_lock(&lock);
            playersGuessed++;
            pthread_cond_broadcast(&cond);
            pthread_mutex_unlock(&lock);

            // Synchronization point if we have more than one player
                // All players need to have sent a guess
                // Fill in that code later
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

            printf("%s\n", GuessResult);

            len = strlen(GuessResult);
            send(clientFD, GuessResult, len, 0);
            sleep(1); 

            //reset playersGuessed for new guess
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
                // Fill in that code later
            if(someoneWon)
                break;
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
            json_builder_add_string_value(builder, "Yes");
            json_builder_end_object(builder);
        }
        json_builder_end_array(builder);
        json_builder_end_object(builder);
        json_builder_end_object(builder);

        root = json_builder_get_root(builder);
        g = json_generator_new();
        json_generator_set_root(g, root);
        char *EndRound = json_generator_to_data(g, NULL);

        printf("%s\n", EndRound);

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

    printf("%s\n", EndGame);

    len = strlen(EndGame);
    send(clientFD, EndGame, len, 0);

    sleep(5);

    return NULL;
}

void Game_Lobby(int portNum)
{
    players = (struct gametArgs**)(malloc(numPlayers*sizeof(struct gametArgs*)));
    char myPort[100];
    sprintf(myPort, "%d", portNum);
    int serverFD =  createSocket_TCP_Listen(NULL, myPort);
    int nClientCount = 0;

    int i;
    for(i = 0; i < numPlayers; i++)
    {
        struct sockaddr_storage their_addr;
        socklen_t sin_size;
        char s[INET6_ADDRSTRLEN];

        
        printf("game server: waiting for connections...\n");
        
        sin_size = sizeof their_addr;
        int clientFD = accept(serverFD, (struct sockaddr *)&their_addr, &sin_size);
        printf("receiving from %d\n", clientFD);
        if (clientFD == -1) 
        {
            perror("accept");
            return;
        }

        inet_ntop(their_addr.ss_family,
                get_in_addr((struct sockaddr *)&their_addr),
                s, sizeof s);
        printf("server: got connection from %s\n", s);
        nClientCount++;
        players[nClientCount-1] = (struct gametArgs*)(malloc(sizeof(struct gametArgs)));
        players[nClientCount-1]->socket = clientFD;
        players[nClientCount-1]->myNum = nClientCount;
        printf("receiving from %d\n", players[nClientCount-1]->socket);

        pthread_create(&(players[nClientCount-1]->threadClient), NULL, Game_Instance, (players[nClientCount-1]));
    }

    pthread_mutex_lock(&lock);
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&lock);
    
    for(i = 0; i < numPlayers; i++)
    {
        pthread_join(players[i]->threadClient, NULL);
    }

}

void * Server_Instance(void * args)
{
    struct servertArgs * myArgs;
    struct servertArgs threadArgs;

    myArgs = (struct servertArgs *) args;
    threadArgs = *myArgs;

    int clientFD = threadArgs.socket;
    int myNum = threadArgs.number;
    char nextPort[100];
    sprintf(nextPort, "%d", threadArgs.nextPort);

    printf("new player waiting for game on port %s\n", nextPort);

    printf("New waiting: socket: %d\n", clientFD);
    // receive join message
    char szBuffer[BUFFER_MAX];
    int	 numBytes;

    printf("Receiving from %d\n", clientFD);
    if ((numBytes = recv(clientFD, szBuffer, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    szBuffer[numBytes] = '\0';

    printf("Received a message of %d bytes from Client\n", numBytes);
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
    json_reader_read_member(cmdReader, "Client");
    char* clientID = (char *)json_reader_get_string_value(cmdReader);
    json_reader_end_member(cmdReader);
    json_reader_end_member(cmdReader);	

    

    printf("%s: player: %s from %s waiting\n", msgType, name, clientID);	

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

    printf("%s\n", joinRes);

    uint16_t len = strlen(joinRes);
    send(clientFD, joinRes, len, 0);
    sleep(1);

    //wait for the lobby to be full
    printf("Player %s waiting for players to join\n", name);
    pthread_mutex_lock(&lock);
    queueWaiting++;
    pthread_cond_broadcast(&cond);
    while(queueWaiting < numPlayers)
    {
        pthread_cond_wait(&cond, &lock);
    }
    pthread_mutex_unlock(&lock);

    printf("starting new game\n");
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

    printf("%s\n", startInstance);

    len = strlen(startInstance);
    send(clientFD, startInstance, len, 0);
    sleep(1);
    return NULL;
}

void Server_Lobby()
{
    
    int serverFD =  createSocket_TCP_Listen(NULL, lobPort);
    while(1)
    {
        queue = (struct servertArgs**)(malloc(numPlayers*sizeof(struct servertArgs*)));
        int nClientCount = 0;
        nonces = (struct secStruct**)(malloc(numPlayers*sizeof(struct secStruct*)));
        int i;
        for(i = 0; i < numPlayers; i++)
        {
            struct sockaddr_storage their_addr;
            socklen_t sin_size;
            char s[INET6_ADDRSTRLEN];
            
            printf("Server lobby: waiting for connections...\n");
            
            sin_size = sizeof their_addr;
            int clientFD = accept(serverFD, (struct sockaddr *)&their_addr, &sin_size);
            printf("receiving from %d\n", clientFD);
            if (clientFD == -1) 
            {
                perror("accept");
                return;
            }

            inet_ntop(their_addr.ss_family,
                    get_in_addr((struct sockaddr *)&their_addr),
                    s, sizeof s);
            printf("server: got connection from %s\n", s);
            nClientCount++;
            queue[nClientCount-1] = (struct servertArgs*)(malloc(sizeof(struct servertArgs)));
            queue[nClientCount-1]->socket = clientFD;
            queue[nClientCount-1]->number = nClientCount;
            queue[nClientCount-1]->nextPort = gamePort;
            printf("receiving from %d\n", queue[nClientCount-1]->socket);

            pthread_create(&(queue[nClientCount-1]->threadClient), NULL, Server_Instance, (queue[nClientCount-1]));
        }

        pthread_mutex_lock(&lock);
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&lock);
        
        for(i = 0; i < numPlayers; i++)
        {
            pthread_join(queue[i]->threadClient, NULL);
        }

        int cpid = fork();

        if(cpid == 0)
        {
            Game_Lobby(gamePort);
            return;
        }

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
		
    // signal(SIGINT, sigint_handler);

    numPlayers = 1; //change to 2
    lobPort = "41100";
    gamePort = 41101;
    numRounds = 1;
    dictFile = "mpwordleserver/defaultDict.txt";
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
	
	Server_Lobby();
	
	printf("And we are done\n");

    return 0;
}

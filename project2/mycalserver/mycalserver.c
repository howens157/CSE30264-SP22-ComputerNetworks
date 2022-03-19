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
#include <stdbool.h>
#include <json-glib/json-glib.h>
#include <json-glib/json-gobject.h>

#define BACKLOG 10   // how many pending connections queue will hold
#define MAXDATASIZE 1024

int sockfd;
int new_fd;

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

int main(int argc, char *argv[])
{
    int sockfd, new_fd;  // listen on sockfd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    const char *PORT;
    bool shouldFork = false;

    signal(SIGINT, sigint_handler);

    if(argc > 1) {
        if(strcmp(argv[1], "-mt") != 0)
        {
            printf("Usage: ./mycalserver/mycalserver FLAGS\n");
            printf("Optional Flags:\n");
            printf("\t-mt: enable the server to use fork() to handle multiple connections at once.\n");
            exit(0);
        }
        printf("Forking Enabled\n");
        shouldFork = true;
    }

    //Parse .mycal JSON to get port number
    g_autoptr(JsonParser) parser = json_parser_new();
    g_autoptr(GError) error = NULL;
    json_parser_load_from_file(parser, "mycalserver/.mycal", &error);
    if(error != NULL)
    {
        g_critical("Unable to parse .mycal: %s", error->message);
        exit(1);
    }
    g_autoptr(JsonNode) root = json_parser_get_root(parser);
    g_autoptr(JsonReader) reader = json_reader_new(root);
    json_reader_read_member(reader, "port");
    PORT = json_reader_get_string_value(reader);
    printf("Read Port Number from .mycal: %s\n", PORT);

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
    //FILE *fdr; 
    int numBytes;
    uint16_t commandLength;
    //uint32_t fileSize;
    //char bufToSend[MAXDATASIZE];
    int cpid = 1;

    printf("server: waiting for connections...\n");
    while(1) {  // main accept() loop

        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);

        if(shouldFork)
        {
            cpid = fork();
        }

        if(cpid == 0 || !shouldFork)
        {
            if(cpid == 0)
            {
                close(sockfd);
            }
            //receive the length of the incoming command from the client
            numBytes = recv(new_fd, &commandLength, sizeof(uint16_t), 0);
            if(numBytes < 0) {
                perror("Error receiving length of command\n");
                continue;
            }
            commandLength = ntohs(commandLength);
            printf("Received length of command: %d\n", commandLength);

            //create a buffer to store the command and receive it from the client
            char commandBuf[commandLength+1];
            memset(commandBuf, 0, sizeof(commandBuf));
            numBytes = recv(new_fd, commandBuf, sizeof(commandBuf), 0);
            if(numBytes < 0) {
                perror("Error receiving command\n");
                continue;
            }
            printf("Received command: %s\n", commandBuf);

            //Parse command
            g_autoptr(JsonParser) cmdParser = json_parser_new();
            g_autoptr(GError) error = NULL;
            json_parser_load_from_data(cmdParser, commandBuf, commandLength, &error);
            if(error != NULL)
            {
                g_critical("Unable to parse command: %s", error->message);
                exit(1);
            }
            g_autoptr(JsonNode) cmdRoot = json_parser_get_root(cmdParser);

            //Read calenderName and action values
            g_autoptr(JsonReader) cmdReader = json_reader_new(cmdRoot);
            json_reader_read_member(cmdReader, "calendarName");
            const char* calName = json_reader_get_string_value(cmdReader);
            json_reader_end_member(cmdReader);
            json_reader_read_member(cmdReader, "action");
            const char* action = json_reader_get_string_value(cmdReader);
            json_reader_end_member(cmdReader);

            //Handle the action appropriately
            if(strcmp(action, "add") == 0)
            {
                json_reader_read_member(cmdReader, "arguments");
                json_reader_read_member(cmdReader, "date");
                const char* date = json_reader_get_string_value(cmdReader);
                json_reader_end_member(cmdReader);
                json_reader_read_member(cmdReader, "time");
                const char* time = json_reader_get_string_value(cmdReader);
                json_reader_end_member(cmdReader);
                json_reader_read_member(cmdReader, "duration");
                const char* dur = json_reader_get_string_value(cmdReader);
                json_reader_end_member(cmdReader);
                json_reader_read_member(cmdReader, "name");
                const char* name = json_reader_get_string_value(cmdReader);
                json_reader_end_member(cmdReader);
                json_reader_read_member(cmdReader, "description");
                const char* desc = json_reader_get_string_value(cmdReader);
                json_reader_end_member(cmdReader);
                json_reader_read_member(cmdReader, "location");
                const char* loc = json_reader_get_string_value(cmdReader);
                json_reader_end_member(cmdReader);

                printf("calendarName: %s, action: %s, date: %s, time: %s, duration: %s, name: %s, description: %s, location: %s\n", calName, action, date, time, dur, name, desc, loc);
            }
            else if(strcmp(action, "remove") == 0)
            {
                json_reader_read_member(cmdReader, "arguments");
                json_reader_read_member(cmdReader, "identifier");
                const char* ident = json_reader_get_string_value(cmdReader);
                json_reader_end_member(cmdReader);

                printf("calendarName: %s, action: %s, identifier: %s\n", calName, action, ident);
            }
            else if(strcmp(action, "update") == 0)
            {
                json_reader_read_member(cmdReader, "arguments");
                json_reader_read_member(cmdReader, "identifier");
                const char* ident = json_reader_get_string_value(cmdReader);
                json_reader_end_member(cmdReader);
                json_reader_read_member(cmdReader, "field");
                const char* field = json_reader_get_string_value(cmdReader);
                json_reader_end_member(cmdReader);
                json_reader_read_member(cmdReader, "value");
                const char* value = json_reader_get_string_value(cmdReader);
                json_reader_end_member(cmdReader);

                printf("calendarName: %s, action: %s, identifier: %s, field: %s, value: %s\n", calName, action, ident, field, value);
            }
            else if(strcmp(action, "get") == 0)
            {
                json_reader_read_member(cmdReader, "arguments");
                json_reader_read_member(cmdReader, "date");
                const char* date = json_reader_get_string_value(cmdReader);
                json_reader_end_member(cmdReader);

                printf("calendarName: %s, action: %s, date: %s\n", calName, action, date);
            }
            else if(strcmp(action, "getrange") == 0)
            {
                json_reader_read_member(cmdReader, "arguments");
                json_reader_read_member(cmdReader, "start");
                const char* start = json_reader_get_string_value(cmdReader);
                json_reader_end_member(cmdReader);
                json_reader_read_member(cmdReader, "end");
                const char* end = json_reader_get_string_value(cmdReader);
                json_reader_end_member(cmdReader);

                printf("calendarName: %s, action: %s, startDate: %s, endDate: %s\n", calName, action, start, end);
            }

            if(cpid == 0)
            {
                close(new_fd);
                exit(0);
            }
        }
        close(new_fd);
    }

    return 0;
}

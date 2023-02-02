/*

  Rhizomatica SMS receiver sample

 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>

#include "process_sms.h"


void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char **argv)
{    
    int parentfd;
    int childfd;
    int portno;
    unsigned int clientlen;
    char *hostaddrp;
    int optval;
    struct sockaddr_in serveraddr;
    struct sockaddr_in clientaddr;
    
    FILE *stream;
    char buf[BUFSIZE];
    char method[BUFSIZE];
    char uri[BUFSIZE];
    char version[BUFSIZE];

    bool is_sms = false;
    char *char_ptr = NULL;

    if (argc < 1 || argc > 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    portno = atoi(argv[1]);
    
    /* open socket descriptor */
    parentfd = socket(AF_INET, SOCK_STREAM, 0);
    if (parentfd < 0)
        error("ERROR opening socket");
    
    /* allows us to restart server immediately */
    optval = 1;
    setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR,
               (const void *)&optval , sizeof(int));
    
    /* bind port to socket */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)portno);
    if (bind(parentfd, (struct sockaddr *) &serveraddr,
             sizeof(serveraddr)) < 0)
        error("ERROR on binding");
    
    /* get us ready to accept connection requests */
    if (listen(parentfd, 5) < 0) /* allow 5 requests to queue up */
        error("ERROR on listen");
    
    /*
     * main loop: wait for a connection request, parse HTTP,
     * serve requested content, close connection.
     */
    clientlen = sizeof(clientaddr);
    while (1) {
        
        /* wait for a connection request */
        childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);
        if (childfd < 0)
            error("ERROR on accept");
        
        hostaddrp = inet_ntoa(clientaddr.sin_addr);
        if (hostaddrp == NULL)
            error("ERROR on inet_ntoa\n");
        
        /* open the child socket descriptor as a stream */
        if ((stream = fdopen(childfd, "r+")) == NULL)
            error("ERROR on fdopen");
        
        /* get the HTTP request line */
        fgets(buf, BUFSIZE, stream);
        printf("%s", buf);
        sscanf(buf, "%s %s %s\n", method, uri, version);
        
        if (strcasecmp(method, "GET")) {
            fprintf(stream, "HTTP/1.1 400 Bad Request\n");
            fprintf(stream, "\r\n");
            fclose(stream);
            close(childfd);
            continue;
        }

        is_sms = false;
        /* read the HTTP headers */
        do
        {
            fgets(buf, BUFSIZE, stream);
            char_ptr = strstr(buf, "Nexmo/MessagingHUB/v1.0");
            if (char_ptr != NULL)
                is_sms = true;
            printf("%s", buf);
        } while(strcmp(buf, "\r\n"));

        if (is_sms)
        {
            process_sms(uri);
        }

        /* print response in case of success */
        // fprintf(stream, "HTTP/1.1 200 OK\n");
        fprintf(stream, "HTTP/1.1 204 No Content\n");
        // fprintf(stream, "Server: HERMES SMS\n");
        fprintf(stream, "\r\n");
        fflush(stream);

        fclose(stream);
        close(childfd);
    }
        
    /* clean up */
    fclose(stream);
    close(childfd);
    close(parentfd);
}

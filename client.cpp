//
// Created by Praise Olukilede on 3/16/23.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#include <arpa/inet.h>

#define SERVERPORT "24607"
#define MAXDATASIZE 200

int main(int argc, char *argv[]){
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv, port;
    //char s[INET6_ADDRSTRLEN];
    struct sockaddr_storage addr;
    socklen_t addr_size;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if((rv = getaddrinfo("localhost", SERVERPORT, &hints, &servinfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("client: connect");
            close(sockfd);
            continue;
        }

        break;
    }

    if(p == NULL){
        fprintf(stderr, "client: failed to connect");
        return 2;
    }


    if(getsockname(sockfd, (struct sockaddr *)&addr, (socklen_t *)&addr_size) == -1){
        perror("client: getsockname");
        return 2;
    }

    if(addr.ss_family == AF_INET){
        struct sockaddr_in *s = (struct sockaddr_in *)&addr;
        port = ntohs(s->sin_port);
    } else {
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
        port = ntohs(s->sin6_port);
    }

    freeaddrinfo(servinfo);
    fprintf(stdout, "Client is up and running\n");

    for(;;){
        char user_input[MAXDATASIZE];
        fprintf(stdout, "Please enter the usernames to check schedule availability: \n");
        fgets(user_input, MAXDATASIZE, stdin);
        if(send(sockfd, user_input, strlen(user_input), 0) == -1){
            perror("send");
            exit(1);
        }
        fprintf(stdout, "Client finished sending usernames to Main Server.\n");
        while(1){
            if((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buf[numbytes] = '\0';
            if(strcmp(buf, "end") == 0){
                break;
            }
            fprintf(stdout, "Client received reply from the main server using TCP over port %d\n", port);
            if(strlen(buf) > 0){
                fprintf(stdout, "%s\n", buf);
            }
        }
        memset(buf, 0, sizeof(buf));
        memset(user_input, 0, sizeof(user_input));
        fprintf(stdout, "-----Start a new request-----\n");
    }
}






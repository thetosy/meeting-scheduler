//
// Created by Praise Olukilede on 3/15/23.
//
#include <iostream>
#include <stdlib.h>
#include <string>
#include <errno.h>
#include <map>
#include <unordered_map>
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <cctype>
#include <algorithm>

#include "timeslotsFuncs.h"
#define SERVERPORT "23607"
#define MYPORT "22607"
#define FALSE 0
#define TRUE 1
#define MAXBUFLEN 100

using namespace std;

int send_sockfd, server_sockfd;
struct addrinfo hints, *servinfo, *servinfo_serverM, *p_serverM, *p;
int rv;
int numbytes;
struct sockaddr_storage their_addr;
char buf[MAXBUFLEN];
socklen_t addr_len;
char s[INET6_ADDRSTRLEN];


unordered_map<string, a_timeslot*> dataB;

int createServerSocket(){
    // create server connection and bind
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;
    // hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo("localhost", MYPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return FALSE;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((server_sockfd = socket(p->ai_family, p->ai_socktype,
                                    p->ai_protocol)) == -1) {
            perror("serverA: socket");
            continue;
        }

        if (::bind(server_sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(server_sockfd);
            perror("serverA: bind");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "serverA: failed to bind socket\n");
        return FALSE;
    }
    //freeaddrinfo(servinfo);
    return TRUE;
}



int readBuildData(){
    char buf[2000];
    char *semicolon;
    int size;
    FILE *fp = NULL;
    if((fp = fopen("b.txt", "r")) == NULL){
        //cerr << "Error opening file" << endl;
        fprintf(stderr, "Error opening file\n");
        return FALSE;
    }
    while((fgets(buf, sizeof(buf), fp)) != NULL){
        semicolon = strchr(buf, ';');
        if(semicolon != NULL){
            *semicolon++ = '\0';
        }
        string username = string(buf);
        username.erase(remove_if(username.begin(), username.end(), [](char c) { return isspace(c); }), username.end());
        size = strlen(semicolon);
        semicolon[size-1] = '\0';
        a_timeslot *timeslot = (a_timeslot*)malloc(sizeof(a_timeslot));
        createTimeSlots(timeslot, semicolon);
        dataB[username] = timeslot;
    }
    fclose(fp);
    return 1;
}

int sendDataToMainServer(){
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;
    if((rv = getaddrinfo("localhost", SERVERPORT, &hints, &servinfo_serverM)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        //cerr << "getaddrinfo: " << gai_strerror(rv) << '\n';
        return FALSE;
    }

    for(p_serverM = servinfo_serverM; p_serverM != NULL; p_serverM = p_serverM->ai_next){
        if((send_sockfd = socket(p_serverM->ai_family, p_serverM->ai_socktype, p_serverM->ai_protocol)) == -1){
            perror("serverB: socket");
            continue;
        }
        break;
    }
    if(p_serverM == NULL){
        fprintf(stderr, "serverB: failed to create socket\n");
        //cerr << "serverA: failed to create socket" << '\n';
        return FALSE;
    }

    for(const auto &ele: dataB){
        int size;
        //cout << ele.first << " " << ele.second << '\n';
        //fprintf(stdout, "%s %s\n", ele.first.c_str(), ele.second.c_str());
        char buf[50];
        strncpy(buf, ele.first.c_str(), 20);
        strncat(buf, ";B", 3);
        //fprintf(stdout, "%s\n", buf);
        size = strlen(buf);
        buf[size] = '\0';
        //fprintf(stdout, "%s\n", buf);
        if((numbytes = sendto(send_sockfd, buf, size, 0, p_serverM->ai_addr, p_serverM->ai_addrlen)) == -1){
            perror("serverB: sendto");
            return FALSE;
        }
    }
    string end = "end;B";
    if((numbytes = sendto(send_sockfd, end.c_str(), strlen(end.c_str()), 0, p_serverM->ai_addr, p_serverM->ai_addrlen)) == -1){
        perror("serverB: sendto");
        return FALSE;
    }
    //freeaddrinfo(servinfo);
    // close(sockfd);
    return TRUE;

}

int main(void){
    if(createServerSocket() != TRUE){
        fprintf(stderr, "Error creating serverB socket\n");
        exit(1);
    }
    fprintf(stdout, "ServerB is up and running using UDP on port %s.\n", MYPORT);
    readBuildData();
    if(sendDataToMainServer() != TRUE){
        exit(1);
    }
    fprintf(stdout, "ServerB finished sending list of usernames to Main Server.\n");
//    timeslots *p = dataB["kinsley"];
//    fprintf(stdout, "%d\n", p->times[0].end);
    addr_len = sizeof(their_addr);

    while(1){
        char new_buf[MAXBUFLEN];
        if ((numbytes = recvfrom(server_sockfd, new_buf, MAXBUFLEN - 1, 0, (struct sockaddr *) &their_addr, &addr_len)) ==
            -1) {
            perror("recvfrom");
            exit(1);
        }
        new_buf[numbytes] = '\0';
        //string usernames = string(new_buf);
        fprintf(stdout, "ServerB received the usernames from Main Server using UDP over port %s.\n", MYPORT);
        // change to local variable
        //a_timeslot *timeslot = (a_timeslot*)malloc(sizeof(a_timeslot));
        a_timeslot timeslot;
        timeslot.count = 0;
        findCommonTimeSlots(new_buf, dataB, &timeslot);
        char reply[MAXBUFLEN];
        convertTimeslotsToString(reply, &timeslot);
        fprintf(stdout, "Found the intersection result: %s for %s.\n", reply, new_buf);
        strncat(reply, ";B", 3);
        if((numbytes = sendto(send_sockfd, reply, strlen(reply), 0, p_serverM->ai_addr, p_serverM->ai_addrlen)) == -1){
            perror("serverB: send intersection");
            exit(1);
        }
        fprintf(stdout, "Server B finished sending the response to Main Server\n");
    }
}





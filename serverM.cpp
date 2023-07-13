//
// Created by Praise Olukilede on 3/15/23.
//
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unordered_set>
#include "timeslotsFuncs.h"

#define FALSE 0
#define TRUE 1
#define MYPORT "23607"
#define MYPORT_TCP "24607"
#define SERVERAPORT "21607"
#define SERVERBPORT "22607"
#define MAXBUFLEN 200
#define BACKLOG 10

using namespace std;

unordered_set <string> usersA;
unordered_set <string> usersB;

int sockfd_udp, sockfd_tcp, clientfd, serverA_sockfd, serverB_sockfd;
struct addrinfo hints, *servinfo, *servinfo_serverA, *servinfo_serverB, *p, *p_serverA, *p_serverB;
int rv;
int numbytes;
struct sockaddr_storage their_addr;
char buf[MAXBUFLEN];
char reply[MAXBUFLEN];
socklen_t addr_len, sin_size;
char s[INET6_ADDRSTRLEN];
struct sigaction sa;
int yes = 1;


void sigchld_handler(int s) {
    (void) s; // quiet unused variable warning

    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

int createMyServerBSocket(){
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo("localhost", SERVERBPORT, &hints, &servinfo_serverB)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return FALSE;
    }

    // loop through all the results and make a socket
    for(p_serverB = servinfo_serverB; p_serverB != NULL; p_serverB = p_serverB->ai_next) {
        if ((serverB_sockfd = socket(p_serverB->ai_family, p_serverB->ai_socktype,
                                     p_serverB->ai_protocol)) == -1) {
            perror("serverM-B: socket");
            continue;
        }
        break;
    }

    if (p_serverB == NULL) {
        fprintf(stderr, "serverM-B: failed to create socket\n");
        return FALSE;
    }
    //freeaddrinfo(servinfo);
    return TRUE;
}

int createMyServerASocket(){
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo("localhost", SERVERAPORT, &hints, &servinfo_serverA)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return FALSE;
    }

    // loop through all the results and make a socket
    for(p_serverA = servinfo_serverA; p_serverA != NULL; p_serverA = p_serverA->ai_next) {
        if ((serverA_sockfd = socket(p_serverA->ai_family, p_serverA->ai_socktype,
                                     p_serverA->ai_protocol)) == -1) {
            perror("serverM-A: socket");
            continue;
        }
        break;
    }

    if (p_serverA == NULL) {
        fprintf(stderr, "serverM-A: failed to create socket\n");
        return FALSE;
    }
    //freeaddrinfo(servinfo);
    return TRUE;
}


int getUsersFromBackupServers() {
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;
    //hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo("localhost", MYPORT, &hints, &servinfo)) != 0) {
        cerr << "getaddrinfo: " << gai_strerror(rv) << '\n';
        return FALSE;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd_udp = socket(p->ai_family, p->ai_socktype,
                                 p->ai_protocol)) == -1) {
            perror("serverM: socket");
            continue;
        }

        if (::bind(sockfd_udp, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd_udp);
            perror("serverM: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        cerr << "serverM: failed to bind socket" << '\n';
        return FALSE;
    }

    freeaddrinfo(servinfo);
    int all_from_a = FALSE;
    int all_from_b = FALSE;
    addr_len = sizeof(their_addr);
    while (!all_from_a || !all_from_b) {
        char *server;
        if ((numbytes = recvfrom(sockfd_udp, buf, MAXBUFLEN - 1, 0, (struct sockaddr *) &their_addr, &addr_len)) ==
            -1) {
            perror("recvfrom");
            return FALSE;
        }
        buf[numbytes] = '\0';
        //fprintf(stdout, "%s", buf);
        server = strchr(buf, ';');
        if (server != NULL) {
            *server++ = '\0';
        }

        if (strcmp(server, "A") == 0) {
            if (strcmp(buf, "end") == 0) {
                all_from_a = TRUE;
                fprintf(stdout, "Main Server received the username list from serverA using UDP over port %s\n",
                        MYPORT);
            } else {
                //fprintf(stdout, "%s\n", buf);
                usersA.insert(buf);
            }
        } else {
            if (strcmp(buf, "end") == 0) {
                all_from_b = TRUE;
                fprintf(stdout, "Main Server received the username list from serverB using UDP over port %s\n",
                        MYPORT);
            } else {
                //fprintf(stdout, "%s\n", buf);
                usersB.insert(buf);
            }

        }
    }
    return TRUE;
}

int main(void) {
    fprintf(stdout, "Main Server is up and running\n");
    if (getUsersFromBackupServers() == FALSE) {
        //fprintf(stdout, "here\n");
        exit(1);
    }
    if(createMyServerASocket() != TRUE){
        fprintf(stderr, "Error connecting to serverA\n");
        exit(1);
    }

    if(createMyServerBSocket() != TRUE){
        fprintf(stderr, "Error connecting to serverB\n");
        exit(1);
    }

//    for (auto ele: usersA) {
//        fprintf(stdout, "%s\n", ele.c_str());
//    }
//
//    for (auto ele: usersB) {
//        fprintf(stdout, "%s\n", ele.c_str());
//    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    //hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo("localhost", MYPORT_TCP, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd_tcp = socket(p->ai_family, p->ai_socktype,
                                 p->ai_protocol)) == -1) {
            perror("serverM: socket");
            continue;
        }

        if (setsockopt(sockfd_tcp, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        if (::bind(sockfd_tcp, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd_tcp);
            perror("serverM: bind");
            continue;
        }
        break;
    }
    freeaddrinfo(servinfo);

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd_tcp, BACKLOG) == -1) {
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

    sin_size = sizeof(their_addr);
    //char buf[MAXDATASIZE];
    clientfd = accept(sockfd_tcp, (struct sockaddr *) &their_addr, &sin_size);
    if (clientfd == -1) {
        perror("accept");
        exit(1);
    }

    while (1) {  // main accept() loop
        memset(buf, 0, sizeof(buf));
        memset(reply, 0, sizeof(reply));
        string sA = "";
        string sB = "";
        string notIn = "";

        //char sB[MAXBUFLEN];

        if ((numbytes = recv(clientfd, buf, MAXBUFLEN - 1, 0)) == -1) {
            perror("recv");
        }
        fprintf(stdout, "Main Server received request from the client using TCP over port %s \n", MYPORT_TCP);
        buf[numbytes] = '\0';
        //fprintf(stdout, "main server recieved %s \n", buf);
        //strcpy(reply, buf);
        // check where the usernames reside
        char *start_ptr = buf;
        char *space_ptr = NULL;
        int is_beginningA = 0;
        int is_beginningB = 0;
        int is_notIn = 0;
        int num_a = 0;
        int num_b = 0;
        int num_c = 0;
        for (;;) {
            // username delimeteed by space
            space_ptr = strchr(start_ptr, ' ');
            if (space_ptr != NULL) {
                *space_ptr++ = '\0';
                string user = string(start_ptr);
                if (usersA.find(user) != usersA.end()) {
                    // found in usersA
                    if (is_beginningA == 0) {
                        sA = sA + user;
                        is_beginningA = 1;
                    } else {
                        sA = sA + ", " + user;
                    }
                    num_a++;
                } else if (usersB.find(user) != usersB.end()) {
                    // found in usersB
                    if (is_beginningB == 0) {
                        sB = sB + user;
                        is_beginningB = 1;
                    } else {
                        sB = sB + ", " + user;
                    }
                    num_b++;
                } else {
                    // not found in both lists
                    if (is_notIn == 0) {
                        notIn = notIn + user;
                        is_notIn = 1;
                    } else {
                        notIn = notIn + ", " + user;
                    }
                    num_c++;
                }
                start_ptr = space_ptr;
            } else {
                // check last user name before exit
                start_ptr[strlen(start_ptr) - 1] = '\0';
                string user = string(start_ptr);
                if (usersA.find(user) != usersA.end()) {
                    // found in usersA
                    if (is_beginningA == 0) {
                        sA = sA + user;
                        is_beginningA = 1;
                    } else {
                        sA = sA + ", " + user;
                    }
                    num_a++;
                } else if (usersB.find(user) != usersB.end()) {
                    // found in usersB
                    if (is_beginningB == 0) {
                        sB = sB + user;
                        is_beginningB = 1;
                    } else {
                        sB = sB + ", " + user;
                    }
                    num_b++;
                } else {
                    // not found in both lists
                    if (is_notIn == 0) {
                        notIn = notIn + user;
                        is_notIn = 1;
                    } else {
                        notIn = notIn + ", " + user;
                    }
                    num_c++;
                }
                break;
            }
        }

        if((num_a + num_b + num_c) > 10){
            string tooManyUserNames = "Too many usernames given. Maximum number is 10\n";
            if (send(clientfd, tooManyUserNames.c_str(), tooManyUserNames.length(), 0) == -1) {
                perror("send");
                exit(1);
            }
            usleep(500);
            string end = "end";
            if(send(clientfd, end.c_str(), end.length(), 0) == -1){
                perror("send");
                exit(1);
            }
            continue;
        }

        if (notIn.length() != 0) {
            //char error_buf[50];
            cout << notIn << " do not exist. Send a reply to the client.\n";
            //fprintf(stdout, "%s do not exist. Send a reply to client.\n", notIn.c_str());
            notIn = notIn + " do not exist";
            //sprintf(error_buf, "%s do not exist\n", notIn.c_str());
            if (send(clientfd, notIn.c_str(), notIn.length(), 0) == -1) {
                perror("send");
                exit(1);
            }
        }
        int timelist_from_a = TRUE;
        int timelist_from_b = TRUE;
        int sentA = 0;
        int sentB = 0;
        int end_transmission = 0;

        if(sA.length() == 0 && sB.length() == 0){
            // none of the usernames are in either servers
            string end = "end";
            if(send(clientfd, end.c_str(), end.length(), 0) == -1){
                perror("send");
                exit(1);
            }
            continue;
        }

        if (sA.length() > 0) {
            cout << "Found " << sA << " located at serverA. Send to serverA.\n";
            //remove(sA.begin(), sA.end(), ' ');
            if ((numbytes = sendto(serverA_sockfd, sA.c_str(), strlen(sA.c_str()), 0, p_serverA->ai_addr,
                                   p_serverA->ai_addrlen)) == -1) {
                perror("serverM: sendto-serverA");
                exit(1);
            }
            sentA = 1;
            timelist_from_a = FALSE;
        }

        if (sB.length() > 0) {
            cout << "Found " << sB << " located at serverB. Send to serverB.\n";
            //remove(sB.begin(), sB.end(), ' ');
            if ((numbytes = sendto(serverB_sockfd, sB.c_str(), strlen(sB.c_str()), 0, p_serverB->ai_addr,
                                   p_serverB->ai_addrlen)) == -1) {
                perror("serverM: sendto-serverB");
                exit(1);
            }
            sentB = 1;
            timelist_from_b = FALSE;
        }


        memset(buf, 0, sizeof(buf));
        addr_len = sizeof(their_addr);
        char a_timelist[MAXBUFLEN];
        char b_timelist[MAXBUFLEN];

        while (!timelist_from_a || !timelist_from_b) {
            char *server;
            if ((numbytes = recvfrom(sockfd_udp, buf, MAXBUFLEN - 1, 0, (struct sockaddr *) &their_addr, &addr_len)) ==
                -1) {
                perror("recvfrom");
                return FALSE;
            }
            buf[numbytes] = '\0';
            server = strchr(buf, ';');
            if (server != NULL) {
                *server++ = '\0';
            }

            if (strcmp(server, "A") == 0) {
                fprintf(stdout,
                        "Main Server received from server A list the intersection result using UDP over port %s:\n",
                        MYPORT);
                fprintf(stdout, "%s\n", buf);
                strcpy(a_timelist, buf);
                timelist_from_a = TRUE;
            } else {
                fprintf(stdout,
                        "Main Server received from server B list the intersection result using UDP over port %s:\n",
                        MYPORT);
                fprintf(stdout, "%s\n", buf);
                strcpy(b_timelist, buf);
                timelist_from_b = TRUE;

            }
        }

        if (sentA == 1) {
                // send to client no available timelist match for all usernames
                if(strcmp(a_timelist, "[]") == 0){
                    //cout << "here1" << "\n";
                    cout << "Found the intersection between results from server A and B:\n[].\n";
                    string client_reply = "Time intervals [] works for " + sA + ", " + sB;
                    if(send(clientfd, client_reply.c_str(), client_reply.length(), 0) == -1){
                        perror("send -1");
                        exit(1);
                    }
                    cout << "Main Server sent the result to the client.\n";
                    end_transmission = 1;
                }
        }
        if(end_transmission != 1){
            if(sentB == 1){
                if(strcmp(b_timelist, "[]") == 0){
                    //cout << "here2" << "\n";
                    cout << "Found the intersection between results from server A and B:\n[].\n";
                    string client_reply = "Time intervals [] works for " + sA + ", " + sB;
                    if(send(clientfd, client_reply.c_str(), client_reply.length(), 0) == -1){
                        perror("send -2");
                        exit(1);
                    }
                    cout << "Main Server sent the result to the client.\n";
                    end_transmission = 1;
                }
            }
        }

        a_timeslot fromServerA;
        a_timeslot fromServerB;
        a_timeslot res;

        if(end_transmission != 1){
            if(sentA == 1 && sentB == 0){
                // just sent to serverA
                string a_timelist_string = string(a_timelist);
                cout << "Found the intersection between results from server A and B:\n" +a_timelist_string +".\n";
                string client_reply_a = "Time intervals " + a_timelist_string + " works for " + sA;
                if(send(clientfd, client_reply_a.c_str(), client_reply_a.length(), 0) == -1){
                    perror("send -3");
                    exit(1);
                }
                cout << "Main Server sent the result to the client.\n";
                end_transmission = 1;
            } else if(sentA == 0 && sentB == 1){
                string b_timelist_string = string(b_timelist);
                cout << "Found the intersection between results from server A and B:\n" +b_timelist_string +".\n";
                string client_reply_b = "Time intervals " + b_timelist_string + " works for " + sB;
                if(send(clientfd, client_reply_b.c_str(), client_reply_b.length(), 0) == -1){
                    perror("send -4");
                    exit(1);
                }
                cout << "Main Server sent the result to the client.\n";
                end_transmission = 1;
            } else {
                //cout << "here3" << "\n";
                createTimeSlots(&fromServerA, a_timelist);
                //cout << "here5" << "\n";
                createTimeSlots(&fromServerB, b_timelist);
                //cout << "here6" << "\n";
                //cout << fromServerA.count << " " << fromServerB.count << '\n';
                //findTimeSlots(&fromServerA, &res);
                //cout << res.count << " " << '\n';
                //findTimeSlots(&fromServerB, &res);
                findTimeSlots(&fromServerA, &fromServerB);
                //cout << "here7" << "\n";
                char reply_both[MAXBUFLEN];
                //convertTimeslotsToString(reply_both, &res);
                convertTimeslotsToString(reply_both, &fromServerB);
                string a_b_timelist = string(reply_both);
                cout << "Found the intersection between results from server A and B:\n" +a_b_timelist +".\n";
                string client_reply_a_b = "Time intervals " + a_b_timelist + " works for " + sA + ", " + sB;
                cout << client_reply_a_b << "\n";
                if(send(clientfd, client_reply_a_b.c_str(), client_reply_a_b.length(), 0) == -1){
                    perror("send -5");
                    exit(1);
                }
                cout << "Main Server sent the result to the client.\n";
                end_transmission = 1;
            }

        }

        if(end_transmission == 1){
            string end = "end";
            usleep(500);
            if(send(clientfd, end.c_str(), end.length(), 0) == -1){
                perror("send");
                exit(1);
            }
        }

    }
}


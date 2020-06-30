#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include <iostream>
#include <fstream>
#include <vector>

#define Healthcenter_TCP_PORT 6955 //6000 + 955 (USCID: 4057818955)
#define Num_of_Departments 3
#define BACKLOG 10	 // how many pending connections queue will hold
#define MAXDATASIZE 100 // max number of bytes we can get at once

using namespace std;

int tcp_fd;
struct sockaddr_in tcp_saddr, tcp_caddr;

void create_tcp_server(){
    char IPaddr[INET_ADDRSTRLEN];
    struct hostent *he;
    
    /* Create a socket */
    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_fd < 0) {
      perror("ERROR creating socket");
      exit(1);
   }
   
   memset(&tcp_saddr, '\0', sizeof(tcp_saddr)); // zero structure out
   tcp_saddr.sin_family = AF_INET; // match the socket() call
   tcp_saddr.sin_addr.s_addr = htonl(INADDR_ANY); // bind to any local address
   tcp_saddr.sin_port = htons(Healthcenter_TCP_PORT); // specify port to listen on

   if ((he = gethostbyname("viterbi-scf2.usc.edu")) == NULL) {
       perror("gethostbyname");
    }

   if (bind(tcp_fd, (struct sockaddr *) &tcp_saddr, sizeof(tcp_saddr)) < 0) { //bind
       perror("ERROR binding");
   }

   if(listen(tcp_fd, 5) < 0) { // listen for incoming connections
        perror("ERROR listening");
   }

   strcpy(IPaddr, inet_ntoa(*(struct in_addr*)he->h_addr));
   
   cout << "The health center has TCP port " << Healthcenter_TCP_PORT << " and IP address " << IPaddr << endl;
}

void saveData(string Hospital_information){
    ofstream fileout("./Healthcenter.txt");
    fileout << Hospital_information << endl;
    fileout.close();
}

void receive_from_hospitals() {
    int new_fd;
    char buf[MAXDATASIZE];
    int numbytes;

    socklen_t clilen = sizeof(tcp_fd);

    new_fd = accept(tcp_fd, (struct sockaddr *) &tcp_caddr, &clilen);
    if (new_fd < 0) {
        perror("ERROR creating socket");
        exit(1);
    }

    if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) > 0) {  
        saveData(string(buf));
        string hospital = string(buf).substr(0,9);
        cout << "Recieved the department list from <" << hospital << ">" << endl;
    }
    sleep(1);
    cout << "End of Phase I for the health center" << endl;
    close(tcp_fd);
    close(new_fd);
}

int main(void) {
    create_tcp_server();
    receive_from_hospitals();
    
    return 0;
}
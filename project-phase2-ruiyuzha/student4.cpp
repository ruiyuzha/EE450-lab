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
#include <string>

#define Healthcenter_TCP_PORT 6955 //6000 + 955 (USCID: 4057818955)
#define Student4_UDP_PORT 22655 //21700 + 955 (USCID: 4057818955)
#define MAXDATASIZE 100 // max number of bytes we can get at once

using namespace std;

int tcp_fd, udp_fd;
struct sockaddr_in tcp_saddr, tcp_caddr;
struct sockaddr_in udp_saddr, udp_caddr;

string readData(string filename){
    string data = "";
    string l;
    fstream infile(filename);
    if(infile.is_open()){
        while(infile >> l){
            data += l;
        }
    }
    return "student4" + data;
}

void TCP_send_student_information(){
    int numbyte;   
    char IPaddr[INET_ADDRSTRLEN];
    char buf[MAXDATASIZE];
       
    /* Create a socket */
    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_fd < 0) {
      perror("ERROR creating socket");
      exit(-1);
   }
   
   memset(&tcp_saddr, '\0', sizeof(tcp_saddr)); // zero structure out
   tcp_saddr.sin_family = AF_INET; // match the socket() call
   tcp_saddr.sin_addr.s_addr = htonl(INADDR_ANY); // bind to any local address
   tcp_saddr.sin_port = htons(Healthcenter_TCP_PORT); // specify port to listen on

   strcpy(IPaddr, inet_ntoa(tcp_saddr.sin_addr));
   
   if (connect(tcp_fd, (struct sockaddr *) &tcp_saddr, sizeof(tcp_saddr)) < 0) { //connecting to server
		perror("Connect server error");
		close(tcp_fd);
		exit(-1);
	}
    
    bzero(&tcp_caddr, sizeof(tcp_caddr));
    socklen_t len = sizeof(tcp_caddr);
    getsockname(tcp_fd, (struct sockaddr *) &tcp_caddr, &len);
    inet_ntop(AF_INET, &tcp_caddr.sin_addr, IPaddr, sizeof(IPaddr));
    int PortS4 = ntohs(tcp_caddr.sin_port);

    cout << "<Student 4> has TCP port " << PortS4 << " and IP address " << IPaddr << endl;

    string student4_information = readData("student4.txt");

    strcpy(buf, student4_information.c_str());
    if ((numbyte = send(tcp_fd, buf, MAXDATASIZE-1, 0)) > 0) {
        sleep(1);
    }

    cout << "Completed sending application for <" <<  student4_information.substr(0,8) << ">" << endl;
    sleep(1);
}

void UDP_recieve_application_result(){
    char IPaddr[INET_ADDRSTRLEN];
    struct hostent *he;

    /* Create a socket */
    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd < 0) {
      perror("ERROR creating socket");
      exit(1);
   }
   
   memset(&udp_saddr, '\0', sizeof(udp_saddr)); // zero structure out
   memset(&udp_caddr, '\0', sizeof(udp_caddr)); // zero structure out
   udp_saddr.sin_family = AF_INET; // match the socket() call
   udp_saddr.sin_addr.s_addr = htonl(INADDR_ANY); // bind to any local address
   udp_saddr.sin_port = htons(Student4_UDP_PORT); // specify port to listen on

   if ((he = gethostbyname("www.google.com")) == NULL) {
       perror("gethostbyname");
    }

   if (bind(udp_fd, (struct sockaddr *) &udp_saddr, sizeof(udp_saddr)) < 0) { //bind
       perror("ERROR binding");
   }
   strcpy(IPaddr, inet_ntoa(*(struct in_addr*)he->h_addr));
   
   cout << "<Student4> has UDP port " << Student4_UDP_PORT << " and IP address " << IPaddr << endl;

   char buf[MAXDATASIZE];

   socklen_t clilen = sizeof(udp_fd);
   recvfrom(udp_fd, buf, MAXDATASIZE, 0, (struct sockaddr *) &udp_caddr, &clilen);
   cout << "<Student4> has received the application result, the result is:" << buf << endl;
   sleep(1); 
   close(udp_fd);
}

int main(void){
    TCP_send_student_information();
    UDP_recieve_application_result();
}
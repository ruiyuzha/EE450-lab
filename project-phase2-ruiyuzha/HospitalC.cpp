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
#include <sstream>
#include <vector>

#define Healthcenter_TCP_PORT 6955 //6000 + 955 (USCID: 4057818955)
#define Hospital3_UDP_PORT 22255 //21300 + 955 (USCID: 4057818955)
#define MAXDATASIZE 100 // max number of bytes we can get at once
#define Num_of_Departments 3

using namespace std;

int tcp_fd, udp_fd;
struct sockaddr_in tcp_saddr, tcp_caddr;
struct sockaddr_in udp_saddr, udp_caddr;

vector<string> readData(string filename){
    vector<string> data;
    string l;
    ifstream infile(filename);
    if(infile.is_open()){
        while(infile >> l){
            data.push_back(l);
        }
    }
    return data;
}

void TCP_send_hospital_information(){
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
    int PortHC = ntohs(tcp_caddr.sin_port);

    cout << "<Hospital C> has TCP port " << PortHC << " and IP address " << IPaddr << " for Phase 1" << endl;

    cout << "<Hospital C> is now connected to the admission office" << endl;

    vector<string> HospitalC_information = readData("HospitalC.txt");

    string str = "HospitalC";
    strcpy(buf, str.c_str());
    if ((numbyte = send(tcp_fd, buf, MAXDATASIZE-1, 0)) > 0) {
        sleep(1);
    } 

    for (int i = 0; i < Num_of_Departments; i++){
        strcpy(buf, HospitalC_information[i].c_str());   
        if ((numbyte = send(tcp_fd, buf, MAXDATASIZE-1, 0)) > 0) {      
            cout << "<Hospital C> has sent <" << buf << "> to the agent" << endl;
            sleep(1);
        }
    }

    cout << "Updating the health center is done for <HospitalC>" << endl;  
    cout << "End of Phase 1 for <HospitalC>" << endl; 
}

void UDP_recieve_admitted_result(){
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
   udp_saddr.sin_port = htons(Hospital3_UDP_PORT); // specify port to listen on

   if ((he = gethostbyname("www.google.com")) == NULL) {
       perror("gethostbyname");
    }

   if (bind(udp_fd, (struct sockaddr *) &udp_saddr, sizeof(udp_saddr)) < 0) { //bind
       perror("ERROR binding");
   }
   strcpy(IPaddr, inet_ntoa(*(struct in_addr*)he->h_addr));
   
   cout << "<HospitalC> has UDP port " << Hospital3_UDP_PORT << " and IP address " << IPaddr << " for Phase 2" << endl;

   char buf[MAXDATASIZE];

   socklen_t clilen = sizeof(udp_fd);
   recvfrom(udp_fd, buf, MAXDATASIZE, 0, (struct sockaddr *) &udp_caddr, &clilen);
   cout << string(buf).substr(0,8) << "has addmitted to HospitalC" << endl;
   sleep(1);
}


int main(void) {
    TCP_send_hospital_information();
    UDP_recieve_admitted_result();  
}
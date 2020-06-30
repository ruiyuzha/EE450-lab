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
#define MAXDATASIZE 100 // max number of bytes we can get at once

using namespace std;

string readData(string filename){
    string data = "";
    string l;
    fstream infile(filename);
    if(infile.is_open()){
        while(infile >> l){
            data += l;
        }
    }
    return "student1" + data;
}

void TCP_send_student_information(){
    int tcp_fd, numbyte;
    struct sockaddr_in tcp_saddr, tcp_caddr;
    
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
    int PortS1 = ntohs(tcp_caddr.sin_port);

    cout << "<Student 1> has TCP port " << PortS1 << " and IP address " << IPaddr << endl;

    string student1_information = readData("student1.txt");

    strcpy(buf, student1_information.c_str());
    if ((numbyte = send(tcp_fd, buf, MAXDATASIZE-1, 0)) > 0) {
        sleep(1);
    }

    cout << "Completed sending application for <" <<  student1_information.substr(0,8) << ">" << endl;
}

int main(void){
    TCP_send_student_information();
}
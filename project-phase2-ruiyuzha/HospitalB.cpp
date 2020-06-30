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
#define MAXDATASIZE 100 // max number of bytes we can get at once
#define Num_of_Departments 3

using namespace std;

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

int main(void) {
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
    int PortHB = ntohs(tcp_caddr.sin_port);

    cout << "<Hospital B> has TCP port " << PortHB << " and IP address " << IPaddr << " for Phase 1" << endl;

    cout << "<Hospital B> is now connected to the admission office" << endl;

    vector<string> HospitalB_information = readData("HospitalB.txt");

    string str = "HospitalB";
    strcpy(buf, str.c_str());
    if ((numbyte = send(tcp_fd, buf, MAXDATASIZE-1, 0)) > 0) {
        sleep(1);
    } 

    for (int i = 0; i < Num_of_Departments; i++){
        strcpy(buf, HospitalB_information[i].c_str());   
        if ((numbyte = send(tcp_fd, buf, MAXDATASIZE-1, 0)) > 0) {      
            cout << "<Hospital B> has sent <" << buf << "> to the agent" << endl;
            sleep(1);
        }
    }
    cout << "Updating the health center is done for <HospitalB>" << endl;  
    cout << "End of Phase 1 for <HospitalB>" << endl; 
}
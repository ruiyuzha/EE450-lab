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

#include <signal.h>
#include <string>
#include <iostream>

#define PORT "4955"  // 4000 + 955 (uscid:4057818955)
#define BACKLOG 10	 // how many pending connections queue will hold
#define MAXDATASIZE 100 // max number of bytes we can get at once 

using namespace std;

void sigchld_handler(int s) {
	while(waitpid(-1, NULL, WNOHANG) > 0);
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Randomly generate a transaction ID (a 8-bit number)
string randTransID () {
	srand(time(NULL));

    int num = rand()%256;
    return to_string(num);
}

// Randomly generate a IPv4 address (the lowest 8-bit is the discovery phase transaction ID)
string randomAddr (string transID) {
	srand(time(NULL));
    string addr = "";
    int curr = 0;

    for (int i = 0; i < 3; i++) {
        curr = rand()%256; // the value between "." is 0-255
        addr = addr + to_string(curr) + ".";
    }

    return addr + transID;
}

int main(void) {
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	int numbytes;
	char buf[1024];


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

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo); // all done with this structure


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
		

		if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) > 0) {
		    cout << "Recieved the following Transaction ID from client: " << buf << endl;
		    sleep(3);    
		}

		//Offer phase
		string IPaddr;
		string TranID;
		string recvID = string(buf);
		IPaddr = randomAddr(recvID);
		TranID = randTransID();
		string msg = IPaddr + "#" + TranID;
	   	strcpy(buf, msg.c_str());
		if ((numbytes = send(new_fd, buf, MAXDATASIZE-1, 0)) > 0) {
			cout << "Sending the following to Client: " << endl;			
			cout << "IP address: " << IPaddr << endl;
			cout << "Transaction ID: " << TranID << endl;
		}		

		if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) > 0) {
			cout << "Received the following request: " << endl;
			cout << "Transaction ID: " << buf << endl;
			sleep(3);
		}

		//Acknowledge phase
		string TranIDack;
	    TranIDack = randTransID();
		strcpy(buf, TranIDack.c_str());
	    if ((numbytes = send(new_fd, buf, MAXDATASIZE-1, 0)) > 0) {
	    	cout << "Sending following acknowledgment: " << endl;
			cout << "IP address: " << IPaddr << endl; 
			cout << "Transaction ID: " << buf << endl;
		}
		close(new_fd);
		exit(0);		
	}
	close(sockfd);

	return 0;
}

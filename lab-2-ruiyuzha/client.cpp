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

#include <string>
#include <iostream>

#define PORT "4955"  // 4000 + 955 (uscid:4057818955)
#define MAXDATASIZE 100 // max number of bytes we can get at once 

using namespace std;
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


int main(int argc, char *argv[]) {
	int sockfd, numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
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

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	//Discovery phase
	string id = to_string(190); // 955/255 = 3*255 + 190 (uscid:4057818955)
    strcpy(buf, id.c_str());    
    if ((numbytes = send(sockfd, buf, MAXDATASIZE-1, 0)) > 0) {    	
        cout << "Sending the following Transaction ID to server: " << buf << endl;
    }

    string recvAddr;
    string recvID;
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) > 0) {
    	string msg = string(buf);
    	int index = msg.find('#');
    	recvAddr = msg.substr(0, index);
    	recvID = msg.substr(index+1);
		cout << "Recived the following: " << endl;
    	cout << "IP address: " << recvAddr << endl;
		cout << "Transaction ID: " << recvID << endl;
  	    sleep(3);
    }

    //Request phase
	string TranID;
    TranID = randTransID();
    strcpy(buf, TranID.c_str());
    if ((numbytes = send(sockfd, buf, MAXDATASIZE-1, 0)) > 0) {
		cout << "Formally requesting the following server: " << endl;
        cout << "IP address: " << recvAddr << endl;
		cout << "Transaction ID: " << buf << endl;
    }

    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) > 0) {
	    cout << "Officially connected to IP Address: " << recvAddr << endl;
		printf("client: received '%s'\n",buf);
	}

	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
	    perror("recv");
	    exit(1);
	}

	buf[numbytes] = '\0';
	close(sockfd);

	return 0;
}

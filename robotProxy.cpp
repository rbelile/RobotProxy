#include <string>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>

#define MAXSTRINGLENGTH 300
using namespace std;

void DieWithError(string errorMessage){ // Error handling function
    fprintf(stdout, "%s\n", errorMessage.c_str());
    exit(1);
}

void UdpServer(char *port){
	// Construct the server address structure
  	struct addrinfo addrCriteria;                   // Criteria for address
  	memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
  	addrCriteria.ai_family = AF_UNSPEC;             // Any address family
  	addrCriteria.ai_flags = AI_PASSIVE;             // Accept on any address/port
  	addrCriteria.ai_socktype = SOCK_DGRAM;          // Only datagram socket
  	addrCriteria.ai_protocol = IPPROTO_UDP;         // Only UDP socket

  	struct addrinfo *servAddr; // List of server addresses
  	int rtnVal = getaddrinfo(NULL, port, &addrCriteria, &servAddr);
  	if (rtnVal != 0)
    	DieWithError("getaddrinfo() failed");

  	// Create socket for incoming connections
  	int sock = socket(servAddr->ai_family, servAddr->ai_socktype,
      	servAddr->ai_protocol);
  	if (sock < 0)
    	DieWithError("socket() failed");

  	// Bind to the local address
  	if (bind(sock, servAddr->ai_addr, servAddr->ai_addrlen) < 0)
    	DieWithError("bind() failed");

  	// Free address list allocated by getaddrinfo()
  	freeaddrinfo(servAddr);
  	for (;;) { // Run forever
    	struct sockaddr_storage clntAddr; // Client address
    	// Set Length of client address structure (in-out parameter)
    	socklen_t clntAddrLen = sizeof(clntAddr);

    	// Block until receive message from a client
    	char buffer[MAXSTRINGLENGTH]; // I/O buffer
    	// Size of received message
    	ssize_t numBytesRcvd = recvfrom(sock, buffer, MAXSTRINGLENGTH, 0,
        	(struct sockaddr *) &clntAddr, &clntAddrLen);
    	if (numBytesRcvd < 0)
      		DieWithError("recvfrom() failed");

        cout << "here";

    	// Send received datagram back to the client
    	ssize_t numBytesSent = sendto(sock, buffer, numBytesRcvd, 0,
        	(struct sockaddr *) &clntAddr, sizeof(clntAddr));

    	if (numBytesSent < 0)
      		DieWithError("sendto() failed");
    	else if (numBytesSent != numBytesRcvd)
      		DieWithError("sendto() sent unexpected number of bytes");
    }
}

void TcpClient(string Server, int robotID, int robotNum, char* port){
    int sock;                        /* Socket descriptor */
    struct sockaddr_in ServAddr; /*  server address */
    string Buffer;       
    //unsigned int StringLen;      /* Length of string to  */
    //int bytesRcvd, totalBytesRcvd;   /* Bytes read in single recv() 
    //and total bytes read */
    struct hostent *host;

    /* Create a reliable, stream socket using TCP */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    /* Construct the server address structure */
    memset(&ServAddr, 0, sizeof(ServAddr));     /* Zero out structure */
    ServAddr.sin_family      = AF_INET;             /* Internet address family */
    ServAddr.sin_addr.s_addr = inet_addr(Server.c_str();   /* Server IP address */
    ServAddr.sin_port        = htons(ServPort); /* Server port */


    if(ServAddr.sin_addr.s_addr == -1)
    {
        host = gethostbyname(Server.c_str());
            ServAddr.sin_addr.s_addr = *((unsigned long *) host->h_addr_list[0]);
    }

    /* Establish the connection to the  server */
    if (connect(sock, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) < 0)
        DieWithError("connect() failed");
    
    close(sock);
    exit(0);
}

int main(int argc, char *argv[]) {
	string hostName;
	int robotID, robotNum;
	in_port_t port;
	char* UdpServerPort;

	if (argc != 9) {
		cout << "Usage: " << argv[0] << " -h <hostname-of-robot> -i <robot-id> -n " <<
			"<robot-number> -p <port>" << endl;
		exit(1);
	}

	for (int i = 0; i < 9; i++) {
		string argvStr = argv[i];
		if (argvStr.compare("-h") == 0) {
			hostName = argv[i+1];
		} else if (argvStr.compare("-i") == 0) {
			robotID = atoi(argv[i+1]);
		} else if (argvStr.compare("-n") == 0) {
			robotNum = atoi(argv[i+1]);
		} else if (argvStr.compare("-p") == 0) {
			port = atoi(argv[i+1]);
			UdpServerPort = argv[i+1];
		}
	}

	UdpServer(UdpServerPort);
}
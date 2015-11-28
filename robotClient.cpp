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
#include <unistd.h>

#define MAXSTRINGLENGTH 300
using namespace std;

void DieWithError(string errorMessage){ // Error handling function
    fprintf(stdout, "%s\n", errorMessage.c_str());
    exit(1);
}


void UdpClient(string server, char* servPort){

  	
  	string echoString;
  	cout << "Please enter an echo message: " << endl;
  	getline(cin, echoString);

  	size_t echoStringLen = echoString.length();
  	if (echoStringLen > 300) // Check input length
    	DieWithError("string too long");

  	// Tell the system what kind(s) of address info we want
  	struct addrinfo addrCriteria;                   // Criteria for address match
  	memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
  	addrCriteria.ai_family = AF_UNSPEC;             // Any address family
  	addrCriteria.ai_socktype = SOCK_DGRAM;          // Only datagram sockets
  	addrCriteria.ai_protocol = IPPROTO_UDP;         // Only UDP protocol

  	// Get address(es)
  	struct addrinfo *servAddr; // List of server addresses
  	int rtnVal = getaddrinfo(server.c_str(), servPort, &addrCriteria, &servAddr);
  	if (rtnVal != 0)
    	DieWithError("getaddrinfo() failed");

  	// Create a datagram/UDP socket
  	int sock = socket(servAddr->ai_family, servAddr->ai_socktype,
      	servAddr->ai_protocol); // Socket descriptor for client
  	if (sock < 0)
    	DieWithError("socket() failed");

  	// Send the string to the server
  	ssize_t numBytes = sendto(sock, echoString.c_str(), echoStringLen, 0,
      	servAddr->ai_addr, servAddr->ai_addrlen);
  	if (numBytes < 0)
    	DieWithError("sendto() failed");
  	else if (numBytes != echoStringLen)
    	DieWithError("sendto() error, sent unexpected number of bytes");

  	// Receive a response

  	struct sockaddr_storage fromAddr; // Source address of server
  	// Set length of from address structure (in-out parameter)
  	socklen_t fromAddrLen = sizeof(fromAddr);

  	char buffer[MAXSTRINGLENGTH + 1]; // I/O buffer

  	numBytes = recvfrom(sock, buffer, MAXSTRINGLENGTH, 0,
      	(struct sockaddr *) &fromAddr, &fromAddrLen);

  	if (numBytes < 0)
    	DieWithError("recvfrom() failed");
  	else if (numBytes != echoStringLen)
    	DieWithError("recvfrom() error, received unexpected number of bytes");

  	freeaddrinfo(servAddr);

  	buffer[echoStringLen] = '\0';     // Null-terminate received data
  	printf("Received: %s\n", buffer); // Print the echoed string

  	close(sock);
  	exit(0);
}


int main(int argc, char *argv[]) {
	string serverName;
	in_port_t serverPort;
	char *UdpClientPort;
	int lenOfSides, numOfSides;

	if (argc != 9) {
		cout << "Usage: " << argv[0] << " -h <hostname-of-server> -p <port> " <<
			"-n <number-of-sides> -l <length-of-sides>" << endl;
		exit(1);
	}

	for (int i = 0; i < 9; i++) {
		string argvStr = argv[i];
		if (argvStr.compare("-h") == 0) {
			serverName = argv[i+1];
		} else if (argvStr.compare("-l") == 0) {
			lenOfSides = atoi(argv[i+1]);
		} else if (argvStr.compare("-n") == 0) {
			numOfSides = atoi(argv[i+1]);
		} else if (argvStr.compare("-p") == 0) {
			serverPort = atoi(argv[i+1]);
			UdpClientPort = argv[i+1];
		}
	}

	UdpClient(serverName, UdpClientPort);
}
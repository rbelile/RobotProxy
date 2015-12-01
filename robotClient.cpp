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
#include <sstream>
#include <iomanip>

#define MAXDATAGRAM 300
#define MAXGROUPDATAGRAM 600
using namespace std;

void DieWithError(string errorMessage){ // Error handling function
    fprintf(stdout, "%s\n", errorMessage.c_str());
    exit(1);
}

string getDate() {
    time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );
    int year = now->tm_year + 1900;
    int month = now->tm_mon + 1;
    int day = now->tm_mday;
    int hour = now->tm_hour;
    int min = now->tm_min; 
    int sec = now->tm_sec;

    stringstream ss;

    ss << "" << month << "/" << day << "/" << year << " " << hour << ":"
        << setfill('0') << setw(2) << min << ":" << setfill('0') << setw(2) << sec;
    return ss.str();
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

	int udpHeaders[7] = { 0, 0, 0, 0, 1, 0, 0 }; // To connect to proxy
	/*cout << "Headers: ";
	for(int i = 0; i < 7; i++) {
		cout << udpHeaders[i] << " ";
	}
	cout << endl;*/

  	// Tell the system what kind(s) of address info we want
  	struct addrinfo addrCriteria;                   // Criteria for address match
  	memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
  	addrCriteria.ai_family = AF_UNSPEC;             // Any address family
  	addrCriteria.ai_socktype = SOCK_DGRAM;          // Only datagram sockets
  	addrCriteria.ai_protocol = IPPROTO_UDP;         // Only UDP protocol

  	// Get address(es)
  	struct addrinfo *servAddr; // List of server addresses
  	int rtnVal = getaddrinfo(serverName.c_str(), UdpClientPort, &addrCriteria, &servAddr);
  	if (rtnVal != 0)
    	DieWithError("getaddrinfo() failed");

  	// Create a datagram/UDP socket
  	int sock = socket(servAddr->ai_family, servAddr->ai_socktype,
      	servAddr->ai_protocol); // Socket descriptor for client
  	if (sock < 0)
    	DieWithError("socket() failed");

  	// Send the string to the server
  	ssize_t numBytes = sendto(sock, udpHeaders, sizeof(udpHeaders), 0,
      	servAddr->ai_addr, servAddr->ai_addrlen);
  	if (numBytes < 0)
    	DieWithError("sendto() failed");
  	else if (numBytes != sizeof(udpHeaders))
    	DieWithError("sendto() error, sent unexpected number of bytes");

  	// Receive a response

  	struct sockaddr_storage fromAddr; // Source address of server
  	// Set length of from address structure (in-out parameter)
  	socklen_t fromAddrLen = sizeof(fromAddr);

  	int buffer[7]; // I/O buffer

  	ssize_t numBytesRcvd = recvfrom(sock, buffer, MAXDATAGRAM, 0,
      	(struct sockaddr *) &fromAddr, &fromAddrLen);

  	if (numBytesRcvd < 0)
    	DieWithError("recvfrom() failed");

	int password = buffer[1];

	udpHeaders[1] = password;

  	numBytes = sendto(sock, udpHeaders, sizeof(udpHeaders), 0,
      	servAddr->ai_addr, servAddr->ai_addrlen);
  	if (numBytes < 0)
    	DieWithError("sendto() failed");
  	else if (numBytes != sizeof(udpHeaders))
    	DieWithError("sendto() error, sent unexpected number of bytes");

	bool isOnSecond = false;

repeat:
	for(int i = 0; i < numOfSides; i++) {
		// Image
		udpHeaders[2] = 2;
		numBytes = sendto(sock, udpHeaders, sizeof(udpHeaders), 0,
      	servAddr->ai_addr, servAddr->ai_addrlen);
  		if (numBytes < 0)
    		DieWithError("sendto() failed");
  		else if (numBytes != sizeof(udpHeaders))
    		DieWithError("sendto() error, sent unexpected number of bytes");

		numBytesRcvd = recvfrom(sock, buffer, MAXDATAGRAM, 0,
      	(struct sockaddr *) &fromAddr, &fromAddrLen);
		if (buffer[2] != 2) {
			DieWithError("Server did not acknowledge image request.");
		}

		ofstream imageFile;
		string fileName = getDate() + " Camera Data.jpeg";
		imageFile.open(fileName.c_str());

		char fullBuffer[MAXDATAGRAM];
		numBytesRcvd = recvfrom(sock, fullBuffer, MAXDATAGRAM, 0,
      	(struct sockaddr *) &fromAddr, &fromAddrLen);
		string tempStr = "" + fullBuffer[20] + fullBuffer[21] + fullBuffer[22] 
			+ fullBuffer[23];
		int numOfDatagrams = atoi(tempStr.c_str());
		for(int j = 28; j < 300; j++) {
			imageFile << fullBuffer[j];
		}

		for(int j = 2; j <= numOfDatagrams; j++) {
			numBytesRcvd = recvfrom(sock, fullBuffer, MAXDATAGRAM, 0,
		   	(struct sockaddr *) &fromAddr, &fromAddrLen);
			for(int j = 28; j < 300; j++) {
				imageFile << fullBuffer[j];
			}
		}
		imageFile.close();

		// GPS
		udpHeaders[2] = 4;
		numBytes = sendto(sock, udpHeaders, sizeof(udpHeaders), 0,
      	servAddr->ai_addr, servAddr->ai_addrlen);
  		if (numBytes < 0)
    		DieWithError("sendto() failed");
  		else if (numBytes != sizeof(udpHeaders))
    		DieWithError("sendto() error, sent unexpected number of bytes");

		numBytesRcvd = recvfrom(sock, buffer, MAXDATAGRAM, 0,
      	(struct sockaddr *) &fromAddr, &fromAddrLen);
		if (buffer[2] != 4) {
			DieWithError("Server did not acknowledge image request.");
		}

		ofstream gpsFile;
		fileName = getDate() + " GPS Data.jpeg";
		gpsFile.open(fileName.c_str());
		
		numBytesRcvd = recvfrom(sock, fullBuffer, MAXDATAGRAM, 0,
      	(struct sockaddr *) &fromAddr, &fromAddrLen);
		for(int j = 28; j < 300; j++) {
			gpsFile << fullBuffer[j];
		}
		gpsFile.close();

		// dGPS
		udpHeaders[2] = 8;
		numBytes = sendto(sock, udpHeaders, sizeof(udpHeaders), 0,
      	servAddr->ai_addr, servAddr->ai_addrlen);
  		if (numBytes < 0)
    		DieWithError("sendto() failed");
  		else if (numBytes != sizeof(udpHeaders))
    		DieWithError("sendto() error, sent unexpected number of bytes");

		numBytesRcvd = recvfrom(sock, buffer, MAXDATAGRAM, 0,
      	(struct sockaddr *) &fromAddr, &fromAddrLen);
		if (buffer[2] != 8) {
			DieWithError("Server did not acknowledge image request.");
		}

		ofstream dgpsFile;
		fileName = getDate() + " dGPS Data.jpeg";
		dgpsFile.open(fileName.c_str());
		
		numBytesRcvd = recvfrom(sock, fullBuffer, MAXDATAGRAM, 0,
      	(struct sockaddr *) &fromAddr, &fromAddrLen);
		for(int j = 28; j < 300; j++) {
			dgpsFile << fullBuffer[j];
		}
		dgpsFile.close();

		// Laser
		udpHeaders[2] = 16;
		numBytes = sendto(sock, udpHeaders, sizeof(udpHeaders), 0,
      	servAddr->ai_addr, servAddr->ai_addrlen);
  		if (numBytes < 0)
    		DieWithError("sendto() failed");
  		else if (numBytes != sizeof(udpHeaders))
    		DieWithError("sendto() error, sent unexpected number of bytes");

		numBytesRcvd = recvfrom(sock, buffer, MAXDATAGRAM, 0,
      	(struct sockaddr *) &fromAddr, &fromAddrLen);
		if (buffer[2] != 16) {
			DieWithError("Server did not acknowledge image request.");
		}

		ofstream laserFile;
		fileName = getDate() + " Laser Data.jpeg";
		laserFile.open(fileName.c_str());

		// reset fullbuffer?
		numBytesRcvd = recvfrom(sock, fullBuffer, MAXDATAGRAM, 0,
      	(struct sockaddr *) &fromAddr, &fromAddrLen);
		tempStr = "" + fullBuffer[20] + fullBuffer[21] + fullBuffer[22] 
			+ fullBuffer[23];
		numOfDatagrams = atoi(tempStr.c_str());
		for(int j = 28; j < 300; j++) {
			laserFile << fullBuffer[j];
		}

		for(int j = 2; j <= numOfDatagrams; j++) {
			numBytesRcvd = recvfrom(sock, fullBuffer, MAXDATAGRAM, 0,
		   	(struct sockaddr *) &fromAddr, &fromAddrLen);
			for(int j = 28; j < 300; j++) {
				laserFile << fullBuffer[j];
			}
		}
		laserFile.close();

		// Move
		udpHeaders[2] = 32;
		udpHeaders[3] = lenOfSides;
		numBytes = sendto(sock, udpHeaders, sizeof(udpHeaders), 0,
      	servAddr->ai_addr, servAddr->ai_addrlen);
  		if (numBytes < 0)
    		DieWithError("sendto() failed");
  		else if (numBytes != sizeof(udpHeaders))
    		DieWithError("sendto() error, sent unexpected number of bytes");

		numBytesRcvd = recvfrom(sock, buffer, MAXDATAGRAM, 0,
      	(struct sockaddr *) &fromAddr, &fromAddrLen);
		if (buffer[2] != 32) {
			DieWithError("Server did not acknowledge move request.");
		}

		// Stop
		udpHeaders[2] = 128;
		numBytes = sendto(sock, udpHeaders, sizeof(udpHeaders), 0,
      	servAddr->ai_addr, servAddr->ai_addrlen);
  		if (numBytes < 0)
    		DieWithError("sendto() failed");
  		else if (numBytes != sizeof(udpHeaders))
    		DieWithError("sendto() error, sent unexpected number of bytes");

		numBytesRcvd = recvfrom(sock, buffer, MAXDATAGRAM, 0,
      	(struct sockaddr *) &fromAddr, &fromAddrLen);
		if (buffer[2] != 128) {
			DieWithError("Server did not acknowledge stop request.");
		}

		// Turn
		int angle = 180 * (numOfSides - 2) / numOfSides;
		int speed = 1.4 * angle / 360;
		udpHeaders[2] = 64;
		udpHeaders[3] = speed;
		numBytes = sendto(sock, udpHeaders, sizeof(udpHeaders), 0,
      	servAddr->ai_addr, servAddr->ai_addrlen);
  		if (numBytes < 0)
    		DieWithError("sendto() failed");
  		else if (numBytes != sizeof(udpHeaders))
    		DieWithError("sendto() error, sent unexpected number of bytes");

		numBytesRcvd = recvfrom(sock, buffer, MAXDATAGRAM, 0,
      	(struct sockaddr *) &fromAddr, &fromAddrLen);
		if (buffer[2] != 64) {
			DieWithError("Server did not acknowledge turn request.");
		}

		// Stop
		udpHeaders[2] = 128;
		numBytes = sendto(sock, udpHeaders, sizeof(udpHeaders), 0,
      	servAddr->ai_addr, servAddr->ai_addrlen);
  		if (numBytes < 0)
    		DieWithError("sendto() failed");
  		else if (numBytes != sizeof(udpHeaders))
    		DieWithError("sendto() error, sent unexpected number of bytes");

		numBytesRcvd = recvfrom(sock, buffer, MAXDATAGRAM, 0,
      	(struct sockaddr *) &fromAddr, &fromAddrLen);
		if (buffer[2] != 128) {
			DieWithError("Server did not acknowledge stop request.");
		}
	}

	// Image
	udpHeaders[2] = 2;
	numBytes = sendto(sock, udpHeaders, sizeof(udpHeaders), 0,
   	servAddr->ai_addr, servAddr->ai_addrlen);
	if (numBytes < 0)
 		DieWithError("sendto() failed");
	else if (numBytes != sizeof(udpHeaders))
 		DieWithError("sendto() error, sent unexpected number of bytes");

	numBytesRcvd = recvfrom(sock, buffer, MAXDATAGRAM, 0,
   	(struct sockaddr *) &fromAddr, &fromAddrLen);
	if (buffer[2] != 2) {
		DieWithError("Server did not acknowledge image request.");
	}

	ofstream imageFile;
	string fileName = getDate() + " Camera Data.jpeg";
	imageFile.open(fileName.c_str());

	char fullBuffer[MAXDATAGRAM];
	numBytesRcvd = recvfrom(sock, fullBuffer, MAXDATAGRAM, 0,
   	(struct sockaddr *) &fromAddr, &fromAddrLen);
	string tempStr = "" + fullBuffer[20] + fullBuffer[21] + fullBuffer[22] 
		+ fullBuffer[23];
	int numOfDatagrams = atoi(tempStr.c_str());
	for(int j = 28; j < 300; j++) {
		imageFile << fullBuffer[j];
	}

	for(int j = 2; j <= numOfDatagrams; j++) {
		numBytesRcvd = recvfrom(sock, fullBuffer, MAXDATAGRAM, 0,
	   	(struct sockaddr *) &fromAddr, &fromAddrLen);
		for(int j = 28; j < 300; j++) {
			imageFile << fullBuffer[j];
		}
	}
	imageFile.close();

	// GPS
	udpHeaders[2] = 4;
	numBytes = sendto(sock, udpHeaders, sizeof(udpHeaders), 0,
   	servAddr->ai_addr, servAddr->ai_addrlen);
	if (numBytes < 0)
 		DieWithError("sendto() failed");
	else if (numBytes != sizeof(udpHeaders))
 		DieWithError("sendto() error, sent unexpected number of bytes");

	numBytesRcvd = recvfrom(sock, buffer, MAXDATAGRAM, 0,
   	(struct sockaddr *) &fromAddr, &fromAddrLen);
	if (buffer[2] != 4) {
		DieWithError("Server did not acknowledge image request.");
	}

	ofstream gpsFile;
	fileName = getDate() + " GPS Data.jpeg";
	gpsFile.open(fileName.c_str());
	
	numBytesRcvd = recvfrom(sock, fullBuffer, MAXDATAGRAM, 0,
   	(struct sockaddr *) &fromAddr, &fromAddrLen);
	for(int j = 28; j < 300; j++) {
		gpsFile << fullBuffer[j];
	}
	gpsFile.close();

	// dGPS
	udpHeaders[2] = 8;
	numBytes = sendto(sock, udpHeaders, sizeof(udpHeaders), 0,
   	servAddr->ai_addr, servAddr->ai_addrlen);
	if (numBytes < 0)
 		DieWithError("sendto() failed");
	else if (numBytes != sizeof(udpHeaders))
 		DieWithError("sendto() error, sent unexpected number of bytes");

	numBytesRcvd = recvfrom(sock, buffer, MAXDATAGRAM, 0,
   	(struct sockaddr *) &fromAddr, &fromAddrLen);
	if (buffer[2] != 8) {
		DieWithError("Server did not acknowledge image request.");
	}

	ofstream dgpsFile;
	fileName = getDate() + " dGPS Data.jpeg";
	dgpsFile.open(fileName.c_str());
	
	numBytesRcvd = recvfrom(sock, fullBuffer, MAXDATAGRAM, 0,
   	(struct sockaddr *) &fromAddr, &fromAddrLen);
	for(int j = 28; j < 300; j++) {
		dgpsFile << fullBuffer[j];
	}
	dgpsFile.close();

	// Laser
	udpHeaders[2] = 16;
	numBytes = sendto(sock, udpHeaders, sizeof(udpHeaders), 0,
   	servAddr->ai_addr, servAddr->ai_addrlen);
	if (numBytes < 0)
 		DieWithError("sendto() failed");
	else if (numBytes != sizeof(udpHeaders))
 		DieWithError("sendto() error, sent unexpected number of bytes");

	numBytesRcvd = recvfrom(sock, buffer, MAXDATAGRAM, 0,
   	(struct sockaddr *) &fromAddr, &fromAddrLen);
	if (buffer[2] != 16) {
		DieWithError("Server did not acknowledge image request.");
	}

	ofstream laserFile;
	fileName = getDate() + " Laser Data.jpeg";
	laserFile.open(fileName.c_str());

	//char fullBuffer[MAXDATAGRAM];
	numBytesRcvd = recvfrom(sock, fullBuffer, MAXDATAGRAM, 0,
   		(struct sockaddr *) &fromAddr, &fromAddrLen);
	tempStr = "" + fullBuffer[20] + fullBuffer[21] + fullBuffer[22] 
		+ fullBuffer[23];
	numOfDatagrams = atoi(tempStr.c_str());
	for(int j = 28; j < 300; j++) {
		laserFile << fullBuffer[j];
	}
	
	laserFile.close();

	if (!isOnSecond) {
		isOnSecond = true;
		numOfSides = numOfSides - 1;
		goto repeat;
	}

	freeaddrinfo(servAddr);

  	close(sock);
  	exit(0);
}

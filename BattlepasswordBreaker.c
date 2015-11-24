/*
	passwordBreaker.c
	Daniel Battle - Assignment 1
	This file is the client that tries to crack the password on the server. Routines:
	main
	selectNextPassword
	validASCIINum
	getTime
	intHandler
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>
#include <signal.h>
#include "Practical.h"

char* selectNextPassword(int numChars);
bool validASCIINum(int num);
double getTime();
void intHandler();

//These are here so they can be printed once the program is terminated by Control + C.
double initialTime;
int attemptCount;

/*
Parameters: int argc, char *argv[]. Argc is the number of command line arguments, while
argv[] is an array that contains the command line arguments.

This is the main function for the client. It creates a socket, then creates random passwords
of a certain length and sends them to a server to see if it got the password correct.
Once the password is guessed correctly, the program terminates and prints out statistics.
*/
int main(int argc, char *argv[]) {
	int sock;  //Socket descriptor
	int numInPass; //Number of characters in the password
	int returnNum;
	char *serverPort;
	char *serverIP;
	char *password;

	attemptCount = 0;
	initialTime = getTime();
	srand(time(NULL));
	signal(SIGINT, intHandler);

	if(argc != 4) {
		DieWithUserMessage("Parameter(s)", "<Server Name>, <Server Port>, <# chars in PW>");
	}

	serverIP = argv[1];
	serverPort = argv[2];
	numInPass = atoi(argv[3]);

	//Set the options for the server address structure
	struct addrinfo addrOpts;
	memset(&addrOpts, 0, sizeof(addrOpts));
	addrOpts.ai_family = AF_UNSPEC;
	addrOpts.ai_flags = AI_PASSIVE;
	addrOpts.ai_socktype = SOCK_DGRAM;
	addrOpts.ai_protocol = IPPROTO_UDP;

	//Get a list of server addresses
	struct addrinfo *serverList;
	returnNum = getaddrinfo(NULL, serverPort, &addrOpts, &serverList);
	if(returnNum != 0) {
		DieWithUserMessage("getaddrinfo() failed", gai_strerror(returnNum));
	}

	//Create a socket
	sock = socket(serverList->ai_family, serverList->ai_socktype, serverList->ai_protocol);
	if(sock < 0) {
		DieWithSystemMessage("socket() failed");
	}

	while(true) { //Loop forever
		password = selectNextPassword(numInPass);
		attemptCount += 1;

		//Send the password
		ssize_t numBytes = sendto(sock, password, sizeof(password), 0, serverList->ai_addr, serverList->ai_addrlen);
		if (numBytes < 0) {
			DieWithSystemMessage("sendto() failed");
		} else if (numBytes != sizeof(password)) {
			DieWithUserMessage("sendto() error", "sent unexpected number of bytes");
		}

		//This is to see if the server times out in one second
		struct timeval tv;
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
			continue; //skip the rest of this iteration
		}

		//Recieve whether we succeeded or not
		struct sockaddr_storage fromServer;
		socklen_t fromServerLength = sizeof(fromServer);
		char buffer[8]; //Buffer for success/failure
		numBytes = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *) &fromServer, &fromServerLength);
		if(numBytes < 0) {
			DieWithSystemMessage("recvfrom() failed");
		}
		buffer[7] = '\0';
		if(strcmp(buffer, "SUCCESS") == 0) {
			printf("\nCorrect password: %s Total attempts: %d. Total time: %fs.\n\n", password, attemptCount, getTime() - initialTime);
			break;
		} else {
			continue;
		}
	}
	freeaddrinfo(serverList);
}

/*
Parameters: none

This routine is called when Control + C is pressed to terminate the program.
It prints out the total number of password attempts and the amount of time
the program was running.
*/
void intHandler() {
	printf("\n\nTotal attempts: %d. Total time: %fs.\n\n", attemptCount, getTime() - initialTime);
	exit(0);
}

/*
Parameters: int numChars. This is the number of characters long the password will be.

This routine creates a randomly generated password for the client to send to the server.
*/
char* selectNextPassword(int numChars) {
	char* password = (char*) malloc(numChars+1);
	int temp;
	int i; //To make C99 happy

	for(i = 0; i < numChars; i++) {
		temp = rand() % (126 + 1 - 33) + 33;
		while(!validASCIINum(temp)) {
			temp = rand() % (126 + 1 - 33) + 33;
		}
		password[i] = (char) temp;
	}
	password[numChars] = 0;
	return password;
}

/*
Parameters: int num. This is the ASCII number that is being checked if it's in the correct range.

This routine checks whether an ASCII number is in the correct ranges for valid characters
for the password. It returns true if it is valid and false otherwise.
*/
bool validASCIINum(int num) {
	if((num >= 33 && num <= 47) || (num >= 49 && num <= 52) || (num >= 58 && num <= 64) || (num >= 91 && num <= 126)) {
		return true;
	} else {
		return false;
	}
}

/*
Parameters: none

This routine gets the current time and returns it in seconds to microsecond precision.
This was provided by Dr. Remy.
*/
double getTime() //From Dr. Remy
{
	struct timeval curTime;
	(void) gettimeofday (&curTime, (struct timezone *) NULL);
	return (((((double) curTime.tv_sec) * 1000000.0) + (double) curTime.tv_usec) / 1000000.0); 
}
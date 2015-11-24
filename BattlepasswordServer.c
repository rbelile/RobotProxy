/*
	passwordServer.c
	Daniel Battle - Assignment 1
	This file is a server that hosts a password. A client connects to it and tries to
	guess the password. Routines:
	main
	generatePW
	validASCIINum
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

char* generatePW(int numChars);
bool validASCIINum(int num);
void intHandler();

//These are here so they can be printed once the program is terminated by Control + C.
int correctGuesses;
int messRecieved;
struct sockaddr_storage clientAddr;

/*
Parameters: int argc, char *argv[]. Argc is the number of command line arguments, while
argv[] is an array that contains the command line arguments.

This is the main function for the server. It creates a password, creates a socket, 
binds to it, then waits until contacted by the client. The client sends a password. If that
password is correct, the server sends a SUCCESS message and creates a new password. If
the password is incorrect, the server waits until contacted by the client again. Once
the server is terminated, it prints statistics out.
*/
int main(int argc, char *argv[]) {
	char *serverPort;
	int numCharPW;
	int returnNum;
	int sock;
	char* currentPW;

	srand(time(NULL));
	signal(SIGINT, intHandler);

	correctGuesses = 0;
	messRecieved = 0;

	if (argc < 3 || argc > 4) { // Test for correct number of arguments
 		DieWithUserMessage("Parameter(s)", "<Server Port>, <# chars in PW>, [<initialPassword>]");
 	}

	serverPort = argv[1];
	if(atoi(serverPort) < -32767 || atoi(serverPort) > 32767) { //max and min 16 bit integer
		DieWithUserMessage("Invalid Parameter", "<Server Port> must be a 16 bit number.");
	}

	numCharPW = atoi(argv[2]);
	if(numCharPW < 1 || numCharPW > 8) {
		DieWithUserMessage("Invalid Parameter", "<# chars in PW> must be between 1 and 8.");
	}

	currentPW = argv[3];
	if(currentPW == NULL) {
		currentPW = generatePW(numCharPW);
	}
	printf("The current password is: %s\n", currentPW);

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

	//Bind the socket
	if (bind(sock, serverList->ai_addr, serverList->ai_addrlen) < 0) {
		DieWithSystemMessage("bind() failed. Please try a port from 5K - 10K");
	}

	freeaddrinfo(serverList); //Free the list of server addresses

	while(true) { //Loop forever
		socklen_t clientAddrLen = sizeof(clientAddr);

		//Recieve passwor attempt
		char buffer[numCharPW]; // I/O buffer
	    ssize_t numBytes = recvfrom(sock, buffer, 500, 0, (struct sockaddr *) &clientAddr, &clientAddrLen);
	    messRecieved += 1;
		if (numBytes < 0) {
			DieWithSystemMessage("recvfrom() failed");
		}

		//Send SUCCESS or FAILURE back to the client
		char *resultMess;
		if(strcmp(buffer, currentPW) == 0) { //success
			resultMess = "SUCCESS";
			correctGuesses += 1;
			numBytes = sendto(sock, resultMess, sizeof(resultMess), 0, (struct sockaddr *) &clientAddr, clientAddrLen);
			currentPW = generatePW(numCharPW);
			printf("Previous password guessed. The current password is: %s\n", currentPW);
		} else {
			resultMess = "FAILURE";
			numBytes = sendto(sock, resultMess, sizeof(resultMess), 0, (struct sockaddr *) &clientAddr, clientAddrLen);
		}
		if(numBytes < 0) {
			DieWithSystemMessage("sendto() failed");
		}
	}
}

/*
Parameters: none

This routine is called when Control + C is pressed to terminate the program.
It prints out the total number of messages received, the number of correct guesses, and
the IP address of the client.
*/
void intHandler() {
	printf("\n\nNumber of messages received: %d. Number of correct guesses: %d.\n", messRecieved, correctGuesses);
	printf("The connected client's IP address: ");
	PrintSocketAddress((struct sockaddr *) &clientAddr, stdout);
	printf("\n\n");
	exit(0);
}

/*
Parameters: int numChars. This is the number of characters long the password will be.

This routine creates a randomly generated password for the server.
*/
char* generatePW(int numChars) {
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
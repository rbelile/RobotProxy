/*
 * CPSC 3600 Final Project
 * UDP Client
 * This program sends robot commands to a server.
 */

#include "shared.h"

 void sendToServer (int);

char response[10000];
int sock; 
struct sockaddr_in servAddr;
struct sockaddr_in fromAddr; 
struct hostent *thehost; 
unsigned short servPort; 
unsigned int fromSize; 
char *servIP; 
int respStringLen; 
char * id;

int main(int argc, char *argv[]) {
	if (argc != 3) 
	{
		fprintf(stderr, "Usage: %s <Server IP> <Server Port>\n", argv[0]);
		exit(1);
	}

	signal(SIGINT, clientCNTCCode);

	servIP = argv[1]; 

	if (argc == 3)
		servPort = atoi(argv[2]); 
	else
		servPort = 7; 

	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		DieWithError("socket() failed");

	memset(&servAddr, 0, sizeof(servAddr)); 
	servAddr.sin_family = AF_INET; 
	servAddr.sin_addr.s_addr = inet_addr(servIP);
	servAddr.sin_port = htons(servPort); 

	if (servAddr.sin_addr.s_addr == -1) {
		thehost = gethostbyname(servIP);
		servAddr.sin_addr.s_addr =  *((unsigned long *) thehost->h_addr_list[0]);
	}

	sendToServer (0);
	//sendToServer (1);
	sendToServer (2);

	sleep(5);

	sendToServer (3);

	sleep(1);

	sendToServer (0);
	//sendToServer (1);
	sendToServer (4);

	sleep(1);

	sendToServer (0);
	//sendToServer (1);
	sendToServer (3);

	sleep(1);

	sendToServer (0);
	//sendToServer (1);
	sendToServer (2);

	sleep(5);

	//sendToServer (3);

	sleep(1);

	sendToServer (0);
	//sendToServer (1);

	close(sock);
	exit(0);
}

void sendToServer (int code) {
	id = (char *) malloc (sizeof (char));
	sprintf (id, "%d", code);
	if (sendto(sock, id, sizeof (id), 0, (struct sockaddr *) &servAddr, sizeof(servAddr)) != sizeof (id)) 
		DieWithError("sendto() sent a different number of bytes than expected");

	fromSize = sizeof(fromAddr);
	respStringLen = recvfrom(sock, response, sizeof(response), 0, (struct sockaddr *) &fromAddr, &fromSize);

	if (servAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr) {
		fprintf(stderr, "Error: received a packet from unknown source \n");
	}

	response[respStringLen] = '\0';
	printf ("%s\n", response);
}

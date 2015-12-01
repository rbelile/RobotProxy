/*
 * UDPClient.c
 *
 *  Created on: Dec 3, 2014
 *      Author: Steven
 */

#include "client.h"
#include "methods.h"

int main(int argc, char* argv[]) {
	struct sockaddr_in serverAddress;	//Contains info for server address
	struct sockaddr_in hostAddress; //contains information for binding
	char *buffer = (char*) malloc(1024);					//buffer for sending commands
	FILE* cmds;							//file pointer to commands.txt
	unsigned int sleeptime;
	int sock1;							//socket
	int sock2;

	if(argc > 2 || argc == 1) {
		err_n_die("Arguments: ./program <ip address>");
	}

	//create socket
	sock1 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	sock2 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sock1 < 0 || sock2 < 0) {
		err_n_die("socket() failed\n");
	}

	memset(&serverAddress, 0, sizeof(serverAddress));

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(5000);
	if(inet_pton(AF_INET, argv[1], &serverAddress.sin_addr.s_addr) == -1) {
		err_n_die("inet_pton() failed\n");
	}

	//bind sock2
	memset(&hostAddress, 0, sizeof(hostAddress));
	hostAddress.sin_family = AF_INET;
	hostAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	hostAddress.sin_port = htons(5001);
	if(bind(sock2, (struct sockaddr *)&hostAddress, sizeof(hostAddress)) < 0) {
		err_n_die("bind() failed\n");
	}

	//read commands from text file.
	cmds = fopen("commands.txt", "r");
	size_t len = 0;
	while(getline(&buffer, &len, cmds) != -1) {

		//Get sleep time
		if(strstr(buffer, "sleep") != NULL){
			//Get ready for bed.
			int i = 0;
			for(i = 0; i < strlen(buffer); i++) {
				if(atoi(&buffer[i]) < 10) {
					//good night
					sleeptime = (unsigned int)buffer[i];
					sleep(sleeptime);
					break;
				} //if
			} //for
			//Shouldn't expect a response after sleeping
			continue;
		} //if

		//send regular command
		else {
			if(sendto(sock1, buffer, MAX, 0, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
				err_n_die("sendto() failed\n");
			} //if
		} //else
		//Block until we get response
		getResponse(sock2);
	} //while

	//TODO: Receive data and save data. infinite loop?
	free(buffer);
	fclose(cmds);
	close(sock1);
	close(sock2);
	return 0;
} //main

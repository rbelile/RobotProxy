// process request

#include "methods.h"


int processRequest(char *request,char *messageBuffer){
	int port;
	char getLine[100] = "";
	char message[50];
	strcpy(message,messageBuffer);
	// create first line of request
	if(strcmp(message,"GPS\n") == 0){
		port = 8082;
		strcat(getLine,"GET state?id=little HTTP/1.1\r\n");
	}else if(strcmp(message,"DGPS\n") == 0){
		port = 8084;
		strcat(getLine,"GET state?id=little HTTP/1.1\r\n");
	}else if(strcmp(message,"LASERS\n") == 0){
		port = 8083;
		strcat(getLine,"GET state?id=little HTTP/1.1\r\n");
	}else if(strcmp(message,"MOVE\n") == 0){
		port = 8082;
		strcat(getLine,"GET twist?id=little&lx=4 HTTP/1.1\r\n");
	}else if(strcmp(message,"TURN\n") == 0){
		port = 8082;
		strcat(getLine,"GET twist?id=little&az=30 HTTP/1.1\r\n");
	}else if(strcmp(message,"STOP\n") == 0){
		port = 8082;
		strcat(getLine,"GET twist?id=little&lx=0 HTTP/1.1\r\n");
	}else {
		port = -1;
	}
	if(port != -1){
		strcat(request,getLine);
		strcat(request,"Connection: Keep-Alive\r\n");
		strcat(request,"Host: www.cs.clemson.edu\r\n");
		strcat(request,"\r\n");
	}
	return port;
} // stores http request in request

#define MAX 100
// Will use port 5000
int getRequest(char *messageBuffer, struct sockaddr_in *clntAddr,int sock){
                    /* Socket */

    unsigned int clientAddrLen;         /* Length of incoming message */
    char buffer[MAX];        /* Buffer for echo string */
    unsigned short echoServPort;     /* Server port */
    int recvMsgSize;                 /* Size of received message */



    clientAddrLen = sizeof(*clntAddr);

    /* Block until receive message from a client */
    if ((recvMsgSize = recvfrom(sock, buffer, MAX, 0,
        (struct sockaddr *) clntAddr, &clientAddrLen)) < 0)
        err_n_die("recvfrom() failed");
    strcpy(messageBuffer,buffer);
 
   
    return 0;
} 

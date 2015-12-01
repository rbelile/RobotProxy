#include <stdio.h>    
#include <sys/socket.h> 
#include <arpa/inet.h>  
#include <stdlib.h>     
#include <string.h>    
#include <unistd.h>     
#include "shared.h"

#define ECHOMAX 1024   

int main(int argc, char *argv[])
{
    int sock;                      
    struct sockaddr_in servAddr;
    struct sockaddr_in clntAddr; 
    unsigned int cliAddrLen;        
    char indexBuffer[ECHOMAX];       
    unsigned short servPort;   
    int recvMsgSize;        
    char http_request[10000];

    if (argc != 4) {
        fprintf(stderr,"Usage:  %s <robot ID> <robot HostName> <UDP Port>\n", argv[0]);
        exit(1);
    }

	char * robotID = argv[1];
	char* robotHostName = argv[2];
    servPort = atoi(argv[3]);  

    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    memset(&servAddr, 0, sizeof(servAddr));  
    servAddr.sin_family = AF_INET;     
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servAddr.sin_port = htons(servPort);  

    if (bind(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        DieWithError("bind() failed");
  
    for (;;) {
        cliAddrLen = sizeof(clntAddr);

        if ((recvMsgSize = recvfrom(sock, indexBuffer, ECHOMAX, 0, (struct sockaddr *) &clntAddr, &cliAddrLen)) < 0)
            DieWithError("recvfrom() failed");

        int index = atoi (indexBuffer);
        char webpage[50];
        char robotP[50];

        switch (index) {
            case 0:
                sprintf (webpage, "/state?id=%s", robotID);
                sprintf (robotP, "8082");
                break;
            case 1:
                sprintf (webpage, "/state?id=%s", robotID);
                sprintf (robotP, "8084");
                break;
            case 2:
                sprintf (webpage, "/twist?id=%s&lx=4", robotID);
                sprintf (robotP, "8082");
                break;
            case 3:
                sprintf (webpage, "/twist?id=%s&lx=0", robotID);
                sprintf (robotP, "8082");
                break;
            case 4:
                sprintf (webpage, "/twist?id=%s&az=30", robotID);
                sprintf (robotP, "8082");
                break;
            default:
                printf ("What???\n");
        }


        sprintf (http_request, "GET %s HTTP/1.1\r\nHost: %s:%s\r\n\r\n", webpage, robotHostName, robotP);
        printf ("%s\n", http_request);

        int s; 
        struct sockaddr_in robotServAddr;
        unsigned short robotServPort; 
        char *robotServIP;  
        char robotBuffer[256];     /* Buffer for echo string */

        robotServIP = robotHostName;
        robotServPort = atoi (robotP);

        if ((s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
            DieWithError("socket() failed");

        memset(&robotServAddr, 0, sizeof(robotServAddr));     
        robotServAddr.sin_family      = AF_INET;       
        robotServAddr.sin_addr.s_addr = inet_addr(robotServIP);  
        robotServAddr.sin_port        = htons(robotServPort); 

        if (connect(s, (struct sockaddr *) &robotServAddr, sizeof(robotServAddr)) < 0)
            DieWithError("connect() failed");
   

        if (send(s, http_request, strlen (http_request), 0) != strlen (http_request))
            DieWithError("send() sent a different number of bytes than expected");

        //printf ("sent stuffs\n");
        int bytesRcvd;
        if ((bytesRcvd = recv(s, robotBuffer, 256 - 1, 0)) <= 0)
            DieWithError("recv() failed or connection closed prematurely");

        close (s);

        if (sendto(sock, robotBuffer, strlen (robotBuffer), 0, (struct sockaddr *) &clntAddr, sizeof(clntAddr)) != strlen (robotBuffer))
            DieWithError("sendto() sent a different number of bytes than expected");
    }
}

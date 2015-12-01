#include "methods.h"


void sendRequest(char * requestData, int portNumber, char * recievedData)
{
	struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET; // Set Family
    serverAddr.sin_port = htons(portNumber); // Set HTTP port number
    serverAddr.sin_addr.s_addr = inet_addr("130.127.192.62"); // Set IP Address

	int tcpSocket = socket(AF_INET, SOCK_STREAM, 0); // Create TCP Socket

    // Connect to HTTP Server
	if(connect(tcpSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
        err_n_die("Error Connecting:");

    // Send Request to HTTP Server
    if(send(tcpSocket, requestData, strlen(requestData), 0) < 0)
        err_n_die("Error with send():");

    // Recive the Data back from the HTTP server
    if(recv(tcpSocket, recievedData, sizeof(recievedData), 0) < 0)
        err_n_die("Error Reciving Return Data:");

    // Clost TCP Socket
    close(tcpSocket);


}

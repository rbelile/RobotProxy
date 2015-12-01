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
#include <time.h>

#define MAXSTRINGLENGTH 300
using namespace std;

int generatePassword();
string getcommand(int check);

 string hostName;
 int robotID, robotNum;
 in_port_t port;
 char* UdpServerPort;

void DieWithError(string errorMessage){ // Error handling function
    fprintf(stdout, "%s\n", errorMessage.c_str());
    exit(1);
}

string getCommand(int check){

    if(check == 0)
    {
      return "Connect";
    }
    else if(Check == 2){
      return "Image"
    }
    else if(Check == 4){
      return "GPS";
    }
    else if(Check == 8){
      return "dGPS";
    }
    else if(Check == 16){
      return "Lasers"; 
    }
    else if(Check == 32){
      return "Move";
    }
    else if(Check == 64){
      return "Turn";
    }
    else if(Check == 128){
      return "Stop";
    }
    else if(Check == 255){
      return "Quit";
    }
    else if(Check == 256){
      return "Error with Client Request";
    }
    else if(Check == 512){
      return "Error from Robot";
    }
    else return "Command not recognized";
}

string checkCommand(string command){
  if(command == "Image") return "8081";
  else if(command == "GPS") return "8082";
  else if(command == "dGPS") return "8084";
  else if(command == "Lasers") return "8083";
  else if(command == "Move") return "8082";
  else if(command == "Turn") return "8082";
  else if(command == "Stop") return "8082";
  else if(command == "Error From Client Request") return "Bad1";
  else if(command == "Error from Robot") return "Bad2";
  else return "Bad";
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
      int buffer[7]; // I/O buffer

      // Size of received message
      ssize_t numBytesRcvd = recvfrom(sock, buffer, MAXSTRINGLENGTH, 0,
          (struct sockaddr *) &clntAddr, &clntAddrLen);
      if (numBytesRcvd < 0)
          DieWithError("recvfrom() failed");

      buffer[1] = generatePassword();
      int password = buffer[1];

      //cout << "Password: " << buffer[1] << endl;

        // Send received datagram back to the client
        ssize_t numBytesSent = sendto(sock, buffer, numBytesRcvd, 0, 
        (struct sockaddr *) &clntAddr, sizeof(clntAddr));

        if (numBytesSent < 0)
            DieWithError("sendto() failed");

      for (;;) {

          ssize_t numBytesRcvd = recvfrom(sock, buffer, MAXSTRINGLENGTH, 0,
            (struct sockaddr *) &clntAddr, &clntAddrLen);
          if (numBytesRcvd < 0)
              DieWithError("recvfrom() failed");
          if(buffer[1] != password){
            break;
          }
        
          string command = getCommand(buffer[2]);
          int requestData = buffer[3]; // Value of the move and turn commands.
          //int sequenceNumber = buffer[4];
          if(command == "Command not recognized") break;
          if(command == "255") break;

          string payload = TcpClient(hostName, robotID, robotNum, command, requestData);
          if(payload == "Error0") break;
          else if(payload == "Error1"){
            cout << "Error with Client Request, waiting for new connection" << endl;
            break;
          } 
          else if(payload == "Error2"){
            cout << "Error from Robot, waiting for new connection" << endl;
            break;
          }

      
      }
      
      ssize_t numBytesRcvd = recvfrom(sock, buffer, MAXSTRINGLENGTH, 0,
          (struct sockaddr *) &clntAddr, &clntAddrLen);
      if (numBytesRcvd < 0)
          DieWithError("recvfrom() failed");
      
      ssize_t numBytesSent = sendto(sock, buffer, numBytesRcvd, 0, 
      (struct sockaddr *) &clntAddr, sizeof(clntAddr));

	  close(sock);
        //else if (numBytesSent != numBytesRcvd)
          //  DieWithError("sendto() sent unexpected number of bytes");
    }
}

string TcpClient(string Server, int robotID, int robotNum, string command, int requestData){

    char* port;

    if(checkCommand(command) == "Bad") return "Error0";
    else if(checkCommand == "Bad1") return "Error1";
    else if(checkCommand == "Bad2") return "Error2";
    else port = checkCommand(command).c_str();
    
    string payload;
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
    ServAddr.sin_addr.s_addr = inet_addr(Server.c_str());   // Server IP address
    //ServAddr.sin_port        = htons(port); /* Server port */


    if(ServAddr.sin_addr.s_addr == -1)
    {
        host = gethostbyname(Server.c_str());
            ServAddr.sin_addr.s_addr = *((unsigned long *) host->h_addr_list[0]);
    }

    /* Establish the connection to the  server */
    if (connect(sock, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) < 0)
        DieWithError("connect() failed");
        
    string HTTPreq = "http://" + hostname + ":";    
	switch(command){
		case 2:
			HTTPreq += "8081/snapshot?topic=/robot9/image?width=600?height=500";
			break;
		case 4:
			HTTPreq += "8082/state?id=5senior";
			break;
		case 8:
			HTTPreq += "8084/state?id=5senior";
			break;
		case 16:
			HTTPreq += "8083/state?id=5senior";
			break;
		case 32:
			HTTPreq += "8082/twist?id=5senior&lx=";
			HTTPreq += requestData;
			break;
		case 64:
			HTTPreq += "8082/twist?id=5senior&az=";
			HTTPreq += requestData;
			break;
		case 128:
			HTTPreq += "8082/twist?id=5senior&lx=0";
			break;
	}
    
    close(sock);
    exit(0);
}

int generatePassword() {
    return rand() % 20000000;
}

int main(int argc, char *argv[]) {
 
  srand(time(NULL));

  if (argc != 9) {
    cout << "Usage: " << argv[0] << " -h <hostname-of-robot> -i <robot-id> -n " 
      << "<robot-number> -p <port>" << endl;
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
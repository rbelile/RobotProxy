/*
Greg Meaders
Cpsc 3600 Project 2
getfile.cpp

This file contains the code to run the Client program for project two. The program will 
communicate with an HTTP server to recieve a requested file, or prints to stdout the information 
of the requested webpage.

Routines:
  dieWithMessage = A function to display a string and then close the program when an error occurs.
  hostname_to_ip = takes in the URL and an empty IP string. Converts the URL to an IP that is useable 
                   for the connection

*/


#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <unistd.h>     /* for close() */
#include <sys/stat.h>   //for getting last modified date.
#include <time.h>
#include <sys/types.h>
#include <string.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <ctime>        //for getting time and date
#include <netdb.h>
#include <errno.h> //For errno - the error number

using namespace std;

#define RCVBUFSIZE 32   /* Size of receive buffer */

//A function to display a string and then close the program when an error occurs.
void DieWithError(string errorMessage){ // Error handling function
    perror(errorMessage.c_str());
    exit(1);
}

//takes in the URL and an empty IP string. Converts the URL to an IP that is useable 
//for the connection
int hostname_to_ip(char *hostname , char *ip)
{
    int sockfd;  
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_in *h;
    int rv;
 
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
    hints.ai_socktype = SOCK_STREAM;
 
    if ( (rv = getaddrinfo( hostname , "http" , &hints , &servinfo)) != 0) 
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
 
    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) 
    {
        h = (struct sockaddr_in *) p->ai_addr;
        strcpy(ip , inet_ntoa( h->sin_addr ) );
    }
     
    freeaddrinfo(servinfo); // all done with this structure
    return 0;
}

int main(int argc, char *argv[])
{

    int sock;                        /* Socket descriptor */
    struct sockaddr_in ServAddr; /*  server address */
    unsigned short ServerPort;     /*  server port */
    string servIP;                    /* Server IP address (dotted quad) */
    char *String;                /* String to send to  server */
    unsigned int StringLen;      /* Length of string to  */
    int bytesRcvd, totalBytesRcvd;   /* Bytes read in single recv() 
                                        and total bytes read */
    bool foundUrl = false;
    bool foundFilename = false;
    bool portFound = false;
    string filename;
    string host;
    string header;
    string receivedFile;
    string originalName;
    
    if(argc == 1) DieWithError("No Url Given, Exiting now");
        

    for(int i = 1; i < argc; i++){

       if(strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "-t") == 0){

           ServerPort = atoi(argv[i+1]);

           if(ServerPort != 0){                                  //will be 0 if port is not the next number
               i++;
               portFound = true;
           }
       }
       else if((strcmp(argv[i], "-f") == 0) && argc >= i+1){  //if -f is not the last argument, 
                                                              //should show that the filename is the next argument 
           string temp = argv[i+1];
           if(temp[0] == '/'){
             filename = argv[i+1];
             i++;
             foundFilename = true;
           }
           else{
            filename = "/";
            originalName = argv[i+1];
            filename = filename + argv[i+1];
            i++;
            foundFilename = true;
          }
       }
       else{
           servIP = argv[i];
           foundUrl == true;
       }
    }
    
    if(portFound == false) ServerPort = 8080;

    servIP = argv[1];
    string URL;
    string requestPath;
    if(strncmp(servIP.c_str(), "http://", 7) == 0)
    { 

      size_t start = 7;
      size_t endUrl = servIP.find_first_of("/", start);
      URL = servIP.substr(start, (endUrl-start));
      requestPath = servIP.substr(endUrl);
    }
    else
    {
      size_t pos = servIP.find('/');
      if(pos != string::npos)
      {
         size_t endUrl = servIP.find_first_of("/");
         URL = servIP.substr(0, endUrl);
         requestPath = servIP.substr(endUrl);
      }
      else{
        URL = servIP;
        requestPath = "/";
      }
    }

    char *hostname = argv[1];
    strcpy(hostname, URL.c_str());
    char ip[100];
    hostname_to_ip(hostname, ip);
    servIP = ip;

    if(foundFilename == true){
      stringstream ss;
      ss << "GET " << filename << " HTTP/1.1\r\nHost: " << URL << "\r\nConnection: Keep-Alive\r\n\r\n";
      header = ss.str();
    }

    else{
      filename = requestPath;
      stringstream ss;
      ss << "GET " << filename << " HTTP/1.1\r\nHost: " << URL << "\r\nConnection: Keep-Alive\r\n\r\n";
      header = ss.str();
    }

    // Create a reliable, stream socket using TCP 
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    // Construct the server address structure 
    memset(&ServAddr, 0, sizeof(ServAddr));     // Zero out structure 
    ServAddr.sin_family      = AF_INET;             // Internet address family 
    ServAddr.sin_addr.s_addr = inet_addr(servIP.c_str());   // Server IP address 
    ServAddr.sin_port        = htons(ServerPort); // Server port 

    // Establish the connection to the  server 
    if (connect(sock, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) < 0)
        DieWithError("connect() failed");

    StringLen = strlen(String);          // Determine input length 

    // Send the string to the server 
    if (send(sock, header.c_str(), header.length(), 0) != header.length())
        DieWithError("send() sent a different number of bytes than expected");

    // Receive the same string back from the server 
    totalBytesRcvd = 0;
    int recvMsgSize;
    char Buffer[RCVBUFSIZE];
    if ((recvMsgSize = recv(sock, Buffer, RCVBUFSIZE, 0)) <= 0)
            DieWithError("recv() failed or connection closed prematurely");

    receivedFile = Buffer;
    //for some reason it was always adding my IP to the end of the first recv, this counters that
    size_t first = receivedFile.find_first_of(".");
    size_t second = receivedFile.find_first_of(".", first+1);
    receivedFile.erase((second-3));
    bool foundchunk = false;
    size_t findchunk;
    while (recvMsgSize ==  RCVBUFSIZE)
    {
        char Buffer[RCVBUFSIZE];
        if(foundchunk == true) break;
        // Receive up to the buffer size (minus 1 to leave space for
        //   a null terminator) bytes from the sender 
        if ((recvMsgSize = recv(sock, Buffer, RCVBUFSIZE, 0)) <= 0)
            DieWithError("recv() failed or connection closed prematurely");
    
        receivedFile += Buffer;

        findchunk = receivedFile.find("Length: ");
        if(findchunk != string::npos){
          foundchunk = true;
          if ((recvMsgSize = recv(sock, Buffer, RCVBUFSIZE, 0)) <= 0)
            DieWithError("recv() failed or connection closed prematurely");

          receivedFile += Buffer;
          break;
        }
    }
    if(foundchunk == true){
      int chunksize;
      size_t startlen = receivedFile.find_first_of( " ", (findchunk+8));
      string endoflen = receivedFile.substr(findchunk+8, startlen);
      chunksize = atoi(endoflen.c_str());
      if(chunksize == 0){DieWithError("Chunk length not found");}
      else{
        while(chunksize != 0){
          char Buffer[RCVBUFSIZE];

          if ((chunksize = recv(sock, Buffer, chunksize, 0)) <= 0)
            DieWithError("recv() failed or connection closed prematurely");

          receivedFile += Buffer;

        }
      }
    }
    //Check if file name was given, if so save the recieved string to it.
    if(foundFilename == true)
    {
      fprintf(stdout, "Saving To: %s\n", originalName.c_str());
      ofstream output(originalName.c_str());
      output << receivedFile;
      output.close();
    }
    else
    {
      int length = receivedFile.length();
      string Length;
      stringstream convert;
      convert << length;
      Length = convert.str();
      string firstpart = "HTTP request sent to " + URL + "\nRecieved Length: " + Length +"\nSaving to: STDOUT\nReceived Information: \n" + receivedFile;
      fprintf(stdout, "%s", firstpart.c_str());

    } 

    close(sock);
    
    exit(0);
}

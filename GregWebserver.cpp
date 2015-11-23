/*
Greg Meaders
Cpsc 3600 Project 2
webserver.cpp

This file contains the code to run the Server Program. It will recieve GET or HEAD requests and either
return the requested file or an appropriate error code.

Routines:
  dieWithMessage = A function to display a string and then close the program when an error occurs.

  getFileType = Takes in the path and returns a string of the type based on the .type of the file.

  getTime = will return current time string, formatted like the pdf requested.

  getFileCreationTime = returns the time in string format of the last time the file was altered.

  findHost = searches through the received header for the Host: section.

  getAccess = checks the requested file for read access.

  find403Error = checks that the user has not requested a file outside of the given directory.

  HandleTCPClient = handles the communicaton with the clients, including error checking and 
                    sending information back to the client.
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
#include <unistd.h>     //for access

#define MAXPENDING 5    /* Maximum outstanding connection requests */
#define RCVBUFSIZE 32   /* Size of receive buffer */
using namespace std;

void DieWithError(string errorMessage){ // Error handling function
    perror(errorMessage.c_str());
    exit(1);
}

//Takes in the path and returns a string of the type based on the .type of the file.
string getFileType(string path){

    size_t startFrom = path.find_last_of(".");
    string type = path.substr(startFrom+1);      //getting the rest of the address after the . in the requested filepath.

    if((type == "html")||(type == "htm")){

        type = "Content -Type: text/html\r\n";
    }
    else if(type == "css"){

        type = "Content -Type: text/css\r\n";    
    }
    else if(type == "js"){

        type = "Content -Type: application/javascript\r\n";    
    }
    else if(type == "txt"){

        type = "Content -Type: text/plain\r\n";    
    }
    else if(type == "jpg"){

        type = "Content -Type: image/jpg\r\n";    
    }
    else if(type == "pdf"){

        type = "Content -Type: application/pdf\r\n";    
    }
    else 
        type = "Content -Type: apllication/octet-stream\r\n";

    return type;
}

//will return current time string, formatted like the pdf requested.
string getTime(){

    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char buffer[80];
    strftime(buffer, 80, "Date: %a, %d %b %Y %T", timeinfo);
    string currentTime = buffer;
    return currentTime;

}

//returns the time in string format of the last time the file was altered.
string getFileCreationTime(string path) {   //when it was last modified
    struct stat attr;
    stat(path.c_str(), &attr);          //getting the last time the file was modified
    struct tm * timeinfo;
    timeinfo = localtime(&attr.st_mtime); //creating a timeinfo like in getTime to format the string
    char buffer[80];
    strftime(buffer, 80, "Last-Modified: %a, %d %b %Y %T\r\n", timeinfo);  //formatting the string to requirements
    string lastModified = buffer;
    return lastModified;
}

//Function to find if the client has a designated Host: section
bool findHost(string message){
    string findMessage = "Host";
    size_t pos;
    bool found = false;
    //finding position of the word host
    pos = message.find(findMessage);
    if(pos != string::npos){
        findMessage = "Host: \r\n";  //useless host
        pos = message.find(findMessage);
        if(pos != string::npos){
            found = false;
        }
        else found = true;
    }
    else found = false;

    return found;
}
//checks the requested file for read access.
bool getAccess(string path){
    int check;
    check = access(path.c_str(), R_OK);

    if(check == 0) return true;
    else return false;
}

//checks that the user has not requested a file outside of the given directory.
bool find403Error(string message){
    string findMessage = "../";
    size_t pos;
    bool found = false;
    pos = message.find(findMessage);
    if(pos != string::npos){
        found = true;
    }
    else found = false;
    return found;
}

//handles the communicaton with the clients, including error checking and 
//sending information back to the client.
void HandleTCPClient(int clntSocket, string fileLocation)
{  /* TCP client handling function */
   
    char Buffer[RCVBUFSIZE];            /* Buffer for string */
    int recvMsgSize;                    /* Size of received message */
    string wholeMessage;
    string recieveTime;

    /* Receive message from client */
    if ((recvMsgSize = recv(clntSocket, Buffer, RCVBUFSIZE, 0)) < 0)
        DieWithError("recv() failed");

    wholeMessage = Buffer;
    /* Send received string and receive again until end of transmission */
    while (recvMsgSize == RCVBUFSIZE)      /* zero indicates end of transmission */
    {
       
        /* See if there is more data to receive */
        if ((recvMsgSize = recv(clntSocket, Buffer, RCVBUFSIZE, 0)) < 0)
            DieWithError("recv() failed");

         wholeMessage += Buffer;
    }
    recieveTime = getTime();
    //getting the first and second word being sent from the client
    size_t foundInstruction = wholeMessage.find_first_of(" ");
    string instruction = wholeMessage.substr(0, foundInstruction);
    
    //+1 so it starts from character after the last space
    //+2 so it will skip the / when requesting a file name, if it prints a blank line then nothing was requested.
    size_t foundRequest = wholeMessage.find_first_of(" ", foundInstruction+1);      
    string request = wholeMessage.substr(foundInstruction+2, foundRequest - (foundInstruction+2));
    
    int status = 0;
    string Time;
    string connectionClose = "Connection: close\r\n";
    
   
    if(instruction == "GET")
    {
        bool hostFound = findHost(wholeMessage);

        if(request.length() < 30)
        {

            if(hostFound == true)
            {
                bool error403 = find403Error(request);

                if(error403 == false)
                {
                    string location = fileLocation + request;
                    fstream f(location.c_str());
                    string line;
                    string content;
                    if(f.good())
                    {
                        while(getline(f, line))
                        {

                            content += line;

                        }
                        if(getAccess(location) == false)
                        {
                                Time = getTime();
                                string header = "HTTP/1.1 403 ERROR\r\n" + Time + "\r\nConnection: close\r\nServer: Webserver/1.0\r\n\r\n 403 access denied";  
                                if(send(clntSocket, header.c_str(), header.length(), 0) != header.length())
                                {
                                    DieWithError("send() failed");
                                }
                                close(clntSocket);
                                status = 403;
                                if(instruction == "GET")
                                {
                                    fprintf(stdout, "%s %s  %s %d \n", instruction.c_str(), request.c_str(), recieveTime.c_str(), status); 
                                }
                                
                        }
                        else
                        {

                            Time = getTime();
                            string LastModified = getFileCreationTime(location);
                            string fileType = getFileType(location);
                            stringstream ss;
                            ss << "Content -Length   "  << content.length() << "\r\n";
                            string fileLength = ss.str();
                            string server = "Server: WebServer/1.0\r\n";
             
                            string header = "HTTP/1.1 200 OK\r\n" + Time + "\r\n" + LastModified + fileType + fileLength + connectionClose + server  + "\r\n";

                            content = header + content;
                            if(send(clntSocket, content.c_str(), content.length(), 0) != content.length())
                            {
                                DieWithError("send() failed");
                            }
                            close(clntSocket);
                            status = 200;
                            if(instruction == "GET")
                            {
                                //printing out server recieve information
                                fprintf(stdout, "%s %s  %s %d \n", instruction.c_str(), request.c_str(), recieveTime.c_str(), status); 
                            }//end Get Print
                        }//End else
                    }
                    else
                    {
                        //handling the 404 file not found error   
                        Time = getTime();  
                        string header = "HTTP/1.1 404 ERROR\r\n" + Time + "\r\nConnection: close\r\nServer: Webserver/1.0\r\n\r\n 404 File Not Found";  
                        if(send(clntSocket, header.c_str(), header.length(), 0) != header.length())
                        {
                            DieWithError("send() failed");
                        }//end Send if
                        close(clntSocket);
                        status = 404;
                
                        if(instruction == "GET")
                        {
                            //printing out server recieve information
                            fprintf(stdout, "%s %s  %s %d \n", instruction.c_str(), request.c_str(), recieveTime.c_str(), status); 
                        }//end GET print

                    }//end 404
                }
                else
                {
                    Time = getTime();
                    string header = "HTTP/1.1 403 ERROR\r\n"+ Time + "\r\nConnection: close\r\nServer: Webserver/1.0\r\n\r\n 403 access denied";  
                    if(send(clntSocket, header.c_str(), header.length(), 0) != header.length())
                    {
                        DieWithError("send() failed");
                    }
                    close(clntSocket);
                    status = 403;
                    if(instruction == "GET")
                    {
                        fprintf(stdout, "%s %s  %s %d \n", instruction.c_str(), request.c_str(), recieveTime.c_str(), status); 
                    }
                }

            }
            else
            {
                Time = getTime();
                string header = "HTTP/1.1 400 ERROR\r\n" + Time + "\r\nConnection: close\r\nServer: Webserver/1.0\r\n\r\n 400 Bad Request";  
                if(send(clntSocket, header.c_str(), header.length(), 0) != header.length())
                {
                    DieWithError("send() failed");
                }
                close(clntSocket);
                status = 400;
                if(instruction == "GET")
                {
                    fprintf(stdout, "%s %s  %s %d \n", instruction.c_str(), request.c_str(), recieveTime.c_str(), status); 
                }
            }

        }
        else
        {
            Time = getTime();
            string header = "HTTP/1.1 414 ERROR\r\n" + Time + "\r\nConnection: close\r\nServer: Webserver/1.0\r\n\r\n 414 Request Too Large.";  
            if(send(clntSocket, header.c_str(), header.length(), 0) != header.length())
            {
                    DieWithError("send() failed");
            }
            close(clntSocket);
            status = 414;
            if(instruction == "GET")
            {
                fprintf(stdout, "%s %s  %s %d \n", instruction.c_str(), request.c_str(), recieveTime.c_str(), status); 
            }

        }
    }
    else if(instruction == "HEAD")
    {
        bool hostFound = findHost(wholeMessage);

        if(request.length() < 30)
        {

            if(hostFound == true)
            {
                bool error403 = find403Error(request);

                if(error403 == false)
                {

                    
                    string location = fileLocation + request;

                    fstream f(location.c_str());
                    string line;
                    
                    if(f.good())
                    {
                        if(getAccess(location) == false)
                        {
                            Time = getTime();
                            string header = "HTTP/1.1 403 ERROR\r\n" + Time + "\r\nConnection: close\r\nServer: Webserver/1.0\r\n\r\n 403 access denied";  
                            if(send(clntSocket, header.c_str(), header.length(), 0) != header.length())
                            {
                                DieWithError("send() failed");
                            }
                            close(clntSocket);
                            status = 403;
                            if(instruction == "HEAD")
                            {
                                fprintf(stdout, "%s %s  %s %d \n", instruction.c_str(), request.c_str(), recieveTime.c_str(), status); 
                            }
                                
                        }
                        else
                        {

                            Time = getTime();
                            string LastModified = getFileCreationTime(location);
                            string fileType = getFileType(location);
                            stringstream ss;
                            string fileLength = ss.str();
                            string server = "Server: WebServer/1.0\r\n";
                            string header = "HTTP/1.1 200 OK\r\n" + Time + "\r\n" + LastModified + fileType + fileLength + connectionClose + server  + "\r\n";

                            
                            if(send(clntSocket, header.c_str(), header.length(), 0) != header.length())
                            {
                                DieWithError("send() failed");
                            }
                            close(clntSocket);
                            status = 200;
                            if(instruction == "HEAD")
                            {
                                //printing out server recieve information
                                fprintf(stdout, "%s %s  %s %d \n", instruction.c_str(), request.c_str(), recieveTime.c_str(), status); 
                            }//end Get Print
                        }//End else
                    }
                    else
                    {
                        Time = getTime();
                        //handling the 404 file not found error     
                        string header = "HTTP/1.1 404 ERROR\r\n" + Time + "\r\nConnection: close\r\nServer: Webserver/1.0\r\n\r\n 404 File Not Found";  
                        if(send(clntSocket, header.c_str(), header.length(), 0) != header.length())
                        {
                            DieWithError("send() failed");
                        }//end Send if
                        close(clntSocket);
                        status = 404;
                
                        if(instruction == "HEAD")
                        {
                            //printing out server recieve information
                            fprintf(stdout, "%s %s  %s %d \n", instruction.c_str(), request.c_str(), recieveTime.c_str(), status); 
                        }//end GET print

                    }//end 404
                }
                else
                {
                    Time = getTime();
                    string header = "HTTP/1.1 403 ERROR\r\n" + Time + "\r\nConnection: close\r\nServer: Webserver/1.0\r\n\r\n 403 access denied";  
                    if(send(clntSocket, header.c_str(), header.length(), 0) != header.length())
                    {
                        DieWithError("send() failed");
                    }
                    close(clntSocket);
                    status = 403;
                    if(instruction == "HEAD")
                    {
                        fprintf(stdout, "%s %s  %s %d \n", instruction.c_str(), request.c_str(), recieveTime.c_str(), status); 
                    }
                }

            }
            else
            {
                Time = getTime();
                string header = "HTTP/1.1 400 ERROR\r\n" + Time + "\r\nConnection: close\r\nServer: Webserver/1.0\r\n\r\n 400 Bad Request";  
                if(send(clntSocket, header.c_str(), header.length(), 0) != header.length())
                {
                    DieWithError("send() failed");
                }
                close(clntSocket);
                status = 400;
                if(instruction == "HEAD")
                {
                    fprintf(stdout, "%s %s  %s %d \n", instruction.c_str(), request.c_str(), recieveTime.c_str(), status); 
                }
            }

        }
        else
        {
            Time = getTime();
            string header = "HTTP/1.1 414 ERROR\r\n" + Time + "\r\nConnection: close\r\nServer: Webserver/1.0\r\n\r\n 414 Request Too Large.";  
            if(send(clntSocket, header.c_str(), header.length(), 0) != header.length())
            {
                    DieWithError("send() failed");
            }
            close(clntSocket);
            status = 414;
            if(instruction == "HEAD")
            {
                fprintf(stdout, "%s %s  %s %d \n", instruction.c_str(), request.c_str(), recieveTime.c_str(), status); 
            }

        }

    }
    else
    {
        Time = getTime();
        //handling the 405 error
        string header = "HTTP/1.1 405 ERROR\r\n" + Time + "\r\nAllow: GET, HEAD\r\nConnection: close\r\nServer: Webserver/1.0\r\n\r\n";
         if(send(clntSocket, header.c_str(), header.length(), 0) != header.length()){
                    DieWithError("send() failed");
                }
                close(clntSocket);
                status = 405;   
    }
    
    
    close(clntSocket);    /* Close client socket */
}

  

int main(int argc, char *argv[])
{
    int servSock;                    /* Socket descriptor for server */
    int clntSock;                    /* Socket descriptor for client */
    struct sockaddr_in servAddr; /* Local address */
    struct sockaddr_in ClntAddr; /* Client address */
    unsigned short ServerPort;     /* Server port */
    unsigned int clntLen;            /* Length of client address data structure */
    string fileLocation;        //Location of the file given from commandline argument;

    //checks for if port and location are in the arguments.
    bool portFound = false;
    bool locationFound = false;

    //making sure that everything from command line is stored in the correct location
    for(int i = 1; i < argc; i++){
        if((strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "-t") == 0) && argc >= i+1){  //if -p or -t is not the last argument, 
                                                                                        //should show that the port is the next number

            ServerPort = atoi(argv[i+1]);

            if(ServerPort != 0){                                  //will be 0 if port is not the next number

                i++;
                portFound = true;
            }
        }
        else{
            fileLocation = argv[i];
            fileLocation = fileLocation + "/";
            locationFound = true;
        }
    } 
    //setting to default values if the information did not come from command line
    if(portFound == false){
        ServerPort = 8080;
    }
    if(locationFound == false){
        fileLocation = "./";
    }
    

    /* Create socket for incoming connections */
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");
      
    /* Construct local address structure */
    memset(&servAddr, 0, sizeof(servAddr));   /* Zero out structure */
    servAddr.sin_family = AF_INET;                /* Internet address family */
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    servAddr.sin_port = htons(ServerPort);      /* Local port */

    /* Bind to the local address */
    if (bind(servSock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        DieWithError("bind() failed");

    /* Mark the socket so it will listen for incoming connections */
    if (listen(servSock, MAXPENDING) < 0)
        DieWithError("listen() failed");

    for (;;) /* Run forever */
    {
        /* Set the size of the in-out parameter */
        clntLen = sizeof(ClntAddr);

        /* Wait for a client to connect */
        if ((clntSock = accept(servSock, (struct sockaddr *) &ClntAddr, 
                               &clntLen)) < 0)
            DieWithError("accept() failed");

        HandleTCPClient(clntSock, fileLocation);        //calling the handle client function wiht the socket and location
    }
    /* NOT REACHED */
}

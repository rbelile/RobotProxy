#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <string.h>
#include <string>
#include <iostream>
#include <fstream>

#define RCVBUFSIZE 1024

using namespace std;

int main(int argc, char *argv[]) {
    int sock;
    int endOfHeaders, whileCheck, j, contentLength, endOfChunkNum;
    struct sockaddr_in webServerAddr; // Web server address
    struct hostent *thehost;
    char *servName;
    char *httpServerName;
    char *servIP;
    bool toFile = false;
    char c;
    ofstream fileOut;
    in_port_t serverPort;
    string getHTTPLine;
    char *fileName;
    char receiveHTTPBuffer[RCVBUFSIZE];
    int bytesRcvd, totalBytesRcvd;
    char contentLenArray[20];
    string fileRequest, stringHTTPServer;
    size_t findSlash;
    char chunkedLength[30];

    // Test for correct number of arguments
    if ((argc != 2) && (argc != 4) && (argc != 6)) {
        fprintf(stderr, "Usage: %s <URL> [-t port] [-f filename]\n", argv[0]);
        exit(1);
    }

    // Parse command line arguments
    servName = argv[1];
    if (argc == 4) {
        string argv2 = argv[2];
        if ((argv2.compare("-p") == 0) || (argv2.compare("-t") == 0)) {
            serverPort = atoi(argv[3]);
        } else if (strcmp(argv[2], "-f") == 0) {
            serverPort = 8080;
            fileName = argv[3];
            toFile = true;
        } else {
            fprintf(stderr, "Usage: %s <URL> [-t port] [-f filename]\n", argv[0]);
            exit(1);
        }
    } else if (argc == 6) {
        serverPort = atoi(argv[3]);
        fileName = argv[5];
        toFile = true;
    } else {
        serverPort = 8080;
    }

    // Create a reliable, stream socket using TCP
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("socket() failed");
        exit(1);
    }

    // Take the http portion off
    if(servName[4] == 's') {
        httpServerName = servName + 8;
    } else {
        httpServerName = servName + 7;
    }
    stringHTTPServer = httpServerName;
    httpServerName = strtok(httpServerName, "/");

    // Get the document to find
    findSlash = stringHTTPServer.find("/");
    if (findSlash != string::npos) {
        fileRequest = stringHTTPServer.substr(findSlash, string::npos);
    } else {
        fileRequest = "/index.html";
    }

    // Construct the server address structure
    memset(&webServerAddr, 0, sizeof(webServerAddr));
    webServerAddr.sin_family = AF_INET;
    thehost = gethostbyname(httpServerName);
    inet_ntoa(*(struct in_addr*)thehost->h_addr);
    webServerAddr.sin_addr.s_addr = ((*(struct in_addr*)thehost->h_addr).s_addr);
    webServerAddr.sin_port = htons(serverPort);

    // Establish the connection to the echo server
    if (connect(sock, (struct sockaddr *) &webServerAddr, sizeof(webServerAddr)) < 0) {
        perror("connect() failed");
        exit(1);
    }

    //HTTP stuff here
    string stringHTTPServerName = httpServerName;
    string httpGet = "GET " + fileRequest + " HTTP/1.1";
    string httpHost = "Host: " + stringHTTPServerName;
    string totalHTTP = httpGet + "\n" + httpHost + "\n\n";

    //Send the string to the server
    if (send(sock, totalHTTP.c_str(), totalHTTP.size(), 0) != totalHTTP.size()) {
        perror("send() sent a different number of bytes than expected");
        exit(1);
    }

    if (toFile) {
        fileOut.open(fileName);
    }

    // Receive the HTTP response from the server
    bool atMessage = false;
    string currentPartMessage;
    size_t foundLoc, chunkedCheck, contentLenCheck;

    // Get to the end of the http headers
    bytesRcvd = recv(sock, receiveHTTPBuffer, RCVBUFSIZE - 1, MSG_PEEK);
    for(int i = 0; i < bytesRcvd; i++) {
        c = receiveHTTPBuffer[i];
        if (c == '\n') {
            c = receiveHTTPBuffer[i+1];
            char c2 = receiveHTTPBuffer[i+2];
            // \r\n\r\n is supposed to be the end of the http headers
            if (c == '\r' && c2 == '\n') {
                endOfHeaders = i + 2;
                break;
            }
        }
    }

    currentPartMessage = receiveHTTPBuffer;
    chunkedCheck = currentPartMessage.find("Transfer-Encoding: chunked");
    contentLenCheck = currentPartMessage.find("Content-Length: ");

    // Gets the length of the HTTP body from the Content-Length header if there
    if (contentLenCheck != string::npos) {
        // Get length of the http body
        j = 0;
        whileCheck = contentLenCheck + 16;
        c = receiveHTTPBuffer[whileCheck];
        while(c != '\n') {
            contentLenArray[j] = c;
            j++;
            whileCheck++;
            c = receiveHTTPBuffer[whileCheck];
        }
        contentLenArray[j] = '\0';
        contentLength = atoi(contentLenArray);

        // Gets the message body and puts it in the file or prints to cout
        char receiveMessage[endOfHeaders+contentLength];
        int k = 0;
        while (k < (endOfHeaders + contentLength)) {
            bytesRcvd = recv(sock, receiveMessage+k, endOfHeaders+contentLength-k, 0);
            k += bytesRcvd;
        }

        for (int y = endOfHeaders+1; y < k; y++) {
            c = receiveMessage[y];
            if (toFile) {
                fileOut << c;
                fileOut << flush;
            } else {
                cout << c;
                cout << flush;
            }
        }
    } else if (chunkedCheck != string::npos) {
        // Get to the beginning of the chunks
        char receiveMessage[endOfHeaders+1];
        bytesRcvd = recv(sock, receiveMessage, endOfHeaders+1, 0);
        bool isEndOfChunks = false;
        char chunkedMessage[30];
        bytesRcvd = recv(sock, chunkedMessage, 30, MSG_PEEK);

        // Go until finding a chunk of 0 
        while (!isEndOfChunks) {
            for (int i = 0; i < bytesRcvd; i++) {
                c = chunkedMessage[i];
                if (c == '\r') {
                    c = chunkedMessage[i+1];
                    if (c == '\n') {
                        chunkedLength[i] = '\0';
                        endOfChunkNum = i+1;
                        break;
                    }
                }
                chunkedLength[i] = c;
            }

            unsigned int chunkBuffer = strtoul(chunkedLength, NULL, 16);
            chunkBuffer = chunkBuffer + 2; // For \r\n
            int j = 0;
            if (chunkBuffer == 2) {
                isEndOfChunks = true;
            }

            // Clear out the chunk part
            memset(&chunkedMessage, 0, sizeof(chunkedMessage));
            bytesRcvd = recv(sock, chunkedMessage, endOfChunkNum, 0);

            // Get the right number of bytes
            char actualMessage[chunkBuffer];
            memset(&actualMessage, 0, sizeof(actualMessage));
            while (j < chunkBuffer) {
                bytesRcvd = recv(sock, actualMessage+j, chunkBuffer-j, 0);
                j += bytesRcvd;
            }
            if (j != chunkBuffer) {
                cout << "\nIncorrect number of bytes. Got " << j << ", should be "
                    << chunkBuffer << endl;
                exit(1);
            }
            cout << actualMessage << flush;
            memset(&chunkedMessage, 0, sizeof(chunkedMessage));
            bytesRcvd = recv(sock, chunkedMessage, 30, MSG_PEEK);
        }
    }

    cout << endl; // Add a blank line
    close(sock);
    exit(0);
}
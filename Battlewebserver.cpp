#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iomanip>

#define MAXPENDING 5
#define RCVBUFSIZE 3000 

using namespace std;

string getDate() {
    time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );
    int year = now->tm_year + 1900;
    int month = now->tm_mon + 1;
    int day = now->tm_mday;
    int hour = now->tm_hour;
    int min = now->tm_min; 
    int sec = now->tm_sec;

    stringstream ss;

    ss << "" << month << "/" << day << "/" << year << " " << hour << ":"
        << setfill('0') << setw(2) << min << ":" << setfill('0') << setw(2) << sec;
    return ss.str();
}

int main(int argc, char *argv[]) {
    int servSock, clntSock;
    struct sockaddr_in webServerAddr, clientAddr;
    unsigned int clntLen;
    in_port_t serverPort;
    string directory = "./";
    char httpBuffer[RCVBUFSIZE];
    int recvMsgSize;

    if (argc > 4) {
        fprintf(stderr, "Usage:  %s [-t Server Port] [directory]\n", argv[0]);
        exit(1);
    }

    // Parse command line arguments
    if (argc == 2) {
        directory = argv[1];
        serverPort = 8080;
    } else if (argc == 3) {
        string argv2 = argv[1];
        if ((argv2.compare("-p") == 0) || (argv2.compare("-t") == 0)) {
            serverPort = atoi(argv[2]);
        } else {
            fprintf(stderr, "Usage:  %s [-t Server Port] [directory]\n", argv[0]);
            exit(1);
        }
    } else if (argc == 4) {
        string argv2 = argv[1];
        if ((argv2.compare("-p") == 0) || (argv2.compare("-t") == 0)) {
            serverPort = atoi(argv[2]);
        } else {
            fprintf(stderr, "Usage:  %s [-t Server Port] [directory]\n", argv[0]);
            exit(1);
        }
        directory = argv[3];
    } else {
        serverPort = 8080;
    }

    /* Create socket for incoming connections */
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("socket() failed");
        exit(1);
    }
      
    // Construct local address structure
    memset(&webServerAddr, 0, sizeof(webServerAddr));   /* Zero out structure */
    webServerAddr.sin_family = AF_INET;                /* Internet address family */
    webServerAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    webServerAddr.sin_port = htons(serverPort);      /* Local port */

    // Bind to the local address
    if (bind(servSock, (struct sockaddr *) &webServerAddr, sizeof(webServerAddr)) < 0) {
        perror("bind() failed");
        exit(1);
    }

    //  Mark the socket so it will listen for incoming connections 
    if (listen(servSock, MAXPENDING) < 0) {
        perror("listen() failed");
        exit(1);
    }

    for (;;) {
        clntLen = sizeof(clientAddr);

        // Wait for a client to connect
        if ((clntSock = accept(servSock, (struct sockaddr *) &clientAddr, &clntLen)) < 0) {
            perror("accept() failed");
            exit(1);
        }

        // Receive message from client
        memset(httpBuffer, 0, RCVBUFSIZE);
        if ((recvMsgSize = recv(clntSock, httpBuffer, RCVBUFSIZE, 0)) < 0) {
            perror("recv() failed");
            exit(1);
        }

        string strHTTPbuffer = httpBuffer;
        string httpToSend, httpBody;

        size_t fileNameStart = strHTTPbuffer.find_first_not_of(" ", 3);
        size_t fileNameEnd = strHTTPbuffer.find(" ", fileNameStart);

        // Parse the http request and respond appropriately
        if (strHTTPbuffer.find("Host:") == string::npos) {
            httpToSend = "HTTP/1.1 400 Bad Request\r\nDate: " +
                    getDate() + "\r\nConnection: close\r\nServer: TheLittleServerThatCould/1.0\r\n\r\n";
            cout << strHTTPbuffer.substr(0, 4) << "    " << strHTTPbuffer.substr(fileNameStart, fileNameEnd-fileNameStart)
                << "    " << getDate() << "    " << "400" << endl;
        } else if(strHTTPbuffer.find("GET") == 0) {
            string filePath = directory + strHTTPbuffer.substr(fileNameStart, fileNameEnd-fileNameStart);
            string contentType = "application/octet-stream";
            if (filePath.length() > 30) {
                httpToSend = "HTTP/1.1 414 Request-URI Too Long\r\nDate: " + getDate() +
                    "\r\nConnection: close\r\nServer: TheLittleServerThatCould/1.0\r\n\r\n";
                cout << "GET   " << strHTTPbuffer.substr(fileNameStart+1, fileNameEnd-fileNameStart)
                    << "    " + getDate() << "   414" << endl;
                goto sendHTTP;
            } else if(filePath.find(".") != string::npos) {
                string extension = filePath.substr(filePath.rfind(".")+1);
                if(extension == "css") {
                    contentType = "text/css";
                } else if (extension == "html" || extension == "htm") {
                    contentType = "text/html";
                } else if (extension == "js") {
                    contentType = "application/javascript";
                } else if (extension == "txt") {
                    contentType = "text/plain";
                } else if (extension == "jpg") {
                    contentType = "image/jpeg";
                } else if (extension == "pdf") {
                    contentType = "application/pdf";
                }
                contentType = "Content-Type: " + contentType + "\r\n";
            }
            ifstream file(filePath.c_str());
            if(file.good()) {
                string line;
                httpToSend = "HTTP/1.1 200 OK\r\nDate: " + getDate() + "\r\nLast-Modified: ";
                while(getline(file, line)) {
                    httpBody = httpBody + line + "\n";
                }

                // Get last modified time
                struct stat buf;
                stat(filePath.c_str(), &buf);
                time_t t = buf.st_mtime;
                struct tm lt;
                localtime_r(&t, &lt);
                localtime_r(&t, &lt);
                char timbuf[80];
                strftime(timbuf, sizeof(timbuf), "%c", &lt);

                stringstream slength;
                slength << httpBody.size();
                httpToSend = httpToSend + timbuf + "\r\nContent-Length: " + 
                    slength.str() + "\r\n" + contentType + "Connection: close\r\n"+
                    "Server: TheLittleServerThatCould/1.0\r\n\r\n" + httpBody;
                cout << "GET   " << strHTTPbuffer.substr(fileNameStart+1, fileNameEnd-fileNameStart)
                    << "    " + getDate() << "   200" << endl;
            } else {
                httpToSend = "HTTP/1.1 404 Not Found\r\nDate: " + getDate() +
                    "\r\nConnection: close\r\nServer: TheLittleServerThatCould/1.0\r\n\r\n";
                cout << "GET   " << strHTTPbuffer.substr(fileNameStart+1, fileNameEnd-fileNameStart)
                    << "    " + getDate() << "   404" << endl;
            }
        } else if(strHTTPbuffer.find("HEAD") == 0) {
            size_t fileNameStart = strHTTPbuffer.find_first_not_of(" ", 4);
            size_t fileNameEnd = strHTTPbuffer.find(" ", fileNameStart);

            string filePath = directory + strHTTPbuffer.substr(fileNameStart, fileNameEnd-fileNameStart);
            string contentType = "application/octet-stream";
            if (filePath.length() > 30) {
                httpToSend = "HTTP/1.1 414 Request-URI Too Long\r\nDate: " + getDate() +
                    "\r\nConnection: close\r\nServer: TheLittleServerThatCould/1.0\r\n\r\n";
                cout << "HEAD   " << strHTTPbuffer.substr(fileNameStart+1, fileNameEnd-fileNameStart)
                    << "    " + getDate() << "   414" << endl;
                goto sendHTTP;
            } else if(filePath.find(".") != string::npos) {
                string extension = filePath.substr(filePath.rfind(".")+1);
                if(extension == "css") {
                    contentType = "text/css";
                } else if (extension == "html" || extension == "htm") {
                    contentType = "text/html";
                } else if (extension == "js") {
                    contentType = "application/javascript";
                } else if (extension == "txt") {
                    contentType = "text/plain";
                } else if (extension == "jpg") {
                    contentType = "image/jpeg";
                } else if (extension == "pdf") {
                    contentType = "application/pdf";
                }
                contentType = "Content-Type: " + contentType + "\r\n";
            }
            ifstream file(filePath.c_str());
            if(file.good()) {
                string line;
                httpToSend = "HTTP/1.1 200 OK\r\nDate: " + getDate() + "\r\nLast-Modified: ";

                // Get last modified time
                struct stat buf;
                stat(filePath.c_str(), &buf);
                time_t t = buf.st_mtime;
                struct tm lt;
                localtime_r(&t, &lt);
                localtime_r(&t, &lt);
                char timbuf[80];
                strftime(timbuf, sizeof(timbuf), "%c", &lt);
                while(getline(file, line)) {
                    httpBody = httpBody + line + "\n";
                }

                stringstream slength;
                slength << httpBody.size();
                httpToSend = httpToSend + timbuf + "\r\nContent-Length: " + 
                    slength.str() + "\r\n" + contentType + "Connection: close\r\n"+
                    "Server: TheLittleServerThatCould/1.0\r\n\r\n";
                cout << "HEAD   " << strHTTPbuffer.substr(fileNameStart, fileNameEnd-fileNameStart)
                    << "    " + getDate() << "   200" << endl;
            } else {
                httpToSend = "HTTP/1.1 404 Not Found\r\nDate: " + getDate() +
                    "\r\nConnection: close\r\nServer: TheLittleServerThatCould/1.0\r\n\r\n";
                cout << "HEAD   " << strHTTPbuffer.substr(fileNameStart, fileNameEnd-fileNameStart)
                    << "    " + getDate() << "   404" << endl;
            }
        } else {
            size_t findSpace = strHTTPbuffer.find(" ");
            cout << strHTTPbuffer.substr(0, findSpace) + "  " << strHTTPbuffer.substr(fileNameStart, fileNameEnd-fileNameStart)
                    << "    " + getDate() << "   405" << endl;
            httpToSend = "HTTP/1.1 405 Method Not Allowed\r\nDate: " +
                getDate() + "\r\nAllow: GET, HEAD" +
                "\r\nConnection: close\r\nServer: TheLittleServerThatCould/1.0\r\n\r\n";
        }
sendHTTP:
        if (send(clntSock, httpToSend.c_str(), httpToSend.size(), 0) != httpToSend.size()) {
            perror("send() sent a different number of bytes than expected");
            exit(1);
        }
        close(clntSock);
    }
}
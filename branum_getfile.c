#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>

void DieWithUserError(const char *msg, const char *detail);
void DieWithError(const char *msg);

int main(int argc, char **argv) 
{
   char * url;
   char * server_name;
   char * path;
   char * hostIP;
   char * file_name;
   int port = 8080;

   struct hostent *host;
   struct in_addr hostAddr;

   FILE * output;
   int file = 0;
   char httpMessage[1024];

   //Interpret data provided in the command line.
   if(argc%2 != 0) 
   {
      DieWithUserError("Parameters",
         "<URL> [-p <Port>] [-o <file_name>]");
   }

   url = argv[1];
   
   int i;
   for(i = 2; (i < argc) && (argv[i][0] == '-'); i++) 
   {
      if(argv[i][1] == 'p') 
         port = atoi(argv[++i]);
      
      else if(argv[i][1] == 'o') 
      {
         file_name = argv[++i];
	      file = 1;
      }
   }

   // Open the file based on given file name
   if(file)
   {
      output = fopen(file_name, "w");
   }

   server_name = (char *)malloc(sizeof(url));
   path = (char *)malloc(sizeof(url));

   server_name = strtok(url, "/");
   server_name = strtok(NULL, "/");
   path = strtok(NULL, "\0");

   // Compile http request response
   strcpy(httpMessage, "GET /");
   strcat(httpMessage, path);
   strcat(httpMessage, " HTTP/1.1\r\nHost: ");
   strcat(httpMessage, server_name);
   strcat(httpMessage, "\r\n\r\n");

   host = gethostbyname(server_name);
   bcopy(host->h_addr, (char *)&hostAddr, sizeof(hostAddr));

   hostIP = inet_ntoa(hostAddr);

   // Socket for sending http request
   int client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if(client_socket < 0) 
      DieWithError("socket() failed");

   struct sockaddr_in servAddr;
   memset(&servAddr, 0, sizeof(servAddr));
   servAddr.sin_family = AF_INET;
   
   int value = inet_pton(AF_INET, hostIP, &servAddr.sin_addr.s_addr);
   
   if(value == 0) 
   {
      DieWithUserError("inet_pton() failed", "invalid address string");
   }
   else if(value < 0) 
   {
      DieWithError("inet_pton() failed");
   }
   servAddr.sin_port = htons(port);

   if(connect(client_socket, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) 
   {
      DieWithError("connect() failed");
   }

   size_t message_length = strlen(httpMessage);

   ssize_t num_bytes = send(client_socket, httpMessage, message_length, 0);
   
   if(num_bytes < 0) 
   {
      DieWithError("send() failed");
   }
   else if(num_bytes != message_length) 
   {
      DieWithUserError("send()", "sent unexpected number of bytes");
   }

   // Receive http response and print
   unsigned int bytes_received = 0;
   int j = 1;
   while(1) 
   {
      char buffer[1024];

      num_bytes = recv(client_socket, buffer, 1024 - 1, 0);
      if(num_bytes < 0) 
      {
         DieWithError("recv() failed");
      }
      if(num_bytes == 0) 
         break;
      
      bytes_received += num_bytes;
      buffer[num_bytes] = '\0';
      
      if(j) 
      {
         if(file) 
            fputs(strstr(buffer, "\r\n\r\n")+4, output);
         else 
            fputs(strstr(buffer, "\r\n\r\n")+4, stdout);
         
         j = 0;
      }
      else
      {
         if(file) 
            fputs(buffer, output);
         else 
            fputs(buffer, stdout);
      }
   }

   fputc('\n', stdout);

   close(client_socket);
   exit(1);

}

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

static const int MAXPENDING = 5;

void HandleTCPClient(int client_socket);
char * getContentType(char * file_name);
char * concat(char *s1, char *s2);
void DieWithUserError(const char *msg, const char *detail);
void DieWithError(const char *msg);

int port;
char *directory;
FILE *file_path;

int main(int argc, char *argv[]) 
{
   port = 8080;
   directory = "./";

   struct sockaddr_in servAddr;
   struct sockaddr_in clntAddr;

   int server_socket;
   int client_socket;

   // Parse command line arguments to get port and current directory
   if(argc != 1) 
   {
      if(argv[1][0] == '-') 
      {
         port = atoi(argv[2]);
	      if(argc == 4) directory = argv[3];
      }
      else
         directory = argv[1];
   }

   // Create socket for incoming connections 
   if((server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) 
   {
      DieWithError("socket() failed");
   }

   // Construct local address structure
   memset(&servAddr, 0, sizeof(servAddr));
   servAddr.sin_family = AF_INET;
   servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   servAddr.sin_port = htons(port);

   // Bind to local address
   if(bind(server_socket, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) 
   {
      DieWithError("bind() failed");
   }

   // Mark the socket so it will listen for incoming connections
   if(listen(server_socket, MAXPENDING) < 0) 
   {
      DieWithError("listen() failed");
   }

   for(;;) 
   { 
      // Set the size of the in-out parameter
      socklen_t clntLen = sizeof(clntAddr);

      
      if ((client_socket = accept(server_socket, (struct sockaddr *) &clntAddr, 
                               &clntLen)) < 0)
      {
         DieWithError("accept() failed");
      }

      HandleTCPClient(client_socket);
   }
}

void HandleTCPClient(int client_socket) 
{
   char *buffer = malloc(1024);
   char *temp = malloc(1024);
   char *temp2 = malloc(1024);

   int recvMsgSize;
   int get_request = 1;
   
   char *response = malloc(1024);
   char *response_code = malloc(3);

   char *file_name;

   int file_length = 0;
   char *length = malloc(50);
   char *http_message = malloc(1024);
   char ch;

   int error_flag = 0;
   int error400_flag = 0;

   char *date = malloc(1000);
   char *lmdate = malloc(1000);

   struct tm tm;
   time_t thisTime;

   struct stat status;

   // Receive a request from the client
   if((recvMsgSize = recv(client_socket, buffer, 1024, 0)) < 0)
      DieWithError("recv() failed");

   // Generate http response
   strcat(temp, buffer);
   strcat(response, "HTTP/1.1 ");

   // Determine if there is a 405 Error
   if(strstr(temp, "GET") == NULL ||
      strcmp(temp, strstr(temp, "GET")) != 0) 
   {
      if (strstr(temp, "HEAD") == NULL ||
         strcmp(temp, strstr(temp, "HEAD")) != 0) 
      {
         strcat(response, "405 Error : Unsupported request\r\nAllow: HEAD, GET\r\n");
	      strcpy(response_code, "405");
         error_flag = 1;
      }
      else 
         get_request = 0;
   }

   // Determine if there is a 400 Error
   else if(strstr(temp, "HTTP/1.1") == NULL) 
   {
      strcat(response, "400 Bad Request\r\n");
      strcpy(response_code, "400");
      error_flag = 1;
      error400_flag = 1;
   }

   if(!error400_flag) 
   {
      strcpy(temp2, temp);
      strtok(temp2, " ");
      file_name = strtok(NULL, " ");
      
      // Determine if there is a 403 error: File being accessed not in current directory
      if(strstr(file_name, "..") != NULL) 
      {
         //strcat(response, "403 Error : File not in current directory\r\n");
         strcat(response, "403 Forbidden\r\n");
         strcpy(response_code, "403");
         error_flag = 1;
      }
   }

   // Determine if there are any file errors
   if(!error_flag) 
   {
      file_path = fopen(concat(directory, file_name), "r");
      if(file_path == NULL) 
      {
         // Determine if there is a 404 Error: File not found
         if(errno == 2) 
         {
	         strcat(response, "404 Error: File not found\r\n");
	         strcpy(response_code, "404");
	         error_flag = 1;
         }
	      // Determine if there is a 403 Error: Permission denied for requested file
	      else if(errno == 13) 
         {
	         //strcat(response, "403 Error: Permission denied\r\n");
            strcat(response, "403 Forbidden\r\n");
	         strcpy(response_code, "403");
	         error_flag = 1;
	      }
      }
         // 200 OK response if there are no HTTP Errors
      else 
      {
         strcat(response, "200 OK\r\n");
	      strcpy(response_code, "200");
      }
   }
   
   // Connection and Server headers for http response
   strcat(response, "Connection: close\r\n");
   strcat(response, "Server: webserver/1.0\r\n");

   // Date header for http response.
   strcat(response, "Date: ");
   time(&thisTime);
   tm = *localtime(&thisTime);
   strftime(date, 1000, "%a, %d %b %Y %H:%M:%S", &tm);
   strcat(response, date);
   strcat(response, "\r\n");

   // Format date
   strftime(date, 1000, "%a %b %Y %H:%M", &tm);
   
   if(!error_flag) 
   {
      // Last-modified stat
      strcat(response, "Last-Modified: ");
      stat(concat(directory, file_name), &status);
      tm = *localtime(&(status.st_mtime));
      strftime(lmdate, 1000, "%a, %d %b %Y %H:%M:%S", &tm);
      strcat(response, lmdate);
      strcat(response, "\r\n");

      //Content -Type header for http response
      strcat(response, "Content -Type: ");
      strcat(response, getContentType(file_name));
      
      // If there was a GET request copy file to response
      if(get_request == 1) 
      {
         while((ch = fgetc(file_path)) != EOF) 
         {
            http_message[file_length] = ch;
	         file_length++;
         }
         http_message[file_length] = '\0';

         sprintf(length, "%d", file_length);

	      // Content -Length header for http response
         strcat(response, "Content -Length: ");
         strcat(response, length);
         strcat(response, "\r\n\r\n");
         strcat(response, http_message);
      }
      else strcat(response, "\r\n");
   }
   else strcat(response, "\r\n");

   // Print output to stdout
   if(error400_flag) 
      printf("%s\t%s\n", date, response_code);
   else
      printf("%s\t%s\t%s\t%s\n", temp2, file_name+1, date, response_code);

   // Send the http response.
   ssize_t numBytes = send(client_socket, response, strlen(response), 0);
   if(numBytes < 0) 
      DieWithError("send() failed");

   else if(numBytes != strlen(response)) 
      DieWithUserError("send()", "sent unexpected number of bytes");

   close(client_socket);

}
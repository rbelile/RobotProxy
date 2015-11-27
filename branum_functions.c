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

char * getContentType(char * file_name)
{
   char * type = malloc(1024);
      
      if(strcmp(strrchr(file_name, '.'), ".html") == 0 ||
         strcmp(strrchr(file_name, '.'), ".htm") == 0) 
      {
         strcat(type, "text/html\r\n");
      }
      else if(strcmp(strrchr(file_name, '.'), ".css") == 0) 
      {
         strcat(type, "text/css\r\n");
      }
      else if(strcmp(strrchr(file_name, '.'), ".js") == 0) 
      {
         strcat(type, "application/javascript\r\n");
      }
      else if(strcmp(strrchr(file_name, '.'), ".txt") == 0) 
      {
         strcat(type, "text/plain\r\n");
      }
      else if(strcmp(strrchr(file_name, '.'), ".jpg") == 0) 
      {
         strcat(type, "image/jpeg\r\n");
      }
      else if(strcmp(strrchr(file_name, '.'), ".pdf") == 0) {
         strcat(type, "application/pdf\r\n");
      }
      else 
      {
         strcat(type, "application/octet-stream\r\n");
      }

      return type;
}

void DieWithUserError(const char *msg, const char *detail) {
   fputs(msg, stderr);
   fputs(": ", stderr);
   fputs(detail, stderr);
   fputc('\n', stderr);
   exit(0);
}

void DieWithError(const char *msg) 
{
   perror(msg);
   exit(0);
}

char * concat(char *s1, char *s2) 
{
   char *result = malloc(strlen(s1)+strlen(s2)+1);
   strcpy(result, s1);
   strcat(result, s2);
   return result;
}

/* Daniel Branum (dbranum)
   CPSC 360
   Section 001
   Homework 1
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>


void brute_force_attack(int max_length);
void password_breaker(char* string, int index, int max_depth);
void error(char *message);


int new_socket;
unsigned int new_size;
char *index = NULL;
int length = 0;
struct sockaddr_in serv_add_breaker;
struct sockaddr_in src_add_breaker;
struct hostent *host;
unsigned short serv_port_breaker;
unsigned int size;
char *serv_ip;
char *breaker_string;
char breaker_buffer[7];
int breaker_string_length;
int response_length;
int num_attempts = 0;
int char_lib_size = 62;
char *char_lib = {"abcdefghijklmnopqrstuvwxyz`~!@#$%^&*()_-+={}[]\\|:;\"'<>,.?/1234"};
char *data;
time_t start_time;
struct timeval timeout;


int main(int argc, char *argv[])
{
   
   start_time = time(NULL);
   
   if(argc != 4)
   {
      printf("Input not valid.\n");
      printf("Input should look like this: passwordBreaker serverName serverPort N\n");
      exit(1);
   }
   if(atoi(argv[3]) > 8)
   {
      printf("Password length not valid.\n");
      exit(0);
   }
   else
      index = malloc(sizeof(char) * (atoi(argv[1])));
   
   serv_ip = argv[1];
   serv_port_breaker = atoi(argv[2]);
   
   timeout.tv_sec = 1;
   timeout.tv_usec = 0;
   
   printf("Creating Socket...\n");
   if((new_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
      error("Unable to create socket.");
   
   
   memset(&serv_add_breaker, 0, sizeof(serv_add_breaker));
   serv_add_breaker.sin_family = AF_INET;
   serv_add_breaker.sin_addr.s_addr = inet_addr(serv_ip);
   serv_add_breaker.sin_port = htons(serv_port_breaker);
   
   if(serv_add_breaker.sin_addr.s_addr == -1)
   {
      host = gethostbyname(serv_ip);
      serv_add_breaker.sin_addr.s_addr = *((unsigned long *) host->h_addr_list[0]);
   }
   
   setsockopt(new_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)(&timeout), sizeof(struct timeval));
   
   data = malloc(9);
   
   brute_force_attack(atoi(argv[3]));
   
   printf("Password unable to be broken.\n");
   printf("Time elapsed = %ld seconds\n", time(NULL) - start_time);
   printf("Number of attempts = %d\n", num_attempts);
   
   return 0;
}

void brute_force_attack(int max_length)
{
   char * buffer = malloc(max_length+1);
   
   for(int i = 1; i <= max_length; i++)
   {
      memset(buffer, 0, max_length + 1);
      password_breaker(buffer, 0, i);
   }
}

void password_breaker(char* string, int index, int max_depth)
{
   for(int i = 0; i < char_lib_size; i++)
   {
      string[index] = char_lib[i];
      if(index == max_depth - 1)
      {
         breaker_string = string;
         breaker_string_length = strlen(breaker_string);
         memset(data, 0, 9);
         strcat(data, breaker_string);
         
         if(sendto(new_socket, data, 9, 0, (struct sockaddr *) &serv_add_breaker, sizeof(serv_add_breaker)) != 9)
            error("Unable to send message.");
         
         num_attempts++;
         
         if((response_length = recvfrom(new_socket, breaker_buffer, 7, 0, (struct sockaddr *) &src_add_breaker, &size)) >= 0)
         {
            if(strcmp("SUCCESS", breaker_buffer) == 0)
            {
               printf("Password broken.\n");
               printf("Password = %s\n", breaker_string);
               printf("Time elapsed = %ld seconds\n", time(NULL) - start_time);
               printf("Number of attempts = %d\n", num_attempts);
               exit(0);
            }
            else if (strcmp("FAILURE", breaker_buffer) == 0)
            {
               //printf("Password unable to be broken.\n");
            }
            else
               printf("Invalid server response.\n");
         }
      }
      
      else
         password_breaker(string, index + 1, max_depth);
   }
}

void error(char *message)
{
   perror(message);
   fprintf(stderr, "Time elpased = %ld seconds\n", time(NULL) - start_time);
   exit(1);
}

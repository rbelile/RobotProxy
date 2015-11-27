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
#include <signal.h>
#include <stdbool.h>


void error(char *errorMessage);
char* generate_password(int length);
void INThandler(int sig);
void add_address(struct sockaddr_in new_address);

char addresses_received[100][INET_ADDRSTRLEN];
int num_addresses_received = 0;
int total_num_recieved = 0;
int num_correct = 0;
bool flag = true;


int main(int argc, char *argv[])
{
   signal(SIGINT, INThandler);
   signal(SIGQUIT, INThandler);
   signal(SIGTERM, INThandler);

   while(flag) 
   {

      int sock;
      struct sockaddr_in serv_add_password;
      struct sockaddr_in client_add_password;
      unsigned int client_add_length;
      char password_buffer[8];
      unsigned short serv_port_password;
      int msg_size = 0; 

    	char *password;

      if(argc == 4)
      {
         password = argv[3];
      }
      else if(argc == 3)
      {
         password = generate_password(atoi(argv[2]));
      }
      else
      {
         printf("Input not valid.\n");
         printf("Input should look like this: passwordServer serverPort N initialPassword(optional)\n");
         exit(1);
      }

    	serv_port_password = atoi(argv[1]);

      printf("Creating Socket...\n");
      if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
         error("Unable to create socket.");

      memset(&serv_add_password, 0, sizeof(serv_add_password));
      serv_add_password.sin_family = AF_INET;
      serv_add_password.sin_addr.s_addr = htonl(INADDR_ANY);
      serv_add_password.sin_port = htons(serv_port_password);

      printf("Binding to port %d...\n", serv_port_password);
      if (bind(sock, (struct sockaddr *) &serv_add_password, sizeof(serv_add_password)) < 0)
         error("Unable to bind to port.");

    	while(flag)
    	{
         client_add_length = sizeof(client_add_password); 
  
         if (((msg_size = recvfrom(sock, password_buffer, 7, 0,
            (struct sockaddr *) &client_add_password, &client_add_length)) >= 0) && (flag == true))
         {
            add_address(client_add_password);
            total_num_recieved++;

            if(strcmp(password, password_buffer) == 0)
            {
               if (sendto(sock, "SUCCESS", 7, 0, 
                     (struct sockaddr *) &client_add_password, sizeof(client_add_password)) != 7)
               {
                  error("Unexpected number of bytes received.");
               }

               password = generate_password(atoi(argv[2]));
               num_correct++;
            }
            else if(strcmp(password, password_buffer) < 0)
            {
               if (sendto(sock, "FAILURE", 7, 0, 
                  (struct sockaddr *) &client_add_password, sizeof(client_add_password)) != 7)
               {
                  error("Unexpected number of bytes received.");
               }
            }
         }
         else
         {
            if(msg_size < 0)
               error("Receive failed.");
            if(!flag) 
            {
               //do nothing
            }
         }
      }
   }

   printf("Number of message received = %d\n",total_num_recieved);
   printf("Number of correct password = %d\n", num_correct);
   printf("Addresses recieved from:\n");
   for(int j = 1; j <= num_addresses_received; j++) 
   {
      printf("%s\n", addresses_received[j]);
   }

	return 0;
}


char* generate_password(int length)
{
	static char char_lib[] = "abcdefghijklmnopqrstuvwxyz`~!@#$%^&*()_-+={}[]\\|:;\"'<>,.?/1234";
	char *new_password = NULL;
	int get_char = 0;

	if(length > 0)
	{
	   new_password = malloc(sizeof(char) * (length));

		for(int i = 0; i < length; i++)
		{
			get_char = (rand() * 654) % sizeof(char_lib);
			new_password[i] = char_lib[get_char];	
		}
	}
	return new_password;
}

void error(char *errorMessage)
{
	perror(errorMessage);
	exit(1);
}

void INThandler(int sig)
{
   sig = 0;
   flag = false;

   printf("%d\n",total_num_recieved);
   printf("%d\n", num_correct);
    
   if(num_addresses_received != 0)
   {
      for(int j = 1; j <= num_addresses_received; j++)
      {
         printf("%s\n", addresses_received[j]);
      }
   }
   exit(0);
}


void add_address(struct sockaddr_in new_address)
{
   char tmp[INET_ADDRSTRLEN];
   inet_ntop(AF_INET, &(new_address.sin_addr), tmp, INET_ADDRSTRLEN);

   bool alreadyExist = false;
   for(int i = 1; i <= num_addresses_received; i++) 
   {
      if(strcmp(addresses_received[i], tmp) == 0)
      {
         alreadyExist = true;
      }
   }

   if(!alreadyExist)
   {
      num_addresses_received++;
      strcpy(addresses_received[num_addresses_received], tmp);
   }
}

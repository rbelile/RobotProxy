/*
Greg Meaders
Cpsc 3600 Project 1
passwordBreaker.c

This file contains the code to run the Client program for project one. The purpose of
this file will communicate with the server using a UDP socket to guess a password that
the server has generated and will loop until the password has been guessed or the user
has closed the program with CTRL-C.

Routines:
  ExitHandler = A function to catch a CTRL-C signal and display the requested information before closing the program
  dieWithMessage = A function to display a string and then close the program when an error occurs.
  
  tryPassword = takes in an initial character array for guesses, the socket, the struct for servAddr, an i to iterate with,
                and a j to handle the depth in recursion.Recursive function that will make a new password with the options 
                and send that to the server, then 
                will recieve from the server a 0 or a 1 depending on if the password was a success. If passwords do
                not match the program will recurse to chose the next option.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <time.h>
#include <poll.h>


static const char* options = "abcdefghijklmnopqrstuvwxyz1234`~!@#$\%^&*()_-+={}[]|:;'<>,.?/\\\"";
clock_t start_time, stop_time;
long guesses = 0;

void  ExitHandler(int sig)
{
     signal(sig, SIG_IGN);
     stop_time = clock();
     printf("There were %lu guesses.\n", guesses);
     printf("Elapsed time: %lf microseconds\n", (double)(stop_time - start_time));
     exit(0);    
}

void dieWithMessage(char *s)
{
    perror(s);
    exit(1);
}

int tryPassword(char *guess, int sock, struct addrinfo *servAddr, int i, int j)
{
  
  int position = 0;

  while(position < strlen(options))
  {
  	guess[i] = options[position];

  	struct sockaddr_storage fromAddr; // Source address of IPaddress

  	// Set length of from address structure (in-out parameter)
  	socklen_t fromAddrLen = sizeof(fromAddr);

  	guesses += 1;
    guess[j + 1] = '\0';
   
  	ssize_t numBytes = sendto(sock, guess, j+1, 0, servAddr->ai_addr, servAddr->ai_addrlen);
  	if (numBytes < 0)
      dieWithMessage("sendto() failed");
  	else if (numBytes != j+1)
      dieWithMessage("sendto() error, sent unexpected number of bytes");

  	char message[2];
  	//starting timeout check
    struct pollfd fd;
    int res;
    
    fd.fd = sock;
    fd.events = POLLIN;
    res = poll(&fd, 1, 1000); // 1000 ms timeout
    if(res == 0)
    {
       printf("Connection timeout, continuing to next iteration.\n");
       int next_length = i + 1;
       if(next_length <= j)
       {
          if(tryPassword(guess, sock, servAddr, next_length, j) == 0)
          {
           position++;
          }
          else
          {
           return 1;
          }
       }
       else
       {
          position++;
       }
    }
    else if(res == -1)
    {
      dieWithMessage("connection Error, client closing");
    }
    else
    {
 	    //recieving from server
 	    numBytes = recv(sock, message, 1, 0); //returning from server, 0 = success, 1 = failure, 2 = timeout
 	    if (numBytes < 0)
            dieWithMessage("recvfrom() failed");
        else if (numBytes != 1)
            dieWithMessage("recvfrom() error, received unexpected number of bytes");
  	    //printf("message is: %s\n", message);
        message[1] = '\0';

  	 if(strcmp(message, "0") == 0)
  	 {
  	   printf("Password guessed! The password was %s\n", guess);
  	   return 0;
     }
      if(strcmp(message, "1") == 0)
      {
    	 int next_length = i + 1;
    	 if(next_length <= j)
    	 {
    		  if(tryPassword(guess, sock, servAddr, next_length, j) == 0)
    		  {
    			 position++;
    		  }
    		  else
    		  {
    			 return 1;
    		  }
    	 }
    	 else
    	 {
    		  position++;
    	 }
    }
    }

  }
    return 0;
}


int main(int argc, char *argv[]) {
                                
  signal(SIGINT, ExitHandler);   //signal handler and a start time to track programs runtime
  start_time = clock();


  if (argc != 4) // Test for correct number of arguments
  {
  	dieWithMessage("3 arguments required, Server Name(IP address), Server Port(between 5k and 10k), and a password length of 1-8\n");
  }



  char *IPaddress = argv[1];     // First arg: Server's IP address

  //Checking for legal port
  char *originalport = argv[2]; // 2nd argument: Port
  int port = atoi(originalport);
  while(port < 5000 || port > 10000)
  {
    printf("port invalid, please choose a number between 5000 and 10000: ");
    scanf("%d", &port);
  }
  char servPort[6];
  sprintf(servPort, "%d", port);

  int length = atoi(argv[3]);// 3rd argument: password Length
  while(length < 1 || length > 8)
  {
    printf("Length invalid, please enter a number between 1 and 8: ");
    scanf("%d", &length);
  }

  
  
  int found = 1;
  char *guess = malloc(sizeof(char)*(length+1));
  int i = 0;
  int j = length - 1;

   

  // Tell the system what kind(s) of address info we want
  struct addrinfo addrCriteria;                   // Criteria for address match
  memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
  addrCriteria.ai_family = AF_UNSPEC;             // Any address family
  addrCriteria.ai_socktype = SOCK_DGRAM;          // Only datagram sockets
  addrCriteria.ai_protocol = IPPROTO_UDP;         // Only UDP protocol

  // Get address(es)
  struct addrinfo *servAddr; // List of IPaddress addresses
  int rtnVal = getaddrinfo(IPaddress, servPort, &addrCriteria, &servAddr);
  if (rtnVal != 0)
    dieWithMessage("getaddrinfo() failed");

  // Create a datagram/UDP socket
  int sock = socket(servAddr->ai_family, servAddr->ai_socktype, servAddr->ai_protocol); // Socket descriptor for client
  if (sock < 0)
    dieWithMessage("socket() failed");
  
  int thing = tryPassword(guess, sock, servAddr, i, j);
  
  if(thing == 0)
  {
  printf("The password was found in %lu guesses!\n", guesses);
  stop_time = clock();
  printf("Elapsed time: %lf microseconds\n", (double)(stop_time - start_time));
  }
  else
  {
    printf("The password was not guessed after %lu guesses!\n", guesses);
    stop_time = clock();
    printf("Elapsed time: %lf microseconds\n", (double)(stop_time - start_time));
    freeaddrinfo(servAddr);
  }

  close(sock);
  exit(0);
}

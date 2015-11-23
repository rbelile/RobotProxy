/*
Greg Meaders
Cpsc 3600 Project 1
passwordServer.c

This code will create the server and generate the first password where each character can equally be any valid character.
After this password is guessed it will generate the next password using the random algorithm. The server will display 
the total number of messages recieved and the total number of times the client correctly guessed the password. The server should
also display a list of all clients(specifically, their IP address) that attempted to guess the password.

Routines: ExitHandler takes care of a user pressing ctrl-c. Will then display the total number of guesses and how many clients guessed
          correctly,followed by how many IP addresses connected.
          
          Ip_stored_already will take in socket information and check if the passed IP address has been stored already or not.
          
          randomPassword takes in the desired length and randomly makes a password of that length from the allowed character list
          
          dieWithMessage will take in a string, then display that string. After display it will exit the program.

*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>
#include <signal.h>

static const char* options = "abcdefghijklmnopqrstuvwxyz1234`~!@#$\%^&*()_-+={}[]|:;'<>,.?/\\\"";
long messages_recieved = 0;   //Making globals to work with the signal handler
int correct_guesses = 0;

typedef struct node
{
    struct sockaddr_storage ips;
    struct node *next;
} NODE;
NODE *first;

void  ExitHandler(int sig)
{
     signal(sig, SIG_IGN);

     printf("\nThere were %lu total guesses\n%d clients guessed correctly!\n", messages_recieved, correct_guesses);
     printf("Connected IP addresses were: \n");
     
     //Printing out IP addresses
     NODE *curr;
     if(first->next != NULL)
    {
      for(curr = first->next; curr != NULL; curr = curr->next)
      {
          struct sockaddr_in *sin = (struct sockaddr_in *)&curr->ips;
          unsigned char *Address = (unsigned char *)&sin->sin_addr.s_addr;
          fprintf(stderr, "%d.%d.%d.%d\n", Address[0], Address[1], Address[2], Address[3]);
      }
   }
     
     exit(0);    
}

bool IP_stored_already(struct sockaddr_storage socket1, struct sockaddr_storage socket2)
{
   bool check = false;
   struct sockaddr_in *sin = (struct sockaddr_in *)&socket1;
   unsigned char *ip1 = (unsigned char *)&sin->sin_addr.s_addr;
   sin = (struct sockaddr_in *)&socket2;
   unsigned char *ip2 = (unsigned char *)&sin->sin_addr.s_addr;
   if(ip1[0] == ip2[0] && ip1[1] == ip2[1] && ip1[2] == ip2[2] && ip1[3] == ip2[3]){check = true;}
   return check;
}

char *randomPassword(int length) 
{    
    
    size_t stringLen = 62;        
    char *randomString;

    randomString = malloc(sizeof(char) * (length +1));

    if (!randomString) {
        return (char*)0;
    }
    srand (time(NULL));  
    unsigned int key = 0;
    int n = 0;
    for (n = 0;n < length;n++) { 
                 
        key = rand() % stringLen;          
        randomString[n] = options[key];
    }

    randomString[length] = '\0';
   
    return randomString;
}

void dieWithMessage(char *s)
{
    perror(s);
    exit(1);
}


int main(int argc, char *argv[]) {

  signal(SIGINT, ExitHandler);
  first = malloc(sizeof(NODE));
  if (argc < 3) // Test for correct number of arguments
    dieWithMessage("Too few arguments, needs serverPort, password length, and optional initial password: ");

  

  int length = atoi(argv[2]);       //returns 0 if cannot be converted to an int
  while(length < 1 || length > 8)
  {
    printf("Length invalid, please enter a number between 1 and 8: ");
    scanf("%d", &length);
  }

  char *initialPass;

  if(argc == 4)
  {
    initialPass = argv[3];
    int i = 0;
    int legal = 1;
    while (legal == 1)
    {

      for(i = 0; i < strlen(initialPass); i++)
      {
        int checker = 0;
        int j = 0;
        for(j = 0; j < strlen(options); j++)
        {
          if(initialPass[i] == options[j])
          {
            checker = 1;
            break;
          }
        }
        if(checker == 0)
        {
          legal = 0;
          break;
        }
      }

      if(legal == 0)
      {

        printf("Initial Password contained invalid characters, please enter a new password: ");
        scanf("%s", initialPass);
        legal = 1;
      }
      else if(legal == 1 && strlen(initialPass) != length)
      {
        printf("Password does not match requested length of %d, please enter a new password of the correct length: ", length);
        scanf("%s", initialPass);
        legal = 1;
      }
      else
      {
        legal = 0;
      }
    }
  }           
  else
  {
    initialPass = randomPassword(length);
  }
  
  char *originalservice = argv[1]; // First arg:  local port/service
  int port = atoi(originalservice);

  while(port < 5000 || port > 10000)
  {
    printf("port invalid, please choose a number between 5000 and 10000: ");
    scanf("%d", &port);
  }
  char service[6];
  sprintf(service, "%d", port);

  
  // Construct the server address structure
  struct addrinfo addrCriteria;                   // Criteria for address
  memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
  addrCriteria.ai_family = AF_UNSPEC;             // Any address family
  addrCriteria.ai_flags = AI_PASSIVE;             // Accept on any address/port
  addrCriteria.ai_socktype = SOCK_DGRAM;          // Only datagram socket
  addrCriteria.ai_protocol = IPPROTO_UDP;         // Only UDP socket

  struct addrinfo *servAddr; // List of server addresses
  int rtnVal = getaddrinfo(NULL, service, &addrCriteria, &servAddr);
  if (rtnVal != 0)
    dieWithMessage("getaddrinfo() failed");

  // Create socket for incoming connections
  int sock = socket(servAddr->ai_family, servAddr->ai_socktype,
      servAddr->ai_protocol);
  if (sock < 0)
    dieWithMessage("socket() failed");

  // Bind to the local address
  if (bind(sock, servAddr->ai_addr, servAddr->ai_addrlen) < 0)
    dieWithMessage("bind() failed");

  // Free address list allocated by getaddrinfo()
  freeaddrinfo(servAddr);
  
  for (;;) { // Run forever
    struct sockaddr_storage clntAddr; // Client address
    // Set Length of client address structure (in-out parameter)
    socklen_t clntAddrLen = sizeof(clntAddr);

    // Block until recieve message from a client
    char message[length + 1]; // I/O buffer
    // Size of recieved message
    int numBytesRcvd = recvfrom(sock, message, length, 0, (struct sockaddr *) &clntAddr, &clntAddrLen);
    if (numBytesRcvd < 0)
      dieWithMessage("recvfrom() failed");
    
    bool storedIP = false;
    NODE *new = malloc(sizeof(NODE));
    new->ips = clntAddr;
    if(first->next == NULL)
    {
        first->next = new;
        new->next = NULL;
        
    }
    else
    {
        NODE *current;
        current = first;
        bool currentNew = true;
        while(current->next != NULL && storedIP == false)
        {
            if(!currentNew)
            {
                storedIP = IP_stored_already(new->ips, current->ips);
            }
            current = current->next;
            currentNew = false;
        } 
         storedIP = IP_stored_already(new->ips, current->ips);
         if(!storedIP)
         {
            current->next = new;
            new->next = NULL;
         }
         storedIP = false;
    }
             
    message[length] = '\0';
    
    messages_recieved += 1;
    if(strcmp(initialPass, message) == 0)
    {
      correct_guesses += 1;
      printf("Password Successfully guessed! Generating new password.\n");
      initialPass = randomPassword(length);
      char *recieve = "0";
      ssize_t BytesSent = sendto(sock, recieve, strlen(recieve), 0, (struct sockaddr *) &clntAddr, sizeof(clntAddr));
      if (BytesSent < 0)
      {
        dieWithMessage("sendto() failed)");
      }
      else if (BytesSent != strlen(recieve))
      {
        dieWithMessage("sendto() sent unexpected number of bytes");
      } 
      
    }
    else
    {
      char *recieve = "1";
      ssize_t BytesSent = sendto(sock, recieve, strlen(recieve), 0, (struct sockaddr *) &clntAddr, sizeof(clntAddr));
      if (BytesSent < 0)
      {
        dieWithMessage("sendto() failed)");
      }
      else if (BytesSent != strlen(recieve))
      {
        dieWithMessage("sendto() sent unexpected number of bytes");
      } 
    }
  }
}

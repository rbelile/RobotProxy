/*
I tested processResponse and it was working. Needs the buffer to be passed to it and the type to be passed to it (GPS, etc)
The formatting or protocol that it uses is [response: 1]/r/n[type]/r/n[length]/r/n[data]

Haven't tested sendResponse but I am pretty sure it works. If not it should be an easy fix.
Needs the buffer, the udp client socket, and the client address.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>     
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netdb.h>      
#include <stdlib.h>    
#include <unistd.h>     
#include <ctype.h>

#include "methods.h"

char* processResponse(char *buffer, char *type, char* resp);
void sendResponse(char *buffer, int sock, struct sockaddr_in ClntAddr);

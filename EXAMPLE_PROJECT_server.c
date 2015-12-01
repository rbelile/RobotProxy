#include "methods.h"
#include "getRequest.h"
#include "processRequest.h"
#include "sendRequest.c"
#include "response.h"

int main(){
  char               buffer[50];
  char               udpresponse[1024];
  char               response[1024];
  char               request[1024];
  struct sockaddr_in servAddr;
  struct sockaddr_in clntAddr;
  int                sock;
  int                recvMsgSize;
  unsigned short     port = 5000;
  unsigned short     roport;
 

  
  if((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    err_n_die("socket() failed\n");
  memset(&servAddr, 0, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(port);
  if(bind(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
    err_n_die("bind() failed\n");

  while(1){
    memset(buffer,0,50);	
    memset(response,0,1024);
    memset(request,0,1024);
    getRequest(buffer, &clntAddr, sock);
    roport   = processRequest(request, buffer);
    printf("request = %s\n",request);
    sendRequest(request,roport,response);
    processResponse(response, buffer,udpresponse);
    sendResponse(udpresponse, sock, clntAddr);
  }
}

// shared.h
// CPSC 360 Final Project

#ifndef SHARED_H
#define SHARED_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>     
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netdb.h>         
#include <unistd.h>     
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <sys/time.h> 
#include <fcntl.h>
#include <sys/stat.h>

#define BUFFMAX 2048   
#define LISTENQ         (1024)
#define SERVERPORT		(8080)
#define MAX_REQ_LINE    (1024)
#define N 6

/*  User-defined data types  */

enum Req_Method {
	GET, HEAD, UNSUPPORTED
};
enum Req_Type {
	SIMPLE, FULL
};

struct ReqInfo {
	enum Req_Method method;
	enum Req_Type type;
	char *referer;
	char *useragent;
	char *resource;
	int status;
};

/* Function Prototypes */
int Service_Request(int conn);
void printTime();
int Parse_HTTP_Header(char * buffer, struct ReqInfo * reqinfo);
int Get_Request(int conn, struct ReqInfo * reqinfo);
void InitReqInfo(struct ReqInfo * reqinfo);
void FreeReqInfo(struct ReqInfo * reqinfo);
int Output_HTTP_Headers(int conn, struct ReqInfo * reqinfo);
int Return_Resource(int conn, int resource, struct ReqInfo * reqinfo);
int Check_Resource(struct ReqInfo * reqinfo);
int Return_Error_Msg(int conn, struct ReqInfo * reqinfo);
void Error_Quit(char const * msg);
int Trim(char * buffer);
int StrUpper(char * buffer);
void CleanURL(char * buffer);
ssize_t Readline(int sockd, void *vptr, size_t maxlen);
ssize_t Writeline(int sockd, const void *vptr, size_t n);

void clientCNTCCode() {
	exit(0);
}

void DieWithError(char *errorMessage) {
	perror(errorMessage);
	exit(1);
}

#endif

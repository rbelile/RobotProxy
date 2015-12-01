//methods.c
#include "methods.h"

void err_n_die(const char* msg){
	perror(msg);
	exit(1);
}
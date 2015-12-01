#include "client.h"

void processResponse(char *response) {
     char buffer[MAX];
     char* reader = NULL;
     int i, length;


     reader = strstr(response, "GPS");
     //GPS response
     if(reader != NULL) {
           reader = strstr(response, "DGPS");
           //DGPS response
           if(reader != NULL) {
                 printf("DGPS:\n");
                 reader = reader + 4;
                 length = reader[0];
                 for(i = 0; i < length; i++) {
                                sprintf(buffer, "%s%c", buffer, reader[i]);
                 }
                 printf("%s\n", buffer);
           }
           else {
                 reader = strstr(response, "GPS");
                 printf("GPS:\n");
                 reader = reader + 3;
                 length = reader[0];
                 for(i = 0; i < length; i++) {
                       sprintf(buffer, "%s%c", buffer, reader[i]);
                 }
           }
     }
     else {
           printf("invalid response");
     }
}

#include "response.h"

char* processResponse(char *buffer, char *type, char*resp)
{
	char *i;
	int data = 0;
	int length = 0;
	char *databuf;
	int k = 0;
	char buf[1024];
	int j = 0;
	for(i=buffer; *i; i++)
	{
		if(data == 0)
		{
			if(*i != '\n')
			{
				buf[j] = *i;
				j++;
			}
			else
			{
				buf[j] = *i;
				int n = strlen(buf);
				while(!isalnum(buf[n]) && n >= 0)
					buf[n--] = '\0';
				if(buf[0] == '\0')
					data = 1;
				int l;
				char *e;
				e = strchr(buf,':');
				if(e!=NULL)
					l = (int)(e-buf);
				else
					l = strlen(buf);
				char copy[1024] = {0};
				strncpy(copy,buf,l);
				copy[l] = '\0';
				if(!strcmp(copy,"Content-Length"))
				{
					char *tbuf = buf;
					char len[40] = {0};
					tbuf  += l + 2;
					strcpy(len, tbuf);
					length = atoi(len);
					databuf = (char *)malloc((length+1)*sizeof(char)+1);
				}
				j = 0;
				memset(&buf[0],'\0',sizeof(buf));
			}
		}
		else
		{
			databuf[k] = *i;
			k++;
		}
	}
	databuf[k] = '\0';

	sprintf(resp, "%s\r\n%d\r\n%s",type,length,databuf);
	free(databuf);
printf("%s\n",resp);
	return resp;
}

void sendResponse(char *buffer, int sock, struct sockaddr_in ClntAddr)
{
	if(sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *) &ClntAddr, sizeof(ClntAddr)))
		err_n_die("sendto() failed");
}

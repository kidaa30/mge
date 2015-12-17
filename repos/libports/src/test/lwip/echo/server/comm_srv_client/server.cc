/* libc includes */


#include <base/env.h>
#include <base/printf.h>
#include <comm_server_session/client.h>
#include <comm_server_session/connection.h>

#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>



#define TRUE 1


void read_msg(int file)
{
	char line[128];
	FILE *f ;
	f = fdopen(file,"r+");
	if(f==NULL)
	{
		printf("error in file\n");
		perror("dopen");
		fclose(f);
	}
	int c ;
	while((c= fgetc(f))){
		printf("%c",c); 
		printf("%d",c); 
		if (c=='\n') {
			char * t = "get the text \n";
			int r =write(file,t,strlen(t));
			if (r <0)
				printf("ERROR: write!\n");
		}
		else 
		if (c ==-1){
				fclose(f);
				return ;
		}
	}
	
}


int main()
{
	commserversession::Connection lwip_server;
	commserversession::Session::RLS accept_r; 
	struct sockaddr_in in_addr;
	int Sport =7777;
	char buff[128];
	int s,msgsock,addrlen ; 
	
	s = lwip_server.comm_srv_socket(AF_INET, SOCK_STREAM, 0);
	if (s == -1) {
		printf("ERROR: Could not create socket!\n");
		return -1;
	}
	printf("Create new socket..\n");

	in_addr.sin_port = htons(Sport);
	in_addr.sin_family = AF_INET;
	in_addr.sin_addr.s_addr = INADDR_ANY;
	addrlen = sizeof (in_addr);
	commserversession::Session::Buffer struct_in ; 
	unsigned char * data= (unsigned char *) &in_addr; 
	int i = 0 ; 
	while (data and i< addrlen)
		struct_in.buff[i++] = *data++;

	if (  lwip_server.comm_srv_bind(s,struct_in, addrlen) == -1) {
		printf("ERROR: Could not bind!\n");
		lwip_server.comm_srv_close(s);
		return -1;
	}

	printf("Bind socket to %d\n", Sport);
	if ( lwip_server.comm_srv_listen(s, 5) == -1) {
		printf("ERROR: Could not listen!\n");
		lwip_server.comm_srv_close(s);
		return -1;
	}
	printf("wait...\n");
	while (1)
	{
		int x = 0 ;
	
		accept_r =lwip_server.comm_srv_accept(s,x);
		msgsock = accept_r.return_value;
		if (msgsock==-1)
			printf("ERROR: accept!\n");
		else
			{
				memset(buff,0,sizeof(buff));
				//printf("writing \n");
				char * txt = " from srever 7777: hi there\n";
				int r =lwip_server.comm_srv_write(msgsock,txt,strlen(txt));
				if (r <0)
					printf("ERROR: write!\n");
				memset(buff,0,128);
				int re = lwip_server.comm_srv_read(msgsock,buff,128);
				if (re < 0)
					printf("ERROR: read!\n");
				else 
					printf("%s",buff);
				lwip_server.comm_srv_close(msgsock);
			}
		
	}// while 
}



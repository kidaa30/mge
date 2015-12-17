
/*
 * \brief  echo_server
 * \author Mohammad Hamad
 * \date   2015-10-21
 *
 */


/*
* echo sever:
*  it listens to the port 7575 and keep printing the coming message from the client.
*/
#include <stdio.h>
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


#include <base/printf.h>

#define LENGTH 100
#define TRUE 1

void read_msg(int file);
int main(int argc , char** argv)
{

	
	int Sport = atoi(argv[1]) ; 
	
	PDBG("start on port %s\n",argv[1]);
	
	int s,msgsock,rval;
	struct sockaddr_in in_addr;
	char rbuf[100];
	
	PDBG("Create new socket..\n");
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == -1) {
		PDBG("ERROR: Could not create socket!\n");
		return -1;
	}


	in_addr.sin_port = htons(Sport);
	in_addr.sin_family = AF_INET;
	in_addr.sin_addr.s_addr = INADDR_ANY;
	if (  bind(s, (struct sockaddr *)&in_addr, sizeof (in_addr)) == -1) {
		PDBG("ERROR: Could not bind!\n");
		close(s);
		return -1;
	}
	if ( listen(s, 5) == -1) {
		PDBG("ERROR: Could not listen!\n");
		close(s);
		return -1;
	}
	PDBG("waiting for client...\n");
	
	while (TRUE)
	{
		int l = 0 ; 
		struct sockaddr_in in_addr_1;
		msgsock =accept(s,(struct sockaddr *)&in_addr_1,(socklen_t *)&l);
		if (msgsock==-1)
			PDBG("ERROR: accept!\n");
		else
			{
				
				char * txt = " from srever  : hi there \n";
				int w=write(msgsock,txt,strlen(txt));
				if (w <0)
					PDBG("ERROR: write!\n");
				int readed = 0;
				int remein=LENGTH;
				int r ; 
				memset(rbuf, 0,LENGTH);
				while(readed < remein ){
					
					r = read(msgsock,rbuf,remein);
					if (s == -1) {
						PDBG("ERROR: while reading!\n");
						return -1;
					}
					/* end of the file */
					if (r==0 )
						break;
					readed+=r;
					remein-=r;
					printf("%s", rbuf);
					memset(rbuf, 0,r);
				}
				
			}
		close(msgsock);
		
	}
}


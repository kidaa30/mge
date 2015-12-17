/* libc includes */
#include <stdint.h>

#ifdef LWIP_NATIVE
#include <lwip/sockets.h>
#define accept(a,b,c)         lwip_accept(a,b,c)
#define bind(a,b,c)           lwip_bind(a,b,c)
#define close(s)              lwip_close(s)
#define connect(a,b,c)        lwip_connect(a,b,c)
#define listen(a,b)           lwip_listen(a,b)
#define recv(a,b,c,d)         lwip_recv(a,b,c,d)
#define select(a,b,c,d,e)     lwip_select(a,b,c,d,e)
#define send(a,b,c,d)         lwip_send(a,b,c,d)
#define socket(a,b,c)         lwip_socket(a,b,c)
#else
/* libc includes */
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#endif

#include <netdb.h>
#include <stdio.h>
#include <sys/fcntl.h>

#define TRUE 1
/*
* This program uses select() to check that someone is trying to connect
* before calling accept().
*/

int sendmsg_to_client ( int s, char * ms)
{
	struct msghdr msg; 
	struct iovec iov; 
	int sl ; 
	
	iov.iov_base = ms;
	iov.iov_len = strlen(ms)+1;


	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;
	
	sl = sendmsg(s, &msg,0);
	
	return sl; 
}
main()
{
	char * message = " hello to my server please enter message ";


	char address[]= "192.168.12.211";
	int sock, length;
	struct sockaddr_in server;
	int msgsock;
	char buf[1024];
	int rval;
	fd_set ready;
	struct timeval to;
	/* Create socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("opening stream socket");
		exit(1);
	}
	int one =1;
	if(fcntl(sock, F_SETFL, O_NONBLOCK)<0){
		perror("ioctl stream socket");
		exit(1);
	}
		; 
	/* Name socket using wildcards */
	server.sin_family = AF_INET;
	//server.sin_addr.s_addr = INADDR_ANY;
	server.sin_addr.s_addr = inet_addr(address);
	server.sin_port =htons( 3333);
	if (bind(sock, &server, sizeof(server))) {
		perror("binding stream socket");
		exit(1);
	}

	/* Find out assigned port number and print it out */
	/*length = sizeof(server);
	if (getsockname(sock, &server, &length)) {
		perror("getting socket name");
		exit(1);
	}*/

	printf("Socket has port #%d\n", ntohs(server.sin_port));
	/* Start accepting connections */
	listen(sock, 5);

	do {
		FD_ZERO(&ready);
		FD_SET(sock, &ready);
		to.tv_sec = 5;
		int r = select(sock + 1, &ready, 0, 0, &to);
		printf(" echo server :r is %d \n", r );
		if (r < 0) {
			perror("select");
			continue;
		}
		printf("after select\n");
		if (FD_ISSET(sock, &ready)) {
			msgsock = accept(sock, (struct sockaddr *)0, (int *)0);
			if (msgsock == -1)
				perror("accept");
			else do {
					//int sndmsgl = sendmsg_to_client(msgsock,message);
					//if (sndmsgl< 0)
					//	printf("error in  sendmsg\n");
					bzero(buf, sizeof(buf));
					if ((rval = read(msgsock, buf, 1024)) < 0)
						perror("reading stream message");
					else if (rval == 2)
						printf("Ending connection\n");
					else
						printf("%d-->%s\n",rval, buf);
			} while (rval > 2);
		close(msgsock);
		} else
			printf("Do something else\n");
	} while (TRUE);
}

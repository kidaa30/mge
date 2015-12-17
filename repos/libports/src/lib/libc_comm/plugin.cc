
/**********************
 ** rpc **
 **********************/

#include <comm_server_session/client.h>
#include <comm_server_session/connection.h>


#include <base/env.h>
#include <base/printf.h>


#define LWIP_COMPAT_SOCKETS 0



#include <sys/socket.h>
#include <netinet/in.h>  
#include <arpa/inet.h>


/**********************
 ** Plugin interface **
 **********************/

#include <libc-plugin/fd_alloc.h>
#include <libc-plugin/plugin_registry.h>

#include <assert.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>


/* the requered marco for the lwip_select 
 * we define lwip_fd_set, lwip_FD_ZERO,lwip_FD_SET,lwip_FD_SET
*/

#include <stdio.h>
#include <string.h>


#ifndef MEMP_NUM_NETCONN
#define MEMP_NUM_NETCONN                143
#endif


/* FD_SET used for lwip_select */
#ifndef lwip_FD_SET
#undef  lwip_FD_SETSIZE
/* Make FD_SETSIZE match NUM_SOCKETS in socket.c */
#define lwip_FD_SETSIZE    MEMP_NUM_NETCONN
#define lwip_FD_SET(n, p)  ((p)->fd_bits[(n)/8] |=  (1 << ((n) & 7)))
#define lwip_FD_CLR(n, p)  ((p)->fd_bits[(n)/8] &= ~(1 << ((n) & 7)))
#define lwip_FD_ISSET(n,p) ((p)->fd_bits[(n)/8] &   (1 << ((n) & 7)))
#define lwip_FD_ZERO(p)    memset((void*)(p),0,sizeof(*(p)))

  typedef struct lwip_fd_set {
          unsigned char fd_bits [(lwip_FD_SETSIZE+7)/8];
        } lwip_fd_set;


#endif /* LWIP_FD_SET */
  

namespace {


/**
 * For the lwip plugin, we only use the lwip file descriptor as context
 */
class Plugin_context : public Libc::Plugin_context
{
	private:

		int _lwip_fd;

	public:

		/**
		 * Constructor
		 *
		 * \param lwip_fd  file descriptor used for interacting with the
		 *                 socket API of lwip
		 */
		Plugin_context(int lwip_fd) : _lwip_fd(lwip_fd) { }

		int lwip_fd() const { return _lwip_fd; }
};


static inline Plugin_context *context(Libc::File_descriptor *fd)
{
	return static_cast<Plugin_context *>(fd->context);
}


static inline int get_lwip_fd(Libc::File_descriptor *sockfdo)
{
	return context(sockfdo)->lwip_fd();
}


struct Plugin : Libc::Plugin
{
	/**
	 * Constructor
	 */
	Plugin();
	
	//
	commserversession::Connection comm_server_connection;

	bool supports_freeaddrinfo(struct ::addrinfo *res);
	bool supports_getaddrinfo(const char *node, const char *service,
	                          const struct ::addrinfo *hints,
	                          struct ::addrinfo **res);
	bool supports_select(int nfds,
	                     fd_set *readfds,
	                     fd_set *writefds,
	                     fd_set *exceptfds,
	                     struct timeval *timeout);
	bool supports_socket(int domain, int type, int protocol);

	Libc::File_descriptor *accept(Libc::File_descriptor *sockfdo,
	                              struct sockaddr *addr,
	                              socklen_t *addrlen);
	int bind(Libc::File_descriptor *sockfdo,
	         const struct sockaddr *addr,
	         socklen_t addrlen);
	int close(Libc::File_descriptor *fdo);
	
	int connect(Libc::File_descriptor *sockfdo,
	            const struct sockaddr *addr,
	            socklen_t addrlen);
	int fcntl(Libc::File_descriptor *sockfdo, int cmd, long val);
	
	void freeaddrinfo(struct ::addrinfo *res);
	int getaddrinfo(const char *node, const char *service,
	                const struct ::addrinfo *hints,
	                struct ::addrinfo **res);
	
	int getpeername(Libc::File_descriptor *sockfdo,
	                struct sockaddr *addr,
	                socklen_t *addrlen);
	int getsockname(Libc::File_descriptor *sockfdo,
	                struct sockaddr *addr,
	                socklen_t *addrlen);
	int getsockopt(Libc::File_descriptor *sockfdo, int level,
	               int optname, void *optval,
	               socklen_t *optlen);
	
	int ioctl(Libc::File_descriptor *sockfdo, int request, char *argp);
	int listen(Libc::File_descriptor *sockfdo, int backlog);
	
	ssize_t read(Libc::File_descriptor *fdo, void *buf, ::size_t count);
	int shutdown(Libc::File_descriptor *fdo, int);
	
	int select(int nfds, fd_set *readfds, fd_set *writefds,
	          fd_set *exceptfds, struct timeval *timeout);
	
	ssize_t send(Libc::File_descriptor *, const void *buf, ::size_t len, int flags);

	ssize_t sendto(Libc::File_descriptor *, const void *buf,
	               ::size_t len, int flags,
	               const struct sockaddr *dest_addr,
	               socklen_t addrlen);
	
	ssize_t recv(Libc::File_descriptor *, void *buf, ::size_t len, int flags);
		
	ssize_t recvfrom(Libc::File_descriptor *, void *buf, ::size_t len, int flags,
	                 struct sockaddr *src_addr, socklen_t *addrlen);
	int setsockopt(Libc::File_descriptor *sockfdo, int level,
	               int optname, const void *optval,
	               socklen_t optlen);
	
	Libc::File_descriptor *socket(int domain, int type, int protocol);
	ssize_t write(Libc::File_descriptor *fdo, const void *buf, ::size_t count);

};



Plugin::Plugin()
{
	PDBG("using libc_comm plugin\n");
}


bool Plugin::supports_freeaddrinfo(struct ::addrinfo *)
{
	return true;
}


bool Plugin::supports_getaddrinfo(const char *, const char *,
                                       const struct ::addrinfo *,
                                       struct ::addrinfo **)
{
	return true;
}


bool Plugin::supports_select(int nfds,
                                  fd_set *readfds,
                                  fd_set *writefds,
                                  fd_set *exceptfds,
                                  struct timeval *timeout)
{
	Libc::File_descriptor *fdo;
	/* return true if any file descriptor which is marked set in one of
	 * the sets belongs to this plugin */
	for (int libc_fd = 0; libc_fd < nfds; libc_fd++) {
		if (FD_ISSET(libc_fd, readfds)
		 || FD_ISSET(libc_fd, writefds) || FD_ISSET(libc_fd, exceptfds)) {
			fdo = Libc::file_descriptor_allocator()->find_by_libc_fd(libc_fd);
			if (fdo && (fdo->plugin == this)) {
				return true;
			}
		}
	}
	return false;
}


bool Plugin::supports_socket(int domain, int, int)
{
	if (domain == AF_INET)
		return true;

	return false;
}


Libc::File_descriptor *Plugin::accept(Libc::File_descriptor *sockfdo,
                                           struct sockaddr *addr, socklen_t *addrlen)
{
	commserversession::Session::RLS accept_result;
	/* if we ask to return the saddress */
	int add_returned =((!addr)||(!addrlen)||( *addrlen==0))?0:1;
	accept_result = comm_server_connection.comm_srv_accept(get_lwip_fd(sockfdo),*addrlen);
	if(accept_result.return_value == -1) {
		return 0;
	}
	
	Plugin_context *context = new (Genode::env()->heap()) Plugin_context(accept_result.return_value);
	Libc::File_descriptor *fd = Libc::file_descriptor_allocator()->alloc(this, context);

	if (!fd)
		PERR("could not allocate file descriptor");
	
	if (add_returned ==1){
		*addrlen =accept_result.length;
		memcpy(addr, accept_result.buff,accept_result.length);
	}
	return fd;
	/*
	int l = *addrlen;
	//PDBG(" start accept\n");
	int lwip_fd = comm_server_connection.comm_srv_accept(get_lwip_fd(sockfdo),l);
	if (lwip_fd == -1) {
		return 0;
	}
	
	Plugin_context *context = new (Genode::env()->heap()) Plugin_context(lwip_fd);
	Libc::File_descriptor *fd = Libc::file_descriptor_allocator()->alloc(this, context);

	if (!fd)
		PERR("could not allocate file descriptor");
	//PDBG(" done accept\n");
	
	return fd;
	*/
}

int Plugin::bind(Libc::File_descriptor *sockfdo, const struct sockaddr *addr,
                      socklen_t addrlen)
{
	commserversession::Session::Buffer local_address ; 
	unsigned char * data= (unsigned char *) addr; 
	memcpy(local_address.buff,addr,addrlen);
	return comm_server_connection.comm_srv_bind(get_lwip_fd(sockfdo),local_address,addrlen);
}


int Plugin::connect(Libc::File_descriptor *sockfdo,
                         const struct sockaddr *addr,
                         socklen_t addrlen)
{
	commserversession::Session::Buffer destination_address ; 
	unsigned char * data= (unsigned char *) addr;
	memcpy(destination_address.buff,addr,addrlen);
	return comm_server_connection.comm_srv_connect(get_lwip_fd(sockfdo),destination_address,addrlen);
}

int Plugin::close(Libc::File_descriptor *fdo)
{
	int result = comm_server_connection.comm_srv_close(get_lwip_fd(fdo));
	if (context(fdo))
		Genode::destroy(Genode::env()->heap(), context(fdo));
	Libc::file_descriptor_allocator()->free(fdo);
	return result;
}


int Plugin::fcntl(Libc::File_descriptor *sockfdo, int cmd, long val)
{
	
	int s = get_lwip_fd(sockfdo);
	int result = -1;
	switch (cmd) {
	case F_GETFL:
	case F_SETFL:
		result = comm_server_connection.comm_srv_fcntl(s, cmd, val);
		break;
	default:
		PERR("unsupported fcntl() request: %d", cmd);
		break;
	}
	//PINF(" result  %d\n",result);
	return result;
}
extern "C" void libc_freeaddrinfo(struct ::addrinfo *);

void Plugin::freeaddrinfo(struct ::addrinfo *res)
{
	return ::libc_freeaddrinfo(res);
}


extern "C" int libc_getaddrinfo(const char *, const char *,
                                const struct ::addrinfo *,
                                struct ::addrinfo **);

int Plugin::getaddrinfo(const char *node, const char *service,
                             const struct ::addrinfo *hints,
                             struct ::addrinfo **res)
{
	return ::libc_getaddrinfo(node, service, hints, res);
}


int Plugin::getpeername(Libc::File_descriptor *sockfdo,
                             struct sockaddr *addr,
                             socklen_t *addrlen)
{
	
	commserversession::Session::RLS getpeername_result;
	getpeername_result = comm_server_connection.comm_srv_getpeername(get_lwip_fd(sockfdo),*addrlen );
	memset(addr,0,*addrlen); 
	memcpy(addr, getpeername_result.buff,getpeername_result.length);
	*addrlen = getpeername_result.length;
	return getpeername_result.return_value;
}


int Plugin::getsockname(Libc::File_descriptor *sockfdo,
                             struct sockaddr *addr,
                             socklen_t *addrlen)
{
	commserversession::Session::RLS getsockname_result;
	getsockname_result = comm_server_connection.comm_srv_getsockname(get_lwip_fd(sockfdo),*addrlen);
	memset(addr,0,*addrlen); 	
	memcpy(addr, getsockname_result.buff,getsockname_result.length);
	*addrlen = getsockname_result.length;
	return  getsockname_result.return_value;
}


int Plugin::getsockopt(Libc::File_descriptor *sockfdo, int level,
                            int optname, void *optval, socklen_t *optlen)
{

	commserversession::Session::RLS getsockopt_result;
	getsockopt_result = comm_server_connection.comm_srv_getsockopt(get_lwip_fd(sockfdo), level, optname, *optlen);
	*optlen =getsockopt_result.length;
	memcpy(optval, getsockopt_result.buff,getsockopt_result.length);
	return  getsockopt_result.return_value;
}

int Plugin::ioctl(Libc::File_descriptor *sockfdo, int request, char *argp)
{
	 commserversession::Session::RLS ioctl_result;
	 commserversession::Session::Buffer argbuf;
        
	switch (request) {
		case FIONBIO:
			 memcpy(argbuf.buff, argp, sizeof(int ));
			ioctl_result =comm_server_connection.comm_srv_ioctl(get_lwip_fd(sockfdo), FIONBIO,argbuf);
			return ioctl_result.return_value;
			break;
		case FIONREAD:
			ioctl_result = comm_server_connection.comm_srv_ioctl(get_lwip_fd(sockfdo), FIONREAD,argbuf);
			memcpy(argp, ioctl_result.buff, sizeof(int ));
			return ioctl_result.return_value;
			break;
		default:
			PERR("unsupported ioctl() request");
			//errno = ENOSYS;
			return -1;
	}
}


int Plugin::listen(Libc::File_descriptor *sockfdo, int backlog)
{
	return comm_server_connection.comm_srv_listen(get_lwip_fd(sockfdo), backlog);
}

ssize_t Plugin::read(Libc::File_descriptor *fdo, void *buf, ::size_t count)
{	
	int re = comm_server_connection.comm_srv_read(get_lwip_fd(fdo),buf,count);
	return re; 
	
}

ssize_t Plugin::recv(Libc::File_descriptor *sockfdo, void *buf, ::size_t len, int flags)
{
	int re = comm_server_connection.comm_srv_recv(get_lwip_fd(sockfdo),buf,len,flags);
	//PDBG("Recv  %s\n",buf);
	return re;  
	
}

ssize_t Plugin::recvfrom(Libc::File_descriptor *sockfdo, void *buf, ::size_t len, int flags,
                         struct sockaddr *from, socklen_t *addrlen)
{
	commserversession::Session::RLS recvfrom_result;
	int intrest = ((!from)||(!addrlen)||( *addrlen==0))?0:1;
	recvfrom_result =comm_server_connection.comm_srv_recvfrom(get_lwip_fd(sockfdo),buf,len,flags,*addrlen); 
	if (intrest==1) {
		memcpy(from, recvfrom_result.buff,recvfrom_result.length);
		*addrlen = recvfrom_result.length;
	}
	return  recvfrom_result.return_value;
		
}


int Plugin::select(int nfds,
                        fd_set *readfds,
                        fd_set *writefds,
                        fd_set *exceptfds,
                        struct timeval *timeout)
{

	commserversession::Session::RPC_FD_SET rpc_fs_in,rpc_fs_out;
	commserversession::Session::Buffer time_buf;
	Libc::File_descriptor *fdo;
	lwip_fd_set lwip_readfds;
	lwip_fd_set lwip_writefds;
	lwip_fd_set lwip_exceptfds;
	int libc_fd;
	int lwip_fd;
	int highest_lwip_fd = -2;
	int result;

	lwip_FD_ZERO(&lwip_readfds);
	lwip_FD_ZERO(&lwip_writefds);
	lwip_FD_ZERO(&lwip_exceptfds);

	for (libc_fd = 0; libc_fd < nfds; libc_fd++) {

		fdo = Libc::file_descriptor_allocator()->find_by_libc_fd(libc_fd);

		// handle only libc_fds that belong to this plugin 
		if (!fdo || (fdo->plugin != this))
			continue;

		if (FD_ISSET(libc_fd, readfds) ||
			FD_ISSET(libc_fd, writefds) ||
			FD_ISSET(libc_fd, exceptfds)) {

			lwip_fd = get_lwip_fd(fdo);

			if (lwip_fd > highest_lwip_fd) {
				highest_lwip_fd = lwip_fd;
			}

			if (FD_ISSET(libc_fd, readfds)) {
				lwip_FD_SET(lwip_fd, &lwip_readfds);
			}

			if (FD_ISSET(libc_fd, writefds)) {
				lwip_FD_SET(lwip_fd, &lwip_writefds);
			}

			if (FD_ISSET(libc_fd, exceptfds)) {
				lwip_FD_SET(lwip_fd, &lwip_exceptfds);
			}
		}
	}

	//copy from struct to rpc_fs_in
	rpc_fs_in.result = highest_lwip_fd + 1;
	memcpy(rpc_fs_in.rbuff, &lwip_readfds  ,sizeof(lwip_fd_set ));
	memcpy(rpc_fs_in.wbuff, &lwip_writefds ,sizeof(lwip_fd_set ));
	memcpy(rpc_fs_in.ebuff, &lwip_exceptfds,sizeof(lwip_fd_set ));
	memcpy(time_buf.buff, timeout      ,sizeof(struct timeval));


	rpc_fs_out = comm_server_connection.comm_srv_select(rpc_fs_in,time_buf);

	// copy from rpc_fs_out to the struct 
	result = rpc_fs_out.result;
	memcpy(&lwip_readfds  ,rpc_fs_out.rbuff, sizeof(lwip_fd_set ));
	memcpy(&lwip_writefds ,rpc_fs_out.wbuff, sizeof(lwip_fd_set ));
	memcpy(&lwip_exceptfds,rpc_fs_out.ebuff, sizeof(lwip_fd_set ));
	


	

	if (result > 0) {

		/* clear result sets */
		FD_ZERO(readfds);
		FD_ZERO(writefds);
		FD_ZERO(exceptfds);

		for (lwip_fd = 0; lwip_fd <= highest_lwip_fd; lwip_fd++) {
			/* find an lwip_fd which is in the set */

			if (lwip_FD_ISSET(lwip_fd, &lwip_readfds) ||
				lwip_FD_ISSET(lwip_fd, &lwip_writefds) ||
				lwip_FD_ISSET(lwip_fd, &lwip_exceptfds)) {

				/* find matching libc_fd for lwip_fd */
				for (libc_fd = 0; libc_fd < nfds; libc_fd++) {

					fdo = Libc::file_descriptor_allocator()->find_by_libc_fd(libc_fd);

					if (fdo && (fdo->plugin == this) &&
					   (get_lwip_fd(fdo) == lwip_fd)) {

						if (lwip_FD_ISSET(lwip_fd, &lwip_readfds)) {

							FD_SET(libc_fd, readfds);
						}

						if (lwip_FD_ISSET(lwip_fd, &lwip_writefds)) {
							FD_SET(libc_fd, writefds);
						}

						if (lwip_FD_ISSET(lwip_fd, &lwip_exceptfds)) {
							FD_SET(libc_fd, exceptfds);
						}
					}
				}
			}
		}
	}
	return result;
}




ssize_t Plugin::send(Libc::File_descriptor *sockfdo, const void *buf, ::size_t len, int flags)
{	
	
	//PDBG("send  %s\n",buf);
	int re  = comm_server_connection.comm_srv_send(get_lwip_fd(sockfdo), buf,len ,flags);
	return re; 
}

ssize_t Plugin::sendto(Libc::File_descriptor *sockfdo, const void *buf,
                       ::size_t len, int flags,
                       const struct sockaddr *dest_addr, socklen_t addrlen)
{
	int result ; 
	commserversession::Session::Buffer sock_add;
	if (addrlen>0)
        	memcpy(sock_add.buff,dest_addr, addrlen);	
	result =  comm_server_connection.comm_srv_sendto(get_lwip_fd(sockfdo),buf, len, flags,sock_add,addrlen);
	return result;	
	 
}


int Plugin::setsockopt(Libc::File_descriptor *sockfdo, int level,
                       int optname, const void *optval,
                       socklen_t optlen)
{

	commserversession::Session::Buffer optvalue_buff; 
	memcpy(optvalue_buff.buff, ( char  *)optval,optlen);
	return comm_server_connection.comm_srv_setsockopt(get_lwip_fd(sockfdo), level, optname, optvalue_buff, optlen);	

}


int Plugin::shutdown(Libc::File_descriptor *sockfdo, int how)
{
	return comm_server_connection.comm_srv_shutdown(get_lwip_fd(sockfdo), how);
}


Libc::File_descriptor *Plugin::socket(int domain, int type, int protocol)
{
	
	int lwip_fd = comm_server_connection.comm_srv_socket( domain,  type,  protocol);
	if (lwip_fd == -1) {
		PERR("comm_srv_socket() failed");
		return 0;
	}
	
	Plugin_context *context = new (Genode::env()->heap()) Plugin_context(lwip_fd);
	Libc::File_descriptor *fdo = Libc::file_descriptor_allocator()->alloc(this, context);
	return fdo ;
}

ssize_t Plugin::write(Libc::File_descriptor *fdo, const void *buf, ::size_t count)
{
	//PDBG("writing:  %s\n",buf);
	int re  = comm_server_connection.comm_srv_write(get_lwip_fd(fdo), buf, count);
	return re; 
}


} /* unnamed namespace */


void create_libc_comm_plugin()
{
	static Plugin lwip_plugin;
}

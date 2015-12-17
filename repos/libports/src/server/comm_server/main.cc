//lwip
#include <lwip/sockets.h>
#include <lwip/genode.h>
#include <nic/packet_allocator.h>
//base
#include <base/printf.h>
#include <base/env.h>
#include <base/sleep.h>
#include <base/heap.h>

#include <cap_session/connection.h>
#include "comm_server_session/comm_server_session.h"

#include <root_multi/component.h>
#include <root_multi/session_component.h>

#include <os/attached_ram_dataspace.h>

#include <os/config.h> //config



namespace commserversession {
	enum { IO_SIZE = 4096 };
	enum { STACK_SIZE = 7096 };
	

  	  /**
	 * commserversession session with own entrypoint
	 */
	class Session_component : public Genode::Session_component_multi<Session, Session_component,STACK_SIZE>,
	                          public Genode::List<Session_component>::Element
	{
	
	private :

		/**
		 * Buffer shared with the terminal client
		 */
		Genode::Attached_ram_dataspace _io_buffer;

	public:
		Session_component(Genode::Cap_session *cap)
		: Genode::Session_component_multi<Session,Session_component, STACK_SIZE>(cap, "comm_server_session_ep"),
		  _io_buffer(Genode::env()->ram_session(), IO_SIZE)
		
		{ }

		
		RLS comm_srv_accept(int s, int add_len )
		{
 			//PDBG("thread_base at %p", Genode::Thread_base::myself());
			RLS result; 
			if (add_len==0)
				result.return_value  =lwip_accept( s,0,0);
			else {
				struct sockaddr_in address;
				result.return_value = lwip_accept( s, (struct sockaddr *)&address,(socklen_t* )&add_len);
				result.length = add_len; 
				memcpy(result.buff,&address,add_len);
			}
				
			return result ;
		}	
		
		int comm_srv_bind(int s,Buffer local_add,int length)
		{
			char  data[length];
			memcpy(data,local_add.buff,length);
			struct sockaddr_in * in_addr = (struct sockaddr_in  *)data;
			return lwip_bind( s, (struct sockaddr *)in_addr, length);
		}

		
		int comm_srv_connect(int s,Buffer dest_add,int length)
		{	
			char  data[length];
			memcpy(data,dest_add.buff,length);
			struct sockaddr_in * in_addr = (struct sockaddr_in  *)data;
			return  lwip_connect(s, (struct sockaddr *)in_addr, length);
		}
		int comm_srv_close(int s)
		{
			return lwip_close(s);
		}

		
		int comm_srv_fcntl(int s, int cmd, int val)
		{
			return lwip_fcntl(s, cmd, (val & O_NONBLOCK) ? -1 : O_NONBLOCK);
			
		}
		
		RLS comm_srv_getpeername(int s , int len )
		{
			struct sockaddr_in dest_add; 
			RLS result ; 
			result.return_value = lwip_getpeername(s,(struct sockaddr *)&dest_add,(socklen_t* )&len );
			result.length = len; 
			memcpy(result.buff,&dest_add,len);
			return result;
		}
		
		RLS comm_srv_getsockname(int s, int len)
		{  
			struct sockaddr_in local_add; 
			RLS result ; 
			result.return_value = lwip_getsockname(s,(struct sockaddr *)&local_add,(socklen_t* )&len );
			result.length = len; 
			memcpy(result.buff,&local_add,len);
			return result;
		}
		
		RLS comm_srv_getsockopt(int s, int level, int optname , int optlen)
		{
			
			RLS result ; 
			// optlen mus be less than BUFFER_LENGTH (200). 
			result.return_value =lwip_getsockopt(s,level, optname, result.buff, (socklen_t *)&optlen);
			result.length = optlen; 
			return result;
		}
		
		RLS comm_srv_ioctl(int s, long cmd,Buffer arg)
		{
			RLS result ; 
			/* since we only support FIONBIO and FIONREAD, arg will be  point integer */
			memcpy(result.buff,arg.buff,sizeof(int));
			result.return_value = lwip_ioctl( s,cmd,result.buff);
			return result;
		}
		
		int comm_srv_listen(int s, int back_log)
		{
			return lwip_listen( s, back_log);
		}
		

		Genode::Dataspace_capability _dataspace() { return _io_buffer.cap(); }

		int  _read(int s, int count)
		{      
			/* XXX we need to check wether count is lager than IO_SIZE */
			char *bf = _io_buffer.local_addr<char>();
			//PDBG(" read from  to %p",bf);
			int rec=lwip_read( s, bf, count);
			return rec; 
			
		}
		int comm_srv_read(int s, void *mem, int len){return 0;}
		
		int  _comm_srv_recv(int s,int count, int flags)
		{      
			/* XXX we need to check wether count is lager than IO_SIZE */
			char *bf = _io_buffer.local_addr<char>();
			//PDBG(" recv from  %p",bf);
			int rec=lwip_recv( s, bf, count,flags);
			return rec; 
		}
		int comm_srv_recv(int s, void *mem, int len,int flags){return 0;}
		
		
		RLS _comm_srv_recvfrom(int s,int len, int flags,int length)
		{
			RLS result ; 
			char *bf = _io_buffer.local_addr<char>();
			if(( length <=0) || (length ==NULL)){ 
				/* Not intrested in the source address */
				result.return_value = lwip_recvfrom( s, bf, len,flags,NULL,NULL);
			}
			else{
				struct sockaddr_in src_add;
				result.return_value = lwip_recvfrom( s, bf, len,flags,(struct sockaddr *)&src_add,(socklen_t* )&length);
				result.length = length; 
				memcpy(result.buff,&src_add,length);
			}
			
			return result;
		}
		RLS comm_srv_recvfrom(int s,void *mem ,int len, int flags,int length)
		{
			RLS result ; 
			result.return_value = 0; 
			return result;
		}

		
		RPC_FD_SET comm_srv_select(RPC_FD_SET fs, Buffer w_t)
		{
			int maxfdp ,r ;
			RPC_FD_SET res; 
			fd_set  rset;
			fd_set  wset;
			fd_set  exset;
			struct timeval  *time_out =(struct timeval  *) malloc(sizeof(struct timeval ));
	
			maxfdp = fs.result;
			memcpy(&rset,fs.rbuff,sizeof(rset));
			memcpy(&wset,fs.wbuff,sizeof(wset));
			memcpy(&exset,fs.ebuff,sizeof(wset));
			memcpy(time_out,w_t.buff,sizeof(struct timeval ));

			r = lwip_select(maxfdp,&rset,&wset,&exset,time_out); 

			res.result = r;
			memcpy(res.rbuff,&rset,sizeof(fd_set ));
			memcpy(res.wbuff,&wset,sizeof(fd_set ));
			memcpy(res.ebuff,&exset,sizeof(fd_set ));
	
			free(time_out);
			return res;

		}
		
		int _comm_srv_send(int s ,int num_bytes,int flags)
		{
			
			num_bytes = Genode::min(num_bytes, _io_buffer.size());
			char const *src = _io_buffer.local_addr<char>();
			int sent= lwip_send(s,src,num_bytes,flags);
			return sent; 
		}

		int comm_srv_send(int s ,const void *dataptr, int num_bytes,int flags) {return 0;}

		int _comm_srv_sendto(int s ,int len ,int flags,Buffer sock_add ,int length)
		{
			int result; 
			char const *buf = _io_buffer.local_addr<char>();
			if (length>0){

				char  data[length];
				memcpy(data, sock_add.buff,length);
				struct sockaddr_in * in_addr = (struct sockaddr_in  *)data;
				result =lwip_sendto(s,buf,len,flags,(sockaddr *)in_addr,(socklen_t) length );
			}
			else 
				result =lwip_sendto(s,buf,len,flags,NULL,0 );
				
			return result;
			
		}


		int comm_srv_sendto(int s ,const void * buf ,int l ,int flags,Buffer sock_add ,int length) {return 0;}

		
		int  comm_srv_setsockopt(int domain,int level,int optname, Buffer optvalue, int  optlen)
		{
			int  result = lwip_setsockopt(domain,level, optname, (void *)optvalue.buff, (socklen_t )optlen);
			return result;
		}

		int comm_srv_shutdown(int s , int how)
		{
			return lwip_shutdown(s,how);
		}
	

		int comm_srv_socket(int domain, int type, int protocol)
		{
			
			// PDBG("thread_base at %p", Genode::Thread_base::myself());
			return lwip_socket(  domain,  type,  protocol);
			
		}

		void _write(int s ,int num_bytes)
		{
			
			num_bytes = Genode::min(num_bytes, _io_buffer.size());
			char const *src = _io_buffer.local_addr<char>();
			//PDBG(" write to %p",src);
			int sent= lwip_write(s,src,num_bytes);
			
		}
		
		int comm_srv_write(int s, const void *dataptr, int  size)
		{ return 0;}
		
		
	};




class Root_component :  public Genode::Root_component_multi<Session_component>
	{
		private:
			Session_component *_create_session(const char *args)
			{
				/* create new session for the requested file */
				return new (md_alloc()) Session_component(cap_session());
			}
		public:
			/**
			 * Constructor
			 *
			 * \param  entrypoint  entrypoint to be used for the sessions
			 * \param  md_alloc    meta-data allocator used for the sessions
			 * \param  cap         cap session
			 */
			Root_component(Genode::Rpc_entrypoint  &entrypoint,
						      Genode::Allocator       &md_alloc,
						      Genode::Cap_session     &cap)
			:
				Genode::Root_component_multi<Session_component>(&entrypoint, &md_alloc, &cap)
			{ }
	};
		

}	


using namespace Genode;
int main(void)
{
	
	enum { STACK_SIZE = 7096 };



	
	lwip_tcpip_init();

	enum { BUF_SIZE = Nic::Packet_allocator::DEFAULT_PACKET_SIZE * 128 };
	

	lwip_nic_init(0,0,0, BUF_SIZE, BUF_SIZE);

	PDBG("gotting ip is done. my IP is");

	/*
	 * Initialize server entry point that serves the root interface.
	 */
	static Cap_connection cap;
	static Rpc_entrypoint ep(&cap, STACK_SIZE, "comm_server_ep");

	/*
	 * Use sliced heap to allocate each session component at a separate
	 * dataspace.
	 */
	static Sliced_heap sliced_heap(env()->ram_session(), env()->rm_session());

	/*
	 * Create root interface for service
	 */
	static commserversession::Root_component multi_root(ep, sliced_heap, cap);

	/*
	 * Announce service at our parent.
	 */
	env()->parent()->announce(ep.manage(&multi_root));

	PINF("Communication Server started");

	sleep_forever();
	return 0;

}

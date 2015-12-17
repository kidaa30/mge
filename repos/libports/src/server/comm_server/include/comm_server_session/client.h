/*
 *\brief Client component fo the comm_srver
 *\author Mohammad Hamad
 *\date 
 */

#ifndef _INCLUDE__COMM_SERVER_SESSION_H__CLIENT_H_
#define _INCLUDE__COMM_SERVER_SESSION_H__CLIENT_H_

#include <comm_server_session/comm_server_session.h>
#include <base/rpc_client.h>
#include <base/printf.h>


//shaed memeory
#include <os/attached_dataspace.h>

#include <util/misc_math.h>
#include <util/string.h>
#include <base/lock.h>





namespace commserversession {

	struct Session_client : Genode::Rpc_client<Session>
	{
		private:

			Genode::Lock _lock;

			/**
			 * Shared-memory buffer used for carrying the payload
			 * of read/write operations
			 */
			Genode::Attached_dataspace _io_buffer;
		public:
	
			Session_client(Genode::Capability<Session> cap)
			:
				Genode::Rpc_client<Session>(cap),
				_io_buffer(call<Rpc_dataspace>())
				{ }
		
		
 			RLS comm_srv_accept(int s, int l )
			{	
				return call<Rpc_comm_srv_accept>( s, l);
			}	
		
 			int comm_srv_bind(int s,Buffer local_add, int length)
			{
				return call<Rpc_comm_srv_bind>( s,local_add,length);
			}	
		
			int comm_srv_connect(int s,Buffer dest_add, int length)
			{
				return call<Rpc_comm_srv_connect>(s,dest_add,length);
			}
		
			int comm_srv_close(int s)
			{
				return call<Rpc_comm_srv_close>( s);
			}

			int  comm_srv_fcntl(int s, int cmd, int val)
			{
				return call<Rpc_comm_srv_fcntl>( s,cmd,val);
			}

			RLS comm_srv_getpeername(int s, int l)
			{
				return call<Rpc_comm_srv_getpeername>(s, l);
			}

			RLS comm_srv_getsockname(int s, int l)
			{
				return call<Rpc_comm_srv_getsockname>(s, l);
			}
		
			RLS comm_srv_getsockopt(int s, int level, int optname , int optlen )
			{
				return call<Rpc_comm_srv_getsockopt>( s,level,optname,optlen);
			}
		
		
			RLS comm_srv_ioctl(int s, long cmd , Buffer arg)
			{	
				return call<Rpc_comm_srv_ioctl>( s,  cmd , arg);
			}	
		
			int comm_srv_listen(int s, int max_conn)
			{	
				return call<Rpc_comm_srv_listen>( s,  max_conn);
			}
			
			int comm_srv_read(int s, void *mem, int len)
			{
				/*
				this operation c
				Genode::Lock::Guard _guard(_lock);
				int readed_bytes = 0;
				int requiered =  len; 
				int avl_bytes , r_b; 
				PDBG(" comm_srv_read , len (%d), readed_bytes(%d) ,requiered(%d)", len,readed_bytes,requiered);
				while (readed_bytes < len) {
					PDBG("readed_bytes < len");
					avl_bytes = Genode::min(requiered, _io_buffer.size());
					PDBG(" avl_bytes (%d)",avl_bytes);
					r_b = call<Rpc_comm_srv_read>(s,avl_bytes);
					Genode::memcpy(mem+readed_bytes, _io_buffer.local_addr<char>(), r_b);
					readed_bytes +=r_b;
					requiered -=r_b;
				}
				*/
				Genode::Lock::Guard _guard(_lock);
				/* read and fill the I/O buffer */
				int num_bytes = call<Rpc_comm_srv_read>(s,len);
				/* copy-out I/O buffer */
				num_bytes = Genode::min(num_bytes, len);
				Genode::memcpy(mem, _io_buffer.local_addr<char>(), num_bytes);
				return num_bytes;
			}
			
		
			int comm_srv_recv(int s, void *mem, int len, int flags)
			{	
				Genode::Lock::Guard _guard(_lock);
				int num_bytes = call<Rpc_comm_srv_recv>(s,len,flags);
				num_bytes = Genode::min(num_bytes, len);
				Genode::memcpy(mem, _io_buffer.local_addr<char>(), num_bytes);
				return num_bytes;
			}

			RLS comm_srv_recvfrom(int s,void *mem ,int len,int flags ,int length)
			{
				RLS  result ;
				Genode::Lock::Guard _guard(_lock);
				result= call<Rpc_comm_srv_recvfrom>(s,len,flags,length);
				result.return_value = Genode::min(result.return_value, len);
				Genode::memcpy(mem, _io_buffer.local_addr<char>(), result.return_value);
				return result;
			}	
			
			RPC_FD_SET comm_srv_select(RPC_FD_SET fs,Buffer  wait_time )
			{
				return call<Rpc_comm_srv_select>(fs,wait_time);
			}

			int comm_srv_send(int s , const void *buf ,int num_bytes,int flags)
			{
				Genode::Lock::Guard _guard(_lock);
				int    written_bytes = 0;
				char const * const src           = (char const *)buf;

				while (written_bytes < num_bytes) {

					/* copy payload to I/O buffer */
					Genode::size_t n = Genode::min(num_bytes - written_bytes,_io_buffer.size());
					Genode::memcpy(_io_buffer.local_addr<char>(),src + written_bytes, n);
					call<Rpc_comm_srv_send>(s,num_bytes,flags);
					written_bytes += n;
				}
				return num_bytes;
			}
		
			int comm_srv_sendto(int s ,const void *buf ,int num_bytes ,int flags, Buffer sock_add ,int sock_len)
			{
				Genode::Lock::Guard _guard(_lock);
				int    written_bytes = 0;
				char const * const src   = (char const *)buf;
				while (written_bytes < num_bytes) {
					Genode::size_t n = Genode::min(num_bytes - written_bytes,_io_buffer.size());
					Genode::memcpy(_io_buffer.local_addr<char>(),src + written_bytes, n);
					call<Rpc_comm_srv_sendto>(s,num_bytes,flags,sock_add,sock_len);
					written_bytes += n;
				}
				return num_bytes;
			}
      
			int comm_srv_setsockopt(int domain, int level,  int optname,Buffer potval ,int optlen)
			{
				return  call<Rpc_comm_srv_setsockopt>(domain, level, optname,potval,optlen);
			}

			int comm_srv_shutdown(int s, int how)
			{
				return call<Rpc_comm_srv_shutdown>(s,how);
			}
		

			int comm_srv_socket(int domain, int type, int protocol)
			{
				return  call<Rpc_comm_srv_socket>(domain, type, protocol);
			}
			
			int comm_srv_write(int s, const void *buf, int num_bytes)
			{
				
				Genode::Lock::Guard _guard(_lock);
				int    written_bytes = 0;
				char const * const src           = (char const *)buf;
				while (written_bytes < num_bytes) {
					Genode::size_t n = Genode::min(num_bytes - written_bytes,
				                               _io_buffer.size());
					Genode::memcpy(_io_buffer.local_addr<char>(),src + written_bytes, n);
					call<Rpc_comm_srv_write>(s,n);
					written_bytes += n;
				}
				return num_bytes;
			}
		
	};
}

#endif /* _INCLUDE__COMM_SERVER_SESSION_H__CLIENT_H_ */

/*
 * \brief  Interface definition of the communication server
 * \author Mohammad hamad
 * \date   27-11-2015
 */

/*
 * this interface provide  all the lwip socket API , we add (comm_srv) prefix infront each name . 
*/


/* Copyright (C) 2008-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__COMM_SEVER_SESSION_
#define _INCLUDE__COMM_SEVER_SESSION_



#include <session/session.h>
#include <base/rpc.h>

#include <dataspace/capability.h>

#define STRUCT_LENGTH     100
#define BUFFER_LENGTH     200	

namespace commserversession {

	struct Session : Genode::Session
	{
		static const char *service_name() { return "COMM_SERVER"; }

		struct Buffer{  char buff[BUFFER_LENGTH];};
		
		// struct which store the Return value (int) the length of buffer with the buffer
		struct RLS { 
			int return_value;
			int length;
			char buff[BUFFER_LENGTH];
		};
		
		//  struct used by select function only.
		struct RPC_FD_SET { 
				int   result;    				// select result , select sock value 
				unsigned char  rbuff[BUFFER_LENGTH];		// read set
				unsigned char  wbuff[BUFFER_LENGTH];		// write set 
				unsigned char  ebuff[BUFFER_LENGTH];		// excue set
				};

		

		virtual RLS comm_srv_accept(int sock_desc, int add_len )=0;
		
		virtual int comm_srv_bind(int sock_desc,Buffer local_add,int add_len)=0;
		
		virtual int comm_srv_connect(int sock_desc,Buffer dest_add , int add_len )=0;

		virtual int comm_srv_close(int sock_desc)=0;

		virtual int comm_srv_fcntl(int sock_desc, int cmd, int val)=0;
		
		virtual RLS comm_srv_getpeername(int sock_desc, int len )=0;

		virtual RLS comm_srv_getsockname(int sock_desc, int len)=0;

		virtual RLS comm_srv_getsockopt(int sock_desc, int level, int optname , int optlen )=0;

		virtual RLS comm_srv_ioctl(int sock_desc, long cmd,Buffer arg)=0;
		
		virtual int comm_srv_listen(int sock_desc, int max_conn)=0;

		virtual int comm_srv_read(int sock_desc, void *mem, int len)=0;
		
		virtual int comm_srv_recv(int sock_desc, void *mem, int len, int flags)=0;
		
		virtual RLS comm_srv_recvfrom(int sock_desc,void *mem,int len, int flags, int sock_len)=0;
		
		virtual RPC_FD_SET comm_srv_select(RPC_FD_SET fs,Buffer wait_t)=0;
		
		virtual int comm_srv_send(int sock_desc,const void *,int,int)=0;

		virtual int comm_srv_sendto(int sock_desc,const void * buff,int len,int flags,Buffer sock_add ,int sock_len)=0;

		virtual int  comm_srv_setsockopt(int sock_desc,int level,int optname ,Buffer potval,int optlen )=0;
		
		virtual int comm_srv_shutdown(int sock_desc, int how)=0;

		virtual int comm_srv_socket(int sock_desc ,int ,int )=0;

		virtual int comm_srv_write(int sock_desc, const void *dataptr, int  size)=0;

		
		
		
		/*******************
		 ** RPC interface **
		 *******************/
		
		

		GENODE_RPC(Rpc_comm_srv_accept, RLS, comm_srv_accept,int, int );

		GENODE_RPC(Rpc_comm_srv_bind, int, comm_srv_bind,int,Buffer,int);

		GENODE_RPC(Rpc_comm_srv_connect, int, comm_srv_connect,int,Buffer, int );

		GENODE_RPC(Rpc_comm_srv_close, int, comm_srv_close,int);

		GENODE_RPC(Rpc_comm_srv_fcntl, int, comm_srv_fcntl,int,int,int);
		
		GENODE_RPC(Rpc_comm_srv_getpeername, RLS,comm_srv_getpeername,int,int);
		
		GENODE_RPC(Rpc_comm_srv_getsockname, RLS,comm_srv_getsockname,int,int);
		
		GENODE_RPC(Rpc_comm_srv_getsockopt,  RLS,comm_srv_getsockopt,int,int ,int, int);
		
		GENODE_RPC(Rpc_comm_srv_ioctl, RLS ,comm_srv_ioctl,int, long, Buffer );

		GENODE_RPC(Rpc_comm_srv_listen, int, comm_srv_listen,int,int);
		
		GENODE_RPC(Rpc_comm_srv_read,int, _read,int,int);
		
		GENODE_RPC(Rpc_comm_srv_recv,int , _comm_srv_recv,int,int,int);

		GENODE_RPC(Rpc_comm_srv_recvfrom,RLS, _comm_srv_recvfrom,int,int,int, int);

		GENODE_RPC(Rpc_comm_srv_select ,RPC_FD_SET,comm_srv_select,RPC_FD_SET,Buffer);
		
		GENODE_RPC(Rpc_comm_srv_send,int, _comm_srv_send,int,int,int);
	
		GENODE_RPC(Rpc_comm_srv_sendto,int, _comm_srv_sendto,int,int,int,Buffer,int);

		GENODE_RPC(Rpc_comm_srv_setsockopt,int,comm_srv_setsockopt,int ,int ,int,Buffer,int );
		
		GENODE_RPC(Rpc_comm_srv_shutdown, int, comm_srv_shutdown,int,int);
				
		GENODE_RPC(Rpc_comm_srv_socket, int, comm_srv_socket,int ,int ,int );

		GENODE_RPC(Rpc_comm_srv_write,void,_write,int,int);
		
		GENODE_RPC(Rpc_dataspace, Genode::Dataspace_capability, _dataspace);
		
		
	
			/*
			 * 'GENODE_RPC_INTERFACE' declaration done manually
			 *
			 * The number of RPC function of this interface exceeds the maximum
			 * number of elements supported by 'Meta::Type_list'. Therefore, we
			 * construct the type list by hand using nested type tuples instead
			 * of employing the convenience macro 'GENODE_RPC_INTERFACE'.
			 */
	
		typedef Genode::Meta::Type_tuple<Rpc_comm_srv_accept,
			Genode::Meta::Type_tuple<Rpc_comm_srv_bind,
			Genode::Meta::Type_tuple<Rpc_comm_srv_close,
			Genode::Meta::Type_tuple<Rpc_comm_srv_connect,
			Genode::Meta::Type_tuple<Rpc_comm_srv_fcntl,
			Genode::Meta::Type_tuple<Rpc_comm_srv_getpeername,
			Genode::Meta::Type_tuple<Rpc_comm_srv_getsockname,
			Genode::Meta::Type_tuple<Rpc_comm_srv_getsockopt,
			Genode::Meta::Type_tuple<Rpc_comm_srv_listen,
			Genode::Meta::Type_tuple<Rpc_comm_srv_ioctl,
			Genode::Meta::Type_tuple<Rpc_comm_srv_read,
			Genode::Meta::Type_tuple<Rpc_comm_srv_recv,
			Genode::Meta::Type_tuple<Rpc_comm_srv_recvfrom,
			Genode::Meta::Type_tuple<Rpc_comm_srv_shutdown,
			Genode::Meta::Type_tuple<Rpc_comm_srv_send,
			Genode::Meta::Type_tuple<Rpc_comm_srv_sendto,
			Genode::Meta::Type_tuple<Rpc_comm_srv_setsockopt,
			Genode::Meta::Type_tuple<Rpc_comm_srv_select,
			Genode::Meta::Type_tuple<Rpc_comm_srv_socket,
			Genode::Meta::Type_tuple<Rpc_comm_srv_write,
			Genode::Meta::Type_tuple<Rpc_dataspace,
					Genode::Meta::Empty>
			 > > > > > > > > > > > > > > > > > > > >  Rpc_functions;		
	};
}

#endif /* _INCLUDE__COMM_SEVER_SESSION_*/

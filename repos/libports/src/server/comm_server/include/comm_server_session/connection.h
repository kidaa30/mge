#ifndef _INCLUDE__COMM_SERVER_SESSION__CONNECTION_H_
#define _INCLUDE__COMM_SERVER_SESSION__CONNECTION_H_

#include <comm_server_session/client.h>
#include <base/connection.h>

namespace commserversession {

	struct Connection : Genode::Connection<Session>, Session_client
	{
		Connection()
		:
			/* create session */
			Genode::Connection<commserversession::Session>(session("foo, ram_quota=4000K")),

			/* initialize RPC interface */
			Session_client(cap()) { }
	};
}

#endif /* _INCLUDE__RPC_LWIP_SESSION__CONNECTION_H_ */

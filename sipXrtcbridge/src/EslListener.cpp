
#include "sipx/bridge/EslListener.h"

#include <fcntl.h>
#include <arpa/inet.h>

extern "C"
{
  #include "esl.h"
}


namespace sipx {
namespace bridge {


#define closesocket(x) close(x)

EslListener::EslListener() :
  _pEventThread(0)
{
}
  
EslListener::~EslListener()
{
  delete _pEventThread;
}
 
bool EslListener::listenForEvents(ConnectionHandler eventHandler, const std::string& address, unsigned short port)
{
  assert(eventHandler);
  assert(port);
  assert(!address.empty());
  
  esl_socket_t server_sock = ESL_SOCK_INVALID;
	struct sockaddr_in addr;

	if ((server_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
  {
		return false;
	}

  _eventHandler = eventHandler;
  _eventServerSocket = server_sock;
  _eventListenerAddress = address;
  _eventListenerPort = port;

  int reuse_addr = 1;
  setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));


	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(address.c_str());
  addr.sin_port = htons(_eventListenerPort);

  if (bind(server_sock, (struct sockaddr *) &addr, sizeof(addr)) < 0)
  {
    if (server_sock != ESL_SOCK_INVALID)
    {
      closesocket(server_sock);
      server_sock = ESL_SOCK_INVALID;
      return false;
    }
  }
	

  if (listen(server_sock, 10000) < 0)
  {
		if (server_sock != ESL_SOCK_INVALID)
    {
      closesocket(server_sock);
      server_sock = ESL_SOCK_INVALID;
      return false;
    }
	}
  
  return true;
}

void EslListener::runEventLoop()
{ 
  for (;;) {
		int client_sock = -1;
		struct sockaddr_in remoteAddress;
		unsigned int clntLen = sizeof(remoteAddress);

		if ((client_sock = accept(_eventServerSocket, (struct sockaddr *) &remoteAddress, &clntLen)) == ESL_SOCK_INVALID)
    {
			if (_eventServerSocket != ESL_SOCK_INVALID)
      {
        closesocket(_eventServerSocket);
        _eventServerSocket = ESL_SOCK_INVALID;
        return;
      }
		}

    //
    // Create the connection and event here then propagate to the event handler if it exists
    //
    if (client_sock > 0 && _eventHandler)
    {
      EslConnection::Ptr pConnection = EslConnection::Ptr(new EslConnection(client_sock));
      if (pConnection->hasInfoEslEvent())
      {
        bool autodeleteHandle = false;
        esl_handle_t* handle = (esl_handle_t*)pConnection->_handle;
        EslEvent::Ptr pEvent = EslEvent::Ptr(new EslEvent(handle->info_event, autodeleteHandle));
        _eventHandler(pConnection, pEvent);
      }
    }
	}
}
  
void EslListener::run()
{
  _pEventThread = new boost::thread(boost::bind(&EslListener::runEventLoop, this));
}
  
void EslListener::stop()
{
  closesocket(_eventServerSocket);
  if (_pEventThread)
    _pEventThread->join();
}


} } // sipx::bridge




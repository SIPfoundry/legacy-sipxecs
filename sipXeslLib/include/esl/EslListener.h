
#ifndef ESLLISTENER_H_INCLUDED
#define	ESLLISTENER_H_INCLUDED

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <string>
#include <esl/EslConnection.h>



class EslListener 
{
public:
  
  typedef boost::function<void(const EslConnection::Ptr&, const EslEvent::Ptr&)> ConnectionHandler;
  
  EslListener();
  
  ~EslListener();
 
  bool listenForEvents(ConnectionHandler eventHandler, unsigned short port);
  
  bool listenForEvents(ConnectionHandler eventHandler, const std::string& address, unsigned short port);
  
  void run();
  
  void stop();
  
protected:

  void runEventLoop();
  
  std::string _eventListenerAddress;
  unsigned short _eventListenerPort;
  EslConnection::Ptr _eventOutbound;
  ConnectionHandler _eventHandler;
  int _eventServerSocket;
  boost::thread* _pEventThread;
};


//
// Inlines
//

inline bool EslListener::listenForEvents(ConnectionHandler eventHandler, unsigned short port)
{
  std::string address = "127.0.0.1";
  return listenForEvents(eventHandler, address, port);
}


#endif	


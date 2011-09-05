
#include <cstdlib>
#include "ZMQNotifier.h"
#include "ZMQSubscriber.h"
#include "ZMQReactor.h"
#include "ZMQReactorClient.h"
#include "ZMQServiceBroker.h"
#include "ZMQPipeLineBroker.h"
#include "ZMQProactorClient.h"
#include "ZMQProactor.h"
#include "ZMQActor.h"
#include "ZMQBlockingRequest.h"
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "ZMQContext.h"
#include "ZMQJsonRpcClient.h"
#include "ZMQJsonRpcServer.h"
#include <cassert>

using namespace std;

static int64_t clockms()
{
  struct timeval tv;
  gettimeofday (&tv, NULL);
  return (int64_t) ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}
bool exitNotifier = false;
bool startNotifier = false;

bool handleEvent(ZMQMessage& message)
{
  ZMQMessage::Headers headers;
  message.parseHeaders(headers);
  //ZMQ_LOG_DEBUG(headers.address << "-" << headers.identifier << "-" << headers.body << "-" << pthread_self());
  exitNotifier = headers.identifier == "call.bye";
  return true;
}



int main(int argc, char** argv)
{
  ZMQLogger::verbosity = ZMQLogger::DEBUG;
  ZMQSocket::Error error;
  //
  // Start the broker
  //
  ZMQPipeLineBroker broker;
  broker.startServingMessages("tcp://192.168.1.10:2020", "tcp://192.168.1.10:2021", error);
  //
  // Start the reactor
  //
  ZMQReactor reactor;
  reactor.startReactor("tcp://192.168.1.10:2022", "tcp://192.168.1.10:2020", "tcp://192.168.1.10:2021", error);
  //
  // Attach actors
  //
  ZMQActor actor1;
  actor1.subscibeToReactor("tcp://192.168.1.10:2022", "call.calling", error);
  actor1.startPerformingTasks(boost::bind(handleEvent, _1), error);
  ZMQActor actor2;
  actor2.subscibeToReactor("tcp://192.168.1.10:2022", "call.ringing", error);
  actor2.startPerformingTasks(boost::bind(handleEvent, _1), error);
  ZMQActor actor3;
  actor3.subscibeToReactor("tcp://192.168.1.10:2022", "call.connected", error);
  actor3.startPerformingTasks(boost::bind(handleEvent, _1), error);
  ZMQActor actor4;
  actor4.subscibeToReactor("tcp://192.168.1.10:2022", "call.bye", error);
  actor4.startPerformingTasks(boost::bind(handleEvent, _1), error);
  //
  // Create our client and post the events
  //
  ZMQReactorClient client;
  client.connectToService("tcp://192.168.1.10:2020", error);
  for (int i = 0; i < 1000000; i++)
  {
    ZMQMessage eventCalling("call.calling 192.168.1.10 ZMQEvent");
    client.postEvent(eventCalling, error);
    ZMQMessage eventRinging("call.ringing 192.168.1.10 ZMQEvent");
    client.postEvent(eventRinging, error);
    ZMQMessage eventConnected("call.connected 192.168.1.10 ZMQEvent");
    client.postEvent(eventConnected, error);
  }
  ZMQMessage eventDisconnected("call.bye 192.168.1.10 ZMQEvent");
  client.postEvent(eventDisconnected, error);
  //
  // Wait for termination signal
  //
  while(!exitNotifier)sleep(0);
  //
  // Terminate services
  //
  actor1.stopPerformingTasks();
  actor2.stopPerformingTasks();
  actor3.stopPerformingTasks();
  actor4.stopPerformingTasks();
  //
  // Stop the reactor
  //
  ZMQ_LOG_DEBUG("STOPPING REACTOR");
  reactor.stopReactor();
  //
  // Finally stop the broker
  //
  ZMQ_LOG_DEBUG("STOPPING BROKER");
  broker.stopServingMessages();

}

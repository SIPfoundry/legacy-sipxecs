
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


static void handleProactorTask(ZMQMessage& task, ZMQMessage& response)
{
  exitNotifier = (task.data() == "quit");
  response.data() = task.data() + " RESPONSE";
}

int main(int argc, char** argv)
{
  ZMQLogger::verbosity = ZMQLogger::DEBUG;
  ZMQSocket::Error error;

  ZMQProactor proactor1;
  ZMQProactor proactor2;
  ZMQProactor proactor3;
  ZMQProactor proactor4;
  ZMQProactor proactor5;
  ZMQProactor proactor6;
  ZMQProactor proactor7;
  ZMQProactor proactor8;

  proactor1.bind("tcp://192.168.1.10:12020", error);
  proactor2.bind("tcp://192.168.1.10:12021", error);
  proactor3.bind("tcp://192.168.1.10:12022", error);
  proactor4.bind("tcp://192.168.1.10:12023", error);
  proactor5.bind("tcp://192.168.1.10:12024", error);
  proactor6.bind("tcp://192.168.1.10:12025", error);
  proactor7.bind("tcp://192.168.1.10:12026", error);
  proactor8.bind("tcp://192.168.1.10:12027", error);

  proactor1.startProactor("test", "proactor1@192.168.0.1",  boost::bind(handleProactorTask, _1, _2), error);
  proactor2.startProactor("test", "proactor2@192.168.0.1",  boost::bind(handleProactorTask, _1, _2), error);
  proactor3.startProactor("test", "proactor3@192.168.0.1",  boost::bind(handleProactorTask, _1, _2), error);
  proactor4.startProactor("test", "proactor4@192.168.0.1",  boost::bind(handleProactorTask, _1, _2), error);
  proactor5.startProactor("test", "proactor5@192.168.0.1",  boost::bind(handleProactorTask, _1, _2), error);
  proactor6.startProactor("test", "proactor6@192.168.0.1",  boost::bind(handleProactorTask, _1, _2), error);
  proactor7.startProactor("test", "proactor7@192.168.0.1",  boost::bind(handleProactorTask, _1, _2), error);
  proactor8.startProactor("test", "proactor8@192.168.0.1",  boost::bind(handleProactorTask, _1, _2), error);

  ZMQProactorClient client;
  client.connect("tcp://192.168.1.10:12020", error);
  client.connect("tcp://192.168.1.10:12021", error);
  client.connect("tcp://192.168.1.10:12022", error);
  client.connect("tcp://192.168.1.10:12023", error);
  client.connect("tcp://192.168.1.10:12024", error);
  client.connect("tcp://192.168.1.10:12025", error);
  client.connect("tcp://192.168.1.10:12026", error);
  client.connect("tcp://192.168.1.10:12027", error);
  client.startProactorClient("test", "test", error);
  
  for (int iter = 0; iter < 10; iter++)
  {
    int count;
    int64_t start = clockms();
    for (count= 0; count < 0xFFFFFFFFFFFF; count++)
    {
      ZMQBlockingRequest::Ptr request(new ZMQBlockingRequest("sample-command"));
      client.blocking_send(request, error);
      if (clockms() - start >= 1000)
        break;
    }
    int64_t end = clockms();
    ZMQ_LOG_DEBUG("Iteration " << iter << " clocks at " << count << " requests per second");
  }

  ZMQBlockingRequest::Ptr request(new ZMQBlockingRequest("quit"));
  client.blocking_send(request, error);

  client.stopProactorClient();
  proactor1.stopProactor();
  proactor2.stopProactor();
  proactor3.stopProactor();
  proactor4.stopProactor();
  proactor5.stopProactor();
  proactor6.stopProactor();
  proactor7.stopProactor();
  proactor8.stopProactor();
  
  return 0;
}

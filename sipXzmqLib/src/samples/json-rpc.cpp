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

enum CallState
{
  IDLE,
  CALLING,
  RINGING,
  CONNECTED,
  DISCONNECTED,
  TERMINATED
};

static bool handle_calling(const ZMQJsonMessage& request, ZMQJsonMessage& result)
{
  result.setResult("call_state", (CALLING));
  return true;
}

static bool handle_ringing(const ZMQJsonMessage& request, ZMQJsonMessage& result)
{
  result.setResult("call_state", (RINGING));
  return true;
}

static bool handle_connected(const ZMQJsonMessage& request, ZMQJsonMessage& result)
{
  result.setResult("call_state", (CONNECTED));
  return true;
}

static bool handle_disconnected(const ZMQJsonMessage& request, ZMQJsonMessage& result)
{
  result.setResult("call_state", (DISCONNECTED));
  return true;
}

static bool handle_terminated(const ZMQJsonMessage& request, ZMQJsonMessage& result)
{
  exitNotifier = true;
  result.setResult("call_state", (TERMINATED));
  return true;
}

static int64_t event_start_tick = 0;
static bool event_last_tick = false;

static void handle_event(const ZMQJsonMessage& event)
{
  assert(event.getMethod() == "handle_event");
  if (clockms() - event_start_tick >= 1000)
    event_last_tick = true;
}

int main(int argc, char** argv)
{
  ZMQLogger::verbosity = ZMQLogger::INFO;
  ZMQJsonRpcServer server;
  ZMQJsonRpcClient client;
  server.registerMethod("handle_calling", boost::bind(handle_calling, _1, _2));
  server.registerMethod("handle_ringing", boost::bind(handle_ringing, _1, _2));
  server.registerMethod("handle_connected", boost::bind(handle_connected, _1, _2));
  server.registerMethod("handle_disconnected", boost::bind(handle_disconnected, _1, _2));
  server.registerMethod("handle_terminated", boost::bind(handle_terminated, _1, _2));
  server.registerNotifier("handle_event", boost::bind(handle_event, _1));
  server.startRpcService("json-test", "tcp", "192.168.1.10", 50202);
  client.connect( "tcp", "192.168.1.10", 50202);
  client.startRpcService("jsonrpcclient@192.168.1.10");
  for (int iter = 0; iter < 10; iter++)
  {
    int count;
    int64_t start = clockms();
    for (count= 0; count < 0xFFFFFFFFFFFF; count++)
    {
      {
      json::Object params;
      ZMQJsonMessage result;
      params["previous_call_state"] = json::Number(IDLE);
      client.execute("handle_calling", params, result);
      int state = 0;
      assert(result.getResult("call_state", state));
      assert(state == CALLING);
      }
      {
      json::Object params;
      ZMQJsonMessage result;
      params["previous_call_state"] = json::Number(CALLING);
      client.execute("handle_ringing", params, result);
      int state = 0;
      assert(result.getResult("call_state", state));
      assert(state == RINGING);
      }
      {
      json::Object params;
      ZMQJsonMessage result;
      params["previous_call_state"] = json::Number(RINGING);
      client.execute("handle_connected", params, result);
      int state = 0;
      assert(result.getResult("call_state", state));
      assert(state == CONNECTED);
      }
      {
      json::Object params;
      ZMQJsonMessage result;
      params["previous_call_state"] = json::Number(CONNECTED);
      client.execute("handle_disconnected", params, result);
      int state = 0;
      assert(result.getResult("call_state", state));
      assert(state == DISCONNECTED);
      }
      if (clockms() - start >= 1000)
        break;
    }
    int64_t end = clockms();
    ZMQ_LOG_INFO("RPC Iteration " << iter << " clocks at " << count * 4 << " requests per second");
  }

  for (int iter = 0; iter < 10; iter++)
  {
    int count;
    event_start_tick = clockms();
    event_last_tick = false;
    for (count= 0; count < 0xFFFFFFFFFFFF; count++)
    {
      json::Object eventParams;
      eventParams["sample-event"] = json::String("this is a sample event");
      client.notify("notifier@192.168.1.10", "handle_event", eventParams);
      if (event_last_tick)
        break;
    }
    int64_t end = clockms();
    ZMQ_LOG_INFO("Event Iteration " << iter << " clocks at " << count * 4 << " requests per second");
  }



  {
  json::Object params;
      ZMQJsonMessage result;
  params["previous_call_state"] = json::Number(DISCONNECTED);
  client.execute("handle_terminated", params, result);
  int state = 0;
  assert(result.getResult("call_state", state));
  assert(state == TERMINATED);
  }
  while(!exitNotifier)sleep(0);
  server.stopRpcService();
  client.stopRpcService();
}




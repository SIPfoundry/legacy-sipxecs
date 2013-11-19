
#include "WebRtcBridge.h"


#define APPLICATION_NAME "WebRtcBridge"
#define DEFAULT_ESL_ADDRESS "127.0.0.1"
#define DEFAULT_ESL_PORT 2022


using namespace sipx;
using namespace sipx::bridge;


WebRtcBridge::WebRtcBridge() :
  _pSwitch(0),
  _eslAddress(DEFAULT_ESL_ADDRESS),
  _eslPort(DEFAULT_ESL_PORT)
{
  _pSwitch = FreeSwitchRunner::instance();
  assert(_pSwitch);
}

WebRtcBridge::~WebRtcBridge()
{
  FreeSwitchRunner::delete_instance();
  _pSwitch = 0;
}

void WebRtcBridge::handleEvent(const EslConnection::Ptr& pConnection, const EslEvent::Ptr& pEvent)
{
  //
  // Check
  //
}

void WebRtcBridge::run(bool noconsole)
{
  //
  // Run the ESL Event Layer
  //
  if (!_eventListener.listenForEvents(boost::bind(&WebRtcBridge::handleEvent, this, _1, _2), _eslAddress, _eslPort))
    return;
  _eventListener.run();
  
  //
  // Run the switch
  //
  _pSwitch->setApplicationName(APPLICATION_NAME);
  _pSwitch->initialize();
  _pSwitch->run(noconsole);
}

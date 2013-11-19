

#ifndef WEBRTCBRIDGE_H_INCLUDED
#define	WEBRTCBRIDGE_H_INCLUDED

#include "sipx/bridge/FreeSwitchRunner.h"
#include "sipx/bridge/EslListener.h"


class WebRtcBridge 
{
public:
  WebRtcBridge();
  
  ~WebRtcBridge();
  
  sipx::bridge::FreeSwitchRunner& bridge();
  
  void run(bool noconsole);
  
  void setProxyAddress(const std::string& proxyAddress);
  
  const std::string& getProxyAddress() const;
  
  void setEslAddress(const std::string& address);
  
  const std::string& getEslAddress() const;

  void setEslPort(int eslPort);
  
  int getEslPort() const;
  
private:
  void handleEvent(const sipx::bridge::EslConnection::Ptr& pConnection, const sipx::bridge::EslEvent::Ptr& pEvent);
  sipx::bridge::EslListener _eventListener;
  sipx::bridge::FreeSwitchRunner* _pSwitch;
  std::string _proxyAddress;
  std::string _eslAddress;
  int _eslPort;
};

//
// Inlines
//
inline sipx::bridge::FreeSwitchRunner& WebRtcBridge::bridge()
{
  return *_pSwitch;
}

inline void WebRtcBridge::setProxyAddress(const std::string& proxyAddress)
{
  _proxyAddress = proxyAddress;
}

inline const std::string& WebRtcBridge::getProxyAddress() const
{
  return _proxyAddress;
}

inline void WebRtcBridge::setEslAddress(const std::string& address)
{
  _eslAddress = address;
}
  
inline const std::string& WebRtcBridge::getEslAddress() const
{
  return _eslAddress;
}

inline void WebRtcBridge::setEslPort(int eslPort)
{
  _eslPort = eslPort;
}
  
inline int WebRtcBridge::getEslPort() const
{
  return _eslPort;
}

#endif	



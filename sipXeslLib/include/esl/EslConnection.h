#ifndef ESL_CONNECTION_H_INLCUDED
#define	ESL_CONNECTION_H_INLCUDED

#include <boost/shared_ptr.hpp>
#include <esl/EslEvent.h>





class EslConnection : boost::noncopyable
{
 public:
  typedef void* ESL_HANDLE_T;
  typedef boost::shared_ptr<EslConnection> Ptr;

	EslConnection(const char *host, const char *port, const char *user, const char *password);
	EslConnection(const char *host, const char *port, const char *password);
	EslConnection(int socket);
	virtual ~EslConnection();
	int socketDescriptor();
	int connected();
	EslEvent::Ptr getInfo();
	int send(const char *cmd);
	EslEvent::Ptr sendRecv(const char *cmd);
	EslEvent::Ptr api(const char *cmd, const char *arg = 0);
	EslEvent::Ptr bgapi(const char *cmd, const char *arg = 0);
	EslEvent::Ptr sendEslEvent(EslEvent::Ptr send_me);
	EslEvent::Ptr recvEslEvent();
	EslEvent::Ptr recvEslEventTimed(int ms);
	EslEvent::Ptr filter(const char *header, const char *value);
	int events(const char *etype, const char *value);
  EslEvent::Ptr set(const char* setArg);
  EslEvent::Ptr export_var(const char* exportArg);
	EslEvent::Ptr execute(const char *app, const char *arg = 0, const char *uuid = 0);
	EslEvent::Ptr executeAsync(const char *app, const char *arg = 0, const char *uuid = 0);
	int setAsyncExecute(const char *val);
	int setEslEventLock(const char *val);
	int disconnect();
  bool hasInfoEslEvent();

//
// Application commands
//
  EslEvent::Ptr answerCall();
    /// Answer and incoming call

  EslEvent::Ptr bridgeToGateway(
    const std::string& gateway,
    const std::string& dialString,
    const std::string& domain,
    const std::string& toUri);
    /// Bridge the incoming call to a gateway.
    /// uuid, domain and toUri can be empty for inbound connections

  EslEvent::Ptr bridgeToUri(
    const std::string& uri,
    const std::string& sipProfile,
    const std::string& domain,
    const std::string& toUri);
    /// Bridge the incoming call to a uri using the sipProfile.
    /// uuid, domain and toUri can be empty for inbound connections

  EslEvent::Ptr bridgeToUriWithRingback(
    const std::string& uri,
    const std::string& sipProfile,
    const std::string& domain,
    const std::string& toUri);
    /// Same as bridgeToUri except ring back will be injected when
    /// alerting is received.  Take note that the call must be in answered
    /// state when using this method.  See answerCall() function.

  ESL_HANDLE_T _handle;
};

inline EslEvent::Ptr EslConnection::set(const char* setArg)
{
  return execute("set", setArg);
}

inline EslEvent::Ptr EslConnection::export_var(const char* exportArg)
{
  return execute("export", exportArg);
}



#endif /// ESL_CONNECTION_H_INLCUDED




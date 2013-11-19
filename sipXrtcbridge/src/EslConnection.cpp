

#include <sstream>
#include "sipx/bridge/EslConnection.h"

extern "C"
{
#include "esl.h"
}


namespace sipx {
namespace bridge {


#define connection_construct_common() { \
  esl_handle_t* handle = new esl_handle_t(); \
  memset(handle, 0, sizeof(*handle)); \
  _handle = handle; }
  
#define cast_handle() \
  esl_handle_t* h = (esl_handle_t*)_handle; \
  esl_handle_t& handle = *h;

EslConnection::EslConnection(const char *host, const char *port, const char *user, const char *password)
{
  connection_construct_common();
  esl_handle_t* handle = (esl_handle_t*)_handle;
  int x_port = atoi(port);
	esl_connect(handle, host, x_port, user, password);
}

EslConnection::EslConnection(const char *host, const char *port, const char *password)
{
  connection_construct_common();
  esl_handle_t* handle = (esl_handle_t*)_handle;
  int x_port = atoi(port);
	esl_connect(handle, host, x_port, 0, password);
}

EslConnection::EslConnection(int socket)
{
  connection_construct_common();
  esl_handle_t* handle = (esl_handle_t*)_handle;
  esl_attach_handle(handle, (esl_socket_t)socket, 0);
}

EslConnection::~EslConnection()
{
  disconnect();
  esl_handle_t* handle = (esl_handle_t*)_handle;
  delete handle;
  _handle = 0;
}

int EslConnection::socketDescriptor()
{
  cast_handle();

  if (handle.connected)
    return (int) handle.sock;
	return -1;
}

bool EslConnection::hasInfoEslEvent()
{
  cast_handle();
  return handle.info_event != 0;
}

int EslConnection::connected()
{
  cast_handle();
  return handle.connected;
}

EslEvent::Ptr EslConnection::getInfo()
{
  cast_handle();
  if (handle.connected && handle.info_event)
  {
		esl_event_t *event;
		esl_event_dup(&event, handle.info_event);
		return EslEvent::Ptr(new EslEvent(event, true));
	}
  return EslEvent::Ptr();
}

int EslConnection::send(const char *cmd)
{
  cast_handle();
  return esl_send(&handle, cmd);
  return 0;
}

EslEvent::Ptr EslConnection::sendRecv(const char *cmd)
{
  cast_handle();

	if (esl_send_recv(&handle, cmd) == ESL_SUCCESS)
  {
		esl_event_t *event;
		esl_event_dup(&event, handle.last_sr_event);
		return EslEvent::Ptr(new EslEvent(event, true));
	}
  return EslEvent::Ptr();
}

EslEvent::Ptr EslConnection::api(const char *cmd, const char *arg)
{
  size_t len;
	char *cmd_buf;

	if (!cmd)
		return EslEvent::Ptr();

	len = strlen(cmd) + (arg ? strlen(arg) : 0) + 10;

	cmd_buf = (char *) malloc(len + 1);
	assert(cmd_buf);

	snprintf(cmd_buf, len, "api %s %s", cmd, arg ? arg : "");
	*(cmd_buf + (len)) = '\0';

	EslEvent::Ptr event = sendRecv(cmd_buf);
	free(cmd_buf);
	return event;
}

EslEvent::Ptr EslConnection::bgapi(const char *cmd, const char *arg)
{
  size_t len;
	char *cmd_buf;

	if (!cmd) {
		return EslEvent::Ptr();
	}

	len = strlen(cmd) + (arg ? strlen(arg) : 0) + 10;

	cmd_buf = (char *) malloc(len + 1);
	assert(cmd_buf);

	snprintf(cmd_buf, len, "bgapi %s %s", cmd, arg ? arg : "");
	*(cmd_buf + (len)) = '\0';

	EslEvent::Ptr event = sendRecv(cmd_buf);
	free(cmd_buf);

	return event;
}

EslEvent::Ptr EslConnection::sendEslEvent(EslEvent::Ptr send_me)
{
  cast_handle();

  if (esl_sendevent(&handle, (esl_event_t*)send_me->getHandle()) == ESL_SUCCESS)
  {
		esl_event_t *e = handle.last_ievent ? handle.last_ievent : handle.last_event;
		if (e)
    {
			esl_event_t *event;
			esl_event_dup(&event, e);
			return EslEvent::Ptr(new EslEvent(event, true));
		}
	}

	return EslEvent::Ptr(new EslEvent("server_disconnected"));
}

EslEvent::Ptr EslConnection::recvEslEvent()
{
  cast_handle();
  if (esl_recv_event(&handle, 1, NULL) == ESL_SUCCESS) {
		esl_event_t *e = handle.last_ievent ? handle.last_ievent : handle.last_event;
		if (e) {
			esl_event_t *event;
			esl_event_dup(&event, e);
			return EslEvent::Ptr(new EslEvent(event, true));
		}
	}

	return EslEvent::Ptr(new EslEvent("server_disconnected"));
}

EslEvent::Ptr EslConnection::recvEslEventTimed(int ms)
{
  cast_handle();
  if (esl_recv_event_timed(&handle, ms, 1, NULL) == ESL_SUCCESS)
  {
		esl_event_t *e = handle.last_ievent ? handle.last_ievent : handle.last_event;
		if (e) {
			esl_event_t *event;
			esl_event_dup(&event, e);
			return EslEvent::Ptr(new EslEvent(event, true));
		}
  }
  return EslEvent::Ptr();
}

EslEvent::Ptr EslConnection::filter(const char *header, const char *value)
{
  cast_handle();

  esl_status_t status = esl_filter(&handle, header, value);

	if (status == ESL_SUCCESS && handle.last_sr_event)
  {
		esl_event_t *event;
		esl_event_dup(&event, handle.last_sr_event);
		return EslEvent::Ptr(new EslEvent(event, true));
	}

  return EslEvent::Ptr();
}

int EslConnection::events(const char *etype, const char *value)
{
  cast_handle();
  esl_event_type_t type_id = ESL_EVENT_TYPE_PLAIN;

	if (!strcmp(etype, "xml")) {
		type_id = ESL_EVENT_TYPE_XML;
	} else if (!strcmp(etype, "json")) {
       // type_id = ESL_EVENT_TYPE_JSON;
	}

	return esl_events(&handle, type_id, value);
}

EslEvent::Ptr EslConnection::execute(const char *app, const char *arg , const char *uuid)
{
  cast_handle();

  if (esl_execute(&handle, app, arg, uuid) == ESL_SUCCESS)
  {
		esl_event_t *event;
		esl_event_dup(&event, handle.last_sr_event);
		return EslEvent::Ptr(new EslEvent(event, true));
	}
  return EslEvent::Ptr();
}

EslEvent::Ptr EslConnection::executeAsync(const char *app, const char *arg, const char *uuid)
{
  cast_handle();

  int async = handle.async_execute;
	int r;

	handle.async_execute = 1;
	r = esl_execute(&handle, app, arg, uuid);
	handle.async_execute = async;

	if (r == ESL_SUCCESS) {
		esl_event_t *event;
		esl_event_dup(&event, handle.last_sr_event);
		return EslEvent::Ptr(new EslEvent(event, true));
	}

  return EslEvent::Ptr();
}

int EslConnection::setAsyncExecute(const char *val)
{
  cast_handle();
  if (val)
		handle.async_execute = esl_true(val);
	return handle.async_execute;
}

int EslConnection::setEslEventLock(const char *val)
{
  cast_handle();
  if (val)
		handle.event_lock = esl_true(val);
	return handle.event_lock;
}

int EslConnection::disconnect()
{
  cast_handle();

  if (handle.connected)
    return esl_disconnect(&handle);
	return 0;
}


//
// Application commands
//

EslEvent::Ptr EslConnection::answerCall()
{
  return execute("answer");
}

EslEvent::Ptr EslConnection::bridgeToGateway(
  const std::string& gateway,
  const std::string& dialString,
  const std::string& domain,
  const std::string& toUri)
{
  std::ostringstream arg;
  if (!toUri.empty())
    arg << "{sip_invite_to_uri=" << toUri << "}";

  if (!domain.empty())
    arg << "{sip_invite_domain=" << domain << "}";

  arg << "sofia/gateway/" << gateway << "/" << dialString;

  execute("set", "hangup_after_bridge=true");

  return execute("bridge", arg.str().c_str(), 0);
}

EslEvent::Ptr EslConnection::bridgeToUri(
  const std::string& uri,
  const std::string& sipProfile,
  const std::string& domain,
  const std::string& toUri)
{
  std::ostringstream arg;
  if (!toUri.empty())
    arg << "{sip_invite_to_uri=" << toUri << "}";

  if (!domain.empty())
    arg << "{sip_invite_domain=" << domain << "}";

  arg << "sofia/" << sipProfile << "/" << uri;

  execute("set", "hangup_after_bridge=true");

  return execute("bridge", arg.str().c_str(), 0);
}

EslEvent::Ptr EslConnection::bridgeToUriWithRingback(
  const std::string& uri,
  const std::string& sipProfile,
  const std::string& domain,
  const std::string& toUri)
{
  std::ostringstream arg;
  if (!toUri.empty())
    arg << "{sip_invite_to_uri=" << toUri << "}";

  if (!domain.empty())
    arg << "{sip_invite_domain=" << domain << "}";

  arg << "sofia/" << sipProfile << "/" << uri;

  execute("set", "hangup_after_bridge=true");
  execute("set", "ringback=${us-ring}");

  return execute("bridge", arg.str().c_str(), 0);
}


} } // sipx::bridge








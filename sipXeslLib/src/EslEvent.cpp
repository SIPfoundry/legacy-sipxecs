
#include <sstream>
#include <esl/EslEvent.h>

extern "C"
{
  #include "esl.h"
}



#define event_construct_common() _event = 0; _mine = false; _hp = 0
#define this_check(x) do { if (!this) { esl_log(ESL_LOG_ERROR, "object is not initalized\n"); return x;}} while(0)
#define this_check_void() do { if (!this) { esl_log(ESL_LOG_ERROR, "object is not initalized\n"); return;}} while(0)


template <size_t size>
bool string_sprintf_string(std::string& str, const char * format, ...)
  /// Write printf() style formatted data to string.
  ///
  /// The size template argument specifies the capacity of
  /// the internal buffer that will store the string result.
  /// Take note that if the buffer size is not enough
  /// the result of vsprintf will result to an overrun
  /// and will corrupt memory.
{
    char buffer[size];
    va_list args;
    va_start(args, format);
    int ret = vsprintf(buffer,format, args);
    va_end (args);
    if (ret >= 0)
    {
      str = buffer;
      return true;
    }
    return false;
}


static void escape(std::string& result, const char* _str, const char* validChars = 0)
{
  static const char * safeChars = "abcdefghijklmnopqrstuvwxyz"
                      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                      "0123456789$-_.!*'(),+#";

  int pos = -1;
  char* offSet = const_cast<char*>(_str);
  char* str = const_cast<char*>(_str);
  int len = strlen(str);

  std::string front;
  while ((pos += (int)(1+strspn(&str[pos+1], validChars == 0 ? safeChars : validChars))) < len)
  {
    std::string escaped;
    if (!string_sprintf_string<4>(escaped, "%%%02X", static_cast<const unsigned char>(str[pos])))
    {
      front = str;
      return;
    }
    front += std::string(offSet, str + pos );
    front += escaped;
    offSet = const_cast<char*>(str) + pos + 1;
  }
  front += std::string(offSet, str + pos );
  result = front;
}

EslEvent::EslEvent()
{
  event_construct_common();
}

EslEvent::EslEvent(const std::string& type, const std::string& subClass)
{
  esl_event_t* event = ((esl_event_t*)_event);
  esl_event_types_t event_id;

	event_construct_common();

  if (type == "jason" && !subClass.empty())
  {
		//if (esl_event_create_json(&event, subClass.c_str()) != ESL_SUCCESS)
		//	return;
		//event_id = event->event_id;
	}
  else
  {

		if (esl_name_event(type.c_str(), &event_id) != ESL_SUCCESS)
			event_id = ESL_EVENT_MESSAGE;

		if (!subClass.empty() && event_id != ESL_EVENT_CUSTOM)
    {
			esl_log(ESL_LOG_WARNING, "Changing event type to custom because you specified a subclass name!\n");
			event_id = ESL_EVENT_CUSTOM;
		}

		if (esl_event_create_subclass(&event, event_id, subClass.c_str()) != ESL_SUCCESS)
    {
			esl_log(ESL_LOG_ERROR, "Failed to create event!\n");
			_event = 0;
		}
	}

	_mine = true;
}

EslEvent::EslEvent(EslEventHandle eventHandle, bool autoDelete)
{
  event_construct_common();
  _mine = autoDelete;
	_event = eventHandle;
}

EslEvent::~EslEvent()
{

  esl_event_t* event = ((esl_event_t*)_event);
	if (event && _mine)
		esl_event_destroy(&event);
}



bool EslEvent::setPriority(int priority)
{
  esl_event_t* event = ((esl_event_t*)_event);

  this_check(false);

	if (event)
  {
    esl_event_set_priority(event, (esl_priority_t)priority);
		return true;
	}
  else
  {
		esl_log(ESL_LOG_ERROR, "Trying to setPriority an event that does not exist!\n");
  }
	return false;
}

std::string EslEvent::getHeader(const char* header_name)
{
  esl_event_t* event = ((esl_event_t*)_event);

  this_check("");

	if (event)
  {
		const char* header = esl_event_get_header(event, header_name);
    return header ? header : "";
  }
	else
		esl_log(ESL_LOG_ERROR, "Trying to getHeader an event that does not exist!\n");

	return "";
}

bool EslEvent::addHeader(const char* headerName, const char* value)
{
  esl_event_t* event = ((esl_event_t*)_event);
  this_check(false);

	if (event)
		return esl_event_add_header_string(event, ESL_STACK_BOTTOM, headerName, value) == ESL_SUCCESS ? true : false;
	else
		esl_log(ESL_LOG_ERROR, "Trying to addHeader an event that does not exist!\n");
	
	return false;
}

bool EslEvent::delHeader(const char* headerName)
{
  esl_event_t* event = ((esl_event_t*)_event);
  this_check(false);

	if (event)
		return esl_event_del_header(event, headerName) == ESL_SUCCESS ? true : false;
	else
		esl_log(ESL_LOG_ERROR, "Trying to delHeader an event that does not exist!\n");

	return false;
}

bool EslEvent::addBody(const char* value)
{
  esl_event_t* event = ((esl_event_t*)_event);
  this_check(false);

	if (event)
		return esl_event_add_body(event, "%s", value) == ESL_SUCCESS ? true : false;
	else
		esl_log(ESL_LOG_ERROR, "Trying to addBody an event that does not exist!\n");
	
	return false;
}


std::string EslEvent::getBody()
{
  esl_event_t* event = ((esl_event_t*)_event);

  this_check((char *)"");

	if (event)
  {
		const char* body = esl_event_get_body(event);
    return body ? body : "";
  }
	else
		esl_log(ESL_LOG_ERROR, "Trying to getBody an event that does not exist!\n");

	return "";
}

std::string EslEvent::getType()
{
  esl_event_t* event = ((esl_event_t*)_event);

  this_check("");

	if (event)
  {
		const char * type = esl_event_name(event->event_id);
    return type ? type : "";
  }else
		esl_log(ESL_LOG_ERROR, "Trying to getType an event that does not exist!\n");

	return (char *) "invalid";
}

const char* EslEvent::firstHeader()
{
  esl_event_t* event = ((esl_event_t*)_event);
  esl_event_header_t *hp;

  if (event)
		_hp = hp = event->headers;

	return nextHeader();
}

const char* EslEvent::nextHeader()
{
  esl_event_header_t* hp = (esl_event_header_t *)_hp;
  const char *name = 0;

	if (hp)
  {
		name = hp->name;
		hp = hp->next;
	}

	return name;
}

std::string EslEvent::serialize(const char* prefix)
{
  esl_event_t* event = ((esl_event_t*)_event);
  this_check("");

  std::ostringstream strm;
	esl_event_header_t *hp;

	/* esl_log_printf(ESL_CHANNEL_LOG, ESL_LOG_INFO, "hit serialized!.\n"); */
	for (hp = event->headers; hp; hp = hp->next)
  {
    //
    // We do not want the SDP.  It's multiline and it clutters loggin if prefix is set
    //
    if (prefix)
      strm << prefix;

    std::string headerName = hp->name;
    strm <<  headerName << ": ";
    std::string headerValue;
    if (headerName.find("_sdp") != std::string::npos)
      escape(headerValue, hp->value);
    else
      headerValue = hp->value;
    strm << headerValue << "\r\n";
	}

  if (event->body)
  {
    strm << "\r\n";
    if (prefix)
      strm << prefix;
    std::string body;
    escape(body, event->body);
    strm << body << "\r\n";
  }

	return strm.str();
}

std::string EslEvent::createLoggerData(const std::string& prefix)
{
  std::string cid = "\t\t";
  cid += prefix;
  cid += " ";

  std::ostringstream strm;
  strm << "\r\n" << "{" << "\r\n";
  strm << const_cast<EslEvent*>(this)->serialize(cid.c_str());
  strm << "\r\n" << "};";
  return strm.str();
}








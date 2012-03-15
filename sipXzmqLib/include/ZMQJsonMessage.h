/*
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */


#ifndef ZMQJSONMESSAGE_H
#define	ZMQJSONMESSAGE_H


#include <string>
#include "json/reader.h"
#include "json/writer.h"
#include "json/elements.h"


class ZMQJsonMessage : public json::Object
{
public:
  ZMQJsonMessage();
  ZMQJsonMessage(const ZMQJsonMessage& msg);
  ZMQJsonMessage(const std::string& msg);
  ~ZMQJsonMessage();

  ZMQJsonMessage& operator=(const ZMQJsonMessage& msg);
  ZMQJsonMessage& operator=(const std::string& msg);

  //  jsonrpc
  //
  //  A String specifying the version of the JSON-RPC protocol. MUST be exactly "2.0".
  //  if jsonrpc is missing, the server MAY handle the Request as JSON-RPC V1.0-Request.
  void setVersion(const std::string& version);
  std::string getVersion() const;

  //  method
  //
  //  A String containing the name of the procedure to be invoked.
  //  Procedure names that begin with the word rpc followed by a period character
  //  (U+002E or ASCII 46) are reserved for rpc-internal methods and extensions
  //  and MUST NOT be used for anything else.
  void setMethod(const std::string& method);
  std::string getMethod() const;

  //  id
  //
  //  A Request identifier that MUST be a JSON scalar (String, Number, True, False),
  //  but SHOULD normally not be Null [1], and Numbers SHOULD NOT contain fractional parts [2].
  //  If omitted, the Request is a Notification.
  //  This id can be used to correlate a Response with its Request.
  //  The server MUST reply with the same value.
  void setId(const std::string& id);
  std::string getId() const;

  //  params
  //
  //  An Object that holds the actual parameter values for the invocation
  //  of the procedure. Can be omitted if empty.
  void setParams(const json::Object& params);
  bool getParams(json::Object& params) const;

  //  result
  //
  //      Required on success, omitted on failure.
  //      An Object that was returned by the procedure. Its contents is entirely defined by the procedure.
  //      This member MUST be entirely omitted if there was an error invoking the procedure.
  void setResults(const json::Object& result);
  bool getResults(json::Object& result) const;

  //  error
  //
  //      Required on error, omitted on success.
  //      An Object containing error information about the fault that occurred before, during or after the call.
  //      This member MUST be entirely omitted if there was no such fault.
  //
  //      code
  //      A Number that indicates the actual error that occurred. This MUST be an integer.
  //
  //      message
  //      A String providing a short description of the error. The message SHOULD be limited to a concise single sentence.
  //
  //      data
  //      Additional information, may be omitted. Its contents is entirely defined by
  //      the application (e.g. detailed error information, nested errors etc
  //
  //      The error-codes -32768 .. -32000 (inclusive) are reserved for pre-defined errors.
  //      Any error-code within this range not defined explicitly below is reserved for future use. [3]
  //
  //      2700 	Parse error. 	Invalid JSON. An error occurred on the server while parsing the JSON text.
  //      2600 	Invalid Request. 	The received JSON is not a valid JSON-RPC Request.
  //      2601 	Method not found. 	The requested remote-procedure does not exist / is not available.
  //      2602 	Invalid params. 	Invalid method parameters.
  //      2603 	Internal error. 	Internal JSON-RPC error.
  //      2099..-32000 	Server error. 	Reserved for implementation-defined server-errors.
  //
  void setError(const json::Object& error);
  bool getError(json::Object& error) const;

  std::string data() const;

  bool getParameter(const std::string& name, std::string& value) const;
  bool getParameter(const std::string& name, double& value) const;
  bool getParameter(const std::string& name, int& value) const;
  bool getParameter(const std::string& name, bool& value) const;
  bool getParameter(const std::string& name, json::Object& value) const;
  bool getParameter(const std::string& name, json::Array& value) const;

  void setParameter(const std::string& name, const std::string& value);
  void setParameter(const std::string& name, const int& value);
  void setParameter(const std::string& name, const double& value);
  void setParameter(const std::string& name, const bool& value);
  void setParameter(const std::string& name, const json::Object& value);
  void setParameter(const std::string& name, const json::Array& value);

  bool getResult(const std::string& name, std::string& value) const;
  bool getResult(const std::string& name, double& value) const;
  bool getResult(const std::string& name, int& value) const;
  bool getResult(const std::string& name, bool& value) const;
  bool getResult(const std::string& name, json::Object& value) const;
  bool getResult(const std::string& name, json::Array& value) const;

  void setResult(const std::string& name, const std::string& value);
  void setResult(const std::string& name, const double& value);
  void setResult(const std::string& name, const int& value);
  void setResult(const std::string& name, const bool& value);
  void setResult(const std::string& name, const json::Object& value);
  void setResult(const std::string& name, const json::Array& value);


};


//
// Inlines
//

inline void ZMQJsonMessage::setParameter(const std::string& name, const int& value)
{
  setParameter(name, static_cast<const double&>(value));
}

inline bool ZMQJsonMessage::getParameter(const std::string& name, int& value) const
{
  double v;
  if (!getParameter(name, v))
    return false;
  int val = static_cast<int>(v + 0.5);
  value = val;
  return true;
}

inline void ZMQJsonMessage::setResult(const std::string& name, const int& value)
{
  setResult(name, static_cast<const double&>(value));
}

inline bool ZMQJsonMessage::getResult(const std::string& name, int& value) const
{
  double v;
  if (!getResult(name, v))
    return false;
  int val = static_cast<int>(v + 0.5);
  value = val;
  return true;
}

#endif	/* ZMQJSONMESSAGE_H */


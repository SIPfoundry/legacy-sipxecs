/*
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
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
#include "json/json_spirit.h"

typedef json_spirit::Value ZMQValue;
typedef json_spirit::Object ZMQObject;
typedef json_spirit::Array ZMQArray;
typedef json_spirit::Pair ZMQPair;

const int ZMQ_TYPE_OBJECT = json_spirit::obj_type ;
const int ZMQ_TYPE_ARRAY = json_spirit::array_type ;
const int ZMQ_TYPE_STRING = json_spirit::str_type;
const int ZMQ_TYPE_BOOL = json_spirit::bool_type ;
const int ZMQ_TYPE_INT = json_spirit::int_type ;
const int ZMQ_TYPE_REAL = json_spirit::real_type;
const int ZMQ_TYPE_NULL = json_spirit::null_type;

class ZMQJsonMessage : protected ZMQObject
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
  void setParams(const ZMQObject& params);
  bool getParams(ZMQObject& params) const;

  //  result
  //
  //      Required on success, omitted on failure.
  //      An Object that was returned by the procedure. Its contents is entirely defined by the procedure.
  //      This member MUST be entirely omitted if there was an error invoking the procedure.
  void setResults(const ZMQObject& result);
  bool getResults(ZMQObject& result) const;

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
  void setError(const ZMQObject& error);
  bool getError(ZMQObject& error) const;

  std::string data() const;

  bool getParameter(const std::string& name, std::string& value) const;
  bool getParameter(const std::string& name, int& value) const;
  bool getParameter(const std::string& name, double& value) const;
  bool getParameter(const std::string& name, bool& value) const;
  bool getParameter(const std::string& name, ZMQObject& value) const;
  bool getParameter(const std::string& name, ZMQArray& value) const;

  void setParameter(const std::string& name, const std::string& value);
  void setParameter(const std::string& name, const int& value);
  void setParameter(const std::string& name, const double& value);
  void setParameter(const std::string& name, const bool& value);
  void setParameter(const std::string& name, const ZMQObject& value);
  void setParameter(const std::string& name, const ZMQArray& value);

  bool getResult(const std::string& name, std::string& value) const;
  bool getResult(const std::string& name, int& value) const;
  bool getResult(const std::string& name, double& value) const;
  bool getResult(const std::string& name, bool& value) const;
  bool getResult(const std::string& name, ZMQObject& value) const;
  bool getResult(const std::string& name, ZMQArray& value) const;

  void setResult(const std::string& name, const std::string& value);
  void setResult(const std::string& name, const int& value);
  void setResult(const std::string& name, const double& value);
  void setResult(const std::string& name, const bool& value);
  void setResult(const std::string& name, const ZMQObject& value);
  void setResult(const std::string& name, const ZMQArray& value);


};


//
// Inlines
//

inline std::string ZMQJsonMessage::data() const
{
  return json_spirit::write(*this);
}


#endif	/* ZMQJSONMESSAGE_H */


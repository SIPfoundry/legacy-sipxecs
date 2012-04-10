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


#include "ZMQJsonMessage.h"
#include "ZMQLogger.h"

/*
const int ZMQ_TYPE_OBJECT = json_spirit::obj_type ;
const int type_array = json_spirit::array_type ;
const int ZMQ_TYPE_STRING = json_spirit::str_type;
const int type_bool = json_spirit::bool_type ;
const int type_int = json_spirit::int_type ;
const int type_real = json_spirit::real_type;
const int type_null = json_spirit::null_type;
*/
ZMQJsonMessage::ZMQJsonMessage()
{
}

ZMQJsonMessage::ZMQJsonMessage(const ZMQJsonMessage& msg) :
  Object(msg)
{
}

ZMQJsonMessage::ZMQJsonMessage(const std::string& msg)
{
  try
  {
    std::stringstream strm;
    strm << msg;
    json::Reader::Read(*this, strm);
  }
  catch(std::exception& error)
  {
    ZMQ_LOG_ERROR("ZMQJsonMessage::ZMQJsonMessage - PARSE ERROR - " << error.what());
  }
}

ZMQJsonMessage::~ZMQJsonMessage()
{
}

ZMQJsonMessage& ZMQJsonMessage::operator=(const ZMQJsonMessage& msg)
{
  *this = msg;
  return *this;
}

ZMQJsonMessage& ZMQJsonMessage::operator = (const std::string& msg)
{
  try
  {
    std::stringstream strm;
    strm << msg;
    json::Reader::Read(*this, strm);
  }
  catch(std::exception& error)
  {
    ZMQ_LOG_ERROR("ZMQJsonMessage::ZMQJsonMessage - PARSE ERROR - " << error.what());
  }
  return *this;
}


//  jsonrpc
//
//  A String specifying the version of the JSON-RPC protocol. MUST be exactly "2.0".
//  if jsonrpc is missing, the server MAY handle the Request as JSON-RPC V1.0-Request.
void ZMQJsonMessage::setVersion(const std::string& version)
{
  (*this)["jsonrpc"] = json::String(version);
}

std::string ZMQJsonMessage::getVersion() const
{
  json::Object::const_iterator iter = Find("jsonrpc");
  if (iter != End())
  {
    return static_cast<const std::string&>(static_cast<const json::String&>(iter->element));
  }
  return std::string();
}


//  method
//
//  A String containing the name of the procedure to be invoked.
//  Procedure names that begin with the word rpc followed by a period character
//  (U+002E or ASCII 46) are reserved for rpc-internal methods and extensions
//  and MUST NOT be used for anything else.
void ZMQJsonMessage::setMethod(const std::string& method)
{
  (*this)["method"] = json::String(method);
}

std::string ZMQJsonMessage::getMethod() const
{
  json::Object::const_iterator iter = Find("method");
  if (iter != End())
  {
    return static_cast<const std::string&>(static_cast<const json::String&>(iter->element));
  }
  return std::string();
}

//  id
//
//  A Request identifier that MUST be a JSON scalar (String, Number, True, False),
//  but SHOULD normally not be Null [1], and Numbers SHOULD NOT contain fractional parts [2].
//  If omitted, the Request is a Notification.
//  This id can be used to correlate a Response with its Request.
//  The server MUST reply with the same value.
void ZMQJsonMessage::setId(const std::string& id)
{
  (*this)["id"] = json::String(id);
}

std::string ZMQJsonMessage::getId() const
{
  json::Object::const_iterator iter = Find("id");
  if (iter != End())
  {
    return static_cast<const std::string&>(static_cast<const json::String&>(iter->element));
  }
  return std::string();
}

//  params
//
//  An Object that holds the actual parameter values for the invocation
//  of the procedure. Can be omitted if empty.
void ZMQJsonMessage::setParams(const json::Object& params)
{
  (*this)["params"] = params;
}

bool ZMQJsonMessage::getParams(json::Object& params) const
{
  json::Object::const_iterator iter = Find("params");
  if (iter != End())
  {
    params = static_cast<const json::Object&>(iter->element);
    return true;
  }
  return false;
}

//  result
//
//      Required on success, omitted on failure.
//      An Object that was returned by the procedure. Its contents is entirely defined by the procedure.
//      This member MUST be entirely omitted if there was an error invoking the procedure.
void ZMQJsonMessage::setResults(const json::Object& result)
{
  (*this)["result"] = result;
}

bool ZMQJsonMessage::getResults(json::Object& result) const
{
  json::Object::const_iterator iter = Find("result");
  if (iter != End())
  {
    result = static_cast<const json::Object&>(iter->element);
    return true;
  }
  return false;
}

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
void ZMQJsonMessage::setError(const json::Object& error)
{
  (*this)["error"] = error;
}

bool ZMQJsonMessage::getError(json::Object& error) const
{
  json::Object::const_iterator iter = Find("error");
  if (iter != End())
  {
    error = static_cast<const json::Object&>(iter->element);
    return true;
  }
  return false;
}

bool ZMQJsonMessage::getParameter(const std::string& name, std::string& value) const
{
  json::Object params;
  if (!getParams(params))
    return false;

  json::Object::const_iterator iter = params.Find(name);
  if (iter != End())
  {
    value = static_cast<const std::string&>(static_cast<const json::String&>(iter->element));
    return true;
  }

  return false;
}

bool ZMQJsonMessage::getParameter(const std::string& name, double& value) const
{
  json::Object params;
  if (!getParams(params))
    return false;

  json::Object::const_iterator iter = params.Find(name);
  if (iter != End())
  {
    value = static_cast<const double&>(static_cast<const json::Number&>(iter->element));
    return true;
  }

  return false;
}



bool ZMQJsonMessage::getParameter(const std::string& name, bool& value) const
{
  json::Object params;
  if (!getParams(params))
    return false;

  json::Object::const_iterator iter = params.Find(name);
  if (iter != End())
  {
    value = static_cast<const bool&>(static_cast<const json::Boolean&>(iter->element));
    return true;
  }

  return false;
}

bool ZMQJsonMessage::getParameter(const std::string& name, json::Object& value) const
{
  json::Object params;
  if (!getParams(params))
    return false;

  json::Object::const_iterator iter = params.Find(name);
  if (iter != End())
  {
    value = static_cast<const json::Object&>(iter->element);
    return true;
  }
  return false;
}

bool ZMQJsonMessage::getParameter(const std::string& name, json::Array& value) const
{
  json::Object params;
  if (!getParams(params))
    return false;
  json::Object::const_iterator iter = params.Find(name);
  if (iter != End())
  {
    value = static_cast<const json::Array&>(iter->element);
    return true;
  }
  return false;
}

void ZMQJsonMessage::setParameter(const std::string& name, const std::string& value)
{
  (*this)["params"][name] = json::String(value);
}

void ZMQJsonMessage::setParameter(const std::string& name, const double& value)
{
  (*this)["params"][name] = json::Number(value);
}

void ZMQJsonMessage::setParameter(const std::string& name, const bool& value)
{
  (*this)["params"][name] = json::Boolean(value);
}

void ZMQJsonMessage::setParameter(const std::string& name, const json::Object& value)
{
  (*this)["params"][name] = value;
}

void ZMQJsonMessage::setParameter(const std::string& name, const json::Array& value)
{
  (*this)["params"][name] = value;
}

bool ZMQJsonMessage::getResult(const std::string& name, std::string& value) const
{
  json::Object results;
  if (!getResults(results))
    return false;

  json::Object::const_iterator iter = Find(name);
  if (iter != End())
  {
    value = static_cast<const std::string&>(static_cast<const json::String&>(iter->element));
    return true;
  }

  return false;
}

bool ZMQJsonMessage::getResult(const std::string& name, double& value) const
{
  json::Object results;
  if (!getResults(results))
    return false;

  json::Object::const_iterator iter = results.Find(name);
  if (iter != End())
  {
    value = static_cast<const double&>(static_cast<const json::Number&>(iter->element));
    return true;
  }

  return false;
}



bool ZMQJsonMessage::getResult(const std::string& name, bool& value) const
{
  json::Object results;
  if (!getResults(results))
    return false;

  json::Object::const_iterator iter = results.Find(name);
  if (iter != End())
  {
    value = static_cast<const bool&>(static_cast<const json::Boolean&>(iter->element));
    return true;
  }

  return false;
}

bool ZMQJsonMessage::getResult(const std::string& name, json::Object& value) const
{
  json::Object results;
  if (!getResults(results))
    return false;

  json::Object::const_iterator iter = results.Find(name);
  if (iter != End())
  {
    value = static_cast<const json::Object&>(iter->element);
    return true;
  }
  return false;
}

bool ZMQJsonMessage::getResult(const std::string& name, json::Array& value) const
{
  json::Object results;
  if (!getResults(results))
    return false;
  json::Object::const_iterator iter = results.Find(name);
  if (iter != End())
  {
    value = static_cast<const json::Array&>(iter->element);
    return true;
  }
  return false;
}

void ZMQJsonMessage::setResult(const std::string& name, const std::string& value)
{
  (*this)["result"][name] = json::String(value);
}

void ZMQJsonMessage::setResult(const std::string& name, const double& value)
{
  (*this)["result"][name] = json::Number(value);
}

void ZMQJsonMessage::setResult(const std::string& name, const bool& value)
{
  (*this)["result"][name] = json::Boolean(value);
}

void ZMQJsonMessage::setResult(const std::string& name, const json::Object& value)
{
  (*this)["result"][name] = value;
}

void ZMQJsonMessage::setResult(const std::string& name, const json::Array& value)
{
  (*this)["result"][name] = value;
}


std::string ZMQJsonMessage::data() const
{
  try
  {
    std::ostringstream strm;
    json::Writer::Write(*this, strm);
    return strm.str();
  }
  catch(std::exception& error)
  {
    ZMQ_LOG_ERROR("ZMQJsonMessage::data - PARSE ERROR - " << error.what());
    return std::string();
  }
}
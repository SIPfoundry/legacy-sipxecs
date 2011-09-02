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


#include "ZMQJsonMessage.h"


using namespace json_spirit;

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
  Value value;
  read(msg, value);
  dynamic_cast<Object&>(*this) = value.get_obj();
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
  Value value;
  read(msg, value);
  dynamic_cast<Object&>(*this) = value.get_obj();
  return *this;
}


//  jsonrpc
//
//  A String specifying the version of the JSON-RPC protocol. MUST be exactly "2.0".
//  if jsonrpc is missing, the server MAY handle the Request as JSON-RPC V1.0-Request.
void ZMQJsonMessage::setVersion(const std::string& version)
{
  for( Object::size_type i = 0; i != this->size(); ++i )
  {
     if ((*this)[i].name_ == "jsonrpc")
     {
       (*this)[i].value_ = version;
       return;
     }
  }
   (*this).push_back(Pair("jsonrpc", version));
}

std::string ZMQJsonMessage::getVersion() const
{
  for( Object::size_type i = 0; i != (*this).size(); ++i )
  {
     if ((*this)[i].name_ == "jsonrpc"  && (*this)[i].value_.type() == ZMQ_TYPE_STRING)
     {
       return (*this)[i].value_.get_str();
     }
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
  for( Object::size_type i = 0; i != (*this).size(); ++i )
  {
     if ((*this)[i].name_ == "method")
     {
       (*this)[i].value_ = method;
       return;
     }
  }
   (*this).push_back(Pair("method", method));
}

std::string ZMQJsonMessage::getMethod() const
{
  for( Object::size_type i = 0; i != (*this).size(); ++i )
  {
     if ((*this)[i].name_ == "method" && (*this)[i].value_.type() == ZMQ_TYPE_STRING)
     {
       return (*this)[i].value_.get_str();
     }
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
  for( Object::size_type i = 0; i != (*this).size(); ++i )
  {
     if ((*this)[i].name_ == "id")
     {
       (*this)[i].value_ = id;
       return;
     }
  }
   (*this).push_back(Pair("id", id));
}

std::string ZMQJsonMessage::getId() const
{
  for( Object::size_type i = 0; i != (*this).size(); ++i )
  {
     if ((*this)[i].name_ == "id" && (*this)[i].value_.type() == ZMQ_TYPE_STRING)
     {
       return (*this)[i].value_.get_str();
     }
  }
  return std::string();
}

//  params
//
//  An Object that holds the actual parameter values for the invocation
//  of the procedure. Can be omitted if empty.
void ZMQJsonMessage::setParams(const ZMQObject& params)
{
  for( Object::size_type i = 0; i != (*this).size(); ++i )
  {
     if ((*this)[i].name_ == "params")
     {
       (*this)[i].value_ = params;
       return;
     }
  }
   (*this).push_back(Pair("params", params));
}

bool ZMQJsonMessage::getParams(ZMQObject& params) const
{
  for( Object::size_type i = 0; i != (*this).size(); ++i )
  {
     const Pair& pair = (*this)[i];
     const std::string& name  = pair.name_;
     const Value& value = pair.value_;

     if (name == "params" && value.type() == ZMQ_TYPE_OBJECT)
     {
       params = value.get_obj();
       return true;
     }
  }
  return false;
}

//  result
//
//      Required on success, omitted on failure.
//      An Object that was returned by the procedure. Its contents is entirely defined by the procedure.
//      This member MUST be entirely omitted if there was an error invoking the procedure.
void ZMQJsonMessage::setResults(const ZMQObject& result)
{
  for( Object::size_type i = 0; i != (*this).size(); ++i )
  {
     if ((*this)[i].name_ == "result")
     {
       (*this)[i].value_ = result;
       return;
     }
  }
   (*this).push_back(Pair("result", result));
}

bool ZMQJsonMessage::getResults(ZMQObject& result) const
{
  for( Object::size_type i = 0; i != (*this).size(); ++i )
  {
     const Pair& pair = (*this)[i];
     const std::string& name  = pair.name_;
     const Value& value = pair.value_;

     if (name == "result" && value.type() == ZMQ_TYPE_OBJECT)
     {
       result = value.get_obj();
       return true;
     }
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
void ZMQJsonMessage::setError(const ZMQObject& error)
{
  for( Object::size_type i = 0; i != (*this).size(); ++i )
  {
     if ((*this)[i].name_ == "error")
     {
       (*this)[i].value_ = error;
       return;
     }
  }
  (*this).push_back(Pair("result", error));
}

bool ZMQJsonMessage::getError(ZMQObject& error) const
{
  for( Object::size_type i = 0; i != (*this).size(); ++i )
  {
     const Pair& pair = (*this)[i];
     const std::string& name  = pair.name_;
     const Value& value = pair.value_;

     if (name == "error" && value.type() == ZMQ_TYPE_OBJECT)
     {
       error = value.get_obj();
       return true;
     }
  }
  return false;
}

#if 0
const int ZMQ_TYPE_OBJECT = json_spirit::obj_type ;
const int ZMQ_TYPE_ARRAY = json_spirit::array_type ;
const int ZMQ_TYPE_STRING = json_spirit::str_type;
const int ZMQ_TYPE_BOOL = json_spirit::bool_type ;
const int ZMQ_TYPE_INT = json_spirit::int_type ;
const int ZMQ_TYPE_REAL = json_spirit::real_type;
const int ZMQ_TYPE_NULL = json_spirit::null_type;
#endif

bool ZMQJsonMessage::getParameter(const std::string& name, std::string& value) const
{
  ZMQObject params;
  if (!getParams(params))
    return false;
  for( Object::size_type i = 0; i != params.size(); ++i )
  {
     if (params[i].name_ == name  && params[i].value_.type() == ZMQ_TYPE_STRING)
     {
       value = params[i].value_.get_str();
       return true;
     }
  }
  return false;
}

bool ZMQJsonMessage::getParameter(const std::string& name, int& value) const
{
  ZMQObject params;
  if (!getParams(params))
    return false;
  for( Object::size_type i = 0; i != params.size(); ++i )
  {
     if (params[i].name_ == name  && params[i].value_.type() == ZMQ_TYPE_INT)
     {
       value = params[i].value_.get_int();
       return true;
     }
  }
  return false;
}

bool ZMQJsonMessage::getParameter(const std::string& name, double& value) const
{
  ZMQObject params;
  if (!getParams(params))
    return false;
  for( Object::size_type i = 0; i != params.size(); ++i )
  {
     if (params[i].name_ == name  && params[i].value_.type() == ZMQ_TYPE_REAL)
     {
       value = params[i].value_.get_real();
       return true;
     }
  }
  return false;
}

bool ZMQJsonMessage::getParameter(const std::string& name, bool& value) const
{
  ZMQObject params;
  if (!getParams(params))
    return false;
  for( Object::size_type i = 0; i != params.size(); ++i )
  {
     if (params[i].name_ == name  && params[i].value_.type() == ZMQ_TYPE_BOOL)
     {
       value = params[i].value_.get_bool();
       return true;
     }
  }
  return false;
}

bool ZMQJsonMessage::getParameter(const std::string& name, ZMQObject& value) const
{
  ZMQObject params;
  if (!getParams(params))
    return false;
  for( Object::size_type i = 0; i != params.size(); ++i )
  {
     if (params[i].name_ == name  && params[i].value_.type() == ZMQ_TYPE_OBJECT)
     {
       value = params[i].value_.get_obj();
       return true;
     }
  }
  return false;
}

bool ZMQJsonMessage::getParameter(const std::string& name, ZMQArray& value) const
{
  ZMQObject params;
  if (!getParams(params))
    return false;
  for( Object::size_type i = 0; i != params.size(); ++i )
  {
     if (params[i].name_ == name  && params[i].value_.type() == ZMQ_TYPE_ARRAY)
     {
       value = params[i].value_.get_array();
       return true;
     }
  }
  return false;
}

void ZMQJsonMessage::setParameter(const std::string& name, const std::string& value)
{
  ZMQObject params;
  getParams(params);
  for( Object::size_type i = 0; i != params.size(); ++i )
  {
     if (params[i].name_ == name)
     {
       params[i].value_ = value;
       setParams(params);
       return;
     }
  }
  params.push_back(Pair(name, value));
  setParams(params);
}

void ZMQJsonMessage::setParameter(const std::string& name, const int& value)
{
  ZMQObject params;
  getParams(params);
  for( Object::size_type i = 0; i != params.size(); ++i )
  {
     if (params[i].name_ == name)
     {
       params[i].value_ = value;
       setParams(params);
       return;
     }
  }
  params.push_back(Pair(name, value));
  setParams(params);
}

void ZMQJsonMessage::setParameter(const std::string& name, const double& value)
{
  ZMQObject params;
  getParams(params);
  for( Object::size_type i = 0; i != params.size(); ++i )
  {
     if (params[i].name_ == name)
     {
       params[i].value_ = value;
       setParams(params);
       return;
     }
  }
  params.push_back(Pair(name, value));
  setParams(params);
}

void ZMQJsonMessage::setParameter(const std::string& name, const bool& value)
{
  ZMQObject params;
  getParams(params);
  for( Object::size_type i = 0; i != params.size(); ++i )
  {
     if (params[i].name_ == name)
     {
       params[i].value_ = value;
       setParams(params);
       return;
     }
  }
  params.push_back(Pair(name, value));
  setParams(params);
}

void ZMQJsonMessage::setParameter(const std::string& name, const ZMQObject& value)
{
  ZMQObject params;
  getParams(params);
  for( Object::size_type i = 0; i != params.size(); ++i )
  {
     if (params[i].name_ == name)
     {
       params[i].value_ = value;
       setParams(params);
       return;
     }
  }
  params.push_back(Pair(name, value));
  setParams(params);
}

void ZMQJsonMessage::setParameter(const std::string& name, const ZMQArray& value)
{
  ZMQObject params;
  getParams(params);
  for( Object::size_type i = 0; i != params.size(); ++i )
  {
     if (params[i].name_ == name)
     {
       params[i].value_ = value;
       setParams(params);
       return;
     }
  }
  params.push_back(Pair(name, value));
  setParams(params);
}

bool ZMQJsonMessage::getResult(const std::string& name, std::string& value) const
{
  ZMQObject results;
  if (!getResults(results))
    return false;
  for( Object::size_type i = 0; i != results.size(); ++i )
  {
     if (results[i].name_ == name  && results[i].value_.type() == ZMQ_TYPE_STRING)
     {
       value = results[i].value_.get_str();
       return true;
     }
  }
  return false;
}

bool ZMQJsonMessage::getResult(const std::string& name, int& value) const
{
  ZMQObject results;
  if (!getResults(results))
    return false;
  for( Object::size_type i = 0; i != results.size(); ++i )
  {
     if (results[i].name_ == name  && results[i].value_.type() == ZMQ_TYPE_INT)
     {
       value = results[i].value_.get_int();
       return true;
     }
  }
  return false;
}

bool ZMQJsonMessage::getResult(const std::string& name, double& value) const
{
  ZMQObject results;
  if (!getResults(results))
    return false;
  for( Object::size_type i = 0; i != results.size(); ++i )
  {
     if (results[i].name_ == name  && results[i].value_.type() == ZMQ_TYPE_REAL)
     {
       value = results[i].value_.get_real();
       return true;
     }
  }
  return false;
}

bool ZMQJsonMessage::getResult(const std::string& name, bool& value) const
{
  ZMQObject results;
  if (!getResults(results))
    return false;
  for( Object::size_type i = 0; i != results.size(); ++i )
  {
     if (results[i].name_ == name  && results[i].value_.type() == ZMQ_TYPE_BOOL)
     {
       value = results[i].value_.get_bool();
       return true;
     }
  }
  return false;
}

bool ZMQJsonMessage::getResult(const std::string& name, ZMQObject& value) const
{
  ZMQObject results;
  if (!getResults(results))
    return false;
  for( Object::size_type i = 0; i != results.size(); ++i )
  {
     if (results[i].name_ == name  && results[i].value_.type() == ZMQ_TYPE_OBJECT)
     {
       value = results[i].value_.get_obj();
       return true;
     }
  }
  return false;
}

bool ZMQJsonMessage::getResult(const std::string& name, ZMQArray& value) const
{
  ZMQObject results;
  if (!getResults(results))
    return false;
  for( Object::size_type i = 0; i != results.size(); ++i )
  {
     if (results[i].name_ == name  && results[i].value_.type() == ZMQ_TYPE_ARRAY)
     {
       value = results[i].value_.get_array();
       return true;
     }
  }
  return false;
}

void ZMQJsonMessage::setResult(const std::string& name, const std::string& value)
{
  ZMQObject results;
  getResults(results);
  for( Object::size_type i = 0; i != this->size(); ++i )
  {
     if (results[i].name_ == name)
     {
       results[i].value_ = value;
       setResults(results);
       return;
     }
  }
  results.push_back(Pair(name, value));
  setResults(results);
}

void ZMQJsonMessage::setResult(const std::string& name, const int& value)
{
  ZMQObject results;
  getResults(results);
  for( Object::size_type i = 0; i != results.size(); ++i )
  {
     if (results[i].name_ == name)
     {
       results[i].value_ = value;
       setResults(results);
       return;
     }
  }
  results.push_back(Pair(name, value));
  setResults(results);
}

void ZMQJsonMessage::setResult(const std::string& name, const double& value)
{
  ZMQObject results;
  getResults(results);
  for( Object::size_type i = 0; i != results.size(); ++i )
  {
     if (results[i].name_ == name)
     {
       results[i].value_ = value;
       setResults(results);
       return;
     }
  }
  results.push_back(Pair(name, value));
  setResults(results);
}

void ZMQJsonMessage::setResult(const std::string& name, const bool& value)
{
  ZMQObject results;
  getResults(results);
  for( Object::size_type i = 0; i != results.size(); ++i )
  {
     if (results[i].name_ == name)
     {
       results[i].value_ = value;
       setResults(results);
       return;
     }
  }
  results.push_back(Pair(name, value));
  setResults(results);
}

void ZMQJsonMessage::setResult(const std::string& name, const ZMQObject& value)
{
  ZMQObject results;
  getResults(results);
  for( Object::size_type i = 0; i != results.size(); ++i )
  {
     if (results[i].name_ == name)
     {
       results[i].value_ = value;
       setResults(results);
       return;
     }
  }
  results.push_back(Pair(name, value));
  setResults(results);
}

void ZMQJsonMessage::setResult(const std::string& name, const ZMQArray& value)
{
  ZMQObject results;
  getResults(results);
  for( Object::size_type i = 0; i != results.size(); ++i )
  {
     if (results[i].name_ == name)
     {
       results[i].value_ = value;
       setResults(results);
       return;
     }
  }
  results.push_back(Pair(name, value));
  setResults(results);
}


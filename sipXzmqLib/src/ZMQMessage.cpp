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


#include "ZMQMessage.h"
#include <iostream>
#include <zmq.hpp>
#include <boost/tokenizer.hpp>

ZMQMessage::ZMQMessage() :
  _isConsumed(false),
  _isInitialized(false)
{
}

ZMQMessage::~ZMQMessage()
{
}

ZMQMessage::ZMQMessage(const ZMQMessage& message) :
  _isConsumed(false),
  _isInitialized(false)
{
  _data = message._data;
}

ZMQMessage::ZMQMessage(const std::string& data) :
  _isConsumed(false),
  _isInitialized(false)
{
  _data = data;
}


void ZMQMessage::swap(ZMQMessage& message)
{
  std::swap(_data, message._data);
}

ZMQMessage& ZMQMessage::operator=(const ZMQMessage& message)
{
  ZMQMessage swapable(message);
  swap(swapable);
  return *this;
}

ZMQMessage& ZMQMessage::operator=(const std::string& data)
{
  _data = data;
  return *this;
}

void ZMQMessage::initData()
{
  if (!_isInitialized)
  {
    _message.rebuild(_data.size());
    memcpy(_message.data(), _data.data(), _data.size());
    _isInitialized = true;
  }
}

void ZMQMessage::initReceivedData()
{
  if (!_isInitialized)
  {
    _data = std::string(static_cast<char*>(_message.data()), _message.size());
    _isInitialized = true;
  }
}

bool ZMQMessage::isNull() const
{
  return false;
}

bool ZMQMessage::parseHeaders(ZMQMessage::Headers& headers) const
{
  //
  // Implementation specific of parsing the proprietary headers from the data of the message
  //
  if (_data.empty())
    return false;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> space(" ");
  tokenizer tokens(_data, space);
  int tokenCount = 0;
  for (tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
  {
    ++tokenCount;
    if ( tokenCount == 1)
      headers.identifier = *tok_iter;
    else if (tokenCount == 2)
      headers.address = *tok_iter;
    else
      break;
  }

  if (_data.size() > headers.identifier.size() + headers.address.size() + 2)
    headers.body = _data.substr(headers.identifier.size() + headers.address.size() + 2);

  return tokenCount >= 2;
}

bool ZMQMessage::encodeAsSuccessResponse()
{
  _data = "zmq::ok";
  return true;
}

bool ZMQMessage::encodeAsErrorResponse(int num, const std::string& what)
{
  std::ostringstream data;
  data << "zmq::error " << num << " " << what;
  _data = data.str();
  return true;
}

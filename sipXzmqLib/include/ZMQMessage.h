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

#ifndef ZMQMESSAGE_H_INCLUDED
#define ZMQMESSAGE_H_INCLUDED

#include <zmq.hpp>
#include <string>
#include <sstream>


class ZMQMessage
{
public:
  struct Headers
  {
    std::string identifier;
    std::string address;
    std::string body;
  };

  ZMQMessage();
  explicit ZMQMessage(const ZMQMessage& message);
  explicit ZMQMessage(const std::string& data);
  ~ZMQMessage();
  void swap(ZMQMessage& message);
  ZMQMessage& operator=(const ZMQMessage& message);
  ZMQMessage& operator=(const std::string& data);
  std::string& data();
  bool isConsumed() const;
  void setConsumed(bool consumed);
  zmq::message_t& zmqMessage();
  bool isNull() const;
  virtual bool parseHeaders(ZMQMessage::Headers& headers) const;
  virtual bool encodeAsSuccessResponse();
  virtual bool encodeAsErrorResponse(int num, const std::string& what);
protected:
  void initData();
  void initReceivedData();
  bool _isConsumed;
  bool _isInitialized;
private:
  zmq::message_t _message;
  std::string _data;
  friend class ZMQSocket;
};


//
// Inlines
//
inline std::string& ZMQMessage::data()
{
  return _data;
}

inline bool ZMQMessage::isConsumed() const
{
  return _isConsumed;
}

inline void ZMQMessage::setConsumed(bool consumed)
{
  _isConsumed = consumed;
}

inline zmq::message_t& ZMQMessage::zmqMessage()
{
  return _message;
}

#endif

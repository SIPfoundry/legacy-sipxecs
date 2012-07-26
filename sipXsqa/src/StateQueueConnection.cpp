/*
 * Copyright (c) eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */

#include "sqa/StateQueueConnection.h"
#include "sqa/StateQueueAgent.h"
#include "os/OsLogger.h"

StateQueueConnection::StateQueueConnection(
  boost::asio::io_service& ioService,
  StateQueueAgent& agent) :
  _ioService(ioService),
  _agent(agent),
  _socket(_ioService),
  _resolver(_ioService),
  _moreReadRequired(0),
  _lastExpectedPacketSize(0),
  _localPort(0),
  _remotePort(0),
  _pInactivityTimer(0)
{
  int expires = _agent.inactivityThreshold() * 1000;
  _pInactivityTimer = new boost::asio::deadline_timer(_ioService, boost::posix_time::milliseconds(expires));
  OS_LOG_DEBUG(FAC_NET, "StateQueueConnection CREATED.");
}

StateQueueConnection::~StateQueueConnection()
{
  delete _pInactivityTimer;
  stop();
  OS_LOG_DEBUG(FAC_NET, "StateQueueConnection DESTROYED.");
}

bool StateQueueConnection::write(const std::string& data)
{
  short version = 1;
  short key = 22172;
  short len = (short)data.size();
  std::stringstream strm;
  strm.write((char*)(&version), sizeof(version));
  strm.write((char*)(&key), sizeof(key));
  strm.write((char*)(&len), sizeof(len));
  strm << data;
  std::string packet = strm.str();
  boost::system::error_code ec;
  bool ok = _socket.write_some(boost::asio::buffer(packet.c_str(), packet.size()), ec) > 0;
  return ok;
}

void StateQueueConnection::handleRead(const boost::system::error_code& e, std::size_t bytes_transferred)
{
  if (!e && bytes_transferred)
  {
    try
    {
      if (!_localPort)
      {
        _localPort = _socket.local_endpoint().port();
        _localAddress = _socket.local_endpoint().address().to_string();
      }

      if (!_remotePort)
      {
        _remotePort = _socket.remote_endpoint().port();
        _remoteAddress = _socket.remote_endpoint().address().to_string();
      }
    }
    catch(...)
    {
      //
      //  Exception is non relevant if it is even thrown
      //
    }
    
    //
    // Start the inactivity timer
    //
    startInactivityTimer();

    OS_LOG_DEBUG(FAC_NET, "StateQueueConnection::handleRead"
            << " BYTES: " << bytes_transferred
            << " SRC: " << _localAddress << ":" << _localPort
            << " DST: " << _remoteAddress << ":" << _remotePort );

    if (_moreReadRequired == 0)
    {
      std::stringstream strm;
      if (!_spillOverBuffer.empty())
        strm << _spillOverBuffer;

 
      strm.write(_buffer.data(), bytes_transferred);

      if (!_spillOverBuffer.empty())
      {
        bytes_transferred += _spillOverBuffer.size();
        OS_LOG_DEBUG(FAC_NET,"StateQueueConnection::handleRead "
                << "Spillover buffer is not empty.  "
                << "Resulting BYTES: " << bytes_transferred);
      }

      short version;
      strm.read((char*)(&version), sizeof(version));
      short key;
      strm.read((char*)(&key), sizeof(key));
      if (version == 1 && key == 22172)
      {
        short len;
        strm.read((char*)(&len), sizeof(len));
        
        _lastExpectedPacketSize = len + sizeof(version) + sizeof(key) + (sizeof(len));
        _moreReadRequired =  bytes_transferred - _lastExpectedPacketSize;
        
        if (!_moreReadRequired)
        {
          char buf[8192];
          strm.read(buf, len);
          _agent.onIncomingRequest(*this, buf, len);
          _spillOverBuffer = std::string();
          if (_lastExpectedPacketSize < bytes_transferred)
          {
            //
            // We have spill over bytes
            //
            std::size_t extraBytes = bytes_transferred - _lastExpectedPacketSize;
            char spillbuf[extraBytes];
            strm.read(spillbuf, extraBytes);
            _spillOverBuffer = std::string(spillbuf, extraBytes);
            OS_LOG_DEBUG(FAC_NET,"StateQueueConnection::handleRead "
                << "Spillover bytes from last read detected.  "
                << "BYTES: " << extraBytes);
          }
        }
        else
        {
          _messageBuffer += _spillOverBuffer;
          _messageBuffer += std::string(_buffer.data(), bytes_transferred);
          _spillOverBuffer = std::string();
          OS_LOG_DEBUG(FAC_NET,"StateQueueConnection::handleRead "
                << "More bytes required to complete message.  "
                << "Required BYTES: " << _moreReadRequired);
        }
      }
    }
    else
    {
      readMore(bytes_transferred);
    }
  }
  else if (e)
  {
    OS_LOG_WARNING(FAC_NET, "StateQueueConnection::handleRead "
                << "TERMINATED - " << e.message() 
                << " Most likely remote closed the connection.");
    boost::system::error_code ignored_ec;
    _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
    _agent.listener().destroyConnection(shared_from_this());
    return;
  }

  start();
}

void StateQueueConnection::readMore(std::size_t bytes_transferred)
{
  _messageBuffer += std::string(_buffer.data(), bytes_transferred);

  std::stringstream strm;
  strm << _messageBuffer;

  short version;
  strm.read((char*)(&version), sizeof(version));
  short key;
  strm.read((char*)(&key), sizeof(key));
  if (version == 1 && key == 22172)
  {
    short len;
    strm.read((char*)(&len), sizeof(len));

    _lastExpectedPacketSize = len + sizeof(version) + sizeof(key) + (sizeof(len));
    if (_messageBuffer.size() < _lastExpectedPacketSize)
      _moreReadRequired = _lastExpectedPacketSize - _messageBuffer.size();
    else
      _moreReadRequired = 0;

    if (!_moreReadRequired)
    {
      char buf[8192];
      strm.read(buf, len);
      _agent.onIncomingRequest(*this, buf, len);
      _spillOverBuffer = std::string();
      if (_lastExpectedPacketSize < _messageBuffer.size())
      {
        //
        // We have spill over bytes
        //
        std::size_t extraBytes = _messageBuffer.size() - _lastExpectedPacketSize;
        char spillbuf[extraBytes];
        strm.read(spillbuf, extraBytes);
        _spillOverBuffer = std::string(spillbuf, extraBytes);
        OS_LOG_DEBUG(FAC_NET, "StateQueueConnection::readMore "
                << "Spillover bytes from last read detected.  "
                << "BYTES: " << extraBytes);
      }
    }
    else
    {
      OS_LOG_DEBUG(FAC_NET, "StateQueueConnection::readMore "
                << "More bytes required to complete message.  "
                << "Required BYTES: " << _moreReadRequired);
    }
  }
}

void StateQueueConnection::start()
{
  OS_LOG_DEBUG(FAC_NET, "StateQueueConnection::start() INVOKED");


  _socket.async_read_some(boost::asio::buffer(_buffer),
            boost::bind(&StateQueueConnection::handleRead, shared_from_this(),
              boost::asio::placeholders::error,
              boost::asio::placeholders::bytes_transferred));
}

void StateQueueConnection::stop()
{
  OS_LOG_DEBUG(FAC_NET, "StateQueueConnection::stop() INVOKED");
  _socket.close();
}

void StateQueueConnection::onInactivityTimeout(const boost::system::error_code& ec)
{
  if (!ec)
  {
    OS_LOG_WARNING(FAC_NET, "StateQueueConnection::onInactivityTimeout "
                  << "No activity on this socket for too long." );
    boost::system::error_code ignored_ec;
    _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
    _socket.close();
  }
}

void StateQueueConnection::startInactivityTimer()
{
  boost::system::error_code ignored_ec;
  _pInactivityTimer->cancel(ignored_ec);
  _pInactivityTimer->expires_from_now(boost::posix_time::milliseconds(_agent.inactivityThreshold() * 1000));
  _pInactivityTimer->async_wait(boost::bind(&StateQueueConnection::onInactivityTimeout, this, boost::asio::placeholders::error));

  OS_LOG_DEBUG(FAC_NET, "StateQueueConnection::startInactivityTimer "
          << " Session inactivity timeout set at " << _agent.inactivityThreshold() << " seconds.");
}
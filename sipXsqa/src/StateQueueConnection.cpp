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
#include "sqa/StateQueueClient.h"
#include "os/OsLogger.h"


// Minimum size of a message read from socket is:
//  - header size: version + key + len
//  - min body size: 1 byte
//  - 1 byte for characther '_';
const std::size_t g_minReadSize = sizeof(short) + sizeof(short) + sizeof(unsigned long) + 1 + 1 ;


StateQueueConnection::StateQueueConnection(
  boost::asio::io_service& ioService,
  StateQueueAgent& agent) :
  _ioService(ioService),
  _agent(agent),
  _socket(_ioService),
  _resolver(_ioService),
  _lastExpectedPacketSize(0),
  _localPort(0),
  _remotePort(0),
  _pInactivityTimer(0),
  _isAlphaConnection(false),
  _isCreationPublished(false),
  _isSocketClosed(false),
  _state(RequestHeader)
{
  int expires = _agent.inactivityThreshold() * 1000;
  _pInactivityTimer = new boost::asio::deadline_timer(_ioService, boost::posix_time::milliseconds(expires));

  OS_LOG_DEBUG(FAC_NET, "StateQueueConnection::StateQueueConnection this:" << this << "  CREATED.");
}

StateQueueConnection::~StateQueueConnection()
{
  delete _pInactivityTimer;

  stop();

  OS_LOG_DEBUG(FAC_NET, "StateQueueConnection::~StateQueueConnection this:" << this << " DESTROYED.");
}

bool StateQueueConnection::write(const std::string& data)
{
  short version = 1;
  short key = SQA_KEY_DEFAULT;
  unsigned long len = (short)data.size();
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

bool StateQueueConnection::consume(std::size_t bytes_transferred)
{
  std::stringstream strm;
  std::size_t strmSize = 0;

  if (!_messageBuffer.empty())
  {
    strm << _messageBuffer;
    strmSize += _messageBuffer.size();
    _messageBuffer.clear();
  }

  strm.write(_buffer.data(), bytes_transferred);
  strmSize += bytes_transferred;

  while (strmSize > 0)
  {
    switch (_state)
    {
    case RequestHeader:
    {
      if (strmSize < g_minReadSize)
      {
        char data[g_minReadSize];
        strm.read((char*)data, strmSize);
        _messageBuffer = std::string(data, strmSize);

        OS_LOG_INFO(FAC_NET,"StateQueueConnection::handleRead this:" << this
              << "More bytes required to complete message header.  "
              << " Read: " << bytes_transferred
              << " Required: " << g_minReadSize);

        return true;
      }

      strm.read((char*)(&_request.version), sizeof(_request.version));
      strm.read((char*)(&_request.key), sizeof(_request.key));
      strm.read((char*)(&_request.len), sizeof(_request.len));
      strmSize -= (sizeof(_request.version) + sizeof(_request.key) + sizeof(_request.len));

      // version and key are valid ?
      if (!(_request.version == 1 && _request.key >= SQA_KEY_MIN && _request.key <= SQA_KEY_MAX))
      {
        //
        // This is a corrupted message.  Simply reset the buffers
        //
        OS_LOG_WARNING(FAC_NET, "StateQueueConnection::handleRead this:" << this
                << "TERMINATED - "
                << " Invalid protocol headers.");
        return false;
      }

      _isAlphaConnection = (_request.key == SQA_KEY_ALPHA);

      //
      // Preserve the expected packet size
      //
      _lastExpectedPacketSize = _request.len + sizeof(_request.version) + sizeof(_request.key) + (sizeof(_request.len));
      if (_lastExpectedPacketSize > SQA_CONN_MAX_READ_BUFF_SIZE)
      {
        //
        // This is a corrupted message.  Simply reset the buffers
        //
        OS_LOG_WARNING(FAC_NET, "StateQueueConnection::handleRead this:" << this
            << "TERMINATED - "
            << " Message exceeds maximum buffer size. Lenth=" << _request.len );
        return false;
      }

      //fall down
      _state = RequestBody;
    }
    case RequestBody:
    {
      if (_request.len > strmSize)
      {
        char data[strmSize];
        strm.read((char*)data, strmSize);
        _messageBuffer = std::string(data, strmSize);

        OS_LOG_INFO(FAC_NET,"StateQueueConnection::handleRead this:" << this
              << "More bytes required to complete message.  "
              << " Read: " << bytes_transferred
              << " Required: " << (_request.len - strmSize)
              << " Expected: " << _lastExpectedPacketSize
              << " Data: " << _request.len);

        return true;
      }

      //
       // we dont need to read more.  there are enough in the buffer
       //
       strm.read(_request.data, _request.len);
       strmSize -= _request.len;

       //
       // Check terminating character to avoid corrupted/truncated/overrun messages
       //
       if (_request.data[_request.len-1] == '_')
       {
         _agent.onIncomingRequest(*this, _request.data, _request.len -1);
         _request.clear();
         _state = RequestHeader;
       }
       else // if (buf[len-1] == '_')
       {
         //
         // This is a corrupted message.  Simply reset the buffers
         //
         OS_LOG_WARNING(FAC_NET, "StateQueueConnection::handleRead this:" << this
             << "TERMINATED - "
             << " Message is not terminated with '_' character");

         return false;
       }
       //start over
       break;
    }
    default:
    {
      return false;
    }
    }
  }

  return true;
}

void StateQueueConnection::handleRead(const boost::system::error_code& e, std::size_t bytes_transferred)
{

  //close connections on error
  if (e)
  {
    if (boost::asio::error::eof == e)
    {
      OS_LOG_ERROR(FAC_NET, "BlockingTcpClient::handleRead this:" << this << " remote closed the connection: " << e.message());
    }
    else
    {
      OS_LOG_WARNING(FAC_NET, "StateQueueConnection::handleRead this:" << this
          << " TERMINATED - " << e.message());
    }

    _agent.onDestroyConnection(shared_from_this());
    if (e != boost::asio::error::operation_aborted)
    {
      abortRead();
    }
    return;
  }

  //
  // Start the inactivity timer
  //
  startInactivityTimer();

  initLocalAddressPort();
  OS_LOG_DEBUG(FAC_NET, "StateQueueConnection::handleRead this:" << this
          << " SRC: " << _localAddress << ":" << _localPort
          << " DST: " << _remoteAddress << ":" << _remotePort
          << " BYTES: " << bytes_transferred);

  bool result = consume(bytes_transferred);

  if (!result)
  {
    abortRead();
  }

  start();
}

void StateQueueConnection::initLocalAddressPort()
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
}

void StateQueueConnection::abortRead()
{
  OS_LOG_WARNING(FAC_NET, "StateQueueConnection::abortRead() this:" << this);

  _state = RequestHeader;
  _messageBuffer = std::string();
  _lastExpectedPacketSize = 0;

  _agent.listener().destroyConnection(shared_from_this());
}

void StateQueueConnection::start()
{
  OS_LOG_DEBUG(FAC_NET, "StateQueueConnection::start() this:" << this << " INVOKED");

  if (_socket.is_open())
  {
    _socket.async_read_some(boost::asio::buffer(_buffer),
              boost::bind(&StateQueueConnection::handleRead, shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
  }
}

void StateQueueConnection::stop()
{
  // _socket.isOpened() is not an option here because it does not return false
  // immediately after a socket is closed.
  // This will result in multiple attempts to close the socket coming from handleRead
  // and lots of TERMINATION log messages being logged.

  // _isSocketClosed is used to close the socket one time only. When the socket is
  // closed it will be set to true and subsequent attempts to close the socket will be
  // dismissed. Also there will be one handleRead call, and there will be only one
  // socket termination message.

  if (!_isSocketClosed)
  {
	  OS_LOG_DEBUG(FAC_NET, "StateQueueConnection::stop() this:" << this << " INVOKED");
	  _isSocketClosed = true;

	  boost::system::error_code ignored_ec;
	  _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
	  _socket.close(ignored_ec);
  }
}

void StateQueueConnection::onInactivityTimeout(const boost::system::error_code& ec)
{
  if (!ec)
  {
    OS_LOG_WARNING(FAC_NET, "StateQueueConnection::onInactivityTimeout this:" << this
                  << " No activity on this socket for too long." );

    stop();
    _agent.listener().destroyConnection(shared_from_this());
  }
}

void StateQueueConnection::startInactivityTimer()
{
  boost::system::error_code ignored_ec;
  _pInactivityTimer->cancel(ignored_ec);
  _pInactivityTimer->expires_from_now(boost::posix_time::milliseconds(_agent.inactivityThreshold() * 1000));
  _pInactivityTimer->async_wait(boost::bind(&StateQueueConnection::onInactivityTimeout, this, boost::asio::placeholders::error));

  OS_LOG_DEBUG(FAC_NET, "StateQueueConnection::startInactivityTimer this:" << this
          << " Session inactivity timeout set at " << _agent.inactivityThreshold() << " seconds.");
}

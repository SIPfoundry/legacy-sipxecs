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
  _pInactivityTimer(0),
  _isAlphaConnection(false),
  _isCreationPublished(false)
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

    //
    // The first read will always have _moreReadRequired as 0
    //
    if (_moreReadRequired == 0)
    {
      std::stringstream strm;

 
      strm.write(_buffer.data(), bytes_transferred);

      
      short version;
      strm.read((char*)(&version), sizeof(version));
      short key;
      strm.read((char*)(&key), sizeof(key));
      if (version == 1 && key >= SQA_KEY_MIN && key <= SQA_KEY_MAX)
      {
        _isAlphaConnection = (key == SQA_KEY_ALPHA);
        unsigned long len;
        strm.read((char*)(&len), sizeof(len));
        //
        // Preserve the expected packet size
        //
        _lastExpectedPacketSize = len + sizeof(version) + sizeof(key) + (sizeof(len));

        if (_lastExpectedPacketSize > bytes_transferred)
          _moreReadRequired =  _lastExpectedPacketSize - bytes_transferred;
        else
          _moreReadRequired = 0;

        //
        // we dont need to read more.  there are enough in the buffer
        //
        if (!_moreReadRequired)
        {
          if (len < SQA_CONN_MAX_READ_BUFF_SIZE)
          {
            char buf[SQA_CONN_MAX_READ_BUFF_SIZE];
            strm.read(buf, len);

            //
            // Check terminating character to avoid corrupted/truncated/overran messages
            //
            if (buf[len-1] == '_')
            {
              _agent.onIncomingRequest(*this, buf, len -1);

              if (_lastExpectedPacketSize < bytes_transferred)
              {
                //
                // We have spill over bytes
                //
                std::size_t extraBytes = bytes_transferred - _lastExpectedPacketSize;
                char spillbuf[extraBytes];
                strm.read(spillbuf, extraBytes);
                OS_LOG_DEBUG(FAC_NET, "StateQueueConnection::handleRead "
                    << "Spillover bytes from last read detected.  "
                    << " BYTES: " << extraBytes
                    << " Buffer: " << _messageBuffer.size()
                    << " Expected: " << _lastExpectedPacketSize
                    << " Data: " << len
                    << " Transferred: " << bytes_transferred);
                memcpy(_buffer.data(), (void*)spillbuf, extraBytes);
                _messageBuffer = std::string();
                handleRead(e, extraBytes);
                return;
              }
            }
            else
            {
              //
              // This is a corrupted message.  Simply reset the buffers
              //
              _messageBuffer = std::string();
              _lastExpectedPacketSize = 0;
              _moreReadRequired = 0;
              OS_LOG_WARNING(FAC_NET, "StateQueueConnection::handleRead "
                  << "TERMINATED - "
                  << " Message is not terminated with '_' character");
              boost::system::error_code ignored_ec;
              _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
              _socket.close(ignored_ec);
              _agent.listener().destroyConnection(shared_from_this());
              return;
            }
          }
          else
          {
            //
            // This is a corrupted message.  Simply reset the buffers
            //
            _messageBuffer = std::string();
            _lastExpectedPacketSize = 0;
            _moreReadRequired = 0;
            OS_LOG_WARNING(FAC_NET, "StateQueueConnection::handleRead "
                << "TERMINATED - "
                    << " Message exceeds maximum buffer size. Lenth=" << len );
            boost::system::error_code ignored_ec;
            _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
            _socket.close(ignored_ec);
            _agent.listener().destroyConnection(shared_from_this());
            return;
          }
        }
        else
        {
          _messageBuffer += std::string(_buffer.data(), bytes_transferred);
          OS_LOG_INFO(FAC_NET,"StateQueueConnection::handleRead "
                << "More bytes required to complete message.  "
                << " Read: " << bytes_transferred
                << " Required: " << _moreReadRequired
                << " Expected: " << _lastExpectedPacketSize
                << " Data: " << len);
        }
      }
      else
      {
        //
        // This is a corrupted message.  Simply reset the buffers
        //
        _messageBuffer = std::string();
        _lastExpectedPacketSize = 0;
        _moreReadRequired = 0;
        OS_LOG_WARNING(FAC_NET, "StateQueueConnection::handleRead "
                << "TERMINATED - "
                    << " Invalid protocol headers.");
        boost::system::error_code ignored_ec;
        _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
        _socket.close(ignored_ec);
        _agent.listener().destroyConnection(shared_from_this());
        return;
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
    _agent.onDestroyConnection(shared_from_this());
    return;
  }

  start();
}

void StateQueueConnection::readMore(std::size_t bytes_transferred)
{
  if (!bytes_transferred)
    return;

  _messageBuffer += std::string(_buffer.data(), bytes_transferred);

  std::stringstream strm;
  strm << _messageBuffer;

  short version;
  strm.read((char*)(&version), sizeof(version));
  short key;
  strm.read((char*)(&key), sizeof(key));
  if (version == 1 && key >= SQA_KEY_MIN && key <= SQA_KEY_MAX)
  {
    unsigned long len;
    strm.read((char*)(&len), sizeof(len));

    _lastExpectedPacketSize = len + sizeof(version) + sizeof(key) + (sizeof(len));
    if (_messageBuffer.size() < _lastExpectedPacketSize)
      _moreReadRequired = _lastExpectedPacketSize - _messageBuffer.size();
    else
      _moreReadRequired = 0;

    if (!_moreReadRequired)
    {
      if (len < SQA_CONN_MAX_READ_BUFF_SIZE)
      {
        char buf[SQA_CONN_MAX_READ_BUFF_SIZE];
        strm.read(buf, len);

        //
        // Check terminating character to avoid corrupted/truncated/overran messages
        //
        if (buf[len-1] == '_')
        {
          _agent.onIncomingRequest(*this, buf, len -1);

          if (_lastExpectedPacketSize < _messageBuffer.size())
          {
            //
            // We have spill over bytes
            //
            std::size_t extraBytes = _messageBuffer.size() - _lastExpectedPacketSize;
            char spillbuf[extraBytes];
            strm.read(spillbuf, extraBytes);
            OS_LOG_DEBUG(FAC_NET, "StateQueueConnection::readMore "
                << "Spillover bytes from last read detected.  "
                << " BYTES: " << extraBytes
                << " Buffer: " << _messageBuffer.size()
                << " Expected: " << _lastExpectedPacketSize
                << " Data: " << len
                << " Transferred: " << bytes_transferred);
            memcpy(_buffer.data(), (void*)spillbuf, extraBytes);
            boost::system::error_code dummy;
            _messageBuffer = std::string();
            handleRead(dummy, extraBytes);
            return;
          }
        }
        else
        {
          //
          // This is a corrupted message.  Simply reset the buffers
          //
          _messageBuffer = std::string();
          _lastExpectedPacketSize = 0;
          _moreReadRequired = 0;
          OS_LOG_WARNING(FAC_NET, "StateQueueConnection::handleRead "
              << "TERMINATED - "
              << " Message is not terminated with '_' character");
          boost::system::error_code ignored_ec;
          _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
          _socket.close(ignored_ec);
          _agent.listener().destroyConnection(shared_from_this());
          return;
        }
      }
      else
      {
        //
        // We got an extreme large len.
        // This is a corrupted message.  Simply reset the buffers
        //
        _messageBuffer = std::string();
        _lastExpectedPacketSize = 0;
        _moreReadRequired = 0;
        OS_LOG_WARNING(FAC_NET, "StateQueueConnection::handleRead "
                << "TERMINATED - "
                << " Message exceeds maximum buffer size. Lenth=" << len );
        boost::system::error_code ignored_ec;
        _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
        _socket.close(ignored_ec);
        _agent.listener().destroyConnection(shared_from_this());
        return;
      }
    }
    else
    {
      OS_LOG_INFO(FAC_NET,"StateQueueConnection::handleRead "
                << "More bytes required to complete message.  "
                << " Read: " << bytes_transferred
                << " Required: " << _moreReadRequired
                << " Expected: " << _lastExpectedPacketSize
                << " Data: " << len);
    }
  }
  else
  {
    //
    // This is a corrupted message.  Simply reset the buffers
    //
    _messageBuffer = std::string();
    _lastExpectedPacketSize = 0;
    _moreReadRequired = 0;
    OS_LOG_WARNING(FAC_NET, "StateQueueConnection::readMore "
                << "TERMINATED - "
                << " Invalid protocol headers." );
    boost::system::error_code ignored_ec;
    _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
    _socket.close(ignored_ec);
    _agent.listener().destroyConnection(shared_from_this());
    return;
  }
}

void StateQueueConnection::start()
{
  OS_LOG_DEBUG(FAC_NET, "StateQueueConnection::start() INVOKED");

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
  OS_LOG_DEBUG(FAC_NET, "StateQueueConnection::stop() INVOKED");
  boost::system::error_code ignored_ec;
  _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
  _socket.close(ignored_ec);
}

void StateQueueConnection::onInactivityTimeout(const boost::system::error_code& ec)
{
  if (!ec)
  {
    OS_LOG_WARNING(FAC_NET, "StateQueueConnection::onInactivityTimeout "
                  << "No activity on this socket for too long." );
    boost::system::error_code ignored_ec;
    _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
    _socket.close(ignored_ec);
    _agent.listener().destroyConnection(shared_from_this());
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
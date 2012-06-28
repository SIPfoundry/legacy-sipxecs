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

#ifndef StateQueueClient_H
#define	StateQueueClient_H

#include <cassert>
#include <zmq.hpp>
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include "StateQueueMessage.h"
#include "BlockingQueue.h"
#include "os/OsLogger.h"
#include <boost/lexical_cast.hpp>
#include "ServiceOptions.h"


class StateQueueClient : public boost::enable_shared_from_this<StateQueueClient>, private boost::noncopyable
{
public:

  enum Type
  {
    Publisher,
    Worker,
    Watcher
  };

  class BlockingTcpClient
  {
  public:
    typedef boost::shared_ptr<BlockingTcpClient> Ptr;
    BlockingTcpClient(boost::asio::io_service& ioService) :
      _ioService(ioService),
      _resolver(_ioService),
      _pSocket(0),
      _isConnected(false)
    {
    }

    ~BlockingTcpClient()
    {
      delete _pSocket;
    }

    bool connect(const std::string& serviceAddress, const std::string& servicePort)
    {
      if (_pSocket && _isConnected)
      {
        return true;
      }
      else if (_pSocket && !_isConnected)
      {
        boost::system::error_code ignored_ec;
       _pSocket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
      }

      delete _pSocket;
      _pSocket = new boost::asio::ip::tcp::socket(_ioService);

      _serviceAddress = serviceAddress;
      _servicePort = servicePort;

      try
      {
        boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), serviceAddress.c_str(), servicePort.c_str());
        boost::asio::ip::tcp::resolver::iterator hosts = _resolver.resolve(query);

        //////////////////////////////////////////////////////////////////////////
        // Only works in 1.47 version of asio.  1.46 doesnt have this utility func
        // boost::asio::connect(*_pSocket, hosts);
           _pSocket->connect(hosts->endpoint()); // so we use the connect member
        //////////////////////////////////////////////////////////////////////////

        _isConnected = true;
      }
      catch(...)
      {
        _isConnected = false;
      }

      return _isConnected;
    }

    bool connect()
    {
      //
      // Initialize State Queue Agent Publisher if an address is provided
      //
      std::string sqaControlAddress;
      std::string sqaControlPort;
      std::ostringstream sqaconfig;
      sqaconfig << SIPX_CONFDIR << "/" << "sipxsqa-client.ini";
      ServiceOptions configOptions(sqaconfig.str());
      std::string controlAddress;
      std::string controlPort;
      if (configOptions.parseOptions())
      {
        bool enabled = false;
        if (configOptions.getOption("enabled", enabled, enabled) && enabled)
        {
          configOptions.getOption("sqa-control-address", _serviceAddress);
          configOptions.getOption("sqa-control-port", _servicePort);
        }
        else
        {
          return false;
        }
      }

      if(_serviceAddress.empty() || _servicePort.empty())
        return false;
      
      return connect(_serviceAddress, _servicePort);
    }

    bool sendAndReceive(const StateQueueMessage& request, StateQueueMessage& response)
    {
      assert(_pSocket);
      std::string data = request.data();
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
      bool ok = _pSocket->write_some(boost::asio::buffer(packet.c_str(), packet.size()), ec) > 0;
      if (!ok)
      {
        _isConnected = false;
        return false;
      }

      len = getNextReadSize();
      if (!len)
        return false;

      char responseBuff[len];
      _pSocket->read_some(boost::asio::buffer((char*)responseBuff, len), ec);
      if (ec)
      {
        _isConnected = false;
        return false;
      }

      std::string responseData(responseBuff, len);
      return response.parseData(responseData);
    }

    short getNextReadSize()
    {
      short version = 1;
      short key = 22172;
      bool hasVersion = false;
      bool hasKey = false;
      short remoteLen = 0;
      while (!hasVersion || !hasKey)
      {
        short remoteVersion;
        short remoteKey;

        //
        // Read the version (must be 1)
        //
        while (true)
        {

          boost::system::error_code ec;
          _pSocket->read_some(boost::asio::buffer((char*)&remoteVersion, sizeof(remoteVersion)), ec);
          if (ec)
          {
            OS_LOG_DEBUG(FAC_NET, "StateQueueClient::getNextReadSize "
                    << "Unable to read version "
                    << "ERROR: " << ec.message());
            _isConnected = false;
            return 0;
          }
          else
          {
            if (remoteVersion == version)
            {
              hasVersion = true;
              break;
            }
          }
        }

        //
        // Read the key (must be 22172)
        //
        while (true)
        {

          boost::system::error_code ec;
          _pSocket->read_some(boost::asio::buffer((char*)&remoteKey, sizeof(remoteKey)), ec);
          if (ec)
          {
            OS_LOG_DEBUG(FAC_NET, "StateQueueClient::getNextReadSize "
                    << "Unable to read secret key "
                    << "ERROR: " << ec.message());
            _isConnected = false;
            return 0;
          }
          else
          {
            if (remoteKey == key)
            {
              hasKey = true;
              break;
            }
          }
        }
      }

      boost::system::error_code ec;
      _pSocket->read_some(boost::asio::buffer((char*)&remoteLen, sizeof(remoteLen)), ec);
      if (ec)
      {
        OS_LOG_DEBUG(FAC_NET, "StateQueueClient::getNextReadSize "
                << "Unable to read secret packet length "
                << "ERROR: " << ec.message());
        _isConnected = false;
        return 0;
      }

      return remoteLen;
    }

    bool isConnected() const
    {
      return _isConnected;
    }
  private:
    boost::asio::io_service& _ioService;
    boost::asio::ip::tcp::resolver _resolver;
    boost::asio::ip::tcp::socket *_pSocket;
    std::string _serviceAddress;
    std::string _servicePort;
    bool _isConnected;
    friend class StateQueueClient;
  };

protected:
  Type _type;
  typedef boost::recursive_mutex mutex;
  typedef boost::lock_guard<mutex> mutex_lock;
  boost::asio::io_service _ioService;
  std::size_t _poolSize;
  std::string _serviceAddress;
  std::string _servicePort;
  typedef BlockingQueue<BlockingTcpClient::Ptr> ClientPool;
  ClientPool _clientPool;
  bool _terminate;
  boost::thread _keepAliveThread;
  zmq::context_t _zmqContext;
  zmq::socket_t _zmqSocket;
  zmq::socket_t _zmqControlSocket;
  boost::thread* _pEventThread;
  std::string _zmqEventId;
  std::string _applicationId;
  typedef BlockingQueue<StateQueueMessage> EventQueue;
  EventQueue _eventQueue;
  std::vector<BlockingTcpClient*> _clientPointers;
  int _expires;
  int _subscriptionExpires;
  int _backoffCount;
  bool _refreshSignin;
  int _currentSigninTick;

public:
  StateQueueClient(
        Type type,
        const std::string& applicationId,
        const std::string& serviceAddress,
        const std::string& servicePort,
        const std::string& zmqEventId,
        std::size_t poolSize
        ) :
    _type(type),
    _ioService(),
    _poolSize(poolSize),
    _serviceAddress(serviceAddress),
    _servicePort(servicePort),
    _clientPool(_poolSize),
    _terminate(false),
    _keepAliveThread(boost::bind(&StateQueueClient::keepAliveLoop, this)),
    _zmqContext(1),
    _zmqSocket(_zmqContext,ZMQ_SUB),
    _zmqControlSocket(_zmqContext, ZMQ_PUB),
    _pEventThread(0),
    _applicationId(applicationId),
    _eventQueue(1000),
    _expires(10),
    _subscriptionExpires(1800),
    _backoffCount(0),
    _refreshSignin(false),
    _currentSigninTick(-1)
  {
      for (std::size_t i = 0; i < _poolSize; i++)
      {
        BlockingTcpClient* pClient = new BlockingTcpClient(_ioService);
        pClient->connect(_serviceAddress, _servicePort);
        _clientPointers.push_back(pClient);
        BlockingTcpClient::Ptr client(pClient);
        _clientPool.enqueue(client);
      }

      if (_type == Watcher)
        _zmqEventId = "sqw.";
      else
        _zmqEventId = "sqa.";

      _zmqEventId += zmqEventId;

      if (_type != Publisher)
      {
        std::ostringstream controlAddress;
        controlAddress << "inproc://" << this;
        _zmqControlSocket.bind(controlAddress.str().c_str());
        subscribe("_TERMINATE", controlAddress.str());
        _pEventThread = new boost::thread(boost::bind(&StateQueueClient::eventLoop, this));
      }
      else
      {
        std::string publisherAddress;
        signin(publisherAddress);
      }
  }

  StateQueueClient(
        Type type,
        const std::string& applicationId,
        const std::string& zmqEventId,
        std::size_t poolSize
        ) :
    _type(type),
    _ioService(),
    _poolSize(poolSize),
    _clientPool(_poolSize),
    _terminate(false),
    _keepAliveThread(boost::bind(&StateQueueClient::keepAliveLoop, this)),
    _zmqContext(1),
    _zmqSocket(_zmqContext,ZMQ_SUB),
    _zmqControlSocket(_zmqContext, ZMQ_PUB),
    _pEventThread(0),
    _applicationId(applicationId),
    _eventQueue(1000),
    _expires(10),
    _subscriptionExpires(1800),
    _backoffCount(0),
    _refreshSignin(false),
    _currentSigninTick(-1)
  {
      for (std::size_t i = 0; i < _poolSize; i++)
      {
        BlockingTcpClient* pClient = new BlockingTcpClient(_ioService);
        pClient->connect();
        _serviceAddress = pClient->_serviceAddress;
        _servicePort = pClient->_servicePort;
        _clientPointers.push_back(pClient);
        BlockingTcpClient::Ptr client(pClient);
        _clientPool.enqueue(client);
      }

      if (_type == Watcher)
        _zmqEventId = "sqw.";
      else
        _zmqEventId = "sqa.";

      _zmqEventId += zmqEventId;

      if (_type != Publisher)
      {
        std::ostringstream controlAddress;
        controlAddress << "inproc://" << this;
        _zmqControlSocket.bind(controlAddress.str().c_str());
        subscribe("_TERMINATE", controlAddress.str());
        _pEventThread = new boost::thread(boost::bind(&StateQueueClient::eventLoop, this));
      }
      else
      {
        std::string publisherAddress;
        signin(publisherAddress);
      }
  }

  ~StateQueueClient()
  {
    _terminate = true;
    _keepAliveThread.join();

    if (_type != Publisher)
    {
      zmq_sendmore(_zmqControlSocket, "_TERMINATE_");
      zmq_sendmore(_zmqControlSocket, "local");
      zmq_sendmore(_zmqControlSocket, "_TERMINATE_");
      zmq_send(_zmqControlSocket, "0");
    }

    if (_pEventThread)
    {
      _pEventThread->join();
      delete _pEventThread;
    }
  }

  void setExpires(int expires) { _expires = expires; }

  bool isConnected()
  {
    for (std::vector<BlockingTcpClient*>::const_iterator iter = _clientPointers.begin();
            iter != _clientPointers.end(); iter++)
    {
      BlockingTcpClient* pClient = *iter;
      if (pClient->isConnected())
        return true;
    }
    return false;
  }

private:
  void keepAliveLoop()
  {
    int sleepCount = 0;
    while (!_terminate)
    {
      if (++sleepCount < 30)
        boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
      else
        sleepCount = 0;
     
      if (_refreshSignin && (--_currentSigninTick == 0))
      {
        std::string publisherAddress;
        signin(publisherAddress);
        OS_LOG_INFO(FAC_NET, "StateQueueClient::keepAliveLoop refreshed signin @ " << publisherAddress);
      }

      if (!sleepCount)
      {
        //
        // send keep-alives
        //
        for (unsigned i = 0; i < _poolSize; i++)
        {
          StateQueueMessage ping;
          StateQueueMessage pong;
          ping.setType(StateQueueMessage::Ping);
          sendAndReceive(ping, pong);
          if (pong.getType() == StateQueueMessage::Pong)
            OS_LOG_DEBUG(FAC_NET, "Keep-alive response received from " << _serviceAddress << ":" << _servicePort);
        }
      }
    }
    OS_LOG_INFO(FAC_NET, "StateQueueClient::keepAliveLoop TERMIANTED.");
  }

  bool subscribe(const std::string& eventId, const std::string& sqaAddress)
  {
    assert(_type != Publisher);

    try
    {
    _zmqSocket.connect(sqaAddress.c_str());
    _zmqSocket.setsockopt(ZMQ_SUBSCRIBE, eventId.c_str(), eventId.size());
    }catch(...)
    {
      return false;
    }
    return true;
  }

  bool signin(std::string& publisherAddress)
  {
    std::ostringstream ss;
    ss << "sqa.signin."
       << std::hex << std::uppercase
       << std::setw(4) << std::setfill('0') << (int) ((float) (0x10000) * random () / (RAND_MAX + 1.0)) << "-"
       << std::setw(4) << std::setfill('0') << (int) ((float) (0x10000) * random () / (RAND_MAX + 1.0));

    std::string id = ss.str();

    StateQueueMessage request;
    request.setType(StateQueueMessage::Signin);
    request.set("message-id", id.c_str());
    request.set("message-app-id", _applicationId.c_str());
    request.set("subscription-expires", _subscriptionExpires);
    request.set("subscription-event", _zmqEventId.c_str());

    std::string clientType;
    if (_type == Publisher)
    {
      request.set("service-type", "publisher");
      clientType = "publisher";
    }
    else if (_type == Worker)
    {
      request.set("service-type", "worker");
      clientType = "worker";
    }
    else if (_type == Watcher)
    {
      request.set("service-type", "watcher");
      clientType = "watcher";
    }

    OS_LOG_NOTICE(FAC_NET, "StateQueueClient::signin Type=" << clientType << " SIGNIN");


    StateQueueMessage response;
    if (!sendAndReceive(request, response))
      return false;

    bool ok = response.get("message-data", publisherAddress);

    if (ok)
    {
      _refreshSignin = true;
      _currentSigninTick = _subscriptionExpires * .75;
      OS_LOG_NOTICE(FAC_NET, "StateQueueClient::signin Type=" << clientType << " SQA=" << publisherAddress << "SUCCEEDED");
    }

    return ok;
  }

  bool logout()
  {
    std::ostringstream ss;
    ss << "sqa.logout."
       << std::hex << std::uppercase
       << std::setw(4) << std::setfill('0') << (int) ((float) (0x10000) * random () / (RAND_MAX + 1.0)) << "-"
       << std::setw(4) << std::setfill('0') << (int) ((float) (0x10000) * random () / (RAND_MAX + 1.0));

    std::string id = ss.str();

    StateQueueMessage request;
    request.setType(StateQueueMessage::Logout);
    request.set("message-id", id.c_str());
    request.set("message-app-id", _applicationId.c_str());
    request.set("subscription-event", _zmqEventId.c_str());

    StateQueueMessage response;
    return sendAndReceive(request, response);
  }

  void eventLoop()
  {
    std::string publisherAddress;
    while(!_terminate)
    {
      if (signin(publisherAddress))
      {
        break;
      }
      else
      {
        OS_LOG_ERROR(FAC_NET, "StateQueueClient::eventLoop "
                  << "Is unable to SIGN IN to SQA service");
        boost::this_thread::sleep(boost::posix_time::microseconds(1000));
      }
    }

    if (_terminate)
      return;

    if (!subscribe(_zmqEventId, publisherAddress))
    {
      OS_LOG_ERROR(FAC_NET, "StateQueueClient::eventLoop "
                  << "Is unable to SUBSCRIBE to SQA service @ " << publisherAddress);
      return;
    }

    assert(_type != Publisher);
    bool firstHit = true;
    while (!_terminate)
    {
      std::string id;
      std::string data;
      int count = 0;
      if (readEvent(id, data, count))
      {
        if (_terminate)
          break;

        OS_LOG_INFO(FAC_NET, "StateQueueClient::eventLoop received event: " << id);
        OS_LOG_DEBUG(FAC_NET, "StateQueueClient::eventLoop received data: " << data);

        if (_type == Worker)
        {
          OS_LOG_DEBUG(FAC_NET, "StateQueueClient::eventLoop popping data: " << id);
          do_pop(firstHit, count, id, data);
        }else if (_type == Watcher)
        {
          OS_LOG_DEBUG(FAC_NET, "StateQueueClient::eventLoop watching data: " << id);
          do_watch(firstHit, count, id, data);
        }
      }
      firstHit = false;
    }

    logout();

    OS_LOG_INFO(FAC_NET, "StateQueueClient::eventLoop TERMINATED.");
  }

  void do_pop(bool firstHit, int count, const std::string& id, const std::string& data)
  {
    //
    // Check if we are the last succesful popper.
    // If count >= 2 we will skip the next message
    // after we have successfully popped
    //

    if (id.substr(0, 3) == "sqw")
      return;

    if (!firstHit && count >= 2 && _backoffCount < count - 1 )
    {
      _backoffCount++; //  this will ensure that we participate next time
      boost::this_thread::yield();
    }
    //
    // Check if we are in the exlude list
    //
    if (data.find(_applicationId.c_str()) != std::string::npos)
    {
      //
      // We are still considered the last popper so don't toggle?
      //
      _backoffCount++;
      return;
    }
    //
    // Pop it
    //
    StateQueueMessage pop;
    pop.setType(StateQueueMessage::Pop);
    pop.set("message-id", id.c_str());
    pop.set("message-app-id", _applicationId.c_str());
    pop.set("message-expires", _expires);

    OS_LOG_INFO(FAC_NET, "StateQueueClient::eventLoop " << _applicationId
              << " Popping event " << id);

    StateQueueMessage popResponse;
    if (!sendAndReceive(pop, popResponse))
    {
      _backoffCount++;
      return;
    }

    //
    // Check if Pop is successful
    //
    std::string messageResponse;
    popResponse.get("message-response", messageResponse);
    if (messageResponse != "ok")
    {
      std::string messageResponseError;
      popResponse.get("message-error", messageResponseError);
      OS_LOG_DEBUG(FAC_NET, "StateQueueClient::eventLoop "
              << "Dropping event " << id
              << " Error: " << messageResponseError);
      _backoffCount++;
    }
    else
    {
      std::string messageId;
      popResponse.get("message-id", messageId);

      OS_LOG_DEBUG(FAC_NET, "StateQueueClient::eventLoop "
              << "Popped event " << id);
      _eventQueue.enqueue(popResponse);
      _backoffCount = 0;
    }
  }
  
  void do_watch(bool firstHit, int count, const std::string& id, const std::string& data)
  {
    OS_LOG_DEBUG(FAC_NET, "StateQueueClient::eventLoop "<< "Received watcher data " << id);
    StateQueueMessage watcherData;
    watcherData.set("message-id", id);
    watcherData.set("message-data", data);
    _eventQueue.enqueue(watcherData);
  }

  bool readEvent(std::string& id, std::string& exclude, int& count)
  {
    assert(_type != Publisher);
    if (!zmq_receive(_zmqSocket, id))
      return false;

    std::string address;
    if (!zmq_receive(_zmqSocket, address))
      return false;

    //
    // Read the exclude vector
    //
    if (!zmq_receive(_zmqSocket, exclude))
      return false;

    //
    // Read number of subscribers active
    //
    std::string strcount;
    if (!zmq_receive(_zmqSocket, strcount))
      return false;

    try
    {
      count = boost::lexical_cast<int>(strcount);
    }
    catch(...)
    {
      return false;
    }
    return true;
  }

  bool zmq_send (zmq::socket_t & socket, const std::string & string) {

      zmq::message_t message(string.size());
      memcpy(message.data(), string.data(), string.size());

      bool rc = socket.send(message);
      return (rc);
  }

  bool zmq_sendmore (zmq::socket_t & socket, const std::string & string) {

      zmq::message_t message(string.size());
      memcpy(message.data(), string.data(), string.size());

      bool rc = socket.send(message, ZMQ_SNDMORE);
      return (rc);
  }

  bool zmq_receive (zmq::socket_t & socket, std::string& value)
  {
      zmq::message_t message;
      socket.recv(&message);
      if (!message.size())
        return false;
      value = std::string(static_cast<char*>(message.data()), message.size());
      return true;
  }

  bool sendAndReceive(const StateQueueMessage& request, StateQueueMessage& response)
  {
    BlockingTcpClient::Ptr conn;
    if (!_clientPool.dequeue(conn))
      return false;

    if (!conn->isConnected() && !conn->connect(_serviceAddress, _servicePort))
      return false;

    bool sent = conn->sendAndReceive(request, response);
    _clientPool.enqueue(conn);
    return sent;
  }

  bool pop(StateQueueMessage& ev)
  {
    return _eventQueue.dequeue(ev);
  }

  bool enqueue(const std::string& id, const std::string& data, int expires = 30, bool publish= false)
  {
    //
    // Enqueue it
    //
    StateQueueMessage enqueueRequest;
    if (!publish)
      enqueueRequest.setType(StateQueueMessage::Enqueue);
    else
      enqueueRequest.setType(StateQueueMessage::EnqueueAndPublish);
    enqueueRequest.set("message-id", id.c_str());
    enqueueRequest.set("message-app-id", _applicationId.c_str());
    enqueueRequest.set("message-expires", _expires);
    enqueueRequest.set("message-data", data);

    StateQueueMessage enqueueResponse;
    if (!sendAndReceive(enqueueRequest, enqueueResponse))
    {
      OS_LOG_ERROR(FAC_NET, "StateQueueClient::sendAndReceive FAILED");
      return false;
    }

    //
    // Check if Queue is successful
    //
    std::string messageResponse;
    enqueueResponse.get("message-response", messageResponse);
    if (messageResponse != "ok")
    {
      std::string messageResponseError;
      enqueueResponse.get("message-error", messageResponseError);
      OS_LOG_ERROR(FAC_NET, "StateQueueClient::enqueue "
                  << "Failed to enqueue " << id
                  << " Error: " << messageResponseError);
      return false;
    }

    return true;
  }

  bool internal_publish(const std::string& id, const std::string& data)
  {
    //
    // Enqueue it
    //
    StateQueueMessage enqueueRequest;
    enqueueRequest.setType(StateQueueMessage::Publish);
    enqueueRequest.set("message-id", id.c_str());
    enqueueRequest.set("message-app-id", _applicationId.c_str());
    enqueueRequest.set("message-data", data);

    OS_LOG_INFO(FAC_NET, "StateQueueClient::internal_publish "<< "publishing data ID=" << id);

    StateQueueMessage enqueueResponse;
    if (!sendAndReceive(enqueueRequest, enqueueResponse))
      return false;

    //
    // Check if Queue is successful
    //
    std::string messageResponse;
    enqueueResponse.get("message-response", messageResponse);
    if (messageResponse != "ok")
    {
      std::string messageResponseError;
      enqueueResponse.get("message-error", messageResponseError);
      OS_LOG_ERROR(FAC_NET, "StateQueueClient::internal_publish "
                  << "Failed to publish " << id
                  << " Error: " << messageResponseError);
      return false;
    }

    return true;
  }

  bool internal_publish_and_persist(int workspace, const std::string& id, const std::string& data, int expires)
  {
    StateQueueMessage enqueueRequest;
    enqueueRequest.setType(StateQueueMessage::PublishAndPersist);
    enqueueRequest.set("message-id", id.c_str());
    enqueueRequest.set("message-data-id", id.c_str());
    enqueueRequest.set("message-app-id", _applicationId.c_str());
    enqueueRequest.set("message-data", data);
    enqueueRequest.set("message-expires", expires);
    enqueueRequest.set("workspace", workspace);

    OS_LOG_INFO(FAC_NET, "StateQueueClient::internal_publish_and_persist "<< "publishing data ID=" << id);

    StateQueueMessage enqueueResponse;
    if (!sendAndReceive(enqueueRequest, enqueueResponse))
      return false;

    //
    // Check if Queue is successful
    //
    std::string messageResponse;
    enqueueResponse.get("message-response", messageResponse);
    if (messageResponse != "ok")
    {
      std::string messageResponseError;
      enqueueResponse.get("message-error", messageResponseError);
      OS_LOG_ERROR(FAC_NET, "StateQueueClient::internal_publish "
                  << "Failed to publish " << id
                  << " Error: " << messageResponseError);
      return false;
    }

    return true;
  }


public:
  
  bool pop(std::string& id, std::string& data)
  {
    StateQueueMessage message;
    if (!pop(message))
      return false;
    return message.get("message-id", id) && message.get("message-data", data);
  }

  bool watch(std::string& id, std::string& data)
  {
    StateQueueMessage message;
    if (!pop(message))
      return false;
    return message.get("message-id", id) && message.get("message-data", data);
  }

  bool enqueue(const std::string& data, int expires = 30, bool publish = false)
  {
    if (_type != Publisher)
      return false;

    std::ostringstream ss;
    ss << _zmqEventId << "."
       << std::hex << std::uppercase
       << std::setw(4) << std::setfill('0') << (int) ((float) (0x10000) * random () / (RAND_MAX + 1.0)) << "-"
       << std::setw(4) << std::setfill('0') << (int) ((float) (0x10000) * random () / (RAND_MAX + 1.0));
    return enqueue(ss.str(), data, expires, publish);
  }

  bool publish(const std::string& eventId, const std::string& data)
  {
    if (_type != Publisher)
      return false;

    std::ostringstream ss;
    ss << "sqw." << eventId << "."
       << std::hex << std::uppercase
       << std::setw(4) << std::setfill('0') << (int) ((float) (0x10000) * random () / (RAND_MAX + 1.0)) << "-"
       << std::setw(4) << std::setfill('0') << (int) ((float) (0x10000) * random () / (RAND_MAX + 1.0));
    return internal_publish(ss.str(), data);
  }

  bool publish(const std::string& eventId, const char* data, int dataLength)
  {
    std::string buff = std::string(data, dataLength);
    return publish(eventId, data, dataLength);
  }

  bool publishAndPersist(int workspace, const std::string& eventId, const std::string& data, int expires)
  {
    if (_type != Publisher)
      return false;

    std::ostringstream ss;
    ss << "sqw." << eventId;
    
    return internal_publish_and_persist(workspace, ss.str(), data, expires);
  }


  bool erase(const std::string& id)
  {
    StateQueueMessage request;
    request.setType(StateQueueMessage::Erase);
    request.set("message-id", id.c_str());
    request.set("message-app-id", _applicationId.c_str());

    StateQueueMessage response;
    if (!sendAndReceive(request, response))
      return false;

    std::string messageResponse;
    response.get("message-response", messageResponse);
    if (messageResponse != "ok")
    {
      std::string messageResponseError;
      response.get("message-error", messageResponseError);
      OS_LOG_ERROR(FAC_NET, "StateQueueClient::erase "
                  << "Failed to erase " << id
                  << " Error: " << messageResponseError);
      return false;
    }
    OS_LOG_ERROR(FAC_NET, "StateQueueClient::erase "
                  << "Successfully erased " << id);
    return true;
  }

  bool persist(int workspace, const std::string& dataId, int expires)
  {
    std::ostringstream ss;
    ss << "sqa.persist."
       << std::hex << std::uppercase
       << std::setw(4) << std::setfill('0') << (int) ((float) (0x10000) * random () / (RAND_MAX + 1.0)) << "-"
       << std::setw(4) << std::setfill('0') << (int) ((float) (0x10000) * random () / (RAND_MAX + 1.0));
    //
    // Enqueue it
    //
    StateQueueMessage request;
    request.setType(StateQueueMessage::Persist);
    request.set("message-id", ss.str().c_str());
    request.set("message-app-id", _applicationId.c_str());
    request.set("message-expires", _expires);
    request.set("workspace", workspace);
    request.set("message-data-id", dataId.c_str());

    StateQueueMessage response;
    if (!sendAndReceive(request, response))
      return false;

    std::string messageResponse;
    response.get("message-response", messageResponse);
    if (messageResponse != "ok")
    {
      std::string messageResponseError;
      response.get("message-error", messageResponseError);
      OS_LOG_ERROR(FAC_NET, "StateQueueClient::set "
                  << "Failed to enqueue " << dataId
                  << " Error: " << messageResponseError);
      return false;
    }

    return true;
  }

  bool set(int workspace, const std::string& dataId, const std::string& data, int expires)
  {
    std::ostringstream ss;
    ss << "sqa.set."
       << std::hex << std::uppercase
       << std::setw(4) << std::setfill('0') << (int) ((float) (0x10000) * random () / (RAND_MAX + 1.0)) << "-"
       << std::setw(4) << std::setfill('0') << (int) ((float) (0x10000) * random () / (RAND_MAX + 1.0));

    StateQueueMessage request;
    request.setType(StateQueueMessage::Set);
    request.set("message-id", ss.str().c_str());
    request.set("message-app-id", _applicationId.c_str());
    request.set("message-expires", _expires);
    request.set("workspace", workspace);
    request.set("message-data-id", dataId.c_str());
    request.set("message-data", data);

    StateQueueMessage response;
    if (!sendAndReceive(request, response))
      return false;

    std::string messageResponse;
    response.get("message-response", messageResponse);
    if (messageResponse != "ok")
    {
      std::string messageResponseError;
      response.get("message-error", messageResponseError);
      OS_LOG_ERROR(FAC_NET, "StateQueueClient::set "
                  << "Failed to enqueue " << dataId
                  << " Error: " << messageResponseError);
      return false;
    }

    return true;
  }

  bool mset(int workspace, const std::string& mapId, const std::string& dataId, const std::string& data, int expires)
  {
    std::ostringstream ss;
    ss << "sqa.mset."
       << std::hex << std::uppercase
       << std::setw(4) << std::setfill('0') << (int) ((float) (0x10000) * random () / (RAND_MAX + 1.0)) << "-"
       << std::setw(4) << std::setfill('0') << (int) ((float) (0x10000) * random () / (RAND_MAX + 1.0));

    StateQueueMessage request;
    request.setType(StateQueueMessage::MapSet);
    request.set("message-id", ss.str().c_str());
    request.set("message-app-id", _applicationId.c_str());
    request.set("message-expires", _expires);
    request.set("workspace", workspace);
    request.set("message-map-id", mapId.c_str());
    request.set("message-data-id", dataId.c_str());
    request.set("message-data", data);

    StateQueueMessage response;
    if (!sendAndReceive(request, response))
      return false;

    std::string messageResponse;
    response.get("message-response", messageResponse);
    if (messageResponse != "ok")
    {
      std::string messageResponseError;
      response.get("message-error", messageResponseError);
      OS_LOG_ERROR(FAC_NET, "StateQueueClient::mset "
                  << "Failed to enqueue " << dataId
                  << " Error: " << messageResponseError);
      return false;
    }

    return true;
  }

  bool get(int workspace, const std::string& dataId, std::string& data)
  {
    std::ostringstream ss;
    ss << "sqa.get."
       << std::hex << std::uppercase
       << std::setw(4) << std::setfill('0') << (int) ((float) (0x10000) * random () / (RAND_MAX + 1.0)) << "-"
       << std::setw(4) << std::setfill('0') << (int) ((float) (0x10000) * random () / (RAND_MAX + 1.0));
    //
    // Enqueue it
    //
    StateQueueMessage request;
    request.setType(StateQueueMessage::Get);
    request.set("message-id", ss.str().c_str());
    request.set("message-app-id", _applicationId.c_str());
    request.set("workspace", workspace);
    request.set("message-data-id", dataId.c_str());

    StateQueueMessage response;
    if (!sendAndReceive(request, response))
      return false;

    std::string messageResponse;
    response.get("message-response", messageResponse);
    if (messageResponse != "ok")
    {
      std::string messageResponseError;
      response.get("message-error", messageResponseError);
      OS_LOG_ERROR(FAC_NET, "StateQueueClient::get "
                  << "Failed to enqueue " << dataId
                  << " Error: " << messageResponseError);
      return false;
    }

    return response.get("message-data", data);
  }

  bool mget(int workspace, const std::string& mapId, const std::string& dataId, std::string& data)
  {
    std::ostringstream ss;
    ss << "sqa.get."
       << std::hex << std::uppercase
       << std::setw(4) << std::setfill('0') << (int) ((float) (0x10000) * random () / (RAND_MAX + 1.0)) << "-"
       << std::setw(4) << std::setfill('0') << (int) ((float) (0x10000) * random () / (RAND_MAX + 1.0));
    //
    // Enqueue it
    //
    StateQueueMessage request;
    request.setType(StateQueueMessage::MapGet);
    request.set("message-id", ss.str().c_str());
    request.set("message-app-id", _applicationId.c_str());
    request.set("workspace", workspace);
    request.set("message-map-id", mapId.c_str());
    request.set("message-data-id", dataId.c_str());

    StateQueueMessage response;
    if (!sendAndReceive(request, response))
      return false;

    std::string messageResponse;
    response.get("message-response", messageResponse);
    if (messageResponse != "ok")
    {
      std::string messageResponseError;
      response.get("message-error", messageResponseError);
      OS_LOG_ERROR(FAC_NET, "StateQueueClient::mget "
                  << "Failed to enqueue " << dataId
                  << " Error: " << messageResponseError);
      return false;
    }

    return response.get("message-data", data);
  }

  bool mgetm(int workspace, const std::string& mapId, std::map<std::string, std::string>& smap)
  {
    std::ostringstream ss;
    ss << "sqa.get."
       << std::hex << std::uppercase
       << std::setw(4) << std::setfill('0') << (int) ((float) (0x10000) * random () / (RAND_MAX + 1.0)) << "-"
       << std::setw(4) << std::setfill('0') << (int) ((float) (0x10000) * random () / (RAND_MAX + 1.0));
    //
    // Enqueue it
    //
    StateQueueMessage request;
    request.setType(StateQueueMessage::MapGetMultiple);
    request.set("message-id", ss.str().c_str());
    request.set("message-app-id", _applicationId.c_str());
    request.set("workspace", workspace);
    request.set("message-map-id", mapId.c_str());

    StateQueueMessage response;
    if (!sendAndReceive(request, response))
      return false;

    std::string messageResponse;
    response.get("message-response", messageResponse);
    if (messageResponse != "ok")
    {
      std::string messageResponseError;
      response.get("message-error", messageResponseError);
      OS_LOG_ERROR(FAC_NET, "StateQueueClient::mgetm "
                  << "Failed to enqueue " << mapId
                  << " Error: " << messageResponseError);
      return false;
    }

    std::string data;
    response.get("message-data", data);

    StateQueueMessage message;
    if (!message.parseData(data))
      return false;

    return message.getMap(smap);
  }

  bool mgeti(int workspace, const std::string& mapId, const std::string& dataId, std::string& data)
  {
    std::ostringstream ss;
    ss << "sqa.get."
       << std::hex << std::uppercase
       << std::setw(4) << std::setfill('0') << (int) ((float) (0x10000) * random () / (RAND_MAX + 1.0)) << "-"
       << std::setw(4) << std::setfill('0') << (int) ((float) (0x10000) * random () / (RAND_MAX + 1.0));
    //
    // Enqueue it
    //
    StateQueueMessage request;
    request.setType(StateQueueMessage::MapGetInc);
    request.set("message-id", ss.str().c_str());
    request.set("message-app-id", _applicationId.c_str());
    request.set("workspace", workspace);
    request.set("message-map-id", mapId.c_str());
    request.set("message-data-id", dataId.c_str());

    StateQueueMessage response;
    if (!sendAndReceive(request, response))
      return false;

    std::string messageResponse;
    response.get("message-response", messageResponse);
    if (messageResponse != "ok")
    {
      std::string messageResponseError;
      response.get("message-error", messageResponseError);
      OS_LOG_ERROR(FAC_NET, "StateQueueClient::mgeti "
                  << "Failed to enqueue " << dataId
                  << " Error: " << messageResponseError);
      return false;
    }

    return response.get("message-data", data);
  }

  bool remove(int workspace, const std::string& dataId)
  {
    std::ostringstream ss;
    ss << "sqa.remove."
       << std::hex << std::uppercase
       << std::setw(4) << std::setfill('0') << (int) ((float) (0x10000) * random () / (RAND_MAX + 1.0)) << "-"
       << std::setw(4) << std::setfill('0') << (int) ((float) (0x10000) * random () / (RAND_MAX + 1.0));
    //
    // Enqueue it
    //
    StateQueueMessage request;
    request.setType(StateQueueMessage::Remove);
    request.set("message-id", ss.str().c_str());
    request.set("message-app-id", _applicationId.c_str());
    request.set("message-data-id", dataId.c_str());
    request.set("workspace", workspace);

    StateQueueMessage response;
    if (!sendAndReceive(request, response))
      return false;

    std::string messageResponse;
    response.get("message-response", messageResponse);
    if (messageResponse != "ok")
    {
      std::string messageResponseError;
      response.get("message-error", messageResponseError);
      OS_LOG_ERROR(FAC_NET, "StateQueueClient::remove "
                  << "Failed to enqueue " << dataId
                  << " Error: " << messageResponseError);
      return false;
    }

    return true;
  }
};


#endif	/* StateQueueClient_H */


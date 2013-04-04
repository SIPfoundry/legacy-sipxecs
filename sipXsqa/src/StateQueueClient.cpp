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

#include "sqa/StateQueueClient.h"
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

StateQueueClient::SQAClientCore::SQAClientCore(
      StateQueueClient* owner,
      int idx,
      int type,
      const std::string& applicationId,
      const std::string& serviceAddress,
      const std::string& servicePort,
      const std::string& zmqEventId,
      std::size_t poolSize,
      int readTimeout,
      int writeTimeout,
      int keepAliveTimeout,
      int signinTimeout
      ) :
        _owner(owner),
        _idx(idx),
        _type(type),
        _keepAliveTimer(0),
        _signinTimer(0),
        _poolSize(poolSize),
        _serviceAddress(serviceAddress),
        _servicePort(servicePort),
        _clientPool(_poolSize),
        _clientPoolSize(0),
        _terminate(false),
        _zmqContext(0),
        _zmqSocket(0),
        _pEventThread(0),
        _zmqEventId(zmqEventId),
        _applicationId(applicationId),
        _expires(10),
        _backoffCount(0),
        _isAlive(true),
        _signinTimeout(signinTimeout),
        _signinState(SQAOpNotDone),
        _signinAttempts(0),
        _keepAliveTimeout(keepAliveTimeout),
        _keepAliveState(SQAOpNotDone),
        _keepAliveAttempts(0),
        _subscribeState(SQAOpNotDone),
        _subscribeAttempts(0)

{
  init(readTimeout, writeTimeout);
}

StateQueueClient::SQAClientCore::~SQAClientCore()
{
  terminate();
}

void StateQueueClient::SQAClientCore::terminate()
{
  if (_terminate)
    return;

  logout();

   _terminate = true;

   if (_zmqContext)
   {
     delete _zmqContext;
     _zmqContext = 0;
   }

  if (_zmqSocket)
  {
    delete _zmqSocket;
    _zmqSocket = 0;
  }

  OS_LOG_INFO(FAC_NET, LOG_TAG_WID(_applicationId)
      << " waiting for event thread to exit.");
  if (_pEventThread)
  {
    _pEventThread->join();
    delete _pEventThread;
    _pEventThread = 0;
  }

  if (_keepAliveTimer)
  {
    _keepAliveTimer->cancel();
    delete _keepAliveTimer;
    _keepAliveTimer = 0;
  }

  if (_signinTimer)
  {
    _signinTimer->cancel();
    delete _signinTimer;
    _signinTimer = 0;
  }

  _isAlive = false;

  OS_LOG_INFO(FAC_NET, LOG_TAG_WID(_applicationId)
      << "Ok");
}

bool StateQueueClient::SQAClientCore::isConnected()
{
  if (_terminate)
  {
    return false;
  }

  if (!_isAlive)
  {
    for (std::vector<BlockingTcpClient::Ptr>::const_iterator iter = _clientPointers.begin();
            iter != _clientPointers.end(); iter++)
    {
      BlockingTcpClient::Ptr pClient = *iter;
      if (pClient->isConnected())
        return true;
    }
  }

  return true;
}

bool StateQueueClient::SQAClientCore::isConnectedNow()
{
  if (_terminate)
  {
    return false;
  }

  StateQueueMessage ping;
  StateQueueMessage pong;
  ping.setType(StateQueueMessage::Ping);
  ping.set("message-app-id", _applicationId.c_str());

  OS_LOG_DEBUG(FAC_NET, LOG_TAG_WID(_applicationId)
      << " ping connectivity to '" << _serviceAddress << "' : '" << _servicePort << "'");
  if (sendAndReceive(ping, pong))
  {
    if (pong.getType() == StateQueueMessage::Pong)
    {
      OS_LOG_DEBUG(FAC_NET, LOG_TAG_WID(_applicationId)
          << " is connected to '" << _serviceAddress << "' : '" << _servicePort <<"'");
      return true;
    }
  }

  _isAlive = false;
  OS_LOG_DEBUG(FAC_NET, LOG_TAG_WID(_applicationId)
      << " is not connected to '" << _serviceAddress << "' : '" << _servicePort <<"'");
  return false;
}

void StateQueueClient::SQAClientCore::init(int readTimeout, int writeTimeout)
{
  OS_LOG_DEBUG(FAC_NET, LOG_TAG_WID(_applicationId)
      << " Starting SQA client core"
      << " idx '" << _idx << "'"
      << " type '" << SQAUtil::getClientStr(_type) << "'"
      << " zmqEventId '" << _zmqEventId << "'"
      << " serviceAddress '" << _serviceAddress << "'"
      << " servicePort '" << _servicePort << "'");


  if (!SQAUtil::isPublisher(_type))
  {
    _zmqContext = new zmq::context_t(1);
    _zmqSocket = new zmq::socket_t(*_zmqContext,ZMQ_SUB);
    int linger = SQA_LINGER_TIME_MILLIS; // milliseconds
    _zmqSocket->setsockopt(ZMQ_LINGER, &linger, sizeof(int));
  }

  for (std::size_t i = 0; i < _poolSize; i++)
  {
    //TODO: check _ioService ptr
    BlockingTcpClient* pClient = new BlockingTcpClient(_applicationId, *_owner->getIoService(), readTimeout, writeTimeout, i == 0 ? SQA_KEY_ALPHA : SQA_KEY_DEFAULT );
    pClient->connect(_serviceAddress, _servicePort);

    if (_localAddress.empty())
      _localAddress = pClient->getLocalAddress();

    BlockingTcpClient::Ptr client(pClient);
    _clientPointers.push_back(client);
    _clientPool.enqueue(client);
  }
  _clientPoolSize = _clientPool.size();

  signin();
  setSigninTimer();

  sendKeepAlive();
  setKeepAliveTimer();

  if (!SQAUtil::isPublisher(_type))
  {
    _pEventThread = new boost::thread(boost::bind(&SQAClientCore::eventLoop, this));
  }

  OS_LOG_INFO(FAC_NET, LOG_TAG_WID(_applicationId)
      << " SQA client core for zmqEventId: " <<  _zmqEventId << " CREATED");
}

bool StateQueueClient::SQAClientCore::sendKeepAlive()
{
  //
  // send keep-alives
  //
  for (unsigned i = 0; i < _poolSize; i++)
  {
    StateQueueMessage ping;
    StateQueueMessage pong;
    ping.setType(StateQueueMessage::Ping);
    ping.set("message-app-id", _applicationId.c_str());

    OS_LOG_DEBUG(FAC_NET, LOG_TAG_WID(_applicationId)
        << "send keep-alive ping to " << _serviceAddress << ":" << _servicePort);
    if (sendAndReceive(ping, pong))
    {
      if (pong.getType() == StateQueueMessage::Pong)
      {
        //
        // Reset it back to the default value
        //
        _isAlive = true;
        _keepAliveState = SQAOpOK;

        OS_LOG_DEBUG(FAC_NET, LOG_TAG_WID(_applicationId)
            << " Keep-alive response received from '" << _serviceAddress << "' : '" << _servicePort <<"'");
      }
    }
    else
    {
      //
      // Reset the keep-alive to 1 so we attempt to reconnect every second
      //
      _isAlive = false;
      _keepAliveState = SQAOpFailed;

      // force signin
      if (_signinTimer) // // timer created
      {
        if (_signinTimer->expires_from_now(boost::posix_time::seconds(_signinTimeout * .75)) > 0)
        {
          signin();
          setSigninTimer();
        }
      }

      OS_LOG_INFO(FAC_NET, LOG_TAG_WID(_applicationId)
          << " keep-alive failed to '" << _serviceAddress << "' : '" << _servicePort << "'");
    }
  }

    _keepAliveAttempts++;
    OS_LOG_DEBUG(FAC_NET, LOG_TAG_WID(_applicationId)
        << ", reschedule keep-alive in '" << _keepAliveTimeout << "'"
        << ", keep-alives so far '" << _keepAliveAttempts << "'");

    return _isAlive;
}

void StateQueueClient::SQAClientCore::setKeepAliveTimer()
{
  if (!_terminate)
  {
    if (!_keepAliveTimer)
    {
      _keepAliveTimer = new boost::asio::deadline_timer(*_owner->getIoService(), boost::posix_time::seconds(_keepAliveTimeout));
    }

    _keepAliveTimer->expires_from_now(boost::posix_time::seconds(_keepAliveTimeout));
    _keepAliveTimer->async_wait(boost::bind(&StateQueueClient::SQAClientCore::keepAliveLoop, this, boost::asio::placeholders::error));
  }
}

void StateQueueClient::SQAClientCore::keepAliveLoop(const boost::system::error_code& e)
{
  if (!e && !_terminate)
  {
    sendKeepAlive();

    setKeepAliveTimer();
  }
  else if (e)
  {
    OS_LOG_ERROR(FAC_NET, LOG_TAG_WID(_applicationId)
        << " keepAliveLoop timer failed with error: " <<  e.message());
  }
  else if (_terminate)
  {
    OS_LOG_INFO(FAC_NET, LOG_TAG_WID(_applicationId)
        << " terminate requested, keepAliveLoop aborted");
  }
}

bool StateQueueClient::SQAClientCore::subscribe(const std::string& eventId, const std::string& sqaAddress)
{
  assert(!SQAUtil::isPublisher(_type));

  OS_LOG_INFO(FAC_NET, LOG_TAG_WID(_applicationId)
      << " eventId '" << eventId << "' address '" << sqaAddress << "'");

  _subscribeAttempts++;

  try
  {
    _zmqSocket->connect(sqaAddress.c_str());
    _zmqSocket->setsockopt(ZMQ_SUBSCRIBE, eventId.c_str(), eventId.size());

  }catch(std::exception& e)
  {
    OS_LOG_INFO(FAC_NET, LOG_TAG_WID(_applicationId)
        << " eventId '" << eventId << "' address '" << sqaAddress << "' FAILED!  Error: " << e.what());
    _subscribeState = SQAOpFailed;
    return false;
  }

  _subscribeState = SQAOpOK;
  return true;
}

bool StateQueueClient::SQAClientCore::signin()
{
  StateQueueMessage request(StateQueueMessage::Signin, _type);
  request.set("message-app-id", _applicationId.c_str());
  request.set("subscription-expires", _signinTimeout);
  request.set("subscription-event", _zmqEventId.c_str());

  std::string clientType = SQAUtil::getClientStr(_type);
  request.set("service-type", clientType);

  OS_LOG_NOTICE(FAC_NET, LOG_TAG_WID(_applicationId)
      << " Type '" << clientType << "' SIGNIN");


  bool ok = false;
  StateQueueMessage response;
  if (sendAndReceive(request, response))
  {
    if (SQAUtil::isWatcher(_type))
    {
      ok = response.get("message-data", _publisherAddress);
    }
    else
    {
      ok = true;
    }
  }
  else
  {
    ok = false;
  }

  OS_LOG_NOTICE(FAC_NET, LOG_TAG_WID(_applicationId)
      << " Type '" << clientType << "' SQA '" << _publisherAddress << ((ok) ? "' SUCCEEDED" : "' FAILED"));


  if (ok)
  {
    _signinState = SQAOpOK;
  }
  else
  {
    _signinState = SQAOpFailed;
  }

  _signinAttempts++;
  OS_LOG_DEBUG(FAC_NET, LOG_TAG_WID(_applicationId)
      << ", reschedule signin in: " << _signinTimeout * .75
      << ", signin so far: " << _signinAttempts);

  return ok;
}

void StateQueueClient::SQAClientCore::setSigninTimer()
{
  if (!_terminate)
  {

    if (!_signinTimer)
    {
      _signinTimer = new boost::asio::deadline_timer(*_owner->getIoService(), boost::posix_time::seconds(_signinTimeout * .75));
    }

    _signinTimer->expires_from_now(boost::posix_time::seconds(_signinTimeout * .75));
    _signinTimer->async_wait(boost::bind(&StateQueueClient::SQAClientCore::signinLoop, this, boost::asio::placeholders::error));
  }
}

void StateQueueClient::SQAClientCore::signinLoop(const boost::system::error_code& e)
{
  if (!e && !_terminate)
  {
    signin();

    setSigninTimer();
  }
  else if (e)
  {
    OS_LOG_ERROR(FAC_NET, LOG_TAG_WID(_applicationId)
        << " signinLoop timer failed with error: " <<  e.message());
  }
  else if (_terminate)
  {
    OS_LOG_INFO(FAC_NET, LOG_TAG_WID(_applicationId)
        << " terminate requested, signinLoop aborted");
  }
}

bool StateQueueClient::SQAClientCore::logout()
{
  StateQueueMessage request(StateQueueMessage::Logout, _type);

  request.set("message-app-id", _applicationId.c_str());
  request.set("subscription-event", _zmqEventId.c_str());

  request.set("service-type", SQAUtil::getClientStr(_type));

  StateQueueMessage response;
  return sendAndReceive(request, response);
}

void StateQueueClient::SQAClientCore::eventLoop()
{
  const int retryTime = 500; //ms
  while(!_terminate)
  {
    if (SQAOpOK == _signinState)
    {
      break;
    }
    else
    {
      OS_LOG_WARNING(FAC_NET, LOG_TAG_WID(_applicationId)
          << " Network Queue did no respond.  Retrying SIGN IN after '" << retryTime << "' ms.");
      boost::this_thread::sleep(boost::posix_time::milliseconds(retryTime));
    }
  }

  bool firstHit = true;
  if (!_terminate)
  {
    while (!_terminate)
    {
      if (subscribe(_zmqEventId, _publisherAddress))
      {
        break;
      }
      else
      {
        OS_LOG_ERROR(FAC_NET, LOG_TAG_WID(_applicationId)
            << " Is unable to SUBSCRIBE to SQA service '" << _publisherAddress << "'");
        boost::this_thread::sleep(boost::posix_time::milliseconds(retryTime));
      }
    }

    assert(!SQAUtil::isPublisher(_type));

    while (!_terminate)
    {
      std::string id;
      std::string data;
      int count = 0;
      if (readEvent(id, data, count))
      {
        if (_terminate)
          break;

        OS_LOG_INFO(FAC_NET, LOG_TAG_WID(_applicationId)
            << "received event: " << id);
        OS_LOG_DEBUG(FAC_NET, LOG_TAG_WID(_applicationId)
            << " received data: " << data);

        if (SQAUtil::isWorker(_type))
        {
          OS_LOG_DEBUG(FAC_NET, LOG_TAG_WID(_applicationId)
              << "popping data: " << id);
          do_pop(firstHit, count, id, data);
        }else if (SQAUtil::isWatcherOnly(_type))
        {
          OS_LOG_DEBUG(FAC_NET, LOG_TAG_WID(_applicationId)
              << " watching data: " << id);
          do_watch(firstHit, count, id, data);
        }
      }
      else
      {
        if (_terminate)
        {
          break;
        }
      }
      firstHit = false;
    }
  }

  if (SQAUtil::isWatcherOnly(_type))
  {
    do_watch(firstHit, 0, SQA_TERMINATE_STRING, SQA_TERMINATE_STRING);
  }
  else if (SQAUtil::isWorker(_type))
  {
    do_pop(firstHit, 0, SQA_TERMINATE_STRING, SQA_TERMINATE_STRING);
  }

  _zmqSocket->close();

  OS_LOG_INFO(FAC_NET, LOG_TAG_WID(_applicationId)
      << " TERMINATED.");
}

void StateQueueClient::SQAClientCore::do_pop(bool firstHit, int count, const std::string& id, const std::string& data)
{
  //
  // Check if we are the last succesful popper.
  // If count >= 2 we will skip the next message
  // after we have successfully popped
  //

  if (id.substr(0, 3) == "sqw")
  {
    OS_LOG_WARNING(FAC_NET, LOG_TAG_WID(_applicationId)
        << " do_pop dropping event " << id);
    return;
  }

  if (id == "__TERMINATE__")
  {
    StateQueueMessage terminate;
    terminate.setType(StateQueueMessage::Pop);
    terminate.set("message-id", "__TERMINATE__");
    terminate.set("message-data", "__TERMINATE__");
    _owner->getEventQueue()->enqueue(terminate.data());
    return;
  }

  if (!firstHit && count >= 2 && _backoffCount < count - 1 )
  {
    _backoffCount++; //  this will ensure that we participate next time
    boost::this_thread::yield();
  }
  //
  // Check if we are in the exclude list
  //
  if (data.find(_applicationId.c_str()) != std::string::npos)
  {
    //
    // We are still considered the last popper so don't toggle?
    //
    _backoffCount++;
    OS_LOG_DEBUG(FAC_NET, LOG_TAG_WID(_applicationId)
        << " do_pop is not allowed to pop " << id);
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

  OS_LOG_INFO(FAC_NET, LOG_TAG_WID(_applicationId)
      << " Popping event " << id);

  StateQueueMessage popResponse;
  if (!sendAndReceive(pop, popResponse))
  {
    OS_LOG_ERROR(FAC_NET, LOG_TAG_WID(_applicationId)
        << " do_pop unable to send pop command for event " << id);
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
    OS_LOG_DEBUG(FAC_NET, LOG_TAG_WID(_applicationId)
            << " Dropping event " << id
            << " Error: " << messageResponseError);
    _backoffCount++;
  }
  else
  {
    std::string messageId;
    popResponse.get("message-id", messageId);
    std::string messageData;
    popResponse.get("message-data", messageData);
    popResponse.set("service-id", _idx);

    OS_LOG_DEBUG(FAC_NET, LOG_TAG_WID(_applicationId)
            << " Popped event " << messageId << " -- " << messageData);
    _owner->getEventQueue()->enqueue(popResponse.data());
    _backoffCount = 0;
  }
}

void StateQueueClient::SQAClientCore::do_watch(bool firstHit, int count, const std::string& id, const std::string& data)
{
  OS_LOG_DEBUG(FAC_NET, LOG_TAG_WID(_applicationId)
      << "Received watcher data " << id);
  StateQueueMessage watcherData;
  watcherData.set("message-id", id);
  watcherData.set("message-data", data);
  watcherData.set("service-id", _idx);
  _owner->getEventQueue()->enqueue(watcherData.data());
}

bool StateQueueClient::SQAClientCore::readEvent(std::string& id, std::string& data, int& count)
{
  assert(!SQAUtil::isPublisher(_type));

  try
  {
    if (!zmq_receive(_zmqSocket, id))
    {
      OS_LOG_ERROR(FAC_NET, LOG_TAG_WID(_applicationId)
          << "0mq failed to receive ID segment.");
      return false;
    }

    std::string address;
    if (!zmq_receive(_zmqSocket, address))
    {
      OS_LOG_ERROR(FAC_NET, LOG_TAG_WID(_applicationId)
          << "0mq failed to receive ADDR segment.");
      return false;
    }

    //
    // Read the data vector
    //
    if (!zmq_receive(_zmqSocket, data))
    {
      OS_LOG_ERROR(FAC_NET, LOG_TAG_WID(_applicationId)
          << "0mq failed to receive DATA segment.");
      return false;
    }

    //
    // Read number of subscribers active
    //
    std::string strcount;
    if (!zmq_receive(_zmqSocket, strcount))
    {
      OS_LOG_ERROR(FAC_NET, LOG_TAG_WID(_applicationId)
          << "0mq failed to receive COUNT segment.");
      return false;
    }

      count = boost::lexical_cast<int>(strcount);
  }
  catch(std::exception& e)
  {
    OS_LOG_ERROR(FAC_NET, LOG_TAG_WID(_applicationId)
        << " Unknown exception: " << e.what());
    return false;
  }
  return true;
}

bool StateQueueClient::SQAClientCore::sendNoResponse(const StateQueueMessage& request)
{
  if (!_isAlive)
    return false;

  BlockingTcpClient::Ptr conn;
  if (!_clientPool.dequeue(conn, 1000))
  {
    OS_LOG_ERROR(FAC_NET, LOG_TAG_WID(_applicationId)
        << " Unable to retrieve a TCP connection for pool.");
    return false;
  }

  if (!conn->isConnected() && !conn->connect(_serviceAddress, _servicePort))
  {
    //
    // Put it back to the queue.  The server is down.
    //
    _clientPool.enqueue(conn);
    _isAlive = false;

    return false;
  }

  bool sent = conn->send(request);
  _clientPool.enqueue(conn);
  return sent;
}

bool StateQueueClient::SQAClientCore::sendAndReceive(const StateQueueMessage& request, StateQueueMessage& response)
{

  if (!_isAlive && request.getType() != StateQueueMessage::Ping)
  {
    //
    // Only allow ping requests to get through when connection is not alive
    //
    return false;
  }

  BlockingTcpClient::Ptr conn;
  if (!_clientPool.dequeue(conn, 1000))
  {
    OS_LOG_ERROR(FAC_NET, LOG_TAG_WID(_applicationId)
        << " Unable to retrieve a TCP connection for pool.");
    return false;
  }

  if (!conn->isConnected() && !conn->connect(_serviceAddress, _servicePort))
  {
    //
    // Put it back to the queue.  The server is down.
    //
    _clientPool.enqueue(conn);

    // the connection of this core is down, need to reset all timers
    _isAlive = false;
    return false;
  }

  bool sent = conn->sendAndReceive(request, response);
  _clientPool.enqueue(conn);

  return sent;
}




StateQueueClient::StateQueueClient(
      int type,
      const std::string& applicationId,
      const std::string& servicesAddresses,
      const std::string& servicePort,
      const std::string& zmqEventId,
      std::size_t poolSize,
      int readTimeout,
      int writeTimeout,
      int keepAliveTimeout,
      int signinTimeout
      ):
  _type(type),
  _applicationId(applicationId),
  _terminate(false),
  _rawEventId(zmqEventId),
  _poolSize(poolSize),
  _readTimeout(readTimeout),
  _writeTimeout(writeTimeout),
  _keepAliveTimeout(keepAliveTimeout),
  _signinTimeout(signinTimeout),
  _eventQueue(1000),
  _expires(10),
  _core(0),
  _ioService(),
  _pIoServiceThread(0)
{
  start(servicesAddresses, servicesAddresses, servicePort);

  OS_LOG_INFO(FAC_NET, LOG_TAG_WID(_applicationId)
      << " SQA client for zmqEventId: " <<  _rawEventId << " CREATED");
}

StateQueueClient::StateQueueClient(
      int type,
      const std::string& applicationId,
      const std::string& zmqEventId,
      std::size_t poolSize,
      int readTimeout,
      int writeTimeout,
      int keepAliveTimeout,
      int signinTimeout
      ) :
  _type(type),
  _applicationId(applicationId),
  _terminate(false),
  _rawEventId(zmqEventId),
  _poolSize(poolSize),
  _readTimeout(readTimeout),
  _writeTimeout(writeTimeout),
  _keepAliveTimeout(keepAliveTimeout),
  _signinTimeout(signinTimeout),
  _eventQueue(1000),
  _expires(10),
  _core(0),
  _ioService(),
  _pIoServiceThread(0)
{
  std::string serviceAddress;
  std::string servicesAddressesAll;
  std::string servicesPort;

  if (getClientOptions(serviceAddress, servicesAddressesAll, servicesPort))
  {
    start(serviceAddress, servicesAddressesAll, servicesPort);
    setFallbackServices(servicesAddressesAll);
  }

  OS_LOG_INFO(FAC_NET, LOG_TAG_WID(_applicationId)
      << " SQA client for zmqEventId: " <<  _rawEventId << " CREATED");
}

StateQueueClient::~StateQueueClient()
{
  terminate();

  _core = NULL;
  std::vector<SQAClientCore*>::iterator it;
  for(it = _cores.begin(); it != _cores.end(); it++)
  {
    SQAClientCore* core = *it;

    delete core;
  }

  _cores.clear();
}

bool StateQueueClient::start(const std::string& serviceAddress, const std::string& servicesAddressesAll, const std::string& port)
{
  OS_LOG_DEBUG(FAC_NET, LOG_TAG_WID(_applicationId)
      << " Starting SQA client"
      << " type: '" << SQAUtil::getClientStr(_type) << "'"
      << " zmqEventId: '" << _rawEventId << "'"
      << " serviceAddress: '" << serviceAddress << "'"
      << " servicesAddressesAll: '" << servicesAddressesAll << "'"
      << " servicePort: '" << port << "'");

  if (serviceAddress.empty() && servicesAddressesAll.empty())
  {
    OS_LOG_ERROR(FAC_NET, LOG_TAG_WID(_applicationId)
            << " no sqa control addresses provided");
    return false;
  }
  else if (servicesAddressesAll.empty())
  {
    OS_LOG_WARNING(FAC_NET, LOG_TAG_WID(_applicationId)
            << " no sqa control fallback addresses provided");
  }

  _fallbackServiceIdx = 0;
  _fallbackTimeout = 10;

  _currentFailedConnects = 0;
  _isFallbackActive = false;
  _fallbackTimer = 0;

  if (SQAUtil::isWatcherOnly(_type))
    _zmqEventId = "sqw.";
  else
    _zmqEventId = "sqa.";

  _zmqEventId += _rawEventId;

  // Push local address first, followed by all addresses
  std::vector<std::string> addresses;
  pushAddresses(addresses, "", serviceAddress);
  pushAddresses(addresses, serviceAddress, servicesAddressesAll);

  if (0 == addresses.size())
  {
    OS_LOG_ERROR(FAC_NET, LOG_TAG_WID(_applicationId)
            << " unable to retrieve a valid sqa control address");
    return false;
  }

  unsigned int addressesNum = addresses.size();
  if (SQAUtil::isPublisher(_type) && addressesNum > 1)
  {
    OS_LOG_INFO(FAC_NET, LOG_TAG_WID(_applicationId)
            << " publisher clients will use only one sqa service address");

    addressesNum = 1;
  }
  _serviceAddress = addresses[0];
  _servicePort = port;

  for (unsigned int i = 0; i < addressesNum; i++)
  {
    std::string coreApplicationId = _applicationId;
    if (i > 0)
    {
      std::stringstream strm;
      strm << _applicationId << "-" << _cores.size();
      coreApplicationId = strm.str();
    }

    SQAClientCore* core = new SQAClientCore(this, _cores.size(), _type, coreApplicationId, addresses[i],
        _servicePort, _zmqEventId, _poolSize, _readTimeout, _writeTimeout, _keepAliveTimeout, _signinTimeout);

    _cores.push_back(core);
  }

  _core = _cores[0];
  _pIoServiceThread = new boost::thread(boost::bind(&boost::asio::io_service::run, &_ioService));

  return true;
}

bool StateQueueClient::getClientOptions(std::string& serviceAddress, std::string& servicesAddressesAll, std::string& servicePort)
{

  if (_clientConfig.empty())
  {
    std::ostringstream sqaconfig;
    sqaconfig << SIPX_CONFDIR << "/" << "sipxsqa-client.ini";
    _clientConfig = sqaconfig.str();
  }

  OS_LOG_DEBUG(FAC_NET, LOG_TAG_WID(_applicationId)
      << " Reading SQA client options from file '" << _clientConfig << "'");

  ServiceOptions configOptions(_clientConfig);

  bool enabled = false;
  bool ok = configOptions.parseOptions();
  if (ok)
  {
    ok = configOptions.getOption("enabled", enabled, enabled) && enabled;
  }
  if (ok)
  {
    // this option is present only if sqa agent enabled on localhost
    configOptions.getOption("sqa-control-address", serviceAddress);
    // this option is mandatory
    ok = configOptions.getOption("sqa-control-address-all", servicesAddressesAll);
  }
  if (ok)
  {
    ok = configOptions.getOption("sqa-control-port", servicePort);
  }

  OS_LOG_DEBUG(FAC_NET, LOG_TAG_WID(_applicationId)
      << " SQA client options are: "
      << " sqa-control-address '" << serviceAddress << "'"
      << " sqa-control-address-all '" << servicesAddressesAll << "'"
      << " sqa-control-port '" << servicePort << "'");

  if  (!ok)
  {
    OS_LOG_ERROR(FAC_NET, LOG_TAG_WID(_applicationId)
        << " Unable to read full sqa client information");
  }

  return ok;
}

void StateQueueClient::pushAddresses(std::vector<std::string>& v, const std::string& excludeAddress, const std::string& newAddresses)
{
  std::vector<std::string> addresses;

  boost::algorithm::split(addresses, newAddresses, boost::is_any_of(", "), boost::token_compress_on);
  for (unsigned int i = 0; i < addresses.size(); i++)
  {
    if (!addresses[i].empty() &&
        (excludeAddress != addresses[i]) &&
        (v.end() == std::find(v.begin(), v.end(), addresses[i])))
    {
      v.push_back(addresses[i]);
    }
  }
}

bool StateQueueClient::setFallbackServices(const std::string& addresses, int fallbackTimeout)
{
  if (_terminate)
    return false;

  if (!SQAUtil::isPublisher(_type))
  {
    OS_LOG_WARNING(FAC_NET, LOG_TAG_WID(_applicationId)
            << " cannot set fallback services addresses for non publisher client");
    return false;
  }

  if (_fallbackServicesAddresses.size() > 0)
  {
    OS_LOG_WARNING(FAC_NET, LOG_TAG_WID(_applicationId)
            << " fallback services addresses already set");
    return false;
  }

  pushAddresses(_fallbackServicesAddresses, _serviceAddress, addresses);

  if (_fallbackServicesAddresses.size() == 0)
  {
    OS_LOG_WARNING(FAC_NET, LOG_TAG_WID(_applicationId)
            << " no fallback service address provided for publisher client");
    return false;
  }

  _fallbackServiceIdx = 0;
  _fallbackTimeout  = fallbackTimeout; //seconds
  _currentFailedConnects = 0;
  _isFallbackActive = false;

  checkFallback();
  setFallbackTimer();

  return true;
}

bool StateQueueClient::tryPrimaryCore()
{
  if (_cores[0]->isConnectedNow())
  {
    _core = _cores[0];

    _currentFailedConnects = 0;
    _isFallbackActive = false;

    OS_LOG_DEBUG(FAC_NET, LOG_TAG_WID(_applicationId)
        << " SWITCHED to primary");
    return true;

  }

  return false;
}

bool StateQueueClient::trySecondaryCore()
{
  for(unsigned int i = 1; i < _cores.size(); i++)
  {
    if (_cores[i]->isConnectedNow())
    {
      _core = _cores[i];
      _currentFailedConnects = 0;
      _isFallbackActive = true;

      OS_LOG_DEBUG(FAC_NET, LOG_TAG_WID(_applicationId)
          << " SWITCHED to secondary '" << i << "'");

      return true;
    }
  }

  return false;
}

bool StateQueueClient::createFallbackCore()
{
  std::string fallbackServiceAddress;
  while (_fallbackServiceIdx < _fallbackServicesAddresses.size())
  {
    unsigned int coreIdx = _cores.size();
    std::string fallbackApplicationId = _applicationId;
    std::stringstream strm;
    strm << _applicationId << "-" << _cores.size();
    fallbackApplicationId = strm.str();

    fallbackServiceAddress = _fallbackServicesAddresses[_fallbackServiceIdx];
    OS_LOG_DEBUG(FAC_NET, LOG_TAG_WID(_applicationId)
        << " try fallback to '" << fallbackServiceAddress << ":" << _servicePort << "'");

    _core = new SQAClientCore(this, coreIdx, _type, fallbackApplicationId, fallbackServiceAddress,
        _servicePort, _zmqEventId, _poolSize, _readTimeout, _writeTimeout, _keepAliveTimeout, _signinTimeout);
    _cores.push_back(_core);

    if (_core->isConnectedNow())
    {
      _isFallbackActive = true;
      OS_LOG_DEBUG(FAC_NET, LOG_TAG_WID(_applicationId)
          << " fallback to '" << fallbackServiceAddress << ":" << _servicePort << "' connected");
      return true;
    }

    _fallbackServiceIdx++;
  }

  return false;
}

void StateQueueClient::checkFallback()
{
  bool hasConnectedCore = false;

  // In case fallback is active try get back to primary core
  if (_isFallbackActive)
  {
    hasConnectedCore = tryPrimaryCore();
  }

  // Update regular fallback stats
  if (!hasConnectedCore)
  {
    if (_core->isConnectedNow())
    {
      _currentFailedConnects = 0;
      hasConnectedCore = true;
    }
    else
    {
      _currentFailedConnects++;
    }
  }

  // No connected core after timeout, try creating a core from fallback addresses
  if (!hasConnectedCore && (_currentFailedConnects > _fallbackTimeout))
  {
    hasConnectedCore = createFallbackCore();
  }

  // No connected core yet (not even from those created from fallback addresses). Iterate existing cores
  // and try get one that is connected
  if (!hasConnectedCore)
  {
    hasConnectedCore = trySecondaryCore();
  }

  if (hasConnectedCore)
  {
    _currentFailedConnects = 0;
  }
}

void StateQueueClient::setFallbackTimer()
{
  if (_terminate)
    return;

  if (!_fallbackTimer)
  {
    _fallbackTimer = new boost::asio::deadline_timer(_ioService, boost::posix_time::seconds(SQA_KEEP_ALIVE_TIMEOUT));
  }

  OS_LOG_DEBUG(FAC_NET, LOG_TAG_WID(_applicationId)
      << " reschedule fallback check in '" << 1 << "' seconds, "
      << " failed connect count so far: " << _currentFailedConnects);

  _fallbackTimer->expires_from_now(boost::posix_time::seconds(1));
  _fallbackTimer->async_wait(boost::bind(&StateQueueClient::fallbackLoop, this, boost::asio::placeholders::error));
}

void StateQueueClient::fallbackLoop(const boost::system::error_code& e)
{
  if (_terminate)
  {
    OS_LOG_INFO(FAC_NET, LOG_TAG_WID(_applicationId)
        << " terminate requested, fallbackLoop aborted");
    return;
  }

  if (e)
  {
    OS_LOG_ERROR(FAC_NET, LOG_TAG_WID(_applicationId)
        << " fallbackLoop timer failed with error: " <<  e.message());
    return;
  }

  checkFallback();
  setFallbackTimer();
}


void StateQueueClient::terminate()
{
  if (_terminate)
    return;

   _terminate = true;

   _ioService.stop();

   std::vector<SQAClientCore*>::iterator it;
   for(it = _cores.begin(); it != _cores.end(); it++)
   {
     SQAClientCore* core = *it;

     core->terminate();
   }

   if (_fallbackTimer)
   {
     _fallbackTimer->cancel();
     delete _fallbackTimer;
     _fallbackTimer = 0;
   }

   if (_pIoServiceThread)
   {
     _pIoServiceThread->join();
     delete _pIoServiceThread;
     _pIoServiceThread = 0;
   }

  OS_LOG_INFO(FAC_NET, LOG_TAG_WID(_applicationId)
      << "Ok");
}


bool StateQueueClient::checkMessageResponse(StateQueueMessage& response)
{
  std::string empty;
  return checkMessageResponse(response, empty);
}

bool StateQueueClient::checkMessageResponse(StateQueueMessage& response, const std::string& dataId)
{
    std::string messageResponse;
    response.get("message-response", messageResponse);
    if (messageResponse == "ok")
    {
      OS_LOG_DEBUG(FAC_NET, LOG_TAG_WID(_applicationId)
          << "Operation successful");
      return true;
    }
    else
    {

      std::string messageType;
      response.get("message-type", messageType);

      std::string messageResponseError;
      response.get("message-error", messageResponseError);

      if (dataId.empty())
      {
      std::string id;
      response.get("message-id", id);

      OS_LOG_ERROR(FAC_NET, LOG_TAG_WID(_applicationId)
          << "Operation '" << messageType << "' failed for id:" << id
          << ". Error: " << messageResponseError);

      }
      else
      {
        OS_LOG_ERROR(FAC_NET, LOG_TAG_WID(_applicationId)
            << "Operation '" << messageType << "' failed for dataId:" << dataId
            << ". Error: " << messageResponseError);
      }

      return false;
    }
}

bool StateQueueClient::pop(StateQueueMessage& ev)
{
  std::string data;
  if (_eventQueue.dequeue(data))
  {
    ev.parseData(data);
    return true;
  }
  return false;
}

bool StateQueueClient::pop(StateQueueMessage& ev, int milliseconds)
{
  std::string data;
  if (_eventQueue.dequeue(data, milliseconds))
  {
    ev.parseData(data);
    return true;
  }
  return false;
}

bool StateQueueClient::enqueue(const std::string& eventId, const std::string& data, int expires, bool publish)
{
  //
  // Enqueue it
  //
  std::string messageTypeStr;
  StateQueueMessage::Type messageType = StateQueueMessage::Unknown;
  if (!publish)
  {
      messageType = StateQueueMessage::Enqueue;
      messageTypeStr = "Enqueue";
  }
  else
  {
      messageType = StateQueueMessage::EnqueueAndPublish;
      messageTypeStr = "EnqueueAndPublish";
  }

  StateQueueMessage enqueueRequest(messageType, _type, eventId);
  enqueueRequest.set("message-app-id", _applicationId.c_str());
  enqueueRequest.set("message-expires", expires);
  enqueueRequest.set("message-data", data);

  std::string id;
  enqueueRequest.get("message-id", id);
  OS_LOG_DEBUG(FAC_NET, LOG_TAG_WID(_applicationId)
          << " operation: " << messageTypeStr
          << " message-id: "      << id
          << " message-app-id: "  << _applicationId
          << " message-data: "    << data
          << " message-expires: "    << expires);

  StateQueueMessage enqueueResponse;
  if (!_core->sendAndReceive(enqueueRequest, enqueueResponse))
  {
    OS_LOG_ERROR(FAC_NET, LOG_TAG_WID(_applicationId)
        << " FAILED");
    return false;
  }

  //
  // Check if Queue is successful
  //
  return checkMessageResponse(enqueueResponse);
}

bool StateQueueClient::internal_publish(const std::string& eventId, const std::string& data, bool noresponse)
{
  StateQueueMessage enqueueRequest(StateQueueMessage::Publish);

  std::string messageId;
  SQAUtil::generateId(messageId, _type, eventId);
  enqueueRequest.set("message-id", messageId.c_str());


  enqueueRequest.set("message-app-id", _applicationId.c_str());
  enqueueRequest.set("message-data", data);

  OS_LOG_DEBUG(FAC_NET,LOG_TAG_WID(_applicationId)
          << " message-id: "      << messageId
          << " message-app-id: "  << _applicationId
          << " message-data: "    << data);

  if (noresponse)
  {
    enqueueRequest.set("noresponse", noresponse);
    return _core->sendNoResponse(enqueueRequest);
  }
  else
  {
    StateQueueMessage enqueueResponse;
    if (!_core->sendAndReceive(enqueueRequest, enqueueResponse))
        return false;

    return checkMessageResponse(enqueueResponse);
  }
}


bool StateQueueClient::pop(std::string& id, std::string& data, int& serviceId)
{
  if (_terminate)
    return false;

  StateQueueMessage message;
  if (!pop(message))
    return false;
  return message.get("message-id", id) && message.get("message-data", data) && message.get("service-id", serviceId);
}

bool StateQueueClient::pop(std::string& id, std::string& data, int& serviceId, int milliseconds)
{
  if (_terminate)
    return false;

  StateQueueMessage message;
  if (!pop(message, milliseconds))
    return false;
  return message.get("message-id", id) && message.get("message-data", data) && message.get("service-id", serviceId);
}


bool StateQueueClient::watch(std::string& id, std::string& data)
{
  if (_terminate)
    return false;

  OS_LOG_INFO(FAC_NET, LOG_TAG_WID(_applicationId)
      << "(" << id << ") INVOKED");
  StateQueueMessage message;
  if (!pop(message))
    return false;

  return message.get("message-id", id) && message.get("message-data", data);
}

bool StateQueueClient::watch(std::string& id, std::string& data, int milliseconds)
{
  if (_terminate)
    return false;

  OS_LOG_INFO(FAC_NET, LOG_TAG_WID(_applicationId)
      << "(" << id << ") INVOKED" );
  StateQueueMessage message;
  if (!pop(message, milliseconds))
    return false;

  return message.get("message-id", id) && message.get("message-data", data);
}


bool StateQueueClient::enqueue(const std::string& data, int expires, bool publish)
{
  if (_terminate)
    return false;

  if (!SQAUtil::isDealer(_type))
    return false;

  return enqueue(_rawEventId, data, expires, publish);
}


bool StateQueueClient::publish(const std::string& eventId, const std::string& data, bool noresponse)
{
  if (_terminate)
    return false;

  if (!SQAUtil::isPublisher(_type))
    return false;

  if (eventId.empty())
      return false;

  return internal_publish(eventId, data, noresponse);
}

bool StateQueueClient::publish(const std::string& eventId, const char* data, int dataLength, bool noresponse)
{
  std::string buff = std::string(data, dataLength);
  return publish(eventId, data, dataLength);
}

bool StateQueueClient::publishAndSet(int workspace, const std::string& eventId, const std::string& data, int expires)
{
  if (_terminate)
    return false;

  if (!SQAUtil::isPublisher(_type))
    return false;

  StateQueueMessage request(StateQueueMessage::PublishAndSet, _type, eventId);

  std::string id;
  request.get("message-id", id);

  request.set("message-app-id", _applicationId.c_str());
  request.set("message-data-id", id.c_str());
  request.set("message-data", data);
  request.set("message-expires", expires);
  request.set("workspace", workspace);

  OS_LOG_DEBUG(FAC_NET, LOG_TAG_WID(_applicationId)
      << " message-id: "      << id
      << " message-app-id: "  << _applicationId
      << " message-data-id: " << id
      << " message-data: "    << data
      << " message-expires: " << expires
      << " workspace: "       << workspace);

  StateQueueMessage response;
  if (!_core->sendAndReceive(request, response))
    return false;

  //
  // Check if Queue is successful
  //
  return checkMessageResponse(response);
}


bool StateQueueClient::erase(const std::string& id, int serviceId)
{
  if (_terminate)
    return false;

  StateQueueMessage request(StateQueueMessage::Erase, _type);
  request.set("message-app-id", _applicationId.c_str());
  request.set("erase-id", id.c_str());

  if (serviceId < 0 || ((unsigned int)serviceId) > _cores.size())
  {
    return false;
  }

  SQAClientCore* core = _cores[serviceId];

  StateQueueMessage response;
  if (!core->sendAndReceive(request, response))
    return false;

  return checkMessageResponse(response);
}

bool StateQueueClient::persist(int workspace, const std::string& dataId, int expires)
{
  if (_terminate)
    return false;

  StateQueueMessage request(StateQueueMessage::Persist, _type);
  request.set("message-app-id", _applicationId.c_str());
  request.set("message-expires", expires);
  request.set("workspace", workspace);
  request.set("message-data-id", dataId.c_str());

  StateQueueMessage response;
  if (!_core->sendAndReceive(request, response))
    return false;

  return checkMessageResponse(response, dataId);
}

bool StateQueueClient::set(int workspace, const std::string& dataId, const std::string& data, int expires)
{
  if (_terminate)
    return false;

  StateQueueMessage request(StateQueueMessage::Set, _type);
  request.set("message-app-id", _applicationId.c_str());
  request.set("message-expires", expires);
  request.set("workspace", workspace);
  request.set("message-data-id", dataId.c_str());
  request.set("message-data", data);

  StateQueueMessage response;
  if (!_core->sendAndReceive(request, response))
    return false;

  return checkMessageResponse(response, dataId);
}

bool StateQueueClient::mset(int workspace, const std::string& mapId, const std::string& dataId, const std::string& data, int expires)
{
  if (_terminate)
    return false;

  StateQueueMessage request(StateQueueMessage::MapSet, _type);
  request.set("message-app-id", _applicationId.c_str());
  request.set("message-expires", expires);
  request.set("workspace", workspace);
  request.set("message-map-id", mapId.c_str());
  request.set("message-data-id", dataId.c_str());
  request.set("message-data", data);

  std::string id;
  request.get("message-id", id);

  OS_LOG_DEBUG(FAC_NET, LOG_TAG_WID(_applicationId)
      << " message-id: "      << id
      << " message-app-id: "  << _applicationId
      << " workspace: "    << workspace
      << " message-map-id: "    << mapId
      << " message-data-id: " << dataId
      << " message-data: "    << data
      << " message-expires: "    << expires);

  StateQueueMessage response;
  if (!_core->sendAndReceive(request, response))
    return false;

  return checkMessageResponse(response, dataId);
}

bool StateQueueClient::get(int workspace, const std::string& dataId, std::string& data)
{
  if (_terminate)
    return false;

  StateQueueMessage request(StateQueueMessage::Get, _type);
  request.set("message-app-id", _applicationId.c_str());
  request.set("workspace", workspace);
  request.set("message-data-id", dataId.c_str());

  StateQueueMessage response;
  if (!_core->sendAndReceive(request, response))
    return false;

  return checkMessageResponse(response, dataId);
}

bool StateQueueClient::mget(int workspace, const std::string& mapId, const std::string& dataId, std::string& data)
{
  if (_terminate)
    return false;

  StateQueueMessage request(StateQueueMessage::MapGet, _type);
  request.set("message-app-id", _applicationId.c_str());
  request.set("workspace", workspace);
  request.set("message-map-id", mapId.c_str());
  request.set("message-data-id", dataId.c_str());

  std::string id;
  request.get("message-id", id);

  OS_LOG_DEBUG(FAC_NET, LOG_TAG_WID(_applicationId)
      << " message-id: "      << id
      << " message-app-id: "  << _applicationId
      << " workspace: "    << workspace
      << " message-map-id: "    << mapId
      << " message-data-id: " << id);

  StateQueueMessage response;
  if (!_core->sendAndReceive(request, response))
    return false;

  return checkMessageResponse(response, dataId);
}

bool StateQueueClient::mgetm(int workspace, const std::string& mapId, std::map<std::string, std::string>& smap)
{
  if (_terminate)
    return false;

  StateQueueMessage request(StateQueueMessage::MapGetMultiple, _type);
  request.set("message-app-id", _applicationId.c_str());
  request.set("workspace", workspace);
  request.set("message-map-id", mapId.c_str());

  StateQueueMessage response;
  if (!_core->sendAndReceive(request, response))
    return false;

  bool ret = checkMessageResponse(response, mapId);
  if (true == ret)
  {
    std::string data;
    response.get("message-data", data);

    StateQueueMessage message;
    if (!message.parseData(data))
      return false;

    return message.getMap(smap);
  }

  return ret;
}

bool StateQueueClient::mgeti(int workspace, const std::string& mapId, const std::string& dataId, std::string& data)
{
  if (_terminate)
    return false;

  StateQueueMessage request(StateQueueMessage::MapGetInc, _type);
  request.set("message-app-id", _applicationId.c_str());
  request.set("workspace", workspace);
  request.set("message-map-id", mapId.c_str());
  request.set("message-data-id", dataId.c_str());

  StateQueueMessage response;
  if (!_core->sendAndReceive(request, response))
    return false;

  bool ret = checkMessageResponse(response, dataId);
  if (true == ret)
  {
    return response.get("message-data", data);
  }

  return ret;
}

bool StateQueueClient::remove(int workspace, const std::string& dataId)
{
  if (_terminate)
    return false;

  StateQueueMessage request(StateQueueMessage::Remove, _type);
  request.set("message-app-id", _applicationId.c_str());
  request.set("message-data-id", dataId.c_str());
  request.set("workspace", workspace);

  StateQueueMessage response;
  if (!_core->sendAndReceive(request, response))
    return false;

  return checkMessageResponse(response, dataId);
}

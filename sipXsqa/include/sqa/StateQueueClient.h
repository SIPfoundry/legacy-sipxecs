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
#include <os/OsLogger.h>
#include <boost/lexical_cast.hpp>
#include "ServiceOptions.h"
#include "SQADefines.h"

#define SQA_LINGER_TIME_MILLIS 5000
#define SQA_TERMINATE_STRING "__TERMINATE__"
#define SQA_CONN_MAX_READ_BUFF_SIZE 65536
#define SQA_CONN_READ_TIMEOUT 1000
#define SQA_CONN_WRITE_TIMEOUT 1000
#define SQA_KEY_MIN 22172	//TODO: This define and its value needs to be documented
#define SQA_KEY_ALPHA 22180	//TODO: This define and its value needs to be documented
#define SQA_KEY_DEFAULT SQA_KEY_MIN
#define SQA_KEY_MAX 22200	//TODO: This define and its value needs to be documented
#define SQA_KEEP_ALIVE_TIMEOUT 1 //sec
#define SQA_SIGNIN_TIMEOUT 1800 //sec
#define SQA_FALLBACK_TIMEOUT 10 //attempts

class StateQueueClient : public boost::enable_shared_from_this<StateQueueClient>, private boost::noncopyable
{
public:
  class SQAClientCore : public boost::enable_shared_from_this<SQAClientCore>, private boost::noncopyable
  {
  public:

    class BlockingTcpClient
    {
    public:
      typedef boost::shared_ptr<BlockingTcpClient> Ptr;

      class ConnectTimer
      {
      public:
        ConnectTimer(BlockingTcpClient* pOwner) :
          _pOwner(pOwner)
        {
          _pOwner->startConnectTimer();
        }

        ~ConnectTimer()
        {
          _pOwner->cancelConnectTimer();
        }
        BlockingTcpClient* _pOwner;
      };

      class ReadTimer
      {
      public:
        ReadTimer(BlockingTcpClient* pOwner) :
          _pOwner(pOwner)
        {
          _pOwner->startReadTimer();
        }

        ~ReadTimer()
        {
          _pOwner->cancelReadTimer();
        }
        BlockingTcpClient* _pOwner;
      };

      class WriteTimer
      {
      public:
        WriteTimer(BlockingTcpClient* pOwner) :
          _pOwner(pOwner)
        {
          _pOwner->startWriteTimer();
        }

        ~WriteTimer()
        {
          _pOwner->cancelWriteTimer();
        }
        BlockingTcpClient* _pOwner;
      };


      BlockingTcpClient(
              const std::string& applicationId,
              boost::asio::io_service& ioService,
              int readTimeout,
              int writeTimeout,
              short key) :
              _applicationId(applicationId),
              _ioService(ioService),
              _resolver(_ioService),
              _pSocket(0),
              _isConnected(false),
              _readTimeout(readTimeout),
              _writeTimeout(writeTimeout),
              _key(key),
              _readTimer(_ioService),
              _writeTimer(_ioService),
              _connectTimer(_ioService)
            {
            }

      ~BlockingTcpClient()
            {
                if (_pSocket)
                {
                    delete _pSocket;
                    _pSocket = 0;
                }
            }

      inline const char* getClassName() {return "BlockingTcpClient";}

      void setReadTimeout(boost::asio::ip::tcp::socket& socket, int milliseconds)
      {
        struct timeval tv;
        tv.tv_sec  = 0;
        tv.tv_usec = milliseconds * 1000;
        setsockopt(socket.native(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
      }

      void setWriteTimeout(boost::asio::ip::tcp::socket& socket, int milliseconds)
      {
        struct timeval tv;
        tv.tv_sec  = 0;
        tv.tv_usec = milliseconds * 1000;
        setsockopt(socket.native(), SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
      }

      void startReadTimer()
      {
        boost::system::error_code ec;
        _readTimer.expires_from_now(boost::posix_time::milliseconds(_readTimeout), ec);
        _readTimer.async_wait(boost::bind(&BlockingTcpClient::onReadTimeout, this, boost::asio::placeholders::error));
      }

      void startWriteTimer()
      {
        boost::system::error_code ec;
        _writeTimer.expires_from_now(boost::posix_time::milliseconds(_writeTimeout), ec);
        _writeTimer.async_wait(boost::bind(&BlockingTcpClient::onWriteTimeout, this, boost::asio::placeholders::error));
      }

      void startConnectTimer()
      {
        boost::system::error_code ec;
        _connectTimer.expires_from_now(boost::posix_time::milliseconds(_readTimeout), ec);
        _connectTimer.async_wait(boost::bind(&BlockingTcpClient::onConnectTimeout, this, boost::asio::placeholders::error));
      }

      void cancelReadTimer()
      {
        boost::system::error_code ec;
        _readTimer.cancel(ec);
      }

      void cancelWriteTimer()
      {
        boost::system::error_code ec;
        _writeTimer.cancel(ec);
      }

      void cancelConnectTimer()
      {
        boost::system::error_code ec;
        _connectTimer.cancel(ec);
      }

      void onReadTimeout(const boost::system::error_code& e)
      {
        if (e)
          return;
        close();
        OS_LOG_ERROR(FAC_NET, LOG_TAG_WID(_applicationId)
            << " read timeout after '" << _readTimeout << "' milliseconds.");
      }

      void onWriteTimeout(const boost::system::error_code& e)
      {
        if (e)
          return;
        close();
        OS_LOG_ERROR(FAC_NET, LOG_TAG_WID(_applicationId)
            <<" write timeout after '" << _writeTimeout << "' milliseconds.");
      }

      void onConnectTimeout(const boost::system::error_code& e)
      {
        if (e)
          return;
        close();
        OS_LOG_ERROR(FAC_NET,  LOG_TAG_WID(_applicationId)
            << " connect timeout after '" << _readTimeout << "' milliseconds.");
      }

      void close()
      {
        if (_pSocket)
        {
          boost::system::error_code ignored_ec;
         _pSocket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
         _pSocket->close(ignored_ec);
         _isConnected = false;
         OS_LOG_INFO(FAC_NET,  LOG_TAG_WID(_applicationId)
             << "  - socket deleted.");
        }
      }

      bool connect(const std::string& serviceAddress, const std::string& servicePort)
      {
        //
        // Close the previous socket;
        //
        close();

        if (_pSocket)
        {
          delete _pSocket;
          _pSocket = 0;
        }

        _pSocket = new boost::asio::ip::tcp::socket(_ioService);

        OS_LOG_INFO(FAC_NET,  LOG_TAG_WID(_applicationId)
            << " creating new connection to '" << serviceAddress << "' : '" << servicePort << "'");

        _serviceAddress = serviceAddress;
        _servicePort = servicePort;

        try
        {
          //tcp::resolver::query query(host, PORT, boost::asio::ip::resolver_query_base::numeric_service);
          boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), serviceAddress.c_str(), servicePort.c_str());
          boost::asio::ip::tcp::resolver::iterator hosts = _resolver.resolve(query);

          ConnectTimer timer(this);
          //////////////////////////////////////////////////////////////////////////
          // Only works in 1.47 version of asio.  1.46 doesnt have this utility func
          // boost::asio::connect(*_pSocket, hosts);
          _pSocket->connect(hosts->endpoint()); // so we use the connect member
          //////////////////////////////////////////////////////////////////////////
          _isConnected = true;
          OS_LOG_INFO(FAC_NET, LOG_TAG_WID(_applicationId)
              << " creating new connection to '" << serviceAddress << "' : '" << servicePort << "' SUCESSFUL.");
        }
        catch(std::exception& e)
        {
          OS_LOG_ERROR(FAC_NET, LOG_TAG_WID(_applicationId)
              << " failed with error " << e.what());
          _isConnected = false;
        }

        return _isConnected;
      }


      bool connect()
      {
        if(_serviceAddress.empty() || _servicePort.empty())
        {
          OS_LOG_ERROR(FAC_NET, LOG_TAG_WID(_applicationId)
              << " remote address is not set");

          return false;
        }

        return connect(_serviceAddress, _servicePort);
      }

      bool send(const StateQueueMessage& request)
      {
        assert(_pSocket);
        std::string data = request.data();

        if (data.size() > SQA_CONN_MAX_READ_BUFF_SIZE - 1) /// Account for the terminating char "_"
        {
          OS_LOG_ERROR(FAC_NET, LOG_TAG_WID(_applicationId)
              << " data size '" << data.size() << "' maximum buffer length of '" << SQA_CONN_MAX_READ_BUFF_SIZE - 1 << "'");
          return false;
        }

        short version = 1;
        unsigned long len = (unsigned long)data.size() + 1; /// Account for the terminating char "_"
        std::stringstream strm;
        strm.write((char*)(&version), sizeof(version));
        strm.write((char*)(&_key), sizeof(_key));
        strm.write((char*)(&len), sizeof(len));
        strm << data << "_";
        std::string packet = strm.str();
        boost::system::error_code ec;
        bool ok = false;

        {
          WriteTimer timer(this);
          ok = _pSocket->write_some(boost::asio::buffer(packet.c_str(), packet.size()), ec) > 0;
        }

        if (!ok || ec)
        {
          OS_LOG_ERROR(FAC_NET, LOG_TAG_WID(_applicationId)
              << " write_some error: " << ec.message());
          _isConnected = false;

          return false;
        }

        return true;
      }

      bool receive(StateQueueMessage& response)
      {
        assert(_pSocket);
        unsigned long len = getNextReadSize();
        if (!len)
        {
          OS_LOG_INFO(FAC_NET, LOG_TAG_WID(_applicationId)
              << " next read size is empty.");

          return false;
        }

        char responseBuff[len];
        boost::system::error_code ec;
        {
          ReadTimer timer(this);
          _pSocket->read_some(boost::asio::buffer((char*)responseBuff, len), ec);
        }

        if (ec)
        {
          if (boost::asio::error::eof == ec)
          {
            OS_LOG_INFO(FAC_NET, LOG_TAG_WID(_applicationId)
                << " remote closed the connection, read_some error: " << ec.message());
          }
          else
          {
            OS_LOG_ERROR(FAC_NET, LOG_TAG_WID(_applicationId)
                << " read_some error: " << ec.message());
          }

          _isConnected = false;
          return false;
        }
        std::string responseData(responseBuff, len);

        return response.parseData(responseData);
      }

      bool sendAndReceive(const StateQueueMessage& request, StateQueueMessage& response)
      {
        if (send(request))
          return receive(response);

        return false;
      }

      unsigned long getNextReadSize()
      {
        bool hasVersion = false;
        bool hasKey = false;
        unsigned long remoteLen = 0;
        while (!hasVersion || !hasKey)
        {
          short remoteVersion;
          short remoteKey;

          //TODO: Refactor the code below to do one read for the three fields
          //
          // Read the version (must be 1)
          //
          while (true)
          {

            boost::system::error_code ec;
            _pSocket->read_some(boost::asio::buffer((char*)&remoteVersion, sizeof(remoteVersion)), ec);
            if (ec)
            {
              if (boost::asio::error::eof == ec)
              {
                OS_LOG_ERROR(FAC_NET, LOG_TAG_WID(_applicationId)
                    << " remote closed the connection, read_some error: " << ec.message());
              }
              else
              {
                OS_LOG_INFO(FAC_NET, LOG_TAG_WID(_applicationId)
                    << " Unable to read version "
                    << "ERROR: " << ec.message());
              }

              _isConnected = false;
              return 0;
            }
            else if (remoteVersion == 1)
               {
                 hasVersion = true;
                 break;
               }
          }

          while (true)
          {

            boost::system::error_code ec;
            _pSocket->read_some(boost::asio::buffer((char*)&remoteKey, sizeof(remoteKey)), ec);
            if (ec)
            {
              if (boost::asio::error::eof == ec)
              {
                OS_LOG_ERROR(FAC_NET, LOG_TAG_WID(_applicationId)
                    << " remote closed the connection, read_some error: " << ec.message());
              }
              else
              {
                OS_LOG_INFO(FAC_NET, LOG_TAG_WID(_applicationId)
                    << " Unable to read secret key "
                    << "ERROR: " << ec.message());
              }

              _isConnected = false;
              return 0;
            }
            else
            {
              if (remoteKey >= SQA_KEY_MIN && remoteKey <= SQA_KEY_MAX)
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
          if (boost::asio::error::eof == ec)
          {
            OS_LOG_ERROR(FAC_NET, LOG_TAG_WID(_applicationId)
                << " remote closed the connection, read_some error: " << ec.message());
          }
          else
          {
            OS_LOG_INFO(FAC_NET, LOG_TAG_WID(_applicationId)
                << " Unable to read secret packet length "
                << "ERROR: " << ec.message());
          }

          _isConnected = false;
          return 0;
        }

        return remoteLen;
      }

      bool isConnected() const
      {
        return _isConnected;
      }

      std::string getLocalAddress()
      {
        try
        {
          if (!_pSocket)
            return "";
          return _pSocket->local_endpoint().address().to_string();
        }
        catch(...)
        {
          return "";
        }
      }
    private:
      std::string _applicationId;
      boost::asio::io_service& _ioService;
      boost::asio::ip::tcp::resolver _resolver;
      boost::asio::ip::tcp::socket *_pSocket;
      std::string _serviceAddress;
      std::string _servicePort;
      bool _isConnected;
      int _readTimeout;
      int _writeTimeout;
      short _key;
      boost::asio::deadline_timer _readTimer;
      boost::asio::deadline_timer _writeTimer;
      boost::asio::deadline_timer _connectTimer;
      friend class SQAClientCore;
    };

  public:
    friend class SQAClientUtil;
    friend class SQAClientTest;
    friend class SQAClientHATest;
    friend class StateQueueClient;
    friend class StateQueueAgentTest;

    SQAClientCore(
          StateQueueClient* owner,
          int idx,
          int type,
          const std::string& applicationId,
          const std::string& serviceAddress,
          const std::string& servicePort,
          const std::string& zmqEventId,
          std::size_t poolSize,
          int readTimeout = SQA_CONN_READ_TIMEOUT,
          int writeTimeout = SQA_CONN_WRITE_TIMEOUT,
          int keepAliveTimeout = SQA_KEEP_ALIVE_TIMEOUT,
          int signinTimeout = SQA_SIGNIN_TIMEOUT
          );

    ~SQAClientCore();

    const char* getClassName();

    const std::string& getLocalAddress();
    void terminate();
    bool isConnected();
    bool isConnectedNow();

    void setExpires(int expires);

    bool signin();
    bool logout();
    bool subscribe(const std::string& eventId, const std::string& sqaAddress);

    bool sendNoResponse(const StateQueueMessage& request);
    bool sendAndReceive(const StateQueueMessage& request, StateQueueMessage& response);

  private:
    void init(int readTimeout, int writeTimeout);
    bool sendKeepAlive();
    void setKeepAliveTimer();
    void keepAliveLoop(const boost::system::error_code& e);

    void setSigninTimer();
    void signinLoop(const boost::system::error_code& e);

    void eventLoop();
    void do_pop(bool firstHit, int count, const std::string& id, const std::string& data);
    void do_watch(bool firstHit, int count, const std::string& id, const std::string& data);
    bool readEvent(std::string& id, std::string& data, int& count);

    static void zmq_free (void *data, void *hint);
    //  Convert string to 0MQ string and send to socket
    static bool zmq_send (zmq::socket_t & socket, const std::string & data);
    //  Sends string as 0MQ string, as multipart non-terminal
    static bool zmq_sendmore (zmq::socket_t & socket, const std::string & data);
    static bool zmq_receive (zmq::socket_t *socket, std::string& value);

  protected:
    StateQueueClient *_owner;
    int _idx;
    int _type;
    boost::asio::deadline_timer *_keepAliveTimer;
    boost::asio::deadline_timer *_signinTimer;

    std::size_t _poolSize;
    std::string _serviceAddress;
    std::string _servicePort;
    typedef BlockingQueue<BlockingTcpClient::Ptr> ClientPool;
    ClientPool _clientPool;
    std::size_t _clientPoolSize;
    bool _terminate;
    zmq::context_t* _zmqContext;
    zmq::socket_t* _zmqSocket;
    boost::thread* _pEventThread;
    std::string _zmqEventId;
    std::string _applicationId;
    std::vector<BlockingTcpClient::Ptr> _clientPointers;
    int _expires;

    int _backoffCount;
    std::string _localAddress;
    int _isAlive;

    int _signinTimeout;
    enum SQAOpState _signinState;
    int  _signinAttempts;

    int _keepAliveTimeout;
    enum SQAOpState _keepAliveState;
    int  _keepAliveAttempts;

    enum SQAOpState _subscribeState;
    bool _subscribeAttempts;

    std::string _publisherAddress;
  };

public:
  friend class SQAClientUtil;
  friend class SQAClientTest;
  friend class SQAClientHATest;
  friend class StateQueueAgentTest;
  typedef SQAClientCore* ServiceId;
  typedef BlockingQueue<std::string> EventQueue;

  StateQueueClient(
        int type,
        const std::string& applicationId,
        const std::string& servicesAddresses,
        const std::string& servicePort,
        const std::string& zmqEventId,
        std::size_t poolSize,
        int readTimeout = SQA_CONN_READ_TIMEOUT,
        int writeTimeout = SQA_CONN_WRITE_TIMEOUT,
        int keepAliveTimeout = SQA_KEEP_ALIVE_TIMEOUT,
        int signinTimeout = SQA_SIGNIN_TIMEOUT
        );
  StateQueueClient(
        int type,
        const std::string& applicationId,
        const std::string& zmqEventId,
        std::size_t poolSize,
        int readTimeout = SQA_CONN_READ_TIMEOUT,
        int writeTimeout = SQA_CONN_WRITE_TIMEOUT,
        int keepAliveTimeout = SQA_KEEP_ALIVE_TIMEOUT,
        int signinTimeout = SQA_SIGNIN_TIMEOUT
        );
  ~StateQueueClient();
  void terminate();

  boost::asio::io_service* getIoService();
  void setSQAClientConfig(const std::string& clientConfigFilePath);
  const std::string& getLocalAddress();
  bool isConnected();
  const char* getClassName();
  EventQueue* getEventQueue();

  bool setFallbackServices(const std::string& addresses, int fallbackTimeout = 10);

  bool pop(std::string& id, std::string& data, int& serviceId);
  bool pop(std::string& id, std::string& data, int& serviceId, int milliseconds);

  bool watch(std::string& id, std::string& data);
  bool watch(std::string& id, std::string& data, int milliseconds);

  bool enqueue(const std::string& data, int expires = 30, bool publish = false);

  bool publish(const std::string& eventId, const std::string& data, bool noresponse);
  bool publish(const std::string& eventId, const char* data, int dataLength, bool noresponse);
  bool publishAndSet(int workspace, const std::string& eventId, const std::string& data, int expires);

  bool erase(const std::string& id, int serviceId=0);
  bool persist(int workspace, const std::string& dataId, int expires);
  bool set(int workspace, const std::string& dataId, const std::string& data, int expires);
  bool mset(int workspace, const std::string& mapId, const std::string& dataId, const std::string& data, int expires);
  bool get(int workspace, const std::string& dataId, std::string& data);
  bool mget(int workspace, const std::string& mapId, const std::string& dataId, std::string& data);
  bool mgetm(int workspace, const std::string& mapId, std::map<std::string, std::string>& smap);
  bool mgeti(int workspace, const std::string& mapId, const std::string& dataId, std::string& data);
  bool remove(int workspace, const std::string& dataId);

private:
  bool start(const std::string& servicesAddresses, const std::string& servicesAddressesAll, const std::string& port);

  bool getClientOptions(std::string& preferredAddresses, std::string& addresses, std::string& port);

  void pushAddresses(std::vector<std::string>& v, const std::string& excludeAddress, const std::string& newAddresses);

  bool tryPrimaryCore();
  bool trySecondaryCore();
  bool createFallbackCore();
  void checkFallback();
  void setFallbackTimer();
  void fallbackLoop(const boost::system::error_code& e);

  bool checkMessageResponse(StateQueueMessage& response);
  bool checkMessageResponse(StateQueueMessage& response, const std::string& dataId);

  bool pop(StateQueueMessage& ev);
  bool pop(StateQueueMessage& ev, int milliseconds);

  bool enqueue(const std::string& eventId, const std::string& data, int expires = 30, bool publish= false);
  bool internal_publish(const std::string& eventId, const std::string& data, bool noresponse = false);

protected:
  int _type;
  std::string _applicationId;
  std::string _serviceAddress; // address of the primary core (for publisher/dealer clients only)
  std::string _servicePort;
  bool _terminate;
  std::string _zmqEventId;
  std::string _rawEventId;
  std::size_t _poolSize;
  int _readTimeout;
  int _writeTimeout;
  int _keepAliveTimeout;
  int _signinTimeout;
  EventQueue _eventQueue;
  int _expires;
  SQAClientCore* _core;
  std::vector<SQAClientCore*> _cores;
  boost::asio::io_service _ioService;
  boost::thread* _pIoServiceThread;
  std::string _clientConfig;

  std::vector<std::string> _fallbackServicesAddresses;
  unsigned int _fallbackServiceIdx;
  unsigned int _fallbackTimeout;
  boost::asio::deadline_timer* _fallbackTimer;
  unsigned int _currentFailedConnects;
  bool _isFallbackActive;
};


inline const std::string& StateQueueClient::SQAClientCore::getLocalAddress()
{
  return _localAddress;
}

 inline void StateQueueClient::SQAClientCore::zmq_free (void *data, void *hint)
{
    free (data);
}

 //  Convert string to 0MQ string and send to socket
inline bool StateQueueClient::SQAClientCore::zmq_send (zmq::socket_t & socket, const std::string & data)
{
  char * buff = (char*)malloc(data.size());
  memcpy(buff, data.c_str(), data.size());
  zmq::message_t message((void*)buff, data.size(), zmq_free, 0);
  bool rc = socket.send(message);
  return (rc);
}

//  Sends string as 0MQ string, as multipart non-terminal
inline bool StateQueueClient::SQAClientCore::zmq_sendmore (zmq::socket_t & socket, const std::string & data)
{
  char * buff = (char*)malloc(data.size());
  memcpy(buff, data.c_str(), data.size());
  zmq::message_t message((void*)buff, data.size(), zmq_free, 0);
  bool rc = socket.send(message, ZMQ_SNDMORE);
  return (rc);
}

inline bool StateQueueClient::SQAClientCore::zmq_receive (zmq::socket_t *socket, std::string& value)
{
    zmq::message_t message;
    socket->recv(&message);
    if (!message.size())
      return false;
    value = std::string(static_cast<char*>(message.data()), message.size());
    return true;
}

inline const char* StateQueueClient::SQAClientCore::getClassName()
{
  return "SQAClientCore";
}

inline void StateQueueClient::SQAClientCore::setExpires(int expires)
{
  _expires = expires;
}

inline boost::asio::io_service* StateQueueClient::getIoService()
{
  return &_ioService;
}

inline void StateQueueClient::setSQAClientConfig(const std::string& clientConfigFilePath)
{
  _clientConfig = clientConfigFilePath;
}

inline const std::string& StateQueueClient::getLocalAddress()
{
  return _core->getLocalAddress();
}

inline bool StateQueueClient::isConnected()
{
  return _core->isConnectedNow();
}

inline const char* StateQueueClient::getClassName()
{
  return "StateQueueClient";
}

inline StateQueueClient::EventQueue* StateQueueClient::getEventQueue()
{
  return &_eventQueue;
}


#endif	/* StateQueueClient_H */


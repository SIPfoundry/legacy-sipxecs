/*
 * Copyright (c) eZuce, Inc. All rights reserved.
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

#ifndef RESTSERVER_H_INCLUDED
#define	RESTSERVER_H_INCLUDED

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "sipxyard/LevelDB.h"


#define REST_DEFAULT_ROOT_DOCUMENT "/root"


class RESTServer : boost::noncopyable
{
public:
  typedef boost::recursive_mutex mutex;
  typedef boost::lock_guard<mutex> mutex_lock;
  typedef Poco::Net::HTTPServerRequest Request;
  typedef Poco::Net::HTTPServerResponse Response;
  typedef void* OS_HANDLE;
  typedef boost::function<void(Request&, Response&)> Handler;
  typedef std::map<std::string, LevelDB*> KVStore;
   
  RESTServer();
  
  RESTServer(RESTServer* pParentStore);
  
  RESTServer(int maxQueuedConnections, int maxThreads);
  
  RESTServer(RESTServer* pParentStore, int maxQueuedConnections, int maxThreads);
  
  ~RESTServer();
  
  bool start(const std::string& address, unsigned short port, bool secure);
  
  bool start(unsigned short port, bool secure);
  
  void stop();
  
  const std::string& getAddress() const;
  
  unsigned short getPort() const;
 
  bool open(const std::string& dataFile);
  
  void setCredentials(const std::string& user, const std::string& password);
  
  void setCustomHandler(const Handler& handler);
  
  void setRootDocument(const std::string& rootDocument);
  
  void setDataDirectory(const std::string& dataDirectory);
  
  const std::string& getDataDirectory() const;
  
  LevelDB* getStore(const std::string& path, bool createIfMissing);
 
protected:  
  void onHandleRequest(Request& request, Response& response);
  
  void onHandleRestRequest(Request& request, Response& response);
  
  bool isAuthorized(Request& request, Response& response);
  
  void sendRestRecordsAsJson(const std::vector<std::string>& pathVector, LevelDB::Records& records, Response& response);
    
  void sendRestRecordsAsValuePairs(const std::string& path, const LevelDB::Records& records, Response& response);
  
  void escapeString(std::string& str);
  
  bool validateInsert(LevelDB* pStore, const std::string& resource, const std::string& value);
private:
  OS_HANDLE _socketHandle;
  OS_HANDLE _secureSocketHandle;
  OS_HANDLE _serverHandle;
  OS_HANDLE _serverParamsHandle;
  int _maxQueuedConnections; 
  int _maxThreads;
  std::string _address;
  unsigned short _port;
  std::string _user;
  std::string _password;
  std::string _rootDocument;
  std::string _dataDirectory;
  mutex _kvStoreMutex;
  KVStore _kvStore;
  Handler _customHandler;
  bool _isSecure;
  RESTServer* _pParentStore;
}; 

//
// Inlines
//

inline bool RESTServer::start(unsigned short port, bool secure)
{
  return start("", port, secure);
}

inline const std::string& RESTServer::getAddress() const
{
  return _address;
}
  
inline unsigned short RESTServer::getPort() const
{
  return _port;
}

inline void RESTServer::setDataDirectory(const std::string& dataDirectory)
{
  _dataDirectory = dataDirectory;
}

inline const std::string& RESTServer::getDataDirectory() const
{
  return _dataDirectory;
}

inline void RESTServer::setCredentials(const std::string& user, const std::string& password)
{
  _user = user;
  _password = password;
}

inline void RESTServer::setCustomHandler(const Handler& handler)
{
  _customHandler = handler;
}

#endif	/* RESTSERVER_H */


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
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include "sipxyard/RESTServer.h"
#include "sipxyard/YardUtils.h"
#include "os/OsLogger.h"

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/SecureStreamSocket.h"
#include "Poco/Net/SecureServerSocket.h"
#include "Poco/Net/X509Certificate.h"
#include "Poco/Net/HTTPBasicCredentials.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Exception.h"
#include "Poco/ThreadPool.h"
#include "Poco/Net/PrivateKeyPassphraseHandler.h"
#include "Poco/Net/SSLManager.h"


using Poco::Net::ServerSocket;
using Poco::Net::SocketAddress;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::HTTPRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPServerParams;
using Poco::Net::HTTPBasicCredentials;
using Poco::Net::HTMLForm;
using Poco::Net::X509Certificate;
using Poco::Net::SecureServerSocket;
using Poco::Net::SecureStreamSocket;
using Poco::ThreadPool;


#define HTTP_SERVER_MAX_QUEUED_CONNECTIONS 100
#define HTTP_SERVER_MAX_THREADS 8

class HTTPServerRequestHandler: public HTTPRequestHandler
{
public:
	HTTPServerRequestHandler(RESTServer::Handler& handler) :
    _handler(handler)
	{
	}
	
	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
	{
    if (_handler)
      _handler(request, response);
	}
  
  RESTServer::Handler& _handler;
};
  
class HTTPServerHandlerFactory: public HTTPRequestHandlerFactory
{
public:
	HTTPServerHandlerFactory(RESTServer::Handler handler) :
    _handler(handler)
	{
	}

	HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request)
	{
    if (!_handler)
      return false;
    
		return new HTTPServerRequestHandler(_handler);
	}
  
  RESTServer::Handler _handler;
};
  

RESTServer::RESTServer() :
  _socketHandle(0),
  _secureSocketHandle(0),
  _serverHandle(0),
  _serverParamsHandle(0),
  _maxQueuedConnections(HTTP_SERVER_MAX_QUEUED_CONNECTIONS),
  _maxThreads(HTTP_SERVER_MAX_THREADS),
  _port(0),
  _rootDocument(REST_DEFAULT_ROOT_DOCUMENT),
  _isSecure(false),
  _pParentStore(0)
{
}

RESTServer::RESTServer(RESTServer* pParentStore) :
  _socketHandle(0),
  _secureSocketHandle(0),
  _serverHandle(0),
  _serverParamsHandle(0),
  _maxQueuedConnections(HTTP_SERVER_MAX_QUEUED_CONNECTIONS),
  _maxThreads(HTTP_SERVER_MAX_THREADS),
  _port(0),
  _rootDocument(REST_DEFAULT_ROOT_DOCUMENT),
  _isSecure(false),
  _pParentStore(pParentStore)
{
}


RESTServer::RESTServer(int maxQueuedConnections, int maxThreads) :
  _socketHandle(0),
  _secureSocketHandle(0),
  _serverHandle(0),
  _serverParamsHandle(0),
  _maxQueuedConnections(maxQueuedConnections),
  _maxThreads(maxThreads),
  _port(0),
  _rootDocument(REST_DEFAULT_ROOT_DOCUMENT),
  _isSecure(false),
  _pParentStore(0)
{
}

RESTServer::RESTServer(RESTServer* pParentStore, int maxQueuedConnections, int maxThreads) :
  _socketHandle(0),
  _secureSocketHandle(0),
  _serverHandle(0),
  _serverParamsHandle(0),
  _maxQueuedConnections(maxQueuedConnections),
  _maxThreads(maxThreads),
  _port(0),
  _rootDocument(REST_DEFAULT_ROOT_DOCUMENT),
  _isSecure(false),
  _pParentStore(pParentStore)
{
}

RESTServer::~RESTServer()
{
  stop();
  
  for (KVStore::const_iterator iter = _kvStore.begin(); iter != _kvStore.end(); iter++)
    delete iter->second;
}
  
bool RESTServer::start(const std::string& address, unsigned short port, bool secure)
{
  
  _address = address;
  _port = port;
  _isSecure = secure;
    
  
  try
  {
    ServerSocket* pSocket = 0;
    SecureServerSocket* pSecureSocket = 0;

    if (!_isSecure)
    {
      if (_address.empty())
      {
        pSocket = new ServerSocket(port);
      }
      else
      {
        SocketAddress sockAddress(_address, port);
        pSocket = new ServerSocket(sockAddress);
        
      }
      _socketHandle = (OS_HANDLE)pSocket;
    }
    else
    {
      Poco::Net::initializeSSL();
      if (_address.empty())
      {
        pSecureSocket = new SecureServerSocket(port);
      }
      else
      {
        SocketAddress sockAddress(_address, port);
        pSecureSocket = new SecureServerSocket(sockAddress);
      }
      _secureSocketHandle = (OS_HANDLE)pSecureSocket;
    }

    

    HTTPServerParams* pParams = new HTTPServerParams();
    _serverParamsHandle = (OS_HANDLE)pParams;

    //
    // Grow the default pool so it has enough to accommodate this servers requirement 
    //
    ThreadPool::defaultPool().addCapacity(_maxThreads);

    pParams->setMaxQueued(_maxQueuedConnections);
    pParams->setMaxThreads(_maxThreads);

    Poco::Net::HTTPServer* pHTTPServer = new Poco::Net::HTTPServer(new HTTPServerHandlerFactory(boost::bind(&RESTServer::onHandleRequest, this, _1, _2)), _isSecure ? *pSecureSocket : *pSocket, pParams);
    _serverHandle = (OS_HANDLE)pHTTPServer;

    pHTTPServer->start();
       
    return true;
  }
  catch(Poco::Exception e)
  {
    OS_LOG_ERROR(FAC_DB, "RESTServer::start - Exception: " << e.message());
    return false;
  }
}


void RESTServer::stop()
{
  if (_serverHandle)
  {
    Poco::Net::HTTPServer* pHTTPServer = (Poco::Net::HTTPServer*)_serverHandle;
    pHTTPServer->stop();
  }
  
  delete (ServerSocket*)_socketHandle;
  delete (SecureServerSocket*)_secureSocketHandle;
  delete (Poco::Net::HTTPServer*)_serverHandle;
  _socketHandle = 0;
  _serverHandle = 0;
  _secureSocketHandle = 0;
  
  if (_isSecure)
  {
    Poco::Net::uninitializeSSL();
  }
}

LevelDB* RESTServer::getStore(const std::string& path, bool createIfMissing)
{
  if (_pParentStore)
    return _pParentStore->getStore(path, createIfMissing);
  
  std::vector<std::string> tokens = YardUtils::string_tokenize(path, "/");
  if(tokens.size() < 3)
  {
    OS_LOG_ERROR(FAC_DB, "RESTServer::getStore - " << path << " a malformed document path");
    return 0;
  }
  
  mutex_lock lock(_kvStoreMutex);
  
  LevelDB* pStore = 0;
  std::string document = tokens[2];
  
  if (_kvStore.find(document) == _kvStore.end())
  {
    //
    // Create a new store
    //
    std::ostringstream strm;

    if (!_dataDirectory.empty())
      strm << _dataDirectory << "/" << document;
    else
      strm << document;
      
    if (createIfMissing || boost::filesystem::exists(strm.str()))
    {
      OS_LOG_INFO(FAC_DB, "RESTServer::getStore - Creating new document store for " << document);
      pStore = new LevelDB();
      pStore->open(strm.str());
      if (pStore->isOpen())
      {
       _kvStore.insert(std::pair<std::string, LevelDB*>(document, pStore));
      }
      else
      {
        OS_LOG_ERROR(FAC_DB, "RESTServer::getStore - Unable to open document store for " << document);
        delete pStore;
        pStore = 0;
      }
    }
    else
    {
      std::cerr << "NO STORE AVAILABLE for " << strm.str() << std::endl;
    }
  }
  else
  {
    pStore = _kvStore.at(document);
  }
  
  if (!pStore)
  {
    OS_LOG_ERROR(FAC_DB, "RESTServer::getStore - No document store available for " << document);
  }
  
  return pStore;
}
bool RESTServer::isAuthorized(Request& request, Response& response)
{
  if (_user.empty())
    return true;
  
  if (!request.hasCredentials())
  {
    response.requireAuthentication(request.getURI());
    response.send();
    return false;
  }
  
  HTTPBasicCredentials cred(request);
  
  bool authorized = cred.getUsername() == _user && cred.getPassword() == _password;
  
  if (!authorized)
  {
    OS_LOG_WARNING(FAC_DB, "RESTServer::isAuthorized - Unauthorized request from " << request.clientAddress().toString());
    response.setStatus(HTTPResponse::HTTP_FORBIDDEN);
    response.send();
  }
  else
  {
    OS_LOG_INFO(FAC_DB, "RESTServer::isAuthorized - Authorization granted to " << request.clientAddress().toString());
  }
  
  return authorized;
}

void RESTServer::onHandleRequest(Request& request, Response& response)
{ 
  if (!isAuthorized(request, response))
  {
    return;
  }
  
  if (YardUtils::string_starts_with(request.getURI(), _rootDocument.c_str()))
  {
    onHandleRestRequest(request, response);
    return;
  }
  
  if (_customHandler)
  {
    _customHandler(request, response);
  }
  else
  {
    response.setStatus(Response::HTTP_BAD_REQUEST);
    response.send();
  }
}

bool RESTServer::validateInsert(LevelDB* pStore, const std::string& resource, const std::string& value)
{
  //
  // Check if the resource already exists as either a lone value or as a structure
  //
  std::string filter = resource + std::string("*");
  LevelDB::Records records;
  pStore->getRecords(filter, records);
  
  if (records.empty())
  {
    //
    // Check if the parent is an opaque object or a structure
    //
    
    std::string currentResource = resource;
    while (true)
    {
      std::vector<std::string> tokens;
      YardUtils::get_path_vector(currentResource, tokens);
      
      if (tokens.size() <= 2)
        break;

      std::string parentKey = "/";

      for (std::size_t i = 0; i < tokens.size() - 1; i++)
        parentKey += tokens[i] + std::string("/");

      LevelDB::Records parentRecords;
      pStore->getRecords(parentKey, parentRecords);

      if (!parentRecords.empty())
        return false;
      
      currentResource = parentKey;
    }
    
    return true;
  }
  else if (records.size() == 1)
  {
    //
    // There is one record. check if the key is equal to the resource
    //
    LevelDB::Records::iterator iter = records.begin();
    if (iter->key == resource)
    {
      //
      // Same key, this is an update
      //
      return true;
    }
  }
  
  //
  // There are multiple records underneath which means the resource is a structure.
  //
  return false;
}

void RESTServer::onHandleRestRequest(Request& request, Response& response)
{
  std::string path = request.getURI();
  
  std::cout << request.getMethod() << " " << path << std::endl;
  
  std::string resource = path;
  YardUtils::prepare_path(resource);
  bool canCreateStore = request.getMethod() == HTTPRequest::HTTP_PUT || request.getMethod() == HTTPRequest::HTTP_POST;
  LevelDB* pStore = getStore(resource, canCreateStore);
  
  if (!pStore)
  {
    response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
    response.send();
    return;
  }
   
  std::string filter = resource + std::string("*");
  
  if (request.getMethod() == HTTPRequest::HTTP_GET)
  {
    LevelDB::Records records;
    pStore->getRecords(filter, records);
    
    if (records.empty())
    {
      OS_LOG_DEBUG(FAC_DB, "RESTServer::onHandleRestRequest - No records found for path " << resource);
      response.setChunkedTransferEncoding(true);
      response.setReason("No Records Found");
      response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
      response.send();
      return;
    }
    
    OS_LOG_DEBUG(FAC_DB, "RESTServer::onHandleRestRequest - " << records.size() << " records found for path " << resource);
    
    std::vector<std::string> tokens;
    YardUtils::get_path_vector(resource, tokens);
    sendRestRecordsAsJson(tokens, records, response);
    
    return;
  }
  else if (request.getMethod() == HTTPRequest::HTTP_DELETE)
  {
    pStore->delKeys(filter);
    response.setReason("Records Deleted");
    response.setStatus(HTTPResponse::HTTP_OK);
    response.send();
    
    OS_LOG_DEBUG(FAC_DB, "RESTServer::onHandleRestRequest - Deleted records " << filter);
    
    return;
  }
  else if (request.getMethod() == HTTPRequest::HTTP_PUT || request.getMethod() == HTTPRequest::HTTP_POST)
  {
    HTMLForm form(request, request.stream());
    if (!resource.empty() && !form.empty() && form.has("value"))
    {
      std::string value = form.get("value");
      escapeString(value);
      
      if (!form.has("no-validate"))
      {
        if (!validateInsert(pStore, resource, value))
        {
          OS_LOG_WARNING(FAC_DB, "RESTServer::onHandleRestRequest - Type mismatch for " << resource << " method=" << request.getMethod());
          response.setReason("Object Type Mismatch");
          response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
          response.send();
          return;
        }
      }
      
      pStore->put(resource, value);
      response.setReason("Records Updated");
      response.setStatus(HTTPResponse::HTTP_OK);
      response.send();
      
      OS_LOG_INFO(FAC_DB, "RESTServer::onHandleRestRequest - Records updated for " << resource << " method=" << request.getMethod())
        
      return;
    }
  }
  
  //
  // Send a 404 if it ever gets here
  //
  response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
  response.send();
}

void RESTServer::escapeString(std::string& str)
{
  size_t pos = 0;
  while((pos = str.find("\"", pos)) != std::string::npos)
  {
     str.replace(pos, 1, "\\\"");
     pos += strlen("\\\"");
  }
}

void RESTServer::sendRestRecordsAsJson(const std::vector<std::string>& pathVector, LevelDB::Records& records, Response& response)
{
  response.setChunkedTransferEncoding(true);
  response.setContentType("text/json");
  YardUtils::json_print(pathVector, records, response.send());
}


void RESTServer::sendRestRecordsAsValuePairs(const std::string& path, const LevelDB::Records& records, Response& response)
{  
  response.setChunkedTransferEncoding(true);
  response.setContentType("text/plain");
  std::ostream& ostr = response.send();

  for (std::vector<LevelDB::Record>::const_iterator iter = records.begin(); iter != records.end(); iter++)
  {
    ostr << iter->key << ": " << iter->value << "\r\n";
  }
}


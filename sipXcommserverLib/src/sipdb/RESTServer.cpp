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

#include "sipdb/RESTServer.h"
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

//
// Used for sorting records
//
static bool compare_records (KeyValueStore::Record& first, KeyValueStore::Record& second)
{
  return first.key.compare(second.key) <= 0;
}

static bool string_starts_with(const std::string& str, const char* key)
{
  return str.find(key) == 0;
}

static bool string_ends_with(const std::string& str, const char* key)
{
  size_t i = str.rfind(key);
  return (i != std::string::npos) && (i == (str.length() - ::strlen(key)));
}

static std::vector<std::string> string_tokenize(const std::string& str, const char* tok)
{
  std::vector<std::string> tokens;
  boost::split(tokens, str, boost::is_any_of(tok), boost::token_compress_on);
  return tokens;
}

static void getPathVector(const std::string& path, std::vector<std::string>& pathVector)
{
  std::vector<std::string> tokens = string_tokenize(path, "/");
  
  for (std::vector<std::string>::const_iterator iter = tokens.begin(); iter != tokens.end(); iter++)
  {
    if (!iter->empty())
      pathVector.push_back(*iter);
  }
}

static bool printOneRecord(std::size_t filterDepth, const std::string& resourceName, std::list<KeyValueStore::Record>& records, std::list<KeyValueStore::Record>::iterator& iter, std::ostream& ostr)
{
  //
  // return false if there are no more records
  //
  if (iter == records.end())
    return false;
  
  //
  // This will hold the tokens of the current key
  //
  std::vector<std::string> keyTokens;
  getPathVector(iter->key, keyTokens);
  
  //
  // This will hold the depth of the current item in the tree
  //
  std::size_t keyDepth = keyTokens.size() - 1;
  
  //
  // This will hold the offSet of the current key relative to the filter depth
  //
  std::size_t keyOffSet = keyDepth - filterDepth;
  
  if (keyOffSet == 0 && keyTokens[keyDepth] == resourceName)
  {
    //
    // This is an exact match.  We print it out and consume the iterator
    //
    ostr << "\"" << keyTokens[keyTokens.size() -1] << "\": " << "\"" << iter->value << "\"";
    iter++;
  }
  else if (keyOffSet > 0)
  {
    //
    // The item falls under a group of elements under the filter tree
    //
    ostr << "\"" << keyTokens[filterDepth] << "\":  {";
    
   
    while (true)
    { 
      std::string previousResource = keyTokens[filterDepth];
      if (!printOneRecord(filterDepth + 1, keyTokens[filterDepth + 1], records, iter, ostr))
        break;
      
      keyTokens.clear();
      getPathVector(iter->key, keyTokens);
      
      if (filterDepth + 1 >= keyTokens.size())
        break;
      
      if (previousResource != keyTokens[filterDepth])
        break;
      
      ostr << ",";
    }
    
    ostr << "}";
  }
  else 
  {
    return false;
  }
  
  return iter != records.end();
}

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
  _rootDocument(REST_DEFAULT_ROOT_DOCUMENT),
  _port(0),
  _isSecure(false)
{
}

RESTServer::RESTServer(int maxQueuedConnections, int maxThreads) :
  _socketHandle(0),
  _secureSocketHandle(0),
  _serverHandle(0),
  _serverParamsHandle(0),
  _maxQueuedConnections(maxQueuedConnections),
  _maxThreads(maxThreads),
  _rootDocument(REST_DEFAULT_ROOT_DOCUMENT),
  _port(0),
  _isSecure(false)
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

KeyValueStore* RESTServer::getStore(const std::string& path)
{
  std::vector<std::string> tokens = string_tokenize(path, "/");
  if(tokens.size() < 3)
    return 0;
  
  mutex_lock lock(_kvStoreMutex);
  
  KeyValueStore* pStore = 0;
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
    
    
    pStore = new KeyValueStore();
    pStore->open(strm.str());
    if (pStore->isOpen())
    {
     _kvStore.insert(std::pair<std::string, KeyValueStore*>(document, pStore));
    }
    else
    {
      delete pStore;
      pStore = 0;
    }
  }
  else
  {
    pStore = _kvStore.at(document);
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
    response.setStatus(HTTPResponse::HTTP_FORBIDDEN);
    response.send();
  }
  
  return authorized;
}

void RESTServer::onHandleRequest(Request& request, Response& response)
{ 
  if (!isAuthorized(request, response))
  {
    return;
  }
  
  if (string_starts_with(request.getURI(), _rootDocument.c_str()))
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

void RESTServer::onHandleRestRequest(Request& request, Response& response)
{
  std::string path = request.getURI();
  if (!string_ends_with(path, "/"))
  {
    path = path + std::string("/");
  }
  
  KeyValueStore* pStore = getStore(path);
  
  if (!pStore)
  {
    response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
    response.send();
    return;
  }
  
  std::vector<std::string> tokens;
  getPathVector(path, tokens);
   
  std::string filter = path + std::string("*");
  
  if (request.getMethod() == HTTPRequest::HTTP_GET)
  {
    KeyValueStore::Records records;
    pStore->getRecords(filter, records);
    
    if (records.empty())
    {
      response.setChunkedTransferEncoding(true);
      response.setStatus(HTTPResponse::HTTP_OK);
      response.send();
      return;
    }
    
    
    sendRestRecordsAsJson(tokens, records, response);
    
    //sendRestRecordsAsValuePairs(path, records, response);
    return;
  }
  else if (request.getMethod() == HTTPRequest::HTTP_DELETE)
  {
    pStore->delKeys(filter);
    response.setStatus(HTTPResponse::HTTP_OK);
    response.send();
    return;
  }
  else if (request.getMethod() == HTTPRequest::HTTP_PUT || request.getMethod() == HTTPRequest::HTTP_POST)
  {
    HTMLForm form(request, request.stream());
    if (!path.empty() && !form.empty() && form.has("value"))
    {
      std::string value = form.get("value");
      escapeString(value);
      pStore->put(path, value);
      response.setStatus(HTTPResponse::HTTP_OK);
      response.send();
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
  
  pos = 0;
  while((pos = str.find("\'", pos)) != std::string::npos)
  {
     str.replace(pos, 1, "\\\'");
     pos += strlen("\\\'");
  }
}

void RESTServer::sendRestJsonDocument(const std::vector<std::string>& pathVector, std::size_t depth, KeyValueStore::Records& unsorted, std::ostream& ostr)
{
  //
  // sort the records
  //
  std::list<KeyValueStore::Record> records;
  std::copy( unsorted.begin(), unsorted.end(), std::back_inserter(records));
  records.sort(compare_records);
  
 
  //
  // Loop through the records
  //
  std::list<KeyValueStore::Record>::iterator iter = records.begin();
  while (printOneRecord(depth, pathVector[depth], records, iter, ostr))
    ostr << ",";
}

void RESTServer::sendRestRecordsAsJson(const std::vector<std::string>& pathVector, KeyValueStore::Records& records, Response& response)
{
  response.setChunkedTransferEncoding(true);
  response.setContentType("text/json");
  std::ostream& ostr = response.send();
  ostr << "{";
  sendRestJsonDocument(pathVector, pathVector.size() - 1, records, ostr);
  ostr << "}";
}


void RESTServer::sendRestRecordsAsValuePairs(const std::string& path, const KeyValueStore::Records& records, Response& response)
{
  //
  // sort the records
  //
  std::list<KeyValueStore::Record> sorted;
  std::copy( records.begin(), records.end(), std::back_inserter(sorted));
  sorted.sort(compare_records);
  
  
  response.setChunkedTransferEncoding(true);
  response.setContentType("text/plain");
  std::ostream& ostr = response.send();

  for (std::list<KeyValueStore::Record>::const_iterator iter = sorted.begin(); iter != sorted.end(); iter++)
  {
    ostr << iter->key << ": " << iter->value << "\r\n";
  }
}

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


#include "sipxyard/RESTClient.h"
#include "os/OsLogger.h"


using Poco::Net::HTMLForm;
using Poco::Net::NameValueCollection;
using Poco::Net::HTTPBasicCredentials;
using Poco::Net::HTTPClientSession;
using Poco::Net::HTTPSClientSession;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPRequest; 
using Poco::StreamCopier;
using Poco::Net::HTTPMessage;

RESTClient::RESTClient(const std::string& host, unsigned short port) :
  _secure(false),
  _host(host),
  _port(port),
  _pSessionHandle(0)
{
}

RESTClient::RESTClient(const std::string& host, unsigned short port, bool secure) :
  _secure(secure),
  _host(host),
  _port(port),
  _pSessionHandle(0)
{
}

RESTClient::~RESTClient()
{
  delete _pSessionHandle;
}
    
void RESTClient::setCredentials(const std::string& user, const std::string& password)
{
  _user = user;
  _password = password;
}

bool RESTClient::execute_POST(const std::string& path, const Params& params, std::string& result, int& status)
{
  return execute(HTTPRequest::HTTP_POST, path, params, result, status);
}

bool RESTClient::execute_GET(const std::string& path, const Params& params, std::string& result, int& status)
{
  return execute(HTTPRequest::HTTP_GET, path, params, result, status);
}

bool RESTClient::execute_PUT(const std::string& path, const Params& params, std::string& result, int& status)
{
  return execute(HTTPRequest::HTTP_PUT, path, params, result,status);
}

bool RESTClient::execute_DELETE(const std::string& path, const Params& params, std::string& result, int& status)
{
  return execute(HTTPRequest::HTTP_DELETE, path, params, result, status);
}

bool RESTClient::execute(const std::string& method, const std::string& path, const Params& params, std::string& result, int& status)
{
  status = 0;
    
  try
  {
    HTTPClientSession* pSession = 0;

    if (!_pSessionHandle)
    {
      if (!_secure)
        pSession = new HTTPClientSession(_host, _port);
      else
        pSession = new HTTPSClientSession(_host, _port);
        
      _pSessionHandle = pSession;
    }
    else
    {
      pSession = _pSessionHandle;
    }

         
    HTTPRequest req(method, path, HTTPMessage::HTTP_1_1);
    
    if (!_user.empty())
    {
      HTTPBasicCredentials cred(_user, _password);
      cred.authenticate(req);
    }
    
    if (!params.empty())
    {
      HTMLForm form;
      for (Params::const_iterator iter = params.begin(); iter != params.end(); iter++)
      {
        form.add(iter->first, iter->second);
      }

      // Send the request.
      form.prepareSubmit(req);
      std::ostream& ostr = pSession->sendRequest(req);
      form.write(ostr);
    }
    else
    {
      pSession->sendRequest(req);
    }
    
    // Receive the response.
	  HTTPResponse res;
    
    std::istream& rs = pSession->receiveResponse(res);
    std::ostringstream strm;
    StreamCopier::copyStream(rs, strm);
    result = strm.str();

    status = res.getStatus();
    
    return (status == HTTPResponse::HTTP_OK);
  }
  catch(Poco::Exception e)
  {
    OS_LOG_ERROR(FAC_DB, "RESTClient::execute Exception: " << e.message())
    delete _pSessionHandle;
    _pSessionHandle = 0;
    return false;
  }
}

bool RESTClient::restPUT(const std::string& path, const std::string& value, int& status)
{
  Params params;
  params["value"] = value;
  std::string result;
  return execute_PUT(path, params, result, status);
}
    
bool RESTClient::restGET(const std::string& path, std::string& result, int& status)
{
  Params params;
  return execute_GET(path, params, result, status);
}

bool RESTClient::restDELETE(const std::string& path, int& status)
{
  Params params;
  std::string result;
  return execute_GET(path, params, result, status);
}

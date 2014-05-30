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

#ifndef RESTCLIENT_H_INCLUDED
#define	RESTCLIENT_H_ICLUDED

#include <map>
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/HTTPBasicCredentials.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/HTTPMessage.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/StreamCopier.h"
#include "Poco/Exception.h"

class RESTClient
{
public:
  typedef std::map<std::string, std::string> Params;

  RESTClient(const std::string& host, unsigned short port);

  RESTClient(const std::string& host, unsigned short port, bool secure);

  ~RESTClient();

  bool restPUT(const std::string& path, const std::string& value, int& status);

  bool restGET(const std::string& path, std::string& result, int& status);

  bool restDELETE(const std::string& path, int& status);

  void setCredentials(const std::string& user, const std::string& password);

  bool execute_POST(const std::string& path, const Params& params, std::string& result, int& status);

  bool execute_GET(const std::string& path, const Params& params, std::string& result, int& status);

  bool execute_PUT(const std::string& path, const Params& params, std::string& result, int& status);

  bool execute_DELETE(const std::string& path, const Params& params, std::string& result, int& status);

  bool execute(const std::string& method, const std::string& path, const Params& params, std::string& result, int& status);

private:
  bool _secure;
  std::string _host;
  unsigned short _port;
  std::string _user;
  std::string _password;
  Poco::Net::HTTPClientSession* _pSessionHandle;
};

#endif


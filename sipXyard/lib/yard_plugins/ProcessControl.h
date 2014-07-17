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

#ifndef PROCESSCONTROL_H_INCLUDED
#define	PROCESSCONTROL_H_INCLUDED

#include "sipxyard/YardPlugin.h"

class ProcessControl : public YardProcessor
{
public:
  ProcessControl();
  virtual ~ProcessControl();
  virtual bool willHandleRequest(const std::string& path);
  virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
  virtual void handleStatusRequest(const std::string& procName, Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
  virtual void handleInitRequest(const std::string& procName, const std::string& cmd, Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
  virtual void handleKillRequest(const std::string& procName, Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
};

#endif	/* PROCESSCONTROL_H */


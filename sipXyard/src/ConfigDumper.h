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

#ifndef CONFIGDUMPER_H_INCLUDED
#define	CONFIGDUMPER_H_INCLUDED

#include "YardProcessor.h"

class ConfigDumper : public YardProcessor
{
public:
  ConfigDumper();
  virtual ~ConfigDumper();
  virtual bool willHandleRequest(const std::string& path);
  virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
  bool dumpKeysAsIni(const std::string& path, const std::string& fileName, bool lastLeafAskey);
};

#endif	// CONFIGDUMPER_H_INCLUDED


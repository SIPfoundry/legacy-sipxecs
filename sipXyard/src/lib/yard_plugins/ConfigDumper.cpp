
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


#include "ConfigDumper.h"
#include "sipxyard/YardUtils.h"
#include "Poco/Net/HTMLForm.h"
#include "os/OsLogger.h"
#include <fstream>


static const char* API_PREFIX = "/api/system-config";


extern "C" YardPlugin::YardProcessorInstance create_yard_instance()
{
  return new ConfigDumper();
}

ConfigDumper::ConfigDumper()
{
}

ConfigDumper::~ConfigDumper()
{
}

bool ConfigDumper::willHandleRequest(const std::string& path)
{
  return path.find(API_PREFIX) == 0;
}

void ConfigDumper::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
  if (!willHandleRequest(request.getURI()))
  {
    response.setReason("Incorrect API Call");
    response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
    response.send();
    return;
  }
  
  std::string path = request.getURI();
  YardUtils::prepare_path(path);
  
  std::vector<std::string> pathVector;
  YardUtils::get_path_vector(path, pathVector);
   
  if (pathVector.size() < 4)
  {
    response.setReason("Incorrect API Call");
    response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
    response.send();
    return;
  }
  
  Poco::Net::HTMLForm form(request, request.stream());
  std::string procName;
  std::string cmd;
  
  procName = pathVector[2];
  cmd = pathVector[3];
  
  if (!procName.empty() && cmd == "replicate")
  {
    std::ostringstream resource;
    resource << "/root/system-config/" << procName;
    
    std::ostringstream fileName;
    
    if (form.has("path"))
    {
      fileName << form["path"]; 
    }
    else
    {
      fileName << _pServer->getDataDirectory() << "/" << procName << ".ini";
    }
    
    if (dumpKeysAsIni(resource.str(), fileName.str(), false))
    {
      response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
      response.send();
      
      OS_LOG_INFO(FAC_DB, "ConfigDumper::handleRequest - Exported " << resource.str() << " to " << fileName.str());
      
      return;
    }
    else
    {
      response.setReason("Unable To Replicate Configuration");
      response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
      response.send();
      OS_LOG_WARNING(FAC_DB,"ConfigDumper::handleRequest - FAILED to export " << resource.str() << " to " << fileName.str());
      return;
    }
  }
  else
  {
    response.setReason("Incorrect API Call");
    response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
    response.send();
    return;
  }
}

bool ConfigDumper::dumpKeysAsIni(const std::string& path, const std::string& fileName, bool lastLeafAsKey)
{
  std::string resource = path;
  YardUtils::prepare_path(resource);
  LevelDB* pStore = _pServer->getStore(resource, false);
  std::string filter = resource + std::string("*");
  LevelDB::Records records;
  pStore->getRecords(filter, records);
  
  if (records.empty())
    return false;
  
  std::ofstream ostrm(fileName.c_str());
  
  if (!ostrm.is_open())
    return false;
  
  for (std::vector<LevelDB::Record>::const_iterator iter = records.begin(); iter != records.end(); iter++)
  {
    std::string key = iter->key;
    if (lastLeafAsKey)
    {
      std::vector<std::string> pathVector;
      YardUtils::get_path_vector(iter->key, pathVector);
      key = pathVector[pathVector.size() - 1];
    }
    else
    {
      //
      // Use dot notation as key
      //
      key = YardUtils::path_to_dot_notation(key, 3);
    }
    
    if (!key.empty() && !iter->value.empty())
      ostrm << key << " = " << iter->value << "\r\n";
  }
  
  return true;
}



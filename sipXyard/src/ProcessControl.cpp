
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


#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include "ProcessControl.h"
#include "Poco/Net/HTMLForm.h"
#include "sipxyard/YardUtils.h"
#include "sipxyard/ProcessInfo.h"
#include "sipxyard/pstream.h"


static const char* API_PREFIX = "/api/proc";


using namespace redi;

// explicit instantiations of template classes
template class redi::basic_pstreambuf<char>;
template class redi::pstream_common<char>;
template class redi::basic_ipstream<char>;
template class redi::basic_opstream<char>;
template class redi::basic_pstream<char>;
template class redi::basic_rpstream<char>;

ProcessControl::ProcessControl()
{
  
}

ProcessControl::~ProcessControl()
{
  
}

bool ProcessControl::willHandleRequest(const std::string& path)
{
  return path.find(API_PREFIX) == 0;
}

void ProcessControl::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
  
  std::string procName;
  std::string cmd;
  
  if (!willHandleRequest(request.getURI()))
  {
    response.setReason("Incorrect API Call");
    response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
    response.send();
    return;
  }
  
  std::vector<std::string> pathVector;
  YardUtils::get_path_vector(request.getURI(),  pathVector);
  if (pathVector.size() < 4)
  {
    response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
    response.send();
    return;
  }
  
  procName = pathVector[2];
  cmd = pathVector[3];
  
  if (cmd == "status")
  {
    handleStatusRequest(procName, request, response);
  }
  else if (cmd == "restart")
  {
    handleInitRequest(procName, "restart",  request, response);
  }
  else if (cmd == "start")
  {
    handleInitRequest(procName, "start",  request, response);
  }
  else if (cmd == "kill")
  {
    handleKillRequest(procName, request, response);
  }
  else
  {
    response.setReason("Invalid API Call");
    response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
    response.send();
    return;
  }
}


void ProcessControl::handleStatusRequest(const std::string& procName, Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
  std::string pid;
  std::string pidFile;
  
  Poco::Net::HTMLForm form(request, request.stream());
  if (form.has("pid"))
  {
    pid = form["pid"];
  }
  else if (form.has("pid-file"))
  {
    pidFile = form["pid-file"];
    boost::filesystem::path fp(pidFile);
    if (boost::filesystem::exists(fp))
    {
      std::string buff;
      std::ifstream ifstrm(pidFile.c_str());
      if (std::getline(ifstrm, buff))
      {
        pid = buff;
      }
    }
  }
  
  if (!pid.empty())
  {
    pid_t processId = 0;
    try 
    { 
      processId = boost::lexical_cast<pid_t>(pid);
      ProcessInfo processInfo(procName, processId);
      
      if (!processInfo.isRunning())
      {
        std::ostringstream error;
        error << "Process " << procName << " Is Not Running.";
        response.setReason(error.str());
        response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
        response.send();
        return;
      }
      
      response.setChunkedTransferEncoding(true);
      response.setContentType("text/json");
      std::ostream& json = response.send();
      json << "{";
      json << "\"" << procName << "\":" << "[";
      json << processInfo.asJsonString();
      json << "]}";
    } 
    catch(...)
    {
      response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
      response.send();
      return;
    }
  }
  else
  {
    //
    // Process ID is not available.  Use the name and find all possible matches
    //
    std::vector<pid_t> pids;
    if (!ProcessInfo::getProcessIdFromName(procName, pids))
    {
      std::ostringstream error;
      error << "Process " << procName << " Is Not Found.";
      response.setReason(error.str());
      response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
      response.send();
      return;
    }
    
    std::ostringstream json;
    
    json << "{";
    json << "\"" << procName << "\":" << "[";
    for (std::size_t i = 0; i < pids.size(); i++)
    {
      try 
      { 
        pid_t pid = pids[i];
        ProcessInfo processInfo(procName, pid);
        json << processInfo.asJsonString();
        
        if (i < pids.size() - 1)
          json << ",";
      } 
      catch(...)
      {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        response.send();
        return;
      }
    }
    
    json << "]}";
    
    response.setChunkedTransferEncoding(true);
    response.setContentType("text/json");
    std::ostream& ostr = response.send();
    ostr << json.str();
  }
}

void ProcessControl::handleKillRequest(const std::string& procName, Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
  std::string pid;
  std::string pidFile;
  std::string signal;
  
  Poco::Net::HTMLForm form(request, request.stream());
  if (form.has("pid"))
  {
    pid = form["pid"];
  }
  else if (form.has("pid-file"))
  {
    pidFile = form["pid-file"];
    boost::filesystem::path fp(pidFile);
    if (boost::filesystem::exists(fp))
    {
      std::string buff;
      std::ifstream ifstrm(pidFile.c_str());
      if (std::getline(ifstrm, buff))
      {
        pid = buff;
      }
    }
  }
  
  if (pid.empty())
  {
    response.setReason("PID Not Specified");
    response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
    response.send();
    return;
  }
  
  if (form.has("signal"))
    signal = form["signal"];

  std::ostringstream script;
  
  if (signal.empty())
    script << "kill " << pid;
  else
    script << "kill -" << signal << " " << pid;
  ipstream cmd;
  cmd.open(script.str());

  if (cmd.eof() || !cmd.good())
  {
    std::ostringstream error;
    error << "Failed to kill " << pid;
    response.setReason(error.str());
    response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
    response.send();
  }
  else
  {
    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    response.send();
  }
}

void ProcessControl::handleInitRequest(const std::string& procName, const std::string& action, Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
  std::ostringstream script;
  script << "/etc/init.d/" << procName;
  
  
  if (!boost::filesystem::exists(script.str()))
  {
    std::ostringstream error;
    error << "Service " << procName << " Is Not Found.";
    response.setReason(error.str());
    response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
    response.send();
    return;
  }
  
  script << " " << action;
  
  ipstream cmd;
  cmd.open(script.str());
  
  if (cmd.eof() || !cmd.good())
  {
    std::ostringstream error;
    error << "Service " << procName << " Failed To Restart";
    response.setReason(error.str());
    response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
    response.send();
  }
  else
  {
    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    response.send();
  }
}


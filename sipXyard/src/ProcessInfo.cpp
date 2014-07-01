
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

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <signal.h>
#include <fstream>
#include <sstream>
#include <vector>

#include "sipxyard/ProcessInfo.h"
#include "sipxyard/YardUtils.h"

#define PROC_DIRECTORY "/proc"
#define PROC_CMDLINE "cmdline"
#define PROC_STATUS "status" 

static bool headerSplit(
    const std::string & header,
    std::string & name,
    std::string & value)
{
  size_t nameBound = header.find_first_of(':');
  if (nameBound == std::string::npos)
  {
    return false;
  }
  name = header.substr(0,nameBound);
  value = header.substr(nameBound+1);
  boost::trim(name);
  boost::trim(value);
  return true;
}

static bool getStatusHeader(const std::string& file, const std::string& name_, std::string& value_)
{
  std::ifstream procif(file.c_str());
  if (!procif.is_open())
    return false;

  std::string headerName = name_;
  boost::to_lower(headerName);

  while(!procif.eof() && !procif.bad())
  {
    std::string line;
    std::getline(procif, line);
    std::string name;
    std::string value;    
    if (headerSplit(line, name, value))
    {
      boost::to_lower(name);
      if (name == headerName)
      {
        value_ = value;
        return true;
      }
    }
    line.clear();
  }

  return false;
}

static bool getStatusHeader(const std::string& file, const std::string& name_, std::vector<std::string>& values_)
{
  std::ifstream procif(file.c_str());
  if (!procif.is_open())
    return false;

  std::string headerName = name_;
  boost::to_lower(headerName);

  values_.clear();
 
  while(!procif.eof() && !procif.bad())
  {
    std::string line;
    std::getline(procif, line);
    std::string name;
    std::string value;    
    if (headerSplit(line, name, value))
    {
      boost::to_lower(name);
      if (name == headerName)
      {
        values_.push_back(value);
      }
    }
  }

 
  return !values_.empty();
}

template <typename T>
T string_to_number(const std::string& str)
{
  try { return boost::lexical_cast<T>(str);} catch(...){return 0;};
}

static unsigned int kbToInt(const std::string& kb)
{
  std::vector<std::string> tokens;
  boost::split(tokens, kb, boost::is_any_of(" "), boost::token_compress_on);
  if (tokens.size() < 2)
    return 0;
  return string_to_number<unsigned int>(tokens[0].c_str());
}

static unsigned int kbTotal(const std::string& file, const std::string& key)
{
  std::vector<std::string> blocks;
  if (!getStatusHeader(file.c_str(), key.c_str(), blocks))
    return 0;
  
  unsigned int total = 0;
  for (std::vector<std::string>::iterator iter = blocks.begin(); iter != blocks.end(); iter++)
    total += kbToInt(*iter);
  
  return total;
}


ProcessInfo::ProcessInfo(const std::string& processName, const std::string& pidFile) :
  _pid(0),
  _pidFile(pidFile),
  _processName(processName)
{
}

ProcessInfo::ProcessInfo(const std::string& processName, pid_t pid) :
  _pid(pid),
  _pidFile(),
  _processName(processName)
{
}

bool ProcessInfo::isRunning() const
{
  std::ostringstream procd;
  procd << "/proc/" << _pid;
  bool ok = false;
  if (boost::filesystem::exists(procd.str().c_str()))
  {
    procd << "/status";
    std::string procName;
    if (!_processName.empty())
    {
      ok = (getStatusHeader(procd.str().c_str(), "name", procName) && _processName.find(procName) == 0);
    }
    else
    {
      ok = true;
    }
  }
  
  return ok;
}

unsigned int ProcessInfo::getMem(bool shared) const
{
  pid_t pid = getProcessId();
  
  if (!pid)
  {
    std::cerr << "Unable to determine process id" << std::endl;
    return 0;
  }
  
  std::ostringstream smapFile;
  smapFile << "/proc/" << pid << "/smaps";

  if (!shared)
    return kbTotal(smapFile.str(), "Private_Dirty") + kbTotal(smapFile.str(), "Private_Clean");
  else
    return kbTotal(smapFile.str(), "Shared_Dirty") + kbTotal(smapFile.str(), "Shared_Clean");
}

unsigned int ProcessInfo::getSystemMem() const
{
  std::string memTotal;
  if (!getStatusHeader("/proc/meminfo", "memtotal", memTotal))
    return 0;
  return string_to_number<unsigned int>(memTotal.c_str());
}

pid_t ProcessInfo::getProcessId() const
{
  if (!_pidFile.empty())
  {
    boost::filesystem::path fp(_pidFile);
    if (boost::filesystem::exists(fp))
    {
      std::string buff;
      std::ifstream ifstrm(_pidFile.c_str());
      if (std::getline(ifstrm, buff))
      {
        _pid = (pid_t)string_to_number<int>(buff.c_str());
      }
    }
  }
    
  return _pid;
}

bool ProcessInfo::getProcessIdFromName(const std::string& name, std::vector<pid_t>& pids)
{
  boost::filesystem::path directory(PROC_DIRECTORY);
  boost::filesystem::directory_iterator end_itr; // default construction yields past-the-end
  for (boost::filesystem::directory_iterator itr(directory); itr != end_itr; ++itr)
  {
    if (!boost::filesystem::is_directory(itr->status()))
    {
      continue;
    }
    
    boost::filesystem::path currentProc = itr->path();
    std::string procFileName;
    #if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION >= 3
      procFileName = currentProc.filename().native();
    #else
      procFileName = currentProc.filename();
    #endif

    pid_t pid = 0;
    try 
    { 
      pid = boost::lexical_cast<pid_t>(procFileName);
    } 
    catch(...)
    {
      continue;
    }
    
   
    std::string statusFile;
    #if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION >= 3
      statusFile =  currentProc.native() + "/status";
    #else
      statusFile =  currentProc.string() + "/status";
    #endif
      
    std::string procName;
    if (getStatusHeader(statusFile, "Name", procName))
    {
      if (procName == name)
      {
        pids.push_back(pid);
      }
    }
  }
  
  return !pids.empty();
}

std::string ProcessInfo::asJsonString() const
{
  std::ostringstream json;
  json << "{";
  json << "\"PID\"" << ": \"" << getProcessId() << "\"" << ",";
  json << "\"RES\"" << ": \"" << getMem(false) << " kB\"" ;
  json << "}";
  return json.str();
}




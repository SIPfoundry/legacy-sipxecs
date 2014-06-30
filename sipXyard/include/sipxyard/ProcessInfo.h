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


#ifndef PROCESSINFO_H_INCLUDED
#define	PROCESSINFO_H_INCLUDED

#include <string>
#include <unistd.h>

class ProcessInfo
{
public:
  ProcessInfo(const std::string& processName, const std::string& pidFile);
  
  ProcessInfo(const std::string& processName, pid_t pid);
  
  bool isRunning() const;
  
  unsigned int getMem(bool shared = false) const;
  
  unsigned int getSystemMem() const;
  
  pid_t getProcessId() const;
  
  static bool getProcessIdFromName(const std::string& name, std::vector<pid_t>& pids);
  
  std::string asJsonString() const;
private:
  mutable pid_t _pid;
  std::string _pidFile;
  std::string _processName;
};


#endif


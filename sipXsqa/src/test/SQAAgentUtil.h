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

#ifndef SQAAgentUtil_H
#define SQAAgentUtil_H

#include "sqa/StateQueueAgent.h"
#include "sqa/sqaclient.h"


#include <sstream>
#include <string>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <iostream>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>   // Declaration for exit()
#include <signal.h>

struct SQAAgentData
{
  typedef boost::shared_ptr<SQAAgentData> Ptr;

  bool ha;
  ServiceOptions *service;
  StateQueueAgent *agent;
  std::string id;

  std::string configFilePath;
  std::string logFilePath;
  std::string clientIniFilePath;

  std::string sqaControlPort;
  std::string sqaControlAddress;

  std::string sqaZmqSubscriptionPort;
  std::string sqaZmqSubscriptionAddress;

  std::string sqaControlAddressAll;

  pid_t pid;

  SQAAgentData():ha(false), service(0),agent(0),pid(0) {};
};

class SQAAgentUtil : boost::noncopyable
{
public:
  SQAAgentUtil() {}
  SQAAgentUtil(const std::string& program);

  ~SQAAgentUtil() {}

  void setProgram(const std::string& program);
  void generateSQAAgentData(std::vector<SQAAgentData::Ptr> &agents, unsigned int agentNum, bool ha);

  void generateSQAClientIni(SQAAgentData::Ptr data, bool noLocalSQAAgent);

  bool startSQAAgent(SQAAgentData::Ptr agentData);
  bool stopSQAAgent(SQAAgentData::Ptr agentData);

private:
  void generateSQAConfig(SQAAgentData::Ptr data);


  std::string _program;
};


#endif  /* SQAAgentUtil_H */


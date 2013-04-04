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

#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include "SQAAgentUtil.h"

const char* g_sqaAddress[] = {"192.168.13.2", "192.168.13.200", "192.168.13.201"};

const unsigned int g_defaultSqaControlPort = 6240;
const unsigned int g_defaultZmqSubscriptionPort = 6242;

SQAAgentUtil::SQAAgentUtil(const std::string& program) :
     _program(program)
{
}

void SQAAgentUtil::setProgram(const std::string& program)
{
     _program == program;
}


void SQAAgentUtil::generateSQAAgentData(std::vector<SQAAgentData::Ptr>& agents, unsigned int agentsNum, bool ha)
{
  agents.clear();

  std::string sqaControlAddressAll;

  for (unsigned int i = 0; i < agentsNum; i++)
  {
    SQAAgentData::Ptr data = SQAAgentData::Ptr(new SQAAgentData());

    {
      std::stringstream strm;
      strm << "SQATestAgent" << i;
      data->id = strm.str();
    }

    {
      std::stringstream strm;
      strm << g_defaultSqaControlPort;
      data->sqaControlPort = strm.str();
    }
    {
      std::stringstream strm;
      strm << g_defaultZmqSubscriptionPort;
      data->sqaZmqSubscriptionPort = strm.str();
    }

    data->sqaControlAddress = g_sqaAddress[i];
    data->sqaZmqSubscriptionAddress = g_sqaAddress[i];

    {
      std::stringstream strm;
      strm << "sipxsqa-config-" << i;
      data->configFilePath = strm.str();
    }

    {
      std::stringstream strm;
      strm << "sipxsqa-client.ini-" << i;
      data->clientIniFilePath = strm.str();
    }

    {
      std::stringstream strm;
      strm << "sipxsqa.log-" << i;
      data->logFilePath = strm.str();
    }

    if (ha)
    {
      sqaControlAddressAll += data->sqaControlAddress + ",";
    }

    data->ha = ha;

    agents.push_back(data);
  }

  std::vector<SQAAgentData::Ptr>::iterator it;
  for (it = agents.begin(); it != agents.end(); it++)
  {
    SQAAgentData::Ptr data = *it;

    if (ha)
    {
      data->sqaControlAddressAll = sqaControlAddressAll;
    }

    generateSQAConfig(data);

    int argc = 5;
    const char *argv[argc];
    argv[0] = _program.c_str();
    argv[1] = "--config-file";
    argv[2] = data->configFilePath.data();
    argv[3] = "--log-file";
    argv[4] = data->logFilePath.data();

    data->service= new ServiceOptions(argc, (char**)argv, data->id);
    data->service->addDaemonOptions();
    data->service->addOptionString("zmq-subscription-address", ": Address where to subscribe for events.");
    data->service->addOptionString("zmq-subscription-port", ": Port where to send subscription for events.");
    data->service->addOptionString("sqa-control-port", ": Port where to send control commands.");
    data->service->addOptionString("sqa-control-address", ": Address where to send control commands.");

    if (!data->service->parseOptions() ||
            !data->service->hasOption("zmq-subscription-address") ||
            !data->service->hasOption("zmq-subscription-port") ||
            !data->service->hasOption("sqa-control-port") ||
            !data->service->hasOption("sqa-control-address") )
    {
      data->service->displayUsage(std::cerr);
      assert(false);
    }
  }
}

void SQAAgentUtil::generateSQAConfig(SQAAgentData::Ptr data)
{
  std::ofstream ofs(data->configFilePath.data(), std::ios_base::trunc | std::ios_base::in);

  ofs << "log-level=7" << "\n"
      << "sqa-control-port=" << data->sqaControlPort << "\n"
      << "zmq-subscription-port=" << data->sqaZmqSubscriptionPort << "\n"
      << "sqa-control-address=" << data->sqaControlAddress << "\n"
      << "zmq-subscription-address=" << data->sqaZmqSubscriptionAddress <<"\n";

  if (!data->sqaControlAddressAll.empty())
  {
    ofs << "sqa-control-address-all=" << data->sqaControlAddressAll << "\n";
  }
}

void SQAAgentUtil::generateSQAClientIni(SQAAgentData::Ptr data, bool noLocalSQAAgent)
{
  std::ofstream ofs(data->clientIniFilePath.data(), std::ios_base::trunc | std::ios_base::in);

  ofs << "enabled=true" << "\n"
      << "sqa-control-port=" << data->sqaControlPort << "\n";

  if (!noLocalSQAAgent)
  {
      ofs << "sqa-control-address=" << data->sqaControlAddress << "\n";
  }

  if (!data->sqaControlAddressAll.empty())
  {
    ofs << "sqa-control-address-all=" << data->sqaControlAddressAll << "\n";
  }
}

bool SQAAgentUtil::startSQAAgent(SQAAgentData::Ptr agentData)
{
  if (agentData->ha)
  {
    pid_t pid = fork();
    if (pid == 0)                // child
    {
    // Code only executed by child process
      agentData->agent = new StateQueueAgent(agentData->id, *agentData->service);
      agentData->agent->run();

      agentData->service->waitForTerminationRequest();
      delete agentData->agent;
      delete agentData->service;
      exit(1);
    }
    else if (pid < 0)            // failed to fork
    {
      return false;
    }
    else                                   // parent
    {
      agentData->pid = pid;
      boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
    }
  }
  else
  {
    agentData->agent = new StateQueueAgent(agentData->id, *agentData->service);
    agentData->agent->run();
  }

  return true;;
}

bool SQAAgentUtil::stopSQAAgent(SQAAgentData::Ptr data)
{
  if (data->ha)
  {
    if (data->pid > 0)
    {
      kill(data->pid, SIGTERM);
      boost::this_thread::sleep(boost::posix_time::milliseconds(1));
      kill(data->pid, SIGTERM);
      boost::this_thread::sleep(boost::posix_time::milliseconds(100));

      int status;
      pid_t pid = waitpid(data->pid, &status, WNOHANG);
      if (pid == data->pid)
      {
        return true;
      }
    }

    return false;
  }
  else
  {
    if (data->agent)
    {
      delete data->agent;
      data->agent = 0;
    }
    else
    {
      return false;
    }

    if (data->service)
    {
      delete data->service;
      data->service = 0;
    }
    else
    {
      return false;
    }

    return true;
  }
}

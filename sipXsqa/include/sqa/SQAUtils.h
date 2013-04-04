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

#ifndef SQAUTILS_H_INCLUDED
#define SQAUTILS_H_INCLUDED

#include <iostream>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

#include "sqa/SQADefines.h"

extern const char* connectionEventStr[];

#define LOG_TAG_WID(id) this->getClassName() << "::" << __FUNCTION__ << "." << __LINE__ << " this:" << this << " id '" << id << "'"
#define LOG_TAG() this->getClassName() << "::" << __FUNCTION__ << "." << __LINE__ << " this:" << this

class SQAUtil
{
public:
  static const int SQAClientRolePublisher = 0x1;
  static const int SQAClientRoleWatcher = 0x10;

  static const int SQAClientRoleDealer = 0x1000;
  static const int SQAClientRoleWorker = 0x10000;

  static const int SQAClientUnknown = 0;
  static const int SQAClientPublisher = SQAClientRolePublisher;
  static const int SQAClientDealer = SQAClientRolePublisher | SQAClientRoleDealer;
  static const int SQAClientWatcher = SQAClientRoleWatcher;
  static const int SQAClientWorker = SQAClientRoleWatcher | SQAClientRoleWorker;


  static bool isPublisher(int clientType)
  {
    return (clientType & SQAClientRolePublisher);
  }

  static bool isPublisherOnly(int clientType)
  {
    return ((clientType & SQAClientRolePublisher) && !(clientType & SQAClientRoleDealer));
  }

  static bool isDealer(int clientType)
  {
    return (clientType & SQAClientRolePublisher && clientType & SQAClientRoleDealer);
  }

  static bool isWatcher(int clientType)
  {
    return (clientType & SQAClientRoleWatcher);
  }

  static bool isWatcherOnly(int clientType)
  {
    return (clientType == SQAClientRoleWatcher);
  }

  static bool isWorker(int clientType)
  {
    return (clientType & SQAClientRoleWatcher && clientType & SQAClientRoleWorker);
  }

  static const char* getClientStr(int clientType)
  {
    if (isPublisherOnly(clientType))
    {
      return "publisher";
    }
    if (isWatcherOnly(clientType))
    {
      return "watcher";
    }
    if (isDealer(clientType))
    {
      return "dealer";
    }
    if (isWorker(clientType))
    {
      return "worker";
    }

    return "unknown";
  }

  static const char* getConnectionEventStr(ConnectionEvent connectionEvent);

  static void generateRecordId(std::string &recordId, ConnectionEvent event);

  static bool generateId(std::string &id, int serviceType, const std::string &eventId)
  {
      bool ret = true;
      std::ostringstream ss;

      if (isDealer(serviceType) || isWorker(serviceType))
      {
        ss << DealerWorkerPrefix;
      }
      else if (isPublisherOnly(serviceType) || isWatcherOnly(serviceType))
      {
        ss << PublisherWatcherPrefix;
      }
      else
      {
        ss << UnknownPrefix;
        ret = false;
      }

      if (!eventId.empty())
      {
          ss << "." << eventId;
      }

      ss << "." << std::hex << std::uppercase
              << std::setw(4) << std::setfill('0') << (int) ((float) (0x10000) * random () / (RAND_MAX + 1.0)) << "-"
              << std::setw(4) << std::setfill('0') << (int) ((float) (0x10000) * random () / (RAND_MAX + 1.0));

      id = ss.str();

      return ret;
  }

  static bool validateId(const std::string &id, int clientType);
  static bool validateId(const std::string &id, int clientType, const std::string &eventId);

  static bool validateIdHexComponent(const std::string &hex);

  static bool generateZmqEventId(std::string &zmqEventId, int clientType, std::string &eventId);
};


#endif //SQAUTILS_H_INCLUDED

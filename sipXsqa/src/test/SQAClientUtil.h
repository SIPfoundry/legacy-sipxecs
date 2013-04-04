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

#ifndef SQAClientUtil_H
#define SQAClientUtil_H

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

struct SQAClientData
{
  int _idx;
  int _type;
  std::string _applicationId;
  std::string _coreApplicationId;
  std::string _servicesAddresses;
  std::string _servicesAddressesAll;
  std::string _servicePort;
  std::string _zmqEventId;
  std::size_t _poolSize;
  int _readTimeout;
  int _writeTimeout;
  int _keepAliveTimeout;
  int _signinTimeout;
  unsigned int _fallbackTimeout;
  bool _serviceIsUp;
  bool _serviceWasUp;
  bool _fallbackDataSet;
  bool _fallbackActive;

  SQAClientData(
      int idx,
      int type,
      const std::string& applicationId,
      const std::string& servicesAddresses,
      const std::string& servicesAddressesAll,
      const std::string& servicePort,
      const std::string& zmqEventId,
      std::size_t poolSize = 1,
      int readTimeout = SQA_CONN_READ_TIMEOUT,
      int writeTimeout = SQA_CONN_WRITE_TIMEOUT,
      int keepAliveTimeout = 2,
      int signinTimeout = 2,
      unsigned int fallbackTimeout = SQA_FALLBACK_TIMEOUT):
        _idx(idx),
        _type(type),
        _applicationId(applicationId),
        _servicesAddresses(servicesAddresses),
        _servicesAddressesAll(servicesAddressesAll),
        _servicePort(servicePort),
        _zmqEventId(zmqEventId),
        _poolSize(poolSize),
        _readTimeout(readTimeout),
        _writeTimeout(writeTimeout),
        _keepAliveTimeout(keepAliveTimeout),
        _signinTimeout(signinTimeout),
        _fallbackTimeout(fallbackTimeout),
        _serviceIsUp(false),
        _serviceWasUp(false),
        _fallbackDataSet(false),
        _fallbackActive(false)
  {
    if (idx > 0)
    {
      std::stringstream strm;
      strm << _applicationId << "-" << idx;
      _coreApplicationId = strm.str();
    }
    else
    {
      _coreApplicationId = _applicationId;
    }
  }

  void setServiceUp() {_serviceIsUp = true; _serviceWasUp = true;}
  void setServiceDown() {_serviceIsUp = false;}
  void setHasFallbackData() {_fallbackDataSet = true;}
  void setFallbackActive() {_fallbackActive = true;}
  void setFallbackInactive() {_fallbackActive = false;}
};

class SQAClientUtil : boost::noncopyable
{
public:
  static void checkSQAClientCoreAfterStartup(StateQueueClient* client, StateQueueClient::SQAClientCore* core, SQAClientData* cd);
  static void checkSQAClientCoreAfterStop(StateQueueClient::SQAClientCore* core);
  static void checkStateQueueClientAfterStartup(StateQueueClient& client, SQAClientData* cd, int coreIdx=0);
  static void checkSQAClientAfterStop(StateQueueClient& client);
  static void checkPublisherAfterStartup(StateQueueClient& publisher, SQAClientData* cd);
  static void checkPublisherFallback(StateQueueClient& publisher, std::vector<SQAClientData*> cd, int fallbackIdx);
  static void checkDealerAfterStartup(StateQueueClient& publisher, SQAClientData* cd);
  static void checkWatcherAfterStartup(StateQueueClient& watcher, std::vector<SQAClientData*> cd);
  static void checkWorkerAfterStartup(StateQueueClient& worker, std::vector<SQAClientData*> cd);

  static void checkRegularPublishWatch(StateQueueClient& publisher, StateQueueClient& watcher, const std::string& eventId, const std::string& data, bool noresponse);

  static void checkRegularEnqueuePop(StateQueueClient& dealer, StateQueueClient& worker, const std::string& eventId, const std::string& data, int serviceId);
//private:
  static void checkFallbackDataAFterStartup(StateQueueClient& client, SQAClientData* cd);
};


#endif  /* SQAClientUtil_H */

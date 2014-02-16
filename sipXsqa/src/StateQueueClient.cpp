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

#include "sqa/StateQueueClient.h"
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <os/OsServiceOptions.h>

// Moved StateQueueClient::BlockingTcpClient::connect to a separate file for fixing:
//
//  /usr/local/sipxecs-branch-4.6/include/os/OsDefs.h:59:22: error: conflicting declaration 'typedef __int64_t __int64'
//   typedef __int64_t __int64;
// sqaclient_wrap.cxx:149:21: error: '__int64' has a previous declaration as 'typedef long long int __int64'
//   typedef long long __int64;

bool StateQueueClient::BlockingTcpClient::connect()
{
  //
  // Initialize State Queue Agent Publisher if an address is provided
  //
  if (_serviceAddress.empty() || _servicePort.empty())
  {
    std::string sqaControlAddress;
    std::string sqaControlPort;
    std::ostringstream sqaconfig;
    sqaconfig << SIPX_CONFDIR << "/" << "sipxsqa-client.ini";
    OsServiceOptions configOptions(sqaconfig.str());
    std::string controlAddress;
    std::string controlPort;
    if (configOptions.parseOptions())
    {
      bool enabled = false;
      if (configOptions.getOption("enabled", enabled, enabled) && enabled)
      {
        configOptions.getOption("sqa-control-address", _serviceAddress);
        configOptions.getOption("sqa-control-port", _servicePort);
      }
      else
      {
        OS_LOG_ERROR(FAC_NET, "BlockingTcpClient::connect() this:" << this << " Unable to read connection information from " << sqaconfig.str());
        return false;
      }
    }
  }

  if(_serviceAddress.empty() || _servicePort.empty())
  {
    OS_LOG_ERROR(FAC_NET, "BlockingTcpClient::connect() this:" << this << " remote address is not set");
    return false;
  }

  return connect(_serviceAddress, _servicePort);
}

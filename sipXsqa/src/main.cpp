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



#include "sqa/ServiceOptions.h"
#include "sqa/StateQueueAgent.h"
#include "sqa/StateQueueDriverTest.h"
#include "sqa/StateQueueConnection.h"
#include "sipXecsService/SipXApplication.h"

#define SIPXSQA_APP_NAME              "StateQueueAgent"

#include "sipXecsService/SipXApplication.h"

#define SIPXSQA_APP_NAME              "StateQueueAgent"

int main(int argc, char** argv)
{
  SipXApplicationData sqaData =
  {
      SIPXSQA_APP_NAME,
      "",
      "",
      "",
      "",
      false, // do not check mongo connection
      true, // increase application file descriptor limits
      SipXApplicationData::ConfigFileFormatIni, // format type for configuration file
      OsMsgQShared::QUEUE_LIMITED, //limited queue
  };

  SipXApplication& sipXApplication = SipXApplication::instance();
  OsServiceOptions& osServiceOptions = sipXApplication.getConfig();

  osServiceOptions.addOptionString("zmq-subscription-address", ": Address where to subscribe for events.", OsServiceOptions::ConfigOption);
  osServiceOptions.addOptionString("zmq-subscription-port", ": Port where to send subscription for events.", OsServiceOptions::ConfigOption);
  osServiceOptions.addOptionString("sqa-control-port", ": Port where to send control commands.", OsServiceOptions::ConfigOption);
  osServiceOptions.addOptionString("sqa-control-address", ": Address where to send control commands.", OsServiceOptions::ConfigOption);
  osServiceOptions.addOptionInt("id", ": Address where to send control commands.", OsServiceOptions::ConfigOption);
  osServiceOptions.addOptionFlag("test-driver", ": Set this flag if you want to run the driver unit tests to ensure proper operations.", OsServiceOptions::ConfigOption);

  // NOTE: this might exit application in case of failure
  sipXApplication.init(argc, argv, sqaData);

  if (!osServiceOptions.hasOption("zmq-subscription-address") ||
      !osServiceOptions.hasOption("zmq-subscription-port") ||
      !osServiceOptions.hasOption("sqa-control-port") ||
      !osServiceOptions.hasOption("sqa-control-address") )
  {
    sipXApplication.displayUsage(std::cerr);
    return -1;
  }

  StateQueueAgent sqa("main", osServiceOptions);
  sqa.run();

  if (osServiceOptions.hasOption("test-driver"))
  {
    StateQueueDriverTest test(sqa);
    if (!test.runTests())
      return -1;
    sqa.stop();
    return 0;
  }
  OS_LOG_INFO(FAC_NET, "State Queue Agent process STARTED.");
  sipXApplication.waitForTerminationRequest(1);
  OS_LOG_INFO(FAC_NET, "State Queue Agent process TERMINATED.");
  return 0;
}


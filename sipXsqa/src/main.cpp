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

int main(int argc, char** argv)
{
  ServiceOptions::daemonize(argc, argv);

  ServiceOptions service(argc, argv, "StateQueueAgent", "1.0.0", "Copyright Ezuce Inc. (All Rights Reserved)");
  service.addDaemonOptions();
  service.addOptionString("zmq-subscription-address", ": Address where to subscribe for events.");
  service.addOptionString("zmq-subscription-port", ": Port where to send subscription for events.");
  service.addOptionString("sqa-control-port", ": Port where to send control commands.");
  service.addOptionString("sqa-control-address", ": Address where to send control commands.");

  service.addOptionInt("id", ": Address where to send control commands.");
  service.addOptionFlag("test-driver", ": Set this flag if you want to run the driver unit tests to ensure proper operations.");

  if (!service.parseOptions() ||
          !service.hasOption("zmq-subscription-address") ||
          !service.hasOption("zmq-subscription-port") ||
          !service.hasOption("sqa-control-port") ||
          !service.hasOption("sqa-control-address") )
  {
    service.displayUsage(std::cerr);
    return -1;
  }

  StateQueueAgent sqa("main", service);
  sqa.run();

  if (service.hasOption("test-driver"))
  {
    StateQueueDriverTest test(sqa);
    if (!test.runTests())
      return -1;
    sqa.stop();
    return 0;
  }
  OS_LOG_INFO(FAC_NET, "State Queue Agent process STARTED.");
  service.waitForTerminationRequest();
  OS_LOG_INFO(FAC_NET, "State Queue Agent process TERMINATED.");
  return 0;
}


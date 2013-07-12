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

#include <vector>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <boost/array.hpp>
#include "os/OsServiceOptions.h"


class OsServiceOptionsTest : public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE(OsServiceOptionsTest);
  CPPUNIT_TEST(testServiceOptions);
  CPPUNIT_TEST_SUITE_END();
public:

  void vectorToCArray(const std::vector<std::string>& args, char*** argv)
  {
    *argv = (char**)std::malloc((args.size() + 1) * sizeof(char*));
    int i=0;
    for(std::vector<std::string>::const_iterator iter = args.begin();
        iter != args.end();
        iter++, ++i)
    {
      std::string arg = *iter;
      (*argv)[i] = (char*)std::malloc((arg.length()+1) * sizeof(char));
      std::strcpy((*argv)[i], arg.c_str());
    }
    (*argv)[args.size()] = NULL; // argv must be NULL terminated
  }

  void freeCArray(int argc, char*** argv)
  {
    for (int i = 0; i < argc; i++)
      free((*argv)[i]);
    free(*argv);
  }

  void testServiceOptions()
  {
    std::vector<std::string> args;

    args.push_back("ServiceOptions");

    //
    // This is a flag.  No value needs to be set
    //
    args.push_back("--is-test-run");

    args.push_back("--local-uri");
    args.push_back("sip:test@ezuce.com");

    args.push_back("--resource-uri");
    args.push_back("sip:resource@ezuce.com");

    args.push_back("--proxy-uri");
    args.push_back("sip:openuc.ezuce.com");

    args.push_back("--sip-port");
    args.push_back("5060");

    args.push_back("--refresh-interval");
    args.push_back("1800");

    args.push_back("--log-file");
    args.push_back("OsServiceOptionsTest");

    args.push_back("log-level");
    args.push_back("7");


    char** argv = 0;
    vectorToCArray(args, &argv);
    OsServiceOptions service(args.size(), argv, "ServiceOptions", "1.0", "Ezuce Inc. All Rights Reserved");
    service.addDaemonOptions();

    service.addOptionFlag("is-test-run", ": Flag signifying this is a test run",  OsServiceOptions::ConfigOption);
    service.addOptionString("local-uri", ": URI to be used in the From header",  OsServiceOptions::ConfigOption);
    service.addOptionString("resource-uri", ": URI of the resource",  OsServiceOptions::ConfigOption);
    service.addOptionString("proxy-uri", ": URI of the proxy that will handle the subscription",  OsServiceOptions::ConfigOption);
    service.addOptionInt("sip-port", "Port to be used by the SIP transport",  OsServiceOptions::ConfigOption);
    service.addOptionInt("refresh-interval", "The refresh intervals for subscriptions. (seconds)", OsServiceOptions::ConfigOption);

    CPPUNIT_ASSERT(service.parseOptions());

    OS_LOG_DEBUG(FAC_KERNEL, "testServiceOptions STARTED");

    CPPUNIT_ASSERT(service.hasOption("is-test-run", true));

    std::string value;
    CPPUNIT_ASSERT(service.getOption("local-uri", value));
    CPPUNIT_ASSERT("sip:test@ezuce.com" == value);

    CPPUNIT_ASSERT(service.getOption("resource-uri", value));
    CPPUNIT_ASSERT("sip:resource@ezuce.com" == value);

    CPPUNIT_ASSERT(service.getOption("proxy-uri", value));
    CPPUNIT_ASSERT("sip:openuc.ezuce.com" == value);

    int val;
    CPPUNIT_ASSERT(service.getOption("sip-port", val));
    CPPUNIT_ASSERT(5060 == val);

    CPPUNIT_ASSERT(service.getOption("refresh-interval", val));
    CPPUNIT_ASSERT(1800 == val);

    OS_LOG_DEBUG(FAC_KERNEL, "testServiceOptions ENDED");
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsServiceOptionsTest);
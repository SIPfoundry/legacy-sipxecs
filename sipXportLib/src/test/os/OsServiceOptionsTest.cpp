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

#include <stdio.h>
#include <sys/resource.h>
#include <vector>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <boost/array.hpp>
#include "os/OsServiceOptions.h"


class OsServiceOptionsTest : public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE(OsServiceOptionsTest);

  CPPUNIT_TEST(testServiceOptions);
  CPPUNIT_TEST(testServiceOptionsConfigOnly);
  CPPUNIT_TEST(testServiceOptionsVectors);
  CPPUNIT_TEST(testServiceOptions_parseOptionsFlags);
<<<<<<< HEAD
=======
  CPPUNIT_TEST(testServiceOptions_ConfigFileTypeConfigDb);
>>>>>>> XX-8299 Make log level change dynamic
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

  void freeCArray(std::size_t argc, char*** argv)
  {
    for (std::size_t i = 0; i < argc; i++)
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
    service._unitTestMode = true;
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

    freeCArray(args.size(), &argv);
    args.clear();

    //
    // Test required field
    //
    args.push_back("ServiceOptions");
    args.push_back("--is-test-run");
    vectorToCArray(args, &argv);
    
    OsServiceOptions service2(args.size(), argv, "ServiceOptions", "1.0", "Ezuce Inc. All Rights Reserved");
    service2._unitTestMode = true;
    service2.addOptionFlag("is-test-run", ": Flag signifying this is a test run",  OsServiceOptions::ConfigOption);
    service2.addOptionString("local-uri", ": URI to be used in the From header",  OsServiceOptions::ConfigOption, true);

    //
    // parseOptions must fail.  local-uri is required
    //
    CPPUNIT_ASSERT(!service2.parseOptions());

    freeCArray(args.size(), &argv);


    args.clear();

    //
    // Test required field with alternate
    //
    args.push_back("ServiceOptions");
    args.push_back("--is-test-run");
    vectorToCArray(args, &argv);

    OsServiceOptions service3(args.size(), argv, "ServiceOptions", "1.0", "Ezuce Inc. All Rights Reserved");
    service3._unitTestMode = true;
    service3.addOptionFlag("is-test-run", ": Flag signifying this is a test run",  OsServiceOptions::ConfigOption);
    service3.addOptionString("local-uri", ": URI to be used in the From header",  OsServiceOptions::ConfigOption, true, "is-test-run");

    //
    // parseOptions must not fail.  local-uri is required but alternate is present
    //
    CPPUNIT_ASSERT(service3.parseOptions());

    freeCArray(args.size(), &argv);


    OS_LOG_DEBUG(FAC_KERNEL, "testServiceOptions ENDED");
  }

  void testServiceOptionsConfigOnly()
  {
    const char* configFile = "OsServiceOptions.data.1";
    const rlim_t bigInt = 18446744073709551615;
    remove(configFile);
    {
      std::ofstream config(configFile);
      config << "paramString = " << "string" << std::endl;
      config << "paramInt = " << 1234 << std::endl;
      config << "paramBigInt = " << bigInt << std::endl;
      config << "paramBool = " << "true" << std::endl;
    }

    OsServiceOptions options(configFile);
    CPPUNIT_ASSERT(options.parseOptions());

    std::string paramString;
    int paramInt = 0;
    rlim_t paramBigInt = 0;
    bool paramBool = false;

    CPPUNIT_ASSERT(options.getOption("paramString", paramString));
    CPPUNIT_ASSERT(options.getOption("paramInt", paramInt));
    CPPUNIT_ASSERT(options.getOption<rlim_t>("paramBigInt", paramBigInt));
    CPPUNIT_ASSERT(options.getOption("paramBool", paramBool));

    CPPUNIT_ASSERT(paramString == "string");
    CPPUNIT_ASSERT(paramInt == 1234);
    CPPUNIT_ASSERT(paramBigInt == bigInt);
    CPPUNIT_ASSERT(paramBool == true);
  }

  void testServiceOptionsVectors()
  {
    std::vector<std::string> args;
    args.push_back("ServiceOptions");
    args.push_back("--is-test-run");

    args.push_back("-s");
    args.push_back("s_item_1");

    args.push_back("--stringItem");
    args.push_back("s_item_2");

    args.push_back("--stringItem");
    args.push_back("s_item_3");

    args.push_back("-i");
    args.push_back("1");

    args.push_back("--intItem");
    args.push_back("2");

    args.push_back("--intItem");
    args.push_back("3");

    char** argv = 0;
    vectorToCArray(args, &argv);
    OsServiceOptions service(args.size(), argv, "ServiceOptions", "1.0", "Ezuce Inc. All Rights Reserved");
    service._unitTestMode = true;

    

    service.addOptionFlag("is-test-run", ": Flag signifying this is a test run",  OsServiceOptions::ConfigOption);
    service.addOptionStringVector('s', "stringItem", ": String item",  OsServiceOptions::ConfigOption);
    service.addOptionIntVector('i', "intItem", ": Int item",  OsServiceOptions::ConfigOption);
    
    CPPUNIT_ASSERT(service.parseOptions());

    std::vector<std::string> strings;
    std::vector<int> ints;

    CPPUNIT_ASSERT(service.getOption("stringItem", strings));
    CPPUNIT_ASSERT(service.getOption("intItem", ints));

    CPPUNIT_ASSERT(strings.size() == 3);
    CPPUNIT_ASSERT(ints.size() == 3);

    CPPUNIT_ASSERT(strings[0] == "s_item_1");
    CPPUNIT_ASSERT(strings[1] == "s_item_2");
    CPPUNIT_ASSERT(strings[2] == "s_item_3");

    CPPUNIT_ASSERT(ints[0] == 1);
    CPPUNIT_ASSERT(ints[1] == 2);
    CPPUNIT_ASSERT(ints[2] == 3);

    freeCArray(args.size(), &argv);
    args.clear();
  }

  void testServiceOptions_parseOptionsFlags()
  {
    char** pArgv = 0;
    int argc = 0;

    OsServiceOptions osServiceOptions(argc, pArgv, "sipXdummyTool", "1.0", "Ezuce Inc. All Rights Reserved");
    osServiceOptions.parseOptions(OsServiceOptions::DisplayExceptionFlag);

    std::ostringstream stream;
    osServiceOptions.displayUsage(stream);


    /* check that no other options are present in help */
    CPPUNIT_ASSERT(std::string("\nsipXdummyTool version 1.0 - Ezuce Inc. All Rights Reserved\n"
                  "\nsipXdummyTool Options:\n\n") == stream.str());
  }

<<<<<<< HEAD
=======
  void testServiceOptions_ConfigFileTypeConfigDb()
  {
    OsServiceOptions osServiceOptions;
    osServiceOptions.loadConfigDbFromFile("/usr/local/sipxecs-branch-4.6/etc/sipxpbx/sipxproxy/sipXproxy-config.vm");

    std::string paramString;

    CPPUNIT_ASSERT(osServiceOptions.getOption("SIPX_PROXY_HOOK_LIBRARY.350_calleralertinfo", paramString));
    CPPUNIT_ASSERT(paramString == "/usr/local/sipxecs-branch-4.6/lib/authplugins/libCallerAlertInfo.so");
  }
>>>>>>> XX-8299 Make log level change dynamic
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsServiceOptionsTest);

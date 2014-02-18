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

#include <sys/resource.h>

#include <boost/lexical_cast.hpp>

typedef struct
{
  const char* pOptionName;
  const char* pOptionValue;
} ConfigurationFileTestData;

ConfigurationFileTestData configurationFileTestData[] =
{
  {"SIPX_PROXY_BIND_IP", "$!{location.address}"},
  {"SIPX_PROXY_DEFAULT_EXPIRES", "$settings.getSetting('SIPX_PROXY_DEFAULT_EXPIRES').Value"},
  {"SIPX_PROXY_DEFAULT_SERIAL_EXPIRES", "$settings.getSetting('SIPX_PROXY_DEFAULT_SERIAL_EXPIRES').Value"},
  {"SIPX_PROXY_HOST_NAME", "$!{location.fqdn}"},
  {"SIPX_PROXY_HOST_ALIASES", "$!{location.address}:${proxyService.sipPort} $!{location.fqdn}:${proxyService.sipPort}"},
  //{"SIPX_PROXY_LOG_CONSOLE", ""},
  //{"SIPX_PROXY_LOG_DIR", ""},
  {"SIPX_PROXY_LOG_LEVEL", "$settings.getSetting('SIPX_PROXY_LOG_LEVEL').Value"},
  //{"SIPX_PROXY_MAX_FORWARDS", ""},
  //{"SIPX_PROXY_STALE_TCP_TIMEOUT", ""},
  {"SIPX_PROXY_TLS_PORT", "$!{proxyService.secureSipPort}"},
  {"SIPX_PROXY_TCP_PORT", "$!{proxyService.sipTCPPort}"},
  {"SIPX_PROXY_UDP_PORT", "$!{proxyService.sipUDPPort}"},
  //{"SIPX_PROXY_CALL_STATE", ""},
  //{"SIPX_PROXY_CALL_STATE_LOG", ""},
  {"SIPX_PROXY_CALL_STATE_DB", "$callResolverSettings.getSetting('CALLRESOLVER_CALL_STATE_DB').Value"},
  //{"SIPX_PROXY_AUTHENTICATE_ALGORITHM", ""},
  {"SIPX_PROXY_AUTHENTICATE_REALM", "${proxyService.realm}"},
  {"SIPX_PROXY_HOSTPORT", "${location.address}:${proxyService.sipPort}"},
  {"SIPX_SEND_TRYING_FOR_NIST", "$settings.getSetting('SIPX_SEND_TRYING_FOR_NIST').Value"},
  {"SIPX_PROXY_HOOK_LIBRARY.200_xfer", "/usr/local/sipxecs-branch-4.6/lib/authplugins/libTransferControl.so"},
  {"SIPX_PROXY_HOOK_LIBRARY.205_subscriptionauth", "/usr/local/sipxecs-branch-4.6/lib/authplugins/libSubscriptionAuth.so"},
  {"SIPX_PROXY.205_subscriptionauth.PACKAGES_REQUIRING_AUTHENTICATION", "$!{settings.getSetting('SIPX_PROXY_DIALOG_SUBSCRIBE_AUTHENTICATION').Value}"},
  {"SIPX_PROXY.205_subscriptionauth.TARGETS_EXEMPTED_FROM_AUTHENTICATION", "^~~rl~"},
  {"SIPX_PROXY_HOOK_LIBRARY.210_msftxchghack", "/usr/local/sipxecs-branch-4.6/lib/authplugins/libMSFT_ExchangeTransferHack.so"},
  {"SIPX_PROXY.210_msftxchghack.USERAGENT", "^RTCC/"},
  {"SIPX_PROXY_HOOK_LIBRARY.300_calldestination", "/usr/local/sipxecs-branch-4.6/lib/authplugins/libCallDestination.so"},
  {"SIPX_PROXY_HOOK_LIBRARY.350_calleralertinfo", "/usr/local/sipxecs-branch-4.6/lib/authplugins/libCallerAlertInfo.so"},
  {"SIPX_PROXY.350_calleralertinfo.EXTERNAL", "$!{settings.getSetting('alert-info/EXTERNAL').Value}"},
  {"SSIPX_PROXY.350_calleralertinfo.EXTERNAL_ENABLED", "$!{settings.getSetting('alert-info/EXTERNAL_ENABLED').Value}"},
  {"SIPX_PROXY.350_calleralertinfo.INTERNAL", "$!{settings.getSetting('alert-info/INTERNAL').Value}"},
  {"SIPX_PROXY.350_calleralertinfo.INTERNAL_ENABLED", "$!{settings.getSetting('alert-info/INTERNAL_ENABLED').Value}"},
  {"SIPX_PROXY.350_calleralertinfo.ON_EXISTING", "$!{settings.getSetting('alert-info/ON_EXISTING').Value}"},
  {"SIPX_PROXY_HOOK_LIBRARY.400_authrules", "/usr/local/sipxecs-branch-4.6/lib/authplugins/libEnforceAuthRules.so"},
  {"SIPX_PROXY.400_authrules.RULES", "/usr/local/sipxecs-branch-4.6/etc/sipxpbx/authrules.xml"},
  {"SIPX_PROXY.400_authrules.IDENTITY_VALIDITY_SECONDS", "300"},
  {"SIPX_PROXY_HOOK_LIBRARY.700_fromalias", "/usr/local/sipxecs-branch-4.6/lib/authplugins/libCallerAlias.so"},
  {"SIPX_PROXY_HOOK_LIBRARY.900_ntap", "/usr/local/sipxecs-branch-4.6/lib/authplugins/libNatTraversalAgent.so"},
  {"SIPX_PROXY_HOOK_LIBRARY.990_emergnotif", "/usr/local/sipxecs-branch-4.6/lib/authplugins/libEmergencyNotify.so"},
  {"SIPX_PROXY.990_emergnotif.EMERGRULES", "/usr/local/sipxecs-branch-4.6/etc/sipxpbx/authrules.xml"},
  {"SIPX_PROXY_HOOK_LIBRARY.995_requestlinter", "/usr/local/sipxecs-branch-4.6/lib/authplugins/libRequestLinter.so"},
  {"SIPX_PROXY_AUTOBAN_THRESHOLD_VIOLATORS", "$!{proxyService.autobanThresholdViolators}"},
  {"SIPX_PROXY_PACKETS_PER_SECOND_THRESHOLD", "$!{proxyService.allowedPacketsPerSecond}"},
  {"SIPX_PROXY_THRESHOLD_VIOLATION_RATE", "$!{proxyService.thresholdViolationRate}"},
  {"SIPX_PROXY_BAN_LIFETIME", "$!{proxyService.banLifetime}"},
  {"SIPX_PROXY_WHITE_LIST", "$!{proxyService.whiteList}"},
  {"SIPX_PROXY_BLACK_LIST", "$!{proxyService.blackList}"},
  {"SIPX_TRAN_HOOK_LIBRARY.100_sipxhomer", "@sipxpbx.lib.dir@/transactionplugins/libsipXhomerProxyPlugin.so"},
};

ConfigurationFileTestData configurationFileOnlyTestData[] =
{
  {"paramString", "string"},
  {"paramUtlString", "utlString"},
  {"paramInt", "1234"},
  {"paramBigInt", "18446744073709551615"},
  {"paramBool_true", "true"},
  {"paramBool_1", "1"},
  {"paramBool_T", "T"},
  {"paramBool_t", "t"}
};

typedef struct
{
  const char shortForm;
  const char* pOptionName;
  const char* pDescription;
  const char* pOptionValue;
  OsServiceOptions::OptionType type;
} OptionTestData;

OptionTestData optionTestData[] =
{
  {0, "is-test-run", ": Flag signifying this is a test run", "", OsServiceOptions::ConfigOption},
  {0, "local-uri", ": URI to be used in the From header", "sip:test@ezuce.com", OsServiceOptions::ConfigOption},
  {0, "resource-uri", ": URI of the resource", "sip:resource@ezuce.com", OsServiceOptions::ConfigOption},
  {0, "proxy-uri", ": URI of the proxy that will handle the subscription", "sip:openuc.ezuce.com", OsServiceOptions::ConfigOption},
  {0, "sip-port", "Port to be used by the SIP transport", "5060", OsServiceOptions::ConfigOption},
  {0, "refresh-interval", "The refresh intervals for subscriptions. (seconds)", "1800", OsServiceOptions::ConfigOption},
  {'L', "log-file", ": Specify the application log file.", "OsServiceOptionsTest", OsServiceOptions::CommandLineOption},
  {'l', "log-level",
   ": Specify the application log priority level. Valid level is between 0-7. 0 (EMERG) 1 (ALERT) 2 (CRIT) 3 (ERR) 4 (WARNING) 5 (NOTICE) 6 (INFO) 7 (DEBUG)",
   "7", OsServiceOptions::CommandLineOption}
};

typedef struct
{
  const char shortForm;
  const char* pOptionName;
  const char* pDescription;
  const char* pOptionValue1;
  const char* pOptionValue2;
  const char* pOptionValue3;
  OsServiceOptions::OptionType type;
} OptionVectorTestData;

OptionVectorTestData optionVectorTestData[] =
{
  {'s', "stringItem", ": String item", "s_item_1", "s_item_2", "s_item_3", OsServiceOptions::ConfigOption},
  {'i', "intItem", ": Int item", "1", "2", "3", OsServiceOptions::ConfigOption}
};

#define CONFIGURATION_FILE_FORMAT_CONFIGDB "OsServiceOptions_data_format_configdb"
#define CONFIGURATION_FILE_FORMAT_INI      "OsServiceOptions_data_format_ini"
#define CONFIGURATION_FILE_FORMAT_VECTORS_INI "OsServiceOptions_data_format_vectors_ini"
#define CONFIGURATION_FILE "OsServiceOptions.data.1"
#define APP_NAME "ServiceOptions"

class OsServiceOptionsTest : public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE(OsServiceOptionsTest);

  CPPUNIT_TEST(testServiceOptions);
  CPPUNIT_TEST(testServiceOptionsConfigOnly);
  CPPUNIT_TEST(testServiceOptions_CommandLineVectors);
  CPPUNIT_TEST(testServiceOptions_ConfigFileVectors);
  CPPUNIT_TEST(testServiceOptions_ConfigFileFormatConfigDb);
  CPPUNIT_TEST(testServiceOptions_ConfigFileFormatIni);
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

    args.push_back(APP_NAME);

    //
    // This is a flag.  No value needs to be set
    //
    args.push_back(std::string("--") + optionTestData[0].pOptionName);

    args.push_back(std::string("--") + optionTestData[1].pOptionName);
    args.push_back(optionTestData[1].pOptionValue);

    args.push_back(std::string("--") + optionTestData[2].pOptionName);
    args.push_back(optionTestData[2].pOptionValue);

    args.push_back(std::string("--") + optionTestData[3].pOptionName);
    args.push_back(optionTestData[3].pOptionValue);

    args.push_back(std::string("--") + optionTestData[4].pOptionName);
    args.push_back(optionTestData[4].pOptionValue);

    args.push_back(std::string("--") + optionTestData[5].pOptionName);
    args.push_back(optionTestData[5].pOptionValue);

    args.push_back(std::string("--") + optionTestData[6].pOptionName);
    args.push_back(optionTestData[6].pOptionValue);

    args.push_back(std::string("--") + optionTestData[7].pOptionName);
    args.push_back(optionTestData[7].pOptionValue);


    char** argv = 0;
    vectorToCArray(args, &argv);
    OsServiceOptions service(args.size(), argv);

    service.addOptionFlag(optionTestData[0].shortForm, optionTestData[0].pOptionName, optionTestData[0].pDescription,  optionTestData[0].type);
    service.addOptionString(optionTestData[1].shortForm, optionTestData[1].pOptionName, optionTestData[1].pDescription,  optionTestData[1].type);
    service.addOptionString(optionTestData[2].shortForm, optionTestData[2].pOptionName, optionTestData[2].pDescription,  optionTestData[2].type);
    service.addOptionString(optionTestData[3].shortForm, optionTestData[3].pOptionName, optionTestData[3].pDescription,  optionTestData[3].type);
    service.addOptionInt(optionTestData[4].shortForm, optionTestData[4].pOptionName, optionTestData[4].pDescription,  optionTestData[4].type);
    service.addOptionInt(optionTestData[5].shortForm, optionTestData[5].pOptionName, optionTestData[5].pDescription,  optionTestData[5].type);
    service.addOptionString(optionTestData[6].shortForm, optionTestData[6].pOptionName, optionTestData[6].pDescription,  optionTestData[6].type);
    service.addOptionInt(optionTestData[7].shortForm, optionTestData[7].pOptionName, optionTestData[7].pDescription,  optionTestData[7].type);

    CPPUNIT_ASSERT(service.parseOptions());

    OS_LOG_DEBUG(FAC_KERNEL, "testServiceOptions STARTED");

    // TEST: Check read values
    CPPUNIT_ASSERT(service.hasOption(optionTestData[0].pOptionName));

    std::string value;
    CPPUNIT_ASSERT(service.getOption(optionTestData[1].pOptionName, value));
    CPPUNIT_ASSERT(optionTestData[1].pOptionValue == value);

    CPPUNIT_ASSERT(service.getOption(optionTestData[2].pOptionName, value));
    CPPUNIT_ASSERT(optionTestData[2].pOptionValue == value);

    CPPUNIT_ASSERT(service.getOption(optionTestData[3].pOptionName, value));
    CPPUNIT_ASSERT(optionTestData[3].pOptionValue == value);

    int val;
    CPPUNIT_ASSERT(service.getOption(optionTestData[4].pOptionName, val));
    CPPUNIT_ASSERT(boost::lexical_cast<int>(optionTestData[4].pOptionValue) == val);

    CPPUNIT_ASSERT(service.getOption(optionTestData[5].pOptionName, val));
    CPPUNIT_ASSERT(boost::lexical_cast<int>(optionTestData[5].pOptionValue) == val);

    freeCArray(args.size(), &argv);
    args.clear();

    //
    // Test required field
    //
    args.push_back(APP_NAME);
    args.push_back(std::string("--") + optionTestData[0].pOptionName);
    vectorToCArray(args, &argv);
    
    OsServiceOptions service2(args.size(), argv);
    service2.addOptionFlag(optionTestData[0].shortForm, optionTestData[0].pOptionName, optionTestData[0].pDescription,  optionTestData[0].type);
    service2.addOptionString(optionTestData[1].shortForm, optionTestData[1].pOptionName, optionTestData[1].pDescription,  optionTestData[1].type, true);

    //
    // parseOptions must fail.  local-uri is required
    //
    std::ostringstream stream;
    CPPUNIT_ASSERT(!service2.parseOptions(OsServiceOptions::DefaultOptionsFlag, stream));

    freeCArray(args.size(), &argv);


    args.clear();

    //
    // Test required field with alternate
    //
    args.push_back(APP_NAME);
    args.push_back(std::string("--") + optionTestData[0].pOptionName);
    vectorToCArray(args, &argv);

    OsServiceOptions service3(args.size(), argv);
    service3.addOptionFlag(optionTestData[0].shortForm, optionTestData[0].pOptionName, optionTestData[0].pDescription,  optionTestData[0].type);
    service3.addOptionString(optionTestData[1].shortForm, optionTestData[1].pOptionName, optionTestData[1].pDescription,  optionTestData[1].type, true, optionTestData[0].pOptionName);

    //
    // parseOptions must not fail.  local-uri is required but alternate is present
    //
    CPPUNIT_ASSERT(service3.parseOptions());

    freeCArray(args.size(), &argv);


    OS_LOG_DEBUG(FAC_KERNEL, "testServiceOptions ENDED");
  }

  void testServiceOptionsConfigOnly()
  {
    const char* configFile = CONFIGURATION_FILE;
    remove(configFile);
    {
      std::ofstream config(configFile);
      config << configurationFileOnlyTestData[0].pOptionName << " = " << configurationFileOnlyTestData[0].pOptionValue << std::endl;
      config << configurationFileOnlyTestData[1].pOptionName << " = " << configurationFileOnlyTestData[1].pOptionValue << std::endl;
      config << configurationFileOnlyTestData[2].pOptionName << " = " <<
          boost::lexical_cast<int>(configurationFileOnlyTestData[2].pOptionValue) << std::endl;
      config << configurationFileOnlyTestData[3].pOptionName << " = " <<
          boost::lexical_cast<rlim_t>(configurationFileOnlyTestData[3].pOptionValue) << std::endl;
      config << configurationFileOnlyTestData[4].pOptionName << " = " << configurationFileOnlyTestData[4].pOptionValue << std::endl;
      config << configurationFileOnlyTestData[5].pOptionName << " = " << configurationFileOnlyTestData[5].pOptionValue << std::endl;
      config << configurationFileOnlyTestData[6].pOptionName << " = " << configurationFileOnlyTestData[6].pOptionValue << std::endl;
      config << configurationFileOnlyTestData[7].pOptionName << " = " << configurationFileOnlyTestData[7].pOptionValue << std::endl;
    }

    OsServiceOptions options(configFile);
    CPPUNIT_ASSERT(options.parseOptions());

    std::string paramString;
    UtlString paramUtlString;
    int paramInt = 0;
    rlim_t paramBigInt = 0;
    bool paramBool = false;

    // TEST: Check read values
    CPPUNIT_ASSERT(options.getOption(configurationFileOnlyTestData[0].pOptionName, paramString));
    CPPUNIT_ASSERT(paramString == configurationFileOnlyTestData[0].pOptionValue);

    CPPUNIT_ASSERT(options.getOption(configurationFileOnlyTestData[1].pOptionName, paramString));
    CPPUNIT_ASSERT(paramString == configurationFileOnlyTestData[1].pOptionValue);

    CPPUNIT_ASSERT(options.getOption(configurationFileOnlyTestData[2].pOptionName, paramInt));
    CPPUNIT_ASSERT(paramInt == boost::lexical_cast<int>(configurationFileOnlyTestData[2].pOptionValue));

    CPPUNIT_ASSERT(options.getOption<rlim_t>(configurationFileOnlyTestData[3].pOptionName, paramBigInt));
    CPPUNIT_ASSERT(paramBigInt == boost::lexical_cast<rlim_t>(configurationFileOnlyTestData[3].pOptionValue));

    CPPUNIT_ASSERT(options.getOption(configurationFileOnlyTestData[4].pOptionName, paramBool));
    CPPUNIT_ASSERT(paramBool == true);

    CPPUNIT_ASSERT(options.getOption(configurationFileOnlyTestData[5].pOptionName, paramBool));
    CPPUNIT_ASSERT(paramBool == true);

    CPPUNIT_ASSERT(options.getOption(configurationFileOnlyTestData[6].pOptionName, paramBool));
    CPPUNIT_ASSERT(paramBool == true);

    CPPUNIT_ASSERT(options.getOption(configurationFileOnlyTestData[7].pOptionName, paramBool));
    CPPUNIT_ASSERT(paramBool == true);
  }

  void checkVectorOptions(OsServiceOptions& osServiceOptions)
  {
    std::vector<std::string> strings;
    std::vector<int> ints;

    // TEST: Check read values
    CPPUNIT_ASSERT(osServiceOptions.getOption(optionVectorTestData[0].pOptionName, strings));
    CPPUNIT_ASSERT(osServiceOptions.getOption(optionVectorTestData[1].pOptionName, ints));

    CPPUNIT_ASSERT(strings.size() == 3);
    CPPUNIT_ASSERT(ints.size() == 3);

    CPPUNIT_ASSERT(strings[0] == optionVectorTestData[0].pOptionValue1);
    CPPUNIT_ASSERT(strings[1] == optionVectorTestData[0].pOptionValue2);
    CPPUNIT_ASSERT(strings[2] == optionVectorTestData[0].pOptionValue3);

    CPPUNIT_ASSERT(ints[0] == boost::lexical_cast<int>(optionVectorTestData[1].pOptionValue1));
    CPPUNIT_ASSERT(ints[1] == boost::lexical_cast<int>(optionVectorTestData[1].pOptionValue2));
    CPPUNIT_ASSERT(ints[2] == boost::lexical_cast<int>(optionVectorTestData[1].pOptionValue3));
  }

  void testServiceOptions_ConfigFileVectors()
  {
    OsServiceOptions osServiceOptions;

    const char* configFile = CONFIGURATION_FILE_FORMAT_VECTORS_INI;
    remove(configFile);

    std::ofstream config(configFile);
    for (size_t row = 0; row < sizeof(optionVectorTestData)/sizeof(OptionVectorTestData); row++)
    {
      config << optionVectorTestData[row].pOptionName << "=" << optionVectorTestData[row].pOptionValue1 << std::endl;
      config << optionVectorTestData[row].pOptionName << "=" << optionVectorTestData[row].pOptionValue2 << std::endl;
      config << optionVectorTestData[row].pOptionName << "=" << optionVectorTestData[row].pOptionValue3 << std::endl;
    }

    osServiceOptions.setConfigurationFile(configFile);

    osServiceOptions.addOptionFlag(optionTestData[0].shortForm, optionTestData[0].pOptionName, optionTestData[0].pDescription,  optionTestData[0].type);
    osServiceOptions.addOptionStringVector(optionVectorTestData[0].shortForm, optionVectorTestData[0].pOptionName, optionVectorTestData[0].pDescription,  optionVectorTestData[0].type);
    osServiceOptions.addOptionIntVector(optionVectorTestData[1].shortForm, optionVectorTestData[1].pOptionName, optionVectorTestData[1].pDescription,  optionVectorTestData[1].type);

    // TEST: Check that parseOptions returns success
    CPPUNIT_ASSERT(osServiceOptions.parseOptions());

    checkVectorOptions(osServiceOptions);
  }

  void testServiceOptions_CommandLineVectors()
  {
    std::vector<std::string> args;
    args.push_back(APP_NAME);
    args.push_back(std::string("--") + optionTestData[0].pOptionName);

    args.push_back(std::string("-") + optionVectorTestData[0].shortForm);
    args.push_back(optionVectorTestData[0].pOptionValue1);

    args.push_back(std::string("--") + optionVectorTestData[0].pOptionName);
    args.push_back(optionVectorTestData[0].pOptionValue2);

    args.push_back(std::string("--") + optionVectorTestData[0].pOptionName);
    args.push_back(optionVectorTestData[0].pOptionValue3);

    args.push_back(std::string("-") + optionVectorTestData[1].shortForm);
    args.push_back(optionVectorTestData[1].pOptionValue1);

    args.push_back(std::string("--") + optionVectorTestData[1].pOptionName);
    args.push_back(optionVectorTestData[1].pOptionValue2);

    args.push_back(std::string("--") + optionVectorTestData[1].pOptionName);
    args.push_back(optionVectorTestData[1].pOptionValue3);

    char** argv = 0;
    vectorToCArray(args, &argv);
    OsServiceOptions osServiceOptions(args.size(), argv);

    osServiceOptions.addOptionFlag(optionTestData[0].shortForm, optionTestData[0].pOptionName, optionTestData[0].pDescription,  optionTestData[0].type);
    osServiceOptions.addOptionStringVector(optionVectorTestData[0].shortForm, optionVectorTestData[0].pOptionName, optionVectorTestData[0].pDescription,  optionVectorTestData[0].type);
    osServiceOptions.addOptionIntVector(optionVectorTestData[1].shortForm, optionVectorTestData[1].pOptionName, optionVectorTestData[1].pDescription,  optionVectorTestData[1].type);

    // TEST: Check that parseOptions returns success
    CPPUNIT_ASSERT(osServiceOptions.parseOptions());

    checkVectorOptions(osServiceOptions);

    freeCArray(args.size(), &argv);
    args.clear();
  }

  void CheckOptions(OsServiceOptions& osServiceOptions)
  {
    for (size_t row = 0; row < sizeof(configurationFileTestData)/sizeof(ConfigurationFileTestData); row++)
    {
      std::string paramString;

      // TEST: Check check that getOption returns with no error
      CPPUNIT_ASSERT(osServiceOptions.getOption(configurationFileTestData[row].pOptionName, paramString));
      // TEST: Check that the parameter is read correctly
      CPPUNIT_ASSERT(paramString == configurationFileTestData[row].pOptionValue);
    }

    for (size_t row = 0; row < sizeof(configurationFileTestData)/sizeof(ConfigurationFileTestData); row++)
    {
      UtlString paramUtlString;

      // TEST: Check check that getOption returns with no error
      CPPUNIT_ASSERT(osServiceOptions.getOption(configurationFileTestData[row].pOptionName, paramUtlString));
      // TEST: Check that the parameter is read correctly
      CPPUNIT_ASSERT(paramUtlString == configurationFileTestData[row].pOptionValue);
    }
  }

  void testServiceOptions_ConfigFileFormatConfigDb()
  {
    OsServiceOptions osServiceOptions;

    const char* configFile = CONFIGURATION_FILE_FORMAT_CONFIGDB;
    remove(configFile);

    std::ofstream config(configFile);
    for (size_t row = 0; row < sizeof(configurationFileTestData)/sizeof(ConfigurationFileTestData); row++)
    {
      config << configurationFileTestData[row].pOptionName << " : " << configurationFileTestData[row].pOptionValue << std::endl;
    }

    osServiceOptions.setConfigurationFile(configFile);

    // TEST: Check check that parseOption returns with no error
    CPPUNIT_ASSERT(osServiceOptions.parseOptions(OsServiceOptions::ParseConfigDbFlag));

    // TEST: Verify read options
    CheckOptions(osServiceOptions);
  }

  void testServiceOptions_ConfigFileFormatIni()
  {
    OsServiceOptions osServiceOptions;

    const char* configFile = CONFIGURATION_FILE_FORMAT_INI;
    remove(configFile);

    std::ofstream config(configFile);
    for (size_t row = 0; row < sizeof(configurationFileTestData)/sizeof(ConfigurationFileTestData); row++)
    {
      config << configurationFileTestData[row].pOptionName << "=" << configurationFileTestData[row].pOptionValue << std::endl;
    }

    osServiceOptions.setConfigurationFile(configFile);

    // TEST: Check check that parseOption returns with no error
    CPPUNIT_ASSERT(osServiceOptions.parseOptions());

    // TEST: Verify read options
    CheckOptions(osServiceOptions);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsServiceOptionsTest);

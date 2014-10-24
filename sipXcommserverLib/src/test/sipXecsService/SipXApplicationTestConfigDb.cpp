#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipdb/MongoDB.h>
#include <mongo/client/dbclient.h>
#include <mongo/client/connpool.h>

#include "sipXecsService/SipXApplication.h"
#include "os/OsServiceOptions.h"

#include <boost/format.hpp>

using namespace std;


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
  {"SIPX_PROXY_LOG_CONSOLE", ""},
  {"SIPX_PROXY_LOG_DIR", ""},
  {"SIPX_PROXY_LOG_LEVEL", "$settings.getSetting('SIPX_PROXY_LOG_LEVEL').Value"},
  {"SIPX_PROXY_MAX_FORWARDS", ""},
  {"SIPX_PROXY_STALE_TCP_TIMEOUT", ""},
  {"SIPX_PROXY_TLS_PORT", "$!{proxyService.secureSipPort}"},
  {"SIPX_PROXY_TCP_PORT", "$!{proxyService.sipTCPPort}"},
  {"SIPX_PROXY_UDP_PORT", "$!{proxyService.sipUDPPort}"},
  {"SIPX_PROXY_CALL_STATE", ""},
  {"SIPX_PROXY_CALL_STATE_LOG", ""},
  {"SIPX_PROXY_CALL_STATE_DB", "$callResolverSettings.getSetting('CALLRESOLVER_CALL_STATE_DB').Value"},
  {"SIPX_PROXY_AUTHENTICATE_ALGORITHM", ""},
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

class SipXApplicationTestConfigDb: public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE(SipXApplicationTestConfigDb);
  CPPUNIT_TEST(testParse);
  CPPUNIT_TEST_SUITE_END();



public:

  SipXApplicationTestConfigDb()
  {
  }

  void createConfigurationFilePath(const std::string& configFilename, std::string& configPath)
  {
    OsPath workingDirectory;
    UtlString fileName;
    if (OsFileSystem::exists(SIPX_CONFDIR))
    {
      workingDirectory = SIPX_CONFDIR;
      OsPath path(workingDirectory);
      path.getNativePath(workingDirectory);
    }
    else
    {
      OsPath path;
      OsFileSystem::getWorkingDirectory(path);
      path.getNativePath(workingDirectory);
    }

     fileName = workingDirectory + OsPathBase::separator + configFilename.c_str();

     configPath = fileName.data();
  }

  void createConfigFile(const std::string& configFilename)
  {
    std::string configPath;

    createConfigurationFilePath(configFilename, configPath);
    remove(configPath.c_str());
    {
      std::ofstream config(configPath.c_str());

      for (size_t row = 0; row < sizeof(configurationFileTestData)/sizeof(ConfigurationFileTestData); row++)
      {
        config << configurationFileTestData[row].pOptionName << " : " << configurationFileTestData[row].pOptionValue << std::endl;
      }
    }
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

  void testParse()
  {
    SipXApplicationData appData =
    {
      "sipxapp",
      "sipxapp-config",
      "sipxapp.log",
      "",
      "SIPX_PREFIX",
      false, // do not check mongo connection
      false, // disable mongo logs
      false, // do not increase application file descriptor limits
      true, // block signals on main thread (and all other threads created by main)
            // and process them only on a dedicated thread
      SipXApplicationData::ConfigFileFormatConfigDb, // format type for configuration file
      OsMsgQShared::QUEUE_UNLIMITED
    };

    char** argv = NULL;
    int argc = 0;

    SipXApplication& sipXApplication = SipXApplication::instance();
    OsServiceOptions& osServiceOptions = sipXApplication.getConfig();

    createConfigFile(appData._configFilename);

    sipXApplication.init(argc, argv, appData);

    // TEST: Verify read options
    CheckOptions(osServiceOptions);

  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SipXApplicationTestConfigDb);

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
  {0, "proxy-uri", ": URI of the proxy that will handle the subscription", "sip:openuc.ezuce.com", OsServiceOptions::ConfigOption}
};

typedef struct
{
  const char* pData;
} ResourceLimitsData;

ResourceLimitsData resourceLimitsData[] =
{
    {"sipxproxy-fd-soft=32768"},
    {"sipxproxy-fd-hard=65536"},
    {"sipxproxy-core-enabled=false"},
    {"sipxpublisher-fd-soft=32768"},
    {"sipxpublisher-fd-hard=65536"},
    {"sipxpublisher-core-enabled=false"},
    {"sipxregistry-fd-soft=32768"},
    {"sipxregistry-fd-hard=65536"},
    {"sipxregistry-core-enabled=false"},
    {"sipxsaa-fd-soft=32768"},
    {"sipxsaa-fd-hard=65536"},
    {"sipxsaa-core-enabled=false"},
    {"sipxrls-fd-soft=32768"},
    {"sipxrls-fd-hard=65536"},
    {"sipxrls-core-enabled=false"},
    {"sipxpark-fd-soft=32768"},
    {"sipxpark-fd-hard=65536"},
    {"sipxpark-core-enabled=false"},
    {"sipxapp-fd-soft=32768"},
    {"sipxapp-fd-hard=65536"},
    {"sipxapp-core-enabled=false"},

};

#define RESOURCE_LIMITS_CONF_NAME "resource-limits.ini"

class SipXApplicationTest: public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE(SipXApplicationTest);
  CPPUNIT_TEST(testDisplayUsageAndVersion);
  CPPUNIT_TEST(testMongoConnection);
  //CPPUNIT_TEST(testIncreaseResourceLimit);
  CPPUNIT_TEST_SUITE_END();



public:

  SipXApplicationTest()
  {
  }

  void createResourceLimitsConfigFile(const std::string& resourceConfPath)
  {
    remove(resourceConfPath.c_str());
    {
      std::ofstream config(resourceConfPath.c_str());

      for (size_t row = 0; row < sizeof(resourceLimitsData)/sizeof(ResourceLimitsData); row++)
      {
        config << resourceLimitsData[row].pData << std::endl;
      }
    }
  }

  static void terminateMongoConnection()
  {
    SipXApplication& sipXApplication = SipXApplication::instance();

    sipXApplication.terminate();
  }

  void testMongoConnection()
  {
    SipXApplication& sipXApplication = SipXApplication::instance();

    // TEST: Check that connection to mongo is successful
    CPPUNIT_ASSERT(sipXApplication.testMongoDBConnection());

    atexit(&SipXApplicationTest::terminateMongoConnection);
  }

  // WARNING: MUST BE RUN AS ROOT
  void testIncreaseResourceLimit()
  {
    SipXApplication& sipXApplication = SipXApplication::instance();

    std::string resourceLimitsConfPath = RESOURCE_LIMITS_CONF_NAME;

    createResourceLimitsConfigFile(resourceLimitsConfPath);

    // TEST: Check that increaseResourceLimits function returns success
    CPPUNIT_ASSERT(sipXApplication.increaseResourceLimits(resourceLimitsConfPath));
  }

  void testDisplayUsageAndVersion()
  {
    SipXApplicationData appData =
    {
      "sipxapp",
      "",
      "sipxapp.log",
      "",
      "SIPX_PREFIX",
      false, // do not check mongo connection
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

    for (size_t row = 0; row < sizeof(optionTestData)/sizeof(OptionTestData); row++)
    {
      osServiceOptions.addOptionString(optionTestData[row].shortForm,
                                        optionTestData[row].pOptionName,
                                        optionTestData[row].pDescription,
                                        optionTestData[row].type);
    }

    sipXApplication.init(argc, argv, appData);

    std::ostringstream stream;
    sipXApplication.displayUsage(stream);

    std::string help = stream.str();

    for (size_t row = 0; row < sizeof(optionTestData)/sizeof(OptionTestData); row++)
    {
      // TEST: Check that option names are present in help
      CPPUNIT_ASSERT(help.find(optionTestData[row].pOptionName) != std::string::npos);

      // TEST: Check that option descriptions are present in help
      CPPUNIT_ASSERT(help.find(optionTestData[row].pDescription) != std::string::npos);
    }

    stream.str("");// clear the stream
    sipXApplication.displayVersion(stream);

    std::string version = (boost::format("Version: %s%s\n") % PACKAGE_VERSION % PACKAGE_REVISION).str();

    // TEST: Check that the version string is correct
    CPPUNIT_ASSERT(stream.str() == version);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SipXApplicationTest);

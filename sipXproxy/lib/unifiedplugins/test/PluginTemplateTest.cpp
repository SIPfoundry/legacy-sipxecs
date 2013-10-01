#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>
#include <sipxproxy/UnifiedProxyPlugin.h>

static bool gIsLoaded = false;

class PluginTemplateTest : public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE(PluginTemplateTest);
  CPPUNIT_TEST(testLoader);
  CPPUNIT_TEST(testAuthenticators);
  CPPUNIT_TEST(testForwarder);
  CPPUNIT_TEST_SUITE_END();
public:
  UnifiedProxyPluginLoader _pluginLoader;
  int _loadedPluginCount;
  
  void setUp()
  {
    if (!gIsLoaded)
    {
      gIsLoaded = true;
      _loadedPluginCount = _pluginLoader.loadUnifiedProxyPlugins(0, 0);
    }
  }

  void tearDown()
  {
  }
  
  void testLoader()
  {
    CPPUNIT_ASSERT(_loadedPluginCount > 0);
    CPPUNIT_ASSERT(_pluginLoader.authenticators().size() > 0);
    CPPUNIT_ASSERT(_pluginLoader.ioProcessors().size() > 0);
    CPPUNIT_ASSERT(_pluginLoader.forwarders().size() > 0);
  }
  
  void testAuthenticators()
  {
    
  }
  
  void testForwarder()
  {
    
  }
  
  
};

CPPUNIT_TEST_SUITE_REGISTRATION(PluginTemplateTest);
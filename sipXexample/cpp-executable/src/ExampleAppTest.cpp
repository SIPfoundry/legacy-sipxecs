#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>
#include <ExampleApp.h>

class ExampleAppTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(ExampleAppTest);
    CPPUNIT_TEST(testHello);
    CPPUNIT_TEST_SUITE_END();

    ExampleApp* app;

public:
    void setup() {
        app = new ExampleApp();
    }

    void tearDown() {
        delete app;
    }

    void testHello() {
        CPPUNIT_ASSERT_EQUAL(0, app->hello());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ExampleAppTest);


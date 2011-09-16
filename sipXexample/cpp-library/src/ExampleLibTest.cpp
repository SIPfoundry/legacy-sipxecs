#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>
#include <sipxexample/ExampleLib.h>

class ExampleLibTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(ExampleLibTest);
    CPPUNIT_TEST(testHello);
    CPPUNIT_TEST_SUITE_END();

    ExampleLib* lib;

public:
    void setup() {
        lib = new ExampleLib();
    }

    void tearDown() {
        delete lib;
    }

    void testHello() {
        CPPUNIT_ASSERT_EQUAL(0, lib->hello());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ExampleLibTest);

//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

class TestTemplate : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(TestTemplate);
    CPPUNIT_TEST(testMethod);
    CPPUNIT_TEST_SUITE_END();


public:
    void testMethod()
    {
        // a few of the more handy assertions
        CPPUNIT_ASSERT(true || false);
        CPPUNIT_ASSERT_EQUAL(1, 3 - 2);
        CPPUNIT_ASSERT_MESSAGE("description", true || false);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("description", 1, 3 - 2);
        //CPPUNIT_FAIL("failed");
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestTemplate);

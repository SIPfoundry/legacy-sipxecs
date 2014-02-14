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
#include <sipxunit/TestUtilities.h>

class StateQueueConnectionTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(StateQueueConnectionTest);
    CPPUNIT_TEST(consumeTest);
    CPPUNIT_TEST_SUITE_END();


public:
    void consumeTest()
    {
        CPPUNIT_ASSERT(true);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(StateQueueConnectionTest);

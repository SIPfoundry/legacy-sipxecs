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

#include <os/OsTime.h>

/**
 * Unittest for OsTime
 */
class OsTimeTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(OsTimeTest);
    CPPUNIT_TEST(testSeconds);
    CPPUNIT_TEST(testCopyConstructor);
    CPPUNIT_TEST_SUITE_END();


public:
    void testSeconds()
    {
        const char *msg;
        OsTime *pInterval1;

        msg = "test the default constructor (if implemented)";
        pInterval1 = new OsTime();
        CPPUNIT_ASSERT_MESSAGE(msg, 0 == pInterval1->seconds());
        CPPUNIT_ASSERT_MESSAGE(msg, 0 == pInterval1->usecs());
        delete pInterval1;
                                                                                
        // test other constructors (if implemented)
        msg = "if a constructor parameter is used to set information in \
an ancestor class, then verify it gets set correctly (i.e., via ancestor \
class accessor method.";
        msg = "try giving the constructor a positive \"millisecond\" value";
        pInterval1 = new OsTime(3010);
        CPPUNIT_ASSERT_MESSAGE(msg, 3     == pInterval1->seconds());
        CPPUNIT_ASSERT_MESSAGE(msg, 10000 == pInterval1->usecs());
        delete pInterval1;
                                                                                
        msg = "try giving the constructor a negative \"millisecond\" value";
        pInterval1 = new OsTime(-3010);
        CPPUNIT_ASSERT_MESSAGE(msg, -3010  == pInterval1->cvtToMsecs());
        delete pInterval1;
                                                                                
        msg = "try giving the constructor a positive \"second\" and \"microsecond\" values";
        pInterval1 = new OsTime(30, 10);
        CPPUNIT_ASSERT_MESSAGE(msg, 30 == pInterval1->seconds());
        CPPUNIT_ASSERT_MESSAGE(msg, 10 == pInterval1->usecs());
        delete pInterval1;
                                                                                
        msg = "try giving the constructor a negative \"seconds\" value";
        pInterval1 = new OsTime(-20, 10);
        CPPUNIT_ASSERT_MESSAGE(msg, -20 == pInterval1->seconds());
        CPPUNIT_ASSERT_MESSAGE(msg, 10 == pInterval1->usecs());
        delete pInterval1;
                                                                                
        msg = "try giving the constructor a negative \"microseconds\" value";
        pInterval1 = new OsTime(20, -30);
        CPPUNIT_ASSERT_MESSAGE(msg, 19 == pInterval1->seconds());
        CPPUNIT_ASSERT_MESSAGE(msg, OsTime::USECS_PER_SEC - 30 == pInterval1->usecs());
        delete pInterval1;
                                                                                
        msg = "try giving the constructor a \"microseconds\" value greater \
than one sec";
        pInterval1 = new OsTime(20, 2000001);
        CPPUNIT_ASSERT_MESSAGE(msg, 22 == pInterval1->seconds());
        CPPUNIT_ASSERT_MESSAGE(msg, 1 == pInterval1->usecs());
        delete pInterval1;
                                                                                
        msg = "try giving the constructor a \"microseconds\" value less \
than one sec";
        pInterval1 = new OsTime(20, -2000001);
        CPPUNIT_ASSERT_MESSAGE(msg, 17 == pInterval1->seconds());
        CPPUNIT_ASSERT_MESSAGE(msg, OsTime::USECS_PER_SEC - 1 == pInterval1->usecs());
        delete pInterval1;
                                                                                
        msg = "try giving the constructor a \"microseconds\" value less \
than one sec";
        pInterval1 = new OsTime(20, -1999999);
        CPPUNIT_ASSERT_MESSAGE(msg, 18 == pInterval1->seconds());
        CPPUNIT_ASSERT_MESSAGE(msg, 1 == pInterval1->usecs());
        delete pInterval1;
    }
                                                                                
    void testCopyConstructor()
    {
        const char *msg;
        OsTime *pInterval1;
        OsTime *pInterval2;

        msg = "test the copy constructor applied to an object created \
using the default constructor";
        pInterval1 = new OsTime();
        pInterval2 = new OsTime(*pInterval1);
        CPPUNIT_ASSERT_MESSAGE(msg, 0 == pInterval1->seconds());
        CPPUNIT_ASSERT_MESSAGE(msg, 0 == pInterval1->usecs());

        CPPUNIT_ASSERT_MESSAGE(msg, pInterval2->seconds() == pInterval1->seconds());
        CPPUNIT_ASSERT_MESSAGE(msg, pInterval2->usecs() == pInterval1->usecs());
        delete pInterval1;
        delete pInterval2;
                                                                                
        msg = "test the copy constructor applied to an object create using a \
non-default constructor";
        pInterval1 = new OsTime(30, 10);
        pInterval2 = new OsTime(*pInterval1);
        CPPUNIT_ASSERT_MESSAGE(msg, 30 == pInterval1->seconds());
        CPPUNIT_ASSERT_MESSAGE(msg, 10 == pInterval1->usecs());
        CPPUNIT_ASSERT_MESSAGE(msg, pInterval2->seconds() == pInterval1->seconds());
        CPPUNIT_ASSERT_MESSAGE(msg, pInterval2->usecs() == pInterval1->usecs());
        delete pInterval1;
        delete pInterval2;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsTimeTest);


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

#include <os/OsBSem.h>
#include <os/OsCSem.h>

class OsSemTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(OsSemTest);
    CPPUNIT_TEST(testBasicSemaphore);
    CPPUNIT_TEST(testCountingSemaphore);
    CPPUNIT_TEST_SUITE_END();


public:
    void testBasicSemaphore()
    {
        OsBSem* pBSem;

        pBSem = new OsBSem(OsBSem::Q_PRIORITY, OsBSem::FULL);
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pBSem->acquire());
        CPPUNIT_ASSERT_EQUAL(OS_BUSY, pBSem->tryAcquire());
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pBSem->release());
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pBSem->acquire(100));
        CPPUNIT_ASSERT_EQUAL(OS_WAIT_TIMEOUT, pBSem->acquire(100));
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pBSem->release());

        delete pBSem;
    }

    void testCountingSemaphore()
    {
        OsCSem* pCSem;

        // the initial count on the semaphore will be 2
        pCSem = new OsCSem(OsCSem::Q_PRIORITY, 2);
                                                             // take it once
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pCSem->acquire(100));
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pCSem->acquire());  // take it twice
        CPPUNIT_ASSERT_EQUAL(OS_BUSY, pCSem->tryAcquire());  // try thrice
                                                             // try once more
        CPPUNIT_ASSERT_EQUAL(OS_WAIT_TIMEOUT, pCSem->acquire(100));
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pCSem->release());  // release once
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pCSem->release());  // release twice
        CPPUNIT_ASSERT_EQUAL(OS_BUSY, pCSem->release());     // release thrice

        delete pCSem;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsSemTest);

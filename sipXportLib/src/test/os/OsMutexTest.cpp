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

#include <os/OsRWMutex.h>
#include <os/OsMutex.h>

class OsMutexTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(OsMutexTest);
    CPPUNIT_TEST(testMutex);
    CPPUNIT_TEST(testRWMutex);
    CPPUNIT_TEST_SUITE_END();


public:
    void testMutex()
    {
        OsMutex* pMutex;

        pMutex = new OsMutex(OsMutex::Q_FIFO);
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMutex->acquire());
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMutex->acquire(100));
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMutex->tryAcquire());
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMutex->release());
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMutex->release());
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMutex->release());

        // TBD: test mutex locked by another thread!
        delete pMutex;
    }

    void testRWMutex()
    {
        OsRWMutex* pRWMutex;

        pRWMutex = new OsRWMutex(OsRWMutex::Q_FIFO);
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pRWMutex->acquireRead());
        CPPUNIT_ASSERT_EQUAL(OS_BUSY, pRWMutex->tryAcquireWrite());
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pRWMutex->releaseRead());
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pRWMutex->tryAcquireWrite());
        CPPUNIT_ASSERT_EQUAL(OS_BUSY, pRWMutex->tryAcquireRead());
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pRWMutex->releaseWrite());
        delete pRWMutex;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsMutexTest);

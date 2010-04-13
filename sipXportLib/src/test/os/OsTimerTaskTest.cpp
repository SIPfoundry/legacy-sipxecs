//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include <unistd.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <sipxunit/TestUtilities.h>
#include <os/OsTimerTask.h>
#include <os/OsTimer.h>
#include <os/OsTime.h>

class OsTimerTaskTest : public CppUnit::TestCase, public OsNotification
{
    CPPUNIT_TEST_SUITE(OsTimerTaskTest);
    CPPUNIT_TEST(testTimerTask);
    CPPUNIT_TEST(testStopAndDeleteRaceCondition);
    CPPUNIT_TEST_SUITE_END();

public:
    virtual OsStatus signal(intptr_t eventData)
    {
       printf("%p signaled\r\n", (void *)eventData );
       return OS_SUCCESS;
    }

    void testTimerTask()
    {
        OsTimerTask* pTimerTask;
        pTimerTask = OsTimerTask::getTimerTask();
        CPPUNIT_ASSERT_MESSAGE("Timer task created 1", pTimerTask != NULL);
        OsTask::delay(500);    // wait 1/2 second

        pTimerTask->destroyTimerTask();

        OsTask::delay(500);    // wait 1/2 second

        pTimerTask = OsTimerTask::getTimerTask();
        CPPUNIT_ASSERT_MESSAGE("Timer task created 2", pTimerTask != NULL);
        OsTask::delay(500);    // wait 1/2 second

        pTimerTask->destroyTimerTask();
    }

    void testStopAndDeleteRaceCondition()
    {
       // no explicit check is done.  If the race condition
       // exists, a segfault will ensue (XECS-2139)
       OsTimerTask* pTimerTask;
       OsTimer*     pTestTimer;
       OsTime       offset( 100000 );

       pTimerTask = OsTimerTask::getTimerTask();
       for( ssize_t index = 0; index < 2500; index ++ )
       {
          pTestTimer = new OsTimer( *this );
          pTestTimer->oneshotAfter( offset );
          usleep( rand() % 10000 );
          pTestTimer->stop(FALSE);
          delete pTestTimer;
       }
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsTimerTaskTest);

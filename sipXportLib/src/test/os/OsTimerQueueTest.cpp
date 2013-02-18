/*
 * Copyright (c) eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */


#include <sys/time.h>

#include <sipxunit/TestUtilities.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <os/OsTimerQueue.h>
#include <os/OsTask.h>
#include <os/OsLoggerHelper.h>

#include <time.h>
#include <string.h>

//Various OsTime durations in msec
OsTime msec1 = OsTime(1);
OsTime msec50 = OsTime(50);
OsTime msec100 = OsTime(100);
OsTime msec200 = OsTime(200);
OsTime msec300 = OsTime(300);
OsTime msec1000 = OsTime(1000);

// msgSubType for the OsMsgTest implementation of OsMsg
int OS_EVENT_TEST = (OsMsg::USER_START);


class OsTimerQueueTest : public CppUnit::TestCase
{
    // Implementation of a OsMsg for testing. This actually sends
    // a string which can be later retrieved from the queue and checked
    // as a validation that a timer fired.
    class OsMsgTest : public OsMsg
    {
       /* //////////////////////////// PUBLIC //////////////////////////////////// */
    public:

       /* ============================ CREATORS ================================== */

       OsMsgTest(const UtlString& str) :
           OsMsg(OS_EVENT, OS_EVENT_TEST),
           _str(str)
        {

        }

       virtual ~OsMsgTest() {}

       /* ============================ MANIPULATORS ============================== */

       virtual OsMsg* createCopy(void) const
        {
           return new OsMsgTest(_str);
        };
       //Create a copy of this msg object (which may be of a derived type)

       /* ============================ ACCESSORS ================================= */


       // Get pointer to the handle value.
       UtlString* getStr()
      {
         return &_str;
      }
       /* ============================ INQUIRY =================================== */

       /* //////////////////////////// PROTECTED ///////////////////////////////// */
    protected:

       /* //////////////////////////// PRIVATE /////////////////////////////////// */
    private:

       OsMsgTest(const OsMsgTest& rOsMsgTest);
       //Copy constructor (not implemented for this class)

       OsMsgTest& operator=(const OsMsgTest& rhs);
       //Assignment operator (not implemented for this class)

       UtlString _str;
    };



    CPPUNIT_TEST_SUITE(OsTimerQueueTest);

    //All unit tests are listed here.

#if 0
    // NOTE: This test is disabled as it will assert and crash because
    // passed queue pointer is NULL.
    CPPUNIT_TEST(testRegularConstructorWithInvalidParam);
#endif // #if 0

    CPPUNIT_TEST(testScheduleOneTimer);
    CPPUNIT_TEST(testScheduleManyTimers);
    CPPUNIT_TEST(testTimersOrderedExpirationOK);
    CPPUNIT_TEST(testStopRemovesNonExpiredTimers);
    CPPUNIT_TEST(testDestructorNoRemainingTimers);

    CPPUNIT_TEST_SUITE_END();

public:

    OsTimerQueueTest() : _msgSignalQueue("OsTimerQueueTest::_msgSignalQueue")
    {
        //only init the queue used in the test
    }

    void setUp()
    {
        //nothing to do
    }

    void tearDown()
    {
        //nothing to do
    }

    /// Verifies if a timer fired as expected
    void checkTimerFired(
            OsMsgQ& messageQueue, // queue where the message should have arrived
            OsTime& messageTimeout, // additional time to wait for the message
            const UtlString& expectedStr, // the expected contents of the message
            int expectedRet // the expected return code of this function
            )
    {
        OsMsg* recvMsg = NULL;
        OsStatus ret = OS_SUCCESS;

        ret = messageQueue.receive(recvMsg, messageTimeout);
        CPPUNIT_ASSERT(expectedRet == ret);

        if (OS_SUCCESS == ret)
        {
            CPPUNIT_ASSERT(recvMsg);
        }

        if (OS_SUCCESS == ret && recvMsg)
        {
            ret = OS_FAILED;

            int msgType = recvMsg->getMsgType();
            int msgSubType = recvMsg->getMsgSubType();
            if(msgType == OsMsg::OS_EVENT &&
                    msgSubType == OS_EVENT_TEST)
            {
                UtlString *recvStr = ((OsMsgTest*)recvMsg)->getStr();
                CPPUNIT_ASSERT(expectedStr == *recvStr);

                if (expectedStr == *recvStr)
                {
                    ret = OS_SUCCESS;
                }
            }

            // Msg is owned by the caller of the recv so it should be deleted
            delete recvMsg;
        }

        CPPUNIT_ASSERT(ret == expectedRet);
    }

    // Check status of a timer queue built with regular constructor
    void checkAfterRegularConstructor(const OsTimerQueue& tq, OsMsgQ *q)
    {
        // TEST: Signal queue was set as specified in constructor
        CPPUNIT_ASSERT(q == tq._signalQueue);

        // TEST: Upon creation internal queue should be empty
        CPPUNIT_ASSERT(0 == tq._timers.size());
    }

    // Check status of a timer queue after stop() called
    void checkAfterStop(const OsTimerQueue& tq)
    {
        // TEST: Internal queue should be empty after stop()
        CPPUNIT_ASSERT(0 == tq._timers.size());
    }

    // Check expected behavior of a queue built with regular constructor which
    // received invalid parameters
    void testRegularConstructorWithInvalidParam()
    {
        OsTimerQueue tq(NULL);
        checkAfterRegularConstructor(tq, NULL);

        OsMsgTest *msgFail = new OsMsgTest("fail");
        // TEST: Schedule should fail to start any timer
        CPPUNIT_ASSERT(OS_FAILED == tq.scheduleOneshotAfter(msgFail, msec100));
        // Failed schedule should not add anything
        CPPUNIT_ASSERT(0 == tq._timers.size());

        // stop() should not crash
        tq.stop();
        checkAfterStop(tq);
    }

    // Check expected behavior of schedule() method for one timer only
    void doTestScheduleOneTimer(OsTime& at, UtlString& strTest)
    {
        OsTimerQueue tq(&_msgSignalQueue);
        checkAfterRegularConstructor(tq, &_msgSignalQueue);

        OsMsgTest *msgTest = new OsMsgTest(strTest);
        // TEST: schedule should succeed in adding a timer
        CPPUNIT_ASSERT(OS_SUCCESS == tq.scheduleOneshotAfter(msgTest, at));
        CPPUNIT_ASSERT(1 == tq._timers.size());

        //give it some time to fire;
        OsTask::delay(at.cvtToMsecs());
        // TEST: timer should have fired
        checkTimerFired(_msgSignalQueue, msec50, strTest, OS_SUCCESS);

        //stop should remove the timer
        tq.stop();
        checkAfterStop(tq);
    }

    // Check expected behavior of schedule() method for one timer only
    void testScheduleOneTimer()
    {
        // TEST: schedule one timer to fire in 1 msec
        UtlString str1("Schedule msec1");
        doTestScheduleOneTimer(msec1, str1);

        // TEST: schedule one timer to fire in 50 msec
        UtlString str50("Schedule msec50");
        doTestScheduleOneTimer(msec50, str50);

        // TEST: schedule one timer to fire in 100 msec
        UtlString str100("Schedule msec100");
        doTestScheduleOneTimer(msec100, str100);

        // TEST: schedule one timer to fire in 1 sec
        UtlString str1000("Schedule msec1000");
        doTestScheduleOneTimer(msec1000, str1000);
    }

    // Helper to plain schedule of a timer
    void doScheduleTimer(OsTimerQueue& tq, OsTime& offset, const char* cstr)
    {
        int  tqSize = tq._timers.size();

        UtlString str(cstr);
        OsMsgTest *msgTest = new OsMsgTest(str);
        CPPUNIT_ASSERT(OS_SUCCESS == tq.scheduleOneshotAfter(msgTest, offset));
        CPPUNIT_ASSERT((tqSize+1) == tq._timers.size());
    }


    // Check expected behavior of schedule() method for more timers
    void testTimersOrderedExpirationOK()
    {
        OsTime startTime;

        OsTime at1;
        OsTime at2;
        OsTime at3;
        OsTime at4;
        OsTime at5;

        OsTimerQueue tq(&_msgSignalQueue);
        checkAfterRegularConstructor(tq, &_msgSignalQueue);

        OsDateTime::getCurTime(startTime);

        // TEST: Add several timers and expect them to be added ordered
        doScheduleTimer(tq, msec1000, "Schedule msec1000");
        doScheduleTimer(tq, msec100, "Schedule msec100");
        doScheduleTimer(tq, msec300, "Schedule msec300");
        doScheduleTimer(tq, msec200, "Schedule msec200");
        doScheduleTimer(tq, msec1000, "Schedule msec1000");

        // extract expiration date from the timers and remove all timers
        at1 = tq._timers.top()._at;
        tq._timers.top()._timer->stop(TRUE); tq._timers.pop();
        at2 = tq._timers.top()._at;
        tq._timers.top()._timer->stop(TRUE); tq._timers.pop();
        at3 = tq._timers.top()._at;
        tq._timers.top()._timer->stop(TRUE); tq._timers.pop();
        at4 = tq._timers.top()._at;
        tq._timers.top()._timer->stop(TRUE); tq._timers.pop();
        at5 = tq._timers.top()._at;
        tq._timers.top()._timer->stop(TRUE); tq._timers.pop();
        CPPUNIT_ASSERT(0 == tq._timers.size() > 0);

        // TEST:  Timers should be in the proper order
        CPPUNIT_ASSERT(at1 <= at2);
        CPPUNIT_ASSERT(at2 <= at3);
        CPPUNIT_ASSERT(at3 <= at4);
        CPPUNIT_ASSERT(at4 <= at5);

        // TEST: Timers expiration dates should be proper calculated as (now + offset)
        CPPUNIT_ASSERT((msec100 + startTime) <= at1);
        CPPUNIT_ASSERT((msec200 + startTime) <= at2);
        CPPUNIT_ASSERT((msec300 + startTime) <= at3);
        CPPUNIT_ASSERT((msec1000 + startTime) <= at4);
        CPPUNIT_ASSERT((msec1000 + startTime) <= at5);
    }

    // Check expected behavior of schedule() method for more timers
    void testScheduleManyTimers()
    {
        OsTimerQueue tq(&_msgSignalQueue);
        checkAfterRegularConstructor(tq, &_msgSignalQueue);

        // TEST: Add three timers and check that they all fired

        UtlString str100("Schedule msec100");
        OsMsgTest *msgTest100 = new OsMsgTest(str100);
        CPPUNIT_ASSERT(OS_SUCCESS == tq.scheduleOneshotAfter(msgTest100, msec100));
        CPPUNIT_ASSERT(1 == tq._timers.size());

        // TEST: Adding second timer should work
        UtlString str200("Schedule msec200");
        OsMsgTest *msgTest200 = new OsMsgTest(str200);
        CPPUNIT_ASSERT(OS_SUCCESS == tq.scheduleOneshotAfter(msgTest200, msec200));
        CPPUNIT_ASSERT(2 == tq._timers.size());

        // TEST: Adding third timer should work
        UtlString str1000("Schedule msec1000");
        OsMsgTest *msgTest1000 = new OsMsgTest(str1000);
        CPPUNIT_ASSERT(OS_SUCCESS == tq.scheduleOneshotAfter(msgTest1000, msec1000));

        // TEST: 3 timers added so the queue size should be 3
        // TEST: This proves that schedule does not remove unexpired timers
        // TEST: cleanUntil works for (interval != INFINITY)
        CPPUNIT_ASSERT(3 == tq._timers.size());

        //give them some time so that 100ms and 200ms timers fire;
        OsTask::delay(msec200.cvtToMsecs());

        // TEST: Check that the 100ms and 200ms timers fired
        checkTimerFired(_msgSignalQueue, msec100, str100, OS_SUCCESS);
        checkTimerFired(_msgSignalQueue, msec100, str200, OS_SUCCESS);

        // TEST: Upon addition schedule should remove the expired timers above
        UtlString str50("Schedule msec50");
        OsMsgTest *msgTest50 = new OsMsgTest(str50);
        CPPUNIT_ASSERT(OS_SUCCESS == tq.scheduleOneshotAfter(msgTest50, msec50));
        // TEST: 3 initial timers, 2 fired, 1 newly added timers so 2 timers should be in the queue
        CPPUNIT_ASSERT(2 == tq._timers.size());

        //give it some time for the remaining timers to fire;
        OsTask::delay(msec100.cvtToMsecs());
        // TEST: Last two timers fired
        checkTimerFired(_msgSignalQueue, msec50, str50, OS_SUCCESS);
        checkTimerFired(_msgSignalQueue, msec1000, str1000, OS_SUCCESS);

        tq.stop();
        // TEST: This proves that stop removes expired timers
        CPPUNIT_ASSERT(0 == tq._timers.size());
    }

    void testStopRemovesNonExpiredTimers()
    {
        OsTimerQueue tq(&_msgSignalQueue);
        checkAfterRegularConstructor(tq, &_msgSignalQueue);

        //TEST: Verify that stop() removes unexpired timers by adding two
        // timers with long expiration and call stop() immediately

        UtlString str300("Schedule msec300");
        OsMsgTest *msgTest300 = new OsMsgTest(str300);
        CPPUNIT_ASSERT(OS_SUCCESS == tq.scheduleOneshotAfter(msgTest300, msec300));
        OsMsgTest *msgTest301 = new OsMsgTest(str300);
        CPPUNIT_ASSERT(OS_SUCCESS == tq.scheduleOneshotAfter(msgTest301, msec300));
        CPPUNIT_ASSERT(2 == tq._timers.size());

        // TEST: Stop removed unexpired timers
        tq.stop();
        CPPUNIT_ASSERT(0 == tq._timers.size());

        //give them some time to supposedly fire;
        OsTask::delay(msec300.cvtToMsecs());

        // TEST: Removed unexpired timers do not fire
        checkTimerFired(_msgSignalQueue, msec100, str300, OS_WAIT_TIMEOUT);

        // TEST: Queue is reusable after stop(). Check this by adding another
        // timer and let it fire.
        UtlString str100("Schedule msec100");
        OsMsgTest *msgTest100 = new OsMsgTest(str100);
        CPPUNIT_ASSERT(OS_SUCCESS == tq.scheduleOneshotAfter(msgTest100, msec100));
        CPPUNIT_ASSERT(1 == tq._timers.size());

        //give them some time to supposedly fire;
        OsTask::delay(msec100.cvtToMsecs());

        // TEST: Latest added timer will fire after stop
        checkTimerFired(_msgSignalQueue, msec100, str100, OS_SUCCESS);
    }


    void testDestructorNoRemainingTimers()
    {
        // TEST: Destructor removes all non-expired timers
        // Verify this by adding a timer with long expiration and let the
        // queue be destroyed before timer fired

        UtlString str1000("Schedule msec1000");

        {
            OsTimerQueue tq(&_msgSignalQueue);
            checkAfterRegularConstructor(tq, &_msgSignalQueue);

            OsMsgTest *msgTest1000 = new OsMsgTest(str1000);
            CPPUNIT_ASSERT(OS_SUCCESS == tq.scheduleOneshotAfter(msgTest1000, msec1000));
            CPPUNIT_ASSERT(1 == tq._timers.size());

            //TEST: Do not call stop(), destructor should remove timers
        }

        //give them some time to supposedly fire;
        OsTask::delay(msec1000.cvtToMsecs());

        // TEST: Removed unexpired timers do not fire
        checkTimerFired(_msgSignalQueue, msec100, str1000, OS_WAIT_TIMEOUT);
    }

    OsMsgQ      _msgSignalQueue;
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsTimerQueueTest);

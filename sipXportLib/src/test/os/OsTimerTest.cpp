//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include <sys/time.h>

#include <sipxunit/TestUtilities.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <os/OsCallback.h>
#include <os/OsTimer.h>
#include <os/OsTimerTask.h>
#include <os/OsTime.h>
#include <sipxunit/TestUtilities.h>
#include <os/OsLock.h>
#include <os/OsEvent.h>
#include <os/OsTimerMsg.h>

#include <time.h>
#include <string.h>

/*
 * OSTIMETOLERANCE is the allowed 'slop', in milliseconds; timers may
 * be off by this much before the test is considered a failure.  The
 * current value may be too high; see XPL-39.
 */
#define OSTIMETOLERANCE 40

//#define REPORT_SKEW(x) printf x
#define REPORT_SKEW(x) /* x */
using namespace std;

OsTime      tenMsec(0, 10000);// timer offset ten msec into the future
OsTime      hundredMsec(0, 100000);// timer offset hundred msec into the future
OsTime      oneSecond(1,0);   // timer offset one second into the future
OsTime      twoSeconds(2,0);  // timer offset two seconds into the future
OsTime      tenSeconds(10,0); // timer offset ten seconds into the future
OsTime      tenYears(10*365*24*60*60, 0);  // ten years into the future

// The state variable for testDeleteFireRace.
typedef enum { INITIAL, DESTROYED, CONFLICT } State;
State testDeleteFireRace_stateVar;

class OsTimerTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(OsTimerTest);

    CPPUNIT_TEST(testImmediateTimer);
    CPPUNIT_TEST(testOneShotAfter);
    CPPUNIT_TEST(testTimerAccuracy);
    CPPUNIT_TEST(testOneShotAt);
    CPPUNIT_TEST(testStopTimerAfterOneShot);
    CPPUNIT_TEST(testPeriodicTimer);
    CPPUNIT_TEST(testOneshotPeriodicComboTimer);
    CPPUNIT_TEST(testStopPeriodicTimer);
    CPPUNIT_TEST(testPeriodicTimer_FractionalTime);
    CPPUNIT_TEST(testDeleteTimerBeforeExpires);

    CPPUNIT_TEST(testStartFire);
    CPPUNIT_TEST(testStartStop);
    CPPUNIT_TEST(testStartPeriodicStop);
    CPPUNIT_TEST(testStartDelete);
    CPPUNIT_TEST(testStartStopStartFire);
    CPPUNIT_TEST(testStartStart);
    CPPUNIT_TEST(testStartStopStop);
    CPPUNIT_TEST(testStartFireStop);
    CPPUNIT_TEST(testStop);
    CPPUNIT_TEST(testStartDeleteAsync);

    CPPUNIT_TEST(testDelayedStopMessage);
    CPPUNIT_TEST(testDeleteFireRace);

    CPPUNIT_TEST_SUITE_END();

private :
    struct timeval startTV;
    struct timeval delayTV;
    struct timeval endTV;

    int gCallBackCount;

public:

    void setUp()
    {
        gCallBackCount = 0;
        startTV.tv_sec = startTV.tv_usec = 0;
        endTV = startTV;
    }

    void tearDown()
    {
    }

    void setTVCalled()
    {
       gettimeofday(&endTV, NULL);
       gCallBackCount++ ;
    }

    static void TVCallback(void* userData, const intptr_t eventData)
    {
       OsTimerTest* foo = (OsTimerTest*) userData;
       foo->setTVCalled();
    }

    long SecsToUsecs(long secs)
    {
        return (secs*OsTime::USECS_PER_SEC);
    }

    long MsecsToUsecs(long mSecs)
    {
        return (mSecs*OsTime::USECS_PER_MSEC);
    }

    long SecsToMsecs(long secs)
    {
        return (secs * OsTime::MSECS_PER_SEC);
    }

    long getTimeDeltaInUsecs()
    {
        return (SecsToUsecs(endTV.tv_sec - startTV.tv_sec) +
               (endTV.tv_usec - startTV.tv_usec));
    }

    void DynaTest()
    {
    }

    void testImmediateTimer()
    {
       // Supposedly doesn't work under XPL-101, but we need to retest.
//#ifdef _WIN32
//       KNOWN_FATAL_BUG("Fails under Win32", "XPL-101");
//#endif
       OsCallback* pNotifier;
       OsTimer* pTimer;
       OsStatus returnValue;
       long diffUSecs;
       pNotifier = new OsCallback((void*)this, TVCallback);
       pTimer = new OsTimer(*pNotifier);
       gCallBackCount = 0;
       gettimeofday(&startTV, NULL);
       returnValue = pTimer->oneshotAfter(OsTime::NO_WAIT);
       // Although the message is supposed to be immediate, give a
       // little extra time
       OsTask::delay(OSTIMETOLERANCE);
       CPPUNIT_ASSERT_MESSAGE("Handle timer 1 (immediate) - ReturnValue",
                              returnValue == OS_SUCCESS);
       CPPUNIT_ASSERT_MESSAGE("Handle timer 1 (immediate) - Timer was fired",
                              gCallBackCount == 1);
       diffUSecs = getTimeDeltaInUsecs();
       REPORT_SKEW(("      Timing inaccuracy = %6ld us;\n", diffUSecs));

       delete pTimer;
       delete pNotifier;
    }

    void testOneShotAfter()
    {
       struct TestOneShotStruct
       {
          const char* testDescription;
          long seconds;
          long milliseconds;
          int tolerance;
       };

       OsCallback* pNotifier;
       string Message;
       int testCount;

       TestOneShotStruct testData[] = {
          { "Test one shot after when time is specified as 0", 0, 0, OSTIMETOLERANCE },
          { "Test one shot after when time is equal to one second", 1, 0,
            OSTIMETOLERANCE },
          /* The next case was added to check if the inaccuracy applies ONLY to decimal
             values or even to integers
          */
          { "Test one shot after when time is greater than one second", 2, 285,
            OSTIMETOLERANCE },
          { "Test one shot after when time is an integer > 1", 3, 0, OSTIMETOLERANCE },
          { "Test one shot after when time is greater than 0 but less than 1", 0, 252,
            OSTIMETOLERANCE },
       };

       testCount = sizeof(testData)/ sizeof(testData[0]);

       for (int i = 0; i < testCount; i++)
       {
          long expectedWaitUSecs;
          long diffUSecs;
          OsTimer* pTimer;
          UtlBoolean returnValue;

          OsTime timeToWait(testData[i].seconds,
                            testData[i].milliseconds*OsTime::USECS_PER_MSEC);

          pNotifier = new OsCallback((void*)this, TVCallback);
          pTimer = new OsTimer(*pNotifier);

          expectedWaitUSecs = SecsToUsecs(testData[i].seconds) +
             MsecsToUsecs(testData[i].milliseconds);

          // Give a small delay so we synchronize with the timer.
          OsTask::delay(20);
          gettimeofday(&startTV, NULL);
          gCallBackCount = 0;
          returnValue = pTimer->oneshotAfter(timeToWait);

          OsTask::delay(expectedWaitUSecs / OsTime::USECS_PER_MSEC +
                        testData[i].tolerance);

          // gCallBackCount is reinitialized to 0 each iteration, so
          // its value should be 1 now.

          UtlString failureMessage;

          failureMessage.remove(0);
          failureMessage.append("Timer did not fire for iteration ");
          failureMessage.appendNumber(i);
          failureMessage.append("\n");
          failureMessage.append(testData[i].testDescription);
          failureMessage.append("\n  seconds:      ");
          failureMessage.appendNumber(testData[i].seconds);
          failureMessage.append("\n  milliseconds: ");
          failureMessage.appendNumber(testData[i].milliseconds);
          failureMessage.append("\n  tolerance:    ");
          failureMessage.appendNumber(testData[i].tolerance);

          KNOWN_BUG("Fails on ecs-fc8. Timing issue.", "XECS-1975");
          CPPUNIT_ASSERT_MESSAGE(failureMessage.data(), gCallBackCount == 1);

          failureMessage.remove(0);
          failureMessage.append("oneshotAfter returned failure on iteration ");
          failureMessage.appendNumber(i);
          failureMessage.append("\n");
          failureMessage.append(testData[i].testDescription);
          failureMessage.append("\n  seconds:      ");
          failureMessage.appendNumber(testData[i].seconds);
          failureMessage.append("\n  milliseconds: ");
          failureMessage.appendNumber(testData[i].milliseconds);
          failureMessage.append("\n  tolerance:    ");
          failureMessage.appendNumber(testData[i].tolerance);
          CPPUNIT_ASSERT_MESSAGE(failureMessage.data(), returnValue);

          diffUSecs = getTimeDeltaInUsecs();
          REPORT_SKEW(("      Timing inaccuracy for iter %3d = %8ld us; Time = %ld.%03ld;\n",
                       i,
                       diffUSecs - expectedWaitUSecs,
                       testData[i].seconds,
                       testData[i].milliseconds
                         ));
          delete pTimer;
          delete pNotifier;
       }
    }

    void testTimerAccuracy()
    {
       OsCallback* pNotifier;
       OsTimer* pTimer;
       long expectedWaitUSecs;
       long diffUSecs;

       pNotifier = new OsCallback((void*)this, TVCallback);
       pTimer = new OsTimer(*pNotifier);

       expectedWaitUSecs=(1*OsTime::USECS_PER_SEC) + (250*OsTime::USECS_PER_MSEC);
       OsTime timeToWait(1, 250*OsTime::USECS_PER_MSEC);

       // Give a small delay so we synchronize with the timer.
       OsTask::delay(20);
       gettimeofday(&startTV, NULL);
       pTimer->oneshotAfter(timeToWait);
       // Sleep for a slightly additional time
       OsTask::delay(expectedWaitUSecs / OsTime::USECS_PER_MSEC +
                     OSTIMETOLERANCE);

       CPPUNIT_ASSERT_MESSAGE("Timer was fired",
                              gCallBackCount == 1);
       diffUSecs = getTimeDeltaInUsecs();
       REPORT_SKEW(("      Timing inaccuracy = %8ld us; Time = %d.%03d;\n",
                    diffUSecs - expectedWaitUSecs,
                    1, 250
                      ));

       delete pTimer;
       delete pNotifier;
    }

    void testOneShotAt()
    {
//       KNOWN_FATAL_BUG("Create tests for methods testOneShotAt and testPeriodicAt", "XPL-40");
       OsCallback* pNotifier;
       OsTimer* pTimer;
       UtlBoolean returnValue;
       long diffUSecs;
       pNotifier = new OsCallback((void*)this, TVCallback);
       pTimer = new OsTimer(*pNotifier);
       // Create an OsDateTime object for 2 seconds in the future
       // and call oneShotAt.
       // Get the current time in seconds and microseconds.
       struct timeval now;
       gettimeofday(&now, NULL);
       // Add 2 to the seconds and put it into a time_t so we can call gmtime.
       time_t now_t = now.tv_sec + 2;
       tm* gmtPlus2 = gmtime(&now_t);
       // Put the components of gmtime into an OsDateTime, and add the
       // microseconds from 'now'.
       OsDateTime odt(
          // gmtime returns (year - 1900), but OsDateTime requires 4-digit year.
          (unsigned short) gmtPlus2->tm_year + 1900,
          (unsigned char) gmtPlus2->tm_mon,
          (unsigned char) gmtPlus2->tm_mday,
          (unsigned char) gmtPlus2->tm_hour,
          (unsigned char) gmtPlus2->tm_min,
          (unsigned char) gmtPlus2->tm_sec,
          (unsigned int) now.tv_usec
          );
       gettimeofday(&startTV, NULL);
       gCallBackCount = 0;
       returnValue = pTimer->oneshotAt(odt);
       // Although the message is supposed to be immediate, give a little extra time
       OsTask::delay(2000 + OSTIMETOLERANCE);
       CPPUNIT_ASSERT_MESSAGE("Handle timer 1 - returnValue", returnValue);
       CPPUNIT_ASSERT_MESSAGE("Timer was fired",
                              gCallBackCount == 1);
       diffUSecs = getTimeDeltaInUsecs();
       REPORT_SKEW(("      Timing inaccuracy = %6ld us;\n",
                    diffUSecs - MsecsToUsecs(2000)));

       delete pTimer;
       delete pNotifier;
    }

    void testStopTimerAfterOneShot()
    {
       OsCallback* pNotifier;
       OsTimer* pTimer;
       pNotifier = new OsCallback((void*)this, TVCallback);
       pTimer = new OsTimer(*pNotifier);
       gCallBackCount = 0;
       pTimer->oneshotAfter(oneSecond);
       OsTask::delay(500);
       pTimer->stop();
       OsTask::delay(1200);
       // We have waited for 1.7 sec after arming the timer, and the timer
       // was scheduled to fire after 1.0 sec.  But we disarmed the timer
       // after 0.5 sec, so gCallBackCount should be 0.
       CPPUNIT_ASSERT_MESSAGE("Verify that canceling the timer disarms it",
                              gCallBackCount == 0);
       delete pTimer;
       delete pNotifier;
    }

    void testPeriodicTimer()
    {
        OsCallback* pNotifier;
        OsTimer* pTimer;
        UtlBoolean returnValue;
        pNotifier = new OsCallback((void*)this, TVCallback);
        pTimer = new OsTimer(*pNotifier);
        gCallBackCount = 0;
        returnValue = pTimer->periodicEvery(twoSeconds, twoSeconds);
        // Give a delay of 10+ seconds . If all went well the call back method
        // must have been called once every 2 seconds and hence the callbackcount
        // must be up by 5.
        OsTask::delay(11250);
        CPPUNIT_ASSERT_MESSAGE("Test periodic timer - verify return value",
            returnValue);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Test periodic timer - verify that the "
                                     "timer is called periodically",
                                     5, gCallBackCount);
        delete pTimer;
        delete pNotifier;
    }

    void testPeriodicTimer_FractionalTime()
    {
       OsCallback* pNotifier;
       OsTimer* pTimer;
       pNotifier = new OsCallback((void*)this, TVCallback);
       pTimer = new OsTimer(*pNotifier);
       gCallBackCount = 0;
       pTimer->periodicEvery(OsTime::NO_WAIT, hundredMsec);
       // Give a delay of 1+ seconds . If all went well the call back method
       // must have been called once in the begining and every 100 milliseconds thereafter
       // and hence the callbackcount must be up by 10+1.
       OsTask::delay(1010);
//       KNOWN_BUG("Intermittent failure here; not predictable", "XPL-52");
       CPPUNIT_ASSERT_EQUAL_MESSAGE("Test periodic timer - verify that the fractional timer is "
                                    "*indeed* called periodically", 11, gCallBackCount);
       delete pTimer;
       delete pNotifier;
    }

    void testOneshotPeriodicComboTimer()
    {
       UtlBoolean returnValue;
       OsCallback* pNotifier;
       OsTimer* pTimer;
       long diffUSecs;
       pNotifier = new OsCallback((void*)this, TVCallback);
       pTimer = new OsTimer(*pNotifier);
       gCallBackCount = 0;
       gettimeofday(&startTV, NULL);
       returnValue = pTimer->periodicEvery(oneSecond, twoSeconds);
       OsTask::delay(1000 + OSTIMETOLERANCE);
       CPPUNIT_ASSERT_MESSAGE("Test oneshot & periodic timer combo - "
                              "verify return value", returnValue);
       CPPUNIT_ASSERT_MESSAGE("Timer was fired",
                              gCallBackCount == 1);
       diffUSecs = getTimeDeltaInUsecs();
       REPORT_SKEW(("      Timing inaccuracy = %ld us;\n",
                    diffUSecs - MsecsToUsecs(1000)));

       // now wait for another 5+ seconds. The total time after starting the timer is
       // 6 seconds.
       OsTask::delay(5340);
       CPPUNIT_ASSERT_EQUAL_MESSAGE("Test oneshot/periodic combo - Verify the timer is called "
                                    "repeatadly as per the second argument", 3, gCallBackCount);
       delete pTimer;
       delete pNotifier;
    }

    void testStopPeriodicTimer()
    {
       OsCallback* pNotifier;
       OsTimer* pTimer;
       OsTimer* pTimer2;
       long diffUSecs;
       pNotifier = new OsCallback((void*)this, TVCallback);
       pTimer = new OsTimer(*pNotifier);
       gCallBackCount = 0;
       pTimer->periodicEvery(oneSecond, twoSeconds);
       // Test the case where the timer is stopped even before the first leg
       // is fired
       OsTask::delay(350);
       pTimer->stop();
       // Wait for another 5 seconds. Neither the first shot nor the repeat legs
       // should ever have been called.
       OsTask::delay(5000);
       CPPUNIT_ASSERT_MESSAGE("Verify that a periodictimer can be stopped even "
                              "before the first leg is called", gCallBackCount == 0);
       delete pTimer;

       pTimer2 = new OsTimer(*pNotifier);
       gCallBackCount = 0;
       gettimeofday(&startTV, NULL);
       pTimer2->periodicEvery(oneSecond, twoSeconds);
       OsTask::delay(1000 + OSTIMETOLERANCE);
       pTimer2->stop();
       // Wait for another 5 seconds. Only the first shot should have been called.
       OsTask::delay(5000);
       CPPUNIT_ASSERT_MESSAGE("Timer was fired",
                              gCallBackCount == 1);
       diffUSecs = getTimeDeltaInUsecs();
       REPORT_SKEW(("      Timing inaccuracy = %ld us;\n",
                    diffUSecs - MsecsToUsecs(1000)));

       // Also verify that only the first leg was called.
       CPPUNIT_ASSERT_EQUAL_MESSAGE("Test stoping periodic timer - Verify that ONLY the first "
                                    "leg was fired", 1, gCallBackCount);
       delete pTimer2;
       delete pNotifier;
    }

    void testDeleteTimerBeforeExpires()
    {
       OsCallback* pNotifier;
       OsTimer* pTimer;
       pNotifier = new OsCallback((void*)this, TVCallback);
       pTimer = new OsTimer(*pNotifier);
       gCallBackCount = 0;
       pTimer->periodicEvery(oneSecond, twoSeconds);
       OsTask::delay(350);
       delete pTimer;
       // Wait for another 5 seconds. Neither the first shot nor the repeat legs
       // should ever have been called.
       OsTask::delay(5000);
       CPPUNIT_ASSERT_MESSAGE("Verify that a periodictimer can be stopped even "
                              "before the first leg is called", gCallBackCount == 0);
       delete pNotifier;
    }

    // Tests for various sequences of operations.

    void testStartFire()
    {
       OsStatus returnValue;
       OsCallback notifier((void*) this, TVCallback);
       OsTimer timer(notifier);
       gCallBackCount = 0;
       returnValue =  timer.oneshotAfter(oneSecond);
       CPPUNIT_ASSERT_MESSAGE("oneshotAfter", returnValue == OS_SUCCESS);
       OsTask::delay(5000);
       CPPUNIT_ASSERT_MESSAGE("Test start/fire", gCallBackCount == 1);
    }

    void testStartStop()
    {
       OsStatus returnValue;
       OsCallback notifier((void*) this, TVCallback);
       OsTimer timer(notifier);
       gCallBackCount = 0;
       returnValue = timer.oneshotAfter(oneSecond);
       CPPUNIT_ASSERT_MESSAGE("oneshotAfter", returnValue == OS_SUCCESS);
       OsTask::delay(500);
       returnValue = timer.stop();
       CPPUNIT_ASSERT_MESSAGE("stop", returnValue == OS_SUCCESS);
       OsTask::delay(5000);
       CPPUNIT_ASSERT_MESSAGE("Test start/stop", gCallBackCount == 0);
    }

    void testStartPeriodicStop()
    {
       OsStatus returnValue;
       OsCallback notifier((void*) this, TVCallback);
       OsTimer timer(notifier);
       gCallBackCount = 0;
       returnValue = timer.periodicEvery(oneSecond, twoSeconds);
       CPPUNIT_ASSERT_MESSAGE("periodicEvery", returnValue == OS_SUCCESS);
       // Allow to fire 3 times
       OsTask::delay(6000);
       returnValue = timer.stop();
       CPPUNIT_ASSERT_MESSAGE("stop", returnValue == OS_SUCCESS);
       OsTask::delay(5000);
       CPPUNIT_ASSERT_MESSAGE("Test start-periodic/fire/stop",
                              gCallBackCount == 3);
    }

    void testStartDelete()
    {
       OsStatus returnValue;
       OsCallback notifier((void*) this, TVCallback);
       OsTimer* pTimer = new OsTimer(notifier);
       gCallBackCount = 0;
       returnValue = pTimer->oneshotAfter(oneSecond);
       CPPUNIT_ASSERT_MESSAGE("oneshotAfter", returnValue == OS_SUCCESS);
       OsTask::delay(500);
       // Delete the timer before it can fire.
       delete pTimer;
       // Make sure it did not fire.
       OsTask::delay(5000);
       CPPUNIT_ASSERT_MESSAGE("Test start/delete", gCallBackCount == 0);
    }

    void testStartStopStartFire()
    {
       OsStatus returnValue;
       OsCallback notifier((void*) this, TVCallback);
       OsTimer timer(notifier);
       gCallBackCount = 0;
       returnValue = timer.oneshotAfter(oneSecond);
       CPPUNIT_ASSERT_MESSAGE("oneshotAfter 1", returnValue == OS_SUCCESS);
       OsTask::delay(500);
       returnValue = timer.stop();
       CPPUNIT_ASSERT_MESSAGE("stop", returnValue == OS_SUCCESS);
       OsTask::delay(500);
       returnValue = timer.oneshotAfter(oneSecond);
       CPPUNIT_ASSERT_MESSAGE("oneshotAfter 2", returnValue == OS_SUCCESS);
       OsTask::delay(5000);
       CPPUNIT_ASSERT_MESSAGE("Test start/fire", gCallBackCount == 1);
    }

    void testStartStart()
    {
       OsStatus returnValue;
       OsCallback notifier((void*) this, TVCallback);
       OsTimer timer(notifier);
       gCallBackCount = 0;
       returnValue = timer.oneshotAfter(oneSecond);
       CPPUNIT_ASSERT_MESSAGE("oneshotAfter 1", returnValue == OS_SUCCESS);
       OsTask::delay(500);
       returnValue = timer.oneshotAfter(oneSecond);
       CPPUNIT_ASSERT_MESSAGE("oneshotAfter 2", returnValue == OS_FAILED);
       OsTask::delay(5000);
       CPPUNIT_ASSERT_MESSAGE("Test start/fire", gCallBackCount == 1);
    }

    void testStartStopStop()
    {
       OsStatus returnValue;
       OsCallback notifier((void*) this, TVCallback);
       OsTimer timer(notifier);
       gCallBackCount = 0;
       returnValue = timer.oneshotAfter(oneSecond);
       CPPUNIT_ASSERT_MESSAGE("oneshotAfter", returnValue == OS_SUCCESS);
       OsTask::delay(500);
       returnValue = timer.stop();
       CPPUNIT_ASSERT_MESSAGE("stop 1", returnValue == OS_SUCCESS);
       OsTask::delay(500);
       returnValue = timer.stop();
       CPPUNIT_ASSERT_MESSAGE("stop 2", returnValue == OS_FAILED);
       OsTask::delay(5000);
       CPPUNIT_ASSERT_MESSAGE("Test start/fire", gCallBackCount == 0);
    }

    void testStartFireStop()
    {
       OsStatus returnValue;
       OsCallback notifier((void*) this, TVCallback);
       OsTimer timer(notifier);
       gCallBackCount = 0;
       returnValue = timer.oneshotAfter(oneSecond);
       CPPUNIT_ASSERT_MESSAGE("oneshotAfter", returnValue == OS_SUCCESS);
       OsTask::delay(1500);
       returnValue = timer.stop();
       CPPUNIT_ASSERT_MESSAGE("stop", returnValue == OS_FAILED);
       OsTask::delay(5000);
       CPPUNIT_ASSERT_MESSAGE("Test start/fire", gCallBackCount == 1);
    }

    void testStop()
    {
       OsStatus returnValue;
       OsCallback notifier((void*) this, TVCallback);
       OsTimer timer(notifier);
       gCallBackCount = 0;
       returnValue = timer.stop();
       CPPUNIT_ASSERT_MESSAGE("stop", returnValue == OS_FAILED);
       CPPUNIT_ASSERT_MESSAGE("Test start/fire", gCallBackCount == 0);
    }

    void testStartDeleteAsync()
    {
       OsStatus returnValue;
       OsCallback notifier((void*) this, TVCallback);
       OsTimer* pTimer = new OsTimer(notifier);
       gCallBackCount = 0;
       returnValue = pTimer->oneshotAfter(oneSecond);
       CPPUNIT_ASSERT_MESSAGE("oneshotAfter", returnValue == OS_SUCCESS);
       OsTask::delay(500);
       // Delete the timer before it can fire using deleteAsync.
       OsTimer::deleteAsync(pTimer);
       // Make sure it did not fire.
       OsTask::delay(5000);
       CPPUNIT_ASSERT_MESSAGE("Test start/deleteAsync", gCallBackCount == 0);
    }

    void testDelayedStopMessage()
    {
       // Test a race condition where a periodic timer fires after
       // the application has stopped it, but before OsTimerTask has
       // processed the update message for the stop.
       // Previously, OsTimerTask would leave the task state as
       // "started" but not put the timer back in the timer queue, so
       // when the update message caused OsTimerTask::removeTimer to run, it
       // couldn't find the timer.
       OsCallback notifier((void*) this, TVCallback);
       OsTimer timer(notifier);

       // Start timer to fire in 1 sec, and every 1 sec after that.
       timer.periodicEvery(OsTime(1, 0), OsTime(1, 0));
       // Delay slightly to ensure the timer start message is processed,
       // so timer.mOutstandingMessages == 0.
       OsTask::delay(100);
       CPPUNIT_ASSERT(timer.mOutstandingMessages == 0);

       // This is timer.OsTimer::stop(TRUE), with a delay put in the middle.
       {
          UtlBoolean synchronous = TRUE;
          OsStatus result;
          {
             UtlBoolean sendMessage = FALSE;

             // Update members.
             {
                OsLock lock(timer.mBSem);

                // Determine whether the call is successful.
                if (OsTimer::isStarted(timer.mApplicationState))
                {
                   // Update state to stopped.
                   timer.mApplicationState++;
                   result = OS_SUCCESS;
                   if (timer.mOutstandingMessages == 0)
                   {
                      // We will send a message.
                      sendMessage = TRUE;
                      timer.mOutstandingMessages++;
                   }
                }
                else
                {
                   result = OS_FAILED;
                }
             }

             // Delay 2 seconds, which is long enough for the timer to fire.
             OsTask::delay(2000);

             // If we need to, send an UPDATE message to the timer task.
             if (sendMessage)
             {
                if (synchronous) {
                   // Send message and wait.
                   OsEvent event;
                   OsTimerMsg msg(OsTimerMsg::UPDATE_SYNC, &timer, &event);
                   OsStatus res = OsTimerTask::getTimerTask()->postMessage(msg);
                   assert(res == OS_SUCCESS);
                   event.wait();
                }
                else
                {
                   // Send message.
                   OsTimerMsg msg(OsTimerMsg::UPDATE, &timer, NULL);
                   OsStatus res = OsTimerTask::getTimerTask()->postMessage(msg);
                   assert(res == OS_SUCCESS);
                }
             }
          }
          CPPUNIT_ASSERT(result == OS_SUCCESS);
       }

       // We do no explicit test for success of this test, as the success
       // condition is that it does not trigger "assert(current)" in
       // OsTimerTask::removeTimer.
    }

    // Service function for testDeleteFireRace.
    static void slowNotifier(void* userData, const intptr_t eventData)
    {
       // Delay for 1 sec.
       OsTask::delay(1000);

       // If the state is DESTROYED, the destructor finished while the
       // notifier was still executing.  Set the state to indicate this.
       if (testDeleteFireRace_stateVar == DESTROYED)
       {
          testDeleteFireRace_stateVar = CONFLICT;
       }
    }

    // Test for a race that happens when OsTimer::~OsTimer is called while
    // a timer is being fired.  In some cases, the destructor couldn't tell
    // that the timer was still being used and deleted it, rather than
    // sending an UPDATE_SYNC message first.
    void testDeleteFireRace()
    {
       // The plan is to start a timer which fires immediately.  The notifier
       // for the timer will wait 1 sec.  (In principle, a timer notifier
       // should not block, but there is no guarantee that a notifier will
       // execute immediately.)  Meanwhile, after 1/2 sec, the mainline will
       // call the destructor.  Under the old implementation of the destructor,
       // it would not discover that the timer was still being used and it
       // would delete it immediately.  After the 1 sec wait is up, the
       // notifier code will continue executing.
       // We detect the error by keeping a state variable.  It starts at
       // INITIAL.  When the destructor is done, it advances the value to
       // DESTROYED.  If the end of the notifier routine sees the
       // value DESTROYED (meaning that the timer has been deleted "out from
       // under" the notifier code), it will advance it to the value CONFLICT.
       // The mainline waits until all this action should have completed and
       // checks for the value DESTROYED (which is OK) or CONFLICT (which
       // is a failure).

       OsCallback notifier((void*) this, slowNotifier);
       OsTimer* pTimer = new OsTimer(notifier);
       // Set the state variable.
       testDeleteFireRace_stateVar = INITIAL;

       // Start the timer, to fire immediately.
       pTimer->oneshotAfter(OsTime(0, 0));

       // Wait 1/2 sec.
       OsTask::delay(500);

       // Destroy the timer.
       delete pTimer;

       // Set the state to DESTROYED.
       testDeleteFireRace_stateVar = DESTROYED;

       // Wait 1 sec, to ensure the notifier has completed, even if the
       // destructor did not wait for it to happen.
       OsTask::delay(1000);

       // Check that the state is DESTROYED, not CONFLICT.
       CPPUNIT_ASSERT(testDeleteFireRace_stateVar == DESTROYED);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsTimerTest);

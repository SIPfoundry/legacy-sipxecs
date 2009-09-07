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
#include <os/OsEvent.h>
#include <os/OsServerTask.h>
#include <os/OsRpcMsg.h>
#include <os/OsDateTime.h>

class RightEventThread : public OsServerTask
{
    public:

    int mNumEvents;
    int mMaxEvents;
    int* mpDeletedEvent;

    RightEventThread(int* eventOutcome, int eventsSize)
    {
        mNumEvents = -1;
        mMaxEvents = eventsSize;
        mpDeletedEvent = eventOutcome;
    }

    UtlBoolean handleMessage(OsMsg& rMsg)
    {
        int waitMsec = rand();
        delay((waitMsec % 3 ) * 50);

        OsEvent* event = ((OsRpcMsg&)rMsg).getEvent();
        CPPUNIT_ASSERT(event);

        mNumEvents++;
        void* eventIndex = NULL;
        event->getUserData(eventIndex);
        //CPPUNIT_ASSERT(mNumEvents == eventIndex);
        CPPUNIT_ASSERT(mNumEvents < mMaxEvents);

        OsStatus eventStat = event->signal(mNumEvents);
        if(eventStat == OS_ALREADY_SIGNALED)
        {
           // The Right side lost, the Left side is done
           // we delete on this side
           delete event;
           event = NULL;
           mpDeletedEvent[mNumEvents] = TRUE;
        }
        else
        {
           // This/Right side won. we do nothing
           mpDeletedEvent[mNumEvents] = FALSE;
           //osPrintf("Right: %d\n", eventStat);
        }
        return(TRUE);
    }
};

class OsEventTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(OsEventTest);
    CPPUNIT_TEST(testTimedEvent);
    CPPUNIT_TEST(testThreadedEvent);
    CPPUNIT_TEST_SUITE_END();


public:
    void testTimedEvent()
    {
        OsTime   eventTimeout(2,0);
        OsEvent* pEvent;

        pEvent = new OsEvent((void*)12345);
        int epochTime = time(NULL);
        CPPUNIT_ASSERT(pEvent->wait(eventTimeout) != OS_SUCCESS);
        pEvent->signal(67890);
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pEvent->wait(eventTimeout));
        pEvent->reset();
        CPPUNIT_ASSERT(pEvent->wait(eventTimeout) != OS_SUCCESS);
        epochTime = time(NULL) - epochTime;

        // Make sure we waited (approximately) 2 seconds each time.
        CPPUNIT_ASSERT(epochTime > 2 && epochTime < 6);

        delete pEvent;
    }

    void testThreadedEvent()
    {
        // Seed the random number generator
        srand(OsDateTime::getSecsSinceEpoch());

        int numTries = 100;
        int* rightResults = new int[numTries];
        int* leftResults = new int[numTries];

        // Create the Right thread.  This context will be the
        // Left thread.
        RightEventThread rightThread(rightResults, numTries);
        rightThread.start();

        int index;
        for(index = 0; index < numTries; index++)
        {
            OsEvent* event = new OsEvent((void*)index);
            OsRpcMsg eventMsg(OsMsg::USER_START,0,*event);
            rightThread.postMessage(eventMsg);

            int waitTimeMsec = (rand() % 3) * 110;
            OsTime time(0, waitTimeMsec * 1000);
            event->wait(time);

            OsStatus eventStat = event->signal(index);
            if(eventStat == OS_ALREADY_SIGNALED)
            {
                // We (Left) lost the other side is done
                intptr_t eventData;
                event->getEventData(eventData);
                CPPUNIT_ASSERT(eventData == index);

                // This/Left side deletes the event
                delete event;
                event = NULL;
               leftResults[index] = TRUE;
            }
            else
            {
                // The other/Right side lost
                // Do nothing
                leftResults[index] = FALSE;
                //osPrintf("Left: %d\n", eventStat);
            }
        }

        OsTask::delay(1000);

        int leftDeletes = 0;
        int rightDeletes = 0;
        for(index = 0; index < numTries; index++)
        {
            if(leftResults[index] == TRUE)
            {
                leftDeletes++;
            }
            if(rightResults[index] == TRUE)
            {
                rightDeletes++;
            }
            if(rightResults[index] == leftResults[index])
            {
               //osPrintf("Left deleted: %d Right deleted: %d\n",
               //           leftDeletes, rightDeletes);
               //osPrintf("[%d]: Both sides %s\n", index,
               //        rightResults[index] ? "Deleted" : "Did not delete");
            }
            CPPUNIT_ASSERT(rightResults[index] != leftResults[index]);
        }

        //osPrintf("Left deleted: %d Right deleted: %d\n",
        //        leftDeletes, rightDeletes);

        CPPUNIT_ASSERT(leftDeletes + rightDeletes == numTries);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsEventTest);

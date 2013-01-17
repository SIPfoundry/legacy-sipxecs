
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
#include <os/OsPooledTask.h>
#include <os/OsRpcMsg.h>
#include <os/OsDateTime.h>




class PooledEventThread : public OsPooledTask
{
    public:

    int mNumEvents;
    int mMaxEvents;
    int* mpDeletedEvent;



    PooledEventThread(int* eventOutcome, int eventsSize) :
       OsPooledTask("OsPooledTask", 10, eventsSize)
    {
        mNumEvents = -1;
        mMaxEvents = eventsSize;
        mpDeletedEvent = eventOutcome;
    }

    UtlBoolean handleMessage(OsMsg& rMsg)
    {
        if (rMsg.getMsgType() == OsMsg::OS_SHUTDOWN)
          return TRUE;

        OsEvent* event = ((OsRpcMsg&)rMsg).getEvent();
        CPPUNIT_ASSERT(event);

        mNumEvents++;
        void* eventIndex = NULL;
        event->getUserData(eventIndex);
        
        CPPUNIT_ASSERT(mNumEvents < mMaxEvents);

        OsStatus eventStat = event->signal(mNumEvents);
        CPPUNIT_ASSERT(eventStat != OS_ALREADY_SIGNALED);



       delete event;
       event = NULL;
       mpDeletedEvent[mNumEvents] = TRUE;
       //std::cout << "EVENT " << mNumEvents << " PROCESSED " << (intptr_t)eventIndex << "\r\n";

        return(TRUE);
    }
};

class OsPooledEventTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(OsPooledEventTest);
    CPPUNIT_TEST(testPooledThreadedEvent);
    CPPUNIT_TEST_SUITE_END();


public:

    void testPooledThreadedEvent()
    {
        // Seed the random number generator
        srand(OsDateTime::getSecsSinceEpoch());

        int numTries = 10;
       

        for (int i = 0; i < numTries; i++)
        {
          int index;
          int* rightResults = new int[numTries];
          for(index = 0; index < numTries; index++)
          {
            rightResults[index] = FALSE;
          }
          // Create the Right thread.  This context will be the
          // Left thread.
          PooledEventThread rightThread(rightResults, numTries);
          rightThread.start();
          
          for(index = 0; index < numTries; index++)
          {
              OsEvent* event = new OsEvent((void*)index);
              OsRpcMsg eventMsg(OsMsg::USER_START,0,*event);
              rightThread.postMessage(eventMsg);
          }

          OsTask::delay(100);

          for(index = 0; index < numTries; index++)
          {
            CPPUNIT_ASSERT(rightResults[index]);
          }

          delete[] rightResults;
        }
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsPooledEventTest);

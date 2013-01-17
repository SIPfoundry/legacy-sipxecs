//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>
#include <os/iostream>

#ifdef __pingtel_on_posix__
#include <dlfcn.h>
#endif

#ifdef TEST
#include "utl/UtlMemCheck.h"
#endif

// APPLICATION INCLUDES

// Start of includes for test block
#include "os/OsBSem.h"
#include "os/OsCallback.h"
#include "os/OsConfigDb.h"
#include "os/OsCSem.h"
#include "os/OsDateTime.h"
#include "os/OsExcept.h"
#include "os/OsEvent.h"
#include "os/OsLock.h"
#include "os/OsMsg.h"
#include "os/OsMsgQ.h"
#include "os/OsMutex.h"
#include "os/OsNameDb.h"
#include "os/OsReadLock.h"
#include "os/OsRWMutex.h"
#include "os/OsTime.h"
#include "os/OsTimer.h"
#include "os/OsWriteLock.h"
#include "os/OsTask.h"
#include "os/OsFS.h"
#include "os/OsProcess.h"
#include "os/OsStatus.h"

// End of includes for test block

//used for OsProcess Testing
OsProcess process;

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* ============================ FUNCTIONS ================================= */

/* ============================ Os Process Test Funcs ================================= */
OsStatus TestProcessIterator()
{
    OsProcessIterator *processIterator;
    cout << "Starting Process Iterator Test..." << endl;

    processIterator = new OsProcessIterator();

    OsStatus retval = processIterator->findFirst(process);
    while (retval == OS_SUCCESS)
    {
        UtlString name;
        process.getProcessName(name);
        cout << "Found PID = " << process.getPID() << "  ParentPID = " << process.getParentPID() << "  Name = " <<
            name.data() << endl;
        retval = processIterator->findNext(process);
    }

    delete processIterator;

    cout << "DONE Process Iterator Test.\n";

    retval = OS_SUCCESS;
    return retval;
}

OsStatus  TestProcessClass()
{

    OsStatus retval = OS_FAILED;

    cout << "Starting Process Class Method Test...\n";

    //try to launch dir
    UtlString appName = "ping";
    UtlString params[10];
    params[0] = "127.0.0.1";

#ifdef _WIN32  //need to do this only on win32, linux already does this by default
    params[1] = "-t";
#endif

    //try and launch IE
    UtlString envKey =   "TESTKEY1";
    UtlString envValue = "TESTVALUE1";
    process.setEnv(envKey,envValue);

    envKey =  "TESTKEY2";
    envValue ="TESTVALUE2";
    process.setEnv(envKey,envValue);

    envKey = "TESTKEY3";
    envValue = "TESTVALUE3";
    process.setEnv(envKey,envValue);

    OsPath startupDir = ".";

    cout << "Launching process: " << appName.data() << endl;
    if (process.launch(appName,params,startupDir) == OS_SUCCESS)
    {
        if (process.isRunning())
        {
            cout << "Successful launch. " << endl;

            //try to set the prio to 1
            int prio;
            process.getPriority(prio);
            cout << "Current priority = " << prio << endl;
            process.setPriority(1);

            process.getPriority(prio);
            cout << "New priority (should say 1) = " << prio << endl;



            OsProcess newProcess;

            //see if we can get the process we started by pid
            if (OsProcess::getByPID(process.getPID(),newProcess) == OS_SUCCESS)
            {
                //wait a bit
                cout << "Waiting 5 secs before killing process..." << endl;
                OsTask::delay(5000);
                //ok, now kill that bad boy...
                if (newProcess.kill() == OS_SUCCESS)
                    cout << "Killed\n";
                else
                    cout << "ERROR: Could not kill process!\n";

                retval = OS_SUCCESS;
            }
        }
        else
            cout << "ERROR: Process says it's not running!\n";
    }
    else
    {
        cout << "ERROR: Could not create process " << appName.data() << endl;
    }

    cout << "DONE Process Class Method Test.\n";

    return retval;
}


OsStatus TestProcessMgr()
{
    OsStatus retval = OS_FAILED;
    OsProcessMgr processManager;

    //now lets try that with the ProcessMgr
    UtlString alias = "MyPing1";

    UtlString appName = "ping";
    UtlString params[10];
    params[0] = "127.0.0.1";
#ifdef _WIN32  //need to do this only on win32, linux already does this by default
    params[1] = "-t";
#endif

    OsPath inputFile = ""; //this means it will use standard input
    OsPath MyPing1OutputFile = "MyPing1Out.txt";
    OsPath MyPing2OutputFile = "MyPing2Out.txt";
    OsPath errFile = "pingerr.txt";

    processManager.setIORedirect(inputFile,MyPing1OutputFile,errFile);

    UtlString MyPing1("MyPing1");
    UtlString MyPing2("MyPing2");
    OsPath startupDir = ".";

    cout << "Starting process " << MyPing1.data() << endl;
    if (processManager.startProcess(MyPing1,appName,params,startupDir) == OS_SUCCESS)
    {
        if (processManager.getAliasState(MyPing1) == PROCESS_STARTED)
        {

            processManager.setIORedirect(inputFile,MyPing2OutputFile,errFile);
            cout << "Starting process " << MyPing2.data() << endl;
            if (processManager.startProcess(MyPing2,appName,params,startupDir) == OS_SUCCESS)
            {
                if (processManager.getAliasState(MyPing2) == PROCESS_STARTED)
                {

                        cout << "Waiting 5 secs before killing process MyPing1...\n";
                        OsTask::delay(5000);

                    if (processManager.stopProcess(MyPing1) != OS_SUCCESS)
                            cout << "ERROR:Couldn't kill MyPing1 process!\n";
                        else
                        {
                        cout << "Successfull take down of MyPing1 process!\n";
                        cout << "Waiting 5 secs before killing process MyPing2...\n";
                        OsTask::delay(5000);

                        alias = "MyPing2";
                        if (processManager.stopProcess(MyPing2) != OS_SUCCESS)
                        {
                            cout << "ERROR:Couldn't kill MyPing2 process!\n";
                        }
                        else
                        {
                            cout << "Successfull take down of MyPing2 process!\n";
                            retval = OS_SUCCESS;
                        }
                    }
                }
                else
                    cerr << "ERROR: process manager says process 2 is NOT running!\n";

            }
            else
                cerr << "ERROR: Unable to  start process 2 in TestProcessMgr\n";
        }
        else
            cerr << "ERROR: process manager says process 1 is NOT running!\n";
    }
    else
        cerr << "ERROR: Unable to start process 1 in TestProcessMgr\n";

    return retval;
}

/* ============================ OsLock Helper Functions =================== */

void guardedWithBSem(OsBSem& rBSem)
{
   OsLock lock(rBSem);

   // if this were a real guarded method, we'd do useful work here

   // destroying the OsLock variable that has been allocated on the stack
   // should release the lock automatically when we exit
}

/* ============================ OsReadLock Helper Functions =============== */

void guardedForReading(OsRWMutex& rRWMutex)
{
   OsReadLock lock(rRWMutex);

   // if this were a real guarded method, we'd do useful work here

   // destroying the OsReadLock variable that has been allocated on the stack
   // should release the reader lock automatically when we exit
}

/* ============================ OsWriteLock Helper Functions ============== */

void guardedForWriting(OsRWMutex& rRWMutex)
{
   OsWriteLock lock(rRWMutex);

   // if this were a real guarded method, we'd do useful work here    <enter description here>

   // destroying the OsWriteLock variable that has been allocated on the stack
   // should release the writer lock automatically when we exit
}

/* ============================ OsMsgQ Helper Functions =================== */

UtlBoolean msgSendHook(const OsMsg& rOsMsg)
{
   cout << "msgSendHook() called" << endl;

   return FALSE;
}

/* ============================ OsTimer Helper Functions ================== */

void handleTimerEvent(const int userData, const int eventData)
{
   cout << "handleTimerEvent called with userData = "
        << userData << ", eventData = " << eventData << endl;
   cout << "  timer fired at " << OsDateTime::getSecsSinceEpoch() <<
           " secs" << endl;
}

void handleRapidTimerEvent(const int userData, const int eventData)
{
   ++*(int *)userData;
}

/* ============================ End of Helper Functions =================== */

#if defined(_VXWORKS)
int ostestmain()
#else
int main()
#endif
{
#ifdef TEST
   UtlMemCheck* pMemCheck = 0;
   pMemCheck = new UtlMemCheck();      // checkpoint for memory leak check
#endif //TEST

   cout << "Entering main()" << endl;

/* ============================ Testing Start ============================= */

   /* ============================ OsProcess ==================================== */
#if defined(_WIN32) || defined(__pingtel_on_posix__)  //vxworks wont use these classes

   osPrintf("Starting Process test...\n");


    //Text Process Class
    if (TestProcessClass() != OS_SUCCESS)
        osPrintf("TestProcessterator FAILED!\n");

    //Test Process Iterator
    if (TestProcessIterator() != OS_SUCCESS)
        osPrintf("TestProcessterator FAILED!\n");

    //Test Process Manager methods
    if (TestProcessMgr() != OS_SUCCESS)
        osPrintf("TestProcessterator FAILED!\n");


    osPrintf("Finished Process test.\n");
#endif

/* ============================ OsTime ==================================== */

   OsTime* pTime;

   pTime = new OsTime();
   delete pTime;

/* ============================ OsMutex =================================== */

   OsMutex* pMutex;

   pMutex = new OsMutex(0);
   delete pMutex;

/* ============================ OsBSem ==================================== */

   OsBSem* pBSem;

   pBSem = new OsBSem(OsBSem::Q_PRIORITY, OsBSem::FULL);
   assert(pBSem->acquire()    == OS_SUCCESS);
   assert(pBSem->tryAcquire() == OS_BUSY);
   assert(pBSem->release()    == OS_SUCCESS);
   delete pBSem;

/* ============================ OsCSem ==================================== */

   OsCSem* pCSem;

   // the initial count on the semaphore will be 2
   pCSem = new OsCSem(OsCSem::Q_PRIORITY, 2);
   assert(pCSem->acquire()    == OS_SUCCESS);  // take it once
   assert(pCSem->acquire()    == OS_SUCCESS);  // take it twice
   assert(pCSem->tryAcquire() == OS_BUSY);
   assert(pCSem->release()    == OS_SUCCESS);  // release once
   assert(pCSem->release()    == OS_SUCCESS);  // release twice
   delete pCSem;

/* ============================ OsLock ==================================== */

   // Create a binary semaphore for use with an OsLock object
   pBSem = new OsBSem(OsBSem::Q_PRIORITY, OsBSem::FULL);

   // Acquire semaphore at the start of the method, release it on exit
   guardedWithBSem(*pBSem);
   delete pBSem;

/* ============================ OsRWMutex ================================= */

   OsRWMutex* pRWMutex;

   pRWMutex = new OsRWMutex(OsRWMutex::Q_FIFO);
   assert(pRWMutex->acquireRead()     == OS_SUCCESS);
   assert(pRWMutex->tryAcquireWrite() == OS_BUSY);
   assert(pRWMutex->releaseRead()     == OS_SUCCESS);
   assert(pRWMutex->tryAcquireWrite() == OS_SUCCESS);
   assert(pRWMutex->tryAcquireRead()  == OS_BUSY);
   assert(pRWMutex->releaseWrite()    == OS_SUCCESS);
   delete pRWMutex;

/* ============================ OsReadLock and OsWriteLock ================ */

   // Create an OsRWMutex for use with OsReadLock and OsWriteLock objects
   pRWMutex = new OsRWMutex(OsRWMutex::Q_FIFO);

   // Acquire read lock at the start of the method, release it on exit
   guardedForReading(*pRWMutex);

   // Acquire write lock at the start of the method, release it on exit
   guardedForWriting(*pRWMutex);
   delete pRWMutex;

/* ============================ OsNameDb ================================== */

   OsNameDb* pNameDb;

   int storedInt;

   pNameDb = OsNameDb::getNameDb();
   assert(pNameDb->isEmpty());
   assert(pNameDb->numEntries() == 0);

   assert(pNameDb->insert("test1", 1) == OS_SUCCESS);
   assert(pNameDb->insert("test1", 2) == OS_NAME_IN_USE);
   assert(!pNameDb->isEmpty());
   assert(pNameDb->numEntries() == 1);

   assert(pNameDb->insert("test2", 2) == OS_SUCCESS);
   assert(pNameDb->numEntries() == 2);

   assert(pNameDb->lookup("test1", NULL)       == OS_SUCCESS);
   assert(pNameDb->lookup("test1", &storedInt) == OS_SUCCESS);
   assert(storedInt == 1);
   assert(pNameDb->lookup("test2", &storedInt) == OS_SUCCESS);
   assert(storedInt == 2);
   assert(pNameDb->lookup("test3", NULL)       == OS_NOT_FOUND);

   pNameDb->remove("test1");
   pNameDb->remove("test2");

   delete pNameDb;

/* ============================ OsMsgQ ==================================== */

   OsMsgQ* pMsgQ1;
   OsMsg* pMsg1;
   OsMsg* pMsg2;
   OsMsg* pRecvMsg;

   pMsgQ1 = new OsMsgQ(OsMsgQ::DEF_MAX_MSGS, OsMsgQ::DEF_MAX_MSG_LEN,
                       OsMsgQ::Q_PRIORITY, "MQ1");

   pMsg1  = new OsMsg(OsMsg::UNSPECIFIED, 0);
   pMsg2  = new OsMsg(OsMsg::UNSPECIFIED, 0);

   assert(pMsgQ1->isEmpty());
   assert(pMsgQ1->numMsgs() == 0);
   assert(pMsgQ1->getSendHook() == NULL);
   pMsgQ1->setSendHook(msgSendHook);
   assert(pMsgQ1->getSendHook() == msgSendHook);

   OsStatus stat = pMsgQ1->send(*pMsg1);

   assert(stat == OS_SUCCESS);
   assert(!pMsgQ1->isEmpty());
   assert(pMsgQ1->numMsgs() == 1);

   stat = pMsgQ1->send(*pMsg2);
   assert(stat == OS_SUCCESS);

   assert(pMsgQ1->numMsgs() == 2);

   stat = pMsgQ1->receive(pRecvMsg);
   assert(stat == OS_SUCCESS);

   delete pRecvMsg;
   assert(pMsgQ1->numMsgs() == 1);

   stat = pMsgQ1->receive(pRecvMsg);
   assert(stat == OS_SUCCESS);
   delete pRecvMsg;

   assert(pMsgQ1->numMsgs() == 0);

   delete pMsg1;
   delete pMsg2;

#if !defined(_VXWORKS)
   try
   {
      OsMsgQ* pMsgQ2 = new OsMsgQ(OsMsgQ::DEF_MAX_MSGS,
                                  OsMsgQ::DEF_MAX_MSG_LEN,
                                  OsMsgQ::Q_PRIORITY,
                                  "MQ1");
      delete pMsgQ2;
   }
   catch (const OsExcept* exc)
   {

      UtlString txt;
      cout << "Exception:" << endl;
      cout << "  Major Code: " << exc->getMajorCode() << endl;
      cout << "  Minor Code: " << exc->getMinorCode() << endl;
      txt = exc->getText();
      cout << "  Text:       " << txt.data()    << endl;
      txt = exc->getContext();
      cout << "  Context:    " << txt.data() << endl;

      delete exc;
   }
#endif

   delete pMsgQ1;

/* ============================ OsCallback ================================ */

   OsCallback* pCallback;

   pCallback = new OsCallback(12345, handleTimerEvent);
   pCallback->signal(67890);
   delete pCallback;

/* ============================ OsEvent =================================== */

   OsTime   eventTimeout(2,0);
   OsEvent* pEvent;

   cout << "Testing OsEvent, please wait..." << endl;
   pEvent = new OsEvent(12345);
   int epochTime = time(NULL);
   assert(pEvent->wait(eventTimeout) != OS_SUCCESS);
   pEvent->signal(67890);
   assert(pEvent->wait(eventTimeout) == OS_SUCCESS);
   pEvent->reset();
   assert(pEvent->wait(eventTimeout) != OS_SUCCESS);
   epochTime = time(NULL) - epochTime;

   // Make sure we waited (approximately) 2 seconds each time.
   assert(epochTime > 2 && epochTime < 6);

   delete pEvent;
   cout << "Done testing OsEvent." << endl;

/* ============================ OsConfigDb ================================ */

   OsConfigDb* pConfigDb;

   pConfigDb = new OsConfigDb();
   delete pConfigDb;



/* ============================ OsTimer =================================== */

   OsTask::delay(500);    // wait 1/2 second
   OsCallback* pNotifier;
   OsCallback* pNotifier2;
   OsTimer*    pTimer;
   OsTimer*    pTimer2;
   OsTime      tenMsec(0, 10000);// timer offset ten msec into the future
   OsTime      oneSecond(1,0);   // timer offset one second into the future
   OsTime      twoSeconds(2,0);  // timer offset two seconds into the future
   OsTime      tenSeconds(10,0); // timer offset ten seconds into the future
   OsTime      tenYears(10*365*24*60*60, 0);  // ten years into the future

   cout << "About to handle timer 1 (immediate)" << endl;
   pNotifier = new OsCallback(1, handleTimerEvent);
   pTimer = new OsTimer(*pNotifier);
   cout << "  Oneshot timer 1 armed at " <<
           OsDateTime::getSecsSinceEpoch() << " secs" << endl;
   pTimer->oneshotAfter(OsTime::NO_WAIT);
   delete pTimer;
   delete pNotifier;

   cout << "About to handle timer 2" << endl;
   pNotifier = new OsCallback(2, handleTimerEvent);
   pTimer = new OsTimer(*pNotifier);
   cout << "  Oneshot timer 2 armed at " <<
           OsDateTime::getSecsSinceEpoch() << " secs" << endl;
   pTimer->oneshotAfter(oneSecond);
   delete pTimer;            // delete the timer before it can expire
   delete pNotifier;

   cout << "About to handle timer 3" << endl;
   pNotifier = new OsCallback(3, handleTimerEvent);
   pTimer = new OsTimer(*pNotifier);
   cout << "  Oneshot timer 3 armed at " <<
           OsDateTime::getSecsSinceEpoch() << " secs" << endl;
   pTimer->oneshotAfter(oneSecond);
   pTimer->stop();           // stop the timer before it can expire
   delete pTimer;
   delete pNotifier;

   cout << "About to handle timer 4 (after 1 sec)" << endl;
   pNotifier = new OsCallback(4, handleTimerEvent);
   pTimer = new OsTimer(*pNotifier);
   cout << "  Oneshot timer armed at " <<
           OsDateTime::getSecsSinceEpoch() << endl;
   pTimer->oneshotAfter(oneSecond);
   OsTask::delay(1500);  // sleep for 1.5 seconds
   delete pTimer;
   delete pNotifier;

   cout << "About to handle timer 5" << endl;
   pNotifier = new OsCallback(5, handleTimerEvent);
   pTimer = new OsTimer(*pNotifier);
   cout << "  Periodic timer 5 armed at " <<
           OsDateTime::getSecsSinceEpoch() << " secs" << endl;
   pTimer->periodicEvery(oneSecond, oneSecond);
   delete pTimer;            // delete the timer before it can expire
   delete pNotifier;

   cout << "About to handle timer 6" << endl;
   pNotifier = new OsCallback(6, handleTimerEvent);
   pTimer = new OsTimer(*pNotifier);
   cout << "  Periodic timer 6 armed at " <<
           OsDateTime::getSecsSinceEpoch() << " secs" << endl;
   pTimer->periodicEvery(oneSecond, oneSecond);
   pTimer->stop();           // stop the timer before it can expire
   delete pTimer;
   delete pNotifier;

   cout << "About to handle timer 7 (immediate, then every second)" << endl;
   cout << "About to handle timer 8 (immediate, then every two seconds)" << endl;
   pNotifier  = new OsCallback(7, handleTimerEvent);
   pNotifier2 = new OsCallback(8, handleTimerEvent);
   pTimer  = new OsTimer(*pNotifier);
   pTimer2 = new OsTimer(*pNotifier2);
   cout << "  Periodic timer 7 armed at " <<
           OsDateTime::getSecsSinceEpoch() << " secs" << endl;
   pTimer->periodicEvery(OsTime::NO_WAIT, oneSecond);
   cout << "  Periodic timer 8 armed at " <<
           OsDateTime::getSecsSinceEpoch() << " secs" << endl;
   pTimer2->periodicEvery(OsTime::NO_WAIT, twoSeconds);
   OsTask::delay(4500);  // sleep for 4.5 seconds
   pTimer->stop();
   pTimer2->stop();
   delete pTimer;
   delete pTimer2;
   delete pNotifier;
   delete pNotifier2;

   cout << "About to handle timer 9 (after ten seconds)" << endl;
   pNotifier = new OsCallback(9, handleTimerEvent);
   pTimer = new OsTimer(*pNotifier);
   cout << "  Oneshot timer 9 armed at " <<
           OsDateTime::getSecsSinceEpoch() << " secs" << endl;
   pTimer->oneshotAfter(tenSeconds);
   OsTask::delay(12000);  // sleep for 12 seconds
   delete pTimer;
   delete pNotifier;

   cout << "About to handle timer 10 (after 6912 seconds)" << endl;
   OsTime secs6912(6912, 0);
   pNotifier = new OsCallback(10, handleTimerEvent);
   pTimer = new OsTimer(*pNotifier);
   cout << "  Oneshot timer 10 armed at " <<
           OsDateTime::getSecsSinceEpoch() << " secs" << endl;
   pTimer->oneshotAfter(secs6912);
   OsTask::delay(12000);  // sleep for 12 seconds
   delete pTimer;
   delete pNotifier;

   cout << "Verify that we can schedule timers in the distant future" << endl;
   pNotifier = new OsCallback(11, handleTimerEvent);
   pTimer  = new OsTimer(*pNotifier);
   cout << "  Oneshot timer armed 11 at " <<
           OsDateTime::getSecsSinceEpoch() << " secs" << endl;
   pTimer->oneshotAfter(tenYears);
   pTimer->stop();
   pTimer->periodicEvery(OsTime::NO_WAIT, tenYears);
   pTimer->stop();
   delete pTimer;
   delete pNotifier;

   cout << "Run 100x/second timer for 2 seconds..." << endl;
   int rapidTimerExpirations = 0;
   pNotifier = new OsCallback(&rapidTimerExpirations, handleRapidTimerEvent);
   pTimer = new OsTimer(*pNotifier);
   cout << "  Rapid-fire timer armed at " <<
           OsDateTime::getSecsSinceEpoch() << " secs" << endl;
   pTimer->periodicEvery(OsTime::NO_WAIT, tenMsec);
   OsTask::delay(2000);
   pTimer->stop();
   cout << "  Rapid-fire timer stopped at " <<
           OsDateTime::getSecsSinceEpoch() << " secs" << endl;
   cout << "Rapid-fire timer expired " << rapidTimerExpirations << " times" << endl;
   delete pTimer;
   delete pNotifier;


/* ============================ OsNameDb cleanup ========================== */

   // The name database may have been created indirectly (as a result of
   // creating objects with global names). If the name database exists,
   // delete it now.
   pNameDb = OsNameDb::getNameDb();
   if (pNameDb != NULL)
      delete pNameDb;

/* ============================ Testing Finish ============================ */

   cout << "Leaving main()" << endl;

#ifdef TEST
   assert(pMemCheck->delta() == 0);    // check for memory leak
   delete pMemCheck;
#endif //TEST

   return 0;
}

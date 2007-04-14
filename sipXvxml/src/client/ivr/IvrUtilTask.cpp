// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#ifdef TEST
#include "utl/UtlMemCheck.h"
#endif /* TEST */

#ifdef __pingtel_on_posix__
#include <unistd.h>
#include <fcntl.h>
#endif

// APPLICATION INCLUDES
#include "IvrUtilTask.h"
#include "os/OsSysLogMsg.h"
#include "os/OsTimer.h"
#include "os/OsQueuedEvent.h"
#include "os/OsEventMsg.h"
#include "os/OsEvent.h"
#include "os/OsSysLogFacilities.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
IvrUtilTask* IvrUtilTask::spInstance = 0;
OsBSem  IvrUtilTask::sLock(OsBSem::Q_PRIORITY, OsBSem::FULL);

// LOCAL FUNCTIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Copy constructor
IvrUtilTask::IvrUtilTask(const IvrUtilTask& rIvrUtilTask)
   : mRWMutex(OsRWMutex::Q_PRIORITY)
{
}

// Destructor
IvrUtilTask::~IvrUtilTask()
{
   // Delete the timer.
   if (mpTimer != NULL)
   {
      mpTimer->stop() ;

      OsQueuedEvent* pEvent = (OsQueuedEvent*) mpTimer->getNotifier() ;
      if (pEvent != NULL)
         delete pEvent;
      pEvent = NULL;

      delete mpTimer ;
      mpTimer = NULL ;
   }

   // Close unbounded log if still open
   if (mpLogFileName)
   {
      delete[] mpLogFileName;
      mpLogFileName = NULL;
   }
}

/* ============================ MANIPULATORS ============================== */

IvrUtilTask* IvrUtilTask::getIvrUtilTask()
{
   UtlBoolean isStarted;

   // If the task object already exists, and the corresponding low-level task
   // has been started, then use it
   if (spInstance != NULL && spInstance->isStarted())
      return spInstance;

   // If the task does not yet exist or hasn't been started, then acquire
   // the lock to ensure that only one instance of the task is started
   sLock.acquire();
   if (spInstance == NULL)
   {
      spInstance = new IvrUtilTask();
   }

   isStarted = spInstance->isStarted();
   if (!isStarted)
   {
      isStarted = spInstance->start();
      assert(isStarted);
   }
   sLock.release();

#ifdef TEST
   if (!sIsTested)
   {
      sIsTested = true;
      spInstance->test();
   }
#endif //TEST

   return spInstance;
}



OsStatus IvrUtilTask::setLogHandle(VXIlogInterface *pLog, char* nfname, int logToStdout)
{
   mRWMutex.acquireWrite();

   mpLog = pLog;
   mLogToStdout = logToStdout;
   if (nfname && ((mpLogFileName && (strcmp(nfname, mpLogFileName) != 0)) || !mpLogFileName))
   {
      if (mpLogFileName)
         delete mpLogFileName;
      int len = strlen(nfname);
      mpLogFileName = new char[len + 1];
      strcpy(mpLogFileName, nfname);
      mpLogFileName[len] = 0;
   }

   setFlushPeriod(mLogFlushPeriod);
   mRWMutex.releaseWrite();
   return OS_SUCCESS;
}

OsStatus IvrUtilTask::setFlushPeriod(int seconds)
{
   OsSysLogMsg msg(OsSysLogMsg::SET_FLUSH_PERIOD, (void*)seconds) ;
   postMessage(msg);

   return OS_SUCCESS;
}


// Flushes the log
OsStatus IvrUtilTask::flushLog()
{   
   OsEvent flushSync ;

   OsSysLogMsg msg(OsSysLogMsg::FLUSH_LOG, (void*) &flushSync) ;
   postMessage(msg);

   flushSync.wait();

   return OS_SUCCESS;
}

// Assignment operator
IvrUtilTask& IvrUtilTask::operator=(const IvrUtilTask& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Constructor
IvrUtilTask::IvrUtilTask(const UtlString& name,
                         int periodSecs,
                         int maxRequestQMsgs)
   : OsServerTask(name, NULL, maxRequestQMsgs),
     mLogFlushPeriod(periodSecs),
     mpTimer(NULL),
     mRWMutex(OsRWMutex::Q_PRIORITY),
     mpLog(0),
     mpLogFileName(0),
     mLogToStdout(0)
{    
   start();

#ifdef TEST
   if (!sIsTested)
   {
      sIsTested = true;
      test();
   }
#endif /* TEST */
}

UtlBoolean IvrUtilTask::handleMessage(OsMsg& eventMessage)
{
   UtlBoolean rc = TRUE;
   OsSysLogMsg* pSysLogMsg = NULL;

   switch (eventMessage.getMsgType())
   {
   case OsMsg::OS_EVENT:
      switch (eventMessage.getMsgSubType())
      {
      case OsEventMsg::NOTIFY:
         processFlushLog(NULL);
         break;
      }
      break ;
   case OsMsg::OS_SYSLOG:
      pSysLogMsg = (OsSysLogMsg*) &eventMessage;
      switch (pSysLogMsg->getMsgSubType())
      {
      case OsSysLogMsg::SET_FLUSH_PERIOD:
         processSetFlushPeriod((int) pSysLogMsg->getData()) ;
         break ;
      case OsSysLogMsg::FLUSH_LOG:
         processFlushLog((OsEvent*) pSysLogMsg->getData());
         break ;
      default:
         break ;
      }
      break ;
   default:
      rc = OsServerTask::handleMessage(eventMessage);
      break ;
   }
   return rc ;
}

// Process a log flush request.
OsStatus IvrUtilTask::processFlushLog(OsEvent* pEvent)
{  
   mRWMutex.acquireWrite();
   if (mpLog && mpLogFileName)
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "IvrUtilTask: flushing log: %s\n", mpLogFileName);
      mpLog->Flush(mpLog, mpLogFileName, mLogToStdout);
   }
   mRWMutex.releaseWrite();

   // Signal passed event
   if (pEvent != NULL)
   {
      pEvent->signal(0);
   }

   return OS_SUCCESS;
}

OsStatus IvrUtilTask::processSetFlushPeriod(int seconds)
{
   mRWMutex.acquireWrite() ;

   OsStatus status = OS_SUCCESS ;

   mLogFlushPeriod = seconds ;

   if (seconds > 0)
   {
      // Enable a new timer / reschedule existing timer
      if (mpTimer == NULL)
      {
         OsQueuedEvent* pEvent = new OsQueuedEvent(*getMessageQueue(), 0);
         mpTimer = new OsTimer(*pEvent) ;
      }
      else
         mpTimer->stop() ;

      // Set time
      mpTimer->periodicEvery(OsTime(), OsTime(seconds, 0)) ;
   }
   else
   {
      // Disable the timer if scheduled.
      if (mpTimer != NULL)
      {
         mpTimer->stop() ;

         OsQueuedEvent* pEvent = (OsQueuedEvent*) mpTimer->getNotifier() ;
         if (pEvent != NULL)
            delete pEvent;
         pEvent = NULL;

         delete mpTimer ;
         mpTimer = NULL ;
      }
   }

   mRWMutex.releaseWrite() ;

   return status;
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

#ifdef TEST

// Set to true after the tests have been executed once
bool IvrUtilTask::sIsTested = false;

// Test this class by running all of its assertion tests
void IvrUtilTask::test()
{
   UtlMemCheck* pMemCheck = 0;
   pMemCheck = new UtlMemCheck();      // checkpoint for memory leak check

   testCreators();
   testManipulators();
   testAccessors();
   testInquiry();

   assert(pMemCheck->delta() == 0);    // check for memory leak
   delete pMemCheck;
}

// Test the creators (and destructor) methods for the class
void IvrUtilTask::testCreators()
{
   UtlMemCheck* pMemCheck  = 0;

   pMemCheck = new UtlMemCheck();      // checkpoint for memory leak check

   // test the default constructor (if implemented)
   // test the copy constructor (if implemented)
   // test other constructors (if implemented)
   //    if a constructor parameter is used to set information in an ancestor
   //       class, then verify it gets set correctly (i.e., via ancestor
   //       class accessor method.
   // test the destructor
   //    if the class contains member pointer variables, verify that the
   //    pointers are getting scrubbed.

   assert(pMemCheck->delta() == 0);    // check for memory leak
   delete pMemCheck;
}

// Test the manipulator methods
void IvrUtilTask::testManipulators()
{
   UtlMemCheck* pMemCheck  = 0;

   pMemCheck = new UtlMemCheck();      // checkpoint for memory leak check

   // test the assignment method (if implemented)
   // test the other manipulator methods for the class

   assert(pMemCheck->delta() == 0);    // check for memory leak
   delete pMemCheck;
}

// Test the accessor methods for the class
void IvrUtilTask::testAccessors()
{
   UtlMemCheck* pMemCheck  = 0;

   pMemCheck = new UtlMemCheck();      // checkpoint for memory leak check

   // body of the test goes here

   assert(pMemCheck->delta() == 0);    // check for memory leak
   delete pMemCheck;
}

// Test the inquiry methods for the class
void IvrUtilTask::testInquiry()
{
   UtlMemCheck* pMemCheck  = 0;

   pMemCheck = new UtlMemCheck();      // checkpoint for memory leak check

   // body of the test goes here

   assert(pMemCheck->delta() == 0);    // check for memory leak
   delete pMemCheck;
}

#endif /* TEST */

/* ============================ FUNCTIONS ================================= */

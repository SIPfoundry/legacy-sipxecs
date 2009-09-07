//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#ifdef __pingtel_on_posix__
#include <unistd.h>
#include <fcntl.h>
#endif

// APPLICATION INCLUDES
#include "os/OsSysLogTask.h"
#include "os/OsSysLogMsg.h"
#include "os/OsStatus.h"
#include "os/OsServerTask.h"
#include "os/OsDateTime.h"
#include "os/OsTimer.h"
#include "os/OsQueuedEvent.h"
#include "os/OsEventMsg.h"
#include "os/OsConnectionSocket.h"
#include "os/OsFS.h"
#include "os/OsEvent.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// LOCAL FUNCTIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsSysLogTask::OsSysLogTask(const int maxInMemoryLogEntries /* = 0 */,
                           const int options /* = OsSysLog:OPT_NONE */,
                           const char *taskName /* = "syslog"*/)
   : OsServerTask(taskName, NULL, 1000, 250)
   , mConsoleEnabled(FALSE)
   , mpRingBuffer(NULL)
   , mLogCount(0)
   , mRingBufferNext(0)
   , mpUnboundedLog(NULL)
   , mFlushPeriod(0)
   , mLogChanged(FALSE)
   , mpTimer(NULL)
   , mpCallback(NULL)
   , mRWMutex(OsRWMutex::Q_PRIORITY)
   , mpLastReopen()
{
   // Init Ring Buffer
   mRingBufferLength = maxInMemoryLogEntries;
   if (mRingBufferLength > 0)
   {
      mpRingBuffer = new char*[mRingBufferLength];
      for (int i=0; i<mRingBufferLength; i++)
      {
         mpRingBuffer[i] = NULL ;
      }
   }

   mOptions = options ;

   // Init Socket List
   for (int i=0; i<MAX_SOCKET_TARGETS; i++)
   {
      mpSockets[i] = NULL ;
   }

   start() ;
   syslog(FAC_LOG, PRI_NOTICE, "Logger Started") ;
}

// Destructor
OsSysLogTask::~OsSysLogTask()
{
   waitUntilShutDown();
   clear() ;

   // Free Ring Buffer
   if (mpRingBuffer != NULL)
   {
      delete mpRingBuffer ;
      mpRingBuffer = NULL ;
   }

   // Close unbounded log if still open
   if (mpUnboundedLog != NULL)
   {
      fclose(mpUnboundedLog) ;
      mpUnboundedLog = NULL ;
   }
}

/* ============================ MANIPULATORS ============================== */

// Clears the log
OsStatus OsSysLogTask::clear()
{
   mRWMutex.acquireWrite() ;

   // Free Data
   for (int i=0; i<mRingBufferLength; i++)
   {
      if (mpRingBuffer[i] != NULL)
      {
         free(mpRingBuffer[i]) ;
         mpRingBuffer[i] = NULL ;
      }
   }

   // Reset Counts
   mRingBufferNext = 0 ;
   mLogChanged = true ;
   mLogCount = 0 ;

   mRWMutex.releaseWrite() ;

   return OS_SUCCESS ;
}


// Flushes the log
OsStatus OsSysLogTask::flush(const OsTime& rTimeout)
{
   OsStatus rc = OS_UNSPECIFIED ;
   OsEvent flushSync ;

   OsSysLogMsg msg(OsSysLogMsg::FLUSH_LOG, (void*) &flushSync) ;
   postMessage(msg) ;

   rc = flushSync.wait(rTimeout) ;

   return rc ;
}




/* ============================ ACCESSORS ================================= */

// Get the max number of log entries
OsStatus OsSysLogTask::getMaxEntries(int& maxEntries)
{
   mRWMutex.acquireRead() ;

   maxEntries = mRingBufferLength ;

   mRWMutex.releaseRead() ;

   return OS_SUCCESS ;
}

// Get a log entried
OsStatus OsSysLogTask::getLogEntries(  const int maxEntries,
                                       char* entries[],
                                       int& actualEntries)
{
   mRWMutex.acquireRead() ;

   OsStatus status = OS_SUCCESS ;
   int iIndex ;
   actualEntries = maxEntries ;

   // Make sure the requested length is sane
   if (actualEntries > mLogCount)
      actualEntries = mLogCount;
   if (actualEntries > mRingBufferLength)
      actualEntries = mRingBufferLength;
   if (actualEntries < 0)
      actualEntries = 0 ;

   // Traverse ring buffer and copy entries
   for (int i=0; i<actualEntries; i++)
   {
      // Calculate index in ring buffer
      if (mLogCount < mRingBufferLength)
         iIndex = ((mRingBufferNext - mLogCount) + i) ;
      else
         iIndex = ((mRingBufferNext - mRingBufferLength) + i) ;
      while (iIndex < 0)
         iIndex += mRingBufferLength ;
      iIndex %= mRingBufferLength ;

      // Copy data
      if (mpRingBuffer[iIndex] != NULL)
      {
         entries[i] = strdup(mpRingBuffer[iIndex]) ;
      }
      else
      {
         entries[i] = NULL ;
      }
   }

   mRWMutex.releaseRead() ;

   return status ;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Assignment operator
OsSysLogTask&
OsSysLogTask::operator=(const OsSysLogTask& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

// Copy constructor
OsSysLogTask::OsSysLogTask(const OsSysLogTask& rOsSysLogTask)
   : mRWMutex(OsRWMutex::Q_PRIORITY)
{
}


// Receive incoming messages and redispatch to the corresponding
// processXXX handler.
UtlBoolean OsSysLogTask::handleMessage(OsMsg& eventMessage)
{
   UtlBoolean bRC = FALSE ;
   OsSysLogMsg* pSysLogMsg = NULL ;
   char* data ;

   switch (eventMessage.getMsgType())
   {
      case OsMsg::OS_EVENT:
         switch (eventMessage.getMsgSubType())
         {
            case OsEventMsg::NOTIFY:
               processFlushLog(NULL) ;
               break;
         default:
               break;
         }
         bRC = TRUE ;
         break ;
      case OsMsg::OS_SYSLOG:
         pSysLogMsg = (OsSysLogMsg*) &eventMessage;
         switch (pSysLogMsg->getMsgSubType())
         {
            case OsSysLogMsg::LOG:
               data = (char*) pSysLogMsg->getData();
               processAdd(data);
               mLogCount++;
               break ;
            case OsSysLogMsg::ENABLE_CONSOLE:
               processConsoleEnable(TRUE);
               break ;
            case OsSysLogMsg::DISABLE_CONSOLE:
               processConsoleEnable(FALSE);
               break ;
            case OsSysLogMsg::HEAD:
               processHeadCommand((int) (intptr_t) pSysLogMsg->getData());
               break ;
            case OsSysLogMsg::TAIL:
               processTailCommand((int) (intptr_t) pSysLogMsg->getData());
               break ;
            case OsSysLogMsg::ADD_SOCKET:
               data = (char*) pSysLogMsg->getData();
               if (data != NULL)
               {
                  processAddSocket(data) ;
                  free(data) ;
               }
               break ;
            case OsSysLogMsg::SET_FILE:
               data = (char*) pSysLogMsg->getData();
               processSetFile(data) ;
               if (data != NULL)
               {
                  free(data) ;
               }
               break ;
            case OsSysLogMsg::SET_CALLBACK:
               processSetCallback((OsSysLogCallback) pSysLogMsg->getData());
               break ;
            case OsSysLogMsg::SET_FLUSH_PERIOD:
               processSetFlushPeriod((int) (intptr_t) pSysLogMsg->getData()) ;
               break ;
            case OsSysLogMsg::FLUSH_LOG:
               processFlushLog((OsEvent*) pSysLogMsg->getData());
               break ;
            default:
               break ;
         }
         bRC = TRUE ;
         break ;
      default:
         bRC = OsServerTask::handleMessage(eventMessage);
         break ;
   }
   return bRC ;
}

// Handles adding a log entry to the tail of the list
OsStatus OsSysLogTask::processAddTail(char* pEntry)
{
   mRWMutex.acquireWrite() ;

   OsStatus status = OS_SUCCESS ;

   // Add to Circular Buffer
   if ((mpRingBuffer != NULL) && (mLogCount < mRingBufferLength))
   {
      int iIndex = (mRingBufferNext - (mLogCount + 1)) ;
      while (iIndex < 0)
         iIndex += mRingBufferLength ;
      iIndex %= mRingBufferLength ;

      if (mpRingBuffer[iIndex] != NULL)
      {
         free(mpRingBuffer[iIndex]) ;
      }
      mpRingBuffer[iIndex] = pEntry ;

      mLogCount++ ;
   }
   else
      status = OS_LIMIT_REACHED ;

   mRWMutex.releaseWrite() ;

   return status ;
}


// Process a log addition
OsStatus OsSysLogTask::processAdd(char* pEntry)
{
   OsStatus status = OS_SUCCESS ;

   mRWMutex.acquireWrite() ;

   /*
    * Display to the console if enabled
    */
   if (mConsoleEnabled)
   {
      osPrintf("%s\n", pEntry) ;
   }


   /*
    * If we have a ring buffer initialized, add to it.
    */
   if (mpRingBuffer != NULL)
   {
      if (mpRingBuffer[mRingBufferNext] != NULL)
      {
         free(mpRingBuffer[mRingBufferNext]) ;
      }
      mpRingBuffer[mRingBufferNext] = pEntry ;
      mRingBufferNext = (mRingBufferNext + 1) % mRingBufferLength ;
   }

   if (mOptions & OsSysLog::OPT_SHARED_LOGFILE)
   {
      if (mUnboundedLogFile.length())
      {
         if ((mpUnboundedLog = fopen(mUnboundedLogFile.data(), "a+")) == NULL)
         {
            syslog(FAC_LOG, PRI_ERR, "Error reopening logfile %s", mUnboundedLogFile.data());
         }
#ifdef __pingtel_on_posix__
         else
         {
            int fd = fileno(mpUnboundedLog);
            fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);
         }
#endif
         // Keep track of the last reopen
         OsDateTime::getCurTimeSinceBoot(mpLastReopen) ;
      }

      if (mpUnboundedLog)
      {
         fprintf(mpUnboundedLog, "%s\n", pEntry) ;
         fclose(mpUnboundedLog);
         mpUnboundedLog = NULL ;
      }
   }
   else
   {
      /*
       * If an unbounded log is initialized, add to it
       */
      if (mpUnboundedLog)
      {
         // Decide if we should close/reopen the log
         OsTime now ;
         OsTime reopenAfter ;

         OsDateTime::getCurTimeSinceBoot(now) ;
         reopenAfter = mpLastReopen + OsTime(MAX_REOPEN_LOG_DELAY_SEC, 0) ;

         if (now > reopenAfter)
         {
            // Close Log
            fclose(mpUnboundedLog) ;
            mpUnboundedLog = NULL ;

            // Reopen Log
            if ((mpUnboundedLog = fopen(mUnboundedLogFile.data(), "a+")) == NULL)
            {
               syslog(FAC_LOG, PRI_ERR, "Error reopening logfile %s", mUnboundedLogFile.data());
            }
#ifdef __pingtel_on_posix__
            else
            {
               int fd = fileno(mpUnboundedLog);
               fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);
            }
#endif
             // Keep track of the last reopen
             OsDateTime::getCurTimeSinceBoot(mpLastReopen) ;
          }

          if (mpUnboundedLog)
          {
             fprintf(mpUnboundedLog, "%s\n", pEntry) ;
             fflush(mpUnboundedLog);
          }
      }
   }


   /*
    * If we have been initialized with target sockets- fire off events to
    * interested parties.
    */
   for (int i=0; i<MAX_SOCKET_TARGETS; i++)
   {
      if (mpSockets[i] != NULL)
      {
         if (mpSockets[i]->write(pEntry, strlen(pEntry), 0) <= 0)
         {
            UtlString hostName;
            mpSockets[i]->getRemoteHostName(&hostName);
            syslog(FAC_LOG, PRI_ERR, "unable to write to socket, closing: %s", hostName.data());

            delete mpSockets[i] ;
            mpSockets[i] = NULL ;
         }
         else
            mpSockets[i]->write("\n", 1, 0) ;
      }
   }

   /*
    * If a callback funtion was registered, call and hand over the log entry
    */
   if ( mpCallback != NULL )
   {
      UtlString date;
      UtlString eventCount;
      UtlString facility;
      UtlString priority;
      UtlString hostname;
      UtlString taskname;
      UtlString taskId;
      UtlString processId;
      UtlString content;

      // Parse the log entry to extract the priority
      OsSysLog::parseLogString(pEntry, date, eventCount, facility, priority,
                               hostname, taskname, taskId, processId,
                               content);

      mpCallback(priority, "SIPxua", pEntry);
   }


   /*
    * Clean up the entry of no ring buffer was installed.  Otherwise, the
    * ring buffer is responsible for clean up.
    */
   if (!mpRingBuffer)
      free(pEntry);

   // Mark the log as changed
   mLogChanged = true ;

   mRWMutex.releaseWrite() ;

   return status ;
}


// Process a console enable/disable command
OsStatus OsSysLogTask::processConsoleEnable(const UtlBoolean enable)
{
   mRWMutex.acquireRead() ;

   OsStatus status = OS_SUCCESS ;

   mConsoleEnabled = enable ;
   syslog(FAC_LOG, PRI_INFO, "console logging enabled: %s", enable ? "true" : "false") ;

   mRWMutex.releaseRead() ;

   return status ;
}


// Process a head command (displays start of in-memory log)
OsStatus OsSysLogTask::processHeadCommand(const int iEntries)
{
   mRWMutex.acquireRead() ;

   OsStatus status = OS_SUCCESS ;
   int iIndex ;
   int iCount = iEntries ;

   // Make sure the requested length is sane
   if (iCount > mLogCount)
      iCount = mLogCount;
   if (iCount > mRingBufferLength)
      iCount = mRingBufferLength;
   if (iCount <= 0)
      iCount = 16 ;

   // Traverse ring buffer and display entries
   for (int i=0; i<iCount; i++)
   {
      if (mLogCount < mRingBufferLength)
         iIndex = ((mRingBufferNext - mLogCount) + i) ;
      else
         iIndex = ((mRingBufferNext - mRingBufferLength) + i) ;
      while (iIndex < 0)
         iIndex += mRingBufferLength ;
      iIndex %= mRingBufferLength ;

      if (mpRingBuffer[iIndex] != NULL)
      {
         osPrintf("%s\n", mpRingBuffer[iIndex]) ;
      }
   }

   mRWMutex.releaseRead() ;

   return status ;
}


// Process a tail command (displays end of in-memory log)
OsStatus OsSysLogTask::processTailCommand(const int iEntries)
{
   mRWMutex.acquireRead() ;

   OsStatus status = OS_SUCCESS ;
   int iIndex ;
   int iCount = iEntries ;

   // Make sure the requested length is sane
   if (iCount > mLogCount)
      iCount = mLogCount;
   if (iCount > mRingBufferLength)
      iCount = mRingBufferLength;
   if (iCount <= 0)
      iCount = 16 ;

   // Traverse ring buffer and display entries
   for (int i=0; i<iCount; i++)
   {
      iIndex = ((mRingBufferNext - iCount) + i) ;
      while (iIndex < 0)
         iIndex += mRingBufferLength ;
      iIndex %= mRingBufferLength ;

      if (mpRingBuffer[iIndex] != NULL)
      {
         osPrintf("%s\n", mpRingBuffer[iIndex]) ;
      }
   }

   mRWMutex.releaseRead() ;

   return status ;
}


// Process the setting of an outbound file.  The flush period
// *MUST* have been set first.
OsStatus OsSysLogTask::processSetFile(const char* szFile)
{
   mRWMutex.acquireWrite() ;

   OsStatus status = OS_SUCCESS ;

   syslog(FAC_LOG, PRI_INFO, "Setting file output %s", szFile ? szFile : "<BLANK>");

   // Clean up if a log files if already in use
   if (mpUnboundedLog != NULL)
   {
      syslog(FAC_LOG, PRI_INFO, "Closing unbounded logfile %s",
            mUnboundedLogFile.data());
      fclose(mpUnboundedLog) ;
      mUnboundedLogFile.remove(0) ;
   }
   mBoundedLogFile.remove(0) ;

   if (szFile)
   {
      // If the flush period is 0, open a unbounded log, otherwise, bounded log
      // files are opened/closed on flushes.
      if (mFlushPeriod == 0)
      {
         // Open the log file, if not asked to use a shared logfile.
         if (!(mOptions & OsSysLog::OPT_SHARED_LOGFILE))
         {
             // Open a unbounded Log
             if ((mpUnboundedLog = fopen(szFile, "a+")) == NULL)
             {
                syslog(FAC_LOG, PRI_ERR, "Error opening logfile %s", szFile);
             }
             else
             {
#ifdef __pingtel_on_posix__
                int fd = fileno(mpUnboundedLog);
                fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);
#endif
             }
         }
         mUnboundedLogFile = szFile ;

         OsDateTime::getCurTimeSinceBoot(mpLastReopen) ;
         mRWMutex.releaseWrite() ;
      }
      else
      {
         mBoundedLogFile = szFile ;

         mRWMutex.releaseWrite() ;

         // Populate
         OsFile file(mBoundedLogFile.data()) ;
         if (file.open(OsFile::READ_ONLY) == OS_SUCCESS)
         {
            UtlString line ;
            while (file.readLine(line) == OS_SUCCESS)
            {
               if (processAddTail(strdup(line.data())) != OS_SUCCESS)
                  break ;
            }
         }
         file.close() ;
      }
   }
   else
   {
       mRWMutex.releaseWrite();
   }

   return status;
}


// Process the setting of a flush period.  This should be performed
// before processSetFile.
OsStatus OsSysLogTask::processSetFlushPeriod(const int iSeconds)
{
   mRWMutex.acquireWrite() ;

   OsStatus status = OS_SUCCESS ;

   mFlushPeriod = iSeconds ;
   syslog(FAC_LOG, PRI_INFO, "file flush period set to %d", iSeconds) ;

   if (iSeconds > 0)
   {
      // Enable a new timer / reschedule existing timer
      if (mpTimer == NULL)
      {
         mpTimer = new OsTimer(getMessageQueue(), 0) ;
      }
      else
         mpTimer->stop() ;

      // Set time
      mpTimer->periodicEvery(OsTime(), OsTime(iSeconds, 0)) ;
   }
   else
   {
      // Disable the timer if scheduled.
      if (mpTimer != NULL)
      {
         mpTimer->stop() ;

         delete mpTimer ;
         mpTimer = NULL ;
      }
   }

   mRWMutex.releaseWrite() ;

   return status;
}


// Process the addition of an output socket
OsStatus OsSysLogTask::processAddSocket(const char* remoteHost)
{
   mRWMutex.acquireWrite() ;

   OsStatus status = OS_SUCCESS;
   UtlBoolean bFoundSpace = FALSE;
   char* host = strdup(remoteHost);
   int port = PORT_NONE;

   // Parse remoteHost into a host and port
   char* portStart = strrchr(host, (int) ':');
   if ((portStart != NULL) && (strlen(portStart) > 1))
   {
       port = atoi((char*) &portStart[1]);
       *portStart = 0;
   }
   else
   {
      syslog(FAC_LOG, PRI_ERR, "output socket missing port %s", host) ;
      status = OS_INVALID_ARGUMENT ;
   }

   // If we parsed correctly, go ahead and construct the socket
   if (status == OS_SUCCESS)
   {
      for (int i=0; i<MAX_SOCKET_TARGETS; i++)
      {
         if (mpSockets[i] == NULL)
         {
            bFoundSpace = TRUE ;

            OsConnectionSocket* pSocket = new OsConnectionSocket(port, host) ;
            mpSockets[i] = pSocket ;

            syslog(FAC_LOG, PRI_ERR, "added output socket (host=%s port=%d)", host, port) ;
         }
      }
   }

   if (bFoundSpace == FALSE)
      syslog(FAC_LOG, PRI_ERR, "failed to add output socket to %s: reached max sockets (%d)", remoteHost, MAX_SOCKET_TARGETS) ;


   mRWMutex.releaseWrite() ;

   return status;
}


// Process a log flush request.
OsStatus OsSysLogTask::processFlushLog(OsEvent* pEvent)
{
   OsStatus status = OS_SUCCESS ;

   mRWMutex.acquireWrite() ;
   if (!mUnboundedLogFile.isNull())
   {
      if (mpUnboundedLog != NULL)
      {
         fflush(mpUnboundedLog) ;
      }
   }
   else
   {
      if (mLogChanged)
      {
         if (!mBoundedLogFile.isNull())
         {
            OsFile file(mBoundedLogFile.data()) ;

            // Open the new log
            if (file.open(OsFile::WRITE_ONLY | OsFile::TRUNCATE | OsFile::CREATE) == OS_SUCCESS)
            {
               int iIndex ;
               int iCount = mLogCount ;

               if (iCount > mRingBufferLength)
                  iCount = mRingBufferLength;

               // Traverse ring buffer and output entries
               for (int i=0; i<iCount; i++)
               {
                  if (mLogCount < mRingBufferLength)
                     iIndex = ((mRingBufferNext - mLogCount) + i) ;
                  else
                     iIndex = ((mRingBufferNext - mRingBufferLength) + i) ;
                  while (iIndex < 0)
                     iIndex += mRingBufferLength ;
                  iIndex %= mRingBufferLength ;

                  if (mpRingBuffer[iIndex] != NULL)
                  {
                     size_t ulBytesWritten ;
                     OsStatus status ;

                     status = file.write(mpRingBuffer[iIndex],
                           strlen(mpRingBuffer[iIndex]), ulBytesWritten) ;

                     if (status == OS_SUCCESS)
                        status = file.write("\r\n", 2, ulBytesWritten) ;

                     if (status != OS_SUCCESS)
                     {
                        syslog(FAC_LOG, PRI_ERR, "Error writting to logfile %s", mBoundedLogFile.data());
                     }
                  }
               }
               file.close() ;
            }
            else
               syslog(FAC_LOG, PRI_ERR, "Error opening logfile %s", mBoundedLogFile.data());
         }
      }
      mLogChanged = false ;
   }

   mRWMutex.releaseWrite() ;

   // Signal passed event
   if (pEvent != NULL)
   {
      pEvent->signal(0) ;
   }

   return status;
}

OsStatus OsSysLogTask::processSetCallback(OsSysLogCallback fn)
{
   OsStatus status = OS_SUCCESS;

   mRWMutex.acquireWrite() ;

   mpCallback = fn;

   mRWMutex.releaseWrite() ;

   return status;
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

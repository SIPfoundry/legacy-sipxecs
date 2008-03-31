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
#include <errno.h>
#include <poll.h>
#include <unistd.h>

// APPLICATION INCLUDES
#include "os/OsServerTaskWaitable.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsServerTaskWaitable::OsServerTaskWaitable(const UtlString& name,
                                           void* pArg,
                                           const int maxRequestQMsgs,
                                           const int priority,
                                           const int options,
                                           const int stackSize) :
   OsServerTask(name, pArg, maxRequestQMsgs, priority, options, stackSize),
   mPipeReadingFd(-1),          // Initialize to invalid state.
   mPipeWritingFd(-1)
{
   // Create the pipe which is used to signal that a message is available.
   int filedes[2];
   int ret = pipe(filedes);
   if (ret == 0)
   {
      mPipeReadingFd = filedes[0];
      mPipeWritingFd = filedes[1];
      OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                    "OsServerTaskWaitable::_ pipe() opened %d -> %d",
                    mPipeWritingFd, mPipeReadingFd);
   }
   else
   {
      mPipeReadingFd = -1;
      mPipeWritingFd = -1;
      OsSysLog::add(FAC_KERNEL, PRI_ERR,
                    "OsServerTaskWaitable::_ pipe() returned %d, errno = %d, getdtablesize() = %d",
                    ret, errno, getdtablesize());
   }
}

// Destructor
OsServerTaskWaitable::~OsServerTaskWaitable()
{
   if (mPipeReadingFd != -1)
   {
      // Close the pipe.
      close(mPipeReadingFd);
      close(mPipeWritingFd);
      OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                    "OsServerTaskWaitable::~ closed %d -> %d",
                    mPipeWritingFd, mPipeReadingFd);
   }
}

/* ============================ MANIPULATORS ============================== */

// Post a message to this task.
OsStatus OsServerTaskWaitable::postMessage(const OsMsg& rMsg, const OsTime& rTimeout,
                                           UtlBoolean sentFromISR)
{
   // Post the message into the message queue.
   OsStatus res = OsServerTask::postMessage(rMsg, rTimeout, sentFromISR);

   // If the post succeeded, write a byte into the pipe (which we assume does
   // not block).
   if (res == OS_SUCCESS)
   {
      assert(write(mPipeWritingFd, &res /* arbitrary */, 1) == 1);
   }

   return res;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/** Return the file descriptor which is ready-to-read when a message is
 *  is in the queue.
 */
int OsServerTaskWaitable::getFd(void) const
{
   return mPipeReadingFd;
}

/** Return TRUE if creating the object has succeeded and it can be used.
 */
UtlBoolean OsServerTaskWaitable::isOk(void) const
{
   return mPipeReadingFd != -1;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// The entry point for the task.
// This method executes a message processing loop until either
// requestShutdown(), deleteForce(), or the destructor for this object
// is called.
int OsServerTaskWaitable::run(void* pArg)
{
   OsMsg*    pMsg = NULL;
   OsStatus  res;
   struct pollfd fds[1];
   fds[0].fd = mPipeReadingFd;
   fds[0].events = POLLIN;

   do
   {
      // Wait for the pipe to become ready to read.
      assert(poll(&fds[0], 1, -1 /* block forever */) >= 0);

      // Check that poll finished because the pipe is ready to read.
      if ((fds[0].revents & POLLIN) != 0)
      {
         res = receiveMessage((OsMsg*&) pMsg, OsTime::NO_WAIT); // receive the message
         assert(res == OS_SUCCESS);

         char buffer[1];
         assert(read(mPipeReadingFd, &buffer, 1) == 1); // read 1 byte from the pipe

         if (!handleMessage(*pMsg))                  // process the message
         {
            OsServerTask::handleMessage(*pMsg);
         }

         if (!pMsg->getSentFromISR())
         {
            pMsg->releaseMsg();                         // free the message
         }
      }
   }
   while (isStarted());

   return 0;        // and then exit
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

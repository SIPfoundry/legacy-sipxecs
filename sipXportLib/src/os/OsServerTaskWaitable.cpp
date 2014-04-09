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

// Limit on the fd's to allow to be allocated for a pipe().
// Approximately this number of fd's will always be available for
// other purposes.
#define FD_HEADROOM 20

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
   //
   // The limit we allow for fd's returned by pipe().
   // fdLimit allows us to enforce some headroom in fd allocation
   // between the fd's assigned by SipClient's and getdtablesize().
   // OsResourceLimit can change the size of allowable fd's so we need to
   // compute it here as opposed to computing it as a static variable.
   //
   int fdSetSize = getdtablesize() - FD_HEADROOM;

   // Create the pipe which is used to signal that a message is available.
   int filedes[2];
   int ret = pipe(filedes);
   // Check for success.
   if (ret == 0)
   {
      // Check if fd's are too large.
      if (filedes[0] <= fdSetSize && filedes[1] <= fdSetSize)
      {
         // Everything is OK.
         mPipeReadingFd = filedes[0];
         mPipeWritingFd = filedes[1];
         Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG,
                       "OsServerTaskWaitable::_ pipe() opened %d -> %d.  getdtablesize() = %d",
                       mPipeWritingFd, mPipeReadingFd, fdSetSize + FD_HEADROOM);
      }
      else
      {
         // fd's are too large and are encroaching on the fd 'headroom'.
         close(filedes[0]);
         close(filedes[1]);
         mPipeReadingFd = -1;
         mPipeWritingFd = -1;
         Os::Logger::instance().log(FAC_KERNEL, PRI_ERR,
                       "OsServerTaskWaitable::_ "
                       "pipe() returned %d -> %d, which exceeds fdSetSize = %d",
                       filedes[1], filedes[0], fdSetSize);
      }
   }
   else
   {
      mPipeReadingFd = -1;
      mPipeWritingFd = -1;
      Os::Logger::instance().log(FAC_KERNEL, PRI_ERR,
                    "OsServerTaskWaitable::_ "
                    "pipe() returned %d, errno = %d, getdtablesize() = %d",
                    ret, errno, fdSetSize + FD_HEADROOM);
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
      Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG,
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
         // Check to see how many messages are in the queue.
         // We need to process all of them before the next poll request.
         int numberMsgs = (getMessageQueue())->numMsgs();
         int i;
         char buffer[1];

         for (i=0;i<numberMsgs;i++)
         {
            res = receiveMessage((OsMsg*&) pMsg, OsTime::NO_WAIT); // receive the message
            assert(res == OS_SUCCESS);

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
   }
   while (isStarted());

   return 0;        // and then exit
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

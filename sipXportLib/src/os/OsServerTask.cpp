//
// Copyright (C) 2009 Nortel Networks, certain elements licensed under a Contributor Agreement.
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

// APPLICATION INCLUDES
#include "os/OsServerTask.h"
#include "os/OsMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const int OsServerTask::DEF_MAX_MSGS = OsMsgQ::DEF_MAX_MSGS;

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsServerTask::OsServerTask(const char* name,
                           void* pArg,
                           const int maxRequestQMsgs,
                           const int priority,
                           const int options,
                           const int stackSize)
:  OsTask(name, pArg, priority, options, stackSize),
   // Give the name of the task to the queue.  This makes it possible to
   // log the name of the task that owns a queue that has filled.
   // Note that if 'name' contains '%d', then the OsTask's name is different
   // from 'name', so we have to fetch the real task name from the OsTask.
   mIncomingQ(getName(),
              maxRequestQMsgs, OsMsgQ::DEF_MAX_MSG_LEN, OsMsgQ::Q_PRIORITY)
   // Other than initialization, no work is required.
{
   OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                 "OsServerTask::_ '%s' queue: %p queue limit: %d",
                 mName.data(), &mIncomingQ, maxRequestQMsgs);
}

// Destructor
// As part of destroying the task, flush all messages from the incoming
// OsMsgQ.
OsServerTask::~OsServerTask()
{
   OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "OsServerTask::~ '%s' %s",
                 mName.data(), TaskStateName(mState));

   waitUntilShutDown(20 * OsTime::MSECS_PER_SEC);
   mIncomingQ.flush();    // dispose of any messages in the request queue
}

/* ============================ MANIPULATORS ============================== */

// Handle an incoming message.
// This is the message handler of last resort. It should only be called when
// the handleMessage() method in the derived class returns FALSE (indicating
// that the message has not been handled.
UtlBoolean OsServerTask::handleMessage(OsMsg& rMsg)
{
   UtlBoolean handled;

   handled = FALSE;

   switch (rMsg.getMsgType())
   {
   case OsMsg::OS_SHUTDOWN:
      OsTask::requestShutdown();
      handled = TRUE;
      OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                    "OsServerTask::handleMessage: "
                    "OS_SHUTDOWN processed");
      break;
   default:
      OsSysLog::add(FAC_KERNEL, PRI_CRIT,
                    "OsServerTask::handleMessage: "
                    "'%s' unhandled message type %d.%d",
                    mName.data(), rMsg.getMsgType(), rMsg.getMsgSubType());
      // Consider doing "assert(FALSE);" here, as this situation never seems
      // to happen in normal operation.
      break;
   }

   return handled;
}

// Post a message to this task.
// Return the result of the message send operation.
OsStatus OsServerTask::postMessage(const OsMsg& rMsg,
                                   const OsTime& rTimeout,
                                   UtlBoolean sentFromISR)
{
   OsStatus res;

   if (sentFromISR)
      res = mIncomingQ.sendFromISR(rMsg);
   else
      res = mIncomingQ.send(rMsg, rTimeout);
   return res;
}

// Post a message to this task.
// Return the result of the message send operation.
OsStatus OsServerTask::postMessageP(OsMsg* pMsg,
                                    const OsTime& rTimeout)
{
   OsStatus res = mIncomingQ.sendP(pMsg, rTimeout);

   return res;
}

// Call OsTask::requestShutdown() and then post an OS_SHUTDOWN message
// to the incoming message queue to unblock the task.
void OsServerTask::requestShutdown(void)
{
   // Don't need to initiate shutdown if the task is already shutting down
   // or already shut down.
   if (isNotShut())
   {
      OsMsg msg(OsMsg::OS_SHUTDOWN, 0);

      OsTask::requestShutdown(); // causes isShuttingDown to return TRUE
      postMessage(msg); // wake up the task run loop
   }
}

/* ============================ ACCESSORS ================================= */

// Get the pointer to the incoming message queue
OsMsgQ* OsServerTask::getMessageQueue()
{
    return(&mIncomingQ);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Waits for a message to arrive on the task's incoming message queue.
OsStatus OsServerTask::receiveMessage(OsMsg*& rpMsg)
{
   return mIncomingQ.receive(rpMsg);
}

// Waits for a message to arrive on the task's incoming message queue.
OsStatus OsServerTask::receiveMessage(OsMsg*& rpMsg,
                                      const OsTime& rTimeout)
{
   return mIncomingQ.receive(rpMsg, rTimeout);
}

// The entry point for the task.
// This method executes a message processing loop until either
// requestShutdown(), deleteForce(), or the destructor for this object
// is called.
int OsServerTask::run(void* pArg)
{
   OsMsg*    pMsg = NULL;
   OsStatus  res;

   do
   {
      res = receiveMessage((OsMsg*&) pMsg);          // wait for a message
      assert(res == OS_SUCCESS);

      if (!handleMessage(*pMsg))                  // process the message
      {
         OsServerTask::handleMessage(*pMsg);
      }

      if (!pMsg->getSentFromISR())
      {
         pMsg->releaseMsg();                         // free the message
      }
   }
   while (isStarted());

   return 0;        // and then exit
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

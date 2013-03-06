//
// Copyright (C) 2009 Nortel Networks, certain elements licensed under a Contributor Agreement.
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsLock.h"
#include "os/shared/OsMsgQShared.h"
#include "os/OsDateTime.h"
#include "os/OsLogger.h"

static OsMsgQShared::QueuePreference gQueuePreference = OsMsgQShared::QUEUE_LIMITED;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

#include "utl/Instrumentation.h"


// Constructor
// If the name is specified but is already in use, throw an exception
OsMsgQShared::OsMsgQShared(const char* name,
                           int maxMsgs,
                           int maxMsgLen,
                           int options,
                           bool reportFull)
   : OsMsgQBase(name),
     _maxMsgLen(maxMsgLen),
     _options(options),
     _reportFull(reportFull),
     _queuePreference(gQueuePreference)
{
    mMaxMsgs = maxMsgs;

    if (OsMsgQShared::QUEUE_LIMITED == _queuePreference)
    {
        _empty = new Semaphore(mMaxMsgs, mMaxMsgs);
        _full = new Semaphore(mMaxMsgs, 0);
    }
    else
    {
        _empty = NULL;
        _full = new Semaphore();
    }
}

// Destructor
OsMsgQShared::~OsMsgQShared()
{
    if (numMsgs())
        flush();    // get rid of any messages in the queue

    if (_empty)
    {
        delete _empty;
        _empty = NULL;
    }

    if (_full)
    {
        delete _full;
        _full = NULL;
    }
}

/* ============================ MANIPULATORS ============================== */

// Insert a message at the tail of the queue
// Wait until there is either room on the queue or the timeout expires.
OsStatus OsMsgQShared::send(const OsMsg& rMsg,
                            const OsTime& rTimeout)
{
   return doSend(rMsg, rTimeout, FALSE, FALSE);
}


// Insert a message at the tail of the queue
// Wait until there is either room on the queue or the timeout expires.
OsStatus OsMsgQShared::sendP(OsMsg* pMsg,
                             const OsTime& rTimeout)
{
   pMsg->setSentFromISR(FALSE);  // set flag in the msg to indicate
                                 //  whether sent from an ISR

   OsStatus ret = doSendCore(pMsg, rTimeout, FALSE, TRUE);

   return ret;
}


// Insert a message at the tail of the queue.
// Sending from an ISR has a couple of implications.  Since we can't
// allocate memory within an ISR, we don't create a copy of the message
// before sending it and the sender and receiver need to agree on a
// protocol (outside this class) for when the message can be freed.
// The sentFromISR flag in the OsMsg object will be TRUE for messages
// sent using this method.
OsStatus OsMsgQShared::sendFromISR(const OsMsg& rMsg)
{
   return doSend(rMsg, OsTime::NO_WAIT, FALSE, TRUE);
}

// Remove a message from the head of the queue
// Wait until either a message arrives or the timeout expires.
// The receiver is responsible for freeing the received message.
// Other than for messages sent from an ISR, the receiver is responsible
// for freeing the received message.
OsStatus OsMsgQShared::receive(OsMsg*& rpMsg, const OsTime& rTimeout)
{
   return doReceive(rpMsg, rTimeout);
}


/* ============================ ACCESSORS ================================= */

// Return the number of messages in the queue
int OsMsgQShared::numMsgs(void)
{
   mutex_critic_sec_lock lock(_cs);
   return(_queue.size());
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Send a message which may be reusable, or may need to be copied first.
OsStatus OsMsgQShared::doSend(const OsMsg& rMsg,
                              const OsTime& rTimeout,
                              const UtlBoolean isUrgent,
                              const UtlBoolean sendFromISR)
{
   OsMsg*     pMsg;
   UtlBoolean copy = !(sendFromISR || rMsg.isMsgReusable());

   if (copy)
   {
      pMsg = rMsg.createCopy();      // we place a copy of the message on the
				     //  queue so that the caller is free to
				     //  destroy the original
   }
   else
   {
      // Remove the const from &rMsg.
      // (The const was just an efficiency hack anyway.)
      pMsg = const_cast <OsMsg*> (&rMsg);
   }

   pMsg->setSentFromISR(sendFromISR);// set flag in the msg to indicate
				     //  whether sent from an ISR

   OsStatus ret = doSendCore(pMsg, rTimeout, isUrgent, copy);

   return ret;
}

// Core send message logic.
OsStatus OsMsgQShared::doSendCore(OsMsg* pMsg,
                                  const OsTime& rTimeout,
                                  UtlBoolean isUrgent,
                                  UtlBoolean deleteWhenDone)
{
   OsStatus ret = OS_SUCCESS;

   if (mSendHookFunc != NULL)
   {
      if (mSendHookFunc(*pMsg))
      {
         // by returning TRUE, the mSendHookFunc indicates that it has handled
         // the message and there is no need to queue the message.
         if (deleteWhenDone)
         {
            // Delete *pMsg, since we are done with it.
            delete pMsg;
         }
         return OS_SUCCESS;
      }
   }

   if (!rTimeout.isInfinite())
   {
     int expireFromNow = rTimeout.cvtToMsecs();
     if (try_enqueue(pMsg, expireFromNow))
       ret = OS_SUCCESS;
     else
       ret = OS_WAIT_TIMEOUT;
   }
   else
   {
     enqueue(pMsg);
     ret = OS_SUCCESS;
   }


   int count = numMsgs();
   
   if (_reportFull && 2 * count > mMaxMsgs)
   {
     OS_LOG_WARNING(FAC_KERNEL,
                   "OsMsgQShared::doSendCore message queue '" << mName.data()
                   << "' is over half full - count = " << count
                   << " max = " << mMaxMsgs);
   }

   system_tap_queue_enqueue(mName.data(), 0, _queue.size());
   return ret;
}

// Helper function for removing a message from the head of the queue
OsStatus OsMsgQShared::doReceive(OsMsg*& rpMsg, const OsTime& rTimeout)
{
  OsStatus ret = OS_SUCCESS;

  if (!rTimeout.isInfinite())
  {
    int expireFromNow = rTimeout.cvtToMsecs();
    if (try_dequeue(rpMsg, expireFromNow))
      ret = OS_SUCCESS;
    else
      ret = OS_WAIT_TIMEOUT;
  }
  else
  {
    dequeue(rpMsg);
    ret = OS_SUCCESS;
  }

   system_tap_queue_dequeue(mName.data(), 0, _queue.size());

   return ret;
}


/* ============================ FUNCTIONS ================================= */


void OsMsgQShared::setQueuePreference(OsMsgQShared::QueuePreference preference)
{
    gQueuePreference = preference;
}

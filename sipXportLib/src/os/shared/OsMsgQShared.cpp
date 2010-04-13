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
#include "os/OsSysLog.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Message Queue implementation for OS's which do not have native message queues
//
// Two kinds of concurrent tasks, called "senders" and "receivers",
// communicate using a message queue. When the queue is empty, receivers are
// blocked until there are messages to receive. When the queue is full,
// senders are blocked until some of the queued messages are received --
// freeing up space in the queue for more messages.
//
// This implementation is based on the description from the book "Operating
// Systems Principles" by Per Brinch Hansen, 1973.  This solution uses:
//   - a counting semaphore (mEmpty) to control the delay of the sender in
//     the following way:
//       initially:      the "empty" semaphore count is set to maxMsgs
//       before send:    acquire(empty)
//       after receive:  release(empty)
//   - a counting semaphore (mFull) to control the delay of the receiver in
//     the following way:
//       initially:      the "full" semaphore count is set to 0
//       before receive: acquire(full)
//       after send:     release(full)
//   - a binary semaphore (mGuard) to ensure against concurrent access to
//     internal object data

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
// If the name is specified but is already in use, throw an exception
OsMsgQShared::OsMsgQShared(const int maxMsgs, const int maxMsgLen,
                     const int options, const UtlString& name)
   :
   OsMsgQBase(name),
   mGuard(OsMutex::Q_PRIORITY + OsMutex::INVERSION_SAFE +
          OsMutex::DELETE_SAFE),
   mEmpty(OsCSem::Q_PRIORITY, maxMsgs, maxMsgs),
   mFull(OsCSem::Q_PRIORITY, maxMsgs, 0),
   mDlist(),
   mOptions(options),
   mHighCnt(0)
{
   mMaxMsgs = maxMsgs;

#ifdef OS_MSGQ_REPORTING
   mIncrementLevel = mMaxMsgs / 20;
   if (mIncrementLevel < 1)
      mIncrementLevel = 1;
   mIncreaseLevel = mIncrementLevel;
   mDecreaseLevel = 0;
#endif

#ifdef MSGQ_IS_VALID_CHECK /* [ */
   OsStatus ret = mGuard.acquire();         // start critical section
   assert(ret == OS_SUCCESS);

   mNumInsertEntry = 0;
   mNumInsertExitOk = 0;
   mNumInsertExitFail = 0;

   mNumRemoveEntry = 0;
   mNumRemoveExitOk = 0;
   mNumRemoveExitFail = 0;

   mLastSuccessTest = 0;

   ret = mGuard.release();         // exit critical section
   assert(ret == OS_SUCCESS);
#endif /* MSGQ_IS_VALID_CHECK ] */
}

// Destructor
OsMsgQShared::~OsMsgQShared()
{
    if (numMsgs())
        flush();    // get rid of any messages in the queue
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

// Insert a message at the head of the queue
// Wait until there is either room on the queue or the timeout expires.
OsStatus OsMsgQShared::sendUrgent(const OsMsg& rMsg,
                                  const OsTime& rTimeout)
{
   return doSend(rMsg, rTimeout, TRUE, FALSE);
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
   OsLock lock(mGuard);

   return(mDlist.entries());
}

// Print information on the message queue to the console
// Output enabled via a compile-time #ifdef
#ifdef MSGQ_IS_VALID_CHECK
void OsMsgQShared::show(void)
{
   osPrintf("* OsMsgQShared: OsMsgQ=%p, options=%d, limitMsgs=%d, maxMsgs=%d, numMsgs=%d\n",
            (void *) this, mOptions, mMaxMsgs, mHighCnt, numMsgs());

   osPrintf("* OsMsgQShared: mEmpty counting semaphore information\n");
   mEmpty.OsCSemShow();

   osPrintf("* OsMsgQShared: mFull counting semaphore information\n");
   mFull.OsCSemShow();
}
#endif

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
   OsStatus ret;
   const void*    insResult;

#ifdef MSGQ_IS_VALID_CHECK /* [ */
   int      msgCnt;

   ret = mGuard.acquire();         // start critical section
   assert(ret == OS_SUCCESS);

   testMessageQ();

   mNumInsertEntry++;

   ret = mGuard.release();         // exit critical section
   assert(ret == OS_SUCCESS);
#endif /* MSGQ_IS_VALID_CHECK ] */

   if (mSendHookFunc != NULL)
   {
      if (mSendHookFunc(*pMsg))
      {
         // by returning TRUE, the mSendHookFunc indicates that it has handled
         // the message and there is no need to queue the message.
#ifdef MSGQ_IS_VALID_CHECK /* [ */
         OsStatus rc = mGuard.acquire();         // start critical section
         assert(rc == OS_SUCCESS);

         mNumInsertExitOk++;
         testMessageQ();

         rc = mGuard.release();         // exit critical section
         assert(rc == OS_SUCCESS);
#endif /* MSGQ_IS_VALID_CHECK ] */
         if (deleteWhenDone)
         {
            // Delete *pMsg, since we are done with it.
            delete pMsg;
         }
         return OS_SUCCESS;
      }
   }

   ret = mEmpty.acquire(rTimeout);   // wait for there to be room in the queue
   if (ret != OS_SUCCESS)
   {
      // Do not log problem with the queue for the syslog task to prevent
      // infinite recursion.
      if (mName != "syslog")
      {
         OsSysLog::add(FAC_KERNEL, PRI_ERR,
                       "OsMsgQShared::doSendCore message send failed for queue '%s' - no room, ret = %d",
                       mName.data(), ret);
      }
      if (ret == OS_BUSY || ret == OS_WAIT_TIMEOUT)
      {
          ret =  OS_WAIT_TIMEOUT;     // send timed out
      }
      if (deleteWhenDone)
      {
         // Delete *pMsg, since we are done with it.
         delete pMsg;
      }
   }
   else
   {
      int count, max;
      mFull.getCountMax(count, max);
      // Do not log problem with the queue for the syslog task to prevent
      // infinite recursion.
      if (2 * count > max && mName != "syslog")
      {
         OsSysLog::add(FAC_KERNEL, PRI_NOTICE,
                       "OsMsgQShared::doSendCore message queue '%s' is over half full - count = %d, max = %d",
                       mName.data(), count, max);
      }

      ret = mGuard.acquire();           // start critical section
      assert(ret == OS_SUCCESS);

      if (isUrgent)
      {
         insResult = mDlist.insertAt(0, pMsg); // insert msg at queue head
      }
      else
      {
         insResult = mDlist.insert(pMsg);      // insert msg at queue tail
      }

#ifdef MSGQ_IS_VALID_CHECK
      msgCnt = mDlist.entries();
      if (msgCnt > mHighCnt)
      {
	 mHighCnt = msgCnt;
      }
#endif

      if (insResult == NULL)
      {                                 // queue insert failed
         OsSysLog::add(FAC_KERNEL, PRI_CRIT,
                       "OsMsgQShared::doSendCore message send failed - insert failed");
         assert(FALSE);

         if (deleteWhenDone)
         {
            // Delete *pMsg, since we are done with it.
            delete pMsg;
         }
         ret = OS_UNSPECIFIED;
      }
      else
      {
         ret = mFull.release();            // signal rcvrs that a msg is available
         assert(ret == OS_SUCCESS);
      }

#ifdef OS_MSGQ_REPORTING
      int curCount;
      UtlBoolean increasedLevel = FALSE;
      UtlBoolean decreasedLevel = FALSE;

      curCount = mDlist.entries();
      if (curCount >= mIncreaseLevel)
      {
          increasedLevel = TRUE;
          while (curCount >= mIncreaseLevel)
          {
              mIncreaseLevel += mIncrementLevel;
          }
          mDecreaseLevel = mIncreaseLevel - (2 * mIncrementLevel);
      }

      if (curCount <= mDecreaseLevel)
      {
          decreasedLevel = TRUE;
          while (curCount <= mDecreaseLevel)
          {
              mDecreaseLevel = mDecreaseLevel - mIncrementLevel;
          }
          mIncreaseLevel = mDecreaseLevel + (2 * mIncrementLevel);
      }
#endif

      OsStatus guardRet = mGuard.release();           // exit critical section
      assert(guardRet == OS_SUCCESS);

#ifdef OS_MSGQ_REPORTING
      if (increasedLevel)
      {
          OsSysLogPriority pri = PRI_INFO;
          if (curCount == mMaxMsgs)
	  {
	     pri = PRI_WARNING;
	  }

          OsSysLog::add(FAC_KERNEL, pri,
                        "OsMsgQShared::doSendCore Message queue %p increased to %d msgs (max=%d)\n",
                        this, curCount, mMaxMsgs);
      }
      else if (decreasedLevel)
      {
          OsSysLog::add(FAC_KERNEL, PRI_INFO,
                        "OsMsgQShared::doSendCore Message queue %p decreased to %d msgs (max=%d)\n",
                        this, curCount, mMaxMsgs);
      }
#endif
   }

#ifdef MSGQ_IS_VALID_CHECK /* [ */
   OsStatus rc = mGuard.acquire();         // start critical section
   assert(rc == OS_SUCCESS);

   if (ret == OS_SUCCESS)
   {
      mNumInsertExitOk++;
   }
   else
   {
      mNumInsertExitFail++;
   }

   testMessageQ();

   rc = mGuard.release();         // exit critical section
   assert(rc == OS_SUCCESS);
#endif /* MSGQ_IS_VALID_CHECK ] */

   return ret;
}

// Helper function for removing a message from the head of the queue
OsStatus OsMsgQShared::doReceive(OsMsg*& rpMsg, const OsTime& rTimeout)
{
   OsStatus ret;

#ifdef MSGQ_IS_VALID_CHECK /* [ */
   ret = mGuard.acquire();         // start critical section
   assert(ret == OS_SUCCESS);

   testMessageQ();
   mNumRemoveEntry++;

   ret = mGuard.release();         // exit critical section
   assert(ret == OS_SUCCESS);
#endif /* MSGQ_IS_VALID_CHECK ] */

   ret = mFull.acquire(rTimeout);  // wait for a message to be available
   if (ret != OS_SUCCESS)
   {
      if (ret == OS_BUSY || ret == OS_WAIT_TIMEOUT)
         ret = OS_WAIT_TIMEOUT;   // receive timed out
      else
      {
         assert(FALSE);
         ret = OS_UNSPECIFIED;
      }
   }
   else
   {
      ret = mGuard.acquire();         // start critical section
      assert(ret == OS_SUCCESS);

      assert(numMsgs() > 0);
      rpMsg = (OsMsg*) mDlist.get();  // get the first message

      if (rpMsg == NULL)              // was there a message?
      {
         assert(FALSE);
         ret = OS_UNSPECIFIED;
      }
      else
      {
         ret = mEmpty.release();         // the remove operation succeeded, signal
         assert(ret == OS_SUCCESS);      //  senders that there is an available
                                         //  message slot.
      }

      (void)mGuard.release();         // exit critical section
   }

#ifdef MSGQ_IS_VALID_CHECK /* [ */
   OsStatus rc = mGuard.acquire();         // start critical section
   assert(rc == OS_SUCCESS);

   if (ret == OS_SUCCESS)
      mNumRemoveExitOk++;
   else
      mNumRemoveExitFail++;

   testMessageQ();

   rc = mGuard.release();         // exit critical section
   assert(rc == OS_SUCCESS);
#endif /* MSGQ_IS_VALID_CHECK ] */


   return ret;
}

#if defined(MSGQ_IS_VALID_CHECK) && defined(OS_CSEM_DEBUG) /* [ */
// Test for message queue integrity
void OsMsgQShared::testMessageQ()
{

   if ( (mNumInsertEntry - mNumInsertExitOk - mNumInsertExitFail == 0) &&
        (mNumRemoveEntry - mNumRemoveExitOk - mNumRemoveExitFail == 0))
   {
      unsigned int numMsgs = mDlist.entries();
      assert(numMsgs == mNumInsertExitOk - mNumRemoveExitOk);
      assert(mEmpty.getValue() == mMaxMsgs - numMsgs);
      assert(mFull.getValue() == numMsgs);
      mLastSuccessTest = 0;
   }
   else
   {
      mLastSuccessTest++;
   }
}
#endif /* defined(MSGQ_IS_VALID_CHECK) && defined(OS_CSEM_DEBUG) ] */

/* ============================ FUNCTIONS ================================= */

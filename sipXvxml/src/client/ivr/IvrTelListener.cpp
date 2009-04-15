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
#include "clientMain.h"
#include "IvrTelListener.h"
#include "tao/TaoMessage.h"
#include "tao/TaoString.h"
#include "os/OsLock.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define INITIAL_MAX_TRANSFERS 100
#define INITIAL_MAX_LISTENERS 100
#define INITIAL_MAX_CLEANUPS 100
#define ADDITIONAL_TRANSFERS 20
#define ADDITIONAL_LISTENERS 20
#define ADDITIONAL_CLEANUPS 20

// STATIC VARIABLE INITIALIZATIONS
IvrTelListener* IvrTelListener::spInstance = 0;
OsBSem  IvrTelListener::sLock(OsBSem::Q_PRIORITY, OsBSem::FULL);

// LOCAL FUNCTIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
IvrTelListener::IvrTelListener(CallManager* pCallMgr,
                int timeoutSec,
                const UtlString& name,
                int maxRequestQMsgs)
: OsServerTask(name, NULL, maxRequestQMsgs),
        mRWMutex(OsRWMutex::Q_PRIORITY),
        mTimeoutSec(timeoutSec),
        mListenerSem(OsBSem::Q_PRIORITY, OsBSem::FULL),
        mListenerCnt(0),
        mCleanupSem(OsBSem::Q_PRIORITY, OsBSem::FULL),
        mTransferSem(OsBSem::Q_PRIORITY, OsBSem::FULL)
{    
   mpCallManager = pCallMgr;
   mMaxNumTransfers = INITIAL_MAX_TRANSFERS;
   mMaxNumListeners = INITIAL_MAX_LISTENERS;
   mMaxNumCleanups = INITIAL_MAX_CLEANUPS;

   mpListeners = (ListenerDB**) malloc(sizeof(ListenerDB *)*mMaxNumListeners);
   mpCleanupInProgress = (CleanupDB**) malloc(sizeof(CleanupDB *)*mMaxNumCleanups);

   if (!mpListeners || !mpCleanupInProgress)
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, "***** ERROR ALLOCATING LISTENERS IN IvrTelListener **** \n");
   }

   int i;
   for (i = 0; i < mMaxNumListeners; i++)
      mpListeners[i] = 0;

   mCleanupCnt = 0;
   for (i = 0; i < mMaxNumListeners; i++)
      mpCleanupInProgress[i] = 0;

   mTransferredCallCnt = 0;
   mTransferredCalls = (TransferredCalls**) malloc(sizeof(TransferredCalls *)*mMaxNumTransfers);
   if (!mTransferredCalls)
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, "***** ERROR ALLOCATING MMEMORY FOR TRANSFER CALLIDS IN IvrTelListener **** \n");
      OsSysLog::flush();
   }

   for (i = 0; i < mMaxNumTransfers; i++)
      mTransferredCalls[i] = 0;

   start();

#ifdef TEST
   if (!sIsTested)
   {
      sIsTested = true;
      test();
   }
#endif /* TEST */
}

UtlBoolean IvrTelListener::handleMessage(OsMsg& eventMessage)
{
   UtlBoolean rc = TRUE;

   switch (eventMessage.getMsgType())
   {
   case OsMsg::TAO_MSG:
   {
      TaoMessage* pTaoMsg = (TaoMessage*) &eventMessage;
      switch (pTaoMsg->getMsgSubType())
      {
      case TaoMessage::EVENT:
         processTaoEvent(pTaoMsg) ;
         break ;
      default:
         break ;
      }
   }
   break ;
   default:
      rc = OsServerTask::handleMessage(eventMessage);
      break ;
   }
   return rc ;
}

// Copy constructor
IvrTelListener::IvrTelListener(const IvrTelListener& rIvrTelListener)
        : mRWMutex(OsRWMutex::Q_PRIORITY),
        mListenerSem(OsBSem::Q_PRIORITY, OsBSem::FULL),
        mCleanupSem(OsBSem::Q_PRIORITY, OsBSem::FULL),
mTransferSem(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
}

// Destructor
IvrTelListener::~IvrTelListener()
{
   waitUntilShutDown();
   mListenerSem.acquire();
   int i;
   for (i = 0; i < mListenerCnt; i++)
   {
      if (mpListeners[i])
      {
         if (mpListeners[i]->mpSemStateChange)
         {
            int tries = 0;  
            while (mpListeners[i]->mSemState == IN_USE && tries++ < 3)
            {
               OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, "IvrTelListener destructor - sem still in use, tries= %d for %s\n",
                             tries, mpListeners[i]->mName.data());
               OsTask::delay(100);
            }

            mpListeners[i]->mpSemStateChange->release();
            delete mpListeners[i]->mpSemStateChange;
            mpListeners[i]->mpSemStateChange = NULL;
         }
         delete mpListeners[i];
         mpListeners[i] = 0;
      }
   }

   if (mpListeners)
   {
      free(mpListeners);
      mpListeners = NULL;
   }

   mCleanupSem.acquire();
   for (i = 0; i < mCleanupCnt; i++)
   {
      if (mpCleanupInProgress[i])
      {
         delete mpCleanupInProgress[i];
         mpCleanupInProgress[i] = 0;
      }
   }

   if (mpCleanupInProgress)
   {
      free(mpCleanupInProgress);
      mpCleanupInProgress = NULL;
   }
   mCleanupSem.release();

   mListenerSem.release();

   mTransferSem.acquire();
   if (mTransferredCalls)
   {
      for (i = 0; i < mTransferredCallCnt; i++)
      {
         if (mTransferredCalls[i])
         {
            delete mTransferredCalls[i];
            mTransferredCalls[i] = 0;
         }
      }
      free(mTransferredCalls);
      mTransferredCalls = NULL;
   }
   mTransferSem.release();
}

/* ============================ MANIPULATORS ============================== */

IvrTelListener* IvrTelListener::getTelListener(CallManager* pCallMgr)
{
   OsLock singletonLock(sLock) ;

   // Make sure one and only one IvrUtlTask exists and is started.
   // First caller creates and starts it.  Subsequent callers get that one.

   if (spInstance == NULL)
   {
      spInstance = new IvrTelListener(pCallMgr);
      UtlBoolean isStarted = spInstance->start();
      assert(isStarted);
   }

#ifdef TEST
   if (!sIsTested)
   {
      sIsTested = true;
      spInstance->test();
   }
#endif //TEST

   return spInstance;
}

// Assignment operator
        IvrTelListener&
IvrTelListener::operator=(const IvrTelListener& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

int IvrTelListener::waitForFinalState(const char* callId, char* remoteAddress)
{      
   int state = 0;
   int tries = 0;
        
   if (callId == NULL)
   {
      return 0;
   }

   while (state == 0 && tries++ < 3)
   {
      mListenerSem.acquire();
      if (mpListeners == NULL)
      {
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, "mpListener is NULL! failed in waitForFinalState for %s\n", callId);
         mListenerSem.release();
         return state;
      }
      OsBSem *pSemStateChange = findStateChangeSemaphore(callId);
      mListenerSem.release();
      if (pSemStateChange)
      {
         OsStatus status = pSemStateChange->acquire(OsTime(mTimeoutSec, 0)); 
         if (status == OS_SUCCESS)
         {
            // got the state change
            mListenerSem.acquire();
            int i = findIndexForListener(callId);
            if (i >= 0)
            {
               state = mpListeners[i]->mpListenerPtr;
               if ((state & 0x0000ffff) == PtEvent::CONNECTION_DISCONNECTED    || 
                   (state & 0x0000ffff) == PtEvent::CONNECTION_ESTABLISHED ||
                   (state & 0x0000ffff) == PtEvent::CONNECTION_CONNECTED ||
                   (state & 0x0000ffff) == PtEvent::CONNECTION_FAILED) 
               {
                  int len = (mpListeners[i]->mRemoteAddesss).length();
                  if (len > 0)
                  {
                     if (len > 255) len = 255;
                     strncpy(remoteAddress, (mpListeners[i]->mRemoteAddesss).data(), len);
                  }
               }
               else
               {
                  OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, 
                                "waitForFinalState - not final state, %s, state=%d tries=%d\n", 
                                callId, state, tries) ;
                  state = 0; // didn't get the state I wanted, will try again
               }

               pSemStateChange->release();

               if (mpListeners[i]->mSemState != IN_USE)
               {
                  OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, 
                                "waitForFinalState - change sem state to IDLE from %d for %s\n", 
                                mpListeners[i]->mSemState, callId);
               }
               mpListeners[i]->mSemState = IDLE;
            }
            else
            {
               OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, 
                             "waitForFinalState - findIndexForListener returned %d for %s\n", 
                             i, callId);
            }
            mListenerSem.release();
         }
         else
         {
            mListenerSem.acquire();
            int i = findIndexForListener(callId);
            if (i >= 0)
            {
               int semState = mpListeners[i]->mSemState;
               mpListeners[i]->mSemState = IDLE;
               if (semState == REQUEST_REMOVE)
               {
                  OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, 
                                "waitForFinalState - timed out %s tries=%d, semState=REQUEST_REMOVE\n", 
                                callId, tries);
                  mListenerSem.release();
                  break; // while loop
               }

               OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, 
                             "waitForFinalState - timed out %s tries=%d, sem state=%d, changing to IDLE\n",
                             callId, tries, semState);
            }
            mListenerSem.release();
         }
      }
      else
      {
         // no state change semaphore found for this call, quit
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, 
                       "waitForFinalState - no state change semaphore is found for %s\n", callId) ;
         break; // while loop
      }
   }

   return state ;
}

UtlBoolean IvrTelListener::stopWaitForFinalState(const char* callId)
{  
   return removeListener(callId);
}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

UtlBoolean IvrTelListener::isCleanupInProgress(const char* callId)
{
   mListenerSem.acquire();
   mCleanupSem.acquire();
   UtlBoolean found = FALSE;

   if (callId == NULL)
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, " IvrTelListener::isCleanupInProgress callId is NULL!\n");
   }
   else
   {
      for (int i = 0; i < mCleanupCnt; i++)
      {
         if (mpCleanupInProgress[i]->mName.compareTo(callId) == 0)
         {
            found = TRUE;
            break;
         }
      }
   }

   mCleanupSem.release();
   mListenerSem.release();

   return found;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Process tao event.
UtlBoolean IvrTelListener::processTaoEvent(TaoMessage* pTaoMsg)
{  
   UtlBoolean rec = TRUE;

   PtEvent::PtEventId      eventId = (PtEvent::PtEventId) pTaoMsg->getTaoObjHandle();
   TaoString argList(pTaoMsg->getArgList(), TAOMESSAGE_DELIMITER);
   UtlBoolean localConnection = atoi(argList[6]);

   switch (eventId)
   {
   case PtEvent::CONNECTION_DISCONNECTED:
   case PtEvent::CONNECTION_FAILED:
      if (!localConnection)
         handleDisconnectCall(argList);

      break;

   default:
      break;
   }

   rec = processTaoMessage(pTaoMsg);
   return rec;
}

void IvrTelListener::handleDisconnectCall(TaoString& arg)
{
   UtlString callId = arg[0];

   {
      doCleanUpCall(callId);
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, "IVR: Dropping call: %s\n", callId.data());
      mTransferSem.acquire();
      for (int i = 0; i < mTransferredCallCnt; i++)
      {
         if (!(mTransferredCalls[i]->mCallId.isNull()) && (mTransferredCalls[i]->mCallId.compareTo(callId) == 0))
         {
            if (!(mTransferredCalls[i]->mTgtCallId.isNull()))
            {
               OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, "IVR: Dropping target call: %s, mCallId matches %s\n", mTransferredCalls[i]->mTgtCallId.data(), callId.data());
               mpCallManager->drop(mTransferredCalls[i]->mTgtCallId);
               removeTransferCallId(i);
               break;
            }
         }
         else if (!(mTransferredCalls[i]->mTgtCallId.isNull()) && (mTransferredCalls[i]->mTgtCallId.compareTo(callId) == 0))
         {
            if (!(mTransferredCalls[i]->mCallId.isNull()))
            {
               OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, "IVR: Dropping target call: %s, tgtCallId matches %s\n", mTransferredCalls[i]->mCallId.data(), callId.data());
               mpCallManager->drop(mTransferredCalls[i]->mCallId);
               removeTransferCallId(i);
               break;
            }
         }
      }
      mTransferSem.release();

      mpCallManager->drop(callId);
   }
}

UtlBoolean IvrTelListener::processTaoMessage(TaoMessage* pTaoMsg) 
{
   PtEvent::PtEventId    eventId = (PtEvent::PtEventId) pTaoMsg->getTaoObjHandle();
   if (!PtEvent::isConnectionEvent(eventId) && (eventId != PtEvent::MULTICALL_META_TRANSFER_STARTED))
   {
      return FALSE;
   }

   TaoString argList(pTaoMsg->getArgList(), TAOMESSAGE_DELIMITER);
   int cnt = argList.getCnt();
   if (cnt < 5)
      return FALSE;

   int i;
#ifdef TEST_PRINT /* [ */
   osPrintf("\n++------------------*---------------------++\n");
   osPrintf(" Ivr recieved msg\n");
   osPrintf(" type     %d\n subtype  %d \n", pTaoMsg->getMsgType(), pTaoMsg->getMsgSubType());
   osPrintf(" event id %d\n", pTaoMsg->getTaoObjHandle());
   osPrintf(" localconnection %d\n", atoi(argList[6]));
   for (i = 0; i < cnt; i++)
      osPrintf("          %s\n", argList[i]);
   osPrintf("\n++------------------*---------------------++\n");
#endif /* TEST_PRINT ] */

   UtlString callId = argList[0];

   PtEvent::PtEventCause cause = (PtEvent::PtEventCause) atoi(argList[4]);

   int numOldCalls = (cnt > 10) ? (cnt - 11) : 0;

   UtlString* oldCallIds = 0;
   if (numOldCalls > 0)
   {       
      oldCallIds = new UtlString[numOldCalls];
      for (i = 0; i < numOldCalls; i++)
         oldCallIds[i] = argList[i + 11]; // 1st is the new call

      if (eventId == PtEvent::MULTICALL_META_TRANSFER_STARTED)
      {
         addTransferCallIds(oldCallIds, numOldCalls);
         return TRUE;
      }
      for (i = 0; i < numOldCalls; i++)
      {
         for (int j = 0; j < mListenerCnt; j++)
         {
            if (oldCallIds[i] == mpListeners[j]->mName)
            {
               if (i < numOldCalls - 1)
                  mpListeners[j]->mAltName = oldCallIds[i + 1];
               else
                  mpListeners[j]->mAltName = oldCallIds[i - 1];
            }
         }
      }
   }

   for (i = 0; i < mListenerCnt; i++)
   {
      if (callId == mpListeners[i]->mName)
      {
         mpListeners[i]->mRemoteAddesss = argList[2];            // remote address
         break;
      }
   }

   if (oldCallIds) 
   {
      delete[] oldCallIds;
      oldCallIds = NULL;
   }

   mListenerSem.acquire();
   if (mpListeners == NULL)
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "mpListener is NULL! failed to add listener for %s\n", callId.data());
      mListenerSem.release();
      return TRUE;
   }
   for (i = 0; i < mListenerCnt; i++)
   {
      if ((callId && mpListeners[i]->mName.compareTo(callId) == 0) ||
          (mpListeners[i]->mAltName.compareTo(callId) == 0))
      {
         if (eventId == PtEvent::CONNECTION_DISCONNECTED || 
             eventId == PtEvent::CONNECTION_ESTABLISHED ||
             eventId == PtEvent::CONNECTION_CONNECTED ||
             eventId == PtEvent::CONNECTION_FAILED)
         {
            mpListeners[i]->mpListenerPtr = (eventId | (cause << 16)); 
            if (mpListeners[i]->mpSemStateChange) 
               mpListeners[i]->mpSemStateChange->release();
            break;
         }
      }
   }
   mListenerSem.release();
   return TRUE;
}

void IvrTelListener::addListener(const char* callId)
{
#ifdef TEST_PRINT
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "IvrTelListener::addListener %s\n", callId);
#endif
   if (callId == NULL)
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "IvrTelListener::addListener callId is NULL!\n");
      return;
   }
   mListenerSem.acquire();
   if (mpListeners == NULL)
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "mpListener is NULL! failed to add listener for %s\n", callId);
      mListenerSem.release();
      return;
   }
   for (int i = 0; i < mListenerCnt; i++)
   {
      if (mpListeners[i]->mName.compareTo(callId) == 0)
      {
         mpListeners[i]->mRef++;
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, "Listener already exists for call %s\n", callId);
         mListenerSem.release();
         return;
      }
   }

   if (mListenerCnt == mMaxNumListeners)
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, "IvrTelListener::addListener - add %d more listeners\n", ADDITIONAL_LISTENERS);

      //make more of em.
      mMaxNumListeners += ADDITIONAL_LISTENERS;
      mpListeners = (ListenerDB **)realloc(mpListeners,sizeof(ListenerDB *)*mMaxNumListeners);
      if (mpListeners == NULL)
      {
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,   
                       "IvrTelListener::addListener - out of memory for call %s\n", callId);
         OsSysLog::flush();
         mListenerSem.release();
         return;
      }

      for (int loop = mListenerCnt;loop < mMaxNumListeners;loop++)
         mpListeners[loop] = 0 ;
   }

   ListenerDB *pListenerDb = new ListenerDB();
   if (pListenerDb)
   {
      pListenerDb->mRef = 1;
      pListenerDb->mName.append(callId);
      pListenerDb->mAltName.remove(0);
      pListenerDb->mRemoteAddesss.remove(0);
      pListenerDb->mpListenerPtr = 0;
      pListenerDb->mpSemStateChange = new OsBSem(OsBSem::Q_PRIORITY, OsBSem::EMPTY);
      pListenerDb->mSemState = IDLE;
      mpListeners[mListenerCnt++] = pListenerDb;
   }
   mListenerSem.release();
#ifdef TEST_PRINT
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "IvrTelListener::addListener added %s\n", callId);
#endif
}

OsStatus IvrTelListener::removeListener(const char* callId)
{
   mListenerSem.acquire();
   OsStatus res = OS_NOT_FOUND;

   if (mpListeners == NULL)
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, "mpListener is NULL! failed to remove listener for %s\n", callId);
      mListenerSem.release();
      return res;
   }
   for (int i = 0; i < mListenerCnt; i++)
   {
      if (mpListeners[i] && mpListeners[i]->mName.compareTo(callId) == 0)
      {
         mpListeners[i]->mRef--;
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, 
                       "IvrTelListener::removeListener callId matches, %s, mRef=%d, sem=%p\n", 
                       callId, mpListeners[i]->mRef, mpListeners[i]->mpSemStateChange);
         res = OS_BUSY;
         if (mpListeners[i]->mRef <= 0)
         {
            if (mpListeners[i]->mpSemStateChange)
            {
               if (mpListeners[i]->mSemState != IDLE)
               {
                  OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, 
                                "IvrTelListener::removeListener %s, sem %d, set to REQUEST_REMOVE\n", 
                                callId, mpListeners[i]->mSemState);
                  mpListeners[i]->mRef++;
                  mpListeners[i]->mSemState = REQUEST_REMOVE;
               }
               else
               {
                  mpListeners[i]->mpSemStateChange->release();
                  delete mpListeners[i]->mpSemStateChange;
                  mpListeners[i]->mpSemStateChange = NULL;

                  delete mpListeners[i];
                  mpListeners[i] = NULL;
                  mListenerCnt--;
                  for (int j = i; j < mListenerCnt; j++)
                  {
                     mpListeners[j] = mpListeners[j + 1];
                  }
                  mpListeners[mListenerCnt + 1] = NULL;
                  OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, 
                                "IvrTelListener::removeListener deleted listener for %s, \n", callId);
                  res = OS_SUCCESS;
               }
            }
         }
         break;
      }
   }

   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, "IvrTelListener::removeListener %s removed=%d\n", callId, res);
   mListenerSem.release();

   return res;
}

void IvrTelListener::addTransferCallIds(UtlString* callIds, int numOldCalls)
{
#ifdef TEST_PRINT
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "IvrTelListener::addTransferCallIds %s %s\n", callIds[0].data(), callIds[1].data());
#endif
   mTransferSem.acquire();
   for (int i = 0; i < mTransferredCallCnt; i++)
   {
      if (!(callIds[0].isNull()) && (mTransferredCalls[i]->mCallId.compareTo(callIds[0]) == 0))
      {
         mTransferredCalls[i]->mRef++;
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "Transferred call already exists for call %s\n", callIds[0].data());
         mTransferSem.release();
         return;
      }
   }

   if (mTransferredCallCnt == mMaxNumTransfers)
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, "IvrTelListener::addTransferCallIds - add %d more transferCalls\n", ADDITIONAL_TRANSFERS);

      //make more of em.
      mMaxNumTransfers += ADDITIONAL_TRANSFERS;
      mTransferredCalls = (TransferredCalls **)realloc(mTransferredCalls, sizeof(TransferredCalls *)*mMaxNumTransfers);
      if (mTransferredCalls == NULL)
      {
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,   
                       "IvrTelListener::addTransfersCallIds - out of memory for call %s\n", callIds[0].data());
         OsSysLog::flush();
         mTransferSem.release();
         return;
      }
      for (int loop = mTransferredCallCnt;loop < mMaxNumTransfers;loop++)
         mTransferredCalls[loop] = 0 ;
   }

   TransferredCalls *pTransfer = new TransferredCalls();
   if (pTransfer)
   {
      pTransfer->mRef = 1;
      pTransfer->mCallId.append(callIds[0]);
      pTransfer->mTgtCallId.append(callIds[1]);
      mTransferredCalls[mTransferredCallCnt++] = pTransfer;
   }
   mTransferSem.release();
#ifdef TEST_PRINT
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "IvrTelListener:::addTransferCallIds added %s %s\n", callIds[0].data(), callIds[1].data());
#endif
}

UtlBoolean IvrTelListener::removeFromCleanupInProgress(const char* callId)
{
   mCleanupSem.acquire();
   UtlBoolean res = FALSE;
   if (mpCleanupInProgress == NULL)
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "mpCleanupInProgress is NULL! failed to remove for %s\n", callId);
   }
   else
   {
      for (int i = 0; i < mCleanupCnt; i++)
      {
         if (mpCleanupInProgress[i] && mpCleanupInProgress[i]->mName.compareTo(callId) == 0)
         {
            delete mpCleanupInProgress[i];
            // Decrement the count of valid entries.
            mCleanupCnt--;
            // Move the last valid element (which was mCleanupCnt-1 and is
            // now mCleanupCnt) into position i.
            mpCleanupInProgress[i] = mpCleanupInProgress[mCleanupCnt];
            // Null out the entry that is no longer valid.
            mpCleanupInProgress[mCleanupCnt] = NULL;
            res = TRUE;
            break;
         }
      }
   }

   mCleanupSem.release();
   return res;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

OsBSem *IvrTelListener::findStateChangeSemaphore(const char* callId)
{
   OsBSem *pSemStateChange = NULL;

   if (callId == NULL)
      return NULL;

   for (int i = 0; i < mListenerCnt; i++)
   {
      if (mpListeners[i]->mName.compareTo(callId) == 0)
      {
         pSemStateChange = mpListeners[i]->mpSemStateChange;
         if (pSemStateChange)
         {
            if ( mpListeners[i]->mSemState == REQUEST_REMOVE)
            {
               OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, 
                             "IvrTelListener:::findStateChangeSemaphore %s sem state not IDLE, state=%d\n", 
                             callId, mpListeners[i]->mSemState);

               pSemStateChange = NULL;
            }
            else
            {
               mpListeners[i]->mSemState = IN_USE;
            }
         }
         break;
      }
   }

   return pSemStateChange;
}

void IvrTelListener::doCleanUpCall(const char* callId)
{
   mListenerSem.acquire();
   mCleanupSem.acquire();

   int VXISessionEnded = 1;
   if (findIndexForListener(callId) >= 0) 
   {
      VXISessionEnded = 0;
      addToCleanupInProgress(callId);
   }

   mCleanupSem.release();
   mListenerSem.release();

   VXICleanUpCall(this, callId, VXISessionEnded);
}

void IvrTelListener::addToCleanupInProgress(const char* callId)
{
   if (callId == NULL)
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "IvrTelListener::addToCleanupInProgress callId is NULL! \n");
      return;
   }

   if (mpCleanupInProgress == NULL)
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "IvrTelListener::addToCleanupInProgress mpCleanupInProgress is NULL! failed to add for %s\n", callId);
      return;
   }

   for (int i = 0; i < mCleanupCnt; i++)
   {
      if (mpCleanupInProgress[i]->mName.compareTo(callId) == 0)
      {
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, "mpCleanupInProgress already exists for call %s\n", callId);
         return;
      }
   }

   if (mCleanupCnt == mMaxNumCleanups)
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, "IvrTelListener::addToCleanupInProgress - add %d more transferCalls\n", ADDITIONAL_CLEANUPS);

      //make more of em.
      mMaxNumCleanups += ADDITIONAL_CLEANUPS;
      mpCleanupInProgress = (CleanupDB **)realloc(mpCleanupInProgress, sizeof(CleanupDB *)*mMaxNumCleanups);
      if (mpCleanupInProgress == NULL)
      {
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,   
                       "IvrTelListener::addToCleanupInProgress - out of memory for call %s\n", callId);
         OsSysLog::flush();
         return;
      }

      for (int loop = mCleanupCnt;loop < mMaxNumCleanups;loop++)
         mpCleanupInProgress[loop] = 0 ;
   }

   CleanupDB *pCleanupDb = new CleanupDB();
   if (pCleanupDb)
   {
      pCleanupDb->mName.append(callId);
      mpCleanupInProgress[mCleanupCnt++] = pCleanupDb;
#ifdef TEST_PRINT
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "IvrTelListener::addToCleanupInProgress succeeded for call %s\n", callId);
#endif
   }
   else
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, "IvrTelListener::addToCleanupInProgress failed to allocate CleanupDB for call %s\n", callId);
   }
}


void IvrTelListener::removeTransferCallId(int index)
{
   if (mTransferredCalls[index])
   {
      delete mTransferredCalls[index];
      mTransferredCalls[index] = 0;
   }

   mTransferredCallCnt--;
   for (int i = index; i < mTransferredCallCnt; i++)
   {
      mTransferredCalls[i] = mTransferredCalls[i + 1];
   }
   mTransferredCalls[mTransferredCallCnt] = NULL;
}

int IvrTelListener::findIndexForListener(const char* callId)
{
   int index = -1;
   for (int i = 0; i < mListenerCnt; i++)
   {
      if (mpListeners[i]->mName.compareTo(callId) == 0)
      {
         index = i;
         break;
      }
   }

   return index;
}

/* ============================ TESTING =================================== */

#ifdef TEST

// Set to true after the tests have been executed once
bool IvrTelListener::sIsTested = false;

// Test this class by running all of its assertion tests
void IvrTelListener::test()
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
void IvrTelListener::testCreators()
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
void IvrTelListener::testManipulators()
{
   UtlMemCheck* pMemCheck  = 0;

   pMemCheck = new UtlMemCheck();      // checkpoint for memory leak check

   // test the assignment method (if implemented)
   // test the other manipulator methods for the class

   assert(pMemCheck->delta() == 0);    // check for memory leak
   delete pMemCheck;
}

// Test the accessor methods for the class
void IvrTelListener::testAccessors()
{
   UtlMemCheck* pMemCheck  = 0;

   pMemCheck = new UtlMemCheck();      // checkpoint for memory leak check

   // body of the test goes here

   assert(pMemCheck->delta() == 0);    // check for memory leak
   delete pMemCheck;
}

// Test the inquiry methods for the class
void IvrTelListener::testInquiry()
{
   UtlMemCheck* pMemCheck  = 0;

   pMemCheck = new UtlMemCheck();      // checkpoint for memory leak check

   // body of the test goes here

   assert(pMemCheck->delta() == 0);    // check for memory leak
   delete pMemCheck;
}

#endif /* TEST */

/* ============================ FUNCTIONS ================================= */

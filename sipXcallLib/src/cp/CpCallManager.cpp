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
#include "os/OsDefs.h"
#include <assert.h>

// APPLICATION INCLUDES
#include <cp/CpCallManager.h>
#include <cp/CpCall.h>
#include <net/NetMd5Codec.h>
#include <os/OsLock.h>
#include <os/OsDateTime.h>
#include <os/OsSocket.h>
#include <os/OsSysLog.h>
#include <os/OsReadLock.h>
#include <os/OsWriteLock.h>
#include <os/OsProcess.h>

#include "ptapi/PtAddressForwarding.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
#ifdef __pingtel_on_posix__ /* [ */
const int    CpCallManager::CALLMANAGER_MAX_REQUEST_MSGS = 6000;
#else
const int    CpCallManager::CALLMANAGER_MAX_REQUEST_MSGS = 1000;
#endif

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpCallManager::CpCallManager(const char* taskName,
                             const char* callIdPrefix,
                             int rtpPortStart,
                             int rtpPortEnd,
                             const char* localAddress,
                             const char* publicAddress) :
OsServerTask(taskName, NULL, CALLMANAGER_MAX_REQUEST_MSGS),
mManagerMutex(OsMutex::Q_FIFO),
mCallListMutex(OsMutex::Q_FIFO),
mCallIndices(),
mAddressForwardMutex(OsMutex::Q_FIFO)
{
   mpAddressForwards = 0;
   mDoNotDisturbFlag = FALSE;
   mMsgWaitingFlag = FALSE;
   mOfferedTimeOut = 0;
   mAddressForwardingCnt = 0;

   // callIdPrefix used to be used to initialize mCallIdPrefix, but
   // mCallIdPrefix has been deleted because we've moved Call-Id generation
   // into CallId in sipXtackLib.

   mRtpPortStart = rtpPortStart;
   mRtpPortEnd = rtpPortEnd;

   if(localAddress && *localAddress)
   {
      mLocalAddress.append(localAddress);
   }
   else
   {
      OsSocket::getHostIp(&mLocalAddress);
   }

   if(publicAddress && *publicAddress)
   {
      mPublicAddress.append(publicAddress);
   }
   else
   {
      OsSocket::getHostIp(&mPublicAddress);
   }

   mLastMetaEventId = 0;
}

// Copy constructor
CpCallManager::CpCallManager(const CpCallManager& rCpCallManager) :
OsServerTask("badCallManagerCopy"),
mManagerMutex(OsMutex::Q_FIFO),
mCallListMutex(OsMutex::Q_FIFO),
mAddressForwardMutex(OsMutex::Q_FIFO)

{
    mDoNotDisturbFlag = rCpCallManager.mDoNotDisturbFlag;
    mMsgWaitingFlag = rCpCallManager.mMsgWaitingFlag;
    mOfferedTimeOut = rCpCallManager.mOfferedTimeOut;

    mAddressForwardingCnt = rCpCallManager.mAddressForwardingCnt;
    if (mAddressForwardingCnt > 0)
    {
        mpAddressForwards = new PtAddressForwarding[mAddressForwardingCnt];
        for (int i = 0; i < mAddressForwardingCnt; i++)
            mpAddressForwards[i] =  rCpCallManager.mpAddressForwards[i];
    }
    else
    {
        mpAddressForwards = 0;
    }

    mLastMetaEventId = 0;
}

// Destructor
CpCallManager::~CpCallManager()
{
    if (mpAddressForwards)
    {
        delete mpAddressForwards;
        mpAddressForwards = 0;
        mAddressForwardingCnt = 0;
    }
}

/* ============================ MANIPULATORS ============================== */

void CpCallManager::getEventSubTypeString(EventSubTypes type,
                                          UtlString& typeString)
{
    switch(type)
    {
    case CP_UNSPECIFIED:
      typeString = "CP_UNSPECIFIED";
      break;

    case CP_GET_INVITE:
      typeString = "CP_GET_INVITE";
      break;

    case CP_ADD_TONE_LISTENER:
      typeString = "CP_ADD_TONE_LISTENER";
      break;

    case CP_REMOVE_TONE_LISTENER:
      typeString = "CP_REMOVE_TONE_LISTENER";
      break;

    case CP_DISABLE_DTMF_EVENT:
      typeString = "CP_DISABLE_DTMF_EVENT";
      break;

    case CP_SIP_MESSAGE:
      typeString = "CP_SIP_MESSAGE";
      break;

    case CP_CALL_EXITED:
      typeString = "CP_CALL_EXITED";
      break;

    case CP_DIAL_STRING:
      typeString = "CP_DIAL_STRING";
      break;

    case CP_FOCUS_CALL:
      typeString = "CP_FOCUS_CALL";
      break;

    case CP_HOLD_CALL:
      typeString = "CP_HOLD_CALL";
      break;

    case CP_OFF_HOLD_CALL:
      typeString = "CP_OFF_HOLD_CALL";
      break;

    case CP_DEQUEUED_CALL:
      typeString = "CP_DEQUEUED_CALL";
      break;

    case CP_MGCP_MESSAGE:
      typeString = "CP_MGCP_MESSAGE";
      break;

    case CP_MGCP_CAPS_MESSAGE:
      typeString = "CP_MGCP_CAPS_MESSAGE";
      break;

    case CP_YIELD_FOCUS:
      typeString = "CP_YIELD_FOCUS";
      break;

    case CP_GET_FOCUS:
      typeString = "CP_GET_FOCUS";
      break;

    case CP_CREATE_CALL:
      typeString = "CP_CREATE_CALL";
      break;

    case CP_GET_CALLS:
      typeString = "CP_GET_CALLS";
      break;

    case CP_CONNECT:
      typeString = "CP_CONNECT";
      break;

    case CP_SINGLE_CALL_TRANSFER:
      typeString = "CP_SINGLE_CALL_TRANSFER";
      break;

    case CP_BLIND_TRANSFER:
      typeString = "CP_BLIND_TRANSFER";
      break;

    case CP_CONSULT_TRANSFER:
      typeString = "CP_CONSULT_TRANSFER";
      break;

    case CP_CONSULT_TRANSFER_ADDRESS:
      typeString = "CP_CONSULT_TRANSFER_ADDRESS";
      break;

    case CP_TRANSFER_CONNECTION:
      typeString = "CP_TRANSFER_CONNECTION";
      break;

    case CP_TRANSFER_CONNECTION_STATUS:
      typeString = "CP_TRANSFER_CONNECTION_STATUS";
      break;

    case CP_TRANSFEREE_CONNECTION:
      typeString = "CP_TRANSFEREE_CONNECTION";
      break;

    case CP_TRANSFEREE_CONNECTION_STATUS:
      typeString = "CP_TRANSFEREE_CONNECTION_STATUS";
      break;

    case CP_DROP:
      typeString = "CP_DROP";
      break;

    case CP_DROP_CONNECTION:
      typeString = "CP_DROP_CONNECTION";
      break;

    case CP_FORCE_DROP_CONNECTION:
      typeString = "CP_FORCE_DROP_CONNECTION";
      break;

    case CP_ANSWER_CONNECTION:
      typeString = "CP_ANSWER_CONNECTION";
      break;

    case CP_ACCEPT_CONNECTION:
      typeString = "CP_ACCEPT_CONNECTION";
      break;

    case CP_REJECT_CONNECTION:
      typeString = "CP_REJECT_CONNECTION";
      break;

    case CP_REDIRECT_CONNECTION:
      typeString = "CP_REDIRECT_CONNECTION";
      break;

    case CP_GET_NUM_CONNECTIONS:
      typeString = "CP_GET_NUM_CONNECTIONS";
      break;

    case CP_GET_CONNECTIONS:
      typeString = "CP_GET_CONNECTIONS";
      break;

    case CP_GET_CALLED_ADDRESSES:
      typeString = "CP_GET_CALLED_ADDRESSES";
      break;

    case CP_GET_CALLING_ADDRESSES:
      typeString = "CP_GET_CALLING_ADDRESSES";
      break;

    case CP_START_TONE_TERM_CONNECTION:
      typeString = "CP_START_TONE_TERM_CONNECTION";
      break;

    case CP_STOP_TONE_TERM_CONNECTION:
      typeString = "CP_STOP_TONE_TERM_CONNECTION";
      break;

    case CP_PLAY_AUDIO_TERM_CONNECTION:
      typeString = "CP_PLAY_AUDIO_TERM_CONNECTION";
      break;

    case CP_STOP_AUDIO_TERM_CONNECTION:
      typeString = "CP_STOP_AUDIO_TERM_CONNECTION";
      break;

    case CP_CREATE_PLAYER:
      typeString = "CP_CREATE_PLAYER";
      break;

    case CP_DESTROY_PLAYER:
      typeString = "CP_DESTROY_PLAYER";
      break;

    case CP_CREATE_PLAYLIST_PLAYER:
      typeString = "CP_CREATE_PLAYLIST_PLAYER";
      break;

    case CP_DESTROY_PLAYLIST_PLAYER:
      typeString = "CP_DESTROY_PLAYLIST_PLAYER";
      break;

    case CP_GET_NUM_TERM_CONNECTIONS:
      typeString = "CP_GET_NUM_TERM_CONNECTIONS";
      break;

    case CP_GET_TERM_CONNECTIONS:
      typeString = "CP_GET_TERM_CONNECTIONS";
      break;

    case CP_IS_LOCAL_TERM_CONNECTION:
      typeString = "CP_IS_LOCAL_TERM_CONNECTION";
      break;

    case CP_HOLD_TERM_CONNECTION:
      typeString = "CP_HOLD_TERM_CONNECTION";
      break;

    case CP_UNHOLD_TERM_CONNECTION:
      typeString = "CP_UNHOLD_TERM_CONNECTION";
      break;

    case CP_UNHOLD_LOCAL_TERM_CONNECTION:
      typeString = "CP_UNHOLD_LOCAL_TERM_CONNECTION";
      break;

    case CP_HOLD_LOCAL_TERM_CONNECTION:
      typeString = "CP_HOLD_LOCAL_TERM_CONNECTION";
      break;

    case CP_OFFERING_EXPIRED:
      typeString = "CP_OFFERING_EXPIRED";
      break;

    case CP_RINGING_EXPIRED:
      typeString = "CP_RINGING_EXPIRED";
      break;

    case CP_GET_CALLSTATE:
      typeString = "CP_GET_CALLSTATE";
      break;

    case CP_GET_CONNECTIONSTATE:
      typeString = "CP_GET_CONNECTIONSTATE";
      break;

    case CP_GET_TERMINALCONNECTIONSTATE:
      typeString = "CP_GET_TERMINALCONNECTIONSTATE";
      break;

    case CP_GET_SESSION:
      typeString = "CP_GET_SESSION";
      break;

   case CP_HOLD_ALL_TERM_CONNECTIONS:
      typeString = "CP_HOLD_ALL_TERM_CONNECTIONS";
      break;

   case CP_UNHOLD_ALL_TERM_CONNECTIONS:
      typeString = "CP_HOLD_ALL_TERM_CONNECTIONS";
      break ;

    case CP_CANCEL_TIMER:
      typeString = "CP_CANCEL_TIMER";
      break;

    case CP_GET_NEXT_CSEQ:
      typeString = "CP_GET_NEXT_CSEQ";
      break;

    default:
      typeString = "?";
      break;
    }
}

// Assignment operator
CpCallManager&
CpCallManager::operator=(const CpCallManager& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

    mAddressForwardingCnt = rhs.mAddressForwardingCnt;
    mpAddressForwards = rhs.mpAddressForwards;
    mDoNotDisturbFlag = rhs.mDoNotDisturbFlag;
    mMsgWaitingFlag = rhs.mMsgWaitingFlag;
    mOfferedTimeOut = rhs.mOfferedTimeOut;

    return *this;
}

CpCall* CpCallManager::findCall(const char* callId)
{
    OsReadLock lock(mCallListMutex);
    UtlDListIterator callIterator(mCallList);
    CpCall* call = NULL;
    UtlVoidPtr* callCollectable = NULL;

    while ((callCollectable = (UtlVoidPtr*) callIterator()))
    {
        call = (CpCall*) callCollectable->getValue();
        if(call && call->hasCallId(callId))
        {
            break;
        }

        call = NULL;
    }

    return(call);
}

void CpCallManager::appendCall(CpCall* call)
{
    OsWriteLock lock(mCallListMutex);
    UtlVoidPtr* callCollectable = new UtlVoidPtr(call);
    mCallList.append(callCollectable);
}

void CpCallManager::pushCall(CpCall* call)
{
    OsWriteLock lock(mCallListMutex);
    UtlVoidPtr* callCollectable = new UtlVoidPtr(call);
    mCallList.insertAt(0, callCollectable);
}


void CpCallManager::setAddressForwarding(int size, PtAddressForwarding *pForwards)
{
    if (size < 1)
        return;

    OsWriteLock lock(mAddressForwardMutex);
    if (mpAddressForwards == 0 && mAddressForwardingCnt == 0)
    {
        mpAddressForwards = new PtAddressForwarding[size];
        mAddressForwardingCnt = size;
        for (int i = 0; i < size; i++)
            mpAddressForwards[i] = PtAddressForwarding(pForwards[i]);
    }
    else
    {
        // Dump the old list
        delete[] mpAddressForwards ;
        mpAddressForwards = NULL ;
        mAddressForwardingCnt = 0 ;

        // Create a new list
        if (size > 0) {
            mpAddressForwards = new PtAddressForwarding[size];
            mAddressForwardingCnt = size;
            for (int k = 0; k < size; k++)
                mpAddressForwards[k] = PtAddressForwarding(pForwards[k]);
        }

    }

    for (int i = 0; i < mAddressForwardingCnt; i++)
    {
        int type = pForwards[i].mForwardingType;

        switch (type)
        {
        case PtAddressForwarding::FORWARD_ON_BUSY:
            mLineBusyBehavior = Connection::FORWARD_ON_BUSY;
            mSipForwardOnBusy = pForwards[i].mDestinationUrl;
            break;
        case PtAddressForwarding::FORWARD_ON_NOANSWER:
            {
                int timeout = pForwards[i].mNoAnswerTimeout;
                mLineAvailableBehavior = Connection::FORWARD_ON_NO_ANSWER;
                mForwardOnNoAnswer = pForwards[i].mDestinationUrl;
                if (timeout > 0)
                    mNoAnswerTimeout = timeout;
                if (mNoAnswerTimeout <= 0)
                    mNoAnswerTimeout = 24;
            }
            break;
        case PtAddressForwarding::FORWARD_UNCONDITIONALLY:
            mLineAvailableBehavior = Connection::FORWARD_UNCONDITIONAL;
            mForwardUnconditional = pForwards[i].mDestinationUrl;
            break;
        }
    }
}

void CpCallManager::cancelAddressForwarding(int size, PtAddressForwarding *pForwards)
{
    mLineBusyBehavior = Connection::BUSY;
    mSipForwardOnBusy = OsUtil::NULL_OS_STRING;
    mLineAvailableBehavior = Connection::RING;
    mForwardOnNoAnswer = OsUtil::NULL_OS_STRING;
    mForwardUnconditional = OsUtil::NULL_OS_STRING;

    int i;
    OsWriteLock lock(mAddressForwardMutex);
    if (pForwards == 0) // cancel all address forwarding
    {
        delete[] mpAddressForwards ;
        mpAddressForwards = NULL ;
        mAddressForwardingCnt = 0 ;
    }
    else
    {
        if (mpAddressForwards && mAddressForwardingCnt > 0)
        {
            for (i = 0; i < size; i++)
            {
                for (int j = 0; j < mAddressForwardingCnt; j++)
                {
                    if (*pForwards == (PtAddressForwarding&) mpAddressForwards[j])
                    {
                        mAddressForwardingCnt--;
                        for (int k = j; k < mAddressForwardingCnt; k++)
                            mpAddressForwards[k] = mpAddressForwards[k + 1];
                        break;
                    }
                }
            }
            if (mAddressForwardingCnt <= 0)
            {
                delete[] mpAddressForwards;
                mpAddressForwards = 0;
                mAddressForwardingCnt = 0;
            }
        }
    }

    for (i = 0; i < mAddressForwardingCnt; i++)
    {
        int type = pForwards[i].mForwardingType;

        switch (type)
        {
        case PtAddressForwarding::FORWARD_ON_BUSY:
            mLineBusyBehavior = Connection::FORWARD_ON_BUSY;
            mSipForwardOnBusy = pForwards[i].mDestinationUrl;
            break;
        case PtAddressForwarding::FORWARD_ON_NOANSWER:
            mLineAvailableBehavior = Connection::FORWARD_ON_NO_ANSWER;
            mForwardOnNoAnswer = pForwards[i].mDestinationUrl;
            if (mNoAnswerTimeout <= 0)
                mNoAnswerTimeout = 24;
            break;
        case PtAddressForwarding::FORWARD_UNCONDITIONALLY:
            mLineAvailableBehavior = Connection::FORWARD_UNCONDITIONAL;
            mForwardUnconditional = pForwards[i].mDestinationUrl;
            break;
        }
    }
}

void CpCallManager::setDoNotDisturb(int flag)
{
    mDoNotDisturbFlag = flag;
}

void CpCallManager::setMessageWaiting(int flag)
{
    mMsgWaitingFlag = flag;
}

void CpCallManager::setOfferedTimeout(int milisec)
{
    mOfferedTimeOut = milisec;
}

/* ============================ ACCESSORS ================================= */

int CpCallManager::getNewMetaEventId()
{
    mLastMetaEventId++;
    return(mLastMetaEventId);
}

/* ============================ INQUIRY =================================== */
UtlBoolean CpCallManager::isCallStateLoggingEnabled()
{
    return(mCallStateLogEnabled);
}


/* //////////////////////////// PROTECTED ///////////////////////////////// */

int CpCallManager::aquireCallIndex()
{
   int index = 0;
   UtlInt matchCallIndexColl;

   // Find the first unused slot
   UtlInt* existingCallIndex = NULL;
   do
   {
      index++;
      matchCallIndexColl.setValue(index);
      existingCallIndex = (UtlInt*) mCallIndices.find(&matchCallIndexColl);

   }
   while(existingCallIndex);

   // Insert the new one
   mCallIndices.insert(new UtlInt(matchCallIndexColl));
   return(index);
}

void CpCallManager::releaseCallIndex(int callIndex)
{
   if(callIndex > 0)
   {
      UtlInt matchCallIndexColl(callIndex);
      UtlInt* callIndexColl = NULL;
      callIndexColl = (UtlInt*) mCallIndices.remove(&matchCallIndexColl);

      if(callIndexColl)
      {
          delete callIndexColl;
      }
      callIndexColl = NULL;
   }
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

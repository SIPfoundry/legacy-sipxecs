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

// APPLICATION INCLUDES
#include <cp/Connection.h>
#include <cp/CpGhostConnection.h>
#include <mi/CpMediaInterface.h>
#include <cp/CpMultiStringMessage.h>
#include <cp/CpCall.h>
#include <net/SdpCodec.h>
#include <net/SipSession.h>
#include <net/CallId.h>
#include <os/OsLock.h>
#include <os/OsMsg.h>
#include <os/OsDatagramSocket.h>
#include <os/OsServerTask.h>
#include <os/OsQueuedEvent.h>
#include <os/OsTimer.h>
#include "os/OsDateTime.h"
#include "os/OsUtil.h"
#include <tao/TaoObjectMap.h>
#include <tao/TaoReference.h>
#include <tao/TaoListenerEventMessage.h>
#include <ptapi/PtConnection.h>
#include <net/TapiMgr.h>

//#define TEST_PRINT 1

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define CONN_DELETE_DELAY_SECS  10    // Number of seconds to wait before a
                                      // connection should be removed from a
                                      // call and deleted.

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
Connection::Connection(CpCallManager* callMgr,
                       CpCall* call,
                       CpMediaInterface* mediaInterface,
                       //UiContext* callUiContext,
                       int offeringDelayMilliSeconds,
                       int availableBehavior,
                       const char* forwardUnconditionalUrl,
                       int busyBehavior, const char* forwardOnBusyUrl,
                       int forwardOnNoAnswerSeconds)
   : mConnectionId(-1)
   , callIdMutex(OsMutex::Q_FIFO)
   , mDeleteAfter(OsTime::OS_INFINITY)
{
#ifdef TEST_PRINT
    UtlString callId;

    if (call) {
       call->getCallId(callId);
       OsSysLog::add(FAC_CP, PRI_DEBUG,
                     "Connection::Connection- %s\n",
                     callId.data());
    } else
       OsSysLog::add(FAC_CP, PRI_DEBUG,
                     "Connection::Connection- call is Null\n");
#endif

    mOfferingDelay = offeringDelayMilliSeconds;
    mLineAvailableBehavior = availableBehavior;
    if(mLineAvailableBehavior == FORWARD_UNCONDITIONAL &&
        forwardUnconditionalUrl != NULL)
    {
        mForwardUnconditional.append(forwardUnconditionalUrl);
    }
    mLineBusyBehavior = busyBehavior;
    if(mLineBusyBehavior == FORWARD_ON_BUSY &&
        forwardOnBusyUrl != NULL)
    {
        mForwardOnBusy.append(forwardOnBusyUrl);
    }
    mForwardOnNoAnswerSeconds = forwardOnNoAnswerSeconds;

    mRemoteIsCallee = FALSE;
    mRemoteRequestedHold = FALSE;
    remoteRtpPort = PORT_NONE;
    sendCodec = -1;
    receiveCodec = -1;
    mLocalConnectionState = CONNECTION_IDLE;
    mRemoteConnectionState = CONNECTION_IDLE;
    mConnectionStateCause = CONNECTION_CAUSE_NORMAL;
    mTerminalConnState = PtTerminalConnection::IDLE;
    mFarEndHoldState = TERMCONNECTION_NONE;
    mResponseCode = 0;
    mResponseText.remove(0);

    mpCallManager = callMgr;
    mpCall = call;
    mpMediaInterface = mediaInterface;
    mConnectionId = -10;
    //mpCallUiContext = callUiContext;

    mpListenerCnt = new TaoReference();
    mpListeners = new TaoObjectMap();

    m_eLastMajor = (SIPX_CALLSTATE_EVENT) -1 ;
    m_eLastMinor = (SIPX_CALLSTATE_CAUSE) -1 ;
    m_eLastAudioMajor = (SIPX_CALLSTATE_EVENT) -1 ;
    m_eLastAudioMinor = (SIPX_CALLSTATE_CAUSE) -1 ;

    CallId::getNewCallId(*this) ;

#ifdef TEST_PRINT
    if (!callId.isNull())
       OsSysLog::add(FAC_CP, PRI_DEBUG,
                     "Connection::Connection -leaving: %s\n",
                     callId.data());
    else
       OsSysLog::add(FAC_CP, PRI_DEBUG,
                     "Connection::Connection -leaving: call is Null\n");
#endif
}

// Copy constructor
Connection::Connection(const Connection& rConnection)
    : UtlString(rConnection)
    , callIdMutex(OsMutex::Q_FIFO)
{
    mpListenerCnt = rConnection.mpListenerCnt;
    mpListeners = rConnection.mpListeners;
}

// Destructor
Connection::~Connection()
{
#ifdef TEST_PRINT
    UtlString callId;
    if (mpCall) {
       mpCall->getCallId(callId);
       OsSysLog::add(FAC_CP, PRI_DEBUG,
                     "Connection destructed: %s\n",
                     callId.data());
    } else
       OsSysLog::add(FAC_CP, PRI_DEBUG,
                     "Connection destructed: call is Null\n");
#endif

   if ( mpListenerCnt )
   {
       delete mpListenerCnt;
       mpListenerCnt = 0;
   }

   if ( mpListeners )
   {
       delete mpListeners;
       mpListeners = 0;
   }


#ifdef TEST_PRINT
    if (!callId.isNull())
       OsSysLog::add(FAC_CP, PRI_DEBUG, "Leaving Connection destructed: %s\n", callId.data());
    else
       OsSysLog::add(FAC_CP, PRI_DEBUG, "Leaving Connection destructed: call is Null\n");
#endif
}

/* ============================ MANIPULATORS ============================== */

void Connection::prepareForSplit()
{
    if ((mpMediaInterface) && (mConnectionId != -1))
    {
        mpMediaInterface->deleteConnection(mConnectionId) ;
    }

    mpCall = NULL ;
    mpMediaInterface = NULL ;
    mConnectionId = -1 ;
}


void Connection::prepareForJoin(CpCall* pNewCall, const char* szLocalAddress, CpMediaInterface* pNewMediaInterface)
{
    mpCall = pNewCall ;
    mpMediaInterface = pNewMediaInterface ;

    mpMediaInterface->createConnection(mConnectionId, szLocalAddress, NULL) ;

    // VIDEO: Need to include window handle!
    // SECURITY:  What about the security attributes?
}


void Connection::setState(int newState, int isLocal, int newCause, int termState)
{
   UtlString oldStateString;
   UtlString newStateString;
   int currentState = isLocal ? mLocalConnectionState : mRemoteConnectionState;
   getStateString(currentState, &oldStateString);
   getStateString(newState, &newStateString);

   int metaEventId = 0;
   int metaEventType = PtEvent::META_EVENT_NONE;
   int numCalls = 0;
   const UtlString* metaEventCallIds = NULL;
   if (mpCall)
   {
      mpCall->getMetaEvent(metaEventId, metaEventType, numCalls,
                           &metaEventCallIds);
   }

   UtlString callId;
   if (mpCall) {
      mpCall->getCallId(callId);
   }
   if (callId.isNull())
      callId="null";

   UtlString strCallName;
   if (mpCall) {
      strCallName = mpCall->getName();
   }
   if (strCallName.isNull())
   {
      strCallName="null";
   }

   if (!isStateTransitionAllowed(newState, currentState))
   {
      // Under some conditions, "invalid" state changes are allowed.
      if (!(!isLocal && metaEventId > 0
            && (metaEventType == PtEvent::META_CALL_TRANSFERRING
                || metaEventType == PtEvent::META_CALL_REPLACING)))
      {
         if (newState == currentState)
         {
            OsSysLog::add(FAC_CP, PRI_DEBUG,
                          "Connection::setState: "
                          "Questionable connection state change - isLocal %d, for call "
                          "'%s' with callid '%s' from %s to %s, cause %d",
                          isLocal, strCallName.data(), callId.data(),
                          oldStateString.data(), newStateString.data(), newCause);
         }
         else
         {
            OsSysLog::add(FAC_CP, PRI_ERR,
                          "Connection::setState: "
                          "Invalid connection state change - isLocal %d, for call "
                          "'%s' with callid '%s' from %s to %s, cause %d",
                          isLocal, strCallName.data(), callId.data(),
                          oldStateString.data(), newStateString.data(), newCause);
         }
         return;
      }
   }

   UtlBoolean bPostStateChange = FALSE;

   if (newState != currentState ||
       newCause != CONNECTION_CAUSE_NORMAL ||
       (newState == currentState &&
           newState == CONNECTION_ALERTING &&
           (0 == isLocal)))
   {
      if (isLocal && newState == CONNECTION_DISCONNECTED)
      {
         if ((   mpCall->canDisconnectConnection(this)
              || newCause == CONNECTION_CAUSE_CANCELLED)
             && metaEventType != PtEvent::META_CALL_TRANSFERRING
             && metaEventType != PtEvent::META_CALL_REPLACING)
         {
            bPostStateChange = TRUE;
         }
      }
      else
      {
         bPostStateChange = TRUE;
      }
   }

   OsSysLog::add(FAC_CP, PRI_DEBUG,
                 "Connection::setState "
                 "Call %s %s state isLocal %d\n"
                 "change\n"
                 "from %s to\n"
                 "\t %s\n"
                 "cause=%d\n"
                 "post change to upper layer %d",
            strCallName.data(),
            callId.data(),
            isLocal,
            oldStateString.data(),
            newStateString.data(),
            newCause,
            bPostStateChange);

   if (bPostStateChange)
   {
      mConnectionStateCause = newCause;
      mTerminalConnState = termState == -1 ? terminalConnectionState(newState) : termState;

      if (isLocal)
      {
         mLocalConnectionState = newState;
      }
      else
      {
         mRemoteConnectionState = newState;
      }

      postTaoListenerMessage(newState, newCause, isLocal);
   }
}

void Connection::setTerminalConnectionState(int newState, int isLocal, int newCause)
{
    mTerminalConnState = newState;
    mConnectionStateCause = newCause;
}


#if 1
int Connection::getState(int isLocal) const
{
   int state;

   if (mRemoteIsCallee)
      state = mRemoteConnectionState;
   else
      state = mLocalConnectionState;

   if ((mLocalConnectionState == CONNECTION_FAILED) &&
       state != mLocalConnectionState)
   {
      UtlString oldStateString, newStateString;
      getStateString(mLocalConnectionState, &oldStateString);
      getStateString(state, &newStateString);
      state = mLocalConnectionState;
   }
   else if ((mRemoteConnectionState == CONNECTION_FAILED) &&
            mRemoteConnectionState != state)
   {
      UtlString oldStateString, newStateString;
      getStateString(mRemoteConnectionState, &oldStateString);
      getStateString(state, &newStateString);
      state = mRemoteConnectionState;
   }

   return state;
}
#endif /* 1 */

int Connection::getState(int isLocal, int& cause) const
{
   cause = mConnectionStateCause;
   int state;
   if (isLocal)
      state = mLocalConnectionState;
   else
      state = mRemoteConnectionState;

   if ((mLocalConnectionState == CONNECTION_FAILED) &&
       state != mLocalConnectionState)
   {
      UtlString oldStateString, newStateString;
      getStateString(mLocalConnectionState, &oldStateString);
      getStateString(state, &newStateString);
      state = mLocalConnectionState;
   }
   else if ((mRemoteConnectionState == CONNECTION_FAILED) &&
            mRemoteConnectionState != state)
   {
      UtlString oldStateString, newStateString;
      getStateString(mRemoteConnectionState, &oldStateString);
      getStateString(state, &newStateString);
      state = mRemoteConnectionState;
   }

   return state;
}

int Connection::getTerminalState(int isLocal) const
{
    int state;

    state = mTerminalConnState;
    return state;
}

void Connection::getStateString(int state, UtlString* stateLabel)
{
    stateLabel->remove(0);

    switch(state)
    {
    case CONNECTION_IDLE:
        stateLabel->append("CONNECTION_IDLE");
        break;

    case CONNECTION_INITIATED:
        stateLabel->append("CONNECTION_INITIATED");
        break;

    case CONNECTION_QUEUED:
        stateLabel->append("CONNECTION_QUEUED");
        break;

    case CONNECTION_OFFERING:
        stateLabel->append("CONNECTION_OFFERING");
        break;

    case CONNECTION_ALERTING:
        stateLabel->append("CONNECTION_ALERTING");
        break;

    case CONNECTION_ESTABLISHED:
        stateLabel->append("CONNECTION_ESTABLISHED");
        break;

    case CONNECTION_FAILED:
        stateLabel->append("CONNECTION_FAILED");
        break;

    case CONNECTION_DISCONNECTED:
        stateLabel->append("CONNECTION_DISCONNECTED");
        break;

    case CONNECTION_DIALING:
        stateLabel->append("CONNECTION_DIALING");
        break;

    default:
        stateLabel->append("CONNECTION_UNKNOWN");
        break;

    }

}

// Assignment operator
Connection&
Connection::operator=(const Connection& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}


void Connection::setLocalAddress(const char* address)
{
    OsLock lock(callIdMutex);
    mLocalAddress.remove(0);
    mLocalAddress.append(address);
}

void Connection::unimplemented(const char* methodName) const
{
    OsSysLog::add(FAC_CP, PRI_WARNING,
        "%s NOT IMPLEMENTED\n",methodName);
}

// Is this connection marked for deletion?
void Connection::markForDeletion()
{
   OsTime timeNow ;
   OsTime deleteAfterSecs(CONN_DELETE_DELAY_SECS, 0) ;

   OsDateTime::getCurTimeSinceBoot(deleteAfterSecs) ;

   mDeleteAfter = timeNow + deleteAfterSecs ;

   OsSysLog::add(FAC_CP, PRI_DEBUG,
       "Connection::markForDeletion connection %p in %d secs (now:%ld then: %ld)",
           this, deleteAfterSecs.seconds(), timeNow.seconds(),
           mDeleteAfter.seconds());
}


void Connection::setMediaInterface(CpMediaInterface* pMediaInterface)
{
    mpMediaInterface = pMediaInterface ;
}



UtlBoolean Connection::validStateTransition(SIPX_CALLSTATE_EVENT eFrom, SIPX_CALLSTATE_EVENT eTo)
{
    UtlBoolean bValid = TRUE ;

    switch (eFrom)
    {
        case DISCONNECTED:
            bValid = (eTo == DESTROYED) ;
            break ;
        case DESTROYED:
            bValid = FALSE ;
            break ;
        default:
            break;
    }

    // Make sure a local focus change doesn't kick off an established event
    if ((eTo == CONNECTED) && (getLocalState() != CONNECTION_ESTABLISHED))
    {
        bValid = FALSE ;
    }

    return bValid ;
}



void Connection::fireSipXEvent(SIPX_CALLSTATE_EVENT eventCode, SIPX_CALLSTATE_CAUSE causeCode, void* pEventData)
{
    UtlString callId ;
    UtlString remoteAddress ;
    SipSession session ;
    UtlBoolean bDuplicateAudio =
            (   eventCode == CALLSTATE_AUDIO_EVENT
             && causeCode == m_eLastAudioMinor) ? TRUE : FALSE;

    // Avoid sending duplicate events
    if ((   (eventCode != m_eLastMajor)
         || (causeCode != m_eLastMinor))
        && validStateTransition(m_eLastMajor, eventCode)
        && !bDuplicateAudio)
    {
        if (eventCode != CALLSTATE_AUDIO_EVENT)
        {
            m_eLastMajor = eventCode;
            m_eLastMinor = causeCode;
        }
        else
        {
            m_eLastAudioMajor = eventCode;
            m_eLastAudioMinor = causeCode;
        }

        getCallId(&callId) ;
        getRemoteAddress(&remoteAddress);
        getSession(session) ;

        TapiMgr::getInstance().fireCallEvent(mpCallManager, callId.data(), &session, remoteAddress.data(), eventCode, causeCode, pEventData) ;
    }
}


/* ============================ ACCESSORS ================================= */
void Connection::getLocalAddress(UtlString* address)
{
    *address = mLocalAddress;
}


void Connection::getCallId(UtlString* callId)
{
    OsLock lock(callIdMutex);

    *callId = connectionCallId ;
}

void Connection::setCallId(const char* callId)
{
    OsLock lock(callIdMutex);

    connectionCallId = callId ;
}

void Connection::getCallerId(UtlString* callerId)
{
    OsLock lock(callIdMutex);

    *callerId = connectionCallerId ;
}

void Connection::setCallerId(const char* callerId)
{
    OsLock lock(callIdMutex);

    connectionCallerId = callerId ;
}


OsStatus Connection::addTaoListener(OsServerTask* pListener,
                                    char* callId,
                                    int ConnectId,
                                    int mask)
{
    if (!mpListenerCnt) mpListenerCnt = new TaoReference();

    if (!mpListeners) mpListeners = new TaoObjectMap();

    if (TAO_IN_USE != mpListeners->insert((TaoObjHandle)pListener, (TaoObjHandle)pListener))
    {
        mpListenerCnt->add();
        return OS_SUCCESS;
    }
    else
        return OS_UNSPECIFIED;
}

void Connection::getResponseText(UtlString& responseText)
{
    responseText.remove(0);
    responseText.append(mResponseText);
}

// Get the time after which this connection can be deleted.  This timespan
// is relative to boot.
OsStatus Connection::getDeleteAfter(OsTime& time)
{
   time = mDeleteAfter ;
   return OS_SUCCESS ;
}

// Get the local state for this connection
int Connection::getLocalState() const
{
   return mLocalConnectionState ;
}

// Get the remote state for this connection
int Connection::getRemoteState() const
{
   return mRemoteConnectionState ;
}



/* ============================ INQUIRY =================================== */

UtlBoolean Connection::isRemoteCallee()
{
   return(mRemoteIsCallee);
}

UtlBoolean Connection::remoteRequestedHold()
{
   return(mRemoteRequestedHold);
}

// Determines if this connection has been marked for deletion and should be
// purged from the call.
UtlBoolean Connection::isMarkedForDeletion() const
{
   return !mDeleteAfter.isInfinite() ;
}


UtlBoolean Connection::isHeld() const
{
    return mFarEndHoldState == TERMCONNECTION_HELD ;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
void Connection::postTaoListenerMessage(int state, int newCause, int isLocal)
{
    int eventId = PtEvent::EVENT_INVALID;
    int termEventId = PtEvent::EVENT_INVALID;
    UtlString causeStr;
    causeStr.remove(0);

#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG, "Connection::postTaoListenerMessage: "
                  "Enter- %s state %d cause %d "
                  "eventid-  %d termeventid %d",
                  (isLocal?"LOCAL":"REMOTE"),
                  state, newCause,
                  eventId, termEventId);
#endif

    switch(state)
    {
    case CONNECTION_IDLE:
        eventId = PtEvent::CONNECTION_CREATED;
        termEventId = PtEvent::TERMINAL_CONNECTION_IDLE;
        break;

    case CONNECTION_INITIATED:
        eventId = PtEvent::CONNECTION_INITIATED;
        termEventId = PtEvent::TERMINAL_CONNECTION_CREATED;
        break;

    case CONNECTION_QUEUED:
        eventId = PtEvent::CONNECTION_QUEUED;
        termEventId = PtEvent::CONNECTION_CREATED;
        break;

    case CONNECTION_OFFERING:
        eventId = PtEvent::CONNECTION_OFFERED;
        break;

    case CONNECTION_DIALING:
        eventId = PtEvent::CONNECTION_DIALING ;
        break;

    case CONNECTION_ALERTING:
        eventId = PtEvent::CONNECTION_ALERTING;
        termEventId = PtEvent::TERMINAL_CONNECTION_RINGING;
        break;

    case CONNECTION_ESTABLISHED:
        eventId = PtEvent::CONNECTION_ESTABLISHED;
        termEventId = PtEvent::TERMINAL_CONNECTION_TALKING;
        break;

    case CONNECTION_FAILED:
        eventId = PtEvent::CONNECTION_FAILED;
        termEventId = PtEvent::TERMINAL_CONNECTION_DROPPED;
        break;

    case CONNECTION_DISCONNECTED:
        eventId = PtEvent::CONNECTION_DISCONNECTED;
        termEventId = PtEvent::TERMINAL_CONNECTION_DROPPED;
        break;

    case PtEvent::TERMINAL_CONNECTION_HELD:
        termEventId = PtEvent::TERMINAL_CONNECTION_HELD;
        break;

    default:
        eventId = PtEvent::CONNECTION_UNKNOWN;
        termEventId = PtEvent::TERMINAL_CONNECTION_UNKNOWN;
        break;

    }

    int cause;
    switch(newCause)
    {
     case CONNECTION_CAUSE_UNKNOWN:
        cause = PtEvent::CAUSE_UNKNOWN;
        causeStr.append("CAUSE_UNKNOWN");
        break;
    case CONNECTION_CAUSE_REDIRECTED:
        cause = PtEvent::CAUSE_REDIRECTED;
        causeStr.append("CAUSE_REDIRECTED");
        break ;

     case CONNECTION_CAUSE_NETWORK_CONGESTION:
        cause = PtEvent::CAUSE_NETWORK_CONGESTION;
        causeStr.append("CAUSE_NETWORK_CONGESTION");
        break;

     case CONNECTION_CAUSE_NETWORK_NOT_OBTAINABLE:
        cause = PtEvent::CAUSE_NETWORK_NOT_OBTAINABLE;
        causeStr.append("CAUSE_NETWORK_NOT_OBTAINABLE");
        break;

     case CONNECTION_CAUSE_DEST_NOT_OBTAINABLE:
        cause = PtEvent::CAUSE_DESTINATION_NOT_OBTAINABLE;
        causeStr.append("CAUSE_DESTINATION_NOT_OBTAINABLE");
        break;

     case CONNECTION_CAUSE_INCOMPATIBLE_DESTINATION:
        cause = PtEvent::CAUSE_INCOMPATIBLE_DESTINATION;
        causeStr.append("CAUSE_INCOMPATIBLE_DESTINATION");
        break;

     case CONNECTION_CAUSE_NOT_ALLOWED:
        cause = PtEvent::CAUSE_NOT_ALLOWED;
        causeStr.append("CAUSE_NOT_ALLOWED");
        break;

     case CONNECTION_CAUSE_NETWORK_NOT_ALLOWED:
        cause = PtEvent::CAUSE_NETWORK_NOT_ALLOWED;
        causeStr.append("CAUSE_NETWORK_NOT_ALLOWED");
        break;

    case CONNECTION_CAUSE_BUSY:
    case CONNECTION_CAUSE_SERVICE_UNAVAILABLE:
        cause = PtEvent::CAUSE_BUSY;
        causeStr.append("CAUSE_BUSY");
        break ;

    case CONNECTION_CAUSE_CANCELLED:
        cause = PtEvent::CAUSE_CALL_CANCELLED;
        causeStr.append("CAUSE_CALL_CANCELLED");
        break ;

    case CONNECTION_CAUSE_TRANSFER:
        cause = PtEvent::CAUSE_TRANSFER;
        causeStr.append("CAUSE_TRANSFER");
        break;

    default:
    case CONNECTION_CAUSE_NORMAL:
        cause = PtEvent::CAUSE_NORMAL;
        causeStr.append("CAUSE_NORMAL");
        break;
    }

    int cnt = 0;
    if (mpListenerCnt)
        cnt = mpListenerCnt->getRef();

    if (cnt > 0)
    {
        TaoObjHandle* pListeners;
        pListeners = new TaoObjHandle[cnt];
        mpListeners->getActiveObjects(pListeners, cnt);

        UtlString callId;

        // Use the connection call id first -- followed by call if
        // unavailable
        getCallId(&callId);                          // arg[0], callId
        if (callId.isNull())
        {
            mpCall->getCallId(callId);
#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG, "Connection::postTaoListenerMessage: "
                          "Connection call id not found, "
                          "Using CpCall Id = %s ",
                          callId.data());
#endif
        }

        callId += TAOMESSAGE_DELIMITER + mLocalAddress;        // arg[1], localAddress

        UtlString remoteAddress;
        getRemoteAddress(&remoteAddress, TRUE);

        if (remoteAddress.isNull())                            // arg[2], remote address
        {
            callId += TAOMESSAGE_DELIMITER + (UtlString)"UNKNOWN";    // not available yet
        }
        else
        {
            callId += TAOMESSAGE_DELIMITER + remoteAddress;
        }

        char buff[128];
        sprintf(buff, "%d", (int)mRemoteIsCallee);
        callId += TAOMESSAGE_DELIMITER + UtlString(buff);    // arg[3], remoteIsCallee

        sprintf(buff, "%d", cause);
        callId += TAOMESSAGE_DELIMITER + UtlString(buff);    // arg[4], cause

        if (mRemoteIsCallee)
        {
            remoteAddress.insert(0, "foreign-terminal-");
            callId += TAOMESSAGE_DELIMITER + remoteAddress;    // arg[5], remote terminal name
        }
        else
        {
            mpCall->getLocalTerminalId(buff, 127);
            callId += TAOMESSAGE_DELIMITER + UtlString(buff);    // arg[5], local terminal name
        }

        if (isLocal)                                        // TAO_OFFER_PARAM_LOCAL_CONNECTION
        {
            callId += TAOMESSAGE_DELIMITER + "1";            // arg[6], isLocal
        }
        else
        {
            callId += TAOMESSAGE_DELIMITER + "0";            // isLocal
        }

        sprintf(buff, "%d", mResponseCode);
        callId += TAOMESSAGE_DELIMITER + UtlString(buff);    // arg[7], SIP response code

        callId += TAOMESSAGE_DELIMITER + mResponseText;        // arg[8], SIP response text

        int argCnt = 9;
        if(mpCall)
        {
            int metaEventId = 0;
            int metaEventType = PtEvent::META_EVENT_NONE;
            int numCalls = 0;
            const UtlString* metaEventCallIds = NULL;
            mpCall->getMetaEvent(metaEventId, metaEventType, numCalls,
                &metaEventCallIds);
            if (metaEventId != PtEvent::META_EVENT_NONE)
            {
                sprintf(buff, "%d", metaEventId);
                callId += TAOMESSAGE_DELIMITER + UtlString(buff);    // arg[9], meta event id
                sprintf(buff, "%d", metaEventType);
                callId += TAOMESSAGE_DELIMITER + UtlString(buff);    // arg[10], meta code
                argCnt += 2;
                for (int i = 0; i < numCalls; i++)
                {
                    if (metaEventCallIds && metaEventCallIds[i])
                    {
                        callId += TAOMESSAGE_DELIMITER + metaEventCallIds[i];    // meta call ids
                        argCnt++;
                    }
                }
            }
        }

        TaoMessage msg(TaoMessage::EVENT,
                       0,
                       0,
                       eventId,
                       0,
                       argCnt,
                       callId);

        UtlString eventIdStr;
        if (eventId != PtEvent::EVENT_INVALID)
        {
            for (int i = 0; i < cnt; i++) // post connection events
            {
                ((OsServerTask*) pListeners[i])->postMessage((OsMsg&)msg);
            }
            mpCall->getStateString(eventId, &eventIdStr);
            mpCallManager->logCallState(callId.data(), eventIdStr.data(), causeStr);
        }

        if (termEventId != PtEvent::EVENT_INVALID)    // post terminal connection events
        {
            msg.setObjHandle(termEventId);
            for (int i = 0; i < cnt; i++)
            {
                ((OsServerTask*) pListeners[i])->postMessage((OsMsg&)msg);
            }

            mpCall->getStateString(termEventId, &eventIdStr);
            mpCallManager->logCallState(callId.data(), eventIdStr.data(), causeStr);

        }

        delete[] pListeners;
        callId.remove(0);
        eventIdStr.remove(0);
        remoteAddress.remove(0);
    }
#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG, "Connection::postTaoListenerMessage: "
                  "Leave- %s state %d cause %d "
                  "eventid-  %d termeventid %d",
                  (isLocal?"LOCAL":"REMOTE"),
                  state, newCause,
                  eventId, termEventId);
#endif

    causeStr.remove(0);
}

void Connection::setOfferingTimer(int milliSeconds)
{
    UtlString    callId;
    SipSession  session ;
    Url         urlTo ;
    UtlString    remoteAddr;

    getSession(session) ;
    session.getCallId(callId) ;
    session.getToUrl(urlTo) ;
    urlTo.toString(remoteAddr) ;

    CpMultiStringMessage* offeringExpiredMessage =
        new CpMultiStringMessage(CpCallManager::CP_OFFERING_EXPIRED,
                    callId.data(), remoteAddr.data());
    OsTimer* timer = new OsTimer((mpCallManager->getMessageQueue()),
            offeringExpiredMessage);
    // Convert from mSeconds to uSeconds
    OsTime timerTime(milliSeconds / 1000, milliSeconds % 1000);
    timer->oneshotAfter(timerTime);
#ifdef TEST_PRINT
    osPrintf("Connection::setOfferingTimer message type: %d %d",
        OsMsg::PHONE_APP, CpCallManager::CP_OFFERING_EXPIRED);
#endif

    callId.remove(0);
    remoteAddr.remove(0);
}

CpMediaInterface* Connection::getMediaInterfacePtr()
{
    return mpMediaInterface;
}

void Connection::setRingingTimer(int seconds)
{
    UtlString callId;
    mpCall->getCallId(callId);
    UtlString remoteAddr;
    getRemoteAddress(&remoteAddr);
    CpMultiStringMessage* offeringExpiredMessage =
        new CpMultiStringMessage(CpCallManager::CP_RINGING_EXPIRED,
                    callId.data(), remoteAddr.data());
    OsTimer* timer = new OsTimer((mpCallManager->getMessageQueue()),
            offeringExpiredMessage);

#ifdef TEST_PRINT
    osPrintf("Setting ringing timeout in %d seconds\n",
        seconds);
#endif

    OsTime timerTime(seconds, 0);
    timer->oneshotAfter(timerTime);
#ifdef TEST_PRINT
    osPrintf("Connection::setRingingTimer message type: %d %d",
        OsMsg::PHONE_APP, CpCallManager::CP_RINGING_EXPIRED);
#endif
    callId.remove(0);
    remoteAddr.remove(0);
}

UtlBoolean Connection::isStateTransitionAllowed(int newState, int oldState)
{
    UtlBoolean isAllowed = TRUE;

#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "Connection::isStateTransitionAllowed: "
                  "state- new %d old %d ",
                  newState, oldState);
#endif

    switch (oldState)
    {
    case CONNECTION_IDLE:
        if (newState == CONNECTION_NETWORK_ALERTING)
        {
            isAllowed = FALSE;
        }
        break;
    case CONNECTION_QUEUED:
    case CONNECTION_OFFERING:
        if (newState != CONNECTION_ALERTING &&
            newState != CONNECTION_ESTABLISHED &&
            newState != CONNECTION_DISCONNECTED &&
            newState != CONNECTION_FAILED &&
            newState != CONNECTION_UNKNOWN)
        {
            isAllowed = FALSE;
        }
        break;
    case CONNECTION_ALERTING:
        if (newState != CONNECTION_ALERTING && // Forked Calls will result in multiple
                                               // provisional responses
            newState != CONNECTION_ESTABLISHED &&
            newState != CONNECTION_DISCONNECTED &&
            newState != CONNECTION_FAILED &&
            newState != CONNECTION_UNKNOWN)
        {
            isAllowed = FALSE;
        }
        break;
    case CONNECTION_ESTABLISHED:
        if (newState != CONNECTION_DISCONNECTED &&
            newState != CONNECTION_FAILED &&
            newState != CONNECTION_UNKNOWN)
        {
            isAllowed = FALSE;
        }
        break;
    case CONNECTION_FAILED:
        if (newState != CONNECTION_DISCONNECTED &&
            newState != CONNECTION_UNKNOWN)
        {
            isAllowed = FALSE;
        }
        break;
    case CONNECTION_DISCONNECTED:
        if (newState != CONNECTION_UNKNOWN)
        {
            isAllowed = FALSE;
        }
        break;
    case CONNECTION_INITIATED:
        if (newState != CONNECTION_DIALING &&
            newState != CONNECTION_ESTABLISHED &&
            newState != CONNECTION_OFFERING &&
            newState != CONNECTION_ALERTING &&
            newState != CONNECTION_DISCONNECTED &&
            newState != CONNECTION_FAILED &&
            newState != CONNECTION_UNKNOWN)
        {
            isAllowed = FALSE;
        }
        break;
    case CONNECTION_DIALING:
        if (newState != CONNECTION_ESTABLISHED &&
            newState != CONNECTION_DISCONNECTED &&
            newState != CONNECTION_FAILED &&
            newState != CONNECTION_UNKNOWN)
        {
            isAllowed = FALSE;
        }
        break;
    case CONNECTION_NETWORK_REACHED:
        if (newState != CONNECTION_NETWORK_ALERTING &&
            newState != CONNECTION_ESTABLISHED &&
            newState != CONNECTION_DISCONNECTED &&
            newState != CONNECTION_FAILED &&
            newState != CONNECTION_UNKNOWN)
        {
            isAllowed = FALSE;
        }
        break;
    case CONNECTION_NETWORK_ALERTING:
        if (newState != CONNECTION_ESTABLISHED &&
            newState != CONNECTION_DISCONNECTED &&
            newState != CONNECTION_FAILED &&
            newState != CONNECTION_UNKNOWN)
        {
            isAllowed = FALSE;
        }
        break;
    default:
    case CONNECTION_UNKNOWN:
        break;
    }

    return isAllowed;
}
/* //////////////////////////// PRIVATE /////////////////////////////////// */

int Connection::terminalConnectionState(int connState)
{
    int state;

    switch(connState)
    {
    case CONNECTION_IDLE:
    case CONNECTION_OFFERING:
    case CONNECTION_INITIATED:
    case CONNECTION_DIALING:
        state = PtTerminalConnection::IDLE;
        break;

    case CONNECTION_QUEUED:
        state = PtTerminalConnection::HELD;
        break;

    case CONNECTION_ALERTING:
        state = PtTerminalConnection::RINGING;
        break;

    case CONNECTION_ESTABLISHED:
        state = PtTerminalConnection::TALKING;
        break;

    case CONNECTION_DISCONNECTED:
        state = PtTerminalConnection::DROPPED;
        break;

    case CONNECTION_FAILED:
    default:
        state = PtTerminalConnection::UNKNOWN;
        break;
    }

    return state;
}

/* ============================ FUNCTIONS ================================= */

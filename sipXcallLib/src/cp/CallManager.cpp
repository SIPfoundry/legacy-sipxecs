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

#include <time.h>
#include <assert.h>

// APPLICATION INCLUDES
#include <cp/CallManager.h>
#include <net/CallId.h>
#include <net/SipMessageEvent.h>
#include <net/SipUserAgent.h>
#include <net/SdpCodec.h>
#include <net/SdpCodecFactory.h>
#include <net/Url.h>
#include <net/SipSession.h>
#include <net/SipDialog.h>
#include <net/SipLineMgr.h>
#include <cp/CpIntMessage.h>
#include <cp/CpStringMessage.h>
#include <cp/CpMultiStringMessage.h>
#include <cp/Connection.h>
#include <mi/CpMediaInterfaceFactory.h>
#include <utl/UtlRegex.h>
#include <os/OsUtil.h>
#include <os/OsConfigDb.h>
#include <os/OsEventMsg.h>
#include <os/OsTimer.h>
#include <os/OsQueuedEvent.h>
#include <os/OsEvent.h>
#include <os/OsReadLock.h>
#include <os/OsWriteLock.h>
#include <net/NameValueTokenizer.h>
#include <cp/CpPeerCall.h>
#include "tao/TaoMessage.h"
#include "tao/TaoProviderAdaptor.h"
#include "tao/TaoString.h"
#include "tao/TaoPhoneComponentAdaptor.h"
#include "ptapi/PtCall.h"
#include "ptapi/PtEvent.h"

// TO_BE_REMOVED
#ifndef EXCLUDE_STREAMING
#include "mp/MpPlayer.h"
#include "mp/MpStreamMsg.h"
#include "mp/MpStreamPlayer.h"
#include "mp/MpStreamQueuePlayer.h"
#include "mp/MpStreamPlaylistPlayer.h"
#include <mp/MpMediaTask.h>
#endif

//#define TEST_PRINT 1

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define MAXIMUM_CALLSTATE_LOG_SIZE 100000
#define CALL_STATUS_FIELD "status"
#define SEND_KEY '#'
char CONVERT_TO_STR[17] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
'*', '#', 'A', 'B', 'C', 'D', 'F'};
/*      _________________________
0--9                0--9
*                     10
#                     11
A--D              12--15
Flash                 16
*/

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CallManager::CallManager(UtlBoolean isRequredUserIdMatch,
                         SipLineMgr* pLineMgrTask,
                         UtlBoolean isEarlyMediaFor180Enabled,
                         SdpCodecFactory* pCodecFactory,
                         int rtpPortStart,
                         int rtpPortEnd,
                         const char* localAddress,
                         const char* publicAddress,
                         SipUserAgent* userAgent,
                         int sipSessionReinviteTimer,
                         PtMGCP* mgcpStackTask,
                         const char* defaultCallExtension,
                         int availableBehavior,
                         const char* unconditionalForwardUrl,
                         int forwardOnNoAnswerSeconds,
                         const char* forwardOnNoAnswerUrl,
                         int busyBehavior,
                         const char* sipForwardOnBusyUrl,
                         OsConfigDb* pConfigDb,
                         CallTypes phonesetOutgoingCallProtocol,
                         int numDialPlanDigits,
                         int holdType,
                         int offeringDelay,
                         const char* locale,
                         int inviteExpireSeconds,
                         int expeditedIpTos,
                         int maxCalls,
                         CpMediaInterfaceFactory* pMediaFactory)
                         : CpCallManager("CallManager-%d", "call",
                         rtpPortStart, rtpPortEnd, localAddress, publicAddress)
                         , mIsEarlyMediaFor180(TRUE)
                         , mpMediaFactory(NULL)
{
#ifdef TEST_PRINT
   OsSysLog::add(FAC_CP, PRI_DEBUG,
                 "CallManager:: %p defaultCallExtension '%s'",
                 this, defaultCallExtension);
#endif
    dialing = FALSE;
    mOffHook = FALSE;
    speakerOn = FALSE;
    flashPending = FALSE;
    mIsEarlyMediaFor180 = isEarlyMediaFor180Enabled;
    mNumDialPlanDigits = numDialPlanDigits;
    mHoldType = holdType;
    mnTotalIncomingCalls = 0;
    mnTotalOutgoingCalls = 0;
    mMaxCalls = maxCalls ;
    mDelayInDeleteCall = 0;

    if (pMediaFactory)
    {
        mpMediaFactory = pMediaFactory;
    }
    else
    {
        assert(false);
    }

    // Instruct the factory to use the specified port range
    mpMediaFactory->getFactoryImplementation()->setRtpPortRange(rtpPortStart, rtpPortEnd) ;
    mStunServer = NULL;

    mLineAvailableBehavior = availableBehavior;
    mOfferedTimeOut = offeringDelay;
    mNoAnswerTimeout = forwardOnNoAnswerSeconds;
    if(forwardOnNoAnswerUrl)
    {
        mForwardOnNoAnswer = forwardOnNoAnswerUrl;
        if (mNoAnswerTimeout < 0)
            mNoAnswerTimeout = 24;  // default
    }
    if(unconditionalForwardUrl)
        mForwardUnconditional = unconditionalForwardUrl;
    mLineBusyBehavior = busyBehavior;
    if(sipForwardOnBusyUrl)
    {
        mSipForwardOnBusy.append(sipForwardOnBusyUrl);
    }
#ifdef TEST
    OsSysLog::add(FAC_CP, PRI_DEBUG, "SIP forward on busy URL: %s\nSIP unconditional forward URL: %s\nSIP no answer timeout:%d URL: %s\n",
        mSipForwardOnBusy.data(), mForwardUnconditional.data(),
        forwardOnNoAnswerSeconds, mForwardOnNoAnswer.data());
#endif

    mLocale = locale ? locale : "";

    if (inviteExpireSeconds > 0 && inviteExpireSeconds < CP_MAXIMUM_RINGING_EXPIRE_SECONDS)
        mInviteExpireSeconds = inviteExpireSeconds;
    else
        mInviteExpireSeconds = CP_MAXIMUM_RINGING_EXPIRE_SECONDS;

    mpLineMgrTask = pLineMgrTask;
    mIsRequredUserIdMatch = isRequredUserIdMatch;
    mExpeditedIpTos = expeditedIpTos;

    // Register with the SIP user agent
    sipUserAgent = userAgent;
    if(sipUserAgent)
    {
        sipUserAgent->addMessageObserver(*(this->getMessageQueue()),
            SIP_INVITE_METHOD,
            TRUE, // want to get requests
            TRUE, // and responses
            TRUE, // Incoming messages
            FALSE); // Don't want to see out going messages
        sipUserAgent->addMessageObserver(*(this->getMessageQueue()),
            SIP_BYE_METHOD,
            TRUE, // want to get requests
            TRUE, // and responses
            TRUE, // Incoming messages
            FALSE); // Don't want to see out going messages
        sipUserAgent->addMessageObserver(*(this->getMessageQueue()),
            SIP_CANCEL_METHOD,
            TRUE, // want to get requests
            TRUE, // and responses
            TRUE, // Incoming messages
            FALSE); // Don't want to see out going messages
        sipUserAgent->addMessageObserver(*(this->getMessageQueue()),
            SIP_ACK_METHOD,
            TRUE, // want to get requests
            FALSE, // no such thing as a ACK response
            TRUE, // Incoming messages
            FALSE); // Don't want to see out going messages
        sipUserAgent->addMessageObserver(*(this->getMessageQueue()),
            SIP_REFER_METHOD,
            TRUE, // want to get requests
            TRUE, // and responses
            TRUE, // Incoming messages
            FALSE); // Don't want to see out going messages
        sipUserAgent->addMessageObserver(*(this->getMessageQueue()),
            SIP_OPTIONS_METHOD,
            FALSE, // don't want to get requests
            TRUE, // do want responses
            TRUE, // Incoming messages
            FALSE); // Don't want to see out going messages
        sipUserAgent->addMessageObserver(*(this->getMessageQueue()),
            SIP_NOTIFY_METHOD,
            TRUE, // do want to get requests
            TRUE, // do want responses
            TRUE, // Incoming messages
            FALSE); // Don't want to see out going messages

        // Allow the "replaces" extension, because CallManager
        // implements the INVITE-with-Replaces logic.
        sipUserAgent->allowExtension(SIP_REPLACES_EXTENSION);

        int sipExpireSeconds = sipUserAgent->getDefaultExpiresSeconds();
        if (mInviteExpireSeconds > sipExpireSeconds) mInviteExpireSeconds = sipExpireSeconds;

    }
    mSipSessionReinviteTimer = sipSessionReinviteTimer;

    if(defaultCallExtension)
    {
        mOutboundLine = defaultCallExtension;
#ifdef TEST_PRINT
       OsSysLog::add(FAC_CP, PRI_DEBUG,
                     "CallManager:: %p setting mOutboundLine to '%s'",
                     this, mOutboundLine.data());
#endif
    }

    // MGCP stack
    mpMgcpStackTask = mgcpStackTask;

    infocusCall = NULL;
    mOutGoingCallType = phonesetOutgoingCallProtocol;
    mLocalAddress = localAddress;

#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager: localAddress: %s mLocalAddress: %s publicAddress: %s mPublicAddress %s\n",
        localAddress, mLocalAddress.data(), publicAddress,
        mPublicAddress.data());
#endif

    mpCodecFactory = pCodecFactory;

#ifdef TEST_PRINT
    // Default the log on
    startCallStateLog();
#else
    // Disable the message log
    stopCallStateLog();
#endif

    mListenerCnt = 0;
    mMaxNumListeners = 20;
    mpListeners = (TaoListenerDb**)malloc(sizeof(TaoListenerDb *)*mMaxNumListeners);

    if (!mpListeners)
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "Unable to allocate listeners\n");
        return;
    }

    for (int i = 0; i < mMaxNumListeners; i++)
        mpListeners[i] = 0;

    // Pre-allocate all of the history memory to minimze fragmentation
    for(int h = 0; h < CP_CALL_HISTORY_LENGTH ; h++)
        mCallManagerHistory[h].capacity(256);

    mMessageEventCount = -1;
    mStunOptions = 0 ;
    mStunKeepAlivePeriodSecs = 0 ;
}

// Copy constructor
CallManager::CallManager(const CallManager& rCallManager) :
CpCallManager("CallManager-%d", "call")
{
}

// Destructor
CallManager::~CallManager()
{
    waitUntilShutDown();
    if(sipUserAgent)
    {
        delete sipUserAgent;
        sipUserAgent = NULL;
    }

    while(getCallStackSize())
    {
        delete popCall();
    }

    if (mMaxNumListeners > 0)  // check if listener exists.
    {
        for (int i = 0; i < mListenerCnt; i++)
        {
            if (mpListeners[i])
            {
                delete mpListeners[i];
                mpListeners[i] = 0;
            }
        }
        free(mpListeners);
    }

    // do not delete the codecFactory it is not owned here

}

/* ============================ MANIPULATORS ============================== */

void CallManager::setOutboundLine(const char* lineUrl)
{
    mOutboundLine = lineUrl ? lineUrl : "";
#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CallManager::setOutboundLine "
                  "%p setting mOutboundLine to '%s'",
                  this, mOutboundLine.data());
#endif
}

OsStatus CallManager::addTaoListener(OsServerTask* pListener,
                                     char* callId,
                                     int ConnectId,
                                     int mask)
{
    OsReadLock lock(mCallListMutex);
    OsStatus rc = OS_UNSPECIFIED;
    if (callId)
    {
        CpCall* handlingCall = findHandlingCall(callId);
        if (handlingCall)
        {
            handlingCall->addTaoListener(pListener, callId, ConnectId, mask);
            rc = OS_SUCCESS;
        }
    }
    else
    {
        if (infocusCall)
        {
            infocusCall->addTaoListener(pListener, callId, ConnectId, mask);
            rc = OS_SUCCESS;
        }

        if (!mCallStack.isEmpty())
        {
            // Create an iterator to sequence through callStack.
            UtlSListIterator iterator(mCallStack);
            UtlVoidPtr* callCollectable;
            CpCall* call;
            callCollectable = (UtlVoidPtr*)iterator();
            while (callCollectable)
            {
                call = (CpCall*)callCollectable->getValue();
                if (call)
                {
                    // Add the listener to it.
                    call->addTaoListener(pListener, callId, ConnectId, mask);
                    rc = OS_SUCCESS;
                }
                callCollectable = (UtlVoidPtr*)iterator();
            }
        }
    }

    rc = addThisListener(pListener, callId, mask);

    return rc;
}


OsStatus CallManager::addThisListener(OsServerTask* pListener,
                                      char* callId,
                                      int mask)
{
    // Check if listener is already added.
    for (int i = 0; i < mListenerCnt; i++)
    {
        // Check whether this listener (defined by PListener and callId) is
        // already in mpListners[].
        if (mpListeners[i] &&
            mpListeners[i]->mpListenerPtr == pListener &&
            (!callId || mpListeners[i]->mName.compareTo(callId) == 0))
        {
            // If so, increment the count for this listener.
            mpListeners[i]->mRef++;
            return OS_SUCCESS;
        }
    }

    // If there is no room for more listeners in mpListeners[]:
    if (mListenerCnt == mMaxNumListeners)
    {
        // Reallocate a larger mpListeners[].
        mMaxNumListeners += 20;
        mpListeners = (TaoListenerDb **)realloc(mpListeners,sizeof(TaoListenerDb *)*mMaxNumListeners);
        for (int loop = mListenerCnt; loop < mMaxNumListeners; loop++)
        {
            mpListeners[loop] = 0 ;
        }
    }

    // Construct a new TaoListenerDb to hold the information about the new
    // listener.
    TaoListenerDb *pListenerDb = new TaoListenerDb();
    // Insert pListener and callId.
    if (callId)
    {
        pListenerDb->mName.append(callId);
    }
    pListenerDb->mpListenerPtr = pListener;
    pListenerDb->mRef = 1;
    // Add it to mpListeners[].
    mpListeners[mListenerCnt++] = pListenerDb;

    return OS_SUCCESS;
}


UtlBoolean CallManager::handleMessage(OsMsg& eventMessage)
{
    int msgType = eventMessage.getMsgType();
    int msgSubType = eventMessage.getMsgSubType();
    UtlBoolean messageProcessed = TRUE;
    UtlString holdCallId;
    UtlBoolean messageConsumed = FALSE;
    CpMediaInterface* pMediaInterface;

    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CallManager::handleMessage  - "
                  "number of current calls (%d) max (%d) msgType %d subType %d",
                  getCallStackSize(), mMaxCalls, msgType, msgSubType);

    switch(msgType)
    {
    case OsMsg::OS_SHUTDOWN:
        messageProcessed = FALSE ; // Let the superclass handle this
        break ;

    case OsMsg::PHONE_APP:
        {
            char eventDescription[100];
            UtlString subTypeString;
            getEventSubTypeString((enum CpCallManager::EventSubTypes)msgSubType,
                subTypeString);
            sprintf(eventDescription, "%s (%d)", subTypeString.data(),
                msgSubType);
            addHistoryEvent(eventDescription);
        }

        switch(msgSubType)
        {
        case CP_MGCP_MESSAGE:
        case CP_MGCP_CAPS_MESSAGE:
        case CP_SIP_MESSAGE:
            {
                OsWriteLock lock(mCallListMutex);
                CpCall* handlingCall = NULL;

                handlingCall = findHandlingCall(eventMessage);

                // This message does not belong to any of the calls
                // If this is an invite for a new call
                // Currently only one call can exist
                if(!handlingCall)
                {
                    UtlString callId;
                    if(msgSubType == CP_SIP_MESSAGE)
                    {
                        const SipMessage* sipMsg = ((SipMessageEvent&)eventMessage).getMessage();
                        if(sipMsg)
                        {
                           UtlString method;
                           sipMsg->getRequestMethod(&method);
                           if(method == SIP_REFER_METHOD)
                           {
                              // If the message is a REFER, generate a new call id for it.
                              // :TODO: Explain why.
                              CallId::getNewCallId(callId);
                           }
                           else
                           {
                              // Otherwise, get the call id from the message.
                              sipMsg->getCallIdField(&callId);
                           }
                           OsSysLog::add(FAC_CP, PRI_DEBUG,
                                         "CallManager::handleMessage "
                                         "Message callid: %s\n",
                                         callId.data());
                        }

                        /////////////////
                        UtlBoolean isUserValid = FALSE;
                        UtlString method;
                        sipMsg->getRequestMethod(&method);

                        if(mpLineMgrTask && mIsRequredUserIdMatch &&
                            method.compareTo(SIP_INVITE_METHOD,UtlString::ignoreCase) == 0)
                        {
                            isUserValid = mpLineMgrTask->isUserIdDefined(sipMsg);
                            if( !isUserValid)
                            {
                                //no such user - return 404
                                SipMessage noSuchUserResponse;
                                noSuchUserResponse.setResponseData(sipMsg,
                                    SIP_NOT_FOUND_CODE,
                                    SIP_NOT_FOUND_TEXT);
                                sipUserAgent->send(noSuchUserResponse);
                            }
                        }
                        else
                        {
                            isUserValid = TRUE;
                        }
                        ////////////////

                        if( isUserValid && CpPeerCall::shouldCreateCall(
                            *sipUserAgent, eventMessage, *mpCodecFactory))
                        {
                            OsSysLog::add(FAC_CP, PRI_DEBUG,
                                          "CallManager::handleMessage  "
                                          "- should create call - number of current calls (%d) max (%d) msgType %d",
                                          getCallStackSize(), mMaxCalls, msgType);

                            // If this call would exceed the limit that we have been
                            // given for calls to handle simultaneously,
                            // send a BUSY_HERE SIP (486) message back to the sender.
                            if(getCallStackSize() >= mMaxCalls)
                            {
                                OsSysLog::add(FAC_CP, PRI_INFO,
                                              "CallManager::handleMessage "
                                              "callStack: sending 486 to INVITE, entries = %ld",
                                              (long)mCallStack.entries());
                                if( (sipMsg->isResponse() == FALSE) &&
                                    (method.compareTo(SIP_ACK_METHOD,UtlString::ignoreCase) != 0) )

                                {
                                    OsSysLog::add(FAC_CP, PRI_ERR,
                                                  "CallManager::handleMessage"
                                                  " number of current calls (%d) >= limit (%d):"
                                                  " sending 486 to INVITE",
                                                  getCallStackSize(), mMaxCalls);

                                    SipMessage busyHereResponse;
                                    busyHereResponse.setInviteBusyData(sipMsg);
                                    sipUserAgent->send(busyHereResponse);
                                }
                            }
                            else
                            {
                                // Create a new SIP call
                                int numCodecs;
                                SdpCodec** codecArray = NULL;
                                getCodecs(numCodecs, codecArray);
                                OsSysLog::add(FAC_CP, PRI_DEBUG,
                                              "CallManager::handleMessage "
                                              "Creating new call for incoming SIP message- number of current calls (%d) max (%d)",
                                              getCallStackSize(), mMaxCalls);

                                UtlString publicAddress;
                                int publicPort;
                                //always use sipUserAgent public address, not the mPublicAddress of this call manager.
                                sipUserAgent->getViaInfo(OsSocket::UDP,publicAddress,publicPort);

                                UtlString localAddress;
                                int port;
                                char szAdapter[256];

                                localAddress = sipMsg->getInterfaceIp();
                                getContactAdapterName(szAdapter, localAddress.data());

                                pMediaInterface = mpMediaFactory->createMediaInterface(
                                    NULL,
                                    localAddress, numCodecs, codecArray,
                                    mLocale.data(), mExpeditedIpTos, mStunServer,
                                    mStunOptions, mStunKeepAlivePeriodSecs);


                                int inviteExpireSeconds;
                                if (sipMsg->getExpiresField(&inviteExpireSeconds) && inviteExpireSeconds > 0)
                                {
                                    if (inviteExpireSeconds > mInviteExpireSeconds)
                                        inviteExpireSeconds = mInviteExpireSeconds;
                                }
                                else
                                    inviteExpireSeconds = mInviteExpireSeconds;

                                handlingCall = new CpPeerCall(mIsEarlyMediaFor180,
                                    this,
                                    pMediaInterface,
                                    aquireCallIndex(),
                                    callId.data(),
                                    sipUserAgent,
                                    mSipSessionReinviteTimer,
                                    mOutboundLine.data(),
                                    mHoldType,
                                    mOfferedTimeOut,
                                    mLineAvailableBehavior,
                                    mForwardUnconditional.data(),
                                    mLineBusyBehavior,
                                    mSipForwardOnBusy.data(),
                                    mNoAnswerTimeout,
                                    mForwardOnNoAnswer.data(),
                                    inviteExpireSeconds);

                                for (int i = 0; i < numCodecs; i++)
                                {
                                    delete codecArray[i];
                                }
                                delete[] codecArray;
                            }
                        }
                    }

                    // If we created a new call
                    if(handlingCall)
                    {
                        handlingCall->start();
                        addTaoListenerToCall(handlingCall);
                        // addToneListener(callId.data(), 0);

                        //if(infocusCall == NULL)
                        //{
                        //    infocusCall = handlingCall;
                        //    infocusCall->inFocus();
                        //}
                        //else
                        // {
                        // Push the new call on the stack
                        pushCall(handlingCall);
                        // }

                        //handlingCall->startMetaEvent( getNewMetaEventId(),
                        //                                        PtEvent::META_CALL_STARTING,
                        //                                        0,
                        //                                        0);
                    }
                } // end new call path

                // Pass on the message if there is a call to process
                if(handlingCall)
                {
#ifdef TEST_PRINT
                    UtlString callIdPr, origCIDpr, targCIDpr;

                    handlingCall->getCallId(callIdPr);
                    handlingCall->getIdOfOrigCall(origCIDpr);
                    handlingCall->getTargetCallId(targCIDpr);

                    OsSysLog::add(FAC_CP, PRI_DEBUG,
                                  "CallManager::handleMessage "
                                  "to CpCall for id='%s' orig='%s' targ='%s'",
                                  callIdPr.data(), origCIDpr.data(), targCIDpr.data());
#endif
                    handlingCall->postMessage(eventMessage);
                    messageProcessed = TRUE;
                }
            }
            break;

        case CP_CALL_EXITED:    // NOTE- Call thread may still be handling CP_DROP msg
            {
                CpCall* call;
				intptr_t callIntptr;
                ((CpIntMessage&)eventMessage).getIntData(callIntptr);
				call = (CpCall*)callIntptr;

                OsSysLog::add(FAC_CP, PRI_DEBUG,
                              "CallManager::handleMessage "
                              "Call EXITING message received: %p "
                              "infofocus: %p - "
                              "number of current calls (%d)\r\n",
                              (void*)call, (void*) infocusCall,
                              getCallStackSize());

#ifdef TEST_PRINT
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                              "CallManager::handleMessage "
                              "stopMetaEvent 3");
#endif
                call->stopMetaEvent();

                mCallListMutex.acquireWrite();
                releaseCallIndex(call->getCallIndex());
                if (infocusCall == call)
                {
                    infocusCall = NULL;
                    // The infocus call is not in the mCallList -- no need to
                    // remove, but we should tell the call that it is no
                    // longer in focus.
                    call->outOfFocus();
                }
                else
                {
                    call = removeCall(call);
                }
                mCallListMutex.releaseWrite();

                if(call)
                {
                    delete call;   // NOTE - destructor changes Call object OS_TASK state
                }

                messageProcessed = TRUE;
                break;
            }

        case CP_DIAL_STRING:
            {
                OsWriteLock lock(mCallListMutex);
                if(infocusCall && dialing)
                {
                    OsSysLog::add(FAC_CP, PRI_DEBUG,
                                  "CallManager::processMessage "
                                  "posting dial string to infocus call\n");
                    ((CpMultiStringMessage&)eventMessage).getString1Data(mDialString) ;
                    infocusCall->postMessage(eventMessage);
                }
                dialing = FALSE;
                messageProcessed = TRUE;
                break;
            }

        case CP_YIELD_FOCUS:
            {
                CpCall* call;
				intptr_t callIntptr;
                ((CpIntMessage&)eventMessage).getIntData(callIntptr);
				call = (CpCall*)callIntptr;

                OsSysLog::add(FAC_CP, PRI_DEBUG,
                              "CallManager::handleMessage "
                              "Call YIELD FOCUS message received: %p",
                              (void*)call);
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                              "CallManager::handleMessage "
                              "infocusCall: %p",
                              infocusCall);
                {
                    OsWriteLock lock(mCallListMutex);
                    if(infocusCall == call)
                    {
                        infocusCall->outOfFocus();
                        pushCall(infocusCall);
                        infocusCall = NULL;
                    }
                }
                messageConsumed = TRUE;
                messageProcessed = TRUE;
                break;
            }
        case CP_GET_FOCUS:
            {
                CpCall* call;
				intptr_t callIntptr;
                ((CpIntMessage&)eventMessage).getIntData(callIntptr);
				call = (CpCall*)callIntptr;
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                              "CallManager::handleMessage "
                              "Call GET FOCUS message received: %p\r\n",
                              (void*)call);
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                              "CallManager::handleMessage "
                              "infocusCall: %p\r\n",
                              infocusCall);
                {
                    OsWriteLock lock(mCallListMutex);
                    if(call && infocusCall != call)
                    {
                        changeCallFocus(call);
                    }
                }
                messageConsumed = TRUE;
                messageProcessed = TRUE;
                break;
            }
        case CP_CREATE_CALL:
            {
                UtlString callId;
                int metaEventId = ((CpMultiStringMessage&)eventMessage).getInt1Data();
                int metaEventType = ((CpMultiStringMessage&)eventMessage).getInt2Data();
                int numCalls = ((CpMultiStringMessage&)eventMessage).getInt3Data();
                UtlBoolean assumeFocusIfNoInfocusCall = ((CpMultiStringMessage&)eventMessage).getInt4Data();
                const char* metaEventCallIds[4];
                UtlString metaCallId0;
                UtlString metaCallId1;
                UtlString metaCallId2;
                UtlString metaCallId3;

                ((CpMultiStringMessage&)eventMessage).getString1Data(callId);
                ((CpMultiStringMessage&)eventMessage).getString2Data(metaCallId0);
                ((CpMultiStringMessage&)eventMessage).getString3Data(metaCallId1);
                ((CpMultiStringMessage&)eventMessage).getString4Data(metaCallId2);
                ((CpMultiStringMessage&)eventMessage).getString5Data(metaCallId3);

                OsSysLog::add(FAC_CP, PRI_DEBUG,
                              "CallManager::handleMessage "
                              "create call %s - number of current calls (%d)\n",
                              callId.data(), getCallStackSize() );

                metaEventCallIds[0] = metaCallId0.data();
                metaEventCallIds[1] = metaCallId1.data();
                metaEventCallIds[2] = metaCallId2.data();
                metaEventCallIds[3] = metaCallId3.data();
                doCreateCall(callId.data(), metaEventId, metaEventType,
                             numCalls, metaEventCallIds, assumeFocusIfNoInfocusCall);

                messageProcessed = TRUE;
                break;
            }

        case CP_GET_CALLS:
            {
                int numCalls = 0;
                UtlString callId;
                UtlSList* callList;
				intptr_t callListIntptr;
                UtlVoidPtr* callCollectable;
                OsProtectedEvent* getCallsEvent = (OsProtectedEvent*) ((CpMultiStringMessage&)eventMessage).getInt1Data();
                getCallsEvent->getIntData(callListIntptr);
				callList = (UtlSList*)callListIntptr;

                if(getCallsEvent)
                {
                    OsReadLock lock(mCallListMutex);
                    // Get the callId for the infocus call
                    if(infocusCall)
                    {
                        infocusCall->getCallId(callId);
                        callList->append(new UtlString(callId));
                        numCalls++;
                    }

                    // Get the callId for the calls in the stack
                    CpCall* call = NULL;

                    UtlSListIterator iterator(mCallStack);
                    callCollectable = (UtlVoidPtr*) iterator();
                    while(callCollectable)
                    {
                        call = (CpCall*) callCollectable->getValue();
                        if(call)
                        {
                            call->getCallId(callId);
                            callList->append(new UtlString(callId));
                            numCalls++;
                        }
                        callCollectable = (UtlVoidPtr*) iterator();
                    }

                    // Signal the caller that we are done.  If the event
                    // has already been signalled from the other side, clean up.
                    if(OS_ALREADY_SIGNALED == getCallsEvent->signal(numCalls))
                    {
                        // The other end must have timed out on the wait
                        callList->destroyAll();
                        delete callList;
                        OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
                        eventMgr->release(getCallsEvent);
                    }
                }
                messageConsumed = TRUE;
                messageProcessed = TRUE;
                break;
            }

        case CP_CONNECT:
            {
                UtlString callId;
                UtlString addressUrl;
                UtlString desiredConnectionCallId ;
                ContactId contactId;
                bool sendPAIheader = FALSE;
                ((CpMultiStringMessage&)eventMessage).getString1Data(callId);
                ((CpMultiStringMessage&)eventMessage).getString2Data(addressUrl);
                ((CpMultiStringMessage&)eventMessage).getString4Data(desiredConnectionCallId);
                contactId = (ContactId) ((CpMultiStringMessage&)eventMessage).getInt1Data();
                void* pDisplay = (void*) ((CpMultiStringMessage&)eventMessage).getInt2Data();
                sendPAIheader = (bool) ((CpMultiStringMessage&)eventMessage).getInt3Data();

                OsSysLog::add(FAC_CP, PRI_DEBUG,
                              "CallManager::handleMessage "
                              "sendPAIheader: %d",
                              sendPAIheader);
                doConnect(callId.data(), addressUrl.data(), desiredConnectionCallId.data(), contactId, pDisplay, sendPAIheader) ;
                messageProcessed = TRUE;
                break;
            }

        case CP_OUTGOING_INFO:
            {
                CpMultiStringMessage& infoMessage = (CpMultiStringMessage&)eventMessage;
                UtlString callId;
                UtlString contentType;
                UtlString sContent;

                infoMessage.getString1Data(callId);
                infoMessage.getString2Data(contentType);
                infoMessage.getString3Data(sContent);
                doSendInfo(callId, contentType, sContent);
                break;
            }

        case CP_SET_INBOUND_CODEC_CPU_LIMIT:
            {
                int iLevel = ((CpMultiStringMessage&)eventMessage).getInt1Data();
                mpCodecFactory->setCodecCPULimit(iLevel) ;
                messageConsumed = TRUE;
                messageProcessed = TRUE;
                break ;
            }
        case CP_ENABLE_STUN:
            {
                UtlString stunServer ;
                int iRefreshPeriod ;
                int iStunOptions ;
                OsNotification* pNotification ;


                CpMultiStringMessage& enableStunMessage = (CpMultiStringMessage&)eventMessage;
                enableStunMessage.getString1Data(stunServer) ;
                iRefreshPeriod = enableStunMessage.getInt1Data() ;
                iStunOptions = enableStunMessage.getInt2Data() ;
                pNotification = (OsNotification*) enableStunMessage.getInt3Data() ;

                doEnableStun(stunServer, iRefreshPeriod, iStunOptions, pNotification) ;
            }
        case CP_ANSWER_CONNECTION:
        case CP_DROP:
        case CP_BLIND_TRANSFER:
        case CP_CONSULT_TRANSFER:
        case CP_CONSULT_TRANSFER_ADDRESS:
        case CP_TRANSFER_CONNECTION:
        case CP_TRANSFER_CONNECTION_STATUS:
        case CP_TRANSFEREE_CONNECTION:
        case CP_TRANSFEREE_CONNECTION_STATUS:
        case CP_HOLD_TERM_CONNECTION:
        case CP_HOLD_ALL_TERM_CONNECTIONS:
        case CP_UNHOLD_ALL_TERM_CONNECTIONS:
        case CP_UNHOLD_TERM_CONNECTION:
        case CP_RENEGOTIATE_CODECS_CONNECTION:
        case CP_RENEGOTIATE_CODECS_ALL_CONNECTIONS:
        case CP_SEND_KEEPALIVE:
        case CP_SET_CODEC_CPU_LIMIT:
        case CP_GET_CODEC_CPU_LIMIT:
        case CP_GET_CODEC_CPU_COST:
        case CP_UNHOLD_LOCAL_TERM_CONNECTION:
        case CP_HOLD_LOCAL_TERM_CONNECTION:
        case CP_START_TONE_TERM_CONNECTION:
        case CP_STOP_TONE_TERM_CONNECTION:
        case CP_PLAY_AUDIO_TERM_CONNECTION:
        case CP_PLAY_BUFFER_TERM_CONNECTION:
        case CP_STOP_AUDIO_TERM_CONNECTION:
        case CP_CREATE_PLAYER:
        case CP_DESTROY_PLAYER:
        case CP_CREATE_PLAYLIST_PLAYER:
        case CP_DESTROY_PLAYLIST_PLAYER:
        case CP_DESTROY_QUEUE_PLAYER:
        case CP_CREATE_QUEUE_PLAYER:
        case CP_SET_PREMIUM_SOUND_CALL:
        case CP_GET_NUM_CONNECTIONS:
        case CP_GET_CONNECTIONS:
        case CP_GET_CALLED_ADDRESSES:
        case CP_GET_CALLING_ADDRESSES:
        case CP_GET_NUM_TERM_CONNECTIONS:
        case CP_GET_TERM_CONNECTIONS:
        case CP_IS_LOCAL_TERM_CONNECTION:
        case CP_ACCEPT_CONNECTION:
        case CP_REJECT_CONNECTION:
        case CP_REDIRECT_CONNECTION:
        case CP_DROP_CONNECTION:
        case CP_FORCE_DROP_CONNECTION:
        case CP_OFFERING_EXPIRED:
        case CP_RINGING_EXPIRED:
        case CP_GET_CALLSTATE:
        case CP_GET_CONNECTIONSTATE:
        case CP_GET_TERMINALCONNECTIONSTATE:
        case CP_GET_SESSION:
        case CP_GET_INVITE:
        case CP_CANCEL_TIMER:
        case CP_GET_NEXT_CSEQ:
        case CP_ADD_TONE_LISTENER:
        case CP_REMOVE_TONE_LISTENER:
        case CP_ENABLE_DTMF_EVENT:
        case CP_DISABLE_DTMF_EVENT:
        case CP_REMOVE_DTMF_EVENT:
        case CP_EZRECORD:
        case CP_SET_OUTBOUND_LINE:
        case CP_STOPRECORD:
        case CP_GET_LOCAL_CONTACTS:
        case CP_GET_MEDIA_CONNECTION_ID:
        case CP_GET_CAN_ADD_PARTY:
        case CP_SPLIT_CONNECTION:
        case CP_JOIN_CONNECTION:
            // Forward the message to the call
            {
                UtlString callId;
                UtlBoolean tooLateToPost = FALSE;
                ((CpMultiStringMessage&)eventMessage).getString1Data(callId);
                OsReadLock lock(mCallListMutex);
                CpCall* call = findHandlingCall(callId);
                if (call)
                {   // eliminate race found in XECS-1859
                    // CM should not post any further messages after CP_DROP
                    // Except for CP_CANCEL_TIMER ala XX-5937 Woof!
                    if (msgSubType != CP_CANCEL_TIMER)
                    {
                        tooLateToPost = call->getDropState();
                    }
#ifdef TEST_PRINT
                    OsSysLog::add(FAC_CP, PRI_DEBUG,
                                  "CallManager::handleMessage "
                                  "is %stoo late to post message %d to '%s'",
                                  (tooLateToPost ? "" : "NOT "),
                                  msgSubType, callId.data());
#endif
                }
                if(!call || tooLateToPost)
                {
                    // The call might have been terminated by asynchronous events.
                    // But output a debugging message, so the programmer can check
                    // to see that the CallId was valid in the past.
                    OsSysLog::add(FAC_CP, PRI_DEBUG,
                                  "CallManager::handleMessage "
                                  "Cannot find CallId: %s to post message: %d",
                                  callId.data(), msgSubType);
                    if( msgSubType == CP_GET_NUM_CONNECTIONS ||
                        msgSubType == CP_GET_CONNECTIONS ||
                        msgSubType == CP_GET_CALLED_ADDRESSES ||
                        msgSubType == CP_GET_CALLING_ADDRESSES ||
                        msgSubType == CP_GET_NUM_TERM_CONNECTIONS ||
                        msgSubType == CP_GET_TERM_CONNECTIONS ||
                        msgSubType == CP_IS_LOCAL_TERM_CONNECTION ||
                        msgSubType == CP_GET_CALLSTATE ||
                        msgSubType == CP_GET_CONNECTIONSTATE ||
                        msgSubType == CP_GET_TERMINALCONNECTIONSTATE ||
                        msgSubType == CP_GET_NEXT_CSEQ ||
                        msgSubType == CP_GET_SESSION ||
                        msgSubType == CP_GET_INVITE ||
                        msgSubType == CP_GET_CODEC_CPU_COST ||
                        msgSubType == CP_GET_CODEC_CPU_LIMIT ||
                        msgSubType == CP_CREATE_PLAYER ||
                        msgSubType == CP_CREATE_PLAYLIST_PLAYER ||
                        msgSubType == CP_CREATE_QUEUE_PLAYER ||
                        msgSubType == CP_DESTROY_PLAYER ||
                        msgSubType == CP_DESTROY_PLAYLIST_PLAYER ||
                        msgSubType == CP_DESTROY_QUEUE_PLAYER ||
                        msgSubType == CP_STOPRECORD ||
                        msgSubType == CP_EZRECORD ||
                        msgSubType == CP_PLAY_BUFFER_TERM_CONNECTION ||
                        msgSubType == CP_GET_LOCAL_CONTACTS ||
                        msgSubType == CP_GET_MEDIA_CONNECTION_ID ||
                        msgSubType == CP_GET_CAN_ADD_PARTY)
                    {
                        // Get the OsProtectedEvent and signal it to go away
                        OsProtectedEvent* eventWithoutCall = (OsProtectedEvent*)
                            ((CpMultiStringMessage&)eventMessage).getInt1Data();
                        if (eventWithoutCall)
                        {
                            OsSysLog::add(FAC_CP, PRI_WARNING,
                                          "CallManager::handleMessage "
                                          "Received a message subtype %d "
                                          "request on invalid callId '%s'; "
                                          "signaled event in message\n",
                                msgSubType, callId.data());

                            // Signal the caller that we are done.  If the event
                            // has already been signalled from the other side, clean up.
                            if(OS_ALREADY_SIGNALED == eventWithoutCall->signal(0))
                            {
                                OsProtectEventMgr* eventMgr =
                                    OsProtectEventMgr::getEventMgr();
                                eventMgr->release(eventWithoutCall);
                                eventWithoutCall = NULL;
                            }
                        }
                        else
                        {
                            OsSysLog::add(FAC_CP, PRI_ERR,
                                          "CallManager::handleMessage "
                                          "Received a message subtype %d request "
                                          "on invalid callId '%s'; no event to signal\n",
                                msgSubType, callId.data());
                        }
                    }
                    else if (msgSubType == CP_REMOVE_DTMF_EVENT)
                    {
                        // 08/19/03 (rschaaf):
                        // The entity requesting the CP_REMOVE_DTMF_EVENT can't delete
                        // the event because it might still be in use.  Instead,
                        // the recipient of the CP_REMOVE_DTMF_EVENT message must take
                        // responsibility for deleting the event.  If the recipient no
                        // longer exists, the CallManager must do the deed.
                        OsQueuedEvent* pEvent = (OsQueuedEvent*)
                            ((CpMultiStringMessage&)eventMessage).getInt1Data();
                        delete pEvent;
                    }
                }
                else
                {
                    if (msgSubType == CP_DROP)
                    {
                        // allow CM to keep call in callStack but prevent CM
                        // from posting any further messages to this call
                        // CP_CALL_EXITED will run clean-up code
                        call->setDropState();
                    }
                    OsStatus r = call->postMessage(eventMessage);
                    assert(r == OS_SUCCESS);
                }
                messageProcessed = TRUE;
                break;
            }

        default:
            {
                OsSysLog::add(FAC_CP, PRI_ERR,
                              "CallManager::handleMessage "
                              "Unknown PHONE_APP CallManager message subType: %d\n",
                              msgSubType);
                messageProcessed = TRUE;
                break;
            }
        }
        break;

        // Timer event expired
    case OsMsg::OS_EVENT:
        // Pull out the OsMsg from the user data and post it
        if(msgSubType == OsEventMsg::NOTIFY)
        {
            OsMsg* timerMsg;
            OsTimer* timer;
	    	void* timerMsgVoid;
	    	intptr_t timerIntptr;

            ((OsEventMsg&)eventMessage).getUserData(timerMsgVoid);
            ((OsEventMsg&)eventMessage).getEventData(timerIntptr);
	    	timerMsg = (OsMsg*)timerMsgVoid;
	    	timer = (OsTimer*)timerIntptr;

            if(timer)
            {
#ifdef TEST_PRINT
                int eventMessageType = timerMsg->getMsgType();
                int eventMessageSubType = timerMsg->getMsgSubType();
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                              "CallManager::handleMessage "
                              "deleting timer for message type: %d %d",
                              eventMessageType, eventMessageSubType);
#endif
                timer->stop();
                delete timer;
                timer = NULL;
            }
            if(timerMsg)
            {
                postMessage(*timerMsg);
                delete timerMsg;
                timerMsg = NULL;
            }
        }
        messageProcessed = TRUE;
        break;
#ifndef EXCLUDE_STREAMING
    case OsMsg::STREAMING_MSG:
        {
            // TO_BE_REMOVED
            MpStreamMsg* pMsg = (MpStreamMsg*) &eventMessage ;
            UtlString callId = pMsg->getTarget() ;

            OsReadLock lock(mCallListMutex);
            CpCall* call = findHandlingCall(callId);
            if(!call)
            {
                // If we cannot find the call, then we must clean up
                // and unblock the caller.
                if (  (msgSubType == MpStreamMsg::STREAM_REALIZE_URL) ||
                    (msgSubType == MpStreamMsg::STREAM_REALIZE_BUFFER))
                {
                    OsNotification* pNotifyHandle = (OsNotification*) pMsg->getPtr1() ;
                    Url* pUrl = (Url*) pMsg->getInt2() ;
                    delete pUrl ;
                    pNotifyHandle->signal(0) ;
                }

            }
            else
            {
               OsStatus s = call->postMessage(eventMessage);
               assert(s == OS_SUCCESS);
            }

            //assert(false);
            break;
        }
#endif

    default:
        {
            OsSysLog::add(FAC_CP, PRI_ERR,
                          "CallManager::handleMessage "
                          "Unknown TYPE %d of CallManager message subType: %d\n", msgType, msgSubType);
            messageProcessed = FALSE;
            break;
        }
    }
#ifdef TEST_PRINT
    printCalls(0);
#endif

    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CallManager::handleMessage - "
                  "leaving- number of current calls (%d) max (%d)",
                  getCallStackSize(), mMaxCalls);
    return(messageProcessed);
}


void CallManager::requestShutdown()
{
    // Need to put a Mutex on the call stack


    UtlSListIterator iterator(mCallStack);
    CpCall* call = NULL;
    UtlVoidPtr* callCollectable;

    while(! mCallStack.isEmpty() && ! iterator.atLast())
    {
        callCollectable = (UtlVoidPtr*) iterator();
        if(callCollectable)
        {
            call = (CpCall*) callCollectable->getValue();
            call->requestShutdown();
        }
    }

    if(infocusCall)
    {
        infocusCall->requestShutdown();
    }

    // Pass the shut down to itself
    OsServerTask::requestShutdown();
    yield();

}

void CallManager::addTaoListenerToCall(CpCall* pCall)
{
    // Check if listener is already added.
    for (int i = 0; i < mListenerCnt; i++)
    {
        if (mpListeners[i] &&
            mpListeners[i]->mpListenerPtr)
        {
            pCall->addTaoListener((OsServerTask*) mpListeners[i]->mpListenerPtr);
        }
    }
}


void CallManager::unhold(const char* callId)
{
    CpStringMessage unholdMessage(CP_OFF_HOLD_CALL, callId);
    postMessage(unholdMessage);
}

void CallManager::createCall(UtlString* callId,
                             int metaEventId,
                             int metaEventType,
                             int numCalls,
                             const char* callIds[],
                             UtlBoolean assumeFocusIfNoInfocusCall)
{
    if(callId->isNull())
    {
       CallId::getNewCallId(*callId);
    }
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CallManager::createCall "
                  "new Id: %s\n", callId->data());
    CpMultiStringMessage callMessage(CP_CREATE_CALL,
                                     callId->data(),
                                     numCalls >= 1 ? callIds[0] : NULL,
                                     numCalls >= 2 ? callIds[1] : NULL,
                                     numCalls >= 3 ? callIds[2] : NULL,
                                     numCalls >= 4 ? callIds[3] : NULL,
                                     metaEventId,
                                     metaEventType,
                                     numCalls,
                                     assumeFocusIfNoInfocusCall);
    postMessage(callMessage);
    mnTotalOutgoingCalls++;

}


OsStatus CallManager::getCalls(int maxCalls, int& numCalls,
                               UtlString callIds[])
{
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    UtlSList* addressList = new UtlSList;
    OsProtectedEvent* callsSet = eventMgr->alloc();
    callsSet->setIntData((intptr_t) addressList);
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    OsStatus returnCode = OS_WAIT_TIMEOUT;
    CpMultiStringMessage getCallsMessage(CP_GET_CALLS, NULL, NULL, NULL,
        NULL, NULL, (intptr_t)callsSet);
    postMessage(getCallsMessage);

    // Wait until the call manager sets the callIDs
    if(callsSet->wait(0, maxEventTime) == OS_SUCCESS)
    {
        int callIndex = 0;
        { // scope the iterator
            UtlSListIterator iterator(*addressList);
            UtlString* callIdCollectable;
            callIdCollectable = (UtlString*)iterator();
            returnCode = OS_SUCCESS;

            while (callIdCollectable)
            {
                if(callIndex >= maxCalls)
                {
                    returnCode = OS_LIMIT_REACHED;
                    break;
                }
                callIds[callIndex] = *callIdCollectable;
                callIndex++;
                callIdCollectable = (UtlString*)iterator();
            }
            numCalls = callIndex;
        } // end of iterator scope
#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CallManager::getCalls "
                      "%d calls\n",
                       numCalls);
#endif
        addressList->destroyAll();
        delete addressList;
        eventMgr->release(callsSet);
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR,
                      "CallManager::getCalls "
                      "TIMED OUT\n");

        // Signal the caller that we are done.  If the event
        // has already been signalled from the other side, clean up.
        if(OS_ALREADY_SIGNALED == callsSet->signal(0))
        {
            addressList->destroyAll();
            delete addressList;
            eventMgr->release(callsSet);
        }
        numCalls = 0;

    }

#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager:getCalls numCalls = %d\n ", numCalls) ;
#endif
    return(returnCode);
}

PtStatus CallManager::connect(const char* callId,
                              const char* toAddressString,
                              const char* fromAddressString,
                              const char* desiredCallIdString,
                              ContactId contactId,
                              const void* pDisplay,
                              const bool sendPAIheader)
{
    UtlString toAddressUrl(toAddressString ? toAddressString : "");
    UtlString fromAddressUrl(fromAddressString ? fromAddressString : "");
    UtlString desiredCallId(desiredCallIdString ? desiredCallIdString : "") ;

    PtStatus returnCode = validateAddress(toAddressUrl);
    if(returnCode == PT_SUCCESS)
    {
        CpMultiStringMessage callMessage(CP_CONNECT, callId, toAddressUrl, fromAddressUrl, desiredCallId,
                                         NULL, contactId, (intptr_t)pDisplay, sendPAIheader);
        postMessage(callMessage);
    }
    return(returnCode);
}

PtStatus CallManager::consult(const char* idleTargetCallId,
                              const char* activeOriginalCallId, const char* originalCallControllerAddress,
                              const char* originalCallControllerTerminalId, const char* consultAddressString,
                              UtlString& targetCallControllerAddress, UtlString& targetCallConsultAddress)
{
    UtlString consultAddressUrl(consultAddressString ? consultAddressString : "");
    PtStatus returnCode = validateAddress(consultAddressUrl);
    if(returnCode == PT_SUCCESS)
    {
        // There is an implied hold for all connections in
        // the original call
        holdAllTerminalConnections(activeOriginalCallId);

        // Not sure if we should explicitly put the consultative
        // call in focus or not.
        // For now we won't
        //unholdTerminalConnection(idleTargetCallId, extension.data(), NULL);

        // Create the consultative connection on the new call
        connect(idleTargetCallId, consultAddressString);

        //targetCallControllerAddress = extension; // temporary kludge
        targetCallControllerAddress = mOutboundLine;
        targetCallConsultAddress = consultAddressUrl;
    }
    return(returnCode);
}

void CallManager::drop(const char* callId)
{
    CpMultiStringMessage callMessage(CP_DROP, callId);
    OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::drop is called for call %s",
                  callId);
    postMessage(callMessage);
}

void CallManager::sendInfo(const char* callId,
                           const char* szContentType,
                           const size_t nContentLength,
                           const char*        szContent)
{
    //    UtlString encodedContent;
    //    NetBase64Codec::encode(nContentLength, (char*) pContent, encodedContent);

    CpMultiStringMessage infoMessage(CP_OUTGOING_INFO, UtlString(callId), UtlString(szContentType), UtlString(szContent, nContentLength));
    postMessage(infoMessage);
}

// Cosultative transfer
PtStatus CallManager::transfer(const char* targetCallId, const char* originalCallId)
{
#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManger::transfer targetCallId=%s, originalCallId=%s\n", targetCallId, originalCallId) ;
#endif

    PtStatus returnCode =  PT_SUCCESS;

    // Make sure the consultative call is on hold
    CpMultiStringMessage allHold(CP_HOLD_ALL_TERM_CONNECTIONS, targetCallId);
    postMessage(allHold);

    // Need to get the To URL from the consultative call
    UtlString addresses[2];
    int numConnections;
    getConnections(targetCallId, 2, numConnections, addresses);

    // It is only valid to have 2 connections (local and transfer target)
    // <2 suggest the target hung up.  >2 suggest we are trying to transfer
    // a conference.
    if (numConnections == 2)
    {
        // Assume the first is always the local connection
        UtlString fromAddress;
        UtlString toAddress;
        //        getToField(targetCallId, addresses[1].data(), toAddress) ;
        toAddress = addresses[1];
        getFromField(targetCallId, addresses[1].data(), fromAddress) ;

        // Construct the replaces header info
        // SIP alert: this is SIP specific and should not be in CallManager

        // Add the Replaces header info to the consultative URL
        UtlString replacesField;
        SipMessage::buildReplacesField(replacesField, targetCallId,
            fromAddress.data(), toAddress.data());

        Url transferTargetUrl(toAddress);
        transferTargetUrl.removeFieldParameters() ;
        transferTargetUrl.setHeaderParameter(SIP_REPLACES_FIELD,
            replacesField.data());
        UtlString transferTargetUrlString;
        transferTargetUrl.toString(transferTargetUrlString);

#ifdef TEST_PRINT
        OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::transfer transferTargetUrlString=%s, targetCallId=%s\n", transferTargetUrlString.data(), targetCallId);
#endif

        // Tell the original call to complete the
        // consultative transfer
        CpMultiStringMessage consultTransfer(CP_CONSULT_TRANSFER,
            originalCallId, transferTargetUrlString.data(), targetCallId);

        postMessage(consultTransfer);
    }
    else
    {
        returnCode = PT_INVALID_STATE;
    }

    return(returnCode);
}

// Transfer an individual participant from one end point to another using
// REFER w/replaces.
PtStatus CallManager::transfer(const char* sourceCallId,
                               const char* sourceAddress,
                               const char* targetCallId,
                               const char* targetAddress,
                               bool        remoteHoldBeforeTransfer)
{
    PtStatus returnCode =  PT_SUCCESS;

    // Place connections on hold
    if (remoteHoldBeforeTransfer)
    {
       CpMultiStringMessage sourceHold(CP_HOLD_TERM_CONNECTION, sourceCallId, sourceAddress);
       postMessage(sourceHold);
       CpMultiStringMessage targetHold(CP_HOLD_TERM_CONNECTION, targetCallId, targetAddress);
       postMessage(targetHold);
    }

    // Construct the replaces header info
    // SIP alert: this is SIP specific and should not be in CallManager
    UtlString fromAddress;
    getFromField(targetCallId, targetAddress, fromAddress);

    // Add the Replaces header info to the consultative URL
    UtlString replacesField;
    SipMessage::buildReplacesField(replacesField, targetCallId, fromAddress,
                                   targetAddress);

    UtlString contactAddress;
    getRemoteContactField(targetCallId, targetAddress, contactAddress);

    Url transferTargetUrl(targetAddress);
    transferTargetUrl.setDisplayName("");
    transferTargetUrl.removeFieldParameters() ;
    transferTargetUrl.setHeaderParameter(SIP_REPLACES_FIELD,
                                         replacesField.data());
    // Add "?Require=replaces" so the transfer attempt is rejected if the
    // target doesn't support INVITE-with-Replaces.
    transferTargetUrl.setHeaderParameter(SIP_REQUIRE_FIELD,
                                         SIP_REPLACES_EXTENSION);
    UtlString transferTargetUrlString;
    transferTargetUrl.toString(transferTargetUrlString);

    // Tell the original call to complete the consultative transfer
    CpMultiStringMessage consultTransfer(CP_CONSULT_TRANSFER_ADDRESS,
                                         sourceCallId, sourceAddress,
                                         targetCallId, targetAddress,
                                         transferTargetUrlString,
                                         0,
                                         // :TODO: Is this parameter used?
                                         // CpPeerCall::handleTransferAddress
                                         // processes these messages but does
                                         // not look at value int2.
                                         /* int2 */ remoteHoldBeforeTransfer
                                         );

    postMessage(consultTransfer);

    return returnCode ;
}




// Split szSourceAddress from szSourceCallId and join it to the specified
// target call id.  The source call/connection MUST be on hold prior
// to initiating the split/join.
PtStatus CallManager::split(const char* szSourceCallId,
                            const char* szSourceAddress,
                            const char* szTargetCallId)
{
    PtStatus status = PT_FAILED ;

    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* splitSuccess = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);

    CpMultiStringMessage splitMessage(CP_SPLIT_CONNECTION,
            szSourceCallId,
            szSourceAddress,
            szTargetCallId,
            NULL,
            NULL,
            (intptr_t) splitSuccess);
    postMessage(splitMessage);

    // Wait until the call finishes split/join
    if(splitSuccess->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t success ;
        splitSuccess->getEventData(success);
        eventMgr->release(splitSuccess);

        if (success)
        {
            status = PT_SUCCESS  ;
        }
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::split TIMED OUT\n");
        // Signal the caller that we are done.  If the event
        // has already been signalled from the other side, clean up.
        if(OS_ALREADY_SIGNALED == splitSuccess->signal(0))
        {
            eventMgr->release(splitSuccess);
        }
    }

    return status ;
}


// Blind transfer?
PtStatus CallManager::transfer_blind(const char* callId, const char* transferToUrl,
                                     UtlString* targetConnectionCallId,
                                     UtlString* targetConnectionAddress,
                                     bool       remoteHoldBeforeTransfer
                                     )
{
    UtlString transferTargetUrl(transferToUrl ? transferToUrl : "");

    PtStatus returnCode = validateAddress(transferTargetUrl);

    if(returnCode == PT_SUCCESS)
    {
        if(targetConnectionAddress)
        {
            *targetConnectionAddress = transferToUrl;
        }
        UtlString targetCallId;
        CallId::getNewCallId(targetCallId);
        if(targetConnectionCallId)
        {
            *targetConnectionCallId = targetCallId;
        }

#ifdef TEST_PRINT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CallManager::transfer "
                      "type: %d transferUrl: \"%s\"\n",
                      mTransferType, transferTargetUrl.data());
#endif

        // CP_BLIND_TRANSFER (i.e. two call blind transfer)
        CpMultiStringMessage transferMessage(CP_BLIND_TRANSFER,
                                             callId,
                                             transferTargetUrl,
                                             targetCallId.data(),
                                             NULL,
                                             NULL,
                                             /* int1 */ getNewMetaEventId(),
                                             /* int2 */ remoteHoldBeforeTransfer);

        postMessage(transferMessage);
    }
    return(returnCode);
}

void CallManager::toneStart(const char* callId, int toneId, UtlBoolean local, UtlBoolean remote)
{
    CpMultiStringMessage startToneMessage(CP_START_TONE_TERM_CONNECTION,
        callId, NULL, NULL, NULL, NULL,
        toneId, local, remote);

    OsStatus s = postMessage(startToneMessage);
    assert(s == OS_SUCCESS);
}

void CallManager::toneStop(const char* callId)
{
    CpMultiStringMessage stopToneMessage(CP_STOP_TONE_TERM_CONNECTION, callId);
    postMessage(stopToneMessage);
}

void CallManager::audioPlay(const char* callId, const char* audioUrl, UtlBoolean repeat, UtlBoolean local, UtlBoolean remote)
{
    CpMultiStringMessage startToneMessage(CP_PLAY_AUDIO_TERM_CONNECTION,
        callId, audioUrl, NULL, NULL, NULL,
        repeat, local, remote);

    postMessage(startToneMessage);
}

void CallManager::bufferPlay(const char* callId, char* audioBuf, int bufSize, int type, UtlBoolean repeat, UtlBoolean local, UtlBoolean remote)
{
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* pEvent = eventMgr->alloc();
    int sTimeout = bufSize / 8000;
    if (sTimeout < CP_MAX_EVENT_WAIT_SECONDS)
      sTimeout = CP_MAX_EVENT_WAIT_SECONDS;

    OsTime maxEventTime(sTimeout, 0);

    CpMultiStringMessage startToneMessage(CP_PLAY_BUFFER_TERM_CONNECTION,
       callId, NULL, NULL, NULL, NULL,
       (intptr_t)pEvent, repeat, local, remote, (intptr_t)audioBuf, bufSize, type);

    OsStatus r =  postMessage(startToneMessage);
    assert(r == OS_SUCCESS);

    // Wait for error response
    if(pEvent->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t success ;
        pEvent->getEventData(success);
        if (OS_ALREADY_SIGNALED == pEvent->signal(0))
        {
            eventMgr->release(pEvent);
        }

        if (success)
        {
           // Do something with this success?
           OsSysLog::add(FAC_CP, PRI_DEBUG,
                         "CallManager::bufferPlay event data = %ld\n",
                         (long)success);
        }
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::bufferPlay TIMED OUT\n");
        // If the event has already been signalled, clean up
        if (OS_ALREADY_SIGNALED == pEvent->signal(0))
        {
            eventMgr->release(pEvent);
        }
    }
}

void CallManager::audioStop(const char* callId)
{
    CpMultiStringMessage stopAudioMessage(CP_STOP_AUDIO_TERM_CONNECTION, callId);
    postMessage(stopAudioMessage);
}

void CallManager::stopPremiumSound(const char* callId)
{
    CpMultiStringMessage premiumSoundMessage(CP_SET_PREMIUM_SOUND_CALL,
        callId, NULL, NULL, NULL, NULL, // strings
        FALSE); // Disabled
    postMessage(premiumSoundMessage);
}


#ifndef EXCLUDE_STREAMING
void CallManager::createPlayer(const char* callId,
                               MpStreamPlaylistPlayer** ppPlayer)
{
    // TO_BE_REMOVED
    OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::createPlayer(MpStreamPlaylistPlayer) for call %s",
                  callId);
    int msgtype = CP_CREATE_PLAYLIST_PLAYER;;

    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* ev = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);

    CpMultiStringMessage msg(msgtype,
        callId, NULL, NULL, NULL, NULL, // strings
        (intptr_t)ev, (intptr_t) ppPlayer, 0); // ints

    postMessage(msg);

    // Wait until the player is created by CpCall
    if(ev->wait(0, maxEventTime) != OS_SUCCESS)
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::createPlayer(MpStreamPlaylistPlayer) TIMED OUT\n");

        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == ev->signal(0))
        {
            eventMgr->release(ev);
        }
    }
    else
    {
        eventMgr->release(ev);
    }

    //assert(false);
}

void CallManager::createPlayer(int type,
                               const char* callId,
                               const char* szStream,
                               int flags,
                               MpStreamPlayer** ppPlayer)
{
    // TO_BE_REMOVED
    int msgtype;

    switch (type)
    {
    case MpPlayer::STREAM_QUEUE_PLAYER:
        msgtype = CP_CREATE_QUEUE_PLAYER;
        break;

    case MpPlayer::STREAM_PLAYER:
    default:
        msgtype = CP_CREATE_PLAYER;
        break;
    }

    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* ev = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    CpMultiStringMessage msg(msgtype,
        callId, szStream, NULL, NULL, NULL, // strings
        (intptr_t)ev, (intptr_t) ppPlayer, flags);   // ints

    postMessage(msg);

    // Wait until the player is created by CpCall
    if(ev->wait(0, maxEventTime) != OS_SUCCESS)
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::createPlayer TIMED OUT\n");

        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == ev->signal(0))
        {
            eventMgr->release(ev);
        }
    }
    else
    {
        eventMgr->release(ev);
    }

    //assert(false);
}


void CallManager::destroyPlayer(const char* callId, MpStreamPlaylistPlayer* pPlayer)
{
    // TO_BE_REMOVED
    OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::destroyPlayer(MpStreamPlaylistPlayer) for call %s",
                  callId);
    int msgtype = CP_DESTROY_PLAYLIST_PLAYER;

    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* ev = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);

    CpMultiStringMessage msg(msgtype,
        callId, NULL, NULL, NULL, NULL, // strings
        (intptr_t) ev, (intptr_t) pPlayer);   // ints

    postMessage(msg);

    // Wait until the media structures are deleted by CpCall
    if(ev->wait(0, maxEventTime) != OS_SUCCESS)
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::destroyPlayer(MpStreamPlaylistPlayer) TIMED OUT\n");

        // If the event has already been signaled, clean up.
        if(OS_ALREADY_SIGNALED == ev->signal(0))
        {
            eventMgr->release(ev);
        }
    }
    else
    {
        eventMgr->release(ev);
    }

    // Delete the object
    delete pPlayer;

    //assert(false);
}


void CallManager::destroyPlayer(int type, const char* callId, MpStreamPlayer* pPlayer)
{
    // TO_BE_REMOVED
    int msgtype;
    switch (type)
    {
    case MpPlayer::STREAM_QUEUE_PLAYER:
        msgtype = CP_DESTROY_QUEUE_PLAYER;
        break;

    case MpPlayer::STREAM_PLAYER:
    default:
        msgtype = CP_DESTROY_PLAYER;
        break;
    }

    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* ev = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);

    CpMultiStringMessage msg(msgtype,
        callId, NULL, NULL, NULL, NULL, // strings
        (intptr_t) ev, (intptr_t) pPlayer);   // ints

    postMessage(msg);

    // Wait until the player is destroyed by CpCall
    if(ev->wait(0, maxEventTime) != OS_SUCCESS)
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::destroyPlayer TIMED OUT\n");

        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == ev->signal(0))
        {
            eventMgr->release(ev);
        }
    }
    else
    {
        eventMgr->release(ev);
    }

    // Delete the object
    switch (type)
    {
    case MpPlayer::STREAM_QUEUE_PLAYER:
        delete (MpStreamQueuePlayer*) pPlayer ;
        break;
    case MpPlayer::STREAM_PLAYER:
    default:
        pPlayer->waitForDestruction() ;
        delete pPlayer ;
        break;
    }

    //assert(false);
}
#endif

void CallManager::setOutboundLineForCall(const char* callId, const char* address, ContactType eType)
{
    CpMultiStringMessage outboundLineMessage(CP_SET_OUTBOUND_LINE, 
                                             callId, address,
                                             NULL, NULL, NULL, (int) eType);

    postMessage(outboundLineMessage);
}

void CallManager::acceptConnection(const char* callId,
                                   const char* address,
                                   ContactType contactType,
                                   const void* hWnd)
{
    CpMultiStringMessage acceptMessage(CP_ACCEPT_CONNECTION, callId, address, NULL, NULL, NULL, (int) contactType, (intptr_t) hWnd);
    postMessage(acceptMessage);
}



void CallManager::rejectConnection(const char* callId,
                                   const char* address,
                                   int errorCode,
                                   const char* errorText)
{
    CpMultiStringMessage acceptMessage(CP_REJECT_CONNECTION, callId, address, errorText, NULL, NULL, errorCode);
    postMessage(acceptMessage);
}

PtStatus CallManager::redirectConnection(const char* callId, const char* address,
                                         const char* forwardAddress)
{
    UtlString forwardAddressUrl(forwardAddress ? forwardAddress : "");
    PtStatus returnCode = validateAddress(forwardAddressUrl);

    if(returnCode == PT_SUCCESS)
    {
        CpMultiStringMessage acceptMessage(CP_REDIRECT_CONNECTION, callId, address,
            forwardAddressUrl.data());
        postMessage(acceptMessage);
    }
    return(returnCode);
}

void CallManager::dropConnection(const char* callId, const char* address)
{
    CpMultiStringMessage acceptMessage(CP_DROP_CONNECTION, callId, address);
    postMessage(acceptMessage);
}

void CallManager::getNumConnections(const char* callId, int& numConnections)
{
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* numConnectionsSet = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    CpMultiStringMessage getNumMessage(CP_GET_NUM_CONNECTIONS, callId, NULL,
        NULL, NULL, NULL,
        (intptr_t)numConnectionsSet);
    postMessage(getNumMessage);

    // Wait until the call sets the number of connections
    if(numConnectionsSet->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t numConnectionsIntptr;
        numConnectionsSet->getEventData(numConnectionsIntptr);
        numConnections = (int)numConnectionsIntptr;
        eventMgr->release(numConnectionsSet);

#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CallManager::getNumConnections "
                      "%d connections\n",
                      numConnections);
#endif
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR,
                      "CallManager::getNumConnections "
                      "TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == numConnectionsSet->signal(0))
        {
            eventMgr->release(numConnectionsSet);
        }
        numConnections = 0;
    }
}

OsStatus CallManager::getConnections(const char* callId, int maxConnections,
                                     int& numConnections, UtlString addresses[])
{
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    UtlSList* addressList = new UtlSList;
    OsProtectedEvent* numConnectionsSet = eventMgr->alloc();
    numConnectionsSet->setIntData((intptr_t) addressList);
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    OsStatus returnCode = OS_WAIT_TIMEOUT;
    CpMultiStringMessage getNumMessage(CP_GET_CONNECTIONS, callId, NULL, NULL,
        NULL, NULL,
        (intptr_t)numConnectionsSet);
    postMessage(getNumMessage);

    // Wait until the call sets the number of connections
    if(numConnectionsSet->wait(0, maxEventTime) == OS_SUCCESS)
    {
        {
            int addressIndex = 0;
            UtlSListIterator iterator(*addressList);
            UtlString* addressCollectable;
            addressCollectable = (UtlString*)iterator();
            returnCode = OS_SUCCESS;

            while (addressCollectable)
            {
                if(addressIndex >= maxConnections)
                {
                    returnCode = OS_LIMIT_REACHED;
                    break;
                }
                addresses[addressIndex] = *addressCollectable;
                addressIndex++;
                addressCollectable = (UtlString*)iterator();
            }
            numConnections = addressIndex;
        }

#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CallManager::getConnections "
                      "%d connections\n",
                      numConnections);
#endif

        addressList->destroyAll();
        delete addressList;
        eventMgr->release(numConnectionsSet);
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getConnections TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == numConnectionsSet->signal(0))
        {
            addressList->destroyAll();
            delete addressList;
            eventMgr->release(numConnectionsSet);
        }
        numConnections = 0;

    }

    return(returnCode);
}

OsStatus CallManager::getCalledAddresses(const char* callId, int maxConnections,
                                         int& numConnections, UtlString addresses[])
{
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    UtlSList* addressList = new UtlSList;
    OsProtectedEvent* numConnectionsSet = eventMgr->alloc();
    numConnectionsSet->setIntData((intptr_t) addressList);
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    OsStatus returnCode = OS_WAIT_TIMEOUT;
    CpMultiStringMessage getNumMessage(CP_GET_CALLED_ADDRESSES, callId, NULL,
        NULL, NULL, NULL,
        (intptr_t)numConnectionsSet);
    postMessage(getNumMessage);

    // Wait until the call sets the number of connections
    if(numConnectionsSet->wait(0, maxEventTime) == OS_SUCCESS)
    {
        int addressIndex = 0;
        {  // set the iterator scope
            UtlSListIterator iterator(*addressList);
            UtlString* addressCollectable;
            addressCollectable = (UtlString*)iterator();
            returnCode = OS_SUCCESS;

            while (addressCollectable)
            {
                if(addressIndex >= maxConnections)
                {
                    returnCode = OS_LIMIT_REACHED;
                    break;
                }
                addresses[addressIndex] = *addressCollectable;
                addressIndex++;
                addressCollectable = (UtlString*)iterator();
            }
            numConnections = addressIndex;
        } // end of interator scope
#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CallManager::getCalledAddresses "
                      "%d addresses\n",
                      numConnections);
#endif

        addressList->destroyAll();
        delete addressList;
        eventMgr->release(numConnectionsSet);
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getCalledAddresses TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == numConnectionsSet->signal(0))
        {
            addressList->destroyAll();
            delete addressList;
            eventMgr->release(numConnectionsSet);
        }
        numConnections = 0;

    }

    return(returnCode);
}

OsStatus CallManager::getCallingAddresses(const char* callId, int maxConnections,
                                          int& numConnections, UtlString addresses[])
{
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    UtlSList* addressList = new UtlSList;
    OsProtectedEvent* numConnectionsSet = eventMgr->alloc();
    numConnectionsSet->setIntData((intptr_t) addressList);
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    OsStatus returnCode = OS_WAIT_TIMEOUT;
    CpMultiStringMessage getNumMessage(CP_GET_CALLING_ADDRESSES, callId, NULL,
        NULL, NULL, NULL,
        (intptr_t)numConnectionsSet);
    postMessage(getNumMessage);

    // Wait until the call sets the number of connections
    if(numConnectionsSet->wait(0, maxEventTime) == OS_SUCCESS)
    {
        int addressIndex = 0;
        UtlSListIterator iterator(*addressList);
        UtlString* addressCollectable;
        addressCollectable = (UtlString*)iterator();
        returnCode = OS_SUCCESS;

        while (addressCollectable)
        {
            if(addressIndex >= maxConnections)
            {
                returnCode = OS_LIMIT_REACHED;
                break;
            }
            addresses[addressIndex] = *addressCollectable;
            addressIndex++;
            addressCollectable = (UtlString*)iterator();
        }
        numConnections = addressIndex;

#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CallManager::getCallingAddresses "
                      "%d addresses\n",
                      numConnections);
#endif

        addressList->destroyAll();
        delete addressList;
        eventMgr->release(numConnectionsSet);
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getCallingAddresses TIMED OUT");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == numConnectionsSet->signal(0))
        {
            addressList->destroyAll();
            delete addressList;
            eventMgr->release(numConnectionsSet);
        }
        numConnections = 0;

    }

    return(returnCode);
}

OsStatus CallManager::getFromField(const char* callId,
                                   const char* address,
                                   UtlString& fromField)
{
    SipSession session;
    OsStatus status = getSession(callId, address, session);

    if(status == OS_SUCCESS)
    {
        Url fromUrl;
        session.getFromUrl(fromUrl);
        fromUrl.toString(fromField);

#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CallManager::getFromField "
                      "%s\n",
                      fromField.data());
#endif
    }

    else
    {
        fromField.remove(0);
    }

    return(status);
}

OsStatus CallManager::getSession(const char* callId,
                                 const char* address,
                                 SipSession& session)
{
   OsSysLog::add(FAC_CP, PRI_DEBUG,
                 "CallManager::getSession "
                 "callId = '%s', address = '%s'",
                 callId, address);
   // CallManager/CpPeerCall SipSession object will be copied
   // to sessionPtr space.
    SipSession* sessionPtr = new SipSession;
#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CallManager::getSession "
                  "allocated session: 0x%p",
                  sessionPtr);
#endif
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* getSessionEvent = eventMgr->alloc();
    getSessionEvent->setIntData((intptr_t) sessionPtr);
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    OsStatus returnCode = OS_WAIT_TIMEOUT;
    CpMultiStringMessage getFieldMessage(CP_GET_SESSION, callId, address,
                                         NULL, NULL, NULL,
                                         (intptr_t)getSessionEvent);
    postMessage(getFieldMessage);

    // Wait until the call sets the session pointer
    if(getSessionEvent->wait(0, maxEventTime) == OS_SUCCESS)
    {
        returnCode = OS_SUCCESS;

        session = *sessionPtr;

        UtlString tCallId;
        session.getCallId(tCallId);
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CallManager::getSession "
                      "deleting 'new'ed' session: %p (callId = %s)",
                      sessionPtr,tCallId.data());

        delete sessionPtr;
        sessionPtr = NULL;
        eventMgr->release(getSessionEvent);
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR,
                      "CallManager::getSession "
                      "TIMED OUT");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == getSessionEvent->signal(0))
        {
#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG,
                          "CallManager::getSession "
                          "deleting timed out session: 0x%p",
                          sessionPtr);
#endif
            delete sessionPtr;
            sessionPtr = NULL;

            eventMgr->release(getSessionEvent);
        }
    }
    return(returnCode);
}


OsStatus CallManager::getSipDialog(const char* callId,
                                   const char* address,
                                   SipDialog& dialog)
{
    OsStatus returnCode = OS_SUCCESS;

    // For now, this is only a wrapper for SipSession and we need to
    // re-implement it when SipSession is deprecated.
    SipSession ssn;

    returnCode = getSession(callId, address, ssn);
    // Copy all the contents into the SipDialog
    if (returnCode == OS_SUCCESS)
    {
       UtlString call;
       ssn.getCallId(call);
       dialog.setCallId(call);

       Url url;
       ssn.getFromUrl(url);
       dialog.setLocalField(url);

       ssn.getToUrl(url);
       dialog.setRemoteField(url);

       ssn.getLocalContact(url);
       dialog.setLocalContact(url);

       ssn.getRemoteContact(url);
       dialog.setRemoteContact(url);

       UtlString uValue;
       ssn.getInitialMethod(uValue);
       dialog.setInitialMethod(uValue);

       ssn.getLocalRequestUri(uValue);
       dialog.setLocalRequestUri(uValue);

       ssn.getRemoteRequestUri(uValue);
       dialog.setRemoteRequestUri(uValue);

       dialog.setLastLocalCseq(ssn.getLastFromCseq());
       dialog.setLastRemoteCseq(ssn.getLastToCseq());
    }

    return(returnCode);
}

OsStatus CallManager::getInvite(const char* callId,
                                const char* address,
                                SipMessage& invite)
{
   OsSysLog::add(FAC_CP, PRI_DEBUG,
                 "CallManager::getInvite "
                 "callId = '%s', address = '%s'",
                 callId, address);
   SipMessage* messagePtr = new SipMessage;
#ifdef TEST_PRINT
   OsSysLog::add(FAC_CP, PRI_DEBUG,
                 "CallManager::getInvite "
                 "allocated message: 0x%p",
                 messagePtr);
#endif
   OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
   OsProtectedEvent* getMessageEvent = eventMgr->alloc();
   getMessageEvent->setIntData((intptr_t) messagePtr);
   OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
   OsStatus returnCode = OS_WAIT_TIMEOUT;
   CpMultiStringMessage getFieldMessage(CP_GET_INVITE, callId, address,
                                        NULL, NULL, NULL,
                                        (intptr_t)getMessageEvent);
   postMessage(getFieldMessage);

   // Wait until the call sets the invite message
   if(getMessageEvent->wait(0, maxEventTime) == OS_SUCCESS)
   {
      returnCode = OS_SUCCESS;

      invite = *messagePtr;

      OsSysLog::add(FAC_CP, PRI_DEBUG,
                    "CallManager::getInvite "
                    "deleting message: %p",
                    messagePtr);

      delete messagePtr;
      messagePtr = NULL;
      eventMgr->release(getMessageEvent);
   }
   else
   {
      OsSysLog::add(FAC_CP, PRI_ERR,
                    "CallManager::getInvite "
                    "TIMED OUT");
      // If the event has already been signalled, clean up.
      if (OS_ALREADY_SIGNALED == getMessageEvent->signal(0))
      {
#ifdef TEST_PRINT
         OsSysLog::add(FAC_CP, PRI_DEBUG,
                       "CallManager::getInvite "
                       "deleting timed out message: %p",
                       messagePtr);
#endif
         delete messagePtr;
         messagePtr = NULL;

         eventMgr->release(getMessageEvent);
      }
   }
   return (returnCode);
}

/**
* Gets the number of lines made available by line manager.
*/
int CallManager::getNumLines()
{
    int iLines = 0;
    if (mpLineMgrTask != NULL)
    {
        iLines = mpLineMgrTask->getNumLines() ;
    }
    return iLines;
}

SipLineMgr* CallManager::getLineManager()
{
    return mpLineMgrTask ;
}

/**
* maxAddressesRequested is the number of addresses requested if available
* numAddressesAvailable is the actual number of addresses available.
* "addresses" is a pre-allocated array of size maxAddressesRequested
*/
OsStatus CallManager::getOutboundAddresses(int maxAddressesRequested,
                                           int& numAddressesAvailable, UtlString** addresses)
{

    OsStatus status = OS_FAILED;

    if (mpLineMgrTask != NULL)
    {
        int iMaxLines = mpLineMgrTask->getNumLines() ;

        SipLine** apLines = new SipLine*[iMaxLines] ;

        for (int i=0; i<iMaxLines; i++)
        {
            apLines[i] = new SipLine() ;
        }

        mpLineMgrTask->getLines(iMaxLines, numAddressesAvailable, apLines) ;

        if (numAddressesAvailable > 0)
        {
            status = OS_SUCCESS;
            for (int i = 0; i < numAddressesAvailable; i++)
            {
                Url urlAddress = apLines[i]->getUserEnteredUrl();
                UtlString strAddress = urlAddress.toString();
                //if the phone is set to Factory Defaults,
                //the UserEnteredUrl comes as sip:4444@
                //in that case we want to use "identity".
                size_t iIndexOfAtSymbol = strAddress.last('@');
                if(iIndexOfAtSymbol == strAddress.length()-1 )
                {
                    urlAddress = apLines[i]->getIdentity() ;
                }
                *addresses[i] = urlAddress.toString() ;
            }
        }

        for (int k=0; k<iMaxLines; k++)
        {
            delete apLines[k] ;
            apLines[k] = NULL ;
        }
        delete[] apLines ;
    }
    return status;
}

OsStatus CallManager::getToField(const char* callId,
                                 const char* address,
                                 UtlString& toField)
{
    SipSession session;
    OsStatus status = getSession(callId, address, session);

    if(status == OS_SUCCESS)
    {
        Url toUrl;
        session.getToUrl(toUrl);
        toUrl.toString(toField);

#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CallManager::getToField "
                      "%s\n",
                      toField.data());
#endif
    }

    else
    {
        toField.remove(0);
    }

    return(status);
}

OsStatus CallManager::getRemoteContactField(const char* callId,
                                            const char* address,
                                            UtlString& remoteContactField)
{
    SipSession session;
    OsStatus status = getSession(callId, address, session);

    if(status == OS_SUCCESS)
    {
        Url contactUrl;
        session.getRemoteContact(contactUrl);
        contactUrl.toString(remoteContactField);

#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CallManager::getRemoteContactField "
                      "'%s'"
                      , remoteContactField.data());
#endif
    }

    else
    {
        remoteContactField.remove(0);
    }

    return status;
}

void CallManager::getNumTerminalConnections(const char* callId, const char* address, int& numConnections)
{
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* numConnectionsSet = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    CpMultiStringMessage getNumMessage(CP_GET_NUM_TERM_CONNECTIONS, callId,
        address, NULL, NULL, NULL,
        (intptr_t)numConnectionsSet);
    postMessage(getNumMessage);

    // Wait until the call sets the number of connections
    if(numConnectionsSet->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t numConnectionsIntptr;
        numConnectionsSet->getEventData(numConnectionsIntptr);
        numConnections = (int)numConnectionsIntptr;

#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CallManager::getNumTerminalConnections "
                      "%d connections\n",
                      numConnections);
#endif

        eventMgr->release(numConnectionsSet);
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR,
                      "CallManager::getNumTerminalConnections "
                      "TIMED OUT");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == numConnectionsSet->signal(0))
        {
            eventMgr->release(numConnectionsSet);
        }
        numConnections = 0;
    }
}

OsStatus CallManager::getTerminalConnections(const char* callId, const char* address, int maxConnections,
                                             int& numConnections, UtlString terminalNames[])
{
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    UtlSList* addressList = new UtlSList;
    OsProtectedEvent* numConnectionsSet = eventMgr->alloc();
    numConnectionsSet->setIntData((intptr_t) addressList);
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    OsStatus returnCode = OS_WAIT_TIMEOUT;
    CpMultiStringMessage getNumMessage(CP_GET_TERM_CONNECTIONS, callId, address,
        NULL, NULL, NULL,
        (intptr_t)numConnectionsSet);
    postMessage(getNumMessage);

    // Wait until the call sets the number of connections
    if(numConnectionsSet->wait(0, maxEventTime) == OS_SUCCESS)
    {
        int terminalIndex = 0;
        UtlSListIterator iterator(*addressList);
        UtlString* terminalNameCollectable;
        terminalNameCollectable = (UtlString*)iterator();
        returnCode = OS_SUCCESS;

        while (terminalNameCollectable)
        {
            if(terminalIndex >= maxConnections)
            {
                returnCode = OS_LIMIT_REACHED;
                break;
            }

#ifdef TEST_PRINT_EVENT
            OsSysLog::add(FAC_CP, PRI_DEBUG,
                          "got terminal: %s\n",
                          terminalNameCollectable->data());
#endif

            terminalNames[terminalIndex] = *terminalNameCollectable;
            terminalIndex++;
            terminalNameCollectable = (UtlString*)iterator();
        }
        numConnections = terminalIndex;

#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CallManager::getTerminalConnections "
                      "%d connections\n",
                      numConnections);
#endif

        addressList->destroyAll();
        delete addressList;
        eventMgr->release(numConnectionsSet);
    }
    else
    {
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == numConnectionsSet->signal(0))
        {
            addressList->destroyAll();
            delete addressList;
            eventMgr->release(numConnectionsSet);
        }
        numConnections = 0;
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getTerminalConnections TIMED OUT\n");

    }

    return(returnCode);
}


// Gets the CPU cost for an individual connection within the specified call.
OsStatus CallManager::getCodecCPUCostCall(const char* callId, int& cost)
{
    OsStatus status = OS_SUCCESS ;

    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* getCodecCPUCostEvent = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    CpMultiStringMessage getCodecCPUCostMsg(CP_GET_CODEC_CPU_COST, callId, NULL,
        NULL, NULL, NULL,
        (intptr_t)getCodecCPUCostEvent);
    postMessage(getCodecCPUCostMsg);


    // Wait until the call sets the number of connections
    if(getCodecCPUCostEvent->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t costIntptr;
        getCodecCPUCostEvent->getEventData(costIntptr);
        cost = (int)costIntptr;
        eventMgr->release(getCodecCPUCostEvent);
    }
    else
    {
        status = OS_BUSY ;
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getCodecCPUCostCall TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == getCodecCPUCostEvent->signal(0))
        {
            eventMgr->release(getCodecCPUCostEvent);
        }
        cost = 0;
    }

    return status ;
}


// Gets the CPU limit for an individual connection within the specified call.
OsStatus CallManager::getCodecCPULimitCall(const char* callId, int& cost)
{
    OsStatus status = OS_SUCCESS ;

    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* getCodecCPULimitEvent = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    CpMultiStringMessage getCodecCPULimitMsg(CP_GET_CODEC_CPU_LIMIT, callId, NULL,
        NULL, NULL, NULL,
        (intptr_t)getCodecCPULimitEvent);
    postMessage(getCodecCPULimitMsg);


    // Wait until the call sets the number of connections
    if(getCodecCPULimitEvent->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t costIntptr;
        getCodecCPULimitEvent->getEventData(costIntptr);
        cost = (int)costIntptr;
        eventMgr->release(getCodecCPULimitEvent);
    }
    else
    {
        status = OS_BUSY ;
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getCodecCPULimitCall TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == getCodecCPULimitEvent->signal(0))
        {
            eventMgr->release(getCodecCPULimitEvent);
        }
        cost = 0;
    }

    return status ;
}


// Sets the CPU codec limit for a call.
OsStatus CallManager::setCodecCPULimitCall(const char* callId,
                                           int limit,
                                           UtlBoolean bRenegotiate)
{
    int iOldLevel = -1 ;

    getCodecCPUCostCall(callId, iOldLevel) ;
    if (iOldLevel != limit)
    {
        CpMultiStringMessage setCPULimitMsg(CP_SET_CODEC_CPU_LIMIT, callId, NULL, NULL, NULL, NULL, limit);
        postMessage(setCPULimitMsg);

        if (bRenegotiate)
            renegotiateCodecsAllTerminalConnections(callId) ;
    }

    return OS_SUCCESS ;
}


// Sets the inbound call CPU limit for codecs
OsStatus CallManager::setInboundCodecCPULimit(int limit)
{
    CpMultiStringMessage setInboundCPULimitMsg(CP_SET_INBOUND_CODEC_CPU_LIMIT, NULL, NULL, NULL, NULL, NULL, limit);
    postMessage(setInboundCPULimitMsg);

    return OS_SUCCESS ;
}

void CallManager::answerTerminalConnection(const char* callId, const char* address, const char* terminalId,
                                           const void* pDisplay)
{
    SIPX_VIDEO_DISPLAY* pDisplayCopy = NULL;

    if (pDisplay)
    {
        pDisplayCopy = new SIPX_VIDEO_DISPLAY(*(SIPX_VIDEO_DISPLAY*)pDisplay);
    }

    CpMultiStringMessage callConnectionMessage(CP_ANSWER_CONNECTION, callId, address, NULL, NULL, NULL, (intptr_t)pDisplayCopy);
    postMessage(callConnectionMessage);
    mnTotalIncomingCalls++;

}

void CallManager::holdTerminalConnection(const char* callId, const char* address, const char* terminalId)
{
    CpMultiStringMessage holdMessage(CP_HOLD_TERM_CONNECTION, callId, address, terminalId);
    postMessage(holdMessage);
}

void CallManager::holdAllTerminalConnections(const char* callId)
{
    CpMultiStringMessage holdMessage(CP_HOLD_ALL_TERM_CONNECTIONS, callId);
    postMessage(holdMessage);
}

void CallManager::holdLocalTerminalConnection(const char* callId)
{
    CpMultiStringMessage holdMessage(CP_HOLD_LOCAL_TERM_CONNECTION, callId);
    postMessage(holdMessage);
}

void CallManager::unholdLocalTerminalConnection(const char* callId)
{
    CpMultiStringMessage holdMessage(CP_UNHOLD_LOCAL_TERM_CONNECTION, callId);
    postMessage(holdMessage);
}

void CallManager::unholdAllTerminalConnections(const char* callId)
{
    // Unhold all of the remote connections
    CpMultiStringMessage unholdMessage(CP_UNHOLD_ALL_TERM_CONNECTIONS, callId);
    postMessage(unholdMessage);

    unholdLocalTerminalConnection(callId) ;
}

void CallManager::unholdTerminalConnection(const char* callId, const char* address, const char* terminalId)
{
    CpMultiStringMessage unholdMessage(CP_UNHOLD_TERM_CONNECTION, callId, address, terminalId);
    postMessage(unholdMessage);
}

void CallManager::renegotiateCodecsTerminalConnection(const char* callId, const char* address, const char* terminalId)
{
    CpMultiStringMessage unholdMessage(CP_RENEGOTIATE_CODECS_CONNECTION, callId, address, terminalId);
    postMessage(unholdMessage);
}

void CallManager::renegotiateCodecsAllTerminalConnections(const char* callId)
{
    CpMultiStringMessage renegotiateMessage(CP_RENEGOTIATE_CODECS_ALL_CONNECTIONS, callId);
    postMessage(renegotiateMessage);
}

void CallManager::sendKeepAlive(const char* callId, UtlBoolean useOptionsForKeepalive)
{
    CpMultiStringMessage keepAliveMessage(CP_SEND_KEEPALIVE, callId, NULL, NULL, NULL, NULL, useOptionsForKeepalive);
    postMessage(keepAliveMessage);
}

UtlBoolean CallManager::isTerminalConnectionLocal(const char* callId, const char* address, const char* terminalId)
{
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    UtlBoolean isLocal;
    OsProtectedEvent* isLocalSet = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    CpMultiStringMessage getNumMessage(CP_IS_LOCAL_TERM_CONNECTION, callId,
        address, terminalId, NULL, NULL,
        (intptr_t)isLocalSet);
    postMessage(getNumMessage);

    // Wait until the call sets the isLocal value
    if(isLocalSet->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t tmpIsLocal;
        isLocalSet->getEventData(tmpIsLocal);
        isLocal = tmpIsLocal;  // workaround conversion issue in newer RW library

#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CallManager::isTerminalConnectionLocal "
                      "%d\n",
                      isLocal);
#endif

        eventMgr->release(isLocalSet);
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR,
                      "CallManager::isTerminalConnectionLocal "
                      "TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == isLocalSet->signal(0))
        {
            eventMgr->release(isLocalSet);
        }
        isLocal = FALSE;
    }
    return(isLocal);
}

OsStatus CallManager::stopRecording(const char* callId)
{
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CallManager::stopRecording "
                  "stopping the recording for call %s",
                  callId);
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* stoprecEvent = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);

    CpMultiStringMessage recordMessage(CP_STOPRECORD, callId,
        NULL, NULL, NULL, NULL, (intptr_t)stoprecEvent,0,0,0);
    postMessage(recordMessage);

    // Wait until the call sets the stop recording event
    if(stoprecEvent->wait(0, maxEventTime) == OS_SUCCESS)
    {
        eventMgr->release(stoprecEvent);
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR,
                      "CallManager::stopRecording "
                      "TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == stoprecEvent->signal(0))
        {
            eventMgr->release(stoprecEvent);
        }
    }

    return OS_SUCCESS;

}

OsStatus CallManager::ezRecord(const char* callId,
                               int ms,
                               int silenceLength,
                               double& duration,
                               const char* fileName,
                               int& dtmfterm,
                               OsProtectedEvent* recordEvent)
{
    OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::ezRecord starting the recording for call %s",
                  callId);
    CpMultiStringMessage recordMessage(CP_EZRECORD,
        callId, fileName, NULL, NULL, NULL,
        (intptr_t)recordEvent, ms, silenceLength, dtmfterm);
    postMessage(recordMessage);

    return OS_SUCCESS;
}


OsStatus CallManager::enableDtmfEvent(const char* callId,
                                      int interDigitSecs,
                                      OsQueuedEvent* dtmfEvent,
                                      UtlBoolean ignoreKeyUp)
{
    CpMultiStringMessage dtmfMessage(CP_ENABLE_DTMF_EVENT, callId,
        NULL, NULL, NULL, NULL,
        (intptr_t)dtmfEvent, interDigitSecs, ignoreKeyUp);
    postMessage(dtmfMessage);
    return OS_SUCCESS;
}

void CallManager::disableDtmfEvent(const char* callId, void* dtmfEvent)
{
    CpMultiStringMessage msg(CP_DISABLE_DTMF_EVENT, callId, NULL, NULL, NULL, NULL, (intptr_t)dtmfEvent);
    postMessage(msg);
}

void CallManager::removeDtmfEvent(const char* callId, void* dtmfEvent)
{
    CpMultiStringMessage msg(CP_REMOVE_DTMF_EVENT, callId, NULL, NULL, NULL, NULL, (intptr_t)dtmfEvent);
    postMessage(msg);
}


void CallManager::addToneListener(const char* callId, void* pListener)
{
    CpMultiStringMessage toneMessage(CP_ADD_TONE_LISTENER, callId, NULL, NULL, NULL, NULL, (intptr_t)pListener);
    postMessage(toneMessage);
}


void CallManager::removeToneListener(const char* callId, void* pListener)
{
    CpMultiStringMessage toneMessage(CP_REMOVE_TONE_LISTENER, callId, NULL, NULL, NULL, NULL, (intptr_t)pListener);
    postMessage(toneMessage);
}


// Assignment operator
CallManager&
CallManager::operator=(const CallManager& rhs)
{
    if (this == &rhs)            // handle the assignment to self case
        return *this;

    return *this;
}

void CallManager::dialString(const char* url)
{
    if(url && strlen(url) > 0)
    {
#ifdef TEST_PRINT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CallManager::dialString "
                      "posting dial string\n");
#endif
        UtlString trimmedUrl(url);
        NameValueTokenizer::frontBackTrim(&trimmedUrl, " \t\n\r");
        CpMultiStringMessage dialEvent(CP_DIAL_STRING, trimmedUrl.data());
        postMessage(dialEvent);
    }
#ifdef TEST
    else
    {
        OsSysLog::add(FAC_CP, PRI_DEBUG, "url NULL or empty string\n");
    }
#endif

}


void CallManager::setTransferType(int type)
{
    mTransferType = type;
}

// Set the maximum number of calls to admit to the system.
void CallManager::setMaxCalls(int maxCalls)
{
    mMaxCalls = maxCalls;
}

// Enable STUN for NAT/Firewall traversal
void CallManager::enableStun(const char* szStunServer,
                             int iKeepAlivePeriodSecs,
                             int stunOptions,
                             OsNotification* pNotification)
{
    CpMultiStringMessage enableStunMessage(CP_ENABLE_STUN, szStunServer, NULL,
            NULL, NULL, NULL, iKeepAlivePeriodSecs, stunOptions, (intptr_t) pNotification) ;
    postMessage(enableStunMessage);
}

/* ============================ ACCESSORS ================================= */
int CallManager::getTransferType()
{
    return(mTransferType);
}

UtlBoolean CallManager::getCallState(const char* callId, int& state)
{
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* callState = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    CpMultiStringMessage getCallStateMessage(CP_GET_CALLSTATE, callId, NULL, NULL,
        NULL, NULL, (intptr_t)callState);
    postMessage(getCallStateMessage);

    // Wait until the call sets the call state
    if(callState->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t stateIntptr;
        callState->getEventData(stateIntptr);
        state = (int)stateIntptr;

#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CallManager::getCallState "
                      "state: %d\n",
                      getConnectionState);
#endif

        eventMgr->release(callState);
        return TRUE;
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR,
                      "CallManager::getCallState "
                      "TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == callState->signal(0))
        {
            eventMgr->release(callState);
        }
        return FALSE;
    }
}

UtlBoolean CallManager::getConnectionState(const char* callId, const char* remoteAddress, int& state)
{
    //:provides the next csequence number for a given call session (leg) if it exists.
    // Note: this does not protect against transaction
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* connectionState = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    CpMultiStringMessage getConnectionStateMessage(CP_GET_CONNECTIONSTATE, callId, remoteAddress, NULL,
        NULL, NULL, (intptr_t)connectionState);
    postMessage(getConnectionStateMessage);

    // Wait until the call sets the connection state
    if(connectionState->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t stateIntptr;
        connectionState->getEventData(stateIntptr);
        state = (int)stateIntptr;

#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CallManager::getConnectionState "
                      "state: %d\n",
                      state);
#endif

        eventMgr->release(connectionState);
        return TRUE;
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getConnectionState TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == connectionState->signal(0))
        {
            eventMgr->release(connectionState);
        }
        return FALSE;
    }
}

UtlBoolean CallManager::getNextSipCseq(const char* callId, const char* remoteAddress, int& nextCseq)
{
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* connectionState = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    CpMultiStringMessage getConnectionStateMessage(CP_GET_NEXT_CSEQ, callId, remoteAddress, NULL,
        NULL, NULL, (intptr_t)connectionState);
    postMessage(getConnectionStateMessage);

    // Wait until the call sets the connection state
    if(connectionState->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t nextCseqIntptr;
        connectionState->getEventData(nextCseqIntptr);
        nextCseq = (int)nextCseqIntptr;

#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CallManager::getConnectionState "
                      "state: %d\n",
                      state);
#endif

        eventMgr->release(connectionState);
        return TRUE;
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getConnectionState TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == connectionState->signal(0))
        {
            eventMgr->release(connectionState);
        }
        nextCseq = -1;
        return FALSE;
    }
}

UtlBoolean CallManager::getTermConnectionState(const char* callId,
                                               const char* address,
                                               const char* terminal,
                                               int& state)
{
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* termConnectionState = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    CpMultiStringMessage getTermConnectionStateMessage(CP_GET_TERMINALCONNECTIONSTATE,
        callId, address, terminal,
        NULL, NULL, (intptr_t)termConnectionState);
    postMessage(getTermConnectionStateMessage);

    // Wait until the call sets the call state
    if(termConnectionState->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t stateIntptr;
        termConnectionState->getEventData(stateIntptr);
        state = (int)stateIntptr;

#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CallManager::getTermConnectionState state: "
                      "%d\n",
                      state);
#endif

        eventMgr->release(termConnectionState);
        return TRUE;
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getTermConnectionState TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == termConnectionState->signal(0))
        {
            eventMgr->release(termConnectionState);
        }
        return FALSE;
    }
}

UtlBoolean CallManager::changeCallFocus(CpCall* callToTakeFocus)
{
    UtlBoolean focusChanged = FALSE;

    if(callToTakeFocus != infocusCall)
    {
        focusChanged = TRUE;
        if(callToTakeFocus)
        {
            callToTakeFocus = removeCall(callToTakeFocus);
            if(callToTakeFocus) callToTakeFocus->inFocus();
        }
        if(infocusCall)
        {
            // Temporary fix so that focus change has happened
            delay(20);


            infocusCall->outOfFocus();
            pushCall(infocusCall);
        }
        infocusCall = callToTakeFocus;
    }
    return(focusChanged);
}

void CallManager::pushCall(CpCall* call)
{
   UtlString callId = "";
   if (call) {
      call->getCallId(callId);
      OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CallManager::pushCall callStack: adding call %p, callId %s, entries = %ld",
                   call, callId.data(), (long)mCallStack.entries());
   }
   mCallStack.insertAt(0, new UtlVoidPtr(call));
#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CallManager::pushCall callStack: adding call %p, entries = %ld",
                  call, (long)mCallStack.entries());
#endif
}

CpCall* CallManager::popCall()
{
    CpCall* call = NULL;
    UtlVoidPtr* callCollectable = (UtlVoidPtr*) mCallStack.get();
    if(callCollectable)
    {
        call = (CpCall*) callCollectable->getValue();
#ifdef TEST_PRINT
        OsSysLog::add(FAC_CP, PRI_INFO,
                      "CallManager::popCall callStack: removing call %p, entries = %ld",
                      call, (long)mCallStack.entries());
#endif
        delete callCollectable;
        callCollectable = NULL;
    }
    return(call);
}

CpCall* CallManager::removeCall(CpCall* call)
{
    UtlString callId = "";
    if (call) {
      call->getCallId(callId);
      OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CallManager::removeCall callStack: removing call %p,callId %s, entries = %ld",
                  call, callId.data(), (long)mCallStack.entries());
    }
    UtlVoidPtr matchCall(call);
    UtlVoidPtr* callCollectable = (UtlVoidPtr*) mCallStack.remove(&matchCall);
    if(callCollectable)
    {
        call = (CpCall*) callCollectable->getValue();
#ifdef TEST_PRINT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CallManager::removeCall callStack: removing call %p, entries = %ld",
                      call, (long)mCallStack.entries());
#endif
        delete callCollectable;
        callCollectable = NULL;
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_DEBUG, "Failed to find call to remove from stack\r\n");
        call = NULL;
    }
    return(call);
}

int CallManager::getCallStackSize()
{
    return(mCallStack.entries());
}

void CallManager::addHistoryEvent(const char* messageLogString)
{
    mMessageEventCount++;
    mCallManagerHistory[mMessageEventCount % CP_CALL_HISTORY_LENGTH] =
        messageLogString;
}

CpCall* CallManager::findHandlingCall(const char* callId)
{
    CpCall* handlingCall = NULL;

    if(infocusCall)
    {
        if(infocusCall->hasCallId(callId))
        {
            handlingCall = infocusCall;
        }
    }

    if(!handlingCall)
    {
        UtlSListIterator iterator(mCallStack);
        UtlVoidPtr* callCollectable;
        CpCall* call;
        callCollectable = (UtlVoidPtr*)iterator();
        while(callCollectable
              && !handlingCall)
        {
            call = (CpCall*)callCollectable->getValue();
            if(call && call->hasCallId(callId))
            {
                handlingCall = call;
            }
            callCollectable = (UtlVoidPtr*)iterator();
        }

    }

    return(handlingCall);
}

CpCall* CallManager::findHandlingCall(int callIndex)
{
    CpCall* handlingCall = NULL;

    if(infocusCall)
    {
        if(infocusCall->getCallIndex() == callIndex)
        {
            handlingCall = infocusCall;
        }
    }

    if(!handlingCall)
    {
        UtlSListIterator iterator(mCallStack);
        UtlVoidPtr* callCollectable;
        CpCall* call;
        callCollectable = (UtlVoidPtr*)iterator();
        while(callCollectable &&
            !handlingCall)
        {
            call = (CpCall*)callCollectable->getValue();
            if(call && call->getCallIndex() == callIndex)
            {
                handlingCall = call;
            }
            callCollectable = (UtlVoidPtr*)iterator();
        }

    }

    return(handlingCall);
}


CpCall* CallManager::findHandlingCall(const OsMsg& eventMessage)
{
    CpCall* handlingCall = NULL;
    CpCall::handleWillingness handlingWeight = CpCall::CP_WILL_NOT_HANDLE;
    CpCall::handleWillingness thisCallHandlingWeight;

    if(infocusCall)
    {
        handlingWeight = infocusCall->willHandleMessage(eventMessage);
        if(handlingWeight != CpCall::CP_WILL_NOT_HANDLE)
        {
            handlingCall = infocusCall;
        }
    }

    if(handlingWeight != CpCall::CP_DEFINITELY_WILL_HANDLE)
    {
        UtlSListIterator iterator(mCallStack);
        UtlVoidPtr* callCollectable;
        CpCall* call;
        callCollectable = (UtlVoidPtr*)iterator();
        while(callCollectable)
        {
            call = (CpCall*)callCollectable->getValue();
            if(call)
            {
                thisCallHandlingWeight =
                    call->willHandleMessage(eventMessage);

                if(thisCallHandlingWeight > handlingWeight)
                {
                    handlingWeight = thisCallHandlingWeight;
                    handlingCall = call;
                }

                if(handlingWeight == CpCall::CP_DEFINITELY_WILL_HANDLE)
                {
                    break;
                }
            }
            callCollectable = (UtlVoidPtr*)iterator();
        }   // done searching all calls for best handler
    }
    return(handlingCall);
}

CpCall* CallManager::findFirstQueuedCall()
{
    CpCall* queuedCall = NULL;
    UtlSListIterator iterator(mCallStack);
    UtlVoidPtr* callCollectable;
    CpCall* call;
    callCollectable = (UtlVoidPtr*)iterator();

    // Go all the way through, the last queued call is the first in line
    while(callCollectable)
    {
        call = (CpCall*)callCollectable->getValue();
        if(call && call->isQueued())
        {
            queuedCall = call;
        }
        callCollectable = (UtlVoidPtr*)iterator();
    }

    return(queuedCall);
}

void CallManager::printCalls(int showHistory)
{
    if (showHistory)
    {
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CallManager::printCalls "
                      "Call Manager message history:\n");
        for(int historyIndex = 0; historyIndex < CP_CALL_HISTORY_LENGTH; historyIndex++)
        {
            if(mMessageEventCount - historyIndex >= 0)
            {
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                              "CallManager::printCalls"
                              " %d) %s\n", mMessageEventCount - historyIndex,
                    (mCallManagerHistory[(mMessageEventCount - historyIndex) % CP_CALL_HISTORY_LENGTH]).data());
            }
        }
        OsSysLog::add(FAC_CP, PRI_DEBUG, "============================\n");
    }

    OsReadLock lock(mCallListMutex);
    if(infocusCall)
    {
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CallManager::printCalls "
                      "infocusCall: %p ",
                      infocusCall);
        //OsSysLog::add(FAC_CP, PRI_DEBUG, "shutting down: %d started: %d suspended: %d\n",
        //    infocusCall->isShuttingDown(),
        //    infocusCall->isStarted(), infocusCall->isSuspended());
        infocusCall->printCall(showHistory);
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CallManager::printCalls "
                      "infocusCall: %p\n",
                      infocusCall);
    }

    int callIndex = 0;

    UtlSListIterator iterator(mCallStack);
    UtlVoidPtr* callCollectable;
    CpCall* call;
    callCollectable = (UtlVoidPtr*)iterator();
    while(callCollectable)
    {
        call = (CpCall*)callCollectable->getValue();
        if(call)
        {
            UtlString callId, origCID, targCID;

            call->getCallId(callId);
            call->getIdOfOrigCall(origCID);
            call->getTargetCallId(targCID);

            OsSysLog::add(FAC_CP, PRI_DEBUG,
                          "CallManager::printCalls "
                          "callStack[%d] = %p id='%s' orig='%s' targ='%s' time=%lu "
                          "%s - %sstarted - %s",
                          callIndex, call, callId.data(), origCID.data(),
                          targCID.data(), call->getElapsedTime(),
                          (call->isShuttingDown() ? "shutting down" : ""),
                          (call->isStarted() ? "" : "not "),
                          (call->isSuspended() ? "suspended" : ""));
            call->printCall(showHistory);
        }
        callCollectable = (UtlVoidPtr*)iterator();
        callIndex++;
    }
    if(callIndex == 0)
    {
        OsSysLog::add(FAC_CP, PRI_DEBUG, "No calls on the stack (max=%d)\n", mMaxCalls);
    }

}

void CallManager::setOutGoingCallType(int callType)
{
    switch(callType)
    {
    case SIP_CALL:
    case MGCP_CALL:
        mOutGoingCallType = callType;
        break;
    default:
        OsSysLog::add(FAC_CP, PRI_WARNING, "CallManger::setOutGoingCallType invalid call type %d\n",
            callType);
        break;
    }
}


void CallManager::startCallStateLog()
{
    mCallStateLogEnabled = TRUE;
#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG, "Call State LOGGING ENABLED\n");
#endif
}

void CallManager::stopCallStateLog()
{
    mCallStateLogEnabled = FALSE;
#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG, "Call State LOGGING DISABLED\n");
#endif
}

void CallManager::setCallStateLogAutoWrite(UtlBoolean state)
{
    mCallStateLogAutoWrite = state;
}

void CallManager::flushCallStateLogAutoWrite()
{
    // Write the previous log messages to the mediaserver log.
    OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::logCallState: %s",
        mCallStateLog.data());
    mCallStateLog.remove(0);
}

void CallManager::clearCallStateLog()
{
    mCallStateLog.remove(0);
}

void CallManager::logCallState(const char* message,
                               const char* eventId,
                               const char* cause)
{

    if(mCallStateLogEnabled)
    {
        if (!message || !eventId || !cause)
            return;

        int len = strlen(message) + strlen(eventId) + strlen(cause);
        if ((mCallStateLog.length() + len + 100) > MAXIMUM_CALLSTATE_LOG_SIZE)
        {
            if (mCallStateLogAutoWrite)
            {
                // If auto-write is enabled, write the previous log messages
                // to the mediaserver log.
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                    "CallManager::logCallState: %s",
                    mCallStateLog.data());
            }
            else
            {
                // Otherwise, report that information was lost.
                // (Don't make this conditional, as it can only be printed
                // if the user has set mCallStateLogEnabled.)
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                    "Call State message cleared because it reached "
                    "max size (%d)", MAXIMUM_CALLSTATE_LOG_SIZE);
            }
            // Clear the call state log.
            mCallStateLog.remove(0);
        }

        // Start the log item with the time in the same timestamp
        // format as the log files.
        {
            OsDateTime now;
            OsDateTime::getCurTime(now);
            UtlString time_string;
            now.getIsoTimeStringZus(time_string);
            mCallStateLog.append(time_string);
        }

        mCallStateLog.append(eventId);

        mCallStateLog.append("\nCause: ");
        mCallStateLog.append(cause);

        TaoString arg(message, TAOMESSAGE_DELIMITER);
        mCallStateLog.append("\nCallId: ");
        mCallStateLog.append(arg[0]);

        mCallStateLog.append("\nLocal Address: ");
        mCallStateLog.append(arg[1]);

        mCallStateLog.append("\nRemote Address: ");
        mCallStateLog.append(arg[2]);

        mCallStateLog.append("\nTerminal Name: ");
        mCallStateLog.append(arg[5]);

        mCallStateLog.append("\nRemote is Callee: ");
        mCallStateLog.append(arg[3]);

        int argCnt = arg.getCnt();
        if (argCnt > 6)
        {
            mCallStateLog.append("\nTerminal Connection local: ");
            mCallStateLog.append(arg[6]);
        }

        if (argCnt > 7)
        {
            mCallStateLog.append("\nSIP Response Code: ");
            mCallStateLog.append(arg[7]);
            mCallStateLog.append("\nSIP Response Text: ");
            mCallStateLog.append(arg[8]);
        }

        if (argCnt > 9)
        {
            mCallStateLog.append("\nMetaEvent Id: ");
            mCallStateLog.append(arg[9]);

            mCallStateLog.append("\nMetaEvent Code: ");
            mCallStateLog.append(arg[10]);

            if (argCnt > 11)
            {
                mCallStateLog.append("\nMeta Event CallId: ");
                for (int i = 11; i < argCnt; i++)
                {
                    mCallStateLog.append("\n\t") ;
                    mCallStateLog.append(arg[i]) ;
                }
            }
        }

        mCallStateLog.append("\n--------------------END--------------------\n");
    }
#ifdef TEST_PRINT
    else
    {
        OsSysLog::add(FAC_CP, PRI_DEBUG, "Call State LOGGING DISABLED\n");
    }
#endif
}

void CallManager::getCallStateLog(UtlString& logData)
{
    logData = mCallStateLog;
}

void CallManager::getAndClearCallStateLog(UtlString& logData)
{
    logData = mCallStateLog;
    mCallStateLog.remove(0);
}

PtStatus CallManager::validateAddress(UtlString& address)
{
    PtStatus returnCode = PT_SUCCESS;

    // Check that we are adhering to one of the address schemes
    // Currently we only support SIP URLs so everything must map
    // to a SIP URL

    RegEx ip4Address("^[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+$");

    // If it is all digits
    RegEx allDigits("^[0-9*]+$");

    if(allDigits.Search(address.data()))
    {
        // There must be a valid default SIP host address (SIP_DIRECTORY_SERVER)
        UtlString directoryServerAddress;
        if(sipUserAgent)
        {
            int port;
            UtlString protocol;
            sipUserAgent->getDirectoryServer(0,&directoryServerAddress, &port,
                &protocol);
        }

        // If there is no host or there is an invalid IP4 address
        // We do not validate DNS host names here so that we do not block
        if(   directoryServerAddress.isNull() // no host
            || (   ip4Address.Search(directoryServerAddress.data())
            && !OsSocket::isIp4Address(directoryServerAddress)
            ))
        {
            returnCode = PT_INVALID_SIP_DIRECTORY_SERVER;
        }

        else
        {
            address.append("@");
            //OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::transfer adding @\n");
        }
    }

    // If it is not all digits it must be a SIP URL
    else
    {
        Url addressUrl(address.data());
        UtlString urlHost;
        addressUrl.getHostAddress(urlHost);
        if(urlHost.isNull())
        {
            returnCode = PT_INVALID_SIP_URL;
        }
        else
        {
            // If the host name is an IP4 address check that it is valid
            if(   ip4Address.Search(urlHost.data())
                && !OsSocket::isIp4Address(urlHost)
                )
            {
                returnCode = PT_INVALID_IP_ADDRESS;
            }

            else
            {
                // It is illegal to have a tag in the
                // To field of an initial INVITE
                addressUrl.removeFieldParameter("tag");
                addressUrl.toString(address);
            }
        }
    }
    return(returnCode);
}

// Get the current number of calls in the system and the maximum number of
// calls to be admitted to the system.
void CallManager::getCalls(int& currentCalls, int& maxCalls)
{
    currentCalls = getCallStackSize();
    maxCalls = mMaxCalls;
}

// The available local contact addresses
OsStatus CallManager::getLocalContactAddresses(const char* callId,
                                               ContactAddress addresses[],
                                               size_t  nMaxAddresses,
                                               size_t& nActaulAddresses)
{
    //:provides the next csequence number for a given call session (leg) if it exists.
    // Note: this does not protect against transaction
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* getLocalContacts = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    CpMultiStringMessage msg(CP_GET_LOCAL_CONTACTS, callId, NULL, NULL,
        NULL, NULL, (intptr_t) getLocalContacts, (intptr_t) addresses, (intptr_t) nMaxAddresses, (intptr_t) &nActaulAddresses);
    postMessage(msg);

    // Wait until the call sets the local contacts state
    if(getLocalContacts->wait(0, maxEventTime) == OS_SUCCESS)
    {
#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CallManager::getLocalContactAddresses "
                      "state: %d\n",
                      state);
#endif
        eventMgr->release(getLocalContacts);
        return OS_SUCCESS;
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR,
                      "CallManager::getLocalContactAddresses "
                      "TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == getLocalContacts->signal(0))
        {
            eventMgr->release(getLocalContacts);
        }
        return OS_BUSY;
    }
}

int CallManager::getMediaConnectionId(const char* szCallId, const char* szRemoteAddress, void** ppInstData)
{
    intptr_t connectionId = -1;
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* getIdEvent = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    CpMultiStringMessage getIdMessage(CP_GET_MEDIA_CONNECTION_ID, szCallId, szRemoteAddress, NULL, NULL, NULL, (intptr_t) getIdEvent, (intptr_t) ppInstData);
    postMessage(getIdMessage);

    // Wait until the call sets the connection id
    if(getIdEvent->wait(0, maxEventTime) == OS_SUCCESS)
    {
        getIdEvent->getEventData(connectionId);
        eventMgr->release(getIdEvent);

    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR,
                      "CallManager::getMediaConnectionId "
                      "TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == getIdEvent->signal(0))
        {
            eventMgr->release(getIdEvent);
        }
        connectionId = -1;
    }
    return connectionId;
}

// Can a new connection be added to the specified call?  This method is
// delegated to the media interface.
UtlBoolean CallManager::canAddConnection(const char* szCallId)
{
    UtlBoolean bCanAdd = FALSE ;

    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* getIdEvent = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    CpMultiStringMessage getIdMessage(CP_GET_CAN_ADD_PARTY, szCallId, NULL, NULL, NULL, NULL, (intptr_t) getIdEvent);
    postMessage(getIdMessage);

    // Wait until the call sets the number of connections
    if(getIdEvent->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t eventData ;
        getIdEvent->getEventData(eventData);
        eventMgr->release(getIdEvent);
        bCanAdd = (UtlBoolean) eventData ;

    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::canAddConnection TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == getIdEvent->signal(0))
        {
            eventMgr->release(getIdEvent);
        }
    }

    return bCanAdd ;

}

// Gets the media interface factory used by the call manager
CpMediaInterfaceFactory* CallManager::getMediaInterfaceFactory()
{
    return mpMediaFactory;
}

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PRIVATE /////////////////////////////////// */

void CallManager::doCreateCall(const char* callId,
                               int metaEventId,
                               int metaEventType,
                               int numMetaEventCalls,
                               const char* metaEventCallIds[],
                               UtlBoolean assumeFocusIfNoInfocusCall)
{
    CpCall* call = findHandlingCall(callId);
    if(call)
    {
        // This is generally bad.  The call should not exist.
        OsSysLog::add(FAC_CP, PRI_ERR,
                      "CallManager::doCreateCall "
                      "cannot create call. CallId: %s already exists.\n",
                      callId);
    }
    else
    {
        if(mOutGoingCallType == SIP_CALL)
        {
            int numCodecs;
            SdpCodec** codecArray = NULL;
#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG,
                          "CallManager::doCreateCall "
                          "getting codec array copy\n");
#endif
            getCodecs(numCodecs, codecArray);
#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG,
                          "CallManager::doCreateCall "
                          "got %d codecs, creating CpPhoneMediaInterface\n",
                          numCodecs);
#endif
            UtlString publicAddress;
            int publicPort;
            //always use sipUserAgent public address, not the mPublicAddress of this call manager.
            sipUserAgent->getViaInfo(OsSocket::UDP,publicAddress,publicPort);

            UtlString localAddress;
            int port;

            sipUserAgent->getLocalAddress(&localAddress, &port);
            CpMediaInterface* mediaInterface = mpMediaFactory->createMediaInterface(
                publicAddress.data(), localAddress.data(),
                numCodecs, codecArray, mLocale.data(), mExpeditedIpTos,
                mStunServer, mStunOptions, mStunKeepAlivePeriodSecs);

            OsSysLog::add(FAC_CP, PRI_DEBUG,
                          "CallManager::doCreateCall "
                          "Creating new SIP Call, mediaInterface: 0x%p\n",
                          mediaInterface);
            call = new CpPeerCall(mIsEarlyMediaFor180,
                this,
                mediaInterface,
                aquireCallIndex(),
                callId,
                sipUserAgent,
                mSipSessionReinviteTimer,
                mOutboundLine.data(),
                mHoldType,
                mOfferedTimeOut,
                mLineAvailableBehavior,
                mForwardUnconditional.data(),
                mLineBusyBehavior,
                mSipForwardOnBusy.data(),
                mNoAnswerTimeout,
                mForwardOnNoAnswer.data());
            // Short term kludge: createCall invoked, this
            // implys the phone is off hook
            call->enableDtmf();
            call->start();
            addTaoListenerToCall(call);
            //          addToneListener(callId, 0); // add dtmf tone listener to mp

            if(metaEventId > 0)
            {
                call->setMetaEvent(metaEventId, metaEventType,
                                   numMetaEventCalls, metaEventCallIds);
            }
            else
            {
                int type = (metaEventType != PtEvent::META_EVENT_NONE) ? metaEventType : PtEvent::META_CALL_STARTING;
                call->startMetaEvent(getNewMetaEventId(), type, numMetaEventCalls, metaEventCallIds);
            }

            // Make this call infocus if there currently is not infocus call
            if(!infocusCall && assumeFocusIfNoInfocusCall)
            {
                infocusCall = call;
                infocusCall->inFocus(0);
            }
            // Other wise add this call to the stack
            else
            {
                pushCall(call);
            }

            for (int i = 0; i < numCodecs; i++)
            {
                delete codecArray[i];
            }
            delete[] codecArray;
        }
    }
}


void CallManager::doConnect(const char* callId,
                            const char* addressUrl,
                            const char* desiredConnectionCallId,
                            ContactId contactId,
                            const void* pDisplay,
                            const bool sendPAIheader )
{
    UtlString outboundLineIdentity;

    CpCall* call = findHandlingCall(callId);
    if(!call)
    {
        // This is generally bad.  The call should exist.
        OsSysLog::add(FAC_CP, PRI_ERR,
                      "CallManager::doConnect "
                      "cannot find CallId: %s\n",
                      callId);
    }
    else
    {
        if ( sendPAIheader == TRUE )
        {
            Url outboundLineUrl(mOutboundLine);
            Url::Scheme outboundLineUrlScheme = outboundLineUrl.getScheme();
            switch (outboundLineUrlScheme)
            {
            case Url::SipsUrlScheme:
               // sips and sip are equivalent for identity purposes,
               //   so just set to sip
               outboundLineUrl.setScheme(Url::SipUrlScheme);
               //   and fall through to extract the identity...

            case Url::SipUrlScheme:
               // case Url::TelUrlScheme: will go here, since 'tel' and 'sip' are the same length
               outboundLineUrl.getUri(outboundLineIdentity);
               outboundLineIdentity.remove(0,4 /* strlen("sip:") */); // strip off the scheme name
               break;

            default:
               // for all other schemes, treat identity as null
               OsSysLog::add(FAC_SIP, PRI_WARNING,
                             "CallManager::doConnect "
                             "mOutboundLine uses unsupported scheme '%s'"
                             " - using null identity",
                             Url::schemeName(outboundLineUrlScheme)
                             );
               break;
            }
#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG,
                          "CallManager::doConnect "
                          "adressUrl '%s'"
                          "desiredConnectionCallId '%s'"
                          "mOutboundLine '%s'"
                          "outboundLineIdentity '%s'",
                          addressUrl, desiredConnectionCallId,
                          mOutboundLine.data(), outboundLineIdentity.data());
#endif
        }

        // For now just send the call a dialString
        CpMultiStringMessage dialStringMessage(CP_DIAL_STRING,
                                               addressUrl,
                                               desiredConnectionCallId,
                                               outboundLineIdentity.data(),
                                               NULL, NULL,
                                               contactId,
                                               (intptr_t)pDisplay) ;
        call->postMessage(dialStringMessage);
        call->setLocalConnectionState(PtEvent::CONNECTION_ESTABLISHED);
#ifdef TEST_PRINT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CallManager::doConnect "
                      "stopMetaEvent 4");
#endif
        call->stopMetaEvent();
    }
}

void CallManager::doSendInfo(const char* callId,
                             const char* szContentType,
                             UtlString   sContent)
{
    CpCall* call = findHandlingCall(callId);
    if(!call)
    {
        // This is generally bad.  The call should exist.
        OsSysLog::add(FAC_CP, PRI_ERR, "doSendInfo cannot find CallId: %s\n", callId);
    }
    else
    {
        CpMultiStringMessage infoMessage(CP_OUTGOING_INFO, UtlString(callId), UtlString(szContentType), sContent) ;
        call->postMessage(infoMessage);
    }
    return;
}


void CallManager::doEnableStun(const char* szStunServer,
                               int iKeepAlivePeriodSecs,
                               int stunOptions,
                               OsNotification* pNotification)
{
    mStunServer = szStunServer ;
    mStunOptions = stunOptions ;
    mStunKeepAlivePeriodSecs = iKeepAlivePeriodSecs ;

    if (sipUserAgent)
    {
        sipUserAgent->enableStun(szStunServer, iKeepAlivePeriodSecs, stunOptions, pNotification) ;
    }
}



UtlBoolean CallManager::disconnectConnection(const char* callId, const char* addressUrl)
{
    OsReadLock lock(mCallListMutex);
    CpCall* call = findHandlingCall(callId);
    if(!call)
    {
        // This is generally bad.  The call should exist.
        OsSysLog::add(FAC_CP, PRI_ERR, "disconnectConnect cannot find CallId: %s\n", callId);
        return FALSE;
    }
    else
    {
        dropConnection(callId, addressUrl);
    }

    return TRUE;

}

void CallManager::getCodecs(int& numCodecs, SdpCodec**& codecArray)
{
    mpCodecFactory->getCodecs(numCodecs,
        codecArray);

}

void CallManager::releaseEvent(const char* callId,
                               OsProtectEventMgr* eventMgr,
                               OsProtectedEvent* dtmfEvent)
{
    removeDtmfEvent(callId, dtmfEvent);

    // If the event has already been signalled, clean up
    if(OS_ALREADY_SIGNALED == dtmfEvent->signal(0))
    {
        eventMgr->release(dtmfEvent);
    }
}

void CallManager::setDelayInDeleteCall(int delayInDeleteCall)
{
    mDelayInDeleteCall = delayInDeleteCall;
}

int CallManager::getDelayInDeleteCall()
{
    return mDelayInDeleteCall;
}

/* ============================ FUNCTIONS ================================= */

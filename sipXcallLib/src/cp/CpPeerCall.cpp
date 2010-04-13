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

// APPLICATION INCLUDES
#include "utl/UtlRegex.h"
#include <cp/CpPeerCall.h>
#include <cp/CpCallManager.h>
#include <cp/CpStringMessage.h>
#include <cp/CpIntMessage.h>
#include <cp/CpMultiStringMessage.h>
#include <net/SipMessageEvent.h>
#include <net/CallId.h>
#include <cp/SipConnection.h>
#include <cp/CpGhostConnection.h>
#include <mi/CpMediaInterface.h>
#include <net/SipUserAgent.h>
#include <net/NameValueTokenizer.h>
#include <net/Url.h>
#include <net/SipSession.h>
#include <ptapi/PtConnection.h>
#include "ptapi/PtCall.h"
#include "ptapi/PtEvent.h"
#include <ptapi/PtTerminalConnection.h>
#include <os/OsReadLock.h>
#include <os/OsWriteLock.h>
#include <os/OsProtectEvent.h>
#include "os/OsQueuedEvent.h"
#include "os/OsTimer.h"
#include "os/OsTime.h"
#include "os/OsDateTime.h"
#include "tao/TaoProviderAdaptor.h"

//#define TEST_PRINT 1

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define CALL_STATUS_FIELD       "status"

#ifdef _WIN32
#   define CALL_CONTROL_TONES
#endif

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpPeerCall::CpPeerCall(UtlBoolean isEarlyMediaFor180Enabled,
                       CpCallManager* callManager,
                       CpMediaInterface* callMediaInterface,
                       int callIndex,
                       const char* callId,
                       SipUserAgent* sipUA,
                       int sipSessionReinviteTimer,
                       const char* defaultCallExtension,
                       int holdType,
                       int offeringDelayMilliSeconds,
                       int availableBehavior,
                       const char* forwardUnconditionalUrl,
                       int busyBehavior,
                       const char* forwardOnBusyUrl,
                       int forwardOnNoAnswerMilliSeconds,
                       const char* forwardOnNoAnswerUrl,
                       int ringingExpireSeconds) :
CpCall(callManager, callMediaInterface, callIndex, callId,
       holdType),
       mConnectionMutex(OsRWMutex::Q_PRIORITY),
       mIsEarlyMediaFor180(TRUE)
{
#ifdef TEST_PRINT
    if (callId)
        OsSysLog::add(FAC_CP, PRI_DEBUG, "CpPeerCall constructor: %s\n", callId);
    else
        OsSysLog::add(FAC_CP, PRI_DEBUG, "CpPeerCall constructor:: callId is Null\n");
#endif

    // SIP and Peer to Peer call intialization
    sipUserAgent = sipUA;
    mIsEarlyMediaFor180 = isEarlyMediaFor180Enabled;
    mSipSessionReinviteTimer = sipSessionReinviteTimer;
    offeringDelay = offeringDelayMilliSeconds;
    lineAvailableBehavior = availableBehavior;
    if(lineAvailableBehavior == Connection::FORWARD_UNCONDITIONAL &&
        forwardUnconditionalUrl != NULL)
    {
        forwardUnconditional.append(forwardUnconditionalUrl);
    }
    lineBusyBehavior = busyBehavior;
    if(lineBusyBehavior == Connection::FORWARD_ON_BUSY &&
        forwardOnBusyUrl != NULL)
    {
        forwardOnBusy.append(forwardOnBusyUrl);
    }
#ifdef TEST_PRINT
    osPrintf("%s-CpPeerCall: SIP Forward on busy: %s\n",
        mName.data(), forwardOnBusy.data());
#endif

    if(forwardOnNoAnswerUrl != NULL && strlen(forwardOnNoAnswerUrl) > 0)
    {
        if ( forwardOnNoAnswerMilliSeconds > -1)
            noAnswerTimeout = forwardOnNoAnswerMilliSeconds;
        else
            noAnswerTimeout = 24;        // default

        forwardOnNoAnswer.append(forwardOnNoAnswerUrl);
    }

    else
    {
        noAnswerTimeout = ringingExpireSeconds;
    }

    if(defaultCallExtension)
    {
        //extension.append(phoneExtension);
        //NameValueTokenizer::frontBackTrim(&extension, " \t\n\r");
        Url outboundLine(defaultCallExtension);


        // For now the terminal id is the extension
        //mLocalTerminalId.append(extension);
        outboundLine.toString(mLocalTerminalId);
        mLocalAddress = mLocalTerminalId;
#ifdef TEST_PRINT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CpPeerCall:: %p setting mLocalAddress to '%s'",
                      this, mLocalAddress.data());
#endif
    }

    // Set the local address for the implied connection
    // This is a bit of a short cut
    // SIP specific stuff should not be here
    //UtlString myServerAddress;
    //UtlString myServerProtocol;
    //int myServerPort;
    //sipUserAgent->getDirectoryServer(0, &myServerAddress,
    //    &myServerPort, &myServerProtocol);
    //if(myServerAddress.isNull())
    //{
    //    sipUserAgent->getFromAddress(&myServerAddress,
    //        &myServerPort, &myServerProtocol);
    //}
    //SipMessage::buildSipUri(&mLocalAddress, myServerAddress.data(),
    //    myServerPort, myServerProtocol.data(), extension.data());

    mDialMode = ADD_PARTY;
    setCallType(CP_NORMAL_CALL);

#ifdef TEST_PRINT
    osPrintf("%s-CpPeerCall::Call outbound line: %s\n", mName.data(), mLocalAddress.data());
#endif

    mbRequestedDrop = false ;
#ifdef TEST_PRINT
    if (callId)
        OsSysLog::add(FAC_CP, PRI_DEBUG, "Leaving CpPeerCall constructor: %s\n", callId);
    else
        OsSysLog::add(FAC_CP, PRI_DEBUG, "Leaving CpPeerCall constructor:: callId is Null\n");
#endif

    eLastMajor = (SIPX_CALLSTATE_EVENT) -1 ;
    eLastMinor = (SIPX_CALLSTATE_CAUSE) -1 ;
}

// Copy constructor
CpPeerCall::CpPeerCall(const CpPeerCall& rCpPeerCall) :
mConnectionMutex(OsRWMutex::Q_PRIORITY)
{
}

// Destructor
CpPeerCall::~CpPeerCall()
{
   waitUntilShutDown(20000) ;
#ifdef TEST_PRINT
    UtlString name = getName();
    if (!mCallId.isNull())
    {
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CpPeerCall-%s destructor: "
                      "%s\n",
                      name.data(), mCallId.data());
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CpPeerCall-%s destructor:: "
                      "callId is Null\n",
                      name.data());
    }
#endif

    Connection* connection = NULL;
    while ((connection = (Connection*) mConnections.get()))
    {
        delete connection;
        connection = NULL;
    }
#ifdef TEST_PRINT
    if (!mCallId.isNull())
    {
        OsSysLog::add(FAC_CP, PRI_DEBUG, "Leaving CpPeerCall-%s destructor: %s\n", name.data(), mCallId.data());
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_DEBUG, "Leaving CpPeerCall-%s destructor:: callId is Null\n", name.data());
    }
#endif
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
CpPeerCall&
CpPeerCall::operator=(const CpPeerCall& rhs)
{
    if (this == &rhs)            // handle the assignment to self case
        return *this;

    return *this;
}

// Handles the processing of a CallManager::CP_DIAL_STRING message
UtlBoolean CpPeerCall::handleDialString(OsMsg* pEventMessage)
{
    UtlString dialString;
    UtlString desiredCallId;
    UtlString paiAddress;
    UtlString remoteHostName;
    ContactId contactId ;

    ((CpMultiStringMessage*)pEventMessage)->getString1Data(dialString);
    ((CpMultiStringMessage*)pEventMessage)->getString2Data(desiredCallId);
    ((CpMultiStringMessage*)pEventMessage)->getString3Data(paiAddress);
    contactId = (ContactType) ((CpMultiStringMessage*)pEventMessage)->getInt1Data();
    void* pDisplay = (void*) ((CpMultiStringMessage*)pEventMessage)->getInt2Data();


#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpPeerCall::dialing "
                  "string: '%s' length: %d "
                  "callId '%s' PAIaddr= '%s'",
                  dialString.data(), dialString.length(),
                  desiredCallId.data(), paiAddress.data());
#endif

    addHistoryEvent("CP_DIAL_STRING (3) \n\tDialString: \"" + dialString + "\"");

    // If all digits or * assume this is a userid not a server address
    RegEx allDigits("^[0-9*]+$");
    if(allDigits.Search(dialString.data()))
    {
        remoteHostName.append(dialString.data());
        remoteHostName.append('@');
    }
    else
    {
        remoteHostName.append(dialString.data());
    }

    if(!isCallIdSet())
    {
        UtlString callId;
        CallId::getNewCallId(callId);
        setCallId(callId.data());
    }

    // Initial two party call or adding a party
    if(mDialMode == ADD_PARTY)
    {
        if (desiredCallId.length() != 0)
        {
            // Use supplied callId
            addParty(remoteHostName.data(), NULL, NULL, desiredCallId.data(), contactId, pDisplay, NULL, paiAddress.data());
        }
        else
        {
            // Use default call id
            addParty(remoteHostName.data(), NULL, NULL, NULL, contactId, pDisplay,  NULL, paiAddress.data());
        }
    }

    return TRUE ;
}

// Handles the processing of a CallManager::CP_DEQUEUED_CALL message
UtlBoolean CpPeerCall::handleDequeueCall(OsMsg* pEventMessage)
{
    Connection* connection = findQueuedConnection();
    if(connection)
    {
#ifdef TEST_PRINT
        osPrintf("%s-About to dequeue connection: %p\n",
                 mName.data(), connection);
#endif
        connection->dequeue(mCallInFocus);
    }

    return TRUE ;
}


// Handles the processing of CallManager::CP_BLIND_TRANSFER and
// CallManager::CP_CONSULT_TRANSFER messages
UtlBoolean CpPeerCall::handleTransfer(OsMsg* pEventMessage)
{
    int msgSubType = pEventMessage->getMsgSubType();

    // This message is received by the original call on
    // the transfer controller
    if(getCallType() == CP_NORMAL_CALL)
    { // case variable scope
        setCallType(CP_TRANSFER_CONTROLLER_ORIGINAL_CALL);

        int metaEventId = ((CpMultiStringMessage*)pEventMessage)->getInt1Data();
        bool remoteHoldBeforeTransfer = ((CpMultiStringMessage*)pEventMessage)->getInt2Data();
        UtlString targetCallId;
        ((CpMultiStringMessage*)pEventMessage)->getString3Data(targetCallId);
        setTargetCallId(targetCallId.data());

        UtlString thisCallId;
        getCallId(thisCallId);
        const char* metaCallIds[2];
        metaCallIds[0] = targetCallId.data(); // New call first
        metaCallIds[1] = thisCallId.data();

        // Start the meta event
        startMetaEvent(metaEventId, PtEvent::META_CALL_TRANSFERRING, 2, metaCallIds);

        // Create the target call
        mpManager->createCall(&targetCallId,
                              metaEventId,
                              PtEvent::META_CALL_TRANSFERRING,
                              2, metaCallIds,
                              FALSE);
        // Do  not assume focus if there is no infocus call
        // as this is not a real call.  It is a place holder for call
        // state on the remote call

        if(msgSubType == CallManager::CP_BLIND_TRANSFER)
        {
            // Transfer does an implicit hold.
            // The local hold is done here.  The remote hold is
            // done on each connection.
            localHold();
        }

        Connection* connection = NULL;
        UtlString transferTargetAddress;
        ((CpMultiStringMessage*)pEventMessage)->getString2Data(transferTargetAddress);
#ifdef TEST_PRINT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CpPeerCall::handleTransfer "
                      "CP_BLIND_TRANSFER "
                      "targetCallId: %s targetAddress: %s "
                      "metaEventId: %d\n",
                      targetCallId.data(), transferTargetAddress.data(), metaEventId);
#endif


        OsReadLock lock(mConnectionMutex);
        UtlDListIterator iterator(mConnections);
        while ((connection = (Connection*) iterator()))
        {
            UtlString originalCallId;
            UtlString connectionAddress;
            connection->getCallId(&originalCallId);

            // Only transfer the call when the callId of the connection matches this CpPeerCall's callId,
            // otherwise, indicates that the the call leg has been replaced, and is in the process of dying.
            // This check is to close the race condition that to transfer a call to a dying connection.

            if (originalCallId == thisCallId)
            {

               UtlBoolean isOk =
                   connection->originalCallTransfer(transferTargetAddress,
                                                    NULL,
                                                    targetCallId.data(),
                                                    remoteHoldBeforeTransfer
                                                               );
               if (!isOk)
               {
                   UtlString targetCallId;
                   getTargetCallId(targetCallId);
                   UtlString remoteAddress;
                   connection->getRemoteAddress(&remoteAddress);
                   UtlString responseText;
                   connection->getResponseText(responseText);
                   postTaoListenerMessage(connection->getResponseCode(),
                                          responseText,
                                          PtEvent::CONNECTION_FAILED,
                                          CONNECTION_STATE,
                                          PtEvent::CAUSE_TRANSFER,
                                          connection->isRemoteCallee(),
                                          remoteAddress,
                                          1,
                                          targetCallId);
                /** SIPXTAPI: TBD **/
#ifdef TEST_PRINT
                   OsSysLog::add(FAC_CP, PRI_DEBUG,
                                 "CpPeerCall::handleTransfer "
                                 "CP_BLIND_TRANSFER posting "
                                 "CONNECTION_FAILED to call: %s",
                                 targetCallId.data());
#endif
               }
               else
               {
                   // Send a message to the target call for each transfered
                   // connection
                   UtlString originalCallId;
                   UtlString connectionAddress;
                   connection->getCallId(&originalCallId);
                   connection->getRemoteAddress(&connectionAddress);
#ifdef TEST_PRINT
                   OsSysLog::add(FAC_CP, PRI_DEBUG,
                                 "CpPeerCall::handleTransfer "
                                 "2 party transfer on connection: %s "
                                 "original call: %s target call: %s",
                                 connectionAddress.data(),
                                 originalCallId.data(), targetCallId.data());
#endif
                   CpMultiStringMessage
                       transferConnect(CallManager::CP_TRANSFER_CONNECTION,
                                       targetCallId.data(),
                                       transferTargetAddress.data(),
                                       originalCallId.data(),
                                       connectionAddress.data(),
                                       NULL,
                                       metaEventId);
                   mpManager->postMessage(transferConnect);
               }
           }
        }
    }

    return TRUE;
}

// Handles the processing of a CallManager::CP_CONSULT_TRANSFER_ADDRESS
// message.
UtlBoolean CpPeerCall::handleTransferAddress(OsMsg* pEventMessage)
{
    CpMultiStringMessage* pMessage = (CpMultiStringMessage*) pEventMessage ;
    int msgSubType = pEventMessage->getMsgSubType() ;

    UtlString sourceCallId ;
    UtlString sourceAddress ;
    UtlString targetCallId ;
    UtlString targetAddress ;
    UtlString targetUrl ;

    // Parse parameters
    pMessage->getString1Data(sourceCallId) ;
    pMessage->getString2Data(sourceAddress) ;
    pMessage->getString3Data(targetCallId) ;
    pMessage->getString4Data(targetAddress) ;
    pMessage->getString5Data(targetUrl) ;

    Connection* pConnection = findHandlingConnection(sourceAddress);
    if (pConnection)
    {
        UtlBoolean bRC = pConnection->originalCallTransfer(targetUrl, sourceAddress, targetCallId) ;
        if (!bRC)
        {

        }
    }

    return TRUE;
}


// Handles the processing of a CallManager::CP_TRANSFER_CONNECTION message
UtlBoolean CpPeerCall::handleTransferConnection(OsMsg* pEventMessage)
{
    UtlString msgIdOfOrigCall;
    UtlString currentIdOfOrigCall;
    getIdOfOrigCall(currentIdOfOrigCall);
    UtlString transferTargetAddress;
    UtlString connectionAddress;
    ((CpMultiStringMessage*)pEventMessage)->getString2Data(transferTargetAddress);
    ((CpMultiStringMessage*)pEventMessage)->getString3Data(msgIdOfOrigCall);
    ((CpMultiStringMessage*)pEventMessage)->getString4Data(connectionAddress);
#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpPeerCall::handleTransferConnection "
                  "target address: %s original call: %s "
                  "target connection address: %s callType: %d",
                  transferTargetAddress.data(), msgIdOfOrigCall.data(),
                  connectionAddress.data(), getCallType());
#endif
    // If it is legal for this call to be a transfer target
    if(getCallType() == CP_NORMAL_CALL
       || (getCallType() == CP_TRANSFER_CONTROLLER_TARGET_CALL
           && currentIdOfOrigCall.compareTo(msgIdOfOrigCall) == 0))
    {
        // Set the original call id so that we can send messages
        // back if necessary.
        if(getCallType() == CP_NORMAL_CALL)
        {
            setIdOfOrigCall(msgIdOfOrigCall.data());
            setCallType(CP_TRANSFER_CONTROLLER_TARGET_CALL);
        }

        // Find the connection
        // Currently do not need to lock as we should not get
        // a connection back and if we do we do nothing with it.
        //OsReadLock lock(mConnectionMutex);
        Connection* connection = findHandlingConnection(connectionAddress);

        // The connection does not exist, for now we can assume
        // this is a blind transfer.  Create a ghost connection
        // and put it in the offering state
        if(! connection)
        {
#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG,
                          "CpPeerCall::handleTransferConnection "
                          "creating ghost connection\n");
#endif
            UtlString thisCallId;
            getCallId(thisCallId);
            mLocalConnectionState = PtEvent::CONNECTION_ESTABLISHED;
            mLocalTermConnectionState = PtTerminalConnection::TALKING;

            connection = new CpGhostConnection(mpManager, this,
                thisCallId.data());
            addConnection(connection);
            connection->targetCallBlindTransfer(connectionAddress, NULL);
            addToneListenersToConnection(connection) ;
        }
        else
        {
#ifdef TEST_PRINT
            // I think this is bad
            OsSysLog::add(FAC_CP, PRI_DEBUG,
                          "CpPeerCall::handleTransferConnection "
                          "connection already exists\n");
#endif
        }
    }


    /*
    * WARNING: We should creating another connection for the TARGET address.
    *          We are not doing this now, because we don't have any code that
    *          steps through state progressions.
    *
    * TODO: CODE ME
    */

    return TRUE ;
}


// Handles the processing of a CallManager::CP_TRANSFEREE_CONNECTION message
UtlBoolean CpPeerCall::handleTransfereeConnection(OsMsg* pEventMessage)
{
    // Message sent to target call on transferee

    UtlString referTo;
    UtlString referredBy;
    UtlString idOfReferringCall;
    UtlString currentIdOfOrigCall;
    getIdOfOrigCall(currentIdOfOrigCall);
    UtlString connectionAddressToAdd;
    ((CpMultiStringMessage*)pEventMessage)->getString2Data(referTo);
    ((CpMultiStringMessage*)pEventMessage)->getString3Data(referredBy);
    ((CpMultiStringMessage*)pEventMessage)->getString4Data(idOfReferringCall);
    ((CpMultiStringMessage*)pEventMessage)->getString5Data(connectionAddressToAdd);
#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpPeerCall::handleTransfereeConnection "
                  "referTo: %s referredBy: \"%s\" "
                  "originalCallId: %s originalConnectionAddress: %s "
                  "callType: %d\n",
                  referTo.data(), referredBy.data(),
                  idOfReferringCall.data(), connectionAddressToAdd.data(),
                  getCallType());
#endif
    if(getCallType() == CP_NORMAL_CALL
       // TBD? I can't find where call type in next line is ever set
       || (getCallType() == CP_TRANSFEREE_TARGET_CALL
           && currentIdOfOrigCall.compareTo(idOfReferringCall) == 0))
    {
        if(getCallType() == CP_NORMAL_CALL)
        {
            setIdOfOrigCall(idOfReferringCall);
        }
        // Do not need to lock as connection is never touched
        // and addConnection does its own locking
        //OsReadLock lock(mConnectionMutex);
        UtlString cleanReferTo;
        Url referToUrl(referTo);
        referToUrl.removeHeaderParameters();
        referToUrl.toString(cleanReferTo);
        Connection* connection = findHandlingConnection(cleanReferTo);

        if(!connection)
        {
            // Create a new connection on this call to connect to the
            // transfer target.
#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG,
                          "CpPeerCall::handleTransfereeConnection "
                          "creating connection via addParty\n");
#endif
            addParty(referTo, referredBy, connectionAddressToAdd, NULL, 0, 0, idOfReferringCall);
            // Note: The connection is added to the call in addParty
        }
#ifdef TEST_PRINT
        // I do not think this is good
        else
        {
            OsSysLog::add(FAC_CP, PRI_DEBUG,
                          "CpPeerCall::handleTransfereeConnection "
                          "connection already exists\n");
        }
#endif

    }
#ifdef TEST_PRINT
    else
    {
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CpPeerCall::handleTransfereeConnection "
                      "callType: %d \n",
                      getCallType());
    }
#endif

    return TRUE ;
}

// Handles the processing of a CallManager::CP_SIP_MESSAGE message
UtlBoolean CpPeerCall::handleSipMessage(OsMsg* pEventMessage)
{
    UtlBoolean bAddedConnection = FALSE ;

    addHistoryEvent("CP_SIP_MESSAGE (1)");

    // There are of course small windows between:
    // findHandlingConnection, addConnection and the
    // read lock taken below.  But you cannot have a
    // read or write lock nested in a write lock.
    Connection* connection =
        findHandlingConnection(*pEventMessage);

    UtlString name = getName();
    if(connection == NULL)
    {
        if (SipConnection::shouldCreateConnection(*sipUserAgent,
            *pEventMessage))
        {
#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG,
                          "CpPeerCall::handleSipMessage "
                          "- connection NULL, shouldCreateConnection TRUE "
                          "msgType %d msgSubType %d \n",
                          pEventMessage->getMsgType(), pEventMessage->getMsgSubType());
#endif
            connection = new SipConnection("", // Get local address from message
                mIsEarlyMediaFor180,
                mpManager,
                this,
                mpMediaInterface,
                //mpCallUiContext,
                sipUserAgent,
                offeringDelay,
                mSipSessionReinviteTimer,
                lineAvailableBehavior,
                forwardUnconditional.data(),
                lineBusyBehavior,
                forwardOnBusy.data());
            addConnection(connection);
            bAddedConnection = TRUE ;
            mDtmfEnabled = TRUE ;
        }
        else
        {
            SipConnection::processNewFinalMessage(sipUserAgent, pEventMessage);
#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG,
                          "CpPeerCall::handleSipMessage "
                          "- processing new final response to INVITE\n" );
#endif
        }
    }  // end NULL connection, create new

#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpPeerCall::handleSipMessage "
                  "- connection %p msgType %d msgSubType %d",
                  connection,
                  pEventMessage->getMsgType(),
                  pEventMessage->getMsgSubType());
#endif

    if(connection)
    {
        OsReadLock lock(mConnectionMutex);
        int previousConnectionState = connection->getState();
        //PtTerminalConnection::TerminalConnectionState prevTermConState;
        //int gotPrevTermConState = getUiTerminalConnectionState(prevTermConState);

        //                if (previousConnectionState == Connection::CONNECTION_IDLE)
        //                {
        //                    startMetaEvent( mpManager->getNewMetaEventId(),
        //                                    PtEvent::META_CALL_STARTING,
        //                                    0,
        //                                    0);
        //                }

        connection->processMessage(*pEventMessage,
                                   mCallInFocus,
                                   !mDtmfEnabled); // mDtmfEnabled is the same as offHook state
                                    //mNumCodecs, mpaCodecArray);

        int currentConnectionState = connection->getState();
        if (   ((previousConnectionState != currentConnectionState)
             || (getCallState() == PtCall::IDLE))
           &&  ((currentConnectionState == Connection::CONNECTION_OFFERING)
             || (currentConnectionState == Connection::CONNECTION_ALERTING)) )
        {
            UtlString responseText;
            connection->getResponseText(responseText);
            setCallState(connection->getResponseCode(), responseText, PtCall::ACTIVE);
            /** SIPXTAPI: TBD **/
        }

        if (   previousConnectionState == Connection::CONNECTION_IDLE
            && currentConnectionState  == Connection::CONNECTION_OFFERING)
        {
#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG,
                          "CpPeerCall::handleSipMessage "
                          "stopMetaEvent 6");
#endif
            stopMetaEvent(connection->isRemoteCallee());
        }

        // If this call does not have a callId set it
        if(!isCallIdSet()
           && (currentConnectionState != Connection::CONNECTION_FAILED
               || currentConnectionState != Connection::CONNECTION_DISCONNECTED
               || currentConnectionState != Connection::CONNECTION_IDLE))
        {
            UtlString callId;
            connection->getCallId(&callId);
            setCallId(callId.data());
        }

        if (bAddedConnection)
        {
            addToneListenersToConnection(connection) ;
        }

    } // End if we created a new connection or used and existing one

    // Check if call is dead and drop it if it is
    dropIfDead();

    return TRUE ;
}

// Handles the processing of a CallManager::CP_DROP_CONNECTION message
UtlBoolean CpPeerCall::handleDropConnection(OsMsg* pEventMessage)
{
    {
        OsReadLock lock(mConnectionMutex);
        UtlString connectionAddress;
        ((CpMultiStringMessage*)pEventMessage)->getString2Data(connectionAddress);
        Connection* connection = findHandlingConnection(connectionAddress);

        if(connection)
        {
            // do not fire the tapi event if it is a ghost connection
            CpGhostConnection* pGhost = NULL;
            pGhost = dynamic_cast<CpGhostConnection*>(connection);
            if (!pGhost)
            {
                connection->fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL) ;
            }
            connection->hangUp();
        }
    }

    // Check if call is dead and drop it if it is
    dropIfDead();

    return TRUE;
}


// Handles the processing of a CallManager::CP_FORCE_DROP_CONNECTION message
UtlBoolean CpPeerCall::handleForceDropConnection(OsMsg* pEventMessage)
{
    {
        OsReadLock lock(mConnectionMutex);
        UtlString connectionAddress;
        ((CpMultiStringMessage*)pEventMessage)->getString2Data(connectionAddress);
        Connection* connection = findHandlingConnection(connectionAddress);

        if(connection)
        {
#ifdef TEST_PRINT
            osPrintf("%s-CpPeerCall::CP_FORCE_DROP_CONNECTION Found connection\n", mName.data());
#endif
            connection->forceHangUp();
            /** SIPXTAPI: TBD **/

        }
        mLocalConnectionState = PtEvent::CONNECTION_DISCONNECTED;
        mLocalTermConnectionState = PtTerminalConnection::DROPPED;
    }

    // Check if call is dead and drop it if it is
    dropIfDead();

    return TRUE;
}


// Handles the processing of a CallManager::CP_GET_CALLED_ADDRESSES and
// CallManager::CP_GET_CALLING_ADDRESSES messages
UtlBoolean CpPeerCall::handleGetAddresses(OsMsg* pEventMessage)
{
    int msgSubType = pEventMessage->getMsgSubType();

    int numConnections = 0;
    UtlBoolean localAdded = FALSE;
    UtlSList* connectionList;
    OsProtectedEvent* getConnEvent = (OsProtectedEvent*) ((CpMultiStringMessage*)pEventMessage)->getInt1Data();
    getConnEvent->getIntData((intptr_t&)connectionList);

    if(getConnEvent)
    {
        // Get the remote connection(s)/address(es)
        { // scope for lock
            Connection* connection = NULL;
            UtlString address;
            OsReadLock lock(mConnectionMutex);
            UtlDListIterator iterator(mConnections);
            while ((connection = (Connection*) iterator()))
            {
                if((msgSubType == CallManager::CP_GET_CALLED_ADDRESSES &&
                    connection->isRemoteCallee() ) ||
                    (msgSubType == CallManager::CP_GET_CALLING_ADDRESSES &&
                    !connection->isRemoteCallee()))
                {
                    connection->getRemoteAddress(&address);
                    connectionList->append(new UtlString(address));
                    numConnections++;
                }
                else if(!localAdded)
                {
                    // Add the local connection/address
                    localAdded = TRUE;
                    connectionList->append(new UtlString(mLocalAddress));
                    numConnections++;
                }
            }
        }
        // Signal the caller that we are done.
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == getConnEvent->signal(numConnections))
        {
            // The other end must have timed out on the wait
            connectionList->destroyAll();
            delete connectionList;
            OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
            eventMgr->release(getConnEvent);
        }
    }

    return TRUE ;
}


// Handles the processing of a CallManager::CP_ACCEPT_CONNECTION
// message
UtlBoolean CpPeerCall::handleAcceptConnection(OsMsg* pEventMessage)
{
    UtlString remoteAddress;
    UtlBoolean connectionFound = FALSE;
    ((CpMultiStringMessage*)pEventMessage)->getString2Data(remoteAddress);
    ContactType eType = (ContactType) ((CpMultiStringMessage*)pEventMessage)->getInt1Data();
    void* hWnd = (void*) ((CpMultiStringMessage*)pEventMessage)->getInt2Data();

    if (hWnd && mpMediaInterface)
    {
        mpMediaInterface->setVideoWindowDisplay(hWnd);
    }

    // This is a bit of a hack/short cut.
    // Find the first remote connection which is in the OFFERING
    // state and assume that it is the connection on
    // which the accept operation is to occur.  The difficulty
    // is that the operation is being called on the local connection
    // but the action must be invoked on the remote connection.  This
    // will need to be fixed for the general case to better support
    // conference calls.  The weird thing from the JTAPI perspective
    // is that the local connection may be ESTABLISHED and an
    // incoming call (new connection) to join an existing conference
    // call will make the local connection go to the OFFERING state.

    SipConnection* connection = NULL;
    UtlString address;
    int connectState;
    OsReadLock lock(mConnectionMutex);
    UtlDListIterator iterator(mConnections);
    while ((connection = (SipConnection*) iterator()))
    {
        connectState = connection->getState();
#ifdef TEST_PRINT
        UtlString remoteAddr;
        UtlString stateString;
        connection->getRemoteAddress(&remoteAddr);
        connection->getStateString(connectState, &stateString);
        osPrintf("%s-CallManager::CP_ACCEPT_CONNECTION connection: %s state: %s\n",
            mName.data(), remoteAddr.data(), stateString.data());
#endif

        if(connectState == Connection::CONNECTION_OFFERING)
        {
            connection->setContactType(eType) ;
            connection->accept(noAnswerTimeout);
            connectionFound = TRUE;
            break;
        }
    }

    if(connectionFound)
    {
        //connection
    }
#ifdef TEST_PRINT
    else
    {
        osPrintf("%s-ERROR: acceptConnection cannot find connectionId: %s\n", mName.data(), remoteAddress.data());
    }
#endif

    return TRUE ;
}


// Handles the processing of a CallManager::CP_REJECT_CONNECTION
// message
UtlBoolean CpPeerCall::handleRejectConnection(OsMsg* pEventMessage)
{
    UtlString remoteAddress;
    UtlString errorText;
    int errorCode ;
    UtlBoolean connectionFound = FALSE;

    ((CpMultiStringMessage*)pEventMessage)->getString2Data(remoteAddress);
    ((CpMultiStringMessage*)pEventMessage)->getString3Data(errorText);
    errorCode = ((CpMultiStringMessage*)pEventMessage)->getInt1Data();

    // This is a bit of a hack/short cut.
    // Find the first remote connection which is in the OFFERING
    // state and assume that it is the connection on
    // which the reject operation is to occur.  The difficulty
    // is that the operation is being called on the local connection
    // but the action must be invoked on the remote connection.  This
    // will need to be fixed for the general case to better support
    // conference calls.  The weird thing from the JTAPI perspective
    // is that the local connection may be ESTABLISHED and an
    // incoming call (new connection) to join an existing conference
    // call will make the local connection go to the OFFERING state.
    Connection* connection = NULL;
    UtlString address;
    int connectState;
    {
        OsReadLock lock(mConnectionMutex);
        UtlDListIterator iterator(mConnections);
        while ((connection = (Connection*) iterator()))
        {
            connectState = connection->getState();

#ifdef TEST_PRINT
            UtlString remoteAddr;
            UtlString stateString;
            connection->getRemoteAddress(&remoteAddr);
            connection->getStateString(connectState, &stateString);
            OsSysLog::add(FAC_CP, PRI_DEBUG,
                          "CpPeerCall::handleRejectConnection "
                          "connection: %s state: %s",
                          remoteAddr.data(), stateString.data());
#endif

            if(connectState == Connection::CONNECTION_OFFERING)
            {
                connection->reject(errorCode, errorText);
                connectionFound = TRUE;
                break;
            }
        }
    }

#ifdef TEST_PRINT
    if(connectionFound)
    {
        //
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CpPeerCall::handleRejectConnection "
                      "cannot find connectionId: %s",
                      remoteAddress.data());
    }
#endif

    // Check if call is dead and drop it if it is
    dropIfDead();

    return TRUE ;
}


// Handles the processing of a CallManager::CP_REDIRECT_CONNECTION
// message
UtlBoolean CpPeerCall::handleRedirectConnection(OsMsg* pEventMessage)
{
    UtlString remoteAddress;
    UtlString forwardAddress;
    UtlBoolean connectionFound = FALSE;
    ((CpMultiStringMessage*)pEventMessage)->getString2Data(remoteAddress);
    ((CpMultiStringMessage*)pEventMessage)->getString3Data(forwardAddress);

    // This is a bit of a hack/short cut.
    // Find the first remote connection which is in the OFFERING
    // or ALERTING state and assume that it is the connection on
    // which the redirect operation is to occur.  The difficulty
    // is that the operation is being called on the local connection
    // but the action must be invoked on the remote connection.  This
    // will need to be fixed for the general case to better support
    // conference calls.  The weird thing from the JTAPI perspective
    // is that the local connection may be ESTABLISHED and an
    // incoming call (new connection) to join an existing conference
    // call will make the local connection go to the OFFERING state.
    Connection* connection = NULL;
    UtlString address;
    int connectState;
    {
        OsReadLock lock(mConnectionMutex);
        UtlDListIterator iterator(mConnections);
        while ((connection = (Connection*) iterator()))
        {
            connectState = connection->getState();

#ifdef TEST_PRINT
            UtlString remoteAddr;
            UtlString stateString;
            connection->getRemoteAddress(&remoteAddr);
            connection->getStateString(connectState, &stateString);
            osPrintf("%s-CallManager::CP_REDIRECT_CONNECTION connection: %s state: %s\n",
                mName.data(), remoteAddr.data(), stateString.data());
#endif

            if(connectState == Connection::CONNECTION_OFFERING ||
                connectState == Connection::CONNECTION_ALERTING)
            {
                connection->redirect(forwardAddress.data());
                connectionFound = TRUE;
                break;
            }
        }
    }

#ifdef TEST_PRINT
    if(connectionFound)
    {
        //
    }
    else
    {
        osPrintf("%s-ERROR: CpPeerCall::CP_REDIRECT_CONNECTION cannot find connectionId: %s\n",
            mName.data(), remoteAddress.data());
    }
#endif

    // Check if call is dead and drop it if it is
    dropIfDead();

    return TRUE ;
}


// Handles the processing of a CallManager::CP_HOLD_TERM_CONNECTION
// message
UtlBoolean CpPeerCall::handleHoldTermConnection(OsMsg* pEventMessage)
{
    UtlString address;
    UtlString terminalId;
    UtlString callId;
    ((CpMultiStringMessage*)pEventMessage)->getString1Data(callId);
    ((CpMultiStringMessage*)pEventMessage)->getString2Data(address);
    ((CpMultiStringMessage*)pEventMessage)->getString3Data(terminalId);

    setTargetCallId(callId.data());
    if(isLocalTerminal(terminalId.data()))
    {
        localHold();
    }
    else
    {
        OsReadLock lock(mConnectionMutex);
        Connection* connection = findHandlingConnection(address);

        if(connection)
        {
            connection->hold();

#if 0
Note: Should not bubble up the response until it is received from the far end. It is supposedly fixed in the PAX branch.
            if (mLocalHeld)
            {
                connection->fireSipXEvent(CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_INACTIVE) ;
            }
            else
            {
                connection->fireSipXEvent(CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD) ;
            }
#endif
        }
        else
        {
#ifdef TEST_PRINT
            osPrintf("%s-ERROR: CpPeerCall::CP_HOLD_TERM_CONNECTION cannot find connectionId: %s terminalId: %s\n",
                mName.data(), address.data(), terminalId.data());
#endif
        }
    }

    return TRUE ;
}


// Handles the processing of a CallManager::CP_HOLD_ALL_TERM_CONNECTIONS
// message
UtlBoolean CpPeerCall::handleHoldAllTermConnection(OsMsg* pEventMessage)
{
    // put all the connections on hold

    // The local connection:
    localHold();

    // All of the remote connections
    OsReadLock lock(mConnectionMutex);
    UtlDListIterator iterator(mConnections);

    Connection* connection = NULL;
    while ((connection = (Connection*) iterator()))
    {
        connection->hold();
    }

    return TRUE ;
}


// Handles the processing of a CallManager::CP_UNHOLD_TERM_CONNECTION
// message
UtlBoolean CpPeerCall::handleUnholdTermConnection(OsMsg* pEventMessage)
{
    UtlString address;
    UtlString terminalId;

    ((CpMultiStringMessage*)pEventMessage)->getString2Data(address);
    ((CpMultiStringMessage*)pEventMessage)->getString3Data(terminalId);

    if(isLocalTerminal(terminalId.data()))
    {
        // Post a message to the callManager to change focus
        CpIntMessage localHoldMessage(CallManager::CP_GET_FOCUS,
            (intptr_t)this);
        mpManager->postMessage(localHoldMessage);
        mLocalHeld = FALSE;
    }
    else
    {
        OsReadLock lock(mConnectionMutex);
        Connection* connection = findHandlingConnection(address);

        if(connection)
        {
            connection->offHold();
            UtlString remoteAddress;
            connection->getRemoteAddress(&remoteAddress);
            UtlString connCallId;
            connection->getCallId(&connCallId);
            if (mLocalTermConnectionState != PtTerminalConnection::TALKING &&
                mLocalTermConnectionState != PtTerminalConnection::IDLE)
            {
                UtlString responseText;
                connection->getResponseText(responseText);
                postTaoListenerMessage(connection->getResponseCode(),
                                       responseText,
                                       PtEvent::TERMINAL_CONNECTION_TALKING,
                                       TERMINAL_CONNECTION_STATE,
                                       PtEvent::CAUSE_UNHOLD,
                                       connection->isRemoteCallee(),
                                       remoteAddress,
                                       0,
                                       connCallId);

                if (mLocalHeld)
                {
                    connection->fireSipXEvent(CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD) ;
                }
                else
                {
                    connection->fireSipXEvent(CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE) ;
                }

#ifdef TEST_PRINT
                osPrintf("%s->>>> CpPeerCall::handleUnholdTermConnection: CallManager::CP_UNHOLD_TERM_CONNECTION:  TERMINAL_CONNECTION_CREATED >>>>\n", mName.data());
#endif
            }
        }
#ifdef TEST_PRINT
        else
        {
            osPrintf("%s-ERROR: CpPeerCall::CP_OFFHOLD_TERM_CONNECTION cannot find connectionId: %s\n",
                mName.data(), address.data());
        }
#endif
    }
    return TRUE ;
}


// Handles the processing of a CallManager::CP_RENEGOTIATE_CODECS_CONNECTION
// message
UtlBoolean CpPeerCall::handleRenegotiateCodecsConnection(OsMsg* pEventMessage)
{
    UtlString address;
    UtlString terminalId;

    ((CpMultiStringMessage*)pEventMessage)->getString2Data(address);
    ((CpMultiStringMessage*)pEventMessage)->getString3Data(terminalId);

    if(isLocalTerminal(terminalId.data()))
    {
#ifdef TEST_PRINT
        osPrintf("%s-ERROR: CpPeerCall::CP_RENEGOTIATE_CODECS_CONNECTION cannot renegotiate codecs for local connectionId: %s\n",
            mName.data(), address.data());
#endif
    }
    else
    {
        OsReadLock lock(mConnectionMutex);
        Connection* connection = findHandlingConnection(address);

        if(connection)
        {

            //UtlString remoteAddress;
            //connection->getRemoteAddress(&remoteAddress);
            if (mLocalTermConnectionState == PtTerminalConnection::TALKING)
            {
                connection->renegotiateCodecs();
            }

            else
            {
                // The may be needed for proper error handling
                //UtlString responseText;
                //connection->getResponseText(responseText);
                //postTaoListenerMessage(connection->getResponseCode(),
                //    responseText,
                //    PtEvent::TERMINAL_CONNECTION_TALKING,
                //    TERMINAL_CONNECTION_STATE,
                //    PtEvent::CAUSE_UNHOLD,
                //    connection->isRemoteCallee(),
                //    remoteAddress);
#ifdef TEST_PRINT
                osPrintf("%s-ERROR: CpPeerCall::handleRenegotiateCodecsConnection: CallManager::CP_RENEGOTIATE_CODECS_CONNECTION\n", mName.data());
#endif
            }
        }
#ifdef TEST_PRINT
        else
        {
            osPrintf("%s-ERROR: CpPeerCall::CP_RENEGOTIATE_CODECS_CONNECTION cannot find connectionId: %s\n",
                mName.data(), address.data());
        }
#endif
    }
    return TRUE ;
}


// Handles the processing of a CallManager::CP_RENEGOTIATE_CODECS_ALL_CONNECTIONS
// message
UtlBoolean CpPeerCall::handleRenegotiateCodecsAllConnections(OsMsg* pEventMessage)
{
    if (mLocalTermConnectionState == PtTerminalConnection::TALKING)
    {
        Connection* connection = NULL;
        OsReadLock lock(mConnectionMutex);
        UtlDListIterator iterator(mConnections);
        while ((connection = (Connection*) iterator()))
        {
            connection->renegotiateCodecs() ;
        }
    }
    return TRUE ;
}

// Handles the processing of a CallManager::CP_KEEPALIVE
// message
UtlBoolean CpPeerCall::handleSendKeepAlive(OsMsg* pEventMessage)
{
    UtlBoolean useOptionsForKeepalive = ((CpMultiStringMessage*)pEventMessage)->getInt1Data();
    if (mLocalConnectionState == PtEvent::CONNECTION_ESTABLISHED)
    {
        SipConnection* connection = NULL;
        OsReadLock lock(mConnectionMutex);
        UtlDListIterator iterator(mConnections);
        while ((connection = (SipConnection*) iterator()))
        {
            connection->sendKeepAlive(useOptionsForKeepalive);
        }
    }
    return TRUE ;
}


// Handles the processing of a CallManager::CP_SET_CODEC_CPU_LIMIT message
UtlBoolean CpPeerCall::handleSetCodecCPULimit(OsMsg& eventMessage)
{
    UtlBoolean bRC = FALSE ;

    int iLevel = ((CpMultiStringMessage&)eventMessage).getInt1Data();
    if (mpMediaInterface)
    {
        mpMediaInterface->setCodecCPULimit(iLevel) ;
        bRC = TRUE ;
    }

    return bRC ;
}


// Handles the processing of a CallManager::CP_GET_CODEC_CPU_COST message
UtlBoolean CpPeerCall::handleGetCodecCPUCost(OsMsg& eventMessage)
{
    UtlBoolean bRC = FALSE ;
    int iCost = -1 ;

    if (mpMediaInterface)
    {
        iCost = mpMediaInterface->getCodecCPUCost();
        bRC = TRUE ;
    }

    OsProtectedEvent* getNumEvent = (OsProtectedEvent*) ((CpMultiStringMessage&)eventMessage).getInt1Data();

    // If the event has already been signalled, clean up
    if(OS_ALREADY_SIGNALED == getNumEvent->signal(iCost))
    {
        // The other end must have timed out on the wait
        OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
        eventMgr->release(getNumEvent);
    }

    return bRC ;
}


// Handles the processing of a CallManager::CP_GET_CODEC_CPU_LIMIT message
UtlBoolean CpPeerCall::handleGetCodecCPULimit(OsMsg& eventMessage)
{
    UtlBoolean bRC = FALSE  ;
    int iCost = -1 ;

    if (mpMediaInterface)
    {
        iCost = mpMediaInterface->getCodecCPULimit();
        bRC = TRUE ;
    }

    OsProtectedEvent* getNumEvent = (OsProtectedEvent*) ((CpMultiStringMessage&)eventMessage).getInt1Data();

    // If the event has already been signalled, clean up
    if(OS_ALREADY_SIGNALED == getNumEvent->signal(iCost))
    {
        // The other end must have timed out on the wait
        OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
        eventMgr->release(getNumEvent);
    }

    return bRC ;
}


// Handles the processing of a CallManager::CP_ADD_TONE_LISTENER
// message
UtlBoolean CpPeerCall::handleAddToneListener(OsMsg* pEventMessage)
{
    // add tone listener to all connecitons
    Connection* connection = NULL;
    intptr_t pListener = ((CpMultiStringMessage*)pEventMessage)->getInt1Data();

    OsReadLock lock(mConnectionMutex);
    UtlDListIterator iterator(mConnections);

    while ((connection = (Connection*) iterator()))
    {
        addToneListenerToFlowGraph(pListener, connection);
    }

    // add tone listener to a local list
    if (!mToneListeners.containsReference((UtlContainable*) pListener))
        mToneListeners.append((UtlContainable*) pListener);

    return TRUE ;
}


// Handles the processing of a CallManager::CP_REMOVE_TONE_LISTENER
// message
UtlBoolean CpPeerCall::handleRemoveToneListener(OsMsg* pEventMessage)
{
    // remove tone listener from all connecitons
    Connection* connection = NULL;
    intptr_t pListener = ((CpMultiStringMessage*)pEventMessage)->getInt1Data();

    OsReadLock lock(mConnectionMutex);
    UtlDListIterator iterator(mConnections);

    while ((connection = (Connection*) iterator()))
    {
        removeToneListenerFromFlowGraph(pListener, connection);
    }

    // remove tone listener from local list
    if (!mToneListeners.containsReference((UtlContainable*)  pListener))
        mToneListeners.removeReference((UtlContainable*) pListener);


    return TRUE ;
}


// Handles the processing of a CallManager::CP_TRANSFER_CONNECTION_STATUS
// message
UtlBoolean CpPeerCall::handleTransferConnectionStatus(OsMsg* pEventMessage)
{
    // This message is sent to the target call on the transfer controller

    UtlString connectionAddress;
    ((CpMultiStringMessage*)pEventMessage)->getString2Data(connectionAddress);
    int connectionState = ((CpMultiStringMessage*)pEventMessage)->getInt1Data();
    int cause = ((CpMultiStringMessage*)pEventMessage)->getInt2Data();

#ifdef TEST_PRINT
    UtlString connState;
    Connection::getStateString(connectionState, &connState);
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpPeerCall::handleTransferConnectionStatus "
                  "CP_TRANSFER_CONNECTION_STATUS connectionAddress: %s state: %s cause: %d\n",
                  connectionAddress.data(), connState.data(), cause);
#endif
    {
        // Find the connection and give it the status
        OsReadLock lock(mConnectionMutex);
        Connection* connection = findHandlingConnection(connectionAddress);
        if(connection)
        {
            OsSysLog::add(FAC_CP, PRI_DEBUG,
                          "CpPeerCall::handleTransferConnectionStatus "
                          "transferControllerStatus");
            connection->transferControllerStatus(connectionState, cause);
        }
#ifdef TEST_PRINT
        else
        {
            UtlString defaultCallId;
            getCallId(defaultCallId);
            OsSysLog::add(FAC_CP, PRI_ERR,
                          "CpPeerCall::handleTransferConnectionStatus "
                          "CP_TRANSFER_CONNECTION_STATUS FAILED to find connection %s in call: %s\n",
                          connectionAddress.data(), defaultCallId.data());
        }
#endif
    }

    // Stop the meta event
#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpPeerCall::handleTransferConnectionStatus "
                  "stopMetaEvent 7");
#endif
    stopMetaEvent();

    // Check if call is dead and drop it if it is
    dropIfDead();

    return TRUE ;
}


// Handles the processing of a CallManager::CP_TRANSFEREE_CONNECTION_STATUS
// message
UtlBoolean CpPeerCall::handleTransfereeConnectionStatus(OsMsg* pEventMessage)
{
    // This message gets sent to the original call on the
    // transferee

    UtlString connectionAddress;
    ((CpMultiStringMessage*)pEventMessage)->getString2Data(connectionAddress);
    int connectionState = ((CpMultiStringMessage*)pEventMessage)->getInt1Data();
    int responseCode = ((CpMultiStringMessage*)pEventMessage)->getInt2Data();
#ifdef TEST_PRINT
    UtlString connectionStateString;
    Connection::getStateString(connectionState, &connectionStateString);
    osPrintf("%s-CpPeerCall::CP_TRANSFEREE_CONNECTION_STATUS connectionAddress: \"%s\" state: %s response: %d\n",
        mName.data(), connectionAddress.data(), connectionStateString.data(), responseCode);
#endif
    // Find the connection and give it the status
    {
        OsReadLock lock(mConnectionMutex);
        Connection* connection = findHandlingConnection(connectionAddress);
        if(connection)
        {
            connection->transfereeStatus(connectionState, responseCode);
        }
#ifdef TEST_PRINT
        else
        {
            UtlString thisCallId;
            getCallId(thisCallId);
            osPrintf("%s-CpPeerCall::CP_TRANSFEREE_CONNECTION_STATUS connection not found in call: %s\n",
                mName.data(), thisCallId.data());
        }
#endif
    }

    // Stop the meta event
#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpPeerCall::handleTransfereeConnectionStatus "
                  "stopMetaEvent 8");
#endif
    stopMetaEvent();

    // Check if call is dead and drop it if it is
    dropIfDead();

    return TRUE ;
}


// Handles the processing of CallManager::CP_GET_NUM_CONNECTIONS
// and CallManager::CP_GET_NUM_TERM_CONNECTIONS messages
UtlBoolean CpPeerCall::handleGetNumConnections(OsMsg* pEventMessage)
{
    int numConnections = mConnections.entries() + 1;
    OsProtectedEvent* getNumEvent = (OsProtectedEvent*)
        ((CpMultiStringMessage*)pEventMessage)->getInt1Data();

    // If the event has already been signalled, clean up
    if(OS_ALREADY_SIGNALED == getNumEvent->signal(numConnections))
    {
        // The other end must have timed out on the wait
        OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
        eventMgr->release(getNumEvent);
    }

    return TRUE ;
}


// Handles the processing of a CallManager::CP_GET_CONNECTIONS
// message
UtlBoolean CpPeerCall::handleGetConnections(OsMsg* pEventMessage)
{
    int numConnections = 0;
    UtlSList* connectionList = 0;
    OsProtectedEvent* getConnEvent = (OsProtectedEvent*)
        ((CpMultiStringMessage*)pEventMessage)->getInt1Data();
    getConnEvent->getIntData((intptr_t&)connectionList);

    if(getConnEvent && connectionList)
    {
        // Add the local connection/address
        connectionList->append(new UtlString(mLocalAddress));
        numConnections++;

        // Get the remote connection(s)/address(es)
        { // scope for lock
            Connection* connection = NULL;
            UtlString address;
            OsReadLock lock(mConnectionMutex);
            UtlDListIterator iterator(mConnections);
            while ((connection = (Connection*) iterator()))
            {
                connection->getRemoteAddress(&address);
                connectionList->append(new UtlString(address));
                numConnections++;
            }
        }
        // Signal the caller that we are done.
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == getConnEvent->signal(numConnections))
        {
            // The other end must have timed out on the wait
            connectionList->destroyAll();
            delete connectionList;
            OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
            eventMgr->release(getConnEvent);
        }
    }

    return TRUE ;
}


// Handles the processing of a CallManager::CP_GET_SESSION
// message
UtlBoolean CpPeerCall::handleGetSession(OsMsg* pEventMessage)
{
    UtlString address;
    UtlString callId;
    bool foundMatch = FALSE;

    ((CpMultiStringMessage*)pEventMessage)->getString1Data(callId);
    ((CpMultiStringMessage*)pEventMessage)->getString2Data(address);
    SipSession* sessionPtr;
    OsProtectedEvent* getFieldEvent = (OsProtectedEvent*)
                 ((CpMultiStringMessage*)pEventMessage)->getInt1Data();
    getFieldEvent->getIntData((intptr_t&)sessionPtr);

    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpPeerCall::handleGetSession "
                  "session: %p for callId %s address %s",
                  sessionPtr, callId.data(), address.data());

    // Check whether the tag is set in addresses or not. If so, do not need to use callId
    // for comparison.
    UtlString address_without_tag;
    UtlBoolean hasTag = checkForTag(address, address_without_tag);

    // Get the remote connection(s)/address(es)
    Connection* connection = NULL;
    UtlString localAddress;
    UtlString remoteAddress;
    UtlString connCallId;
    OsReadLock lock(mConnectionMutex);
    UtlDListIterator iterator(mConnections);
    while ((connection = (Connection*) iterator()))
    {
        connection->getCallId(&connCallId);
        connection->getLocalAddress(&localAddress);
        connection->getRemoteAddress(&remoteAddress);

        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CpPeerCall::handleGetSession looking at %s, %s, %s",
                      connCallId.data(), localAddress.data(),
                      remoteAddress.data());

        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CpPeerCall::handleGetSession callId= %s, address= %s, hasTag=%d ",
                      callId.data(), address.data(),
                      hasTag);
        // Compare the call-ID and the address, both with the tag parameter
        // (as the address is stored after call establishment) and without
        // (as the address is stored during call offering).
        if (callId.compareTo(connCallId) == 0 &&
            (address.compareTo(localAddress) == 0 ||
             address.compareTo(remoteAddress) == 0 ||
             (hasTag && address_without_tag.compareTo(localAddress) == 0) ||
             (hasTag && address_without_tag.compareTo(remoteAddress) == 0)))
        {
            SipSession session;
            connection->getSession(session);
            OsSysLog::add(FAC_CP, PRI_DEBUG,
                          "CpPeerCall::handleGetSession "
                          "copying session: %p",
                          sessionPtr);

            *sessionPtr = SipSession(session);
            foundMatch = TRUE;
            // Signal the caller that we are done.
            break;
        }
    }

    if (foundMatch == FALSE)
    {
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CpPeerCall::handleGetSession "
                      "no matching session found");
    }

    // If the event has already been signalled, clean up
    if(OS_ALREADY_SIGNALED == getFieldEvent->signal(1))
    {
        // The other end must have timed out on the wait
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CpPeerCall::handleGetSession deleting session: %p",
                      sessionPtr);
        delete sessionPtr;
        sessionPtr = NULL;

        OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
        eventMgr->release(getFieldEvent);
    }

    return TRUE ;
}


// Handles the processing of a CallManager::CP_GET_INVITE
// message
UtlBoolean CpPeerCall::handleGetInvite(OsMsg* pEventMessage)
{
    UtlString address;
    UtlString callId;
    ((CpMultiStringMessage*)pEventMessage)->getString1Data(callId);
    ((CpMultiStringMessage*)pEventMessage)->getString2Data(address);
    SipMessage* messagePtr;
    OsProtectedEvent* getFieldEvent = (OsProtectedEvent*)
        ((CpMultiStringMessage*)pEventMessage)->getInt1Data();
    getFieldEvent->getIntData((intptr_t&)messagePtr);

    OsSysLog::add(FAC_CP, PRI_DEBUG, "CpPeerCall::handleGetInvite message: %p for callId %s address %s",
                  messagePtr, callId.data(), address.data());

    // Check whether the tag is set in addresses or not. If so, do not need to use callId
    // for comparison.
    UtlString address_without_tag;
    UtlBoolean hasTag = checkForTag(address, address_without_tag);

    // Get the remote connection(s)/address(es)
    Connection* connection = NULL;
    UtlString localAddress;
    UtlString remoteAddress;
    UtlString connCallId;
    OsReadLock lock(mConnectionMutex);
    UtlDListIterator iterator(mConnections);
    while ((connection = (Connection*) iterator()))
    {
        connection->getCallId(&connCallId);
        connection->getLocalAddress(&localAddress);
        connection->getRemoteAddress(&remoteAddress);

        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CpPeerCall::handleGetInvite looking at %s, %s, %s",
                      connCallId.data(), localAddress.data(),
                      remoteAddress.data());

        // Compare the call-ID and the address, both with the tag parameter
        // (as the address is stored after call establishment) and without
        // (as the address is stored during call offering).
        if (callId.compareTo(connCallId) == 0 &&
            (address.compareTo(localAddress) == 0 ||
             address.compareTo(remoteAddress) == 0 ||
             (hasTag && address_without_tag.compareTo(localAddress) == 0) ||
             (hasTag && address_without_tag.compareTo(remoteAddress) == 0)))
        {
           // Have to cast the Connection* into a SipConnection*, because only
           // SipConnection has an INVITE attached.
           SipConnection* sipConnection =
              dynamic_cast <SipConnection*> (connection);
           // No action if the cast fails.
           if (sipConnection)
           {
              // Copy the saved INVITE from the SipConnection into *messagePtr.
              sipConnection->getInvite(messagePtr);
              if (OsSysLog::willLog(FAC_CP, PRI_DEBUG))
              {
                 UtlString text;
                 ssize_t length;
                 messagePtr->getBytes(&text, &length);
                 OsSysLog::add(FAC_CP, PRI_DEBUG,
                               "CpPeerCall::handleGetInvite INVITE found '%s'",
                               text.data());
              }
           }
           else
           {
              // The connection should always be a SipConnection.
              OsSysLog::add(FAC_CP, PRI_WARNING,
                            "CpPeerCall::handleGetInvite could not cast Connection %p to SipConnection",
                            connection);
           }

           // Signal the caller that we are done.
           break;
        }
    }

    // If the event has already been signalled, clean up
    if(OS_ALREADY_SIGNALED == getFieldEvent->signal(1))
    {
        // The other end must have timed out on the wait
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CpPeerCall::handleGetInvite deleting message: %p",
                      messagePtr);
        delete messagePtr;
        messagePtr = NULL;

        OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
        eventMgr->release(getFieldEvent);
    }

    return TRUE ;
}


// Handles the processing of a CallManager::CP_GET_CALLSTATE
// message
UtlBoolean CpPeerCall::handleGetCallState(OsMsg* pEventMessage)
{
    OsProtectedEvent* getStateEvent = (OsProtectedEvent*)
        ((CpMultiStringMessage*)pEventMessage)->getInt1Data();

    // Signal the caller that we are done.
    // If the event has already been signalled, clean up
    if(OS_ALREADY_SIGNALED == getStateEvent->signal(mCallState))
    {
        // The other end must have timed out on the wait
        OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
        eventMgr->release(getStateEvent);
    }

    return TRUE ;
}


// Handles the processing of a CallManager::CP_GET_CONNECTIONSTATE
// message
UtlBoolean CpPeerCall::handleGetConnectionState(OsMsg* pEventMessage)
{
    UtlString remoteAddress;
    ((CpMultiStringMessage*)pEventMessage)->getString2Data(remoteAddress);
    OsProtectedEvent* getStateEvent = (OsProtectedEvent*)
        ((CpMultiStringMessage*)pEventMessage)->getInt1Data();

    int state;
    if (!getConnectionState(remoteAddress, state))
        state = PtConnection::UNKNOWN;

    // Signal the caller that we are done.
    // If the event has already been signalled, clean up
    if(OS_ALREADY_SIGNALED == getStateEvent->signal(state))
    {
        // The other end must have timed out on the wait
        OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
        eventMgr->release(getStateEvent);
    }

    return TRUE ;
}


// Handles the processing of a CallManager::CP_GET_NEXT_CSEQ
// message
UtlBoolean CpPeerCall::handleGetNextCseq(OsMsg* pEventMessage)
{
    UtlString remoteAddress;
    ((CpMultiStringMessage*)pEventMessage)->getString2Data(remoteAddress);
    OsProtectedEvent* getStateEvent = (OsProtectedEvent*)
        ((CpMultiStringMessage*)pEventMessage)->getInt1Data();
    Connection* connection = findHandlingConnection(remoteAddress);

    int nextCseq = -1;
    if(connection)
    {
        // Bad bad assumption on the connection type
        // This suppose to be a short term solution
        nextCseq = ((SipConnection*)connection)->getNextCseq();
    }

    // Signal the caller that we are done.
    // If the event has already been signalled, clean up
    if(OS_ALREADY_SIGNALED == getStateEvent->signal(nextCseq))
    {
        // The other end must have timed out on the wait
        OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
        eventMgr->release(getStateEvent);
    }

    return TRUE ;
}

// Enumerate possible contact addresses
void CpPeerCall::getLocalContactAddresses( ContactAddress contacts[],
                                          size_t nMaxContacts,
                                          size_t& nActualContacts)
{
    UtlString ipAddress ;
    int port ;

    // Order: Local, NAT, configure
    nActualContacts = 0 ;

    if (    (nActualContacts < nMaxContacts) &&
        (sipUserAgent->getLocalAddress(&ipAddress, &port)))
    {
        contacts[nActualContacts].eContactType = ContactAddress::LOCAL;
        strncpy(contacts[nActualContacts].cIpAddress, ipAddress.data(), 32);
        contacts[nActualContacts].iPort = port;
        nActualContacts++;
    }

    if (    (nActualContacts < nMaxContacts) &&
        (sipUserAgent->getNatMappedAddress(&ipAddress, &port)))
    {
        contacts[nActualContacts].eContactType = ContactAddress::NAT_MAPPED;
        strncpy(contacts[nActualContacts].cIpAddress, ipAddress.data(), 32);
        contacts[nActualContacts].iPort = port;
        nActualContacts++;
    }

    if (    (nActualContacts < nMaxContacts) &&
        (sipUserAgent->getConfiguredPublicAddress(&ipAddress, &port)))
    {
        contacts[nActualContacts].eContactType = ContactAddress::CONFIG;
        strncpy(contacts[nActualContacts].cIpAddress, ipAddress.data(), 32);
        contacts[nActualContacts].iPort = port;
        nActualContacts++;
    }
}


UtlBoolean CpPeerCall::handleGetLocalContacts(OsMsg* pEventMessage)
{
    OsProtectedEvent* pProtectedEvent = (OsProtectedEvent*)
        ((CpMultiStringMessage*)pEventMessage)->getInt1Data();

    ContactAddress* addresses = (ContactAddress*) ((CpMultiStringMessage*)pEventMessage)->getInt2Data();
    size_t nMaxAddresses = (size_t) ((CpMultiStringMessage*)pEventMessage)->getInt3Data();
    size_t* nActualAddresses = (size_t*) ((CpMultiStringMessage*)pEventMessage)->getInt4Data();

    getLocalContactAddresses(addresses, nMaxAddresses, *nActualAddresses) ;

    // Signal the caller that we are done.
    // If the event has already been signalled, clean up
    if(OS_ALREADY_SIGNALED == pProtectedEvent->signal(0))
    {
        // The other end must have timed out on the wait
        OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
        eventMgr->release(pProtectedEvent);
    }

    return TRUE ;
}


// Handles the processing of a
// CallManager::CP_GET_TERMINALCONNECTIONSTATE message
UtlBoolean CpPeerCall::handleGetTerminalConnectionState(OsMsg* pEventMessage)
{
    UtlString remoteAddress;
    UtlString terminal;
    ((CpMultiStringMessage*)pEventMessage)->getString2Data(remoteAddress);
    ((CpMultiStringMessage*)pEventMessage)->getString3Data(terminal);
    OsProtectedEvent* getStateEvent = (OsProtectedEvent*)
        ((CpMultiStringMessage*)pEventMessage)->getInt1Data();

    int state;
    if (!getTermConnectionState(remoteAddress, terminal, state))
        state = PtTerminalConnection::UNKNOWN;

    // Signal the caller that we are done.
    // If the event has already been signalled, clean up
    if(OS_ALREADY_SIGNALED == getStateEvent->signal(state))
    {
        // The other end must have timed out on the wait
        OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
        eventMgr->release(getStateEvent);
    }

    return TRUE ;
}


// Handles the processing of a CallManager::CP_IS_LOCAL_TERM_CONNECTION
// message
UtlBoolean CpPeerCall::handleIsLocalTerminalConnection(OsMsg* pEventMessage)
{
    UtlString terminalId;
    ((CpMultiStringMessage*)pEventMessage)->getString3Data(terminalId);
    OsProtectedEvent* getNumEvent =
        (OsProtectedEvent*) ((CpMultiStringMessage*)pEventMessage)->getInt1Data();

#ifdef TEST_PRINT
    osPrintf("%s-CpPeerCall::CP_IS_LOCAL_TERM_CONNECTION terminal ID:%s event: %x\n",
        mName.data(), terminalId.data(), getNumEvent);
#endif

    UtlBoolean isLocal = FALSE;

    if(isLocalTerminal(terminalId) && getNumEvent)
    {
        isLocal = TRUE;
    }

    // If the event has already been signalled, clean up
    if(OS_ALREADY_SIGNALED == getNumEvent->signal(isLocal))
    {
        // The other end must have timed out on the wait
        OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
        eventMgr->release(getNumEvent);
    }

    return TRUE ;
}


// Handles the processing of a CallManager::CP_CANCEL_TIMER
// message
UtlBoolean CpPeerCall::handleCancelTimer(OsMsg* pEventMessage)
{
    // Find the connection to be transitioned out of OFFERING
    UtlString address;
    ((CpMultiStringMessage*)pEventMessage)->getString2Data(address);
    Connection*  connection = findHandlingConnection(address);

    if(connection)
    {
        connection->forceHangUp();
        dropIfDead();
    }

    return TRUE ;
}


// Handles the processing of a CallManager::CP_OFFERING_EXPIRED
// message
UtlBoolean CpPeerCall::handleOfferingExpired(OsMsg* pEventMessage)
{
    // Find the connection to be transitioned out of OFFERING
    UtlString address;
    ((CpMultiStringMessage*)pEventMessage)->getString2Data(address);
    OsReadLock lock(mConnectionMutex);
    Connection*  connection = findHandlingConnection(address);

    if(connection)
    {
        int connectionState = connection->getState();

        // If we do have a timeout, note it to both the
        // console and syslog
        if (connectionState == Connection::CONNECTION_OFFERING)
        {
            UtlString    msg ;
            SipSession  session ;
            Url         urlFrom, urlTo ;
            UtlString    callId, from, to ;

            connection->getSession(session) ;
            session.getCallId(callId) ;
            session.getFromUrl(urlFrom) ;
            urlFrom.toString(from) ;
            session.getToUrl(urlTo) ;
            urlTo.toString(to) ;

            msg = "CP_OFFERING_EXPIRED for address: " + address ;
            msg += "\n\tHandling CallId: " + callId ;
            msg += "\n\tHandling From: " + from ;
            msg += "\n\tHandling To: " + to ;

            OsSysLog::add(FAC_CP, PRI_ERR, "%s", msg.data()) ;
#ifdef TEST_PRINT
            osPrintf("%s-%s\n", mName.data(), msg.data()) ;
#endif
        }

        // If the call is in focus when the offering
        // timer expired, assume we will accept the call
        if(mCallInFocus &&
            connectionState == Connection::CONNECTION_OFFERING)
        {
            // Unconditional forwarding is on
            if(lineAvailableBehavior ==
                Connection::FORWARD_UNCONDITIONAL &&
                !forwardUnconditional.isNull())
            {
#ifdef TEST_PRINT
                osPrintf("%s-CpPeerCall::CP_OFFERING_EXPIRED unconditional forward to: %s",
                    mName.data(), forwardUnconditional.data());
#endif
                UtlString forwardAddressUrl(forwardUnconditional.data());
                if (PT_SUCCESS == mpManager->validateAddress(forwardAddressUrl))
                    connection->redirect(forwardAddressUrl.data());
#ifdef TEST_PRINT
                else
                {
                    osPrintf("%s-ERROR: CpPeerCall::invalid  forwarding (unconditional forwarding) address: %s\n",
                        mName.data(), forwardUnconditional.data());
                }
#endif
                forwardAddressUrl = OsUtil::NULL_OS_STRING;
            }

            // Otherwise accept the call
            else
            {
#ifdef TEST_PRINT
                osPrintf("%s-CpPeerCall::CP_OFFERING_EXPIRED accepting", mName.data());
#endif
                connection->accept(noAnswerTimeout);
            }
        }

        // The call is out of focus reject the offer
        else if(connectionState == Connection::CONNECTION_OFFERING)
        {
            // If forward on busy is enabled
            if(lineBusyBehavior == Connection::FORWARD_ON_BUSY &&
                !forwardOnBusy.isNull())
            {
#ifdef TEST_PRINT
                osPrintf("%s-CpPeerCall::CP_OFFERING_EXPIRED forward on busy to: %s",
                    mName.data(), forwardOnBusy.data());
#endif

                UtlString forwardAddressUrl(forwardOnBusy.data());
                if (PT_SUCCESS == mpManager->validateAddress(forwardAddressUrl))
                    connection->redirect(forwardAddressUrl.data());
#ifdef TEST_PRINT
                else
                {
                    osPrintf("%s-ERROR: CpPeerCall::invalid forwarding (forward on busy)  address: %s\n",
                        mName.data(), forwardOnBusy.data());
                }
#endif
                forwardAddressUrl = OsUtil::NULL_OS_STRING;
            }

            // Otherwise reject the call
            else
            {
#ifdef TEST_PRINT
                osPrintf("%s-CpPeerCall::CP_OFFERING_EXPIRED rejecting", mName.data());
#endif
                connection->reject(SIP_BUSY_CODE, SIP_BUSY_TEXT);
            }
        }
    }
#ifdef TEST_PRINT
    else
    {
        osPrintf("%s-ERROR: CpPeerCall::CP_OFFERING_EXPIRED cannot find connectionId: %s\n",
            mName.data(), address.data());
    }
#endif

    return TRUE ;
}


// Handles the processing of a CallManager::CP_RINGING_EXPIRED
// message
UtlBoolean CpPeerCall::handleRingingExpired(OsMsg* pEventMessage)
{
    // Find the connection to be transitioned out of OFFERING
    UtlString address;
    ((CpMultiStringMessage*)pEventMessage)->getString2Data(address);
    OsReadLock lock(mConnectionMutex);
    Connection*  connection = findHandlingConnection(address);

    // We got here because forward on no answer is enabled
    // and the timer expired.
    if(connection &&
        connection->getState() ==
        Connection::CONNECTION_ALERTING)
    {
        if (lineAvailableBehavior == Connection::FORWARD_ON_NO_ANSWER &&
            !forwardOnNoAnswer.isNull())
        {
#ifdef TEST_PRINT
            osPrintf("%s-CpPeerCall::CP_OFFERING_EXPIRED forward on no answer to: %s",
                mName.data(), forwardOnNoAnswer.data());
#endif

            UtlString forwardAddressUrl(forwardOnNoAnswer.data());
            if (PT_SUCCESS == mpManager->validateAddress(forwardAddressUrl))
                connection->redirect(forwardAddressUrl.data());
#ifdef TEST_PRINT
            else
            {
                osPrintf("%s-ERROR: CpPeerCall::invalid forwarding( forwardOnNoAnswer) address: %s\n",
                    mName.data(), forwardOnNoAnswer.data());
            }
#endif
            forwardAddressUrl = OsUtil::NULL_OS_STRING;
        }

        // We now drop the call if no one picks up this call after so long
        else
        {
#ifdef TEST_PRINT
            osPrintf("%s-CpPeerCall::handleRingingExpired rejecting", mName.data());
#endif
            connection->reject(SIP_BUSY_CODE, SIP_BUSY_TEXT);
        }
    }

    return TRUE ;
}


// Handles the processing of a CallManager::CP_UNHOLD_ALL_TERM_CONNECTIONS
// message
UtlBoolean CpPeerCall::handleUnholdAllTermConnections(OsMsg* pEventMessage)
{
    Connection* connection = NULL;
    OsReadLock lock(mConnectionMutex);
    UtlDListIterator iterator(mConnections);

    while ((connection = (Connection*) iterator()))
    {
        connection->offHold();

        if (mLocalTermConnectionState != PtTerminalConnection::TALKING &&
            mLocalTermConnectionState != PtTerminalConnection::IDLE)
        {
            UtlString responseText;
            UtlString remoteAddress;

            connection->getResponseText(responseText);
            connection->getRemoteAddress(&remoteAddress);
            UtlString connCallId;
            connection->getCallId(&connCallId);

            postTaoListenerMessage(connection->getResponseCode(),
                                   responseText,
                                   PtEvent::TERMINAL_CONNECTION_TALKING,
                                   TERMINAL_CONNECTION_STATE,
                                   PtEvent::CAUSE_UNHOLD,
                                   connection->isRemoteCallee(),
                                   remoteAddress,
                                   0,
                                   connCallId);

            connection->fireSipXEvent(CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE) ;
        }
    }

    return TRUE ;
}


// Handles the processing of a CallManager::CP_UNHOLD_LOCAL_TERM_CONNECTION
// message
UtlBoolean CpPeerCall::handleUnholdLocalTermConnection(OsMsg* pEventMessage)
{
    // Post a message to the callManager to change focus
    CpIntMessage localHoldMessage(CallManager::CP_GET_FOCUS, (intptr_t)this);
    mpManager->postMessage(localHoldMessage);
    mLocalHeld = FALSE;

    return TRUE ;
}


// Handles the processing of a CallManager::CP_HOLD_LOCAL_TERM_CONNECTION
// message
UtlBoolean CpPeerCall::handleHoldLocalTermConnection(OsMsg* pEventMessage)
{
    // Post a message to the callManager to change focus
    CpIntMessage localHoldMessage(CallManager::CP_YIELD_FOCUS, (intptr_t)this);
    mpManager->postMessage(localHoldMessage);
    mLocalHeld = TRUE;

    return TRUE ;
}


UtlBoolean CpPeerCall::handleCallMessage(OsMsg& eventMessage)
{
    int msgSubType = eventMessage.getMsgSubType();
    UtlBoolean processedMessage = TRUE;
    CpMultiStringMessage* multiStringMessage = (CpMultiStringMessage*)&eventMessage;

    if(   msgSubType != CallManager::CP_SIP_MESSAGE
       && msgSubType != CallManager::CP_DIAL_STRING)
    {
        CpCall::addHistoryEvent(msgSubType, multiStringMessage);
    }

    // Either we are the caller and are done dialing at this point
    // Or we are the callee, turn off local only DTMF tone
    mRemoteDtmf = TRUE;

    switch(msgSubType)
    {
    case CallManager::CP_SIP_MESSAGE:
        handleSipMessage(&eventMessage);
        break;

    case CallManager::CP_DROP_CONNECTION:
        handleDropConnection(&eventMessage) ;
        break;

    case CallManager::CP_FORCE_DROP_CONNECTION:
        handleForceDropConnection(&eventMessage) ;
        break;

    case CallManager::CP_DIAL_STRING:
        handleDialString(&eventMessage);
        break;

    case CallManager::CP_OUTGOING_INFO:
        handleSendInfo(&eventMessage);
        break;

    case CallManager::CP_DEQUEUED_CALL:
        handleDequeueCall(&eventMessage) ;
        break;

    case CallManager::CP_ANSWER_CONNECTION:
    {
        CpMultiStringMessage* pMessage = (CpMultiStringMessage*)&eventMessage;
        const void* pDisplay = (void*) pMessage->getInt1Data();
        offHook(pDisplay);
        free((void*) pDisplay);
        break;
    }

    case CallManager::CP_BLIND_TRANSFER:
    case CallManager::CP_CONSULT_TRANSFER:
        handleTransfer(&eventMessage) ;
        break ;

    case CallManager::CP_CONSULT_TRANSFER_ADDRESS:
        handleTransferAddress(&eventMessage) ;
        break ;

    case CallManager::CP_TRANSFER_CONNECTION:
        handleTransferConnection(&eventMessage);
        break ;

    case CallManager::CP_TRANSFER_CONNECTION_STATUS:
        handleTransferConnectionStatus(&eventMessage);
        break ;

    case CallManager::CP_TRANSFEREE_CONNECTION:
        handleTransfereeConnection(&eventMessage);
        break;

    case CallManager::CP_TRANSFEREE_CONNECTION_STATUS:
        handleTransfereeConnectionStatus(&eventMessage);
        break ;

    case CallManager::CP_GET_NUM_CONNECTIONS:
    case CallManager::CP_GET_NUM_TERM_CONNECTIONS:
        handleGetNumConnections(&eventMessage);
        break ;

    case CallManager::CP_GET_MEDIA_CONNECTION_ID:
        handleGetMediaConnectionId(&eventMessage);
        break;

    case CallManager::CP_GET_CAN_ADD_PARTY:
        handleGetCanAddParty(&eventMessage);
        break;

    case CallManager::CP_SPLIT_CONNECTION:
        handleSplitConnection(&eventMessage);
        break ;

    case CallManager::CP_JOIN_CONNECTION:
        handleJoinConnection(&eventMessage);
        break ;

    case CallManager::CP_GET_CONNECTIONS:
        handleGetConnections(&eventMessage);
        break ;

    case CallManager::CP_GET_CALLED_ADDRESSES:
    case CallManager::CP_GET_CALLING_ADDRESSES:
        handleGetAddresses(&eventMessage);
        break ;

    case CallManager::CP_GET_SESSION:
#ifdef TEST_PRINT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CpPeerCall handleCallMessage: "
                      "getsession mDropping %d",
                      mDropping);
#endif
        handleGetSession(&eventMessage);
        break ;

    case CallManager::CP_GET_INVITE:
        handleGetInvite(&eventMessage);
        break ;

    case CallManager::CP_GET_TERM_CONNECTIONS:
        handleGetTermConnections(&eventMessage);
        break;

    case CallManager::CP_GET_CALLSTATE:
        handleGetCallState(&eventMessage);
        break ;

    case CallManager::CP_GET_CONNECTIONSTATE:
        handleGetConnectionState(&eventMessage);
        break ;

    case CallManager::CP_GET_NEXT_CSEQ:
        handleGetNextCseq(&eventMessage);
        break ;

    case CallManager::CP_GET_TERMINALCONNECTIONSTATE:
        handleGetTerminalConnectionState(&eventMessage);
        break ;

    case CallManager::CP_IS_LOCAL_TERM_CONNECTION:
        handleIsLocalTerminalConnection(&eventMessage);
        break ;

    case CallManager::CP_ACCEPT_CONNECTION:
        handleAcceptConnection(&eventMessage);
        break;

    case CallManager::CP_REJECT_CONNECTION:
        handleRejectConnection(&eventMessage);
        break;

    case CallManager::CP_REDIRECT_CONNECTION:
        handleRedirectConnection(&eventMessage);
        break;

    case CallManager::CP_HOLD_TERM_CONNECTION:
        handleHoldTermConnection(&eventMessage);
        break;

    case CallManager::CP_HOLD_ALL_TERM_CONNECTIONS:
        handleHoldAllTermConnection(&eventMessage);
        break;

    case CallManager::CP_UNHOLD_TERM_CONNECTION:
        handleUnholdTermConnection(&eventMessage);
        break;

    case CallManager::CP_RENEGOTIATE_CODECS_CONNECTION:
        handleRenegotiateCodecsConnection(&eventMessage);
        break;

    case CallManager::CP_RENEGOTIATE_CODECS_ALL_CONNECTIONS:
        handleRenegotiateCodecsAllConnections(&eventMessage);
        break ;

    case CallManager::CP_SEND_KEEPALIVE:
        handleSendKeepAlive(&eventMessage);
        break;

    case CallManager::CP_SET_CODEC_CPU_LIMIT:
        handleSetCodecCPULimit(eventMessage);
        break ;

    case CallManager::CP_GET_CODEC_CPU_COST:
        handleGetCodecCPUCost(eventMessage);
        break ;

    case CallManager::CP_GET_CODEC_CPU_LIMIT:
        handleGetCodecCPULimit(eventMessage);
        break ;

    case CallManager::CP_CANCEL_TIMER:
        handleCancelTimer(&eventMessage);
        break ;

    case CallManager::CP_OFFERING_EXPIRED:
        handleOfferingExpired(&eventMessage);
        break ;

    case CallManager::CP_RINGING_EXPIRED:
        handleRingingExpired(&eventMessage);
        break ;

    case CallManager::CP_ADD_TONE_LISTENER:
        handleAddToneListener(&eventMessage);
        break;

    case CallManager::CP_REMOVE_TONE_LISTENER:
        handleRemoveToneListener(&eventMessage);
        break;

    case CallManager::CP_UNHOLD_ALL_TERM_CONNECTIONS:
        handleUnholdAllTermConnections(&eventMessage);
        break ;

    case CallManager::CP_UNHOLD_LOCAL_TERM_CONNECTION:
        handleUnholdLocalTermConnection(&eventMessage);
        break ;

    case CallManager::CP_HOLD_LOCAL_TERM_CONNECTION:
        handleHoldLocalTermConnection(&eventMessage);
        break ;

    case CallManager::CP_SET_OUTBOUND_LINE:
        handleSetOutboundLine( &eventMessage );
        break;

    case CallManager::CP_GET_LOCAL_CONTACTS:
        handleGetLocalContacts(&eventMessage) ;
        break ;
    default:
        processedMessage = FALSE;
#ifdef TEST_PRINT
        osPrintf("%s-Unknown PHONE_APP CallManager message subtype: %d\n",
            mName.data(), msgSubType);
#endif
        break;
    }

    return(processedMessage);
}

UtlBoolean CpPeerCall::handleSendInfo(OsMsg* pEventMessage)
{
    CpMultiStringMessage& infoMessage = (CpMultiStringMessage&) *pEventMessage;
    UtlString callId;
    UtlString contentType;
    UtlString sContent;

    infoMessage.getString1Data(callId);
    infoMessage.getString2Data(contentType);
    infoMessage.getString3Data(sContent);

    UtlString connectionCallId;
    Connection* connection = NULL;
    OsReadLock lock(mConnectionMutex);
    UtlDListIterator iterator(mConnections);

    while ((connection = (Connection*) iterator()))
    {
        connection->getCallId(&connectionCallId);
        if(strcmp(callId, connectionCallId.data()) == 0)
        {
            connection->sendInfo(contentType, sContent);
            break;
        }
    }
    return true;
}

UtlBoolean CpPeerCall::handleGetMediaConnectionId(OsMsg* pEventMessage)
{
    int connectionId = -1;
    CpMultiStringMessage* pMultiMessage = (CpMultiStringMessage*) pEventMessage;
    OsProtectedEvent* event = (OsProtectedEvent*)
        ((CpMultiStringMessage*)pEventMessage)->getInt1Data();

    UtlString callId;
    UtlString remoteAddress;

    pMultiMessage->getString1Data(callId);
    pMultiMessage->getString2Data(remoteAddress);
    void** ppInstData = (void**)pMultiMessage->getInt2Data();

    Connection* connection = NULL;
    OsReadLock lock(mConnectionMutex);
    UtlDListIterator iterator(mConnections);
    while ((connection = (Connection*) iterator()))
    {
        UtlString connectionRemoteAddress;

        connection->getRemoteAddress(&connectionRemoteAddress);
        if (connectionRemoteAddress == remoteAddress)
        {
            connectionId = connection->getConnectionId();
            if (ppInstData)
            {
                *ppInstData = connection->getMediaInterfacePtr();
            }
            break;
        }
    }

    // If the event has already been signalled, clean up
    if(event && OS_ALREADY_SIGNALED == event->signal(connectionId))
    {
        // The other end must have timed out on the wait
        OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
        eventMgr->release(event);
    }

    return true;
}

UtlBoolean CpPeerCall::handleGetCanAddParty(OsMsg* pEventMessage)
{
    UtlBoolean bCanAdd = FALSE ;
    CpMultiStringMessage* pMultiMessage = (CpMultiStringMessage*) pEventMessage;
    OsProtectedEvent* event = (OsProtectedEvent*)
        ((CpMultiStringMessage*)pEventMessage)->getInt1Data();

    UtlString callId;
    pMultiMessage->getString1Data(callId);

    if (mpMediaInterface)
    {
        bCanAdd = mpMediaInterface->canAddParty() ;
    }

    // If the event has already been signalled, clean up
    if(event && OS_ALREADY_SIGNALED == event->signal(bCanAdd))
    {
        // The other end must have timed out on the wait
        OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
        eventMgr->release(event);
    }

    return true;
}

// Handles the processing of a CP_SPLIT_CONNECTION message
UtlBoolean CpPeerCall::handleSplitConnection(OsMsg* pEventMessage)
{
    UtlString sourceCallId ;
    UtlString remoteAddress ;
    UtlString targetCallId ;
    CpMultiStringMessage* pMultiMessage = (CpMultiStringMessage*) pEventMessage;
    pMultiMessage->getString1Data(sourceCallId) ;
    pMultiMessage->getString2Data(remoteAddress) ;
    pMultiMessage->getString3Data(targetCallId) ;
    OsProtectedEvent* pEvent = (OsProtectedEvent*) pMultiMessage->getInt1Data();
    UtlBoolean bAutoUnhold = (UtlBoolean) pMultiMessage->getInt2Data();

    OsSysLog::add(FAC_CP, PRI_INFO, "Splitting connection %s from %s to %s",
            remoteAddress.data(), sourceCallId.data(), targetCallId.data()) ;

    Connection* pConnection = findHandlingConnection(remoteAddress);
    if (pConnection != NULL)
    {
        OsWriteLock lock(mConnectionMutex); // TODO: Move write lock above and inline findHandling

        // Call must be on hold prior to split/join.
        if (pConnection->isHeld())
        {
            pConnection->prepareForSplit() ;
            mConnections.remove(pConnection) ;

            CpMultiStringMessage joinMessage(CallManager::CP_JOIN_CONNECTION,
                    targetCallId,
                    remoteAddress,
                    NULL,
                    NULL,
                    NULL,
                    (intptr_t) pConnection,
                    (intptr_t) pEvent,
                    (int) bAutoUnhold) ;
            mpManager->postMessage(joinMessage);
        }
        else
        {
            if (pEvent)
            {
                pEvent->signal(FALSE) ;
            }
        }
    }
    else
    {
        if (pEvent)
        {
            pEvent->signal(FALSE) ;
        }
    }

    return true ;
}

// Handles the processing of a CP_JOIN_CONNECTION message
UtlBoolean CpPeerCall::handleJoinConnection(OsMsg* pEventMessage)
{
    UtlString remoteAddress ;
    UtlString sourceCallId ;

    CpMultiStringMessage* pMultiMessage = (CpMultiStringMessage*) pEventMessage ;
    Connection* pConnection = (Connection*) pMultiMessage->getInt1Data() ;
    OsProtectedEvent* pEvent = (OsProtectedEvent*) pMultiMessage->getInt2Data() ;

    UtlBoolean bAutoUnhold = (UtlBoolean) pMultiMessage->getInt3Data() ;
    pMultiMessage->getString1Data(sourceCallId) ;
    pMultiMessage->getString2Data(remoteAddress) ;

    OsSysLog::add(FAC_CP, PRI_INFO, "Joining connection %s to %s (unhold=%d)",
            remoteAddress.data(), sourceCallId.data(), bAutoUnhold) ;

    pConnection->prepareForJoin(this, NULL, mpMediaInterface) ;
    addConnection(pConnection) ;

    if (bAutoUnhold)
    {
        pConnection->offHold() ;
    }

    if (pEvent)
    {
        pEvent->signal(TRUE) ;
    }

    return true ;
}


void CpPeerCall::handleSetOutboundLine(OsMsg* pEventMessage)
{
    UtlString strOutboundAddress;
    ((CpMultiStringMessage*)pEventMessage)->getString2Data(strOutboundAddress);

    Url outboundLine(strOutboundAddress);
    outboundLine.toString(mLocalTerminalId);
    mLocalAddress = mLocalTerminalId;
#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpPeerCall::handleSetOutboundLine "
                  "setting mLocalAddress to '%s'",
                  mLocalAddress.data());
#endif
}

void CpPeerCall::handleGetTermConnections(OsMsg* pEventMessage)
{
    int numConnections = 0;
    UtlSList* connectionList;
    UtlString connectionAddress;
    ((CpMultiStringMessage*)pEventMessage)->getString2Data(connectionAddress);
    OsProtectedEvent* getConnEvent = (OsProtectedEvent*) ((CpMultiStringMessage*)pEventMessage)->getInt1Data();
    getConnEvent->getIntData((intptr_t&)connectionList);

    if (connectionAddress.compareTo(mLocalAddress) == 0)
    {
        // Add the local terminalConnection/terminalName
        connectionList->append(new UtlString(mLocalTerminalId));
        numConnections++;
    }
    else
        // Get the remote terminalConnection(s)/terminalName(es)
    { // scope for lock
        OsReadLock lock(mConnectionMutex);
        Connection* connection = NULL;
        UtlString address;
        int found = 0;
        UtlDListIterator iterator(mConnections);
        while ((connection = (Connection*) iterator()))
        {
            Url remoteUrl(connectionAddress);

            if (connection->isSameRemoteAddress(remoteUrl))
            {
                connection->getRemoteAddress(&address);

                address.insert(0, "foreign-terminal-");
                found = 1;
#ifdef TEST_PRINT
                osPrintf("%s-set terminal: %s\n", mName.data(), address.data());
#endif

                connectionList->append(new UtlString(address));
                numConnections++;
            }
        }
        if (!found)    // didn't find match, tags differ?
        {
            connection = findHandlingConnection(connectionAddress);
            if (connection)
            {
                connection->getRemoteAddress(&address);

                address.insert(0, "foreign-terminal-");
#ifdef TEST_PRINT
                osPrintf("%s-CpPeerCall::getTerminalConnections MISSED in first pass found: %s\n",
                    mName.data(), address.data());
#endif
                connectionList->append(new UtlString(address));
                numConnections++;
            }
        }
    }
    // Signal the caller that we are done.
    ;
    // If the event has already been signalled, clean up
    if(OS_ALREADY_SIGNALED == getConnEvent->signal(numConnections))
    {
        // The other end must have timed out on the wait
        connectionList->destroyAll();
        delete connectionList;
        OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
        eventMgr->release(getConnEvent);
    }
}


Connection* CpPeerCall::addParty(const char* transferTargetAddress,
                                 const char* callController,
                                 const char* originalCallConnectionAddress,
                                 const char* newCallId,
                                 ContactId contactId,
                                 const void* pDisplay,
                                 const char* originalCallId,
                                 const char* paiAddress)
{
    SipConnection* connection = NULL;

    // Should be using the outgoing call type here
    // for SIP, MGCP, etc.
#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpPeerCall::addParty "
                  "mLocalAddress is '%s' paiAddr= '%s'",
                  mLocalAddress.data(), paiAddress);
#endif
    connection = new SipConnection(mLocalAddress,
                                   mIsEarlyMediaFor180,
                                   mpManager,
                                   this,
                                   mpMediaInterface,
                                   sipUserAgent,
                                   offeringDelay,
                                   mSipSessionReinviteTimer);

    connection->setContactId(contactId);
    ContactAddress* pContact = NULL;

    pContact = sipUserAgent->getContactDb().find(contactId);

    if (pContact)
    {
        connection->setContactType(pContact->eContactType);
    }
    else
    {
        connection->setContactType(ContactAddress::AUTO);
    }
    addConnection(connection);

    UtlString callId;
    // Get the call-id from the call
    getCallId(callId);

    if (newCallId != NULL)
    {
        callId = newCallId ;
    }

    connection->dial(transferTargetAddress,
                     mLocalAddress.data(),
                     callId.data(),
                     callController,
                     originalCallConnectionAddress,
                     FALSE,
                     pDisplay,
                     originalCallId,
                     paiAddress);

    addToneListenersToConnection(connection) ;

    return connection;
}

UtlBoolean CpPeerCall::hasCallId(const char* callIdString)
{
    UtlString connectionCallId;
    UtlBoolean foundCallId = FALSE;
    Connection* connection = NULL;
    OsReadLock lock(mConnectionMutex);
    UtlDListIterator iterator(mConnections);

    while ((connection = (Connection*) iterator()))
    {
        connection->getCallId(&connectionCallId);
        if(strcmp(callIdString, connectionCallId.data()) == 0)
        {
            foundCallId = TRUE;
            break;
        }

    }

    UtlString callId;
    getCallId(callId);
    if(!foundCallId && callId.compareTo(callIdString) == 0)
    {
        foundCallId = TRUE;
    }

    return(foundCallId);
}

void CpPeerCall::inFocus(int talking)
{
    OsReadLock lock(mConnectionMutex);
    Connection* connection = (Connection*) mConnections.first();

    int remoteIsCallee = 1;
    UtlString remoteAddress;
    if(connection)
    {
        UtlString connectionCallId;
        connection->getCallId(&connectionCallId);
        remoteIsCallee = connection->isRemoteCallee();
        connection->getRemoteAddress(&remoteAddress);
#ifdef TEST_PRINT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CpPeerCall::inFocus "
                      "connection call-id= %s out of focus?",
                      connectionCallId.data());
#endif
    }

    // Notify listeners that the local connection is in focus
    if (!talking)
    {
        int responseCode = 0;
        UtlString responseText;
        if (connection)
        {
            responseCode = connection->getResponseCode();
            connection->getResponseText(responseText);
        }

        if (getCallState() != PtCall::ACTIVE)
        {
            setCallState(responseCode, responseText, PtCall::ACTIVE, PtEvent::CAUSE_NEW_CALL);
        }
#ifdef TEST_PRINT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CpPeerCall::inFocus "
                      "post %d (%s) INITIATED",
                      responseCode, responseText.data());
#endif
        postTaoListenerMessage(responseCode,
                               responseText,
                               PtEvent::CONNECTION_INITIATED,
                               CONNECTION_STATE,
                               PtEvent::CAUSE_NEW_CALL,
                               remoteIsCallee,
                               remoteAddress);

        if (mLocalTermConnectionState == PtTerminalConnection::IDLE)
        {
#ifdef TEST_PRINT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CpPeerCall::inFocus "
                      "post %d (%s) CONNECTION_CREATED",
                      responseCode, responseText.data());
#endif
            postTaoListenerMessage(responseCode,
                                   responseText,
                                   PtEvent::TERMINAL_CONNECTION_CREATED,
                                   TERMINAL_CONNECTION_STATE,
                                   PtEvent::CAUSE_NEW_CALL,
                                   remoteIsCallee,
                                   remoteAddress);

            int metaEventId = 0;
            int metaEventType = PtEvent::META_EVENT_NONE;
            int numCalls = 0;
            const UtlString* metaEventCallIds = NULL;
            getMetaEvent(metaEventId, metaEventType,
                         numCalls, &metaEventCallIds);
            if(metaEventType != PtEvent::META_CALL_TRANSFERRING
               && metaEventType != PtEvent::META_CALL_REPLACING)
            {
#ifdef TEST_PRINT
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                              "CpPeerCall::inFocus "
                              "stopMetaEvent 9");
#endif
                stopMetaEvent();
            }
        }
    }
    else
    {
        UtlDListIterator iterator(mConnections);
        while ((connection = (Connection*) iterator()))
        {
            int cause = 0;
            int state = connection->getState(cause);
            if (state != Connection::CONNECTION_ALERTING || mLocalTermConnectionState == PtTerminalConnection::HELD)
            {
                UtlString responseText;
                connection->getResponseText(responseText);
                UtlString connCallId;
                connection->getCallId(&connCallId);
#ifdef TEST_PRINT
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                              "CpPeerCall::inFocus "
                              "post %s TALKING/UNHOLD",
                              remoteAddress.data());
#endif
                postTaoListenerMessage(connection->getResponseCode(),
                                       responseText, PtEvent::TERMINAL_CONNECTION_TALKING,
                                       TERMINAL_CONNECTION_STATE,
                                       PtEvent::CAUSE_UNHOLD,
                                       remoteIsCallee,
                                       remoteAddress,
                                       0,
                                       connCallId);
            }
        }
    }

    UtlDListIterator iterator(mConnections);
    while ((connection = (Connection*) iterator()))
    {
        int cause = 0;
        int state = connection->getState(cause);
        if (state != Connection::CONNECTION_ALERTING || mLocalTermConnectionState == PtTerminalConnection::HELD)
        {
            connection->fireSipXEvent(CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE) ;
        }
    }
    CpCall::inFocus();
}

void CpPeerCall::outOfFocus()
{
    CpCall::outOfFocus();

    OsReadLock lock(mConnectionMutex);

    UtlDListIterator iterator(mConnections);
    Connection* connection = NULL;

    while ((connection = (Connection*)iterator()))
    {
        if(connection && (connection->remoteRequestedHold() || mLocalHeld))
        {
            int remoteIsCallee = 1;
            UtlString responseText;
            UtlString remoteAddress;
            UtlString connectionCallId;
            connection->getCallId(&connectionCallId);
            remoteIsCallee = connection->isRemoteCallee();
    #ifdef TEST_PRINT
            osPrintf("%s-Call %s out of focus\n",
                mName.data(), connectionCallId.data());
    #endif
            connection->getRemoteAddress(&remoteAddress);
            connection->getResponseText(responseText);


            // Notify listeners that the local connection is out of focus
            postTaoListenerMessage(connection->getResponseCode(),
                                   responseText,
                                   PtEvent::TERMINAL_CONNECTION_HELD,
                                   TERMINAL_CONNECTION_STATE,
                                   PtEvent::CAUSE_NORMAL,
                                   remoteIsCallee,
                                   remoteAddress,
                                   0,
                                   connectionCallId);
        }

        if (connection->isHeld())
        {
            connection->fireSipXEvent(CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_INACTIVE) ;
        }
        else
        {
            connection->fireSipXEvent(CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD) ;
        }
    }
}

void CpPeerCall::onHook()
{
#ifdef TEST_PRINT
    osPrintf("%s-CpPeerCall: hanging up\n", mName.data());
#endif

    UtlString thisCallId;
    getCallId(thisCallId);

    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpPeerCall::onHook "
                  "hanging up this call %s ...",
                  thisCallId.data());

    Connection* connection = NULL;

    // Take this call out of focus right away
    CpIntMessage localHoldMessage(CallManager::CP_YIELD_FOCUS, (intptr_t) this);
    mpManager->postMessage(localHoldMessage);
    {
        OsReadLock lock(mConnectionMutex);
        UtlDListIterator iterator(mConnections);

        while ((connection = (Connection*) iterator()))
        {
            connection->hangUp();
            //connection->setMediaInterface(NULL) ;

            // do not fire the tapi event if it is a ghost connection
            CpGhostConnection* pGhost = NULL;
            pGhost = dynamic_cast<CpGhostConnection*>(connection);
            if (!pGhost)
            {
                connection->fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL) ;
            }
        }
    }

    dropIfDead();
}

void CpPeerCall::hangUp(const char* callId,
                        const char* toTag,
                        const char* fromTag)
{
    Connection* connection = findHandlingConnection(callId,
        toTag,
        fromTag,
        FALSE);
    if(connection)
    {
        connection->hangUp();
    }
}


// Get the connection state for the connection identified by the designated callId,
// toTag, and fromTag.  Returns TRUE if the connection is found, else returns FALSE.
UtlBoolean CpPeerCall::getConnectionState(const char* callId,
                                          const char* toTag,
                                          const char* fromTag,
                                          int&        state,
                                          UtlBoolean   strictCompare)
{
    UtlBoolean bRC = FALSE ;

    Connection* connection = findHandlingConnection(callId, toTag, fromTag, strictCompare);
    if(connection)
    {
        state = connection->getState() ;
        bRC = TRUE ;
    }

    return bRC ;
}


void CpPeerCall::dropIfDead()
{
    UtlString thisCallId;
    getCallId(thisCallId);

#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpPeerCall::dropIfDead "
                  "callId: %s mDropping: %d\n",
                  thisCallId.data(), mDropping);
#endif

    int localConnectionState;

    // If all the connections are dead, drop the call
    if (mDropping && !isConnectionLive(&localConnectionState))
    {
        if (mbRequestedDrop)
        {
#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG,
                          "CpPeerCall::dropIfDead "
                          "called multiple times for call %10p",
                          this) ;
#endif
            return ;
        }
        else
        {
            mbRequestedDrop = true ;
        }

        setCallState(0, "", PtCall::INVALID);
        mDtmfEnabled = FALSE;

        // Signal the manager to Shutdown the task
        // Do this at the very last opportunity
#ifdef TEST_PRINT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CpPeerCall::dropIfDead "
                      "callId: %s Posting call exit: %X",
                      thisCallId.data(), (void*) this);
#endif
        {
            OsReadLock lock(mConnectionMutex);

            if (mConnections.entries())
            {
                    // Notify listeners that call is going to be torn down
                    UtlDListIterator iterator(mConnections);
                    Connection* connection = NULL;
                    while ((connection = (Connection*) iterator()))
                    {
                        // do not fire the tapi event if it is a ghost connection
                        CpGhostConnection* pGhost = NULL;
                        pGhost = dynamic_cast<CpGhostConnection*>(connection);
                        if (!pGhost)
                        {
                            connection->fireSipXEvent(CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL) ;
                        }
                    }

                    // Drop the call immediately
                    if (mpManager->getDelayInDeleteCall() == 0)
                    {
                        CpIntMessage ExitMsg(CallManager::CP_CALL_EXITED, (intptr_t)this) ;
                        mpManager->postMessage(ExitMsg) ;
                        OsSysLog::add(FAC_CP, PRI_DEBUG,
                                      "CpPeerCall::dropIfDead "
                                      "%s Posting call exit: %p",
                                      thisCallId.data(), this);
                    }
                    else
                    {
                        // For media server we need to hold off the deletion for a while
                        CpIntMessage * pExitMsg = new CpIntMessage(CallManager::CP_CALL_EXITED,(intptr_t)this);
                        OsTimer * timer = new OsTimer(mpManager->getMessageQueue(), (void*)pExitMsg);
                        OsTime timerTime(mpManager->getDelayInDeleteCall(), 0);
                        timer->oneshotAfter(timerTime);
                        UtlString thisCallId;
                        getCallId(thisCallId);
                        OsSysLog::add(FAC_CP, PRI_DEBUG,
                                      "CpPeerCall::dropIfDead "
                                      "Wait for %d secs to signal the exit for call %s ...",
                                     mpManager->getDelayInDeleteCall(), thisCallId.data());
                        OsSysLog::add(FAC_CP, PRI_DEBUG,
                                      "CpPeerCall::dropIfDead "
                                      "creating CpIntMessage %p timer %p",
                                     pExitMsg, timer);
                    }
            }
            else
            {
                // Drop the call immediately
                CpIntMessage ExitMsg(CallManager::CP_CALL_EXITED, (intptr_t)this) ;
                mpManager->postMessage(ExitMsg) ;
            }
        }      // end connection mutex lock brackets
    }
    else
    {
        dropDeadConnections();
    }
}

void CpPeerCall::dropDeadConnections()
{
    OsWriteLock lock(mConnectionMutex);
    Connection* connection = NULL;
    int         connectionState;
    OsTime      now ;

#ifdef TEST_PRINT
    UtlString thisCallId;
    getCallId(thisCallId);
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpPeerCall::dropDeadConnections "
                  "callId: %s: %X",
                  thisCallId.data(), (void*) this);
#endif
    OsDateTime::getCurTimeSinceBoot(now) ;
    UtlDListIterator iterator(mConnections);
    while ((connection = (Connection*) iterator()))
    {
        UtlString connCallId;
        connection->getCallId(&connCallId);

        // 1. Look for newly disconnected/failed connections and fire off events
        // and mark for deletion.

        int cause = 0;
        connectionState = connection->getState(0, cause);    // get remote connection state
#ifdef TEST_PRINT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CpPeerCall::dropDeadConnections "
                      "callId: %s: connection state %d",
                      thisCallId.data(), connectionState);
#endif
        if (!connection->isMarkedForDeletion() &&
            (connectionState ==  Connection::CONNECTION_DISCONNECTED ||
            connectionState == Connection::CONNECTION_FAILED))
        {
            UtlString addr;
            connection->getLocalAddress(&addr);
            int localState = connection->getState(1, cause);    // get local state
#ifdef TEST_PRINT
            UtlString stateStr;
            connection->getStateString(localState, &stateStr);
            OsSysLog::add(FAC_CP, PRI_DEBUG,
                          "CpPeerCall::dropDeadConnections "
                          "callId: %s: localState %s address %s, local addr %s",
                          thisCallId.data(), stateStr.data(), addr.data(), mLocalAddress.data());
#endif
            if (mLocalAddress == addr)
            {
                if (localState ==  Connection::CONNECTION_DISCONNECTED)
                {
                    UtlString responseText;
                    connection->getResponseText(responseText);
                    postTaoListenerMessage(connection->getResponseCode(),
                                           responseText,
                                           PtEvent::CONNECTION_DISCONNECTED,
                                           CONNECTION_STATE,
                                           PtEvent::CAUSE_NORMAL, // cause
                                           1,                     // remoteIsCallee
                                           "",                    // remoteAddress
                                           0,                     // isRemote
                                           connCallId);


                    // do not fire the tapi event if it is a ghost connection
                    CpGhostConnection* pGhost = NULL;
                    pGhost = dynamic_cast<CpGhostConnection*>(connection);
                    if (!pGhost)
                    {
                        connection->fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL) ;
                    }
                    else
                    {
                        pGhost->disconnectForSipXTapi();
                    }

                    postTaoListenerMessage(connection->getResponseCode(),
                                           responseText,
                                           PtEvent::TERMINAL_CONNECTION_DROPPED,
                                           TERMINAL_CONNECTION_STATE,
                                           PtEvent::CAUSE_NORMAL, // cause
                                           1,                     // remoteIsCallee
                                           "",                    // remoteAddress
                                           0,                     // isRemote
                                           connCallId);

                }
                else if (localState ==  Connection::CONNECTION_FAILED)
                {
                    UtlString responseText;
                    connection->getResponseText(responseText);
                    postTaoListenerMessage(connection->getResponseCode(),
                                           responseText,
                                           PtEvent::CONNECTION_FAILED,
                                           CONNECTION_STATE,
                                           PtEvent::CAUSE_NORMAL, // cause
                                           1,                     // remoteIsCallee
                                           "",                    // remoteAddress
                                           0,                     // isRemote
                                           connCallId);

                    postTaoListenerMessage(connection->getResponseCode(),
                                           responseText,
                                           PtEvent::TERMINAL_CONNECTION_DROPPED,
                                           TERMINAL_CONNECTION_STATE,
                                           PtEvent::CAUSE_NORMAL, // cause
                                           1,                     // remoteIsCallee
                                           "",                    // remoteAddress
                                           0,                     // isRemote
                                           connCallId);


                    CpGhostConnection* pGhost = NULL;
                    pGhost = dynamic_cast<CpGhostConnection*>(connection);

                    // do not fire the tapi event if it is a ghost connection
                    if (!pGhost)
                    {
                        connection->fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL) ;
                    }
                    else
                    {
                        pGhost->disconnectForSipXTapi();
                    }
                }

                // Mark the connection for deletion
                connection->markForDeletion() ;
            }
        }

        // 2. Look for connections which could be removed/deleted
        if (connection->isMarkedForDeletion())
        {
            OsTime deleteAfter ;
            connection->getDeleteAfter(deleteAfter) ;
            if (now > deleteAfter)
            {
#ifdef TEST_PRINT
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                              "CpPeerCall::dropDeadConnections "
                              "callId: %s: delete marked connection %p",
                              thisCallId.data(), connection);
#endif
                mConnections.destroy(connection);
            }
        }
    }
}


void CpPeerCall::offHook(const void* pDisplay)
{
#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpPeerCall::offHook\n");
#endif
    OsReadLock lock(mConnectionMutex);
    Connection* connection = NULL;

    UtlDListIterator iterator(mConnections);
    while ((connection = (Connection*) iterator()))
    {
        if(connection &&
            connection->getState() == Connection::CONNECTION_ALERTING)
        {
            connection->answer(pDisplay);
            mLocalConnectionState = PtEvent::CONNECTION_ESTABLISHED;
        }
    }

    mDtmfEnabled = TRUE;
}



/* ============================ ACCESSORS ================================= */
UtlBoolean CpPeerCall::getConnectionState(const char* remoteAddress, int& state)
{
    OsReadLock lock(mConnectionMutex);
    Connection* pConnection = findHandlingConnection((UtlString&) remoteAddress);

    if (pConnection)
    {
        int connState = pConnection->getRemoteState();
        switch(connState)
        {
        case Connection::CONNECTION_IDLE:
            state = PtConnection::IDLE;
            break;

        case Connection::CONNECTION_QUEUED:
            state = PtConnection::QUEUED;
            break;

        case Connection::CONNECTION_OFFERING:
            state = PtConnection::OFFERED;
            break;

        case Connection::CONNECTION_DIALING:
            state = PtConnection::DIALING;
            break;

        case Connection::CONNECTION_ALERTING:
            state = PtConnection::ALERTING;
            break;

        case Connection::CONNECTION_ESTABLISHED:
            state = PtConnection::ESTABLISHED;
            break;

        case Connection::CONNECTION_FAILED:
            state = PtConnection::FAILED;
            break;

        case Connection::CONNECTION_DISCONNECTED:
            state = PtConnection::DISCONNECTED;
            break;

        default:
            state = PtConnection::UNKNOWN;
            break;

        }
        return TRUE;
    }
    else if (!strcmp(mLocalAddress, remoteAddress))
    {
        switch(mLocalConnectionState)
        {
        case PtEvent::CONNECTION_IDLE:
        case PtEvent::CONNECTION_INITIATED:
        case PtEvent::CONNECTION_CREATED:
            state = PtConnection::IDLE;
            break;

        case PtEvent::CONNECTION_QUEUED:
            state = PtConnection::QUEUED;
            break;

        case PtEvent::CONNECTION_OFFERED:
            state = PtConnection::OFFERED;
            break;

        case PtEvent::CONNECTION_DIALING:
            state = PtConnection::DIALING;
            break;

        case PtEvent::CONNECTION_ALERTING:
            state = PtConnection::ALERTING;
            break;

        case PtEvent::CONNECTION_ESTABLISHED:
            state = PtConnection::ESTABLISHED;
            break;

        case PtEvent::CONNECTION_FAILED:
            state = PtConnection::FAILED;
            break;

        case PtEvent::CONNECTION_DISCONNECTED:
            state = PtConnection::DISCONNECTED;
            break;

        default:
            state = PtConnection::UNKNOWN;
            break;

        }
        return TRUE;
    }
    else
        return FALSE;
}

UtlBoolean CpPeerCall::getTermConnectionState(const char* address,
                                              const char* terminal,
                                              int& state)
{
    UtlBoolean rc = TRUE;
    state = PtTerminalConnection::UNKNOWN;

    OsReadLock lock(mConnectionMutex);
    Connection* pConnection = findHandlingConnection((UtlString&) address);

    if (pConnection)
    {
        state = pConnection->getTerminalState(0);
    }
    else if (!strcmp(mLocalAddress, address))
    {
        state = mLocalTermConnectionState;
    }
    else
    {
        rc = FALSE;
    }

    return rc;
}

void CpPeerCall::printCall(int showHistory)
{
    Connection* connection = NULL;
    OsReadLock lock(mConnectionMutex);
    UtlDListIterator iterator(mConnections);
    UtlString connectionAddress;
    UtlString connectionState;
    UtlString connectionCallId;
    int connectionIndex = 0;
    int cause = 0;

    CpCall::printCall(showHistory);

    while ((connection = (Connection*) iterator()))
    {
        connection->getRemoteAddress(&connectionAddress);
        Connection::getStateString(connection->getState(cause),
            &connectionState);
        connection->getCallId(&connectionCallId);
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CpPeerCall::printCall "
                      "%s-\tconnection[%d](%p): %s callId: %s\n\t\tstate: %s cause: %d\n",
            mName.data(), connectionIndex, connection, connectionAddress.data(),
            connectionCallId.data(), connectionState.data(), cause);
        connectionIndex++;
    }
}

void CpPeerCall::getLocalAddress(char* address, int maxLen)
{
    int len = mLocalAddress.length();

    len = (maxLen <= len) ? (maxLen - 1) : len;

    if (!mLocalAddress.isNull())
    {
        strncpy(address, mLocalAddress.data(), len);
    }
    address[len] = 0;
#ifdef TEST_PRINT
    osPrintf("%s-CpPeerCall::getLocalAddress %s\n",
        mName.data(), address);
#endif
}

void CpPeerCall::getLocalTerminalId(char* terminal, int maxLen)
{
    int len = mLocalTerminalId.length();

    len = (maxLen <= len) ? (maxLen - 1) : len;

    if (!mLocalTerminalId.isNull())
    {
        strncpy(terminal, mLocalTerminalId.data(), len);
    }
    terminal[len] = 0;
}

/* ============================ INQUIRY =================================== */

UtlBoolean CpPeerCall::shouldCreateCall(SipUserAgent& sipUa, OsMsg& eventMessage,
                                        SdpCodecFactory& codecFactory)
{
    UtlBoolean createCall = FALSE;
    int msgType = eventMessage.getMsgType();
    int msgSubType = eventMessage.getMsgSubType();

    if(msgType == OsMsg::PHONE_APP &&
        msgSubType == CallManager::CP_SIP_MESSAGE)
    {
        createCall = SipConnection::shouldCreateConnection(sipUa, eventMessage,
            &codecFactory);
    }
    // This should work with the connection Factory to decide which
    // type of connection and if the connection should be created.
    return(createCall);
}

CpCall::handleWillingness CpPeerCall::willHandleMessage(const OsMsg& eventMessage)
{
    CpCall::handleWillingness takeTheMessage = CP_WILL_NOT_HANDLE;

    if(eventMessage.getMsgType() == OsMsg::PHONE_APP
       && eventMessage.getMsgSubType() == CallManager::CP_SIP_MESSAGE)
    {
        const SipMessage* sipMsg = ((SipMessageEvent&)eventMessage).getMessage();

        if(sipMsg)
        {
            int seqNum;
            UtlString seqMethod;
            sipMsg->getCSeqField(&seqNum,&seqMethod);
            UtlString strToField;
            sipMsg->getToField(&strToField);

            // DWW 08/18/03 If we are dropping and a invite comes in, but
            // has a to field, we should let the callmanager create a new call.
            // Otherwise, if a invite comes in, and does not have a to field,
            // the we SHOULD handle it.

            if (mDropping
                && seqMethod == "INVITE"
                && strToField.length())
            {
                ;
            }
            else
            {
                UtlString callId;
                sipMsg->getCallIdField(&callId);

#ifdef TEST_PRINT
                osPrintf("%s-Message mCallId %s callid: %s\n",
                    mName.data(), mCallId.data(), callId.data());
#endif

                UtlBoolean thisCallHasCallId = hasCallId(callId.data());
                if(thisCallHasCallId)
                {
                    takeTheMessage = CP_DEFINITELY_WILL_HANDLE;
                }

                // Check if this is an INVITE with a Replaces header
                if(takeTheMessage == CP_WILL_NOT_HANDLE
                   && !sipMsg->isResponse())
                {
                    UtlString method;
                    sipMsg->getRequestMethod(&method);

                    if(method.compareTo(SIP_INVITE_METHOD) == 0)
                    {
                        // If there is a Replaces header check for a
                        // match of the Call-Id in the Replaces header
                        UtlString toTag;
                        UtlString fromTag;
                        if (sipMsg->getReplacesData(callId, toTag, fromTag))
                        {
                            UtlBoolean replacesMatchesThisCallId =
                                hasCallId(callId.data());
                            if(replacesMatchesThisCallId)
                            {
                                takeTheMessage = CP_MAY_HANDLE;
                            }
                        }
                    }
                }
            }
        }
    }

    return(takeTheMessage);
}

UtlBoolean CpPeerCall::isQueued()
{

    return(findQueuedConnection() != NULL);
}

UtlBoolean CpPeerCall::isConnectionLive(int* localConnectionState)
{
    UtlBoolean liveConnections = FALSE;
    OsReadLock lock(mConnectionMutex);
    UtlDListIterator iterator(mConnections);
    Connection* connection = NULL;
    UtlBoolean connectionState;
    if(localConnectionState)
    {
        *localConnectionState = getLocalConnectionState(mLocalConnectionState);
    }


    while ((connection = (Connection*) iterator()))
    {
        int cause;
        connectionState = connection->getState(0, cause);    // get remote connection state

        if(localConnectionState &&
            *localConnectionState != Connection::CONNECTION_ESTABLISHED &&
            connectionState == Connection::CONNECTION_DISCONNECTED)
        {
            *localConnectionState = connectionState;
        }

        if(connectionState != Connection::CONNECTION_DISCONNECTED &&
            connectionState != Connection::CONNECTION_FAILED  &&
            connectionState != Connection::CONNECTION_UNKNOWN)
            // Atleast sometimes we do not want to kill the call
            // if there are IDLE connections
            //connectionState != Connection::CONNECTION_IDLE)
        {
#ifdef TEST_PRINT
            UtlString remoteAddr;
            connection->getRemoteAddress(&remoteAddr);
            OsSysLog::add(FAC_CP, PRI_DEBUG,
                "%s-Connection %s state: %d not dead\n",
                mName.data(), remoteAddr.data(), connectionState);
#endif
            liveConnections = TRUE;
            if(localConnectionState)
                *localConnectionState = Connection::CONNECTION_ESTABLISHED;
            break;
        }
    }

#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG,
        "%s-CpPeerCall::isConnectionLive: %d\n",
        mName.data(), liveConnections);
#endif

    /*    if (*localConnectionState != Connection::CONNECTION_DISCONNECTED &&
    *localConnectionState != Connection::CONNECTION_FAILED &&
    *localConnectionState != Connection::CONNECTION_UNKNOWN)
    {
    liveConnections = 1;    // connection may be in hold state, as in consultative transfer (trasferee)
    }
    */
    return(liveConnections);
}

UtlBoolean CpPeerCall::isConnection(const char* callId,
                                    const char* toTag,
                                    const char* fromTag)
{
    return(NULL != findHandlingConnection(callId,
        toTag,
        fromTag,
        FALSE));
}

UtlBoolean CpPeerCall::isLocalTerminal(const char* terminalId)
{
    UtlBoolean isLocal = FALSE;
    if(terminalId)
    {
        UtlString term = UtlString(terminalId);

        ssize_t pos = term.index("foreign-terminal");
        if (pos < 0)
        {
            UtlString address;
            UtlString user;
            UtlString protocol;
            int port;

            SipMessage::parseAddressFromUri(terminalId, &address, &port, &protocol, &user) ;

#ifdef TEST_PRINT
            osPrintf("%s-CpPeerCall::isLocalTerminal terminalId: %s mLocalTerminalId: %s\n",
                mName.data(), terminalId, mLocalTerminalId.data());
#endif

            isLocal = ((mLocalTerminalId.compareTo(terminalId) == 0) || (mLocalTerminalId.compareTo(user) == 0)) ;

        }
    }
    return(isLocal);
}

UtlBoolean CpPeerCall::canDisconnectConnection(Connection* pConnection)
{
    UtlBoolean ret;
    Connection* connection = NULL;
    OsReadLock lock(mConnectionMutex);
    UtlDListIterator iterator(mConnections);
    int cnt = 0;
    int contains = 0;

    while ((connection = (Connection*) iterator()))
    {
        cnt++;
        if (connection == pConnection) contains = 1;
    }

    ret = ((cnt >= 1) &&
        contains  &&
        (!mLocalHeld ||
        mLocalTermConnectionState != PtTerminalConnection::HELD));

    return ret;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
Connection* CpPeerCall::findHandlingConnection(OsMsg& eventMessage)
{
    Connection* connection = NULL;
    OsReadLock lock(mConnectionMutex);
    UtlDListIterator iterator(mConnections);

    if (mConnections.entries())
    {
        while ((connection = (Connection*) iterator()))
        {
            if(connection->willHandleMessage(eventMessage)) break;
            connection = NULL;
        }
    }

    return(connection);
}

Connection* CpPeerCall::findHandlingConnection(const char* callId,
                                               const char* toTag,
                                               const char* fromTag,
                                               UtlBoolean  strictCompare)
{
    Connection* connection = NULL;
    OsReadLock lock(mConnectionMutex);
    UtlDListIterator iterator(mConnections);

    while ((connection = (Connection*) iterator()))
    {
        if(connection->isConnection(callId,
            toTag,
            fromTag,
            strictCompare)) break;
        connection = NULL;
    }
    return(connection);
}

Connection* CpPeerCall::findHandlingConnection(UtlString& remoteAddress)
{
    OsReadLock lock(mConnectionMutex);
    Connection* connection = NULL;
    UtlDListIterator iterator(mConnections);

    while ((connection = (Connection*) iterator()))
    {
        UtlString connectionRemoteAddress;
        Url remoteUrl(remoteAddress);

        connection->getRemoteAddress(&connectionRemoteAddress);
        if (!connectionRemoteAddress.isNull())
        {
            Url connectionAddressUrl(connectionRemoteAddress);

            // This allows remoteUrl to match if it does not have a tag
            if(SipMessage::isSameSession(remoteUrl, connectionAddressUrl)) break;

            // This allows connectionAddressUrl to match if it does not have a tag
            if(SipMessage::isSameSession(connectionAddressUrl, remoteUrl)) break;

            connection = NULL;
        }
    }
    return(connection);
}

void CpPeerCall::addConnection(Connection* connection)
{
    connection->setLocalAddress(mLocalAddress.data());
    OsWriteLock lock(mConnectionMutex);

    mConnections.append(connection);

    addTaoListenerToConnection(connection);
}

void CpPeerCall::addToneListenersToConnection(Connection* connection)
{
    OsReadLock lock(mConnectionMutex);

    // Add tone listeners to each/every new connection
    UtlDListIterator iterator(mToneListeners);
    void* pListener ;
    while ((pListener = (void*) iterator()))
    {
        addToneListenerToFlowGraph((intptr_t) pListener, connection);
    }
}

void CpPeerCall::addTaoListenerToConnection(Connection* connection)
{
    for (int i = 0; i < mListenerCnt; i++)
    {
        connection->addTaoListener((OsServerTask*) (mpListeners[i]->mpListenerPtr));
    }
}

Connection* CpPeerCall::findQueuedConnection()
{
    Connection* connection = NULL;
    OsReadLock lock(mConnectionMutex);
    UtlDListIterator iterator(mConnections);

    while ((connection = (Connection*) iterator()))
    {
        if(connection->getState() == Connection::CONNECTION_QUEUED) break;
        connection = NULL;
    }
    return(connection);
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */
UtlBoolean CpPeerCall::checkForTag(const UtlString &address,
                                   UtlString& address_without_tag)
{
   // If not a sip: URL, there is nothing complex to be done.
   if (address.compareTo("sip:") == 0)
   {
      return FALSE;
   }

   // Parse into a URI and look for a "tag" URI parameter.
   UtlString tag;
   Url url(address);
   url.getFieldParameter("tag", tag);

   if (tag.isNull())
   {
      return FALSE;
   }
   else
   {
      // If the tag was seen, construct the form without the tag.
      url.removeFieldParameter("tag");
      url.toString(address_without_tag);
      return TRUE;
   }
}

/* ============================ FUNCTIONS ================================= */

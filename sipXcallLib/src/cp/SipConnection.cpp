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

#ifdef __pingtel_on_posix__
#include <stdlib.h>
#endif

// APPLICATION INCLUDES
#include <os/OsQueuedEvent.h>
#include <os/OsTimer.h>
#include <os/OsUtil.h>
#include <net/SipMessageEvent.h>
#include <net/SipUserAgent.h>
#include <net/NameValueTokenizer.h>
#include <net/SdpCodecFactory.h>
#include <net/Url.h>
#include <net/SipSession.h>
#include <net/NetBase64Codec.h>
#include <cp/SipConnection.h>
#include <mi/CpMediaInterface.h>
#include <cp/CallManager.h>
#include <cp/CpCallManager.h>
#include <cp/CpPeerCall.h>
#include <cp/CpMultiStringMessage.h>
#include <cp/CpIntMessage.h>
#include "ptapi/PtCall.h"
#include <net/TapiMgr.h>
#include <net/SipLineMgr.h>

//#define TEST_PRINT 1

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define CALL_STATUS_FIELD "status"

#ifdef _WIN32
#   define CALL_CONTROL_TONES
#endif

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipConnection::SipConnection(const char* outboundLineAddress,
                             UtlBoolean isEarlyMediaFor180Enabled,
                             CpCallManager* callMgr,
                             CpCall* call,
                             CpMediaInterface* mediaInterface,
                             //UiContext* callUiContext,
                             SipUserAgent* sipUA,
                             int offeringDelayMilliSeconds,
                             int sessionReinviteTimer,
                             int availableBehavior,
                             const char* forwardUnconditionalUrl,
                             int busyBehavior,
                             const char* forwardOnBusyUrl)
                             : Connection(callMgr, call, mediaInterface, offeringDelayMilliSeconds,
                             availableBehavior, forwardUnconditionalUrl,
                             busyBehavior, forwardOnBusyUrl)
                             , inviteFromThisSide(0)
                             , mIsEarlyMediaFor180(TRUE)
                             , mContactId(0)
{
    sipUserAgent = sipUA;
    mInviteMsg = NULL;
    mReferMessage = NULL;
    lastLocalSequenceNumber = 0;
    lastRemoteSequenceNumber = -1;
    reinviteState = ACCEPT_INVITE;
    mIsEarlyMediaFor180 = isEarlyMediaFor180Enabled;
    mDropping = FALSE ;
    mWaitingForKeepAliveResponse = FALSE;
    mPrevSdp = NULL;

    // Build a from tag
    int fromTagInt = rand();
    char fromTagBuffer[60];
    sprintf(fromTagBuffer, "%dc%d", call->getCallIndex(), fromTagInt);
    mFromTag = fromTagBuffer;

    if(outboundLineAddress)
    {
        mFromUrl = outboundLineAddress;

        // Before adding the from tag, construct the local contact with the
        // device's NAT friendly contact information (getContactUri).  The host
        // and port from that replaces the public address or record host and
        // port.  The UserId and URL parameters should be retained.
        UtlString contactHostPort;
        UtlString address;
        Url tempUrl(mFromUrl);
        sipUserAgent->getContactUri(&contactHostPort);
        Url hostPort(contactHostPort);
        hostPort.getHostAddress(address);
        tempUrl.setHostAddress(address);
        tempUrl.setHostPort(hostPort.getHostPort());
        tempUrl.toString(mLocalContact);

        // Set the from tag in case this is an outbound call
        // If this is an in bound call, the from URL will get
        // over written by the To field from the SIP request
        mFromUrl.setFieldParameter("tag", mFromTag);
    }

    mDefaultSessionReinviteTimer = sessionReinviteTimer;
    mSessionReinviteTimer = 0;

#ifdef TEST_PRINT
    osPrintf("SipConnection::mDefaultSessionReinviteTimer = %d\n",
        mDefaultSessionReinviteTimer);
#endif

    mIsReferSent = FALSE;
    mIsAcceptSent = FALSE;

    mbCancelling = FALSE;        // this is the flag that indicates CANCEL is sent
    // but no response has been received  if set to TRUE

    // State variable which indicates an action to
    // perform after hold has completed.
    mHoldCompleteAction = CpCallManager::CP_UNSPECIFIED;
}

// Copy constructor
SipConnection::SipConnection(const SipConnection& rSipConnection)
{
}

// Destructor
SipConnection::~SipConnection()
{
    UtlString callId;
#ifdef TEST_PRINT
    if (mpCall) {
        mpCall->getCallId(callId);
        OsSysLog::add(FAC_CP, PRI_DEBUG, "Entering SipConnection destructor: %s\n", callId.data());
    } else
        OsSysLog::add(FAC_CP, PRI_DEBUG, "Entering SipConnection destructor: call is Null\n");
#endif

    if(mInviteMsg)
    {
        delete mInviteMsg;
        mInviteMsg = NULL;
    }
    if(mReferMessage)
    {
        delete mReferMessage;
        mReferMessage = NULL;
    }
    if(mPrevSdp)
    {
       delete mPrevSdp;
       mPrevSdp=NULL;
    }
    mProvisionalToTags.destroyAll();
#ifdef TEST_PRINT
    if (!callId.isNull())
        OsSysLog::add(FAC_CP, PRI_DEBUG, "Leaving SipConnection destructor: %s\n", callId.data());
    else
        OsSysLog::add(FAC_CP, PRI_DEBUG, "Leaving SipConnection destructor: call is Null\n");
#endif
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipConnection&
SipConnection::operator=(const SipConnection& rhs)
{
    if (this == &rhs)            // handle the assignment to self case
        return *this;

    return *this;
}


UtlBoolean SipConnection::dequeue(UtlBoolean callInFocus)
{
    UtlBoolean connectionDequeued = FALSE;
#ifdef TEST_PRINT
    osPrintf("Connection::dequeue this: %p mInviteMsg: %p\n", this, mInviteMsg);
#endif
    if(getState() == CONNECTION_QUEUED)
    {
        int tagNum = -1;
        proceedToRinging(mInviteMsg, sipUserAgent, tagNum, mLineAvailableBehavior);

        setState(CONNECTION_ALERTING, CONNECTION_LOCAL);
        /** SIPXTAPI: TBD **/

        connectionDequeued = TRUE;
    }

    return(connectionDequeued);
}

UtlBoolean SipConnection::requestShouldCreateConnection(const SipMessage* sipMsg,
                                                        SipUserAgent& sipUa,
                                                        SdpCodecFactory* codecFactory)
{
    UtlBoolean createConnection = FALSE;
    UtlString method;
    sipMsg->getRequestMethod(&method);
    UtlString toField;
    UtlString address;
    UtlString protocol;
    int port;
    UtlString user;
    UtlString userLabel;
    UtlString tag;
    sipMsg->getToAddress(&address, &port, &protocol, &user, &userLabel, &tag);

    // Dangling or deleted ACK
    if(method.compareTo(SIP_ACK_METHOD) == 0)
    {
        // Ignore it and do not create a connection
        createConnection = FALSE;
    }

    // INVITE to create a connection
    //if to tag is already set then return 481 error
    else if(method.compareTo(SIP_INVITE_METHOD) == 0 && tag.isNull())
    {
        // Assume the best case, as this will be checked
        // again before the call is answered
        UtlBoolean atLeastOneCodecSupported = TRUE;
        if(codecFactory == NULL ||
            codecFactory->getCodecCount() == 0)
        {
            atLeastOneCodecSupported = TRUE;
        }

        // Verify that we have some RTP codecs in common
        else
        {
            // Get the SDP and find out if there are any
            // codecs in common
            UtlString rtpAddress;
            int rtpPort;
            int rtcpPort;
            int videoRtpPort;
            int videoRtcpPort;
            const SdpBody* bodyPtr = sipMsg->getSdpBody();
            if(bodyPtr)
            {
                int numMatchingCodecs = 0;
                SdpCodec** matchingCodecs = NULL;
                bodyPtr->getBestAudioCodecs(*codecFactory,
                    numMatchingCodecs,
                    matchingCodecs,
                    rtpAddress, rtpPort, rtcpPort,
                    videoRtpPort, videoRtcpPort);
                if(numMatchingCodecs > 0)
                {
                    // Need to cleanup the codecs
                    for(int codecIndex = 0; codecIndex < numMatchingCodecs; codecIndex++)
                    {
                        delete matchingCodecs[codecIndex];
                        matchingCodecs[codecIndex] = NULL;
                    }
                    delete[] matchingCodecs;
                    atLeastOneCodecSupported = TRUE;
                }
                else
                {
                    atLeastOneCodecSupported = FALSE;

                    // Send back a bad media error
                    // There are no codecs in common
                    SipMessage badMediaResponse;
                    badMediaResponse.setInviteBadCodecs(sipMsg, &sipUa);
                    sipUa.send(badMediaResponse);
                }

                delete bodyPtr;
            }

            // Assume that SDP will be sent in ACK
            else
                atLeastOneCodecSupported = TRUE;
        }

        if(atLeastOneCodecSupported)
        {
            // Create a new connection
            createConnection = TRUE;
        }
        else
        {
            createConnection = FALSE;
#ifdef TEST_PRINT
            osPrintf("SipConnection::requestShouldCreateConnection FALSE INVITE with no supported RTP codecs\n");
#endif
        }
    }

    // NOTIFY for REFER
    // a non-existing transaction.
    else if(method.compareTo(SIP_NOTIFY_METHOD) == 0)
    {
        UtlString eventType;
        sipMsg->getEventField(eventType);
        eventType.toLower();
        ssize_t typeIndex = eventType.index(SIP_EVENT_REFER);
        if(typeIndex >=0)
        {
            // Send a bad callId/transaction message
            SipMessage badTransactionMessage;
            badTransactionMessage.setBadTransactionData(sipMsg);
            sipUa.send(badTransactionMessage);
        }
        // All other NOTIFY events are ignored
        createConnection = FALSE;
    }

    else if(method.compareTo(SIP_REFER_METHOD) == 0)
    {
        createConnection = TRUE;
    }

    // All other methods: this is part of
    // a non-existing transaction.
    else
    {
        // Send a bad callId/transaction message
        SipMessage badTransactionMessage;
        badTransactionMessage.setBadTransactionData(sipMsg);
        sipUa.send(badTransactionMessage);
        createConnection = FALSE;
    }

    return createConnection;
}

// returns TRUE for new INVITE (some conditions) or REFER requests
UtlBoolean SipConnection::shouldCreateConnection(SipUserAgent& sipUa,
                                                 OsMsg& eventMessage,
                                                 SdpCodecFactory* codecFactory)
{
    UtlBoolean createConnection = FALSE;
    int msgType = eventMessage.getMsgType();
    int msgSubType = eventMessage.getMsgSubType();
    const SipMessage* sipMsg = NULL;
    int messageType;

    if(msgType == OsMsg::PHONE_APP &&
        msgSubType == CallManager::CP_SIP_MESSAGE)
    {
        sipMsg = ((SipMessageEvent&)eventMessage).getMessage();
        messageType = ((SipMessageEvent&)eventMessage).getMessageStatus();
#ifdef TEST_PRINT
        osPrintf("SipConnection::messageType: %d\n", messageType);
#endif

        switch(messageType)
        {
            // This is a request which failed to get sent
        case SipMessageEvent::TRANSPORT_ERROR:
        case SipMessageEvent::SESSION_REINVITE_TIMER:
        case SipMessageEvent::AUTHENTICATION_RETRY:
            // Ignore it and do not create a connection
            createConnection = FALSE;
            break;

        default:
            // Its a SIP Response
            if(sipMsg->isResponse())
            {
                // Ignore it and do not create a connection
                createConnection = FALSE;
            }
            // Its a SIP Request
            else
            {
                createConnection = SipConnection::requestShouldCreateConnection(sipMsg, sipUa, codecFactory);
            }
            break;
        }

        if(!createConnection)
        {
            UtlString msgBytes;
            ssize_t numBytes;
            sipMsg->getBytes(&msgBytes, &numBytes);
            msgBytes.insert(0, "SipConnection::shouldCreateConnection: FALSE\n");
#ifdef TEST_PRINT
            osPrintf("%s\n", msgBytes.data());
#endif
        }
#ifdef TEST_PRINT
        else
        {

            osPrintf("Create a SIP connection\n");
        }
#endif

    }

    return(createConnection);
}

// Select a compatible contact type given the request URL
ContactType SipConnection::selectCompatibleContactType(const SipMessage& request)
{
    ContactType contactType = mContactType ;
    char szAdapter[256];
    UtlString localAddress;
    getLocalAddress(&localAddress);

    getContactAdapterName(szAdapter, localAddress.data());

    UtlString requestUriHost ;
    int requestUriPort ;
    UtlString strUri ;
    request.getRequestUri(&strUri) ;
    Url requestUri(strUri) ;

    requestUri.getHostAddress(requestUriHost) ;
    requestUriPort = requestUri.getHostPort() ;
    if (!portIsValid(requestUriPort))
    {
        requestUriPort = 5060 ;
    }

    ContactAddress config_contact;
    ContactAddress stun_contact;
    ContactAddress local_contact;

    if (sipUserAgent->getContactDb().getRecordForAdapter(config_contact,
                                                         szAdapter,
                                                         ContactAddress::CONFIG) &&
        (strcmp(config_contact.cIpAddress, requestUriHost) == 0) &&
        (requestUriPort == (!portIsValid(config_contact.iPort) ? 5060 : config_contact.iPort)))
    {
        mContactId = config_contact.id;
        contactType = ContactAddress::CONFIG;
    }
    else if (sipUserAgent->getContactDb().getRecordForAdapter(stun_contact, szAdapter, ContactAddress::NAT_MAPPED) &&
        (strcmp(stun_contact.cIpAddress, requestUriHost) == 0) &&
        (requestUriPort == (!portIsValid(stun_contact.iPort) ? 5060 : stun_contact.iPort)))

    {
        mContactId = stun_contact.id;
        contactType = ContactAddress::NAT_MAPPED;
    }
    else if (sipUserAgent->getContactDb().getRecordForAdapter(local_contact, szAdapter, ContactAddress::LOCAL) &&
        (strcmp(local_contact.cIpAddress, requestUriHost) == 0) &&
        (requestUriPort == (!portIsValid(local_contact.iPort) ? 5060 : local_contact.iPort)))

    {
        mContactId = local_contact.id;
        contactType = ContactAddress::LOCAL;
    }

    return contactType ;
}


void SipConnection::updateContact(Url* pContactUrl, ContactType eType)
{
    UtlString useIp ;

    if ((mContactId == 0) && mInviteMsg)
    {
        ContactType cType;

        cType = selectCompatibleContactType(*mInviteMsg);
        mContactType = cType;
    }

    // get the Contact DB id that was set during
    // selectCompatibleContacts
    ContactAddress* pContact = sipUserAgent->getContactDb().find(mContactId);
    if (pContact == NULL)
    {
        if (eType == ContactAddress::AUTO ||
            eType == ContactAddress::NAT_MAPPED ||
            eType == ContactAddress::RELAY)
        {
            pContact =
               sipUserAgent->getContactDb().findByType(ContactAddress::NAT_MAPPED);
        }

        if (pContact == NULL)
        {
            pContact =
               sipUserAgent->getContactDb().findByType(ContactAddress::LOCAL);
        }
    }

    if (pContact)
    {
        pContactUrl->setHostAddress(pContact->cIpAddress);
        pContactUrl->setHostPort(pContact->iPort);
    }
}

void SipConnection::buildLocalContact(Url fromUrl,
                                      UtlString& localContact)
{
    Url preferredContact;
    UtlString requestURI ;

    if(!inviteFromThisSide)
    {
        requestURI = mRemoteUriStr ;
    }


    // Attempt to find/get the preferred contact for the line
    SipLine line ;
    SipLineMgr* pLineMgr = mpCallManager->getLineManager() ;
    if (pLineMgr
        && pLineMgr->getLine(fromUrl.toString(), localContact, requestURI, line)
        && line.getPreferredContactUri(preferredContact))
    {
        // OsSysLog::add(FAC_CP, PRI_DEBUG, "Found line definition: %s", preferredContact.toString().data()) ;
    }
    else
    {
        UtlString contactHostPort;
        UtlString address;
        sipUserAgent->getContactUri(&contactHostPort);
        preferredContact = contactHostPort ;

        Url hostPort(contactHostPort);
        hostPort.getHostAddress(address);
        int port = hostPort.getHostPort();

        UtlString displayName;
        UtlString userId;
        fromUrl.getDisplayName(displayName);
        fromUrl.getUserId(userId);

        preferredContact = Url(mLocalContact, FALSE);
        preferredContact.setUserId(userId.data());
        preferredContact.setDisplayName(displayName);
        preferredContact.setHostAddress(address);
        preferredContact.setHostPort(port);
        preferredContact.includeAngleBrackets();
    }

    updateContact(&preferredContact, mContactType) ;
    preferredContact.toString(localContact);
}

void SipConnection::buildLocalContact(UtlString& localContact)
{
    UtlString contactHostPort;
    UtlString address;

    sipUserAgent->getContactUri(&contactHostPort);
    Url hostPort(contactHostPort);
    hostPort.getHostAddress(address);
    int port = hostPort.getHostPort();

    Url contactUrl(mLocalContact, FALSE);
    contactUrl.setHostAddress(address);
    contactUrl.setHostPort(port);
    contactUrl.includeAngleBrackets();

    updateContact(&contactUrl, mContactType) ;
    contactUrl.toString(localContact);
}

UtlBoolean SipConnection::dial(const char* dialString,
                               const char* localLineAddress,
                               const char* callId,
                               const char* callController,
                               const char* originalCallConnection,
                               UtlBoolean requestQueuedCall,
                               const void* pDisplay,
                               const char* originalCallId,
						       const char* paiAddress)
{
    UtlBoolean dialOk = FALSE;
    SipMessage sipInvite;
    const char* callerDisplayName = NULL;
    int receiveRtpPort;
    int receiveRtcpPort;
    int receiveVideoRtpPort;
    int receiveVideoRtcpPort;
    SdpSrtpParameters srtpParams;
    UtlString rtpAddress;
    UtlString dummyFrom;
    UtlString fromAddress;
    UtlString goodToAddress;
    int cause = CONNECTION_CAUSE_NORMAL;

    if(getState() == CONNECTION_IDLE && mpMediaInterface != NULL)
    {
        UtlString localAddress ;
        ContactAddress* pAddress = sipUserAgent->getContactDb().getLocalContact(mContactId) ;
        if (pAddress != NULL)
        {
            localAddress = pAddress->cIpAddress ;
        }

        // Create a new connection in the media flowgraph
        mpMediaInterface->createConnection(mConnectionId, localAddress, (void*)pDisplay);
        mpMediaInterface->setContactType(mConnectionId, mContactType) ;
        SdpCodecFactory supportedCodecs;
        mpMediaInterface->getCapabilities(mConnectionId,
                                          rtpAddress,
                                          receiveRtpPort,
                                          receiveRtcpPort,
                                          receiveVideoRtpPort,
                                          receiveVideoRtcpPort,
                                          supportedCodecs,
                                          srtpParams);

        mRemoteIsCallee = TRUE;
        setCallId(callId);

        lastLocalSequenceNumber++;

        buildFromToAddresses(dialString, "xxxx", callerDisplayName,
                             dummyFrom, goodToAddress);

        // The local address is always set
        mFromUrl.toString(fromAddress);
#ifdef TEST_PRINT
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipConnection::dial "
                      "Using To address: '%s'",
                      goodToAddress.data());
#endif

        { //memory scope
            // Get the codecs
            int numCodecs;
            SdpCodec** rtpCodecsArray = NULL;
            supportedCodecs.getCodecs(numCodecs, rtpCodecsArray);

#ifdef TEST_PRINT
            UtlString codecsString;
            supportedCodecs.toString(codecsString);
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipConnection::dial "
                          "codecs:\n%s\n",
                          codecsString.data());
#endif

            // Prepare to receive the codecs
            mpMediaInterface->startRtpReceive(mConnectionId,
                numCodecs,
                rtpCodecsArray,
                srtpParams);

            // Create a contact using the host & port from the
            // SipUserAgent and the display name and userId from
            // the from URL.

            // Create the INVITE to send
            sipInvite.setInviteData(fromAddress.data(),
                                    goodToAddress.data(), NULL,
                                    mLocalContact.data(),
                                    callId,
                                    rtpAddress.data(), receiveRtpPort, receiveRtcpPort,
                                    receiveVideoRtpPort, receiveVideoRtcpPort, &srtpParams,
                                    lastLocalSequenceNumber,
                                    numCodecs, rtpCodecsArray,
                                    mDefaultSessionReinviteTimer,
                                    paiAddress);

            // Free up the codecs and the array
            for(int codecIndex = 0; codecIndex < numCodecs; codecIndex++)
            {
                delete rtpCodecsArray[codecIndex];
                rtpCodecsArray[codecIndex] = NULL;
            }
            delete[] rtpCodecsArray;
            rtpCodecsArray = NULL;
        }

            if (callController && callController[0] != '\0')
            {
                fireSipXEvent(CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_TRANSFER, (void*)originalCallId) ;
            }

        // Set caller preference if caller wants queueing or campon
        if(requestQueuedCall)
        {
            sipInvite.addRequestDisposition(SIP_DISPOSITION_QUEUE);
        }

        // Set the requested by field (BYE Also style transfer)
        if(callController && originalCallConnection == NULL)
        {
            UtlString requestedByField(callController);
            const char* alsoTags = strchr(dialString, '>');
            ssize_t uriIndex = requestedByField.index('<');
            if(uriIndex < 0)
            {
                requestedByField.insert(0, '<');
                requestedByField.append('>');
            }
            if(alsoTags)
            {
                alsoTags++;
                requestedByField.append(alsoTags);
            }
            sipInvite.setRequestedByField(requestedByField.data());
            cause = CONNECTION_CAUSE_TRANSFER;
        }

        // Set the header fields for REFER style transfer INVITE
        /*else*/ if(callController && originalCallConnection)
        {
            mOriginalCallConnectionAddress = originalCallConnection;
            sipInvite.setReferredByField(callController);

            UtlString referencesValue(originalCallId);
            referencesValue.append(";rel=refer");
            sipInvite.setHeaderValue(SIP_REFERENCES_FIELD,
                                     referencesValue);

            cause = CONNECTION_CAUSE_TRANSFER;
        }

        // Save a copy of the invite
        mInviteMsg = new SipMessage(sipInvite);
        inviteFromThisSide = TRUE;
        mPrevSdp = (mInviteMsg->getSdpBody())->copy();
        setCallerId();

        setState(Connection::CONNECTION_ESTABLISHED, Connection::CONNECTION_LOCAL);

        // Send the INVITE
        if ( !goodToAddress.isNull() 
           && send(sipInvite))
        {
            setState(CONNECTION_INITIATED, CONNECTION_REMOTE, cause);
#ifdef TEST_PRINT
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipConnection::dial "
                          "INVITE sent successfully");
#endif
            setState(CONNECTION_OFFERING, CONNECTION_REMOTE, cause);
            dialOk = TRUE;
            fireSipXEvent(CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, (void*)originalCallId) ;
        }
        else
        {
#ifdef TEST_PRINT
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipConnection::dial "
                          "INVITE send failed");
#endif
            setState(CONNECTION_FAILED, CONNECTION_REMOTE, CONNECTION_CAUSE_DEST_NOT_OBTAINABLE);
            fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_BADADDRESS) ;
            // Failed to send a message for transfer
            if(callController && !goodToAddress.isNull())
            {
                // Send back a status to the original call
                UtlString originalCallId;
                mpCall->getIdOfOrigCall(originalCallId);
                CpMultiStringMessage transfereeStatus(CallManager::CP_TRANSFEREE_CONNECTION_STATUS,
                    originalCallId.data(),
                    mOriginalCallConnectionAddress.data(),
                    NULL, NULL, NULL,
                    CONNECTION_FAILED, SIP_REQUEST_TIMEOUT_CODE);
#ifdef TEST_PRINT
                OsSysLog::add(FAC_SIP, PRI_DEBUG,
                              "Entering SipConnection::dial "
                              "posting CP_TRANSFEREE_CONNECTION_STATUS to call: '%s'",
                              originalCallId.data());
#endif
                mpCallManager->postMessage(transfereeStatus);
            }

        }
    }

    return(dialOk);
}

UtlBoolean SipConnection::sendInfo(UtlString contentType, UtlString sContent)
{
    bool bRet = false;


    SipMessage sipInfoMessage;
    UtlString fromAddress;
    UtlString toAddress;
    UtlString callId;
    UtlString uri ;

    mToUrl.getUri(uri) ;
    getFromField(&fromAddress);
    getRemoteAddress(&toAddress);
    getCallId(&callId);
    sipInfoMessage.setRequestData(SIP_INFO_METHOD, uri, fromAddress, toAddress, callId);
    sipInfoMessage.setContactField(mLocalContact.data());
    sipInfoMessage.setContentType(contentType.data());
    sipInfoMessage.setContentLength(sContent.length());
    HttpBody* pBody = new HttpBody(sContent.data(), sContent.length());
    sipInfoMessage.setBody(pBody);
    sipInfoMessage.setCSeqField(lastLocalSequenceNumber++, SIP_INFO_METHOD);

    if(send(sipInfoMessage, sipUserAgent->getMessageQueue()))
    {
        bRet = true;
    }
    else
    {
        // With sipX TAPI, send network error event.
        SIPX_INFOSTATUS_INFO info;

        memset((void*) &info, 0, sizeof(SIPX_INFOSTATUS_INFO));

        info.event = INFOSTATUS_NETWORK_ERROR;
        info.nSize = sizeof(SIPX_INFOSTATUS_INFO);
        info.hInfo = 0;
        info.szResponseText = (const char*)"INFO: network error";
        TapiMgr::getInstance().fireEvent(this->mpCallManager,
                                         EVENT_CATEGORY_INFO_STATUS, &info);
    }

    //delete pBody; // DONT delete here!  body is deleted by HttpMessage class
    return bRet;
}

UtlBoolean SipConnection::answer(const void* pDisplay)
{
#ifdef TEST_PRINT
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
        "Entering SipConnection::answer mInviteMsg=%p", mInviteMsg);
#endif

    UtlBoolean answerOk = FALSE;
    UtlBoolean hasSdpBody = FALSE;
    UtlString rtpAddress;
    int receiveRtpPort;
    int receiveRtcpPort;
    int receiveVideoRtpPort;
    int receiveVideoRtcpPort;
    SdpCodecFactory supportedCodecs;
    SdpSrtpParameters srtpParams;

    int currentState = getState();
    if( mpMediaInterface != NULL &&
        mInviteMsg && !inviteFromThisSide &&
        (currentState == CONNECTION_ALERTING ||
        currentState == CONNECTION_OFFERING ||
        currentState == CONNECTION_INITIATED ||
        currentState == CONNECTION_IDLE))
    {
        int numMatchingCodecs = 0;
        SdpCodec** matchingCodecs = NULL;
        SdpDirectionality directionality; 	// result not yet used

        mpMediaInterface->setVideoWindowDisplay(pDisplay);
        // Get supported codecs
        mpMediaInterface->getCapabilities(mConnectionId,
            rtpAddress,
            receiveRtpPort,
            receiveRtcpPort,
            receiveVideoRtpPort,        // VIDEO: TODO
            receiveVideoRtcpPort,
            supportedCodecs,
            srtpParams);

        getInitialSdpCodecs(mInviteMsg, supportedCodecs,
            numMatchingCodecs, matchingCodecs,
            remoteRtpAddress, remoteRtpPort, remoteRtcpPort, &directionality);


        hasSdpBody = mInviteMsg->hasSdpBody();
        if(numMatchingCodecs <= 0 && hasSdpBody)
        {
#ifdef TEST_PRINT
            osPrintf("No matching codecs rejecting call\n");
#endif

            // No common codecs send INVITE error response
            SipMessage sipResponse;
            sipResponse.setInviteBadCodecs(mInviteMsg, sipUserAgent);
            send(sipResponse);

            setState(CONNECTION_FAILED, CONNECTION_LOCAL, CONNECTION_CAUSE_RESOURCES_NOT_AVAILABLE);
            fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_RESOURCES) ;
        }

        // Compatable codecs send OK response
        else
        {
            // Respond with an OK
#ifdef TEST_PRINT
            osPrintf("Sending INVITE OK\n");
#endif

            // There was no SDP in the INVITE, so give them all of
            // the codecs we support.
            if(!hasSdpBody)
            {
#ifdef TEST_PRINT
                osPrintf("Sending initial SDP in OK\n");
#endif

                // There were no codecs specified in the INVITE
                // Give the full set of supported codecs
                supportedCodecs.getCodecs(numMatchingCodecs, matchingCodecs);
            }

            // If there was SDP in the INVITE and it indicated hold:
            if(hasSdpBody && remoteRtpPort <= 0)
            {
                rtpAddress = "0.0.0.0";  // hold address
            }

            OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipConnection::answer");
            // Tweak Contact given request URI / settings
            setContactType(selectCompatibleContactType(*mInviteMsg)) ;

            // Get Media Capabilties (need to call again because rtp
            // addresses and ports may have changed)
            mpMediaInterface->getCapabilities(mConnectionId,
                rtpAddress,
                receiveRtpPort,
                receiveRtcpPort,
                receiveVideoRtpPort,        // VIDEO: TODO
                receiveVideoRtcpPort,
                supportedCodecs,
                srtpParams);

            // Build response
            SipMessage sipResponse;

            sipResponse.setInviteOkData(mInviteMsg, rtpAddress.data(),
                receiveRtpPort, receiveRtcpPort,
                receiveVideoRtpPort, receiveVideoRtcpPort,
                numMatchingCodecs, matchingCodecs,
                srtpParams,
                mDefaultSessionReinviteTimer, mLocalContact.data());

            // Send a INVITE OK response
            if(!send(sipResponse))
            {
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                    "SipConnection::answer: INVITE OK failed: %s",
                    remoteRtpAddress.data());
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                    "SipConnection::answer: CONNECTION_FAILED, CONNECTION_LOCAL, CONNECTION_CAUSE_NORMAL");
                //phoneSet->setStatusDisplay(displayMsg);
                setState(CONNECTION_FAILED, CONNECTION_LOCAL, CONNECTION_CAUSE_NORMAL);
                fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NETWORK) ;
            }
            else
            {
                mPrevSdp = (sipResponse.getSdpBody())->copy();
                setState(CONNECTION_ESTABLISHED, CONNECTION_LOCAL, CONNECTION_CAUSE_NORMAL);
                if (mTerminalConnState == PtTerminalConnection::HELD)
                {
                    fireSipXEvent(CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD);
                }
                else
                {
                    fireSipXEvent(CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE);
                }

                answerOk = TRUE;

                // Setup media channel
#ifdef TEST_PRINT
                osPrintf("Setting up flowgraph receive\n");
#endif
                // Start receiving media
                SdpCodec recvCodec((SdpCodec::SdpCodecTypes) receiveCodec);
                mpMediaInterface->startRtpReceive(mConnectionId,
                    numMatchingCodecs, matchingCodecs, srtpParams);

                // if we have a send codec chosen Start sending media
                if(numMatchingCodecs > 0)
                {
                    mpMediaInterface->setConnectionDestination(mConnectionId,
                        remoteRtpAddress.data(),
                        remoteRtpPort,
                        remoteRtcpPort,
                        receiveVideoRtpPort,
                        receiveVideoRtcpPort);
                    // Set up the remote RTP sockets
#ifdef TEST_PRINT
                    osPrintf("RTP SENDING address: %s port: %d\n", remoteRtpAddress.data(), remoteRtpPort);
#endif

                    if(remoteRtpPort > 0)
                    {
                        //SdpCodec sndCodec((SdpCodec::SdpCodecTypes)
                        //    sendCodec);
                        mpMediaInterface->startRtpSend(mConnectionId,
                            numMatchingCodecs, matchingCodecs, srtpParams);

                        // If sipX TAPI, fire audio start event
                        UtlString audioCodecName;
                        UtlString videoCodecName;
                        SIPX_CODEC_INFO tapiCodec;
                        if (mpMediaInterface->getPrimaryCodec(mConnectionId,
                                                              audioCodecName,
                                                              videoCodecName,
                                                              &tapiCodec.audioCodec.iPayloadType,
                                                              &tapiCodec.videoCodec.iPayloadType) == OS_SUCCESS)
                        {
                            strncpy(tapiCodec.audioCodec.cName, audioCodecName.data(), SIPXTAPI_CODEC_NAMELEN-1);
                            strncpy(tapiCodec.videoCodec.cName, videoCodecName.data(), SIPXTAPI_CODEC_NAMELEN-1);
                            fireSipXEvent(CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, &tapiCodec) ;
                        }
                    }
                }

#ifdef TEST_PRINT
                osPrintf("RECEIVING RTP\n");
#endif

                mInviteMsg->getAllowField(mAllowedRemote);
            }
        }

        // Free up the codec copies and array
        for(int codecIndex = 0; codecIndex < numMatchingCodecs; codecIndex++)
        {
            delete matchingCodecs[codecIndex];
            matchingCodecs[codecIndex] = NULL;
        }
        delete[] matchingCodecs;
        matchingCodecs = NULL;
    }

#ifdef TEST_PRINT
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
        "Leaving SipConnection::answer mInviteMsg=%p ", mInviteMsg);
#endif

    return(answerOk);
}

UtlBoolean SipConnection::accept(int ringingTimeOutSeconds)
{
    UtlBoolean ringingSent = FALSE;
    int cause = 0;
#ifdef TEST_PRINT
    osPrintf("SipConnection::accept ringingTimeOutSeconds=%d\n", ringingTimeOutSeconds);
#endif
    if(mpMediaInterface != NULL && mInviteMsg &&
        !inviteFromThisSide && getState(cause) == CONNECTION_OFFERING)
    {
        UtlString rtpAddress;
        int receiveRtpPort;
        int receiveRtcpPort;
        int receiveVideoRtpPort;
        int receiveVideoRtcpPort;
        //UtlBoolean receiveCodecSet;
        //UtlBoolean sendCodecSet;
        int numMatchingCodecs = 0;
        SdpCodec** matchingCodecs = NULL;
        SdpCodecFactory supportedCodecs;
        UtlString replaceCallId;
        UtlString replaceToTag;
        UtlString replaceFromTag;
        SdpSrtpParameters srtpParams;
        SdpDirectionality directionality; 	// result not yet used

        // Make sure that this isn't part of a transfer.  If we find a
        // REPLACES header, then we shouldn't accept the call, but rather
        // we should return a 481 response.
        if (mInviteMsg->getReplacesData(replaceCallId, replaceToTag, replaceFromTag))
        {
            SipMessage badTransaction;
            badTransaction.setBadTransactionData(mInviteMsg);
            send(badTransaction);
#ifdef TEST_PRINT
            osPrintf("SipConnection::accept - CONNECTION_FAILED, cause BUSY : 754\n");
#endif
            setState(CONNECTION_FAILED, CONNECTION_REMOTE, CONNECTION_CAUSE_BUSY);
            fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_UNKNOWN) ;
        }
        else
        {
            mpMediaInterface->getCapabilities(mConnectionId,
                rtpAddress,
                receiveRtpPort,
                receiveRtcpPort,
                receiveVideoRtpPort,    // VIDEO: TODO
                receiveVideoRtcpPort,
                supportedCodecs,
                srtpParams);

            // Get the codecs if SDP is provided
            getInitialSdpCodecs(mInviteMsg,
                supportedCodecs,
                numMatchingCodecs, matchingCodecs,
                remoteRtpAddress, remoteRtpPort, remoteRtcpPort, &directionality);

            // Try to setup for early receipt of media.
            if(numMatchingCodecs > 0)
            {
                SdpCodec recvCodec((SdpCodec::SdpCodecTypes) receiveCodec);
                mpMediaInterface->startRtpReceive(mConnectionId,
                        numMatchingCodecs, matchingCodecs, srtpParams);
            }
            ringingSent = TRUE;
            proceedToRinging(mInviteMsg, sipUserAgent, -1, mLineAvailableBehavior);

            // Keep track of the fact that this is a transfer
            if(cause != CONNECTION_CAUSE_TRANSFER)
            {
                cause = CONNECTION_CAUSE_NORMAL;
            }
            setState(CONNECTION_ALERTING, CONNECTION_LOCAL, cause);
            fireSipXEvent(CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL) ;

            // If forward on no answer is enabled set the timer
            if(ringingTimeOutSeconds > 0 )
            {
                // Set a timer to post a message to this call
                // to timeout the ringing and forward
                setRingingTimer(ringingTimeOutSeconds);
            }

            // Free up the codec copies and array
            for(int codecIndex = 0; codecIndex < numMatchingCodecs; codecIndex++)
            {
                delete matchingCodecs[codecIndex];
                matchingCodecs[codecIndex] = NULL;
            }
            delete[] matchingCodecs;
            matchingCodecs = NULL;
        }
    }

    return(ringingSent);
}

UtlBoolean SipConnection::reject(int errorCode, const char* errorText)
{
    UtlBoolean responseSent = FALSE;
    if(mInviteMsg && !inviteFromThisSide)
    {
        int state = getState();
        if (state == CONNECTION_OFFERING)
        {
            UtlString replaceCallId;
            UtlString replaceToTag;
            UtlString replaceFromTag;

            // Make sure that this isn't part of a transfer.  If we find a
            // REPLACES header, then we shouldn't accept the call, but rather
            // we should return a 481 response.
            if (mInviteMsg->getReplacesData(replaceCallId, replaceToTag, replaceFromTag))
            {
                SipMessage badTransaction;
                badTransaction.setBadTransactionData(mInviteMsg);
                responseSent = send(badTransaction);
#ifdef TEST_PRINT
                osPrintf("SipConnection::reject - CONNECTION_FAILED, cause BUSY : 825\n");
#endif
                setState(CONNECTION_FAILED, CONNECTION_REMOTE, CONNECTION_CAUSE_BUSY);
                fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_UNKNOWN) ;
            }
            else
            {
                // Validate error/reject reason
                if (errorCode <= 100 || errorCode >= 700 ||
                        errorText == NULL || strlen(errorText) == 0)
                {
                    errorCode = SIP_BUSY_CODE ;
                    errorText = SIP_BUSY_TEXT ;
                }

                SipMessage rejectMessage;
                rejectMessage.setResponseData(mInviteMsg, errorCode, errorText);
                responseSent = send(rejectMessage);
#ifdef TEST_PRINT
                osPrintf("SipConnection::reject - CONNECTION_FAILED, cause BUSY : 833\n");
#endif
                setState(CONNECTION_FAILED, CONNECTION_REMOTE, CONNECTION_CAUSE_BUSY);
                fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL) ;
            }
        }
        else if (state == CONNECTION_ALERTING)
        {
            SipMessage terminateMessage;
            terminateMessage.setRequestTerminatedResponseData(mInviteMsg);
            responseSent = send(terminateMessage);
#ifdef TEST_PRINT
            osPrintf("SipConnection::reject - CONNECTION_DISCONNECTED, cause CONNECTION_CAUSE_CANCELLED : 845\n");
#endif
            setState(CONNECTION_DISCONNECTED, CONNECTION_REMOTE, CONNECTION_CAUSE_CANCELLED);
            fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL) ;
        }
    }
    return(responseSent);
}

UtlBoolean SipConnection::redirect(const char* forwardAddress)
{
    UtlBoolean redirectSent = FALSE;
    if(mInviteMsg && !inviteFromThisSide &&
        (getState() == CONNECTION_OFFERING ||
        getState() == CONNECTION_ALERTING))
    {
        UtlString targetUrl;
        UtlString dummyFrom;
        const char* callerDisplayName = NULL;
        const char* targetCallId = NULL;
        buildFromToAddresses(forwardAddress, targetCallId, callerDisplayName,
            dummyFrom, targetUrl);
        // Send a redirect message
        SipMessage redirectResponse;
        redirectResponse.setForwardResponseData(mInviteMsg,
            targetUrl.data());
        redirectSent = send(redirectResponse);
        setState(CONNECTION_DISCONNECTED, CONNECTION_REMOTE, CONNECTION_CAUSE_REDIRECTED);
        setState(CONNECTION_DISCONNECTED, CONNECTION_LOCAL, CONNECTION_CAUSE_REDIRECTED);
        fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_REDIRECTED) ;

        targetUrl = OsUtil::NULL_OS_STRING;
        dummyFrom = OsUtil::NULL_OS_STRING;
    }
    return(redirectSent);
}

UtlBoolean SipConnection::hangUp()
{
    return(doHangUp());
}

UtlBoolean SipConnection::hold()
{
    UtlBoolean messageSent = FALSE;
    SdpSrtpParameters srtpParams;
    // If the call is connected and we are not in the middle of a SIP transaction
    if(mpMediaInterface != NULL &&
            mInviteMsg && getState() == CONNECTION_ESTABLISHED &&
            reinviteState == ACCEPT_INVITE &&
            mTerminalConnState != PtTerminalConnection::HELD)
    {
        UtlString rtpAddress;
        int receiveRtpPort;
        int receiveRtcpPort;
        int receiveVideoRtpPort;
        int receiveVideoRtcpPort;
        SdpCodecFactory supportedCodecs;
        mpMediaInterface->getCapabilities(mConnectionId,
            rtpAddress,
            receiveRtpPort,
            receiveRtcpPort,
            receiveVideoRtpPort,    // VIDEO: TODO
            receiveVideoRtcpPort,
            supportedCodecs,
            srtpParams);
        int numCodecs = 0;
        SdpCodec** codecsArray = NULL;
        supportedCodecs.getCodecs(numCodecs, codecsArray);

        // Build an INVITE with the RTP address in the SDP of 0.0.0.0
        SipMessage holdMessage;
        holdMessage.setReinviteData(mInviteMsg,
            mRemoteContactUri,
            mLocalContact.data(),
            inviteFromThisSide,
            mRouteField,
            "0.0.0.0",
            receiveRtpPort,
            receiveRtcpPort,
            receiveVideoRtpPort,
            receiveVideoRtcpPort,
            ++lastLocalSequenceNumber,
            numCodecs,
            codecsArray,
            &srtpParams,
            mDefaultSessionReinviteTimer);

        if(mInviteMsg)
        {
            delete mInviteMsg;
        }
        mInviteMsg = new SipMessage(holdMessage);
        inviteFromThisSide = TRUE;

        if(send(holdMessage))
        {
            mPrevSdp = (mInviteMsg->getSdpBody())->copy();
            messageSent = TRUE;

            // Disallow INVITEs while this transaction is taking place
            reinviteState = REINVITING;
            mFarEndHoldState = TERMCONNECTION_HOLDING;
        }

        // Free up the codec copies and array
        for(int codecIndex = 0; codecIndex < numCodecs; codecIndex++)
        {
            delete codecsArray[codecIndex];
            codecsArray[codecIndex] = NULL;
        }
        delete[] codecsArray;
        codecsArray = NULL;
    }
    return(messageSent);
}

UtlBoolean SipConnection::offHold()
{
    return(doOffHold(FALSE));
}

UtlBoolean SipConnection::renegotiateCodecs()
{
    return(doOffHold(TRUE));
}

UtlBoolean SipConnection::sendKeepAlive(UtlBoolean useOptionsForKeepalive)
{
    if(!useOptionsForKeepalive)
    {
       if(mpMediaInterface != NULL &&
          mInviteMsg && getState() == CONNECTION_ESTABLISHED &&
          reinviteState == ACCEPT_INVITE &&
          mTerminalConnState == PtTerminalConnection::TALKING)
       {
           if(mPrevSdp)
           {
               UtlString bodyString;
               ssize_t len;

               SipMessage keepAliveMessage;
               keepAliveMessage.setReinviteData(mInviteMsg,
                                                mRemoteContactUri,
                                                mLocalContact.data(),
                                                inviteFromThisSide,
                                                mRouteField,
                                                ++lastLocalSequenceNumber,
                                                mDefaultSessionReinviteTimer);

               keepAliveMessage.setBody(mPrevSdp->copy());
               // Add the content type for the body
               keepAliveMessage.setContentType(SDP_CONTENT_TYPE);

               // Add the content length
               mPrevSdp->getBytes(&bodyString, &len);
               keepAliveMessage.setContentLength(len);

               delete mInviteMsg;

               mInviteMsg = new SipMessage(keepAliveMessage);
               inviteFromThisSide = TRUE;

               if(send(keepAliveMessage))
               {
                   // Disallow INVITEs while this transaction is taking place
                   reinviteState = REINVITING;
                   return(TRUE);
               }
           }
           else
           {
              doOffHold(TRUE);
           }
       }
    }
    else if(getState() == CONNECTION_ESTABLISHED &&
            mTerminalConnState == PtTerminalConnection::TALKING &&
            mInviteMsg)
    {
        SipMessage sipOptionsMessage;

        lastLocalSequenceNumber++;
        sipOptionsMessage.setOptionsData(mInviteMsg, mRemoteContactUri.data(),
                                         inviteFromThisSide, lastLocalSequenceNumber,
                                         mRouteField.data(), mLocalContact.data());

        sipOptionsMessage.setHeaderValue(SIP_ACCEPT_FIELD, SDP_CONTENT_TYPE);
        mWaitingForKeepAliveResponse=TRUE;

        return( send(sipOptionsMessage, sipUserAgent->getMessageQueue()) );
    }

    return (FALSE);
}

UtlBoolean SipConnection::doOffHold(UtlBoolean forceReInvite)
{
    UtlBoolean messageSent = FALSE;
    SdpSrtpParameters srtpParams;

    // If the call is connected and
    // we are not in the middle of a SIP transaction
    if(mpMediaInterface != NULL &&
            mInviteMsg && getState() == CONNECTION_ESTABLISHED &&
            reinviteState == ACCEPT_INVITE &&
            (mTerminalConnState == PtTerminalConnection::HELD ||
            (forceReInvite &&
            mTerminalConnState == PtTerminalConnection::TALKING)))

    {
        UtlString rtpAddress;
        int receiveRtpPort;
        int receiveRtcpPort;
        int receiveVideoRtpPort;
        int receiveVideoRtcpPort;
        SdpCodecFactory supportedCodecs;
        mpMediaInterface->getCapabilities(mConnectionId,
            rtpAddress,
            receiveRtpPort,
            receiveRtcpPort,
            receiveVideoRtpPort,    // VIDEO: TODO
            receiveVideoRtcpPort,
            supportedCodecs,
            srtpParams);

        int numCodecs = 0;
        SdpCodec** rtpCodecs = NULL;
        supportedCodecs.getCodecs(numCodecs, rtpCodecs);

        // Build an INVITE with the RTP address in the SDP
        // as the real address
#ifdef TEST_PRINT
        osPrintf("SipConnection::offHold rtpAddress: %s\n",
            rtpAddress.data());
#endif
        SipMessage offHoldMessage;
        offHoldMessage.setReinviteData(mInviteMsg,
            mRemoteContactUri,
            mLocalContact.data(),
            inviteFromThisSide,
            mRouteField,
            rtpAddress.data(),
            receiveRtpPort,
            receiveRtcpPort,
            receiveVideoRtpPort,
            receiveVideoRtcpPort,
            ++lastLocalSequenceNumber,
            numCodecs,
            rtpCodecs,
            &srtpParams,
            mDefaultSessionReinviteTimer);

        // Free up the codec copies and array
        for(int codecIndex = 0; codecIndex < numCodecs; codecIndex++)
        {
            delete rtpCodecs[codecIndex];
            rtpCodecs[codecIndex] = NULL;
        }
        delete[] rtpCodecs;
        rtpCodecs = NULL;

        if(mInviteMsg)
        {
            delete mInviteMsg;
        }
        mInviteMsg = new SipMessage(offHoldMessage);
        inviteFromThisSide = TRUE;
        mPrevSdp = (offHoldMessage.getSdpBody())->copy();

        if(send(offHoldMessage))
        {
            messageSent = TRUE;
            // Disallow INVITEs while this transaction is taking place
            reinviteState = REINVITING;

            // If we are doing a forced reINVITE
            // there are no state changes

            // Otherwise signal the offhold state changes
            if(!forceReInvite)
            {
                mFarEndHoldState = TERMCONNECTION_TALKING;
                if (mpCall->getCallType() != CpCall::CP_NORMAL_CALL)
                {
                    mpCall->setCallType(CpCall::CP_NORMAL_CALL);
                }
                setState(CONNECTION_ESTABLISHED, CONNECTION_REMOTE, CONNECTION_CAUSE_UNHOLD);
                // fireSipXEvent(CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE) ;
            }
        }
    }
    return(messageSent);
}

UtlBoolean SipConnection::originalCallTransfer(UtlString& dialString,
                                               const char* transferControllerAddress,
                                               const char* targetCallId,
                                               bool        holdBeforeTransfer
                                               )
{
    UtlBoolean ret = FALSE;

    mIsReferSent = FALSE;
#ifdef TEST_PRINT
    UtlString remoteAddr;
    getRemoteAddress(&remoteAddr);
    UtlString conState;
    getStateString(getState(), &conState);
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
        "SipConnection::originalCallTransfer "
        "on %s %p %p:\"%s\" state: %s",
                  remoteAddr.data(),
                  mInviteMsg,
                  dialString.data(),
                  dialString.length() ? dialString.data() : "", conState.data());
#endif
    if(mInviteMsg
       && dialString
       && *dialString
       && getState() == CONNECTION_ESTABLISHED)
    {
        // If the transferee (the party at the other end of this
        // connection) supports the REFER method
        const char* callerDisplayName = NULL;

        // If blind transfer
        {
            UtlString targetUrl;
            UtlString dummyFrom;
            buildFromToAddresses(dialString, targetCallId, callerDisplayName,
                dummyFrom, targetUrl);
            dialString = targetUrl;
            //alsoUri.append(";token1");
        }

        if(isMethodAllowed(SIP_REFER_METHOD))
        {
            fireSipXEvent(CALLSTATE_TRANSFER, CALLSTATE_TRANSFER_INITIATED) ;

            mTargetCallConnectionAddress = dialString;
            mTargetCallId = targetCallId;
            if (    holdBeforeTransfer
                && (   mFarEndHoldState == TERMCONNECTION_TALKING
                    || mFarEndHoldState == TERMCONNECTION_NONE
                    ))
            {
               // If the connection is not already on hold, do a hold
               // first and then do the REFER transfer
                mHoldCompleteAction = CpCallManager::CP_BLIND_TRANSFER;
                //need to do a remote hold first
                // Then after that is complete do the REFER
                hold();
                ret = TRUE;
            }
            else
            {
                // Send a REFER to tell the transferee to
                // complete a blind transfer
                doBlindRefer();
                ret = mIsReferSent;
            }
        }
        else
        {
            fireSipXEvent(CALLSTATE_TRANSFER, CALLSTATE_TRANSFER_FAILURE) ;
#if 0
            // Use the old BYE Also method of transfer
            doHangUp(dialString.data(),
                transferControllerAddress);
            mTargetCallConnectionAddress = dialString;
            ret = TRUE;
#endif
        }
    }
    return(ret);
}

void SipConnection::doBlindRefer()
{
    // Send a REFER message
    SipMessage referRequest;
    lastLocalSequenceNumber++;

    referRequest.setReferData(mInviteMsg,
        inviteFromThisSide,
        lastLocalSequenceNumber,
        mRouteField.data(),
        mLocalContact.data(),
        mRemoteContactUri.data(),
        mTargetCallConnectionAddress.data(),
        // The following keeps the target call id on the
        // transfer target the same as the consultative call
        //mTargetCallId);
        // The following does not set the call Id on the xfer target
        "");
    UtlString referencesValue(mTargetCallId);
    referencesValue.append(";rel=xfer");
    referRequest.setHeaderValue(SIP_REFERENCES_FIELD,
                                referencesValue);

    mIsReferSent = send(referRequest);
}

UtlBoolean SipConnection::targetCallBlindTransfer(const char* dialString,
                                                  const char* transferControllerAddress)
{
    // This should never get here
    unimplemented("SipConnection::targetCallBlindTransfer");
    return(FALSE);
}

UtlBoolean SipConnection::transferControllerStatus(int connectionState, int response)
{
    if (connectionState == Connection::CONNECTION_FAILED)
    {
        setState(connectionState, CONNECTION_REMOTE);
    }
    // It should never get here
    unimplemented("SipConnection::transferControllerStatus");
    return(FALSE);
}

UtlBoolean SipConnection::transfereeStatus(int callState, int returnCode)
{
    UtlBoolean referResponseSent = FALSE;

#ifdef TEST_PRINT
    osPrintf("SipConnection::transfereeStatus callType: %d referMessage: %p\n",
        mpCall->getCallType(), mReferMessage);
#endif
    // If this call and connection received a REFER request
    if(mpCall->getCallType() ==
        CpCall::CP_TRANSFEREE_ORIGINAL_CALL &&
        mReferMessage)
    {
        UtlString transferMethod;
        mReferMessage->getRequestMethod(&transferMethod);

        // REFER type transfer
        if(transferMethod.compareTo(SIP_REFER_METHOD) == 0)
        {
            int num;
            UtlString method;
            mReferMessage->getCSeqField(&num , &method);

            UtlString event;
            event.append(SIP_EVENT_REFER);
            event.append(";cseq=");
            char buff[50];
            sprintf(buff,"%d", num);
            event.append(buff);

            // Generate an appropriate NOTIFY message to indicate the
            // outcome
            SipMessage referNotify;

            HttpBody* body = NULL;
            lastLocalSequenceNumber++;
            referNotify.setNotifyData(mReferMessage,
                                      lastLocalSequenceNumber,
                                      mRouteField,
                                      NULL,
                                      event,
                                      "id");
            if(callState == CONNECTION_ESTABLISHED)
            {
                //transferResponse.setReferOkData(mReferMessage);
                body = new HttpBody(SIP_REFER_SUCCESS_STATUS, -1, CONTENT_TYPE_MESSAGE_SIPFRAG);
            }
            else if(callState == CONNECTION_ALERTING)
            {
                SipMessage alertingMessage;
                switch(returnCode)
                {
                case SIP_EARLY_MEDIA_CODE:
                    alertingMessage.setResponseFirstHeaderLine(SIP_PROTOCOL_VERSION,
                        returnCode, SIP_RINGING_TEXT);
                    break;

                default:
                    alertingMessage.setResponseFirstHeaderLine(SIP_PROTOCOL_VERSION,
                        returnCode, SIP_RINGING_TEXT);
                    break;
                }
                UtlString messageBody;
                ssize_t len;
                alertingMessage.getBytes(&messageBody,&len);

                body = new HttpBody(messageBody.data(), -1, CONTENT_TYPE_MESSAGE_SIPFRAG);
            }
            else
            {
                //transferResponse.setReferFailedData(mReferMessage);
                body = new HttpBody(SIP_REFER_FAILURE_STATUS, -1, CONTENT_TYPE_MESSAGE_SIPFRAG);
            }
            referNotify.setBody(body);

            // Add the content type for the body
            referNotify.setContentType(CONTENT_TYPE_MESSAGE_SIPFRAG);

            // Add the content length
            ssize_t len;
            UtlString bodyString;
            body->getBytes(&bodyString, &len);
            referNotify.setContentLength(len);

            referResponseSent = send(referNotify);

            // Only delete if this is a final notify
            if(callState != CONNECTION_ALERTING && mReferMessage)
            {
                delete mReferMessage;
                mReferMessage = NULL;
            }

            // If we are in the middle of a transfer meta event
            // on the target phone and target call it ends here
            int metaEventId = 0;
            int metaEventType = PtEvent::META_EVENT_NONE;
            int numCalls = 0;
            const UtlString* metaEventCallIds = NULL;
            if(mpCall)
            {
               OsSysLog::add(FAC_CP, PRI_DEBUG,
                             "SipConnection::transfereeStatus "
                             "META_CALL_TRANSFERRING end");
               mpCall->getMetaEvent(metaEventId, metaEventType,
                                    numCalls, &metaEventCallIds);
               if(   metaEventId > 0
                  && metaEventType == PtEvent::META_CALL_TRANSFERRING)
               {
#ifdef TEST_PRINT
                   OsSysLog::add(FAC_CP, PRI_DEBUG,
                                 "SipConnection::transfereeStatus "
                                 "stopMetaEvent 1");
#endif
                  mpCall->stopMetaEvent();
               }
            }
        }

        // Should be BYE 'Also' type transfer
        else
        {
            SipMessage transferResponse;
            if(callState == CONNECTION_ESTABLISHED)
            {
                transferResponse.setOkResponseData(mReferMessage, mLocalContact);
            }
            else
            {
                transferResponse.setReferFailedData(mReferMessage);
            }
            referResponseSent = send(transferResponse);
            if(mReferMessage) delete mReferMessage;
            mReferMessage = NULL;
        }
    }

    else
    {
        osPrintf("SipConnection::transfereeStatus FAILED callType: %d mReferMessage: %p\n",
            mpCall->getCallType() , mReferMessage);
    }
    return(referResponseSent);
}

UtlBoolean SipConnection::doHangUp(const char* dialString,
                                   const char* callerId)
{
    UtlBoolean hangUpOk = FALSE;

    if (!mDropping)
    {
        mDropping = true ;

        int cause;
        // always get remote connection state
        int currentState = getState(0, cause);
        const char* callerDisplayName = NULL;
        SipMessage sipRequest;
        UtlString alsoUri;

        // If blind transfer
        if(dialString && *dialString)
        {
            UtlString dummyFrom;
            buildFromToAddresses(dialString, callerId, callerDisplayName,
                dummyFrom, alsoUri);
        }

        // Tell the other end that we are hanging up
        // Need to send SIP CANCEL if we are the caller
        // and the callee connection state is not finalized
        if(mRemoteIsCallee
           && currentState != CONNECTION_FAILED
           && currentState != CONNECTION_ESTABLISHED
           && currentState != CONNECTION_DISCONNECTED
           && currentState != CONNECTION_UNKNOWN)
        {
            // We are the caller, cancel the incomplete call
            // Send a CANCEL
            //sipRequest = new SipMessage();

            // We are calling and the call is not setup yet so
            // cancel.  If we get a subsequent OK, we need to send
            // a BYE.
            if(inviteFromThisSide)
            {
                sipRequest.setCancelData(mInviteMsg);
                mLastRequestMethod = SIP_CANCEL_METHOD;

                // If this was a canceled transfer INVITE, send back a status
                if(!mOriginalCallConnectionAddress.isNull())
                {
                    UtlString originalCallId;
                    mpCall->getIdOfOrigCall(originalCallId);
                    CpMultiStringMessage transfereeStatus(CallManager::CP_TRANSFEREE_CONNECTION_STATUS,
                        originalCallId.data(),
                        mOriginalCallConnectionAddress.data(),
                        NULL, NULL, NULL,
                        CONNECTION_FAILED, SIP_REQUEST_TIMEOUT_CODE);
#ifdef TEST_PRINT
                    osPrintf("SipConnection::processResponse posting CP_TRANSFEREE_CONNECTION_STATUS to call: %s\n",
                        originalCallId.data());
#endif
                    mpCallManager->postMessage(transfereeStatus);

                }
            }

            // Someone is calling us and we are hanging up before the
            // call is setup.  This is not likely to occur and I am
            // not sure what the right thing to do here CANCEL or BYE.
            else
            {
                lastLocalSequenceNumber++;
#ifdef TEST_PRINT
                osPrintf("doHangup BYE route: %s\n", mRouteField.data());
#endif
                sipRequest.setByeData(mInviteMsg,
                    mRemoteContactUri,
                    inviteFromThisSide,
                    lastLocalSequenceNumber,
                    mRouteField.data(),
                    alsoUri.data(),
                    mLocalContact.data());

                mLastRequestMethod = SIP_BYE_METHOD;
            }

            if(send(sipRequest))
            {
#ifdef TEST_PRINT

                if(inviteFromThisSide)
                    osPrintf("unsetup call CANCEL message sent\n");
                else
                    osPrintf("unsetup call BYE message sent\n");
#endif
                // Lets try not setting this to disconected until
                // we get the response or a timeout
                //setState(CONNECTION_DISCONNECTED, CONNECTION_REMOTE, CONNECTION_CAUSE_CANCELLED);

                // However we need something to indicate that the call
                // is being cancelled to handle a race condition when the callee responds with 200 OK
                // before it receives the Cancel.
                mbCancelling = TRUE;

                hangUpOk = TRUE;
            }
        }

        // We are the Caller or callee
        else if(currentState == CONNECTION_ESTABLISHED)
        {
            // the call is connected
            // Send a BYE
            //sipRequest = new SipMessage();
            //UtlString directoryServerUri;
            //if(!inviteFromThisSide)
            //{
            //UtlString dirAddress;
            //UtlString dirProtocol;
            //int dirPort;

            //sipUserAgent->getDirectoryServer(0, &dirAddress,
            //        &dirPort, &dirProtocol);
            //SipMessage::buildSipUri(&directoryServerUri,
            //    dirAddress.data(), dirPort, dirProtocol.data());
            //}
            lastLocalSequenceNumber++;

#ifdef TEST_PRINT
            osPrintf("setup call BYE route: %s remote contact: %s\n",
                mRouteField.data(), mRemoteContactUri.data());
#endif
            sipRequest.setByeData(mInviteMsg,
                mRemoteContactUri,
                inviteFromThisSide,
                lastLocalSequenceNumber,
                mRouteField.data(),
                alsoUri.data(),
                mLocalContact.data());

            mLastRequestMethod = SIP_BYE_METHOD;
            if (!send(sipRequest))
            {
                // If sending the BYE failed, we will receive no response to end
                // the connection.  So we have to generate it here, as we cannot
                // allow the inability to send a BYE to make it impossible to
                // disconnect a call.
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                    "SipConnection::doHangUp: Sending BYE failed.  "
                    "Terminating connection.");
                setState(CONNECTION_DISCONNECTED, CONNECTION_REMOTE);
                fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL);
            }

            hangUpOk = TRUE;
        }

        if (mpMediaInterface != NULL)
        {
            mpMediaInterface->stopRtpSend(mConnectionId);
            mpMediaInterface->stopRtpReceive(mConnectionId);

            fireSipXEvent(CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP);
        }
    }

    return(hangUpOk);
}

void SipConnection::buildFromToAddresses(const char* dialString,
                                         const char* callerId,
                                         const char* callerDisplayName,
                                         UtlString& fromAddress,
                                         UtlString& goodToAddress) const
{
    UtlString sipAddress;
    int sipPort;
    UtlString sipProtocol;

    fromAddress.remove(0);
    goodToAddress.remove(0);

    // Build a From address
    sipUserAgent->getFromAddress(&sipAddress, &sipPort, &sipProtocol);
    SipMessage::buildSipUri(&fromAddress, 
                            sipAddress.data(),
                            sipPort,
                            sipProtocol.data(), 
                            callerId,
                            callerDisplayName,
                            mFromTag.data());

    // Check the to Address
    UtlString toAddress;
    UtlString toProtocol;
    UtlString toUser;
    UtlString toUserLabel;

    int toPort;

#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG,
        "SipConnection::buildFromToAddresses "
        "got dial string: '%s'",
        dialString);
#endif

    // Use the Url object to perserve parameters and display name
    Url toUrl(dialString);
    toUrl.getHostAddress(toAddress);

    //SipMessage::parseAddressFromUri(dialString, &toAddress, &toPort,
    //      &toProtocol, &toUser, &toUserLabel);
    if(toAddress.isNull())
    {
        sipUserAgent->getDirectoryServer(0, &toAddress,
            &toPort, &toProtocol);
#ifdef TEST_PRINT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
            "SipConnection::buildFromToAddresses "
            "Got directory server: '%s'",
            toAddress.data());
#endif
        toUrl.setHostAddress(toAddress.data());
        toUrl.setHostPort(toPort);
        if(!toProtocol.isNull())
        {
            toUrl.setUrlParameter("transport", toProtocol.data());
        }
    }
    //SipMessage::buildSipUri(&goodToAddress, toAddress.data(),
    //              toPort, toProtocol.data(), toUser.data(),
    //              toUserLabel.data());
    toUrl.toString(goodToAddress);
#ifdef TEST_PRINT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
            "SipConnection::buildFromToAddresses "
            "goodToAddress: '%s'",
            goodToAddress.data());

        toUrl.kedump();
#endif
}

UtlBoolean SipConnection::processMessage(OsMsg& eventMessage,
                                         UtlBoolean callInFocus,
                                         UtlBoolean onHook)
{
    int msgType = eventMessage.getMsgType();
    int msgSubType = eventMessage.getMsgSubType();
    UtlBoolean processedOk = TRUE;
    const SipMessage* sipMsg = NULL;
    int messageType;

    if(msgType == OsMsg::PHONE_APP &&
        msgSubType == CallManager::CP_SIP_MESSAGE)
    {
        sipMsg = ((SipMessageEvent&)eventMessage).getMessage();
        messageType = ((SipMessageEvent&)eventMessage).getMessageStatus();
#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG,
                          "SipConnection::processMessage "
                          "messageType: %d", messageType);
#endif
        UtlBoolean messageIsResponse = sipMsg->isResponse();
        UtlString method;
        if(!messageIsResponse)
        {
            sipMsg->getRequestMethod(&method);
        }

        // This is a request which failed to get sent
        if(messageType == SipMessageEvent::TRANSPORT_ERROR)
        {
#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG,
                          "SipConnection::processMessage "
                          "transport error method: %s",
                          messageIsResponse ? method.data() : "response");
#endif
            if(!mInviteMsg)
            {
#ifdef TEST_PRINT
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                              "SipConnection::processMessage "
                              "failed response");
#endif
                // THis call was not setup (i.e. did not try to sent an
                // invite and we did not receive one.  This is a bad call
                setState(CONNECTION_FAILED, CONNECTION_REMOTE, CONNECTION_CAUSE_DEST_NOT_OBTAINABLE);
                fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_BADADDRESS) ;
            }
            // We only care about INVITE.
            // BYE, CANCLE and ACK are someone else's problem.
            // REGISTER and OPTIONS are handled else where
            else if(sipMsg->isSameMessage(mInviteMsg) &&
                getState() == CONNECTION_OFFERING)
            {
#ifdef TEST_PRINT
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                              "SipConnection::processMessage "
                              "No response to INVITE");
#endif
                setState(CONNECTION_FAILED, CONNECTION_REMOTE, CONNECTION_CAUSE_DEST_NOT_OBTAINABLE);
                fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_BADADDRESS) ;

#ifdef TEST_PRINT
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                              "SipConnection::processMessage "
                              "originalConnectionAddress: %s "
                              "connection state: CONNECTION_FAILED transport failed",
                              mOriginalCallConnectionAddress.data());
#endif

                // If this was a failed transfer INVITE, send back a status
                if(!mOriginalCallConnectionAddress.isNull())
                {
                    UtlString originalCallId;
                    mpCall->getIdOfOrigCall(originalCallId);
                    CpMultiStringMessage transfereeStatus(CallManager::CP_TRANSFEREE_CONNECTION_STATUS,
                        originalCallId.data(),
                        mOriginalCallConnectionAddress.data(),
                        NULL, NULL, NULL,
                        CONNECTION_FAILED, SIP_REQUEST_TIMEOUT_CODE);
#ifdef TEST_PRINT
                    OsSysLog::add(FAC_CP, PRI_DEBUG,
                                  "SipConnection::processMessage "
                                  "posting CP_TRANSFEREE_CONNECTION_STATUS to call: %s",
                        originalCallId.data());
#endif
                    mpCallManager->postMessage(transfereeStatus);

                }

            }

            // We did not get a response to the session timer
            // re-invite, so terminate the connection
            else if(sipMsg->isSameMessage(mInviteMsg) &&
                getState() == CONNECTION_ESTABLISHED &&
                reinviteState == REINVITING &&
                mSessionReinviteTimer > 0)
            {
#ifdef TEST_PRINT
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                              "SipConnection::processMessage "
                              "failed session timer request\n");
#endif
                hangUp();
            }

            // A BYE or CANCEL failed to get sent
            else if(!messageIsResponse &&
                (method.compareTo(SIP_BYE_METHOD) == 0 ||
                method.compareTo(SIP_CANCEL_METHOD) == 0))
            {
#ifdef TEST_PRINT
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                              "SipConnection::processMessage "
                              "failed BYE");
#endif
                setState(CONNECTION_DISCONNECTED, CONNECTION_REMOTE, CONNECTION_CAUSE_DEST_NOT_OBTAINABLE);
                fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NETWORK) ;
            }
            else
            {
                if (reinviteState == REINVITING)
                    reinviteState = ACCEPT_INVITE;
#ifdef TEST_PRINT
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                              "SipConnection::processMessage "
                              "unhandled failed message");
#endif
            }

            processedOk = TRUE;
        }   // end Transport error

        // Session timer is about to expire send a Re-INVITE to
        // keep the session going
        else if(messageType == SipMessageEvent::SESSION_REINVITE_TIMER)
        {
            extendSessionReinvite();
        }

        // The response was blocked by the user agent authentication
        // This message is only to keep in sync. with the sequence number
        else if(messageType == SipMessageEvent::AUTHENTICATION_RETRY)
        {
            lastLocalSequenceNumber++;

            if(sipMsg->isResponse())
            {
                // If this was the INVITE we need to update the
                // cached invite so that its cseq is up to date
                if(mInviteMsg && sipMsg->isResponseTo(mInviteMsg))
                {
                    mInviteMsg->setCSeqField(lastLocalSequenceNumber,
                        SIP_INVITE_METHOD);

                    //This was moved to SipUserAgent:
                    // Need to send an ACK to finish transaction
                    //SipMessage* ackMessage = new SipMessage();
                    //ackMessage->setAckData(mInviteMsg);
                    //send(ackMessage);
                }
#ifdef TEST_PRINT
                else
                {
                    OsSysLog::add(FAC_CP, PRI_DEBUG,
                                  "SipConnection::processMessage "
                                  "Authentication failure does not match last invite");
                }

                OsSysLog::add(FAC_CP, PRI_DEBUG,
                              "SipConnection::processMessage "
                              "incrementing lastSequeneceNumber");
#endif
                // If this was the INVITE we need to update the
                // cached invite so that its cseq is up to date

            }
#ifdef TEST_PRINT
            else
            {
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                              "SipConnection::processMessage "
                              "request with AUTHENTICATION_RETRY");
            }
#endif
        }   // end authentication retry


        else if(sipMsg->isResponse())
        {
#ifdef TEST_PRINT
            ((SipMessage*)sipMsg)->logTimeEvent("PROCESSING");
#endif
            processedOk = processResponse(sipMsg, callInFocus, onHook);
            //numCodecs, rtpCodecs);
        }
        else
        {
#ifdef TEST_PRINT
            ((SipMessage*)sipMsg)->logTimeEvent("PROCESSING");
#endif
            processedOk = processRequest(sipMsg, callInFocus, onHook);
            //numCodecs, rtpCodecs);
        }

#ifdef TEST_PRINT
        sipMsg->dumpTimeLog();
#endif

    }
    else
    {
        processedOk = FALSE;
    }

    return(processedOk);
}

UtlBoolean SipConnection::extendSessionReinvite()
{
    UtlBoolean messageSent = FALSE;
    if(inviteFromThisSide && mSessionReinviteTimer > 0 &&
        mInviteMsg && getState() == CONNECTION_ESTABLISHED)
    {
        SipMessage reinvite(*mInviteMsg);

        // Up the sequence number and resend
        lastLocalSequenceNumber++;
        reinvite.setCSeqField(lastLocalSequenceNumber, SIP_INVITE_METHOD);

        // Reset the transport states
        reinvite.resetTransport();
        reinvite.removeTopVia();

        //remove all routes
        UtlString route;
        while ( reinvite.removeRouteUri(0 , &route)){}

        if ( !mRouteField.isNull())
        {
            //set correct route
            reinvite.setRouteField(mRouteField);
        }

        messageSent = send(reinvite);
        delete mInviteMsg;
        mInviteMsg = new SipMessage(reinvite);

        // Disallow the other side from ReINVITing until this
        // transaction is complete.
        if(messageSent)
            reinviteState = REINVITING;
#ifdef TEST_PRINT
        osPrintf("Session Timer ReINVITE reinviteState: %d\n",
            reinviteState);
#endif
    }

    // A stray timer expired and the call does not exist.
    else if(mInviteMsg == NULL &&
        getState() == CONNECTION_IDLE)
    {
        setState(CONNECTION_FAILED, CONNECTION_REMOTE);
        fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_RESOURCES) ;
    }

    return(messageSent);
}

UtlBoolean SipConnection::processRequest(const SipMessage* request,
                                         UtlBoolean callInFocus,
                                         UtlBoolean onHook)
{
    UtlString sipMethod;
    UtlBoolean processedOk = TRUE;
    request->getRequestMethod(&sipMethod);

    UtlString name = mpCall->getName();
#ifdef TEST_PRINT
    int requestSequenceNum = 0;
    UtlString requestSeqMethod;
    request->getCSeqField(&requestSequenceNum, &requestSeqMethod);

    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "SipConnection::processRequest "
                  "mInviteMsg: %p requestSequenceNum: %d "
                  "lastRemoteSequenceNumber: %d "
                  "connectionState: %d reinviteState: %d",
                  mInviteMsg, requestSequenceNum,
                  lastRemoteSequenceNumber,
                  getState(), reinviteState);
#endif

    // INVITE
    // We are being hailed
    if(strcmp(sipMethod.data(),SIP_INVITE_METHOD) == 0)
    {
#ifdef TEST_PRINT
        osPrintf("%s in INVITE case\n", name.data());
#endif
        processInviteRequest(request);
    }

    // SIP REFER received (transfer)
    else if(strcmp(sipMethod.data(),SIP_REFER_METHOD) == 0)
    {
#ifdef TEST_PRINT
        osPrintf("SIP REFER method received\n");
#endif
        processReferRequest(request);
    }

    // SIP ACK received
    else if(strcmp(sipMethod.data(),SIP_ACK_METHOD) == 0)
    {
#ifdef TEST_PRINT
        osPrintf("%s SIP ACK method received\n", name.data());
#endif
        processAckRequest(request);
    }

    // BYE
    // The call is being shutdown
    else if(strcmp(sipMethod.data(), SIP_BYE_METHOD)  == 0)
    {
#ifdef TEST_PRINT
        osPrintf("%s %s method received to close down call\n",
            name.data(), sipMethod.data());
#endif
        processByeRequest(request);
    }

    // CANCEL
    // The call is being shutdown
    else if(strcmp(sipMethod.data(), SIP_CANCEL_METHOD) == 0)
    {
#ifdef TEST_PRINT
        osPrintf("%s %s method received to close down call\n",
            name.data(), sipMethod.data());
#endif
        processCancelRequest(request);
    }

    // NOTIFY
    else if(strcmp(sipMethod.data(), SIP_NOTIFY_METHOD) == 0)
    {
#ifdef TEST_PRINT
        osPrintf("%s method received\n",
            sipMethod.data());
#endif
        processNotifyRequest(request);
    }

    else
    {
#ifdef TEST_PRINT
        osPrintf("SipConnection::processRequest %s method NOT HANDLED\n",
            sipMethod.data());
#endif
    }

    return(processedOk);
}

void SipConnection::processInviteRequest(const SipMessage* request)
{
#ifdef TEST_PRINT
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "Entering "
                 "SipConnection::processInviteRequest "
                 "mInviteMsg=%p ",
                 mInviteMsg);
#endif

    UtlString sipMethod;
    request->getRequestMethod(&sipMethod);
    UtlString callId;
    //UtlBoolean receiveCodecSet;
    //UtlBoolean sendCodecSet;
    int requestSequenceNum = 0;
    UtlString requestSeqMethod;
    int tagNum = -1;

    setLocalAddress(request->getInterfaceIp().data());
    request->getCSeqField(&requestSequenceNum, &requestSeqMethod);

    // Create a media connection if one does not yet exist
    if(mConnectionId < 0 && mpMediaInterface != NULL)
    {
        // Create a new connection in the flow graph
        mpMediaInterface->createConnection(mConnectionId,
                                           request->getInterfaceIp().data(),
                                           NULL /* VIDEO: WINDOW HANDLE */);
    }

    // It is safer to always set the To field tag regardless
    // of whether there are more than 1 via's
    // If there are more than 1 via's we are suppose to put
    // a tag in the to field
    // UtlString via;
    //if(request->getViaField(&via, 1))
    {
        UtlString toAddr;
        UtlString toProto;
        int toPort;
        UtlString tag;

        request->getToAddress(&toAddr, &toPort, &toProto, NULL,
            NULL, &tag);
        // The tag is not set, add it
        if(tag.isNull())
        {
            tagNum = rand();
        }
    }

    // Replaces is independent of REFER so
    // do not assume there must be a Refer-To or Refer-By
    UtlBoolean hasReplaceHeader = FALSE;
    // in use, "doesReplaceCallLegExist == TRUE" also means that a Replaces hdr was found
    UtlBoolean doesReplaceCallLegExist = FALSE;
    int       replaceCallLegState = -1 ;
    UtlString replaceCallId;
    UtlString replaceToTag;
    UtlString replaceFromTag;
    hasReplaceHeader = request->getReplacesData(replaceCallId,
                                                replaceToTag,
                                                replaceFromTag);
#ifdef TEST_PRINT
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
        "SipConnection::processInviteRequest "
        "hasReplaceHeader %d replaceCallId %s replaceToTag %s replaceFromTag %s",
        hasReplaceHeader,
        replaceCallId.data(),
        replaceToTag.data(),
        replaceFromTag.data());
#endif
    if (hasReplaceHeader)
    {
        // Ugly assumption that this is a CpPeerCall
        doesReplaceCallLegExist =
            ((CpPeerCall*)mpCall)->getConnectionState(replaceCallId.data(),
                                                      replaceToTag.data(),
                                                      replaceFromTag.data(),
                                                      replaceCallLegState,
                                                      TRUE);
    }

    // Check if this is a valid new invite...
    // If this is previous to the last invite, send error response
    if(mInviteMsg && requestSequenceNum < lastRemoteSequenceNumber)
    {
        SipMessage sipResponse;
        sipResponse.setBadTransactionData(request);
        if(tagNum >= 0)
        {
            sipResponse.setToFieldTag(tagNum);
        }
        send(sipResponse);
    }

    // if this is the same invite.....
    else if(mInviteMsg
            && !inviteFromThisSide
            && requestSequenceNum == lastRemoteSequenceNumber)
    {
        UtlString viaField;
        mInviteMsg->getViaField(&viaField, 0);
        UtlString oldInviteBranchId;
        SipMessage::getViaTag(viaField.data(), "branch", oldInviteBranchId);
        request->getViaField(&viaField, 0);
        UtlString newInviteBranchId;
        SipMessage::getViaTag(viaField.data(), "branch", newInviteBranchId);

        // from a different branch, send loop detected response
        if(!oldInviteBranchId.isNull()
           && oldInviteBranchId.compareTo(newInviteBranchId) != 0)
        {
            SipMessage sipResponse;
            sipResponse.setLoopDetectedData(request);
            if(tagNum >= 0)
            {
                sipResponse.setToFieldTag(tagNum);
            }
            send(sipResponse);
        }
        else // same branch, ignore
        {
            // no-op, ignore duplicate INVITE
            OsSysLog::add(FAC_SIP, PRI_WARNING,
                          "SipConnection::processInviteRequest "
                          "received duplicate request");
        }
    }       // end duplicate invite
    // from here on, message Cseq > this object's lastRemoteSequenceNumber
    else if (hasReplaceHeader && !doesReplaceCallLegExist)
    {
        // has replace header, but it does not match this call, so send 481.
        SipMessage badTransaction;
        badTransaction.setBadTransactionData(request);
        send(badTransaction);

        // Bug 3658: as transfer target, call waiting disabled, transferee comes via proxy
        // manager, sequence of messages is 2 INVITEs from proxy, 486 then this 481.
        // When going from IDLE to DISCONNECTED, set the dropping flag so call will get
        // cleaned up
        mpCall->setDropState(TRUE);
        setState(CONNECTION_DISCONNECTED, CONNECTION_REMOTE);
        setState(CONNECTION_DISCONNECTED, CONNECTION_LOCAL);
        fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_UNKNOWN) ;
#ifdef TEST_PRINT
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipConnection::processInviteRequest "
                      "CONNECTION_DISCONNECTED, replace call leg does not match\n");
#endif
    }
    // Proceed to offering state
    else if((getState() == CONNECTION_IDLE  // New call
             && mConnectionId > -1          // Media resources available
             && (   mLineAvailableBehavior == RING          // really not busy
                 || mLineAvailableBehavior == RING_SILENT   // pretend to ring
                 || mLineAvailableBehavior == FORWARD_ON_NO_ANSWER
                 || mLineAvailableBehavior == FORWARD_UNCONDITIONAL
                 || mLineBusyBehavior == FAKE_RING))        // pretend not busy
            // I do not remember what the follow case is for:
            || (getState() == CONNECTION_OFFERING           // not call to self
                && mRemoteIsCallee
                && !request->isSameMessage(mInviteMsg)))
    {
        lastRemoteSequenceNumber = requestSequenceNum;

        //set the allows field
        if(mAllowedRemote.isNull())
        {
            // Get the methods the other side supports
            request->getAllowField(mAllowedRemote);
        }

        // This should be the first SIP message
        // Set the connection's callId
        getCallId(&callId);
        if(callId.isNull())
        {
            request->getCallIdField(&callId);
            setCallId(callId.data());
        }

        // Save a copy of the INVITE
        mInviteMsg = new SipMessage(*request);

        UtlString requestString;
        mInviteMsg->getRequestUri(&requestString);
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "SipConnection::processInviteRequest - "
                      "mInviteMsg request URI '%s'",
                      requestString.data());

        inviteFromThisSide = FALSE;
        setCallerId();
        //save line Id
        {
           // Get the request URI, which is in addr-spec format.
           UtlString uri;
           request->getRequestUri(&uri);
           // Convert the URI to name-addr format.
           Url parsedUri(uri, TRUE);
           // Store into mLocalContact, which is in name-addr format.
           parsedUri.toString(mLocalContact);
           OsSysLog::add(FAC_CP, PRI_DEBUG,
                         "SipConnection::processInviteRequest - "
                         "parsedURI to string '%s'",
                         mLocalContact.data());
        }

        int cause = CONNECTION_CAUSE_NORMAL;

        // Replaces is independent of REFER so
        // do not assume there must be a Refer-To or Refer-By
        // Assume the replaces call leg does not exist if the call left
        // state state is not established.  The latest RFC states that
        // one cannot use replaces for an early dialog
        if (doesReplaceCallLegExist
            && (replaceCallLegState != CONNECTION_ESTABLISHED))
        {
            doesReplaceCallLegExist = FALSE ;
        }

        // Allow transfer if the call leg exists and the call leg is
        // established.  Transferring a call while in early dialog is
        // illegal.
        // At this point, a TRUE value means
        //  - have replaces header
        //  - found a valid call leg to match replaces id
        //  - the call leg to be replaced is "ESTABLISHED", so ok to do transfer
        if (doesReplaceCallLegExist)
        {
            cause = CONNECTION_CAUSE_TRANSFER;

            // Setup the meta event data
            int metaEventId = mpCallManager->getNewMetaEventId();
            const char* metaEventCallIds[2];
            metaEventCallIds[0] = callId.data();        // target call Id
            metaEventCallIds[1] = replaceCallId.data(); // original call Id
            mpCall->startMetaEvent(metaEventId,
                                   PtEvent::META_CALL_REPLACING,
                                   2, metaEventCallIds);
            mpCall->setCallType(CpCall::CP_TRANSFER_TARGET_TARGET_CALL);

            fireSipXEvent(CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_TRANSFERRED, (void*) replaceCallId.data()) ;

#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG,
                          "SipConnection::processInviteRequest - "
                          "replaceCallId: %s, toTag: %s, fromTag: %s",
                          replaceCallId.data(),
                          replaceToTag.data(),
                          replaceFromTag.data());
#endif
        }
        else
        {
            // This call does not contain a REPLACES header, however, it
            // may still be part of a blind transfer, so look for the
            // referred-by or requested-by headers
            UtlString referredBy;
            UtlString requestedBy;
            request->getReferredByField(referredBy);
            request->getRequestedByField(requestedBy);
            if (!referredBy.isNull() || !requestedBy.isNull())
            {
                cause = CONNECTION_CAUSE_TRANSFER;
                mpCall->setCallType(CpCall::CP_TRANSFER_TARGET_TARGET_CALL);
            }

            mpCall->startMetaEvent( mpCallManager->getNewMetaEventId(),
                                    PtEvent::META_CALL_STARTING,
                                    0, 0, mRemoteIsCallee);

            if (!mRemoteIsCallee)   // inbound call
            {
                mpCall->setCallState(mResponseCode, mResponseText, PtCall::ACTIVE);
                setState(CONNECTION_ESTABLISHED, CONNECTION_REMOTE, PtEvent::CAUSE_NEW_CALL);
                setState(CONNECTION_INITIATED, CONNECTION_LOCAL, PtEvent::CAUSE_NEW_CALL);
                fireSipXEvent(CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL) ;
            }
        }

        // If this is not part of a call leg replaces operation
        // we normally go to offering so that the application
        // can decide to accept, reject or redirect
        if(!doesReplaceCallLegExist)
        {
            setState(CONNECTION_OFFERING, CONNECTION_LOCAL, cause);
            fireSipXEvent(CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE) ;
        }

        // Always get the remote contact as it may can change over time
        UtlString contactInResponse;
        if (request->getContactEntry(0 , &contactInResponse))
        {
            Url temp(contactInResponse) ;
            temp.removeHeaderParameters() ;
            temp.removeFieldParameters() ;
            temp.includeAngleBrackets() ;
            temp.toString(mRemoteContact) ;
            temp.getUri(mRemoteContactUri) ;
        }

        // Get the route for subsequent requests
        request->buildRouteField(&mRouteField);
#ifdef TEST_PRINT
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipConnection::processInviteRequest "
                      "mset mRouteField: %s\n",
                      mRouteField.data());
#endif
        // Set the to tag if it is not set in the Invite
        if(tagNum >= 0)
        {
            mInviteMsg->setToFieldTag(tagNum);

            // Update the cached from field after saving the tag
            mInviteMsg->getToUrl(mFromUrl);
        }
#ifdef TEST_PRINT
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipConnection::processInviteRequest "
                      "Offering delay: %d\n",
                      mOfferingDelay);
#endif
        // If we are replacing a call let's answer the call
        // immediately do not go to offering first.
        if (doesReplaceCallLegExist)
        {
            // When the replaceCallLeg exists, we need to update
            // mpCall's callId with the new one as the SipSession for
            // the new call leg and the call leg being replaced share
            // the same instance of mpCall -- if callId is not
            // updated, the call will be linked to the dying leg.
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipConnection::processInviteRequest "
                          "mInviteMsg=%p",
                          mInviteMsg);

            mpCall->setCallId(callId.data());
            // Go immediately to answer the call
            answer();

            // Bob 11/16/01: The following setState was added to close a race between
            // the answer (above) and hangup (below).  The application layer is notified
            // of state changed on the replies to these messages.  These can lead to
            // dropped transfer if the BYE reponse is received before INVITE response.
            setState(CONNECTION_ESTABLISHED, CONNECTION_REMOTE, CONNECTION_CAUSE_TRANSFER);
            if (mTerminalConnState == PtTerminalConnection::HELD)
            {
                fireSipXEvent(CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD);
            }
            else
            {
                fireSipXEvent(CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE);
            }

            // Drop the leg to be replaced
            ((CpPeerCall*)mpCall)->hangUp(replaceCallId.data(),
                                          replaceToTag.data(),
                                          replaceFromTag.data());
        }
        else if(mOfferingDelay == IMMEDIATE)
        {
            accept(mForwardOnNoAnswerSeconds);
        }
        // The connection stays in Offering state until timeout
        // or told to do otherwise
        else
        {
            // Should send a Trying response
            //SipMessage offeringTryingMessage;
            //offeringTryingMessage.setTryingResponseData(mInviteMsg);
            //send(offeringTryingMessage);

            // If the delay is not forever, setup a timer to expire
            if(mOfferingDelay > IMMEDIATE)
            {
                setOfferingTimer(mOfferingDelay);
            }
            // Other wise do nothing, let it sit in offering forever
        }
    }   // end handling new Invite
    // Re-INVITE allowed
    else if(   mpMediaInterface != NULL
            && mInviteMsg
            && requestSequenceNum > lastRemoteSequenceNumber
            &&getState() == CONNECTION_ESTABLISHED
            &&reinviteState == ACCEPT_INVITE)
    {
        // Keep track of the last sequence number;
        lastRemoteSequenceNumber = requestSequenceNum;


        // Always get the remote contact as it may can change over time
        UtlString contactInResponse;
        if (request->getContactEntry(0 , &contactInResponse))
        {
            Url temp(contactInResponse) ;
            temp.removeHeaderParameters() ;
            temp.removeFieldParameters() ;
            temp.includeAngleBrackets() ;
            temp.toString(mRemoteContact) ;
            temp.getUri(mRemoteContactUri) ;
        }

        // The route set is set only on the initial dialog transaction
        //request->buildRouteField(&mRouteField);
        //osPrintf("reINVITE set mRouteField: %s\n", mRouteField.data());

        // Do not allow other Requests until the ReINVITE is complete
        reinviteState = REINVITED;
#ifdef TEST_PRINT
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipConnection::processInviteRequest "
                      "reinviteState: %d\n",
                      reinviteState);
#endif

        UtlString rtpAddress;
        int receiveRtpPort;
        int receiveRtcpPort;
        int receiveVideoRtpPort;
        int receiveVideoRtcpPort;
        SdpCodecFactory supportedCodecs;
        SdpSrtpParameters srtpParams;
        mpMediaInterface->getCapabilities(mConnectionId,
            rtpAddress,
            receiveRtpPort,
            receiveRtcpPort,
            receiveVideoRtpPort,    // VIDEO: TODO
            receiveVideoRtcpPort,
            supportedCodecs,
            srtpParams);

        int numMatchingCodecs = 0;
        SdpCodec** matchingCodecs = NULL;
        SdpDirectionality directionality;
        // Get the RTP info from the message if present
        // Should check the content type first
        if(getInitialSdpCodecs(request,
            supportedCodecs, numMatchingCodecs, matchingCodecs,
            remoteRtpAddress, remoteRtpPort, remoteRtcpPort, &directionality))
        {
            // If the codecs match send an OK
            if(numMatchingCodecs > 0)
            {
                // Setup media channel
                mpMediaInterface->setConnectionDestination(mConnectionId,
                    remoteRtpAddress.data(),
                    remoteRtpPort,
                    remoteRtcpPort,
                    receiveVideoRtpPort,
                    receiveVideoRtcpPort);
#ifdef TEST_PRINT
                osPrintf("ReINVITE RTP SENDING address: %s port: %d\n", remoteRtpAddress.data(), remoteRtpPort);
#endif

                // Far side requested hold

                if(remoteRtpPort == 0 ||
                    remoteRtpAddress.compareTo("0.0.0.0") == 0 ||
                    directionality == sdpDirectionalitySendOnly ||
                    directionality == sdpDirectionalityInactive)
                {
                    //receiveRtpPort = 0;
                    // Leave the receive on to drain the buffers
                    mpMediaInterface->stopRtpReceive(mConnectionId);
                    mpMediaInterface->stopRtpSend(mConnectionId);
                    fireSipXEvent(CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP);
                    if (mTerminalConnState == PtTerminalConnection::HELD)
                    {
                        fireSipXEvent(CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_INACTIVE);
                    }
                    else
                    {
                        fireSipXEvent(CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD);
                    }

                    mRemoteRequestedHold = TRUE;
                }
                else if(remoteRtpPort > 0
                       && remoteRtpAddress.compareTo("0.0.0.0") != 0
                       &&   (directionality == sdpDirectionalitySendRecv
                         ||  directionality == sdpDirectionalityRecvOnly))
                {
                    mpMediaInterface->startRtpReceive(mConnectionId,
                        numMatchingCodecs, matchingCodecs, srtpParams);

                    mpMediaInterface->startRtpSend(mConnectionId,
                        numMatchingCodecs, matchingCodecs, srtpParams);

                    // Fire a CALLSTATE_CONNECTED_ACTIVE event for remote side taking
                    // us off hold
                    if (mTerminalConnState == PtTerminalConnection::HELD)
                    {
                        fireSipXEvent(CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD);
                    }
                    else
                    {
                        fireSipXEvent(CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE);
                    }

                    // If sipX TAPI, fire audio start event
                    UtlString audioCodecName;
                    UtlString videoCodecName;
                    SIPX_CODEC_INFO tapiCodec;
                    if (mpMediaInterface->getPrimaryCodec(mConnectionId,
                                                            audioCodecName,
                                                            videoCodecName,
                                                            &tapiCodec.audioCodec.iPayloadType,
                                                            &tapiCodec.videoCodec.iPayloadType) == OS_SUCCESS)
                    {
                        strncpy(tapiCodec.audioCodec.cName, audioCodecName.data(), SIPXTAPI_CODEC_NAMELEN-1);
                        strncpy(tapiCodec.videoCodec.cName, videoCodecName.data(), SIPXTAPI_CODEC_NAMELEN-1);
                        fireSipXEvent(CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, &tapiCodec) ;
                    }
                }

                // Send a INVITE OK response
#ifdef TEST_PRINT
                osPrintf("Sending INVITE OK\n");
#endif

                if(remoteRtpPort <= 0 ||
                    remoteRtpAddress.compareTo("0.0.0.0") == 0)
                {
                    rtpAddress.remove(0);
                    rtpAddress.append("0.0.0.0");  // hold address
                }
#ifdef TEST_PRINT
                osPrintf("REINVITE: using RTP address: %s\n",
                    rtpAddress.data());
#endif

                SipMessage sipResponse;
                sipResponse.setInviteOkData(request, rtpAddress.data(),
                    receiveRtpPort, receiveRtcpPort,
                    receiveVideoRtpPort,
                    receiveVideoRtcpPort,
                    numMatchingCodecs,
                    matchingCodecs,
                    srtpParams, mDefaultSessionReinviteTimer,
                    mLocalContact.data());
                if(tagNum >= 0)
                {
                    sipResponse.setToFieldTag(tagNum);
                }
                send(sipResponse);
                mPrevSdp = (sipResponse.getSdpBody())->copy();

                // Save the invite for future reference
                if(mInviteMsg)
                {
                    delete mInviteMsg;
                }
                mInviteMsg = new SipMessage(*request);
                inviteFromThisSide = FALSE;
                setCallerId();


                if(tagNum >= 0)
                {
                    mInviteMsg->setToFieldTag(tagNum);

                    // Update the cached from field after saving the tag
                    mInviteMsg->getToUrl(mFromUrl);
                }

            }
            // If the codecs do not match send an error
            else
            {
                // No common codecs send INVITE error response
                SipMessage sipResponse;
                sipResponse.setInviteBadCodecs(request, sipUserAgent);
                if(tagNum >= 0)
                {
                    sipResponse.setToFieldTag(tagNum);
                }
                send(sipResponse);
            }
        }
        else
        {
#ifdef TEST_PRINT
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipConnection::processInviteRequest "
                          "No SDP in reINVITE "
                          "REINVITE: using RTP address: %s\n",
                rtpAddress.data());
#endif

            // Use the full set of capabilities as the other
            // side did not give SDP to find common/matching
            // codecs
            supportedCodecs.getCodecs(numMatchingCodecs,
                matchingCodecs);

            SipMessage sipResponse;
            sipResponse.setInviteOkData(request, rtpAddress.data(),
                receiveRtpPort, receiveRtcpPort,
                receiveVideoRtpPort,
                receiveVideoRtcpPort,
                numMatchingCodecs, matchingCodecs,
                srtpParams,
                mDefaultSessionReinviteTimer, mLocalContact.data());
            if(tagNum >= 0)
            {
                sipResponse.setToFieldTag(tagNum);
            }
            send(sipResponse);

            // Save the invite for future reference
            if(mInviteMsg)
            {
                delete mInviteMsg;
            }
            mInviteMsg = new SipMessage(*request);
            inviteFromThisSide = FALSE;
            setCallerId();

            if(tagNum >= 0)
            {
                mInviteMsg->setToFieldTag(tagNum);

                // Update the cached from field after saving the tag
                mInviteMsg->getToUrl(mFromUrl);
            }
        }

        // Free up the codec copies and array
        for(int codecIndex = 0; codecIndex < numMatchingCodecs; codecIndex++)
        {
            delete matchingCodecs[codecIndex];
            matchingCodecs[codecIndex] = NULL;
        }
        delete[] matchingCodecs;
        matchingCodecs = NULL;
    }   // end handling reInvite

    // Busy, but queue call
    else if(mpMediaInterface != NULL && getState() == CONNECTION_IDLE &&
        ((mLineBusyBehavior == QUEUE_SILENT) ||
        (mLineBusyBehavior == QUEUE_ALERT) ||
        (mLineBusyBehavior == BUSY &&
        request->isRequestDispositionSet(SIP_DISPOSITION_QUEUE))))
    {
        lastRemoteSequenceNumber = requestSequenceNum;

#ifdef TEST_PRINT
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipConnection::processInviteRequest "
                      "Busy, queuing call");
#endif

        // This should be the first SIP message
        // Set the connection's callId
        getCallId(&callId);
        if(callId.isNull())
        {
            request->getCallIdField(&callId);
            setCallId(callId.data());
        }

        // Always get the remote contact as it may can change over time
        UtlString contactInResponse;
        if (request->getContactEntry(0 , &contactInResponse))
        {
            Url temp(contactInResponse) ;
            temp.removeHeaderParameters() ;
            temp.removeFieldParameters() ;
            temp.includeAngleBrackets() ;
            temp.toString(mRemoteContact) ;
            temp.getUri(mRemoteContactUri) ;
        }

        // Get the route for subsequent requests
        request->buildRouteField(&mRouteField);
#ifdef TEST_PRINT
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipConnection::processInviteRequest "
                      "queuedINVITE set mRouteField: %s",
                      mRouteField.data());
#endif

        // Get the capabilities to figure out the matching codecs
        UtlString rtpAddress;
        int receiveRtpPort;
        int receiveRtcpPort;
        int receiveVideoRtpPort;
        int receiveVideoRtcpPort;

        SdpCodecFactory supportedCodecs;
        SdpSrtpParameters srtpParams;
        mpMediaInterface->getCapabilities(mConnectionId,
            rtpAddress,
            receiveRtpPort,
            receiveRtcpPort,
            receiveVideoRtpPort,        // VIDEO: TODO
            receiveVideoRtcpPort,
            supportedCodecs,
            srtpParams);

        // Get the codecs
        int numMatchingCodecs = 0;
        SdpCodec** matchingCodecs = NULL;
        SdpDirectionality directionality; 	// result not yet used
        getInitialSdpCodecs(request,
            supportedCodecs,
            numMatchingCodecs, matchingCodecs,
            remoteRtpAddress, remoteRtpPort, remoteRtcpPort, &directionality);

        // We are not suppose to go into offering state before, after
        // or at all when queuing a call.


        // Save a copy of the invite for future reference
        mInviteMsg = new SipMessage(*request);
        inviteFromThisSide = FALSE;
        setCallerId();

        // Create and send a queued response
        SipMessage sipResponse;
        sipResponse.setQueuedResponseData(request);
        if(tagNum >= 0)
        {
            sipResponse.setToFieldTag(tagNum);
        }
        send(sipResponse);
        mPrevSdp = (sipResponse.getSdpBody())->copy();
        setState(CONNECTION_QUEUED, CONNECTION_LOCAL);
        /** SIPXTAPI: TBD **/

        // Free up the codec copies and array
        for(int codecIndex = 0; codecIndex < numMatchingCodecs; codecIndex++)
        {
            delete matchingCodecs[codecIndex];
            matchingCodecs[codecIndex] = NULL;
        }
        delete[] matchingCodecs;
        matchingCodecs = NULL;
    }   // end queuing new call on busy
    // Forward on busy
    else if(getState() == CONNECTION_IDLE &&
        /*!callInFocus && */ mLineBusyBehavior == FORWARD_ON_BUSY &&
        !mForwardOnBusy.isNull())
    {
#ifdef TEST_PRINT
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipConnection::processInviteRequest "
                      "Busy, forwarding call to \"%s\"",
            mForwardOnBusy.data());
#endif

        SipMessage sipResponse;
        sipResponse.setForwardResponseData(request,
            mForwardOnBusy.data());
        if(tagNum >= 0)
        {
            sipResponse.setToFieldTag(tagNum);
        }
        send(sipResponse);

#ifdef TEST_PRINT
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipConnection::processInviteRequest "
                      " - CONNECTION_FAILED, cause BUSY : 2390");
#endif
        setState(CONNECTION_FAILED, CONNECTION_LOCAL, CONNECTION_CAUSE_BUSY);
        fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_BUSY) ;
    }   // end forwarding call on busy
    // If busy
    else
    {
#ifdef TEST_PRINT
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipConnection::processInviteRequest "
                      "busy returning response "
                      "Busy behavior: %d infocus: %d forward on busy URL: %s",
            mLineBusyBehavior, -5/*callInFocus*/, mForwardOnBusy.data());
#endif
        // This should be the first SIP message
        // Set the connection's callId
        getCallId(&callId);
        if(callId.isNull())
        {
            request->getCallIdField(&callId);
            setCallId(callId.data());
        }

        // Send back a busy INVITE response
        SipMessage sipResponse;
        sipResponse.setInviteBusyData(request);
        if(tagNum >= 0)
        {
            sipResponse.setToFieldTag(tagNum);
        }
        send(sipResponse);

        // Special case:
        // We sent the invite to our self
        if(mInviteMsg && request->isSameMessage(mInviteMsg))
        {
            // Do not set state here it will be done when
            // the response comes back
        }
        else
        {
            if (reinviteState != REINVITING)
            {
#ifdef TEST_PRINT
                OsSysLog::add(FAC_SIP, PRI_DEBUG,
                              "SipConnection::processInviteRequest "
                              "- busy, not HELD state, setting state to CONNECTION_FAILED\n");
#endif
                setState(CONNECTION_FAILED, CONNECTION_LOCAL, CONNECTION_CAUSE_BUSY);
                fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_BUSY) ;
            }
        }
    }

#ifdef TEST_PRINT
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
        "Leaving "
        "SipConnection::processInviteRequest "
        "mInviteMsg=%p ",
        mInviteMsg);
#endif
} // End of processInviteRequest


void SipConnection::processReferRequest(const SipMessage* request)
{
    mIsAcceptSent = FALSE;

    UtlString referTo;
    UtlString referredBy;
    Url  requestToUrl;
    request->getReferredByField(referredBy);
    request->getReferToField(referTo);
    request->getToUrl(requestToUrl);

    //reject Refers to non sip URLs
    Url referToUrl(referTo);
    UtlString protocol;
    referToUrl.getUrlType(protocol);

    int connectionState = getState();
    // Cannot transfer if there is not already a call set up
    if(connectionState != CONNECTION_ESTABLISHED &&
        connectionState != CONNECTION_IDLE)
    {
        SipMessage sipResponse;
        sipResponse.setReferDeclinedData(request);
        send(sipResponse);
    }

    // If there is not exactly one Refered-By
    // or not exactly one Refer-To header
    // or there is already a REFER in progress
    else if(request->getHeaderValue(1, SIP_REFERRED_BY_FIELD) != NULL ||
        request->getHeaderValue(1, SIP_REFER_TO_FIELD) != NULL ||
        mReferMessage)
    {
        SipMessage sipResponse;
        sipResponse.setRequestBadRequest(request);
        send(sipResponse);
    }

    //if Url is not of type Sip
    else if (protocol.index("SIP" , 0, UtlString::ignoreCase) != 0)
    {
        SipMessage sipResponse;
        sipResponse.setRequestBadUrlType(request);
        send(sipResponse);
    }
    // Give the transfer a try.
    else if(connectionState == CONNECTION_ESTABLISHED)
    {
        // Create a second call if it does not exist already
        // Set the target call id in this call
        // Set this call's type to transferee original call
        UtlString targetCallId;
        Url targetUrl(referTo);
        targetUrl.getHeaderParameter(SIP_CALLID_FIELD, targetCallId);
        // targetUrl.removeHeaderParameters();
        targetUrl.toString(referTo);
        //SipMessage::parseParameterFromUri(referTo.data(), "Call-ID",
        //    &targetCallId);
#ifdef TEST_PRINT
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipConnection::processReferRequest "
                      "REFER refer-to: %s callid: %s\n",
                      referTo.data(), targetCallId.data());
#endif

        // Set up the meta event data
        int metaEventId = mpCallManager->getNewMetaEventId();
        const char* metaEventCallIds[2];
        UtlString thisCallId;
        getCallId(&thisCallId);
        metaEventCallIds[0] = targetCallId.data();
        metaEventCallIds[1] = thisCallId.data();

        // I am not sure we want the focus change to
        // be automatic.  It is here for now to make it work
        CpIntMessage yieldFocus(CallManager::CP_YIELD_FOCUS, (intptr_t)mpCall);
        mpCallManager->postMessage(yieldFocus);

        // The new call by default assumes focus.
        // Mark the new call as part of this transfer meta event
        mpCallManager->createCall(&targetCallId, metaEventId,
                                  PtEvent::META_CALL_TRANSFERRING,
                                  2, metaEventCallIds);
        mpCall->setTargetCallId(targetCallId.data());
        mpCall->setCallType(CpCall::CP_TRANSFEREE_ORIGINAL_CALL);

        // Mark the begining of a transfer meta event in this call
        mpCall->startMetaEvent(metaEventId,
                               PtEvent::META_CALL_TRANSFERRING,
                               2, metaEventCallIds);

        // Set the OutboundLine (From Field) to match the original
        // call, so it looks like this new call comes from the same
        // caller as the REFER request To Field

        // Clear all non-URL parameters
        requestToUrl.removeHeaderParameters();
        requestToUrl.removeFieldParameters();
        mpCallManager->setOutboundLineForCall(targetCallId,
                                              requestToUrl.toString()) ;

        // Send a message to the target call to create the
        // connection and send the INVITE
        UtlString remoteAddress;
        getRemoteAddress(&remoteAddress);
        CpMultiStringMessage transfereeConnect(CallManager::CP_TRANSFEREE_CONNECTION,
                                               targetCallId.data(),
                                               referTo.data(), referredBy.data(),
                                               thisCallId.data(),
                                               remoteAddress.data());
#ifdef TEST_PRINT
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipConnection::processReferRequest "
                      "from ESTAB posting CP_TRANSFEREE_CONNECTION\n");
#endif
        mpCallManager->postMessage(transfereeConnect);

        // Send an accepted response, a NOTIFY is sent later to
        // provide the resulting outcome
        SipMessage sipResponse;
        sipResponse.setResponseData(request, SIP_ACCEPTED_CODE,
                                    SIP_ACCEPTED_TEXT, mLocalContact);
        mIsAcceptSent = send(sipResponse);

        // Save a copy for the NOTIFY
        mReferMessage = new SipMessage(*request);
    }

    else if(connectionState == CONNECTION_IDLE)
    {
        // Set the identity of this connection
        request->getFromUrl(mToUrl);
        request->getToUrl(mFromUrl);
        UtlString callId;
        request->getCallIdField(&callId);
        setCallId(callId);
        UtlString fromField;
        mToUrl.toString(fromField);

        // Post a message to add a connection to this call
        CpMultiStringMessage transfereeConnect(
            CallManager::CP_TRANSFEREE_CONNECTION,
            callId.data(),
            referTo.data(),
            referredBy.data(),
            callId.data(),
            fromField.data());
#ifdef TEST_PRINT
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipConnection::processReferRequest "
                      "from IDLE posting CP_TRANSFEREE_CONNECTION\n");
#endif
        mpCallManager->postMessage(transfereeConnect);

        // Assume focus, probably not the right thing
        //mpCallManager->unhold(callId.data());

        // Send back a response
        SipMessage referResponse;
        referResponse.setResponseData(request, SIP_ACCEPTED_CODE,
            SIP_ACCEPTED_TEXT, mLocalContact);
        mIsAcceptSent = send(referResponse);

        // Save a copy for the NOTIFY
        mReferMessage = new SipMessage(*request);

        setState(CONNECTION_UNKNOWN, CONNECTION_REMOTE);
        /** SIPXTAPI: TBD **/
    }
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "Leaving "
                  "SipConnection::processReferRequest ");
} // end of processReferRequest

void SipConnection::processNotifyRequest(const SipMessage* request)
{
    UtlString eventType;
    request->getEventField(eventType);

    // IF this is a REFER result notification
    ssize_t refIndex = eventType.index(SIP_EVENT_REFER);
    if(refIndex >= 0)
    {
        UtlString contentType;
        request->getContentType(&contentType);
        const HttpBody* body = request->getBody();

        // If we have a body that contains the REFER status/outcome
        if (     body
            && (  contentType.index(CONTENT_TYPE_SIP_APPLICATION, 0, UtlString::ignoreCase) == 0
               || contentType.index(CONTENT_TYPE_MESSAGE_SIPFRAG, 0, UtlString::ignoreCase) == 0))
        {
            // Send a NOTIFY response, we like the content
            // Need to send this ASAP and before the BYE
            SipMessage notifyResponse;
            notifyResponse.setOkResponseData(request, mLocalContact);
            send(notifyResponse);

            // Extract the message body, which is a sipfrag containing the
            // response that the transferee got from the transfer target.
            const char* bytes;
            ssize_t numBytes;
            body->getBytes(&bytes, &numBytes);

            SipMessage response(bytes, numBytes);

            // Extract the response code from the sipfrag.
            int state;
            int cause;
            int responseCode = response.getResponseStatusCode();
            mResponseCode = responseCode;
            response.getResponseStatusText(&mResponseText);
            if (OsSysLog::willLog(FAC_CP, PRI_DEBUG))
            {
               UtlString callId;
               getCallId(&callId);
               OsSysLog::add(FAC_CP, PRI_DEBUG,
                             "SipConnection::processNotifyRequest "
                             "response %d '%s' for call '%s'",
                             responseCode, mResponseText.data(),
                             callId.data());
            }

            // How to respond to this NOTIFY:
            enum {
               PENDING,         // no change in state
               SUCCESS,         // transfer succeeded
               FAILURE          // transfer failed
            } effect;

            // 1xx responses
            if (responseCode == SIP_TRYING_CODE)
            {
               // No sipX event.
               effect = PENDING;
            }
            else if (responseCode == SIP_EARLY_MEDIA_CODE)
            {
                // Note: this is the state for the transferee
                // side of the connection between the transferee
                // and the transfer target (i.e. not the target
                // which is actually ringing)
                state = CONNECTION_ESTABLISHED;
                cause = CONNECTION_CAUSE_UNKNOWN;
                fireSipXEvent(CALLSTATE_TRANSFER, CALLSTATE_TRANSFER_RINGING) ;
                effect = PENDING;
            }
            else if (responseCode == SIP_RINGING_CODE)
            {
                // Note: this is the state for the transferee
                // side of the connection between the transferee
                // and the transfer target (i.e. not the target
                // which is actually ringing)
                state = CONNECTION_OFFERING;
                cause = CONNECTION_CAUSE_NORMAL;
                fireSipXEvent(CALLSTATE_TRANSFER, CALLSTATE_TRANSFER_RINGING) ;
                effect = PENDING;
            }
            else if (   responseCode >= SIP_1XX_CLASS_CODE
                     && responseCode <  SIP_2XX_CLASS_CODE)
            {
               // No sipX event.
               effect = PENDING;
            }
            // 2xx responses
            else if (   responseCode >= SIP_2XX_CLASS_CODE
                     && responseCode <  SIP_3XX_CLASS_CODE)
            {
                state = CONNECTION_ESTABLISHED;
                cause = CONNECTION_CAUSE_NORMAL;
                fireSipXEvent(CALLSTATE_TRANSFER, CALLSTATE_TRANSFER_SUCCESS) ;
                effect = SUCCESS;
            }
            // 3xx, 4xx, 5xx, and 6xx responses.
            // If we see a 3xx response reported by the transferee,
            // then the transferee is considering it a final response
            // (and not recursing on it), and so the transfer
            // operation failed.
            else if (responseCode == SIP_DECLINE_CODE)
            {
                state = CONNECTION_FAILED;
                cause = CONNECTION_CAUSE_CANCELLED;
                fireSipXEvent(CALLSTATE_TRANSFER, CALLSTATE_TRANSFER_FAILURE) ;
                effect = FAILURE;
            }
            else if (   responseCode == SIP_BAD_METHOD_CODE
                     || responseCode == SIP_UNIMPLEMENTED_METHOD_CODE)
            {
                state = CONNECTION_FAILED;
                cause = CONNECTION_CAUSE_INCOMPATIBLE_DESTINATION;
                fireSipXEvent(CALLSTATE_TRANSFER, CALLSTATE_TRANSFER_FAILURE) ;
                effect = FAILURE;
            }
            else if (responseCode == SIP_SERVICE_UNAVAILABLE_CODE)
            {
                state = CONNECTION_FAILED;
                cause = CONNECTION_CAUSE_SERVICE_UNAVAILABLE;
                fireSipXEvent(CALLSTATE_TRANSFER, CALLSTATE_TRANSFER_FAILURE) ;
                effect = FAILURE;
            }
            else if (responseCode == SIP_BUSY_CODE)
            {
                state = CONNECTION_FAILED;
                cause = CONNECTION_CAUSE_BUSY;
                fireSipXEvent(CALLSTATE_TRANSFER, CALLSTATE_TRANSFER_FAILURE) ;
                effect = FAILURE;
            }
            else
            {
                // All other failure final responses.
                state = CONNECTION_FAILED;
                cause = CONNECTION_CAUSE_UNKNOWN;
                fireSipXEvent(CALLSTATE_TRANSFER, CALLSTATE_TRANSFER_FAILURE) ;
                effect = FAILURE;
            }

            switch (effect)
            {
            case PENDING:
               /* Do nothing. */
               OsSysLog::add(FAC_CP, PRI_DEBUG,
                             "SipConnection::processNotifyRequest "
                             "transfer pending");
               break;
            case SUCCESS:
            {
               // Success response.
               // Signal the connection in the target call with the
               // final status.
               UtlString targetCallId;
               UtlString toField;
               mToUrl.toString(toField);
               mpCall->getTargetCallId(targetCallId);
               CpMultiStringMessage transferControllerStatus(
                  CallManager::CP_TRANSFER_CONNECTION_STATUS,
                  targetCallId.data(), toField.data(),
                  NULL, NULL, NULL,
                  state, cause);
               OsSysLog::add(FAC_CP, PRI_DEBUG,
                             "SipConnection::processNotifyRequest "
                             "transfer succeeded, posting "
                             "CP_TRANSFER_CONNECTION_STATUS to call '%s', "
                             "state %d, cause %d, dropping call",
                             targetCallId.data(), state, cause);
               mpCallManager->postMessage(transferControllerStatus);

               // Drop this connection, the transfer succeeded
               // Do the drop at the last possible momment so that
               // both calls have some overlap.
               doHangUp();
            }
            break;
            case FAILURE:
            {
               // Failure response.
               // Signal the connection in the target call with the
               // final status.
               UtlString targetCallId;
               UtlString toField;
               mToUrl.toString(toField);
               mpCall->getTargetCallId(targetCallId);
               CpMultiStringMessage transferControllerStatus(
                  CallManager::CP_TRANSFER_CONNECTION_STATUS,
                  targetCallId.data(), toField.data(),
                  NULL, NULL, NULL,
                  state, cause);
               OsSysLog::add(FAC_CP, PRI_DEBUG,
                             "SipConnection::processNotifyRequest "
                             "transfer failed, posting "
                             "CP_TRANSFER_CONNECTION_STATUS to call '%s', "
                             "state %d, cause %d",
                             targetCallId.data(), state, cause);
               mpCallManager->postMessage(transferControllerStatus);

               // Take the original call off hold, so it can resume.
               UtlString callId;
               getCallId(&callId);
               UtlString remoteAddress;
               getRemoteAddress(&remoteAddress);
               mpCallManager->unholdTerminalConnection(callId.data(),
                                                       remoteAddress.data(),
                                                       NULL);
               mpCallManager->unholdLocalTerminalConnection(callId.data());
            }
            break;
            }
        } // End if body in the NOTIFY

        // Unknown NOTIFY content type for this event type REFER
        else
        {
            // This probably should be some sort of error response
            // Send a NOTIFY response
            SipMessage notifyResponse;
            notifyResponse.setOkResponseData(request, mLocalContact);
            send(notifyResponse);
        }

    } // End REFER NOTIFY
} // End of processNotifyRequest

void SipConnection::processAckRequest(const SipMessage* request)
{
#ifdef TEST_
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "SipConnection::processAckRequest "
                  "entering");
#endif

    //UtlBoolean receiveCodecSet;
    //UtlBoolean sendCodecSet;
    int requestSequenceNum = 0;
    UtlString requestSeqMethod;

    request->getCSeqField(&requestSequenceNum, &requestSeqMethod);

    // If this ACK belongs to the last INVITE and
    // we are accepting the INVITE
    if(mpMediaInterface != NULL
       && getState() == CONNECTION_ESTABLISHED
       && (lastRemoteSequenceNumber == requestSequenceNum
           || mIsAcceptSent))
    {
        UtlString rtpAddress;
        int receiveRtpPort;
        int receiveRtcpPort;
        int receiveVideoRtpPort;
        int receiveVideoRtcpPort;
        SdpCodecFactory supportedCodecs;
        SdpSrtpParameters srtpParams;
        mpMediaInterface->getCapabilities(mConnectionId,
            rtpAddress,
            receiveRtpPort,
            receiveRtcpPort,
            receiveVideoRtpPort,    // VIDEO: TODO
            receiveVideoRtcpPort,
            supportedCodecs,
            srtpParams);

        // If codecs set ACK in SDP
        // If there is an SDP body find the best
        //codecs, address & port
        int numMatchingCodecs = 0;
        SdpCodec** matchingCodecs = NULL;
        SdpDirectionality directionality; 	// result not yet used
        if(getInitialSdpCodecs(request,
            supportedCodecs,
            numMatchingCodecs, matchingCodecs,
            remoteRtpAddress, remoteRtpPort, remoteRtcpPort, &directionality) &&
            numMatchingCodecs > 0)
        {
            // Set up the remote RTP sockets
            mpMediaInterface->setConnectionDestination(mConnectionId,
                remoteRtpAddress.data(),
                remoteRtpPort,
                remoteRtcpPort,
                receiveVideoRtpPort,
                receiveVideoRtcpPort);

#ifdef TEST_PRINT
            osPrintf("RTP SENDING address: %s port: %d\n", remoteRtpAddress.data(), remoteRtpPort);
#endif

            mpMediaInterface->startRtpSend(mConnectionId,
                numMatchingCodecs, matchingCodecs, srtpParams);

            // If sipX TAPI, fire audio start event
            UtlString audioCodecName;
            UtlString videoCodecName;
            SIPX_CODEC_INFO tapiCodec;
            if (mpMediaInterface->getPrimaryCodec(mConnectionId,
                                                    audioCodecName,
                                                    videoCodecName,
                                                    &tapiCodec.audioCodec.iPayloadType,
                                                    &tapiCodec.videoCodec.iPayloadType) == OS_SUCCESS)
            {
                strncpy(tapiCodec.audioCodec.cName, audioCodecName.data(), SIPXTAPI_CODEC_NAMELEN-1);
                strncpy(tapiCodec.videoCodec.cName, videoCodecName.data(), SIPXTAPI_CODEC_NAMELEN-1);
                fireSipXEvent(CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, &tapiCodec) ;
            }
        }
#ifdef TEST_PRINT
        osPrintf("ACK reinviteState: %d\n", reinviteState);
#endif

        // Free up the codec copies and array
        for(int codecIndex = 0; codecIndex < numMatchingCodecs; codecIndex++)
        {
            delete matchingCodecs[codecIndex];
            matchingCodecs[codecIndex] = NULL;
        }
        if(matchingCodecs) delete[] matchingCodecs;
        matchingCodecs = NULL;

        if(reinviteState == ACCEPT_INVITE)
        {
            inviteFromThisSide = FALSE;
            setCallerId();

            setState(CONNECTION_ESTABLISHED, CONNECTION_REMOTE);
            if (mTerminalConnState == PtTerminalConnection::HELD)
            {
                fireSipXEvent(CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD);
            }
            else
            {
                fireSipXEvent(CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE);
            }


            // If the other side did not send an Allowed field in
            // the INVITE, send an OPTIONS method to see if the
            // otherside supports methods such as REFER
            if(mAllowedRemote.isNull())
            {
                lastLocalSequenceNumber++;
                SipMessage optionsRequest;
                optionsRequest.setOptionsData(mInviteMsg, mRemoteContactUri, inviteFromThisSide,
                    lastLocalSequenceNumber, mRouteField.data(), mLocalContact);
                send(optionsRequest);
            }
        }
        // Reset to allow sub sequent re-INVITE
        else if(reinviteState == REINVITED)
        {
#ifdef TEST_PRINT
            osPrintf("ReINVITE ACK - ReINVITE allowed again\n");
#endif
            reinviteState = ACCEPT_INVITE;
        }

        // If we are in the middle of a transfer meta event
        // on the target phone and target call it ends here
        if(mpCall->getCallType() ==
            CpCall::CP_TRANSFER_TARGET_TARGET_CALL)
        {
            mpCall->setCallType(CpCall::CP_NORMAL_CALL);
#ifdef TEST_PRINT
             OsSysLog::add(FAC_CP, PRI_DEBUG,
                          "SipConnection::processAckRequest "
                          "stopMetaEvent 10");
#endif
           mpCall->stopMetaEvent();
        }
    }

    // Else error response to the ACK
    //getState() != CONNECTION_ESTABLISHED
    // requestSequenceNum != requestSequenceNum
    else
    {
#ifdef TEST_PRINT
        osPrintf("Ignoring ACK connectionState: %d request CSeq: %d invite CSeq: %d\n",
            getState(), requestSequenceNum, requestSequenceNum);
#endif

        // If there is no invite message then shut down this connection
        if(!mInviteMsg)
        {
            setState(CONNECTION_FAILED, CONNECTION_LOCAL);
            fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_UNKNOWN) ;
        }


        // ACKs do not get a response
    }

} // End of processAckRequest

void SipConnection::processByeRequest(const SipMessage* request)
{
#ifdef TEST_PRINT
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "Entering SipConnection::processByeRequest "
                  "mInviteMsg=%p ",
                  mInviteMsg);
#endif
    int requestSequenceNum = 0;
    UtlString requestSeqMethod;

    request->getCSeqField(&requestSequenceNum, &requestSeqMethod);

    if(mInviteMsg && lastRemoteSequenceNumber < requestSequenceNum)
    {
        lastRemoteSequenceNumber = requestSequenceNum;

        // Stop the media usage ASAP
        if (mpMediaInterface != NULL)
        {
            mpMediaInterface->stopRtpSend(mConnectionId);
            fireSipXEvent(CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP);
        }

        // Build an OK response
        SipMessage sipResponse;
        sipResponse.setOkResponseData(request, mLocalContact);
        send(sipResponse);

#ifdef SINGLE_CALL_TRANSFER     // doesn't seem to be defined
        // Check for blind transfer
        int alsoIndex = 0;
        UtlString alsoUri;
        UtlString transferController;
        request->getFromField(&transferController);
        while(request->getAlsoUri(alsoIndex, &alsoUri))
        {
            ((CpPeerCall*)mpCall)->addParty(alsoUri.data(), transferController.data(), NULL);
            alsoIndex++;
        }

#else       // so this code should run?
        UtlString thereAreAnyAlsoUri;
        if(request->getAlsoUri(0, &thereAreAnyAlsoUri))
        {
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipConnection::processByeRequest "
                          "getAlsoUri non-zero ");

            // Two call model BYE Also transfer

            // Create a second call if it does not exist already
            // Set the target call id in this call
            // Set this call's type to transferee original call
            UtlString targetCallId;

            // I am not sure we want the focus change to
            // be automatic.  It is here for now to make it work
            CpIntMessage yieldFocus(CallManager::CP_YIELD_FOCUS, (intptr_t)mpCall);
            mpCallManager->postMessage(yieldFocus);

            // The new call by default assumes focus
            mpCallManager->createCall(&targetCallId);
            mpCall->setTargetCallId(targetCallId.data());
            mpCall->setCallType(CpCall::CP_TRANSFEREE_ORIGINAL_CALL);

            int alsoIndex = 0;
            UtlString alsoUri;
            UtlString transferController;
            request->getFromField(&transferController);
            UtlString remoteAddress;
            getRemoteAddress(&remoteAddress);
            UtlString thisCallId;
            getCallId(&thisCallId);
            UtlString referredBy;
            request->getFromField(&referredBy);
            while(request->getAlsoUri(alsoIndex, &alsoUri))
            {
                //((CpPeerCall*)mpCall)->addParty(alsoUri.data(),
                //    transferController.data(), NULL);
                alsoIndex++;

#   ifdef TEST_PRINT
                osPrintf("SipConnection::processRequest BYE Also URI: %s callid: %s\n",
                    alsoUri.data(), targetCallId.data());
#   endif
                // Send a message to the target call to create the
                // connection and send the INVITE
                CpMultiStringMessage transfereeConnect(CallManager::CP_TRANSFEREE_CONNECTION,
                    targetCallId.data(), alsoUri.data(), referredBy.data(), thisCallId.data(),
                    remoteAddress.data(), TRUE /* Use BYE Also style INVITE*/ );

#   ifdef TEST_PRINT
                OsSysLog::add(FAC_SIP, PRI_DEBUG,
                              "SipConnection::processByeRequest "
                              "posting CP_TRANSFEREE_CONNECTION\n");
#   endif
                mpCallManager->postMessage(transfereeConnect);
            }

            // Send a trying response as it is likely to take a while
            //SipMessage sipTryingResponse;
            //sipTryingResponse.setTryingResponseData(request);
            //send(sipTryingResponse);
        }
#endif

        setState(CONNECTION_DISCONNECTED, CONNECTION_REMOTE);
        fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL) ;
    }   // end had invite message and cseq rules were met

    // BYE is not legal in the current state
    else
    {
        // Build an error response
        SipMessage sipResponse;
        sipResponse.setByeErrorData(request);
        send(sipResponse);

        // Do not change the state

        // I do not recall the context of the above comment
        // May want to change to failed state in all cases
        if(getState() == CONNECTION_IDLE)
        {
            setState(CONNECTION_FAILED, CONNECTION_LOCAL);
            fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_UNKNOWN) ;
        }
        else if(!mInviteMsg)
        {
            // If an invite was not sent or received something
            // is wrong.  This bye is invalid.
            setState(CONNECTION_FAILED, CONNECTION_LOCAL);
            fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_UNKNOWN) ;
        }
    }
#ifdef TEST_PRINT
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "Leaving SipConnection::processByeRequest "
                  "mInviteMsg=%p ",
                  mInviteMsg);
#endif
} // End of processByeRequest

void SipConnection::processCancelRequest(const SipMessage* request)
{
    int requestSequenceNum = 0;
    UtlString requestSeqMethod;

    request->getCSeqField(&requestSequenceNum, &requestSeqMethod);

    int calleeState = getState();

    // If it is acceptable to CANCLE the call
    if(lastRemoteSequenceNumber == requestSequenceNum &&
        calleeState != CONNECTION_IDLE &&
        calleeState != CONNECTION_DISCONNECTED &&
        calleeState !=  CONNECTION_FAILED &&
        calleeState != CONNECTION_ESTABLISHED)
    {


        // Build a 487 response
        if (!inviteFromThisSide)
        {
            SipMessage sipResponse;
            sipResponse.setRequestTerminatedResponseData(mInviteMsg);
            send(sipResponse);
        }

        setState(CONNECTION_DISCONNECTED, CONNECTION_REMOTE, CONNECTION_CAUSE_CANCELLED);
        fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL) ;

        // Build an OK response
        SipMessage cancelResponse;
        cancelResponse.setOkResponseData(request, mLocalContact);
        send(cancelResponse);
    }

    // CANCEL is not legal in the current state
    else
    {
        // Build an error response
        SipMessage sipResponse;
        sipResponse.setBadTransactionData(request);
        send(sipResponse);

        // Do not change the state

        // Do not know where the above comment came from
        // If there was no invite sent or received this is a bad call
        if(!mInviteMsg)
        {
            setState(CONNECTION_FAILED, CONNECTION_LOCAL, CONNECTION_CAUSE_CANCELLED);
            fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_UNKNOWN) ;
        }
    }
} // End of processCancelRequest

UtlBoolean SipConnection::getInitialSdpCodecs(const SipMessage* sdpMessage,
                                              SdpCodecFactory& supportedCodecsArray,
                                              int& numCodecsInCommon,
                                              SdpCodec** &codecsInCommon,
                                              UtlString& remoteAddress,
                                              int& remotePort,
                                              int& remoteRtcpPort,
                                              SdpDirectionality* directionality) const
{
    int videoRtpPort;
    int videoRtcpPort;
    // Get the RTP info from the message if present
    // Should check the content type first
    const SdpBody* sdpBody = sdpMessage->getSdpBody();
    if(sdpBody)
    {
#ifdef TEST_PRINT
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipConnection::getInitialSdpCodecs "
                      "SDP body in INVITE, finding best codec");
#endif
        // This does not support SDP data with multiple media lines (m=)
        // This handicap is also present in SdpBody::getBestAudioCodes

        int mediaIndex = 0;

        if(!sdpBody->getMediaDirection(mediaIndex, directionality))
        {
            // The default value is sendrecv if there is no attribute.
            *directionality = sdpDirectionalitySendRecv;
        }

        sdpBody->getBestAudioCodecs(supportedCodecsArray,
            numCodecsInCommon,
            codecsInCommon,
            remoteAddress,
            remotePort,
            remoteRtcpPort,
            videoRtpPort,
            videoRtcpPort);
    }
#ifdef TEST_PRINT
    else
    {
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipConnection::getInitialSdpCodecs "
                      "No SDP in message");
    }
#endif
    if(sdpBody != NULL)
    {
        delete sdpBody;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

UtlBoolean SipConnection::processResponse(const SipMessage* response,
                                          UtlBoolean callInFocus,
                                          UtlBoolean onHook)
{
    int sequenceNum;
    UtlString sequenceMethod;
    UtlBoolean processedOk = TRUE;
    UtlString responseText;
    UtlString contactUri;
    int previousState = getState();
    int responseCode = response->getResponseStatusCode();
    mResponseCode = responseCode;

    response->getResponseStatusText(&responseText);
    mResponseText = responseText;
    response->getCSeqField(&sequenceNum, &sequenceMethod);

    if(!mInviteMsg)
    {
        // An invite was not sent or received.  This call is invalid.
        setState(CONNECTION_FAILED, CONNECTION_REMOTE);
        fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL) ;
    }

    else if(strcmp(sequenceMethod.data(), SIP_INVITE_METHOD) == 0)
    {
        processInviteResponse(response);
        if (mTerminalConnState == PtTerminalConnection::HELD)
        {
            UtlString remoteAddress;
            getRemoteAddress(&remoteAddress);
            postTaoListenerMessage(PtEvent::TERMINAL_CONNECTION_HELD, PtEvent::CAUSE_NEW_CALL);
            /** SIPXTAPI TBD **/
            // fireSipXEvent(CONNECTED, CONNECTED_INACTIVE_HELD) ;
        }

    } // End INVITE responses

    // REFER responses:
    else if(strcmp(sequenceMethod.data(), SIP_REFER_METHOD) == 0)
    {
        processReferResponse(response);
    }

#ifdef TEST_PRINT
    // ACK response ???
    else if(strcmp(sequenceMethod.data(), SIP_ACK_METHOD) == 0)
    {
        osPrintf("ACK response ignored: %d %s\n", responseCode,
            responseText.data());
    }
#endif

    // Options response
    else if(strcmp(sequenceMethod.data(), SIP_OPTIONS_METHOD) == 0)
    {
        processOptionsResponse(response);
    }

    // NOTIFY response
    else if(strcmp(sequenceMethod.data(), SIP_NOTIFY_METHOD) == 0)
    {
        processNotifyResponse(response);
    }

    // else
    // BYE, CANCEL responses
    else if(   strcmp(sequenceMethod.data(), SIP_BYE_METHOD) == 0
            || strcmp(sequenceMethod.data(), SIP_CANCEL_METHOD) == 0)
    {
        // We check the sequence number and method name of the
        // last sent request to make sure this is a response to
        // something that we actually sent
        if(   lastLocalSequenceNumber == sequenceNum
           && sequenceMethod.compareTo(mLastRequestMethod) == 0)
        {
#ifdef TEST_PRINT
            osPrintf("%s response: %d %s\n", sequenceMethod.data(),
                responseCode, responseText.data());
#endif
            if(responseCode >= SIP_OK_CODE)
            {
                if (mpMediaInterface != NULL)
                {
                    mpMediaInterface->stopRtpSend(mConnectionId);
                    mpMediaInterface->stopRtpReceive(mConnectionId);
                    fireSipXEvent(CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP);
                }
            }

            // If this is the response to a BYE 'Also' transfer
            if (  getState() == CONNECTION_ESTABLISHED
               && responseCode >= SIP_OK_CODE
               && strcmp(sequenceMethod.data(), SIP_BYE_METHOD) == 0
               && !mTargetCallConnectionAddress.isNull()
               && !isMethodAllowed(SIP_REFER_METHOD))
            {
                // We need to send notification to the target call
                // as to whether the transfer failed or succeeded
                int state;
                int cause;
                if(responseCode == SIP_OK_CODE)
                {
                    state = CONNECTION_ESTABLISHED;
                    //cause = CONNECTION_CAUSE_NORMAL;
                    //setState(CONNECTION_DISCONNECTED, CONNECTION_CAUSE_TRANSFER);
                    cause = CONNECTION_CAUSE_TRANSFER;
                }
                else if(responseCode == SIP_BAD_EXTENSION_CODE ||
                    responseCode == SIP_UNIMPLEMENTED_METHOD_CODE)
                {
                    state = CONNECTION_FAILED;
                    cause = CONNECTION_CAUSE_INCOMPATIBLE_DESTINATION;
                }
                else if(responseCode == SIP_DECLINE_CODE)
                {
                    state = CONNECTION_FAILED;
                    cause = CONNECTION_CAUSE_CANCELLED;
                }
                else
                {
                    state = CONNECTION_FAILED;
                    cause = CONNECTION_CAUSE_BUSY;
                }

                setState(state, CONNECTION_REMOTE, cause);
                /** SIPXTAPI: TBD **/

                // Send the message to the target call
                UtlString targetCallId;
                UtlString toField;
                mToUrl.toString(toField);
                mpCall->getTargetCallId(targetCallId);
                CpMultiStringMessage transferControllerStatus(CallManager::CP_TRANSFER_CONNECTION_STATUS,
                    targetCallId.data(), toField.data(),
                    NULL, NULL, NULL,
                    state, cause);
#ifdef TEST_PRINT
                osPrintf("SipConnection::processResponse BYE posting CP_TRANSFER_CONNECTION_STATUS to call: %s\n",
                    targetCallId.data());
#endif
                mpCallManager->postMessage(transferControllerStatus);

                // Reset mTargetCallConnectionAddress so that if this connection
                // is not disconnected due to transfer failure, it can
                // try another transfer or just disconnect.
                mTargetCallConnectionAddress = "";

            }

            //for BYE
            else if(   responseCode >= SIP_OK_CODE
                    && lastLocalSequenceNumber == sequenceNum
                    && (strcmp(sequenceMethod.data(), SIP_BYE_METHOD) == 0))
            {
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                    "SipConnection::processResponse: "
                    "Response %d received for BYE", responseCode);
                setState(CONNECTION_DISCONNECTED, CONNECTION_REMOTE);
                fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL) ;

                // If we are in the middle of a transfer meta event
                // on the target phone and target call it ends here
                int metaEventId = 0;
                int metaEventType = PtEvent::META_EVENT_NONE;
                int numCalls = 0;
                const UtlString* metaEventCallIds = NULL;
                if(mpCall)
                {
                    mpCall->getMetaEvent(metaEventId, metaEventType,
                                         numCalls, &metaEventCallIds);
                    if(metaEventId > 0
                       && (metaEventType == PtEvent::META_CALL_TRANSFERRING
                           || metaEventType == PtEvent::META_CALL_REPLACING))
                    {
#ifdef TEST_PRINT
                         OsSysLog::add(FAC_CP, PRI_DEBUG,
                                      "SipConnection::processResponse: "
                                      "stopMetaEvent 2 eventType= %x",
                                      metaEventType);
#endif
                       mpCall->stopMetaEvent();
                    }
                }
            }
            else if(responseCode >= SIP_OK_CODE &&
                lastLocalSequenceNumber == sequenceNum &&
                (strcmp(sequenceMethod.data(), SIP_CANCEL_METHOD) == 0) &&
                previousState != CONNECTION_ESTABLISHED &&
                previousState != CONNECTION_FAILED &&
                previousState != CONNECTION_DISCONNECTED &&
                previousState != CONNECTION_UNKNOWN &&
                previousState != CONNECTION_FAILED)
            {
                //start 32 second timer according to the bis03 draft
                UtlString callId;
                mpCall->getCallId(callId);
                UtlString remoteAddr;
                getRemoteAddress(&remoteAddr);
                CpMultiStringMessage* CancelTimerMessage =
                    new CpMultiStringMessage(CpCallManager::CP_CANCEL_TIMER, callId.data(), remoteAddr.data());
                OsTimer* timer = new OsTimer(mpCallManager->getMessageQueue(), (void*)CancelTimerMessage);
                OsTime timerTime(32,0);
                timer->oneshotAfter(timerTime);
            }

            else if(sequenceMethod.compareTo(SIP_BYE_METHOD) == 0)
            {
                processByeResponse(response);
            }
            else if(sequenceMethod.compareTo(SIP_CANCEL_METHOD) == 0)
            {
                processCancelResponse(response);
            }
            // else Ignore provisional responses
            else
            {
#ifdef TEST_PRINT
                osPrintf("%s provisional response ignored: %d %s\n",
                    sequenceMethod.data(),
                    responseCode,
                    responseText.data());
#endif
            }
        }
        else
        {
#ifdef TEST_PRINT
            osPrintf("%s response ignored: %d %s invalid cseq last: %d last method: %s\n",
                sequenceMethod.data(),
                responseCode,
                responseText.data(),
                lastLocalSequenceNumber,
                mLastRequestMethod.data());
#endif
        }
    }//END - else if(strcmp(sequenceMethod.data(), SIP_BYE_METHOD) == 0 || strcmp(sequenceMethod.data(), SIP_CANCEL_METHOD) == 0)

    // Unknown method response
    else
    {
#ifdef TEST_PRINT
        osPrintf("%s response ignored: %d %s\n",
            sequenceMethod.data(),
            responseCode,
            responseText.data());
#endif
    }

    return(processedOk);
}  // End of processResponse

void SipConnection::processInviteResponse(const SipMessage* response)
{
    int sequenceNum;
    UtlString sequenceMethod;
    int responseCode = response->getResponseStatusCode();
    int previousState = getState();

    response->getCSeqField(&sequenceNum, &sequenceMethod);

    if(lastLocalSequenceNumber == sequenceNum
       && mInviteMsg)
    {
        UtlString toAddr;
        UtlString toProto;
        int toPort;
        UtlString inviteTag;

        // Check to see if there is a tag set in the To field that
        // should be remembered for future messages.
        mInviteMsg->getToAddress(&toAddr, &toPort, &toProto,
            NULL, NULL, &inviteTag);

        // Save to-tags from all provo responses in a hashmap.
        // Use To tag as key and the remote contact as value
		// Delete provisional to-tag entries when final response arrives
        if(inviteTag.isNull())
        {
            response->getToAddress(&toAddr, &toPort, &toProto,
                NULL, NULL, &inviteTag);

#ifdef TEST_PRINT
            osPrintf("SipConnection::processInviteResponse no invite tag, got response tag: \"%s\"\n",
                inviteTag.data());
#endif

            if(!inviteTag.isNull())
            {
                if(responseCode >= SIP_OK_CODE)
                {
                    mInviteMsg->setToFieldTag(inviteTag.data());

                    // Update the cased to field after saving the tag
                    mInviteMsg->getToUrl(mToUrl);

                    // We have a To Tag from a final response. Delete all
                    // provsional tags
                    mProvisionalToTags.destroyAll();

                }
                else // provisional responses with To Tags
                {
                    UtlString * tmpToTag = new UtlString(inviteTag);
                    UtlString * tmpContactAddress = new UtlString();

                    response->getContactEntry(0, tmpContactAddress);

                    if(!mProvisionalToTags.contains(tmpToTag))
                    {
                        mProvisionalToTags.insertKeyAndValue(tmpToTag,
                                tmpContactAddress);
                    }
                    else
                    {
                        // ERROR: We should never really see this happen
                        OsSysLog::add(FAC_CP, PRI_ERR,
                             "SipConnection::processInviteResponse "
                             "provisional response without to-tag ");
						delete tmpToTag;
                        delete tmpContactAddress;
                    }
                }
            }
        }
    }

#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "SipConnection::processInviteResponse "
                  "responseCode %d reinviteState %d "
                  "sequenceNum %d lastLocalSequenceNumber%d",
                  responseCode,
                  reinviteState,
                  sequenceNum,
                  lastLocalSequenceNumber);
#endif
    if((responseCode == SIP_RINGING_CODE ||
        responseCode == SIP_EARLY_MEDIA_CODE) &&
        lastLocalSequenceNumber == sequenceNum &&
        reinviteState == ACCEPT_INVITE)
    {
        UtlBoolean isEarlyMedia = TRUE;
#ifdef TEST_PRINT
        osPrintf("received Ringing response\n");
#endif
        if( responseCode == SIP_RINGING_CODE && !mIsEarlyMediaFor180)
        {
            isEarlyMedia = FALSE;
        }
        // If there is SDP we have early media or remote ringback
        int cause = CONNECTION_CAUSE_NORMAL;
        if(response->hasSdpBody() && isEarlyMedia && mpMediaInterface != NULL)
        {
            cause = CONNECTION_CAUSE_UNKNOWN;

            // If this is the initial INVITE
            if(reinviteState == ACCEPT_INVITE)
            {
                // Setup the sending of audio
                UtlString rtpAddress;
                int receiveRtpPort;
                int receiveRtcpPort;
                int receiveVideoRtpPort;
                int receiveVideoRtcpPort;
                SdpCodecFactory supportedCodecs;
                SdpSrtpParameters srtpParams;
                mpMediaInterface->getCapabilities(mConnectionId,
                    rtpAddress,
                    receiveRtpPort,
                    receiveRtcpPort,
                    receiveVideoRtpPort,        // VIDEO: TODO
                    receiveVideoRtcpPort,
                    supportedCodecs,
                    srtpParams);
                // Setup the media channel
                // The address should be retrieved from the sdpBody
                int numMatchingCodecs = 0;
                SdpCodec** matchingCodecs = NULL;
        		SdpDirectionality directionality; 	// result not yet used
                getInitialSdpCodecs(response, supportedCodecs,
                    numMatchingCodecs, matchingCodecs,
                    remoteRtpAddress, remoteRtpPort, remoteRtcpPort, &directionality);

                if(numMatchingCodecs > 0)
                {
                    // Set up the remote RTP sockets if we have a legitimate
                    // address to send RTP
                    if(!remoteRtpAddress.isNull() &&
                        remoteRtpAddress.compareTo("0.0.0.0") != 0)
                    {
                        mpMediaInterface->setConnectionDestination(mConnectionId,
                                remoteRtpAddress.data(),
                                remoteRtpPort,
                                remoteRtcpPort,
                                receiveVideoRtpPort,
                                receiveVideoRtcpPort);
                    }

                    if(!remoteRtpAddress.isNull() &&
                        remoteRtpAddress.compareTo("0.0.0.0") != 0 &&
                        remoteRtpPort > 0 &&
                        mTerminalConnState != PtTerminalConnection::HELD)
                    {
                        mpMediaInterface->startRtpSend(mConnectionId,
                            numMatchingCodecs,
                            matchingCodecs,
                            srtpParams);

                        // If sipX TAPI, fire audio start event
                        UtlString audioCodecName;
                        UtlString videoCodecName;
                        SIPX_CODEC_INFO tapiCodec;
                        if (mpMediaInterface->getPrimaryCodec(mConnectionId,
                                                              audioCodecName,
                                                              videoCodecName,
                                                              &tapiCodec.audioCodec.iPayloadType,
                                                              &tapiCodec.videoCodec.iPayloadType) == OS_SUCCESS)
                        {
                            strncpy(tapiCodec.audioCodec.cName, audioCodecName.data(), SIPXTAPI_CODEC_NAMELEN-1);
                            strncpy(tapiCodec.videoCodec.cName, videoCodecName.data(), SIPXTAPI_CODEC_NAMELEN-1);
                            fireSipXEvent(CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, &tapiCodec) ;
                        }
                    }

                } // End if there are matching codecs

                // Free up the codec copies and array
                for(int codecIndex = 0; codecIndex < numMatchingCodecs; codecIndex++)
                {
                    delete matchingCodecs[codecIndex];
                    matchingCodecs[codecIndex] = NULL;
                }
                if(matchingCodecs) delete[] matchingCodecs;
                matchingCodecs = NULL;

            } // End if this is an initial INVITE
        } // End if this is an early media, provisional response

        setState(CONNECTION_ALERTING, CONNECTION_REMOTE, cause);
        if (responseCode == SIP_EARLY_MEDIA_CODE)
        {
            fireSipXEvent(CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_MEDIA) ;
        }
        else
        {
            fireSipXEvent(CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL) ;
        }
    }   // end ringing or early media

    // Start busy tone the other end is busy
    else if(responseCode == SIP_BUSY_CODE
            && lastLocalSequenceNumber == sequenceNum
            && reinviteState == ACCEPT_INVITE)
    {
#ifdef TEST_PRINT
        osPrintf("received INVITE Busy\n");
#endif
        setState(CONNECTION_FAILED, CONNECTION_REMOTE, CONNECTION_CAUSE_BUSY);
        fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_BUSY) ;
    }   // end BUSY

    // Call queued
    else if(responseCode == SIP_QUEUED_CODE &&
        lastLocalSequenceNumber == sequenceNum &&
        reinviteState == ACCEPT_INVITE)
    {
#ifdef TEST_PRINT
        osPrintf("received INVITE queued\n");
#endif
        setState(CONNECTION_QUEUED, CONNECTION_REMOTE);
        /** SIPXTAPI: TBD **/

        // Should there be a queued tone?

        // Do not send an ACK as this is informational
    }   // end queued

    // Failed invite
    else if(responseCode >= SIP_BAD_REQUEST_CODE &&
        lastLocalSequenceNumber == sequenceNum)
    {
        if (reinviteState == ACCEPT_INVITE)

        {
            UtlString responseText;
            response->getResponseStatusText(&responseText);
#ifdef TEST_PRINT
            osPrintf("INVITE failed with response: %d %s\n",
                responseCode, responseText.data());
#endif

            int cause = CONNECTION_CAUSE_UNKNOWN;
            int warningCode;

            switch(responseCode)
            {
            case HTTP_UNAUTHORIZED_CODE:
                cause = CONNECTION_CAUSE_NOT_ALLOWED;
                break;

            case HTTP_PROXY_UNAUTHORIZED_CODE:
                cause = CONNECTION_CAUSE_NETWORK_NOT_ALLOWED;
                break;

            case SIP_REQUEST_TIMEOUT_CODE:
                cause = CONNECTION_CAUSE_CANCELLED;
                break;

            case SIP_BAD_REQUEST_CODE:
                response->getWarningCode(&warningCode);
                if(warningCode == SIP_WARN_MEDIA_NAVAIL_MEDIA_TYPE_CODE)
                {
                    // incompatable media
                    cause = CONNECTION_CAUSE_INCOMPATIBLE_DESTINATION;
                }
                break;

            default:

                if(responseCode < SIP_SERVER_INTERNAL_ERROR_CODE)
                    cause = CONNECTION_CAUSE_INCOMPATIBLE_DESTINATION;
                else if(responseCode >= SIP_SERVER_INTERNAL_ERROR_CODE &&
                    responseCode < SIP_GLOBAL_BUSY_CODE)
                    cause = CONNECTION_CAUSE_NETWORK_NOT_OBTAINABLE;
                if(responseCode >= SIP_GLOBAL_BUSY_CODE)
                    cause = CONNECTION_CAUSE_NETWORK_CONGESTION;
                else cause = CONNECTION_CAUSE_UNKNOWN;
                break;
            }

            // in case the response is "487 call terminated"
            if(responseCode == SIP_REQUEST_TERMINATED_CODE)
            {
                cause = CONNECTION_CAUSE_CANCELLED;
                setState(CONNECTION_DISCONNECTED, CONNECTION_REMOTE, cause);
                fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_BUSY) ;
            }
            else if (responseCode == SIP_NOT_FOUND_CODE)
            {
                cause = CONNECTION_CAUSE_DEST_NOT_OBTAINABLE;
                setState(CONNECTION_DISCONNECTED, CONNECTION_REMOTE, cause);
                fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_BADADDRESS) ;
            }
            else
            {
                setState(CONNECTION_FAILED, CONNECTION_REMOTE, cause);
                /** SIPXTAPI: TBD **/
            }

            mbCancelling = FALSE;   // if was cancelling, now cancelled.

        }

        else if (reinviteState == REINVITING &&
                 responseCode != SIP_REQUEST_NOT_ACCEPTABLE_HERE_CODE)
        {
            // ACK gets sent by the SipUserAgent for error responses
            //SipMessage sipRequest;
            //sipRequest.setAckData(response,mInviteMsg);
            //send(sipRequest);

            reinviteState = ACCEPT_INVITE;
            //processedOk = false;

            // Temp Fix: If we failed to renegotiate a invite, failed the
            // connection so that the upper layers can react.  We *SHOULD*
            // fire off a new event to application layer indicating that the
            // reinvite failed -- or make hold/unhold blocking. (Bob 8/14/02)

            postTaoListenerMessage(CONNECTION_FAILED, CONNECTION_CAUSE_INCOMPATIBLE_DESTINATION, false);
            fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_RESOURCES) ;
        }
        else if (reinviteState == REINVITING &&
                 responseCode == SIP_REQUEST_NOT_ACCEPTABLE_HERE_CODE)
        {
            reinviteState = ACCEPT_INVITE;

            // RFC 3261 states:
            /*
                During the session, either Alice or Bob may decide to change the
                characteristics of the media session.  This is accomplished by
                sending a re-INVITE containing a new media description.  This re-
                INVITE references the existing dialog so that the other party knows
                that it is to modify an existing session instead of establishing a
                new session.  The other party sends a 200 (OK) to accept the change.
                The requestor responds to the 200 (OK) with an ACK.  If the other
                party does not accept the change, he sends an error response such as
                488 (Not Acceptable Here), which also receives an ACK.  However, the
                failure of the re-INVITE does not cause the existing call to fail -
                the session continues using the previously negotiated
                characteristics.  Full details on session modification are in Section
                14.
            */
            // my interpretation of this is that we need to continue the session
            // and not cause a disconnect - MDC 6/23/2005

            fireSipXEvent(CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_REQUEST_NOT_ACCEPTED) ;
        }
    }   // error response

    // INVITE OK too late
    // The other end picked up, but this end already hungup
    else if(responseCode == SIP_OK_CODE &&
        lastLocalSequenceNumber == sequenceNum &&
        (getState() == CONNECTION_DISCONNECTED || mbCancelling))
    {

        mbCancelling = FALSE;   // if was cancelling, now cancelled.
        // Send an ACK
        SipMessage sipAckRequest;
        sipAckRequest.setAckData(response,mInviteMsg);
        send(sipAckRequest);


        // Always get the remote contact as it may can change over time
        UtlString contactInResponse;
        if (response->getContactEntry(0 , &contactInResponse))
        {
            Url temp(contactInResponse) ;
            temp.removeHeaderParameters() ;
            temp.removeFieldParameters() ;
            temp.includeAngleBrackets() ;
            temp.toString(mRemoteContact) ;
            temp.getUri(mRemoteContactUri) ;
        }

        // Get the route for subsequent requests
        response->buildRouteField(&mRouteField);
#ifdef TEST_PRINT
        osPrintf("okINVITE set mRouteField: %s\n", mRouteField.data());
#endif

        // Send a BYE
        SipMessage sipByeRequest;
        lastLocalSequenceNumber++;
        sipByeRequest.setByeData(mInviteMsg, mRemoteContactUri, TRUE,
            lastLocalSequenceNumber, mRouteField.data(), NULL,
            mLocalContact.data());

        mLastRequestMethod = SIP_BYE_METHOD;
        send(sipByeRequest);
#ifdef TEST_PRINT
        osPrintf("glare: received OK to hungup call, sending ACK & BYE\n");
#endif

    }   // end OK too late

    // INVITE OK
    // The other end picked up
    else if(responseCode == SIP_OK_CODE
            && lastLocalSequenceNumber == sequenceNum)
    {
#ifdef TEST_PRINT
        osPrintf("received INVITE OK\n");
#endif
        // connect the call

        //save the contact field in the response to send further reinvites
        UtlString contactInResponse;
        if (response->getContactEntry(0 , &contactInResponse))
        {
            Url temp(contactInResponse) ;
            temp.removeHeaderParameters() ;
            temp.removeFieldParameters() ;
            temp.includeAngleBrackets() ;
            temp.toString(mRemoteContact) ;
            temp.getUri(mRemoteContactUri) ;
        }

        // Get the route for subsequent requests only if this is
        // the initial transaction
        if(previousState != CONNECTION_ESTABLISHED)
        {
            response->buildRouteField(&mRouteField);
#ifdef TEST_PRINT
            osPrintf("okINVITE set mRouteField: %s\n", mRouteField.data());
#endif
        }

        // Get the session timer if set
        mSessionReinviteTimer = 0;
        response->getSessionExpires(&mSessionReinviteTimer);

        // Set a timer to reINVITE to keep the session up
        if(mSessionReinviteTimer > mDefaultSessionReinviteTimer &&
            mDefaultSessionReinviteTimer != 0) // if default < 0 disallow
        {
            mSessionReinviteTimer = mDefaultSessionReinviteTimer;
        }

        int timerSeconds = mSessionReinviteTimer;
        if(mSessionReinviteTimer > 2)
        {
            timerSeconds = mSessionReinviteTimer / 2; //safety factor
        }

        // Construct an ACK
        SipMessage sipRequest;
        sipRequest.setAckData(response, mInviteMsg, NULL,
            mSessionReinviteTimer);
        // Set the route field
        if(!mRouteField.isNull())
        {
            sipRequest.setRouteField(mRouteField.data());
#ifdef TEST_PRINT
            osPrintf("Adding route to ACK: %s\n", mRouteField.data());
#endif
        }
#ifdef TEST_PRINT
        else
        {

            osPrintf("No route to add to ACK :%s\n", mRouteField.data());
        }
#endif

        // Start the session reINVITE timer
        if(mSessionReinviteTimer > 0)
        {
            SipMessageEvent* sipMsgEvent =
                new SipMessageEvent(new SipMessage(sipRequest),
                SipMessageEvent::SESSION_REINVITE_TIMER);
            OsTimer* timer = new OsTimer((mpCallManager->getMessageQueue()),
                (void*)sipMsgEvent);
            // Convert from mSeconds to uSeconds
            OsTime timerTime(timerSeconds, 0);
            timer->oneshotAfter(timerTime);
#ifdef TEST_PRINT
            osPrintf("SipConnection::processInviteResponse timer message type: %d %d",
                OsMsg::PHONE_APP, SipMessageEvent::SESSION_REINVITE_TIMER);
#endif
        }

        // Send the ACK message
        send(sipRequest);
        bool sendDelayedRefer = FALSE;

        if (mFarEndHoldState == TERMCONNECTION_HOLDING)
        {
            setTerminalConnectionState(PtTerminalConnection::HELD, 1);
            mFarEndHoldState = TERMCONNECTION_HELD;

            if (mTerminalConnState == PtTerminalConnection::HELD)  // just set this two lines above???
            {
                OsSysLog::add(FAC_SIP, PRI_DEBUG,
                              "SipConnection::processInviteResponse, "
                              "CONNECTED_INACTIVE response for HOLD");
                fireSipXEvent(CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_INACTIVE);
            }
            else
            {
                fireSipXEvent(CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD);
            }

            if (mpMediaInterface != NULL)
            {
                mpMediaInterface->stopRtpSend(mConnectionId);
                fireSipXEvent(CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP);
            }

            // The prerequisite hold was completed we
            // can now do the next action/transaction
            switch(mHoldCompleteAction)
            {
            case CpCallManager::CP_BLIND_TRANSFER:
                // This hold was performed as a precurser to
                // A blind transfer.
                mHoldCompleteAction = CpCallManager::CP_UNSPECIFIED;
                //doBlindRefer();
                sendDelayedRefer = TRUE;
                break;

            default:
                // Bogus action, reset it
                mHoldCompleteAction = CpCallManager::CP_UNSPECIFIED;
                break;
            }
        }   // end TERMCONNECTION_HOLDING
        else if (mFarEndHoldState == TERMCONNECTION_TALKING)
        {
            setTerminalConnectionState(PtTerminalConnection::TALKING, 1);
            mFarEndHoldState = TERMCONNECTION_NONE;
        }

#ifdef TEST_PRINT
        osPrintf("200 OK reinviteState: %d\n", reinviteState);
#endif

        UtlString rtpAddress;
        int receiveRtpPort;
        int receiveRtcpPort;
        int receiveVideoRtpPort;
        int receiveVideoRtcpPort;
        SdpCodecFactory supportedCodecs;
        SdpSrtpParameters srtpParams;

        if (mpMediaInterface != NULL)
        {
            mpMediaInterface->getCapabilities(mConnectionId,
                    rtpAddress,
                    receiveRtpPort,
                    receiveRtcpPort,
                    receiveVideoRtpPort,        // VIDEO: TODO
                    receiveVideoRtcpPort,
                    supportedCodecs,
                    srtpParams);
        }
        // Setup the media channel
        // The address should be retrieved from the sdpBody
        int numMatchingCodecs = 0;
        SdpCodec** matchingCodecs = NULL;
        SdpDirectionality directionality; 	// result not yet used
        getInitialSdpCodecs(response, supportedCodecs,
            numMatchingCodecs, matchingCodecs,
            remoteRtpAddress, remoteRtpPort, remoteRtcpPort, &directionality);

        if (numMatchingCodecs > 0 && mpMediaInterface != NULL)
        {
            // Set up the remote RTP sockets if we have a legitimate
            // address to send RTP
            if(!remoteRtpAddress.isNull() &&
                remoteRtpAddress.compareTo("0.0.0.0") != 0)
            {
                mpMediaInterface->setConnectionDestination(mConnectionId,
                        remoteRtpAddress.data(),
                        remoteRtpPort,
                        remoteRtcpPort,
                        receiveVideoRtpPort,
                        receiveVideoRtcpPort);

            }

            if(reinviteState == ACCEPT_INVITE)
            {
                setState(CONNECTION_ESTABLISHED, CONNECTION_REMOTE);
                if (mTerminalConnState == PtTerminalConnection::HELD)
                {
                    fireSipXEvent(CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD);
                }
                else
                {
                    fireSipXEvent(CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE);
                }
            }

            //osPrintf("RTP SENDING address: %s port: %d \nRTP LISTENING port: %d\n",
            //                      remoteRtpAddress.data(), remoteRtpPort, localRtpPort);

            // No RTP address, stop sending media
            if(remoteRtpAddress.isNull() ||
                remoteRtpAddress.compareTo("0.0.0.0") == 0)
            {
                mpMediaInterface->stopRtpSend(mConnectionId);
                // stop receiving it too
                mpMediaInterface->stopRtpReceive(mConnectionId);
                fireSipXEvent(CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP);
            }

            else if(remoteRtpPort > 0 && mTerminalConnState != PtTerminalConnection::HELD)
            {
                mpMediaInterface->startRtpReceive(mConnectionId,
                    numMatchingCodecs,
                    matchingCodecs,
                    srtpParams);

                mpMediaInterface->startRtpSend(mConnectionId,
                    numMatchingCodecs,
                    matchingCodecs,
                    srtpParams);

                // If sipX TAPI, fire an audio start event
                UtlString audioCodecName;
                UtlString videoCodecName;
                SIPX_CODEC_INFO tapiCodec;
                if (mpMediaInterface->getPrimaryCodec(mConnectionId,
                                                      audioCodecName,
                                                      videoCodecName,
                                                      &tapiCodec.audioCodec.iPayloadType,
                                                      &tapiCodec.videoCodec.iPayloadType) == OS_SUCCESS)
                {
                    strncpy(tapiCodec.audioCodec.cName, audioCodecName.data(), SIPXTAPI_CODEC_NAMELEN-1);
                    strncpy(tapiCodec.videoCodec.cName, videoCodecName.data(), SIPXTAPI_CODEC_NAMELEN-1);

                    if (mTerminalConnState == PtTerminalConnection::TALKING)
                    {
                        fireSipXEvent(CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE) ;
                    }
                    else if (mTerminalConnState == PtTerminalConnection::HELD)
                    {
                        fireSipXEvent(CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD) ;
                    }
                    fireSipXEvent(CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, &tapiCodec) ;
                }
            }
            else if(mTerminalConnState == PtTerminalConnection::HELD)
            {
                mpMediaInterface->stopRtpSend(mConnectionId);
                mpMediaInterface->stopRtpReceive(mConnectionId);
                fireSipXEvent(CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP);
            }
        }   // end media setup path

        // Original INVITE response with no SDP
        // cannot setup call
        else if(reinviteState == ACCEPT_INVITE)
        {
#ifdef TEST_PRINT
            osPrintf("No SDP in INVITE OK response\n");
#endif
            setState(CONNECTION_FAILED, CONNECTION_REMOTE, CONNECTION_CAUSE_INCOMPATIBLE_DESTINATION);
            fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_RESOURCES) ;

            // Send a BYE
            //UtlString directoryServerUri;
            SipMessage sipByeRequest;
            //if(!inviteFromThisSide)
            //{
            //    UtlString dirAddress;
            //    UtlString dirProtocol;
            //    int dirPort;

            //    sipUserAgent->getDirectoryServer(0, &dirAddress,
            //              &dirPort, &dirProtocol);
            //    SipMessage::buildSipUri(&directoryServerUri,
            //        dirAddress.data(), dirPort, dirProtocol.data());
            //}
            lastLocalSequenceNumber++;
#ifdef TEST_PRINT
            osPrintf("no SDP BYE route: %s\n", mRouteField.data());
#endif
            sipByeRequest.setByeData(mInviteMsg,
                mRemoteContactUri,
                inviteFromThisSide,
                lastLocalSequenceNumber,
                //directoryServerUri.data(),
                mRouteField.data(),
                NULL, // no alsoUri
                mLocalContact.data());

            mLastRequestMethod = SIP_BYE_METHOD;
            send(sipByeRequest);

        }

        // Ignore reINVITE OK responses without SDP
        else
        {
        }

        // Allow ReINVITEs from the other side again
        if(reinviteState == REINVITING)
        {
            reinviteState = ACCEPT_INVITE;
        }

        // If we do not already know what the other side
        // support (i.e. methods)
        if(mAllowedRemote.isNull())
        {
            // Get the methods the other side supports
            response->getAllowField(mAllowedRemote);

            // If the other side did not set the allowed field
            // send an OPTIONS request to see what it supports
            if(mAllowedRemote.isNull())
            {
                lastLocalSequenceNumber++;
                SipMessage optionsRequest;
                optionsRequest.setOptionsData(mInviteMsg,
                    mRemoteContactUri,
                    inviteFromThisSide,
                    lastLocalSequenceNumber,
                    mRouteField.data(), mLocalContact);

                send(optionsRequest);
            }
        }

        // Free up the codec copies and array
        for(int codecIndex = 0; codecIndex < numMatchingCodecs; codecIndex++)
        {
            delete matchingCodecs[codecIndex];
            matchingCodecs[codecIndex] = NULL;
        }
        if(matchingCodecs) delete[] matchingCodecs;
        matchingCodecs = NULL;

        if (sendDelayedRefer)   // send refer AFTER options, gets CSeq's right
        {
            doBlindRefer();
        }
    }   // end 2xx

    // Redirect
    else if(responseCode >= SIP_MULTI_CHOICE_CODE &&
        responseCode < SIP_BAD_REQUEST_CODE &&
        lastLocalSequenceNumber == sequenceNum)
    {
#ifdef TEST_PRINT
        osPrintf("Redirect received\n");
#endif

        // ACK gets sent by the SipUserAgent for error responses
        //SipMessage sipRequest;
        //sipRequest.setAckData(response, mInviteMsg);
        //send(sipRequest);

        // If the call has not already failed
        if(getState() != CONNECTION_FAILED)
        {
            // Get the first contact uri
            UtlString contactUri;
            response->getContactUri(0, &contactUri);
#ifdef TEST_PRINT
            osPrintf("Redirecting to: %s\n", contactUri.data());
#endif

            if(!contactUri.isNull() && mInviteMsg)
            {
                // Create a new INVITE
                lastLocalSequenceNumber++;
                SipMessage sipRequest(*mInviteMsg);
                sipRequest.changeUri(contactUri.data());

                // Don't use the contact in the to field for redirect
                // Use the same To field, but clear the tag
                mToUrl.removeFieldParameter("tag");
                UtlString toField;
                mToUrl.toString(toField);
                sipRequest.setRawToField(toField);
                //sipRequest.setRawToField(contactUri.data());

                // Set incremented Cseq
                sipRequest.setCSeqField(lastLocalSequenceNumber,
                    SIP_INVITE_METHOD);

                // Decrement the max-forwards header
                int maxForwards;
                if(!sipRequest.getMaxForwards(maxForwards))
                {
                    maxForwards = SIP_DEFAULT_MAX_FORWARDS;
                }
                maxForwards--;
                sipRequest.setMaxForwards(maxForwards);

                //reomove all routes
                UtlString route;
                while ( sipRequest.removeRouteUri(0,&route)){}

                //now it is first send
                sipRequest.clearDNSField();
                sipRequest.resetTransport();

                // Get rid of the original invite and save a copy of
                // the new one
                if(mInviteMsg) delete mInviteMsg;
                mInviteMsg = NULL;
                mInviteMsg = new SipMessage(sipRequest);
                inviteFromThisSide = TRUE;

                // As the To field is not modified this is superfluous
                //setCallerId();

                // Send the invite
                if(send(sipRequest))
                {
                    // Change the state back to Offering
                    setState(CONNECTION_OFFERING, CONNECTION_REMOTE, CONNECTION_CAUSE_REDIRECTED);
                    fireSipXEvent(CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL) ;
                }
                else
                {
                    UtlString redirected;
                    ssize_t len;
                    sipRequest.getBytes(&redirected, &len);
#ifdef TEST_PRINT
                    osPrintf("==== Redirected failed ===>%s\n====>\n",
                        redirected.data());
#endif
                    // The send failed
                    setState(CONNECTION_FAILED, CONNECTION_REMOTE, CONNECTION_CAUSE_NETWORK_NOT_OBTAINABLE);
                    fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NETWORK) ;
                }
            }

            // don't know how we got here, this is bad
            else
            {
                setState(CONNECTION_FAILED, CONNECTION_REMOTE);
                fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_UNKNOWN) ;
            }
        }
    }       // end REDIRECT

    else
    {
        UtlString responseText;
        response->getResponseStatusText(&responseText);
#ifdef TEST_PRINT
        osPrintf("Ignoring INVITE response: %d %s\n", responseCode,
            responseText.data());
#endif
        if(responseCode >= SIP_OK_CODE &&
            (lastLocalSequenceNumber == sequenceNum || mIsReferSent))
        {
#ifdef TEST_PRINT
            // I do not understand what cases go through this else
            // so this is just a error message for now.
            if(responseCode < SIP_3XX_CLASS_CODE)
            {
                osPrintf("ERROR: SipConnection::processInviteResponse sending ACK for failed INVITE\n");
            }
#endif

            // Send an ACK
            SipMessage sipRequest;
            sipRequest.setAckData(response,mInviteMsg);
            send(sipRequest);
            if(reinviteState == REINVITED)
            {
#ifdef TEST_PRINT
                osPrintf("ReINVITE ACK - ReINVITE allowed again\n");
#endif
                reinviteState = ACCEPT_INVITE;
            }
        }
    }   // end ACK for not-understood response case

    // If we have completed a REFER based transfer
    // Send the status back to the original call & connection
    // as to whether it succeeded or not.
#ifdef TEST_PRINT
    UtlString connState;
    getStateString(getState(), &connState);
    osPrintf("SipConnection::processResponse originalConnectionAddress: %s connection state: %s\n",
        mOriginalCallConnectionAddress.data(), connState.data());
    getStateString(previousState, &connState);
    osPrintf("SipConnection::processResponse originalConnectionAddress: %s previous state: %s\n",
        mOriginalCallConnectionAddress.data(), connState.data());
#endif
    int currentState = getState();
    if(previousState != currentState &&
        !mOriginalCallConnectionAddress.isNull())
    {
        if(currentState == CONNECTION_ESTABLISHED ||
            currentState == CONNECTION_FAILED ||
            currentState == CONNECTION_ALERTING)
        {
            UtlString idOfOriginalCall;
            mpCall->getIdOfOrigCall(idOfOriginalCall);
            CpMultiStringMessage transfereeStatus(CallManager::CP_TRANSFEREE_CONNECTION_STATUS,
                idOfOriginalCall.data(),
                mOriginalCallConnectionAddress.data(),
                NULL, NULL, NULL,
                currentState, responseCode);
#ifdef TEST_PRINT
            osPrintf("SipConnection::processResponse posting CP_TRANSFEREE_CONNECTION_STATUS to call: %s\n",
                idOfOriginalCall.data());
#endif
            mpCallManager->postMessage(transfereeStatus);

            // This is the end of the transfer meta event
            // whether successful or not
            //mpCall->stopMetaEvent();

        }
    }
} // End of processInviteResponse

void SipConnection::processReferResponse(const SipMessage* response)
{
    int state = CONNECTION_UNKNOWN;
    int cause = CONNECTION_CAUSE_UNKNOWN;
    int responseCode = response->getResponseStatusCode();

    // 2xx class responses are no-ops as it only indicates
    // the transferee is attempting to INVITE the transfer target
    if(responseCode == SIP_OK_CODE)
    {
        state = CONNECTION_DIALING;
        cause = CONNECTION_CAUSE_NORMAL;

        fireSipXEvent(CALLSTATE_TRANSFER, CALLSTATE_TRANSFER_ACCEPTED) ;
    }
    else if(responseCode == SIP_ACCEPTED_CODE)
    {
        state = CONNECTION_OFFERING;
        cause = CONNECTION_CAUSE_NORMAL;

        fireSipXEvent(CALLSTATE_TRANSFER, CALLSTATE_TRANSFER_ACCEPTED) ;
    }
    else if(responseCode == SIP_DECLINE_CODE)
    {
        state = CONNECTION_FAILED;
        cause = CONNECTION_CAUSE_CANCELLED;

        fireSipXEvent(CALLSTATE_TRANSFER, CALLSTATE_TRANSFER_FAILURE) ;
    }
    else if(responseCode == SIP_BAD_METHOD_CODE ||
        responseCode == SIP_UNIMPLEMENTED_METHOD_CODE)
    {
        state = CONNECTION_FAILED;
        cause = CONNECTION_CAUSE_INCOMPATIBLE_DESTINATION;

        fireSipXEvent(CALLSTATE_TRANSFER, CALLSTATE_TRANSFER_FAILURE) ;
    }
    else if(responseCode >= SIP_MULTI_CHOICE_CODE)
    {
        state = CONNECTION_FAILED;
        cause = CONNECTION_CAUSE_BUSY;

        fireSipXEvent(CALLSTATE_TRANSFER, CALLSTATE_TRANSFER_FAILURE) ;
    }

#if 0
    // REFER is not supported, try BYE 'Also'
    if(responseCode == SIP_BAD_METHOD_CODE  ||
        responseCode == SIP_UNIMPLEMENTED_METHOD_CODE)
    {
        UtlString thisAddress;
        response->getFromField(&thisAddress);
        doHangUp(mTargetCallConnectionAddress.data(), thisAddress.data());

        // If we do not know what the other side supports
        // set it to BYE only so that we do not try REFER again
        if(mAllowedRemote.isNull())
            mAllowedRemote = SIP_BYE_METHOD;
        else
        {
            int pos = mAllowedRemote.index("REFER", 0, UtlString::ignoreCase);

            if (pos >= 0)
            {
                mAllowedRemote.remove(pos, 6);
            }
        }
    }
#endif

    // We change state on the target/consultative call
    if(responseCode >= SIP_OK_CODE)
    {
        // Signal the connection in the target call with the final status
        UtlString targetCallId;
        UtlString toField;
        mToUrl.toString(toField);
        mpCall->getTargetCallId(targetCallId);
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipConnection::processReferResponse "
                      "callId %s, state %d, cause %d",
                      targetCallId.data(), state, cause);
        CpMultiStringMessage
            transferControllerStatus(CallManager::CP_TRANSFER_CONNECTION_STATUS,
                                     targetCallId.data(),
                                     toField.data(),
                                     NULL, NULL, NULL,
                                     state, cause);
#ifdef TEST_PRINT
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipConnection::processReferResponse "
                      "posting CP_TRANSFER_CONNECTION_STATUS "
                      "to target call: %s",
                      targetCallId.data());
#endif
        mpCallManager->postMessage(transferControllerStatus);

        // Drop this connection, the transfer succeeded
        // Do the drop at the last possible momment so that
        // both calls have some overlap.
        if(responseCode == SIP_OK_CODE)
        {
            doHangUp();
        }
    }

#ifdef TEST_PRINT
    // We ignore provisional response
    // as they do not indicate any state change
    else
    {
        osPrintf("SipConnection::processResponse ignoring REFER response %d\n",
            responseCode);
    }
#endif
} // End of processReferResponse

void SipConnection::processNotifyResponse(const SipMessage* response)
{
#ifdef TEST_PRINT
    int responseCode = response->getResponseStatusCode();
    osPrintf("SipConnection::processNotifyResponse NOTIFY response: %d ignored\n",
        responseCode);
#else
    response->getResponseStatusCode();
#endif
}

void SipConnection::processOptionsResponse(const SipMessage* response)
{
    int responseCode = response->getResponseStatusCode();
    UtlString responseText;
    int sequenceNum;
    UtlString sequenceMethod;

    response->getResponseStatusText(&responseText);
    response->getCSeqField(&sequenceNum, &sequenceMethod);

    if(responseCode == SIP_OK_CODE &&
        lastLocalSequenceNumber == sequenceNum)
        response->getAllowField(mAllowedRemote);

    // It seems the other side does not support OPTIONS
    else if(responseCode > SIP_OK_CODE &&
        lastLocalSequenceNumber == sequenceNum)
    {
        if( mWaitingForKeepAliveResponse &&
           (responseCode == SIP_NOT_FOUND_CODE ||
           responseCode == SIP_REQUEST_TIMEOUT_CODE ||
           responseCode == SIP_BAD_TRANSACTION_CODE))
        {
            // We got an error response from the far end for an in-dialog OPTION we send.
            // This indicates that the far-end was unable to find the dialog associated
            // with this call. Mark this call as failed.
            setState(CONNECTION_FAILED, CONNECTION_REMOTE, CONNECTION_CAUSE_UNKNOWN);
        }
        response->getAllowField(mAllowedRemote);

        // Assume default minimum
        if(mAllowedRemote.isNull())
            mAllowedRemote = "INVITE, BYE, ACK, CANCEL, REFER";
    }
#ifdef TEST_PRINT
    else
        osPrintf("%s response ignored: %d %s invalid cseq last: %d\n",
        sequenceMethod.data(),
        responseCode,
        responseText.data(),
        lastLocalSequenceNumber);
#endif

    // Ignore 1xx provisional responses
    if (responseCode >= SIP_2XX_CLASS_CODE)
    {
       // Reset the keepalive response flag
       mWaitingForKeepAliveResponse=FALSE;
    }

} // End of processOptionsResponse

void SipConnection::processByeResponse(const SipMessage* response)
{
    // Set a timer so that if we get a 100 and never get a
    // final response, we still tear down the connection
    int responseCode = response->getResponseStatusCode();
    if(responseCode == SIP_TRYING_CODE)
    {
        UtlString localAddress;
        UtlString remoteAddress;
        UtlString callId;
        getFromField(&localAddress);
        getToField(&remoteAddress);
        getCallId(&callId);

        CpMultiStringMessage* expiredBye =
            new CpMultiStringMessage(CallManager::CP_FORCE_DROP_CONNECTION,
            callId.data(), remoteAddress.data(), localAddress.data());
        OsTimer* timer = new OsTimer((mpCallManager->getMessageQueue()),
            (void*)expiredBye);
        // Convert from mSeconds to uSeconds
#ifdef TEST_PRINT
        osPrintf("Setting BYE timeout to %d seconds\n",
            sipUserAgent->getSipStateTransactionTimeout() / 1000);
#endif
        OsTime timerTime(sipUserAgent->getSipStateTransactionTimeout() / 1000, 0);
        timer->oneshotAfter(timerTime);
    }

    else if(responseCode >= SIP_2XX_CLASS_CODE)
    {
        // All final codes are treated the same, since if attempting the
        // BYE fails, for safety, we need to terminate the call anyway.
        // Stop sending & receiving RTP
        if (mpMediaInterface != NULL)
        {
            mpMediaInterface->stopRtpSend(mConnectionId);
            mpMediaInterface->stopRtpReceive(mConnectionId);
            fireSipXEvent(CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP);
        }
    }
} // End of processByeResponse

void SipConnection::processCancelResponse(const SipMessage* response)
{
    // Set a timer so that if we get a 100 and never get a
    // final response, we still tear down the connection
    int responseCode = response->getResponseStatusCode();
    if(responseCode == SIP_TRYING_CODE)
    {
        UtlString localAddress;
        UtlString remoteAddress;
        UtlString callId;
        getFromField(&localAddress);
        getToField(&remoteAddress);
        getCallId(&callId);

        CpMultiStringMessage* expiredBye =
            new CpMultiStringMessage(CallManager::CP_FORCE_DROP_CONNECTION,
            callId.data(), remoteAddress.data(), localAddress.data());

        OsTimer* timer = new OsTimer((mpCallManager->getMessageQueue()),
            (void*)expiredBye);
        // Convert from mSeconds to uSeconds
#ifdef TEST_PRINT
        osPrintf("Setting CANCEL timeout to %d seconds\n",
            sipUserAgent->getSipStateTransactionTimeout() / 1000);
#endif
        OsTime timerTime(sipUserAgent->getSipStateTransactionTimeout() / 1000, 0);
        timer->oneshotAfter(timerTime);
    }
    else if(responseCode >= SIP_2XX_CLASS_CODE)
    {
        // Stop sending & receiving RTP
        if (mpMediaInterface != NULL)
        {
            mpMediaInterface->stopRtpSend(mConnectionId);
            mpMediaInterface->stopRtpReceive(mConnectionId);
            fireSipXEvent(CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP);
        }
    }

} // End of processCancelResponse



void SipConnection::setCallerId()
{
    UtlString newCallerId;

    if(mInviteMsg)
    {
        UtlString user;
        UtlString addr;
        //UtlString fromProtocol;
        Url uri;
        UtlString userLabel;
        //int port;
        if(!inviteFromThisSide)
        {
            mInviteMsg->getFromUrl(mToUrl);
            uri = mToUrl;
            mInviteMsg->getToUrl(mFromUrl);
            mInviteMsg->getRequestUri(&mRemoteUriStr);

#ifdef TEST_PRINT
            UtlString fromString;
            UtlString toString;
            mToUrl.toString(toString);
            mFromUrl.toString(fromString);
            osPrintf("SipConnection::setCallerId INBOUND to: %s from: %s\n",
                toString.data(), fromString.data());
#endif
        }
        else
        {
            mInviteMsg->getToUrl(mToUrl);
            uri = mToUrl;
            mInviteMsg->getFromUrl(mFromUrl);
            mInviteMsg->getRequestUri(&mLocalUriStr);

#ifdef TEST_PRINT
            UtlString fromString;
            UtlString toString;
            mToUrl.toString(toString);
            mFromUrl.toString(fromString);
            osPrintf("SipConnection::setCallerId INBOUND to: %s from: %s\n",
                toString.data(), fromString.data());
#endif
        }

        uri.getHostAddress(addr);
        //port = uri.getHostPort();
        //uri.getUrlParameter("transport", fromProtocol);
        uri.getUserId(user);
        uri.getDisplayName(userLabel);
        // Set the caller ID
        // Use the first that is not empty string of:
        // user label
        // user id
        // host address
        NameValueTokenizer::frontBackTrim(&userLabel, " \t\n\r");
#ifdef TEST_PRINT
        osPrintf("SipConnection::setCallerid label: %s user %s address: %s\n",
            userLabel.data(), user.data(), addr.data());
#endif

        if(!userLabel.isNull())
        {
            newCallerId.append(userLabel.data());
        }
        else
        {
            NameValueTokenizer::frontBackTrim(&user, " \t\n\r");
            if(!user.isNull())
            {
                newCallerId.append(user.data());
            }
            else
            {
                NameValueTokenizer::frontBackTrim(&addr, " \t\n\r");
                newCallerId.append(addr.data());
            }
        }
    }
    Connection::setCallerId(newCallerId.data());
}

UtlBoolean SipConnection::processNewFinalMessage(SipUserAgent* sipUa,
                                                 OsMsg* eventMessage)
{
    UtlBoolean sendSucceeded = FALSE;

    int msgType = eventMessage->getMsgType();
    int msgSubType = eventMessage->getMsgSubType();
    const SipMessage* sipMsg = NULL;

    if(msgType == OsMsg::PHONE_APP &&
        msgSubType == CallManager::CP_SIP_MESSAGE)
    {
        sipMsg = ((SipMessageEvent*)eventMessage)->getMessage();
        int port;
        int sequenceNum;
        UtlString method;
        UtlString address;
        UtlString protocol;
        UtlString user;
        UtlString userLabel;
        UtlString tag;
        UtlString sequenceMethod;
        sipMsg->getToAddress(&address, &port, &protocol, &user, &userLabel, &tag);
        sipMsg->getCSeqField(&sequenceNum, &method);

        int responseCode = sipMsg->getResponseStatusCode();

        // INVITE to create a connection
        //if to tag is already set then return 481 error
        if(method.compareTo(SIP_INVITE_METHOD) == 0 &&
            !tag.isNull() &&
            responseCode == SIP_OK_CODE)
        {
            UtlString fromField;
            UtlString toField;
            UtlString uri;
            UtlString callId;

            sipMsg->getFromField(&fromField);
            sipMsg->getToField(&toField);
            sipMsg->getContactUri( 0 , &uri);
            if(uri.isNull())
                uri.append(toField.data());

            sipMsg->getCallIdField(&callId);
            SipMessage* ackMessage = new SipMessage();
            ackMessage->setAckData(uri,
                fromField,
                toField,
                callId,
                sequenceNum);
            sendSucceeded = sipUa->send(*ackMessage);
            delete ackMessage;

            if (sendSucceeded)
            {
                SipMessage* byeMessage = new SipMessage();
                byeMessage->setByeData(uri,
                    fromField,
                    toField,
                    callId,
                    NULL,
                    sequenceNum + 1);

                sendSucceeded = sipUa->send(*byeMessage);
                delete byeMessage;
            }
        }

    }
    return sendSucceeded;
}


void SipConnection::setContactType(ContactType eType)
{
    mContactType = eType ;
    if (mpMediaInterface != NULL)
    {
        mpMediaInterface->setContactType(mConnectionId, eType) ;
    }

    UtlString localContact;
    buildLocalContact(mFromUrl, localContact);
    mLocalContact = localContact ;

    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipConnection::setContactType "
                  "contact type %d contactUrl '%s'",
                  eType, localContact.data());
}

/* ============================ ACCESSORS ================================= */

UtlBoolean SipConnection::getRemoteAddress(UtlString* remoteAddress) const
{
    return(getRemoteAddress(remoteAddress, FALSE));
}


UtlBoolean SipConnection::getRemoteAddress(UtlString* remoteAddress,
                                           UtlBoolean leaveFieldParmetersIn) const
{
    // leaveFieldParmetersIn gives the flexibility of getting the
    // tag when the connection is still an early dialog

    int remoteState = getState();
    // If this is an early dialog or we explicily want the
    // field parameters
    if(leaveFieldParmetersIn ||
        remoteState == CONNECTION_ESTABLISHED ||
        remoteState == CONNECTION_DISCONNECTED ||
        remoteState == CONNECTION_FAILED ||
        remoteState == CONNECTION_UNKNOWN)
    {
        // Cast as the toString method is not const
        ((Url)mToUrl).toString(*remoteAddress);
    }

    else
    {
        Url toNoFieldParameters(mToUrl);
        toNoFieldParameters.removeFieldParameters();
        toNoFieldParameters.toString(*remoteAddress);
    }

#ifdef TEST_PRINT
    osPrintf("SipConnection::getRemoteAddress address: %s\n",
        remoteAddress->data());
#endif

    return(mInviteMsg != NULL);
}

UtlBoolean SipConnection::isSameRemoteAddress(Url& remoteAddress) const
{
    return(isSameRemoteAddress(remoteAddress, TRUE));
}

UtlBoolean SipConnection::isSameRemoteAddress(Url& remoteAddress,
                                              UtlBoolean tagsMustMatch) const
{
    UtlBoolean isSame = FALSE;

    int remoteState = getState();
    // If this is an early dialog or we explicily want the
    // field parameters
    Url mToUrlTmp(mToUrl);

    if(tagsMustMatch ||
        remoteState == CONNECTION_ESTABLISHED ||
        remoteState == CONNECTION_DISCONNECTED ||
        remoteState == CONNECTION_FAILED ||
        remoteState == CONNECTION_UNKNOWN)
    {
        isSame = SipMessage::isSameSession(mToUrlTmp, remoteAddress);
    }
    else
    {
        // The do not requrie a tag in the remote address
        isSame = SipMessage::isSameSession(remoteAddress, mToUrlTmp);
    }

    return(isSame);
}

UtlBoolean SipConnection::getSession(SipSession& session)
{
    UtlString callId;
    getCallId(&callId);
    SipSession ssn;
    ssn.setCallId(callId.data());
    ssn.setLastFromCseq(lastLocalSequenceNumber);
    ssn.setLastToCseq(lastRemoteSequenceNumber);
    ssn.setFromUrl(mFromUrl);
    ssn.setToUrl(mToUrl);
    ssn.setLocalContact(Url(mLocalContact.data(), FALSE));
    ssn.setRemoteContact(Url(mRemoteContact.data(), FALSE));
    ssn.setProvisionalToTags(mProvisionalToTags);

    if (!mRemoteUriStr.isNull())
        ssn.setRemoteRequestUri(mRemoteUriStr);
    if (!mLocalUriStr.isNull())
        ssn.setLocalRequestUri(mLocalUriStr);

    session = ssn;
    return(TRUE);
}

int SipConnection::getNextCseq()
{
    lastLocalSequenceNumber++;
    return(lastLocalSequenceNumber);
}

OsStatus SipConnection::getFromField(UtlString* fromField)
{
    OsStatus ret = OS_SUCCESS;

    UtlString host;
    mFromUrl.getHostAddress(host);
    if(host.isNull())
        ret = OS_NOT_FOUND;

    mFromUrl.toString(*fromField);

#ifdef TEST_PRINT
    osPrintf("SipConnection::getFromAddress address: %s\n",
        fromField->data());
#endif

    return ret;
}

OsStatus SipConnection::getToField(UtlString* toField)
{
    OsStatus ret = OS_SUCCESS;
    UtlString host;
    mToUrl.getHostAddress(host);
    if (host.isNull())
        ret = OS_NOT_FOUND;

    mToUrl.toString(*toField);

#ifdef TEST_PRINT
    osPrintf("SipConnection::getToAddress address: %s\n",
        toField->data());
#endif

    return ret;
}

OsStatus SipConnection::getInvite(SipMessage* message)
{
    OsStatus ret = OS_SUCCESS;

    // Copy mInviteMsg to the destination.
    *message = *mInviteMsg;

    if (OsSysLog::willLog(FAC_CP, PRI_DEBUG))
    {
       UtlString text;
       ssize_t length;
       mInviteMsg->getBytes(&text, &length);
       OsSysLog::add(FAC_CP, PRI_DEBUG,
                     "SipConnection::getInvite this = %p, mInviteMsg = %p, message = '%s'",
                     this, mInviteMsg, text.data());
    }

    return ret;
}


/* ============================ INQUIRY =================================== */

UtlBoolean SipConnection::willHandleMessage(OsMsg& eventMessage) const
{
    int msgType = eventMessage.getMsgType();
    int msgSubType = eventMessage.getMsgSubType();
    UtlBoolean handleMessage = FALSE;
    const SipMessage* sipMsg = NULL;
    int messageType;

    // Do not handle message if marked for deletion
    if (isMarkedForDeletion())
        return false ;

    if(msgType == OsMsg::PHONE_APP &&
        msgSubType == CallManager::CP_SIP_MESSAGE)
    {
        sipMsg = ((SipMessageEvent&)eventMessage).getMessage();
        messageType = ((SipMessageEvent&)eventMessage).getMessageStatus();

        // If the callId, To and From match it belongs to this message
        if(mInviteMsg && mInviteMsg->isSameSession(sipMsg))
        {
            handleMessage = TRUE;
        }
        else if(mInviteMsg)
        {
            // Trick to reverse the To & From fields
            SipMessage toFromReversed;

            toFromReversed.setByeData(mInviteMsg,
                mRemoteContactUri,
                FALSE, 1, "", NULL, mLocalContact.data());
            if(toFromReversed.isSameSession(sipMsg))
            {
                handleMessage = TRUE;
            }
        }

    }

    return(handleMessage);
}

UtlBoolean SipConnection::isConnection(const char* callId,
                                       const char* toTag,
                                       const char* fromTag,
                                       UtlBoolean  strictCompare) const
{
    UtlBoolean matches = FALSE;

    // Do not handle message if marked for deletion
    if (isMarkedForDeletion())
        return false ;

    if(mInviteMsg)
    {
        UtlString thisCallId;
        mInviteMsg->getCallIdField(&thisCallId);

        if(thisCallId.compareTo(callId) == 0)
        {
            UtlString thisFromTag;
            UtlString thisToTag;
            mFromUrl.getFieldParameter("tag", thisFromTag);
            mToUrl.getFieldParameter("tag", thisToTag);

            if (strictCompare)
            {
                // for transfer target in a consultative call,
                // thisFromTag is remote, thisToTag is local
                if((thisFromTag.compareTo(toTag) == 0 &&
                    thisToTag.compareTo(fromTag) == 0 ))
                {
                    matches = TRUE;
                }
            }

            // Do a sloppy comparison
            //  Allow a match either way
            else
            {
                if((thisFromTag.compareTo(fromTag) == 0 &&
                    thisToTag.compareTo(toTag) == 0 ) ||
                    (thisFromTag.compareTo(toTag) == 0 &&
                    thisToTag.compareTo(fromTag) == 0 ))
                {
                    matches = TRUE;
                }
            }
#ifdef TEST_PRINT
            osPrintf("SipConnection::isConnection toTag=%s\n\t fromTag=%s\n\t thisToTag=%s\n\t thisFromTag=%s\n\t matches=%d\n",
                toTag, fromTag, thisToTag.data(), thisFromTag.data(), (int)matches) ;
#endif
        }
    }

    return(matches);
}

// Determine if the other side of this connection (remote side)
// supports the given method
UtlBoolean SipConnection::isMethodAllowed(const char* method)
{
    // Eventually we may want to send an OPTIONS request if
    // we do not know.  For now assume that the other side
    // sent an Allowed header field in the final response.
    // If we do not know (mAllowedRemote is NULL) assume
    // it is supported.
    UtlBoolean methodSupported = TRUE;
    ssize_t methodIndex = mAllowedRemote.index(method);
    if(methodIndex >=0)
    {
        methodSupported = TRUE;
    }

    // We explicitly know that it does not support this method
    else if(!mAllowedRemote.isNull())
    {
        methodSupported = FALSE;
    }
#ifdef TEST_PRINT
    osPrintf("SipConnection::isMethodAllowed method: %s allowed: %s return: %d index: %ld null?: %d\n",
        method, mAllowedRemote.data(), methodSupported, methodIndex,
        mAllowedRemote.isNull());
#endif

    return(methodSupported);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void SipConnection::proceedToRinging(const SipMessage* inviteMessage,
                                     SipUserAgent* sipUserAgent, int tagNum,
                                     int availableBehavior)
{
    UtlString name = mpCall->getName();
#ifdef TEST_PRINT
    osPrintf("%s SipConnection::proceedToRinging\n", name.data());
#endif

    // Send back a ringing INVITE response
    SipMessage sipResponse;
    sipResponse.setInviteRingingData(inviteMessage, mLocalContact);
    if(tagNum >= 0)
    {
        sipResponse.setToFieldTag(tagNum);
    }
    if(send(sipResponse))
    {
#ifdef TEST_PRINT
        osPrintf("INVITE Ringing sent successfully\n");
#endif
    }
    else
    {
#ifdef TEST_PRINT
        osPrintf("INVITE Ringing send failed\n");
#endif
    }
}

UtlBoolean SipConnection::send(SipMessage& message,
                    OsMsgQ* responseListener,
                    void* responseListenerData)
{
#ifdef TEST_PRINT
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipConnection::send ");
#endif
    // If we don't know the proper interface, use the stack default
    if (message.getInterfaceIp().length() < 1)
    {
        int port = PORT_NONE;
        UtlString localIp;
        sipUserAgent->getLocalAddress(&localIp, &port);
        message.setInterfaceIpPort(localIp, port);
    }

    // Catch-all: Use derived contact instead of stack-default if
    // not already set
    UtlString ignore;
    if (!message.getContactField(0, ignore))
    {
        message.setContactField(mLocalContact) ;
    }

    return sipUserAgent->send(message, responseListener, responseListenerData);
}
/* ============================ FUNCTIONS ================================= */

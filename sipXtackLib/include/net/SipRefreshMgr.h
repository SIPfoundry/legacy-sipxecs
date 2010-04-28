//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef SIPREFRESHMGR_H
#define SIPREFRESHMGR_H

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsServerTask.h"
#include "net/SipMessage.h"
#include "net/SipTcpServer.h"
#include "net/SipMessageList.h"
#include "net/SipMessageEvent.h"
#include "utl/UtlRandom.h"
#include "utl/UtlHashMap.h"
#include "tapi/sipXtapiEvents.h" /* :TODO: CIRCULAR */
#include "tapi/sipXtapiInternal.h" /* :TODO: CIRCULAR */

#include "net/SipLine.h"


// DEFINES
#define DEFAULT_PERCENTAGE_TIMEOUT 48 //48%
#define FAILED_PERCENTAGE_TIMEOUT 24 //24%
#define SIP_LINE_LINEID "lineID"
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipUserAgent;
class SipLineMgr;

class SipRefreshMgr : public OsServerTask
{
public:
    //INITIALIZE
    UtlBoolean init(
        SipUserAgent *ptrToMyAgent,
        int sipTcpPort = SIP_PORT,
        int sipUdpPort = SIP_PORT,
        const char* defaultUser = NULL,
        const char* publicAddress = NULL,
        const char* defaultSipAddress = NULL,
        const char* sipDirectoryServers = NULL,
        const char* sipRegistryServers = NULL,
        int defaultRegistryTimeout = 3600,      // one hr
        int defaultSubscribeTimeout = 60*60*24, // 24 hrs
        int restartCount = 1,
        const char* macAddress = NULL );

    void StartRefreshMgr();

    /**
     * Mutator for the mDefaultRegistryPeriodMember
     */
    void setRegistryPeriod(const int periodInSeconds);

    /**
     * Mutator for the mDefaultSubscribePeriodMember
     */
    void setSubscribeTimeout(const int periodInSeconds);

    /**
     * Accessor for the mDefaultSubscribePeriodMember
     */
    const int getSubscribeTimeout();

    void addMessageConsumer( OsServerTask* messageEventListener );

    void addMessageObserver (
        OsMsgQ& messageQueue,
        const char* sipMethod = NULL,
        UtlBoolean wantRequests = TRUE,
        UtlBoolean wantResponses = TRUE,
        UtlBoolean wantIncoming = TRUE,
        UtlBoolean wantOutGoing = FALSE,
        const char* eventName = NULL,
        void* observerData = NULL );

    //: Add a SIP message observer for SIP messages meeting the filter criteria
    //! param: messageQueue - the queue on which an SipMessageEvent is dispatched
    //! param: sipMethod - the specific method type of the requests or responses to be observed.  NULL or a null string indicates all methods.
    //! param: wantRequests - want to observe SIP requests
    //! param: wantResponses - want to observe SIP responses
    //! param: wantIncoming - want to observe SIP messages originating from the network.
    //! param: wantOutGoing - want to observe SIP messages originating from locally.
    //! param: eventName - want to observer SUBSCRIBE or NOTIFY requests having the given event type
    //! param: observerData - data to be attached to SIP messages queued on the observer

    //void removeMessageConsumer(OsServerTask* messageConsumer);
    //: Remove a SIP message recipient

    //REGISTER METHODS

    // A method to register/unregister a contact should not be called until
    // any previous operation on that contact has completed, as messages may
    // need to be re-sent with authorization, and CSeq could get out
    // of sequence.

    UtlBoolean newRegisterMsg (
        const Url& fromUrl,
        const UtlString& lineId,
        int registryPeriodSeconds = -1,
        Url* pPreferredContactUri = NULL);

    void reRegisterAll();

    void reRegister ( const Url& fromUrl );

    void unRegisterUser (
        const Url& fromUrl,
        const UtlBoolean& onStartup = FALSE,
        const UtlString& lineid ="" );

    //SUBSRIBE METHODS
    void reSubscribeAll();

    void unSubscribeAll();
      //:Unsubscribe all

    void setLineMgr(SipLineMgr* const lineMgr);
    //: Sets a pointer to the line manager

    SipLineMgr* const getLineMgr() const;
    //: Gets the line manager pointer

    UtlBoolean newSubscribeMsg( SipMessage& message );

    SipRefreshMgr();

    void dumpMessageLists(UtlString& results);
      //:Appends the message contents of both the mRegisterList and
      // mSubscribeList

    virtual ~SipRefreshMgr();

    virtual UtlBoolean handleMessage( OsMsg& eventMessage );

    UtlBoolean getNatMappedAddress(UtlString* pIpAddress, int* pPort);
      //: Get the NAT mapped address (if available)

protected:
    SipLineMgr* mpLineMgr;
    // the line manager object that uses this refresh manager

    // MsgType categories defined for use by the system
    enum RefreshMsgTypes
    {
        UNSPECIFIED = 0,
        START_REFRESH_MGR
    };

    // Common Methods
    UtlBoolean isUAStarted();

    void waitForUA();

    void queueMessageToObservers (
        SipMessageEvent& event,
        const char* method);

    void getFromAddress (
        UtlString* address,
        int* port,
        UtlString* protocol);

    void rescheduleAfterTime (
        SipMessage* message,
        int percentage = DEFAULT_PERCENTAGE_TIMEOUT );

    void sendToObservers (
        const OsMsg& eventMessage,
        SipMessage * registerRequest );

    OsStatus sendRequest (
        SipMessage& registerRequest,
        const char *method);

    void rescheduleRequest (
        SipMessage* registerRequest,
        int secondsFromNow,
        const char* method,
        int percentage = DEFAULT_PERCENTAGE_TIMEOUT,
        UtlBoolean sendImmediate = FALSE );

    void processOKResponse (
        SipMessage* registerResponse,
        SipMessage* registerRequest );

    void parseContactFields (
        SipMessage* message,
        SipMessage* sipRequest,
        int& expireVal );

    void processResponse(
        const OsMsg& eventMessage,
        SipMessage* registerRequest);

    void createTagNameValuePair( UtlString& tagNamevaluePair );

    void generateCallId (
        const UtlString& lineId,
        const UtlString& method,
        UtlString& callid,
        UtlBoolean onStartup = FALSE );

    // register
    void registerUrl(
        const char* registerFromAddress,
        const char* registerToAddress,
        const char* registerUri,
        const char* contactUrl,
        const UtlString& registerCallId,
        int registerPeriod = -1);

    UtlBoolean isDuplicateRegister(
        const Url& url,
        SipMessage& oldMessage );

    UtlBoolean isDuplicateRegister( const Url& url );

    void addToRegisterList( SipMessage* message);

    UtlBoolean removeFromRegisterList( SipMessage* message );
     //: Returns TRUE if message was found and removed from list.
     //: Message is NOT deleted.

    // subscribe
    void addToSubscribeList( SipMessage * message);

    UtlBoolean removeFromSubscribeList( SipMessage* message );
     //: Returns TRUE if message was found and removed from list.
     //: Message is NOT deleted.

    UtlBoolean isDuplicateSubscribe ( const Url& url );

    UtlBoolean isDuplicateSubscribe(
        const Url& fromUrl,
        SipMessage &oldMsg );

    void getContactField(
        const Url& registerToField,
        UtlString& contact,
        const UtlString& lineId = "",
        Url* pPreferredContactUri = NULL);

    void removeAllFromRequestList(SipMessage* response);
    //: Removes all prior request records for this response
    //: from the SipMessageLists (mRegisterList & mSubscribeList)

    void removeAllFromRequestList(SipMessage* response, SipMessageList* pRequestList);
    //: Removes all prior request records for this response
    //: from the passed-in SipMessageList

    UtlBoolean isExpiresZero(SipMessage* pRequest);
      //: Is the expires field set to zero for the specified msg?

    void fireSipXLineEvent(const Url& url, const UtlString& lineId, const SIPX_LINESTATE_EVENT event, const SIPX_LINESTATE_CAUSE cause);
    //: event firing method used to notify sipXtapi of line events

    SIPX_LINESTATE_EVENT getLastLineEvent(const UtlString& lineId);
    //: holding on to the last known line event type

    void setLastLineEvent(const UtlString& lineId, const SIPX_LINESTATE_EVENT eMajor);
    //: sets the last line event type

    UtlHashMap* mpLastLineEventMap;

    // register
    int mDefaultRegistryPeriod;
    SipMessageList mRegisterList;
    OsRWMutex mRegisterListMutexR;
    OsRWMutex mRegisterListMutexW;
    UtlString mRegistryServer;

    // subscribe
    int mDefaultSubscribePeriod;
    SipMessageList mSubscribeList;
    OsRWMutex mSubscribeListMutexR;
    OsRWMutex mSubscribeListMutexW;

    // common
    UtlBoolean mIsStarted;
    UtlHashBag mMessageObservers;
    OsRWMutex mObserverMutex;
    OsMutex mUAReadyMutex;
    UtlString mContactAddress;
    UtlString mDefaultSipAddress;
    UtlString mSipIpAddress;
    UtlString mDefaultUser;
    UtlString mMacAddress;
    UtlString mRestartCountStr;
    SipUserAgent* mMyUserAgent;
    int mTcpPort;
    int mUdpPort;
    int mRestartCount;
    UtlRandom mRandomNumGenerator;
};

#endif // SIPREFRESHMGR_H

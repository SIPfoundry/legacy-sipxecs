//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _CallManager_h_
#define _CallManager_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <cp/CpCallManager.h>
#include <cp/Connection.h>
#include <mi/CpMediaInterfaceFactoryImpl.h>
#include <net/QoS.h>

#include <tao/TaoObjectMap.h>
#include <os/OsProtectEventMgr.h>

// DEFINES
#ifdef LONG_EVENT_RESPONSE_TIMEOUTS
#  define CP_MAX_EVENT_WAIT_SECONDS    2592000    // 30 days in seconds
#else
#  define CP_MAX_EVENT_WAIT_SECONDS    30         // time out, seconds
#endif

#define CP_CALL_HISTORY_LENGTH 50

#define CP_MAXIMUM_RINGING_EXPIRE_SECONDS 180

#define CALL_DELETE_DELAY_SECS  10    // Number of seconds between a drop
                                      // request (call) and call deletion
                                      // (call manager)

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS


// FORWARD DECLARATIONS
class SdpCodec;
class CpCall;
class SipUserAgent;
class OsConfigDb;
class PtMGCP;
class TaoObjectMap;
class TaoReference;
class SdpCodecFactory;
class CpMultiStringMessage;
class SipSession;
class SipDialog;
class SipLineMgr;
class CpMediaInterfaceFactory;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class CallManager : public CpCallManager
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   CallManager(UtlBoolean isRequiredUserIdMatch,
               SipLineMgr* lineMgrTask,
               UtlBoolean isEarlyMediaFor180Enabled,
               SdpCodecFactory* pCodecFactory,
               int rtpPortStart,
               int rtpPortEnd,
               const char* localAddress,
               const char* publicAddress,
               SipUserAgent* userAgent,
               int sipSessionReinviteTimer,               // Suggested value: 0
               PtMGCP* mgcpStackTask,                     // Suggested value: NULL
               const char* defaultCallExtension,          // Suggested value: NULL
               int availableBehavior,                     // Suggested value: Connection::RING
               const char* unconditionalForwardUrl,       // Suggested value: NULL
               int forwardOnNoAnswerSeconds,              // Suggested value: -1
               const char* forwardOnNoAnswerUrl,          // Suggested value: NULL
               int busyBehavior,                          // Suggested value: Connection::BUSY
               const char* sipForwardOnBusyUrl,           // Suggested value: NULL
               OsConfigDb* speedNums,                     // Suggested value: NULL
               CallTypes phonesetOutgoingCallProtocol,    // Suggested value: CallManager::SIP_CALL
               int numDialPlanDigits,                     // Suggested value: 4
               int holdType,                              // Suggested value: CallManager::NEAR_END_HOLD
               int offeringDelay,                         // Suggested value: Connection::IMMEDIATE
               const char* pLocal,                        // Suggested value: ""
               int inviteExpireSeconds,                   // Suggested value: CP_MAXIMUM_RINGING_EXPIRE_SECONDS
               int expeditedIpTos,                        // Suggested value: QOS_LAYER3_LOW_DELAY_IP_TOS
               int maxCalls,                              // Suggested value: 10
               CpMediaInterfaceFactory* pMediaFactory);   // Suggested value: NULL) ;

   virtual
   ~CallManager();
     //:Destructor

/* ============================ MANIPULATORS ============================== */
   virtual OsStatus addTaoListener(OsServerTask* pListener,
                                   char* callId = NULL,
                                   int ConnectId = 0,
                                   int mask = 0);
     //:Register as a listener for call and connection events.

    virtual UtlBoolean handleMessage(OsMsg& eventMessage);

    virtual void requestShutdown(void);

    virtual void setOutboundLine(const char* lineUrl);
    virtual void setOutboundLineForCall(const char* callId,
                                        const char* address,
                                        ContactType eType = ContactAddress::AUTO);

    // Operations for calls
    virtual void createCall(UtlString* callId,
                            int metaEventId = 0,
                            int metaEventType = PtEvent::META_EVENT_NONE,
                            int numMetaEventCalls = 0,
                            const char* metaEventCallIds[] = NULL,
                            UtlBoolean assumeFocusIfNoInFocusCall = TRUE);
    virtual OsStatus getCalls(int maxCalls, int& numCalls, UtlString callIds[]);

    virtual PtStatus connect(const char* callId,
                             const char* toAddress,
                             const char* fromAddress = NULL,
                             const char* desiredConnectionCallId = NULL,
                             ContactId contactId = 0,
                             const void* pDisplay = NULL,
                             const bool sendPAIheader = 0) ;

    virtual PtStatus consult(const char* idleTargetCallId,
        const char* activeOriginalCallId, const char* originalCallControllerAddress,
        const char* originalCallControllerTerminalId, const char* consultAddressUrl,
        UtlString& targetCallControllerAddress, UtlString& targetCallConsultAddress);
    virtual void drop(const char* callId);
    virtual PtStatus transfer_blind(const char* callId, const char* transferToUrl,
                                    UtlString* targetCallId,
                                    UtlString* targetConnectionAddress = NULL,
                                    bool       remoteHoldBeforeTransfer = true
                                    );
    // Blind transfer

    virtual PtStatus transfer(const char* targetCallId, const char* originalCallId);
    // Consultative transfer
    // The couple targetCallId & targetConnectionAddress return/define
    // the transfer target connection in the resulting new transfer
    // target call

    PtStatus transfer(const char* sourceCallId,
                      const char* sourceAddress, // name-addr
                      const char* targetCallId,
                      const char* targetAddress, // name-addr
                      bool        remoteHoldBeforeTransfer = true
                      ) ;
    //: Transfer an individual participant from one end point to another using
    //: REFER w/replaces.


    virtual PtStatus split(const char* szSourceCallId,
                           const char* szSourceAddress,
                           const char* szTargetCallId) ;
    //: Split szSourceAddress from szSourceCallId and join it to the specified
    //: target call id.  The source call/connection MUST be on hold prior
    //: to initiating the split/join.


    virtual void toneStart(const char* callId, int toneId, UtlBoolean local, UtlBoolean remote);
    virtual void toneStop(const char* callId);
    virtual void audioPlay(const char* callId, const char* audioUrl, UtlBoolean repeat, UtlBoolean local, UtlBoolean remote);
    virtual void bufferPlay(const char* callId, char* audiobuf, int bufSize, int type, UtlBoolean repeat, UtlBoolean local, UtlBoolean remote);
    virtual void audioStop(const char* callId);
    virtual void stopPremiumSound(const char* callId);
#ifndef EXCLUDE_STREAMING
    virtual void createPlayer(const char* callid, MpStreamPlaylistPlayer** ppPlayer) ;
    virtual void createPlayer(int type, const char* callid, const char* szStream, int flags, MpStreamPlayer** ppPlayer) ;
    virtual void destroyPlayer(const char* callid, MpStreamPlaylistPlayer* pPlayer)  ;
    virtual void destroyPlayer(int type, const char* callid, MpStreamPlayer* pPlayer)  ;
#endif


    // Operations for calls & connections
    virtual void acceptConnection(const char* callId,
                                  const char* address,
                                  ContactType contactType = ContactAddress::AUTO,
                                  const void* hWnd = NULL);

    virtual void rejectConnection(const char* callId,
                                  const char* address,
                                  int         errorCode = SIP_BUSY_CODE,
                                  const char* errorText = SIP_BUSY_TEXT);
    virtual PtStatus redirectConnection(const char* callId, const char* address, const char* forwardAddressUrl);
    // Note that dropConnection() drops only one connection (call leg), not the entire call.
    // Use drop() to drop an entire call.
    virtual void dropConnection(const char* callId, const char* address);
    virtual void getNumConnections(const char* callId, int& numConnections);
        virtual OsStatus getConnections(const char* callId, int maxConnections,
                int& numConnections, UtlString addresses[]);
    virtual OsStatus getCalledAddresses(const char* callId, int maxConnections,
                int& numConnections, UtlString addresses[]);
    virtual OsStatus getCallingAddresses(const char* callId, int maxConnections,
                int& numConnections, UtlString addresses[]);

    // Operations for calls & terminal connections
    virtual void answerTerminalConnection(const char* callId, const char* address, const char* terminalId,
                                          const void* pDisplay = NULL);
    virtual void holdTerminalConnection(const char* callId, const char* address, const char* terminalId);
    virtual void holdAllTerminalConnections(const char* callId);
    virtual void holdLocalTerminalConnection(const char* callId);
    virtual void unholdLocalTerminalConnection(const char* callId);
    virtual void unholdAllTerminalConnections(const char* callId);
    virtual void unholdTerminalConnection(const char* callId, const char* addresss, const char* terminalId);
    virtual void renegotiateCodecsTerminalConnection(const char* callId, const char* addresss, const char* terminalId);
    virtual void renegotiateCodecsAllTerminalConnections(const char* callId);
    virtual void sendKeepAlive(const char* callId, UtlBoolean useOptionsForKeepalive);
         virtual void getNumTerminalConnections(const char* callId, const char* address, int& numTerminalConnections);
         virtual OsStatus getTerminalConnections(const char* callId, const char* address,
                int maxTerminalConnections, int& numTerminalConnections, UtlString terminalNames[]);
    virtual UtlBoolean isTerminalConnectionLocal(const char* callId, const char* address, const char* terminalId);
    virtual OsStatus getSession(const char* callId,
                                const char* address,
                                SipSession& session);

    virtual OsStatus getSipDialog(const char* callId,
                                  const char* address,
                                  SipDialog& dialog);

    virtual OsStatus getInvite(const char* callId,
                               const char* address,
                               SipMessage& invite);

    // Stimulus based operations DEPRICATED DO NOT USE
    virtual void unhold(const char* callId);
    virtual void dialString(const char* url);

    virtual UtlBoolean disconnectConnection(const char* callId, const char* addressUrl);

    virtual void setTransferType(int type);

    virtual void addToneListener(const char* callId, void* pListener);

    virtual void removeToneListener(const char* callId, void* pListener);

         // Change the function name from getDtmfTone to enableDtmfEvent, to better
         // reflect what it does.
         // There are three functions used to process dtmf events for a client:
         //             -- enableDtmfEvent: registers a dtmfEvent in CpCall (adding it to
         //                     CpCall's mDtmfEvents list, set 'enabled' flag to TRUE.  When a dtmf
         //                     notification by MprRecoreder to the CpCall is recieved, CpCall
         //                     will signal it to this event.
         //             -- disableDtmfEvent: temporarily remove the event from the CpCall's
         //                     mDtmfEvents list so that this client is not going to receive any
         //                     dtmf notification until enableDtmfEvent is called again. this
         //                     function does not delete the event object, it just sets the 'enabled'
         //                     flag to FALSE.
         //             -- removeDtmfEvent: pernmanently remove the dtmf event object from
         //                     CpCall's mDtmfEvents list and delete the event object.

    virtual OsStatus enableDtmfEvent(const char* callId,
                                     int interDigitSecs,
                                     OsQueuedEvent* dtmfEvent,
                                     UtlBoolean ignoreKeyUp);

    virtual void disableDtmfEvent(const char* callId, void* dtmfEvent);

    virtual void removeDtmfEvent(const char* callId, void* dtmfEvent);


    virtual OsStatus ezRecord(const char* callId,
                        int ms,
                        int silenceLength,
                        double& duration,
                        const char* fileName,
                        int& dtmfterm,
                        OsProtectedEvent* recordEvent = NULL);

    virtual OsStatus setCodecCPULimitCall(const char* callId, int limit, UtlBoolean bRenegotiate) ;
      //:Sets the CPU codec limit for a call.  Each connection within the call
      //:may only use a codec of the specified CPU intensity (or lesser).

    virtual OsStatus setInboundCodecCPULimit(int limit)  ;
      //:Sets the inbound call CPU limit for codecs

    virtual OsStatus stopRecording(const char* callId);
    //: tells media system stop stop a curretn recording

    virtual void setMaxCalls(int maxCalls);
    //:Set the maximum number of calls to admit to the system.

    virtual void enableStun(const char* szStunServer,
                            int iKeepAlivePeriodSecs,
                            int stunOptions,
                            OsNotification *pNotification = NULL) ;
    //:Enable STUN for NAT/Firewall traversal

    virtual void sendInfo(const char* callId,
                           const char* szContentType,
                           const size_t nContenLength,
                           const char*  szContent);
   //: Sends an INFO message to the other party(s) on the call

/* ============================ ACCESSORS ================================= */
   /**
    * Gets the number of lines made available by line manager.
    */
   virtual int getNumLines();

   /**
    * Gets the call manager's line manager.  Usage of a line manager is
    * optional and callers should expect a null return code.
    */
   virtual SipLineMgr* getLineManager() ;

  /**
   * maxAddressesRequested is the number of addresses requested if available
   * numAddressesAvailable is the actual number of addresses available.
   * "addresses" is a pre-allocated array of size maxAddressesRequested
   */
   virtual OsStatus getOutboundAddresses(int maxAddressesRequested,
                                       int& numAddressesAvailable, UtlString** addresses);

   virtual UtlBoolean getCallState(const char* callId, int& state);
        virtual UtlBoolean getConnectionState(const char* callId, const char* remoteAddress, int& state);

        virtual UtlBoolean getTermConnectionState(const char* callId,
                                                                                        const char* address,
                                                                                        const char* terminal,
                                                                                        int& state);

    UtlBoolean getNextSipCseq(const char* callId,
                             const char* remoteAddress,
                             int& nextCseq);
    //:provides the next csequence number for a given call session (leg) if it exists.
    // Note: this does not protect against transaction

    void printCalls(int showHistory = 1);

    void setOutGoingCallType(int callType);

    int getTransferType();

    virtual PtStatus validateAddress(UtlString& address);

    virtual void startCallStateLog();

    virtual void stopCallStateLog();

    virtual void setCallStateLogAutoWrite(UtlBoolean);

    virtual void clearCallStateLog();

    virtual void logCallState(const char* message,
                              const char* eventId,
                              const char* cause);


    // Get the contents of the call state log.
    virtual void getCallStateLog(UtlString& logData);
    // Get the contents of the call state log and clear it.
    // (Avoids the race condition of calling getCallStateLog then
    // clearCallStateLog.)
    virtual void getAndClearCallStateLog(UtlString& logData);
    // Flush the CallStateLog in the same way that auto-writing does
    // (by writing it to the log file).
    virtual void flushCallStateLogAutoWrite();

    // Soon to be removed:
    virtual OsStatus getFromField(const char* callId, const char* remoteAddress,  UtlString& fromField);
    virtual OsStatus getToField(const char* callId, const char* remoteAddress,  UtlString& toField);
    virtual OsStatus getRemoteContactField(const char* callId, const char* remoteAddress,  UtlString& remoteContactField);

   virtual OsStatus getCodecCPUCostCall(const char* callId, int& cost);
     //:Gets the CPU cost for an individual connection within the specified
     //:call.

   virtual OsStatus getCodecCPULimitCall(const char* callId, int& cost);
     //:Gets the CPU cost for an individual connection within the specified
     //:call.
     // It is assumed that each connection is at worst this level; however,
     // may actually use less resources.

   virtual void getCalls(int& currentCalls, int& maxCalls);
     //:Get the current number of calls in the system and the maximum number of
     //:calls to be admitted to the system.

   virtual OsStatus getLocalContactAddresses(const char* callid,
                                             ContactAddress addresses[],
                                             size_t  nMaxAddresses,
                                             size_t& nActaulAddresses) ;
     //:The available local contact addresses

   virtual CpMediaInterfaceFactory* getMediaInterfaceFactory() ;
     //: Gets the media interface factory used by the call manager

   virtual int getMediaConnectionId(const char* szCallId, const char* remoteAddress, void** ppInstData = NULL);
   //: Gets the Media Connection ID
   //: @param szCallId The call-id string of the call with which the connection id is associated
   //: @param remoteAddress The remote address of the call's connection, with which the connection id is associated
   //: @param ppInstData Reserved for future use.

   virtual UtlBoolean canAddConnection(const char* szCallId);
   //: Can a new connection be added to the specified call?  This method is
   //: delegated to the media interface.

   virtual void setDelayInDeleteCall(int delayInDeleteCall);
   //: Set the number of seconds to delay in deleting the call

   virtual int getDelayInDeleteCall();
   //: Get the number of seconds to delay in deleting the call

/* ============================ INQUIRY =================================== */
   int getTotalNumberOutgoingCalls() { return mnTotalOutgoingCalls;}
   int getTotalNumberIncomingCalls() { return mnTotalIncomingCalls;}

   SipUserAgent* getUserAgent() { return sipUserAgent;}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
        TaoListenerDb**                 mpListeners;
        int                                             mListenerCnt;
        int                                             mMaxNumListeners;

        void addTaoListenerToCall(CpCall* pCall);

        OsStatus addThisListener(OsServerTask* pListener,
                                 char* callId,
                                 int mask);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    UtlString mOutboundLine;
    UtlBoolean dialing;
    UtlBoolean mOffHook;
    UtlBoolean speakerOn;
    UtlBoolean flashPending;
    SipUserAgent* sipUserAgent;
    int mSipSessionReinviteTimer;
    CpCall* infocusCall;
    UtlSList mCallStack;
    UtlString mDialString;
    int mOutGoingCallType;
    PtMGCP* mpMgcpStackTask;
    int mNumDialPlanDigits;
    int mHoldType;
    SdpCodecFactory* mpCodecFactory;
    int mTransferType;                  // while this does get set, I can't find any code that uses it to decide anything (ke)
    UtlString mLocale;
    int mMessageEventCount;
    int mnTotalIncomingCalls;
    int mnTotalOutgoingCalls;
    int mExpeditedIpTos;
    UtlString mCallManagerHistory[CP_CALL_HISTORY_LENGTH];
    UtlBoolean mIsEarlyMediaFor180;
    SipLineMgr*       mpLineMgrTask;     //Line manager
    UtlBoolean mIsRequredUserIdMatch;
    // mMaxCalls can be changed by code running in other threads.
    volatile int mMaxCalls;

    UtlString mStunServer ;
    int mStunOptions ;
    int mStunKeepAlivePeriodSecs ;
    CpMediaInterfaceFactory* mpMediaFactory;

    // Delay in deleting the call
    int mDelayInDeleteCall;

    // Private accessors
    void pushCall(CpCall* call);
    CpCall* popCall();
    CpCall* removeCall(CpCall* call);
    int getCallStackSize();
    UtlBoolean changeCallFocus(CpCall* callToTakeFocus);
    CpCall* findHandlingCall(const char* callId);
    CpCall* findHandlingCall(int callIndex);
    CpCall* findHandlingCall(const OsMsg& eventMessage);
    CpCall* findFirstQueuedCall();
    void getCodecs(int& numCodecs, SdpCodec**& codecArray);
    void addHistoryEvent(const char* messageLogString);

    void doHold();

    void doCreateCall(const char* callId,
                        int metaEventId = 0,
                        int metaEventType = PtEvent::META_EVENT_NONE,
                        int numMetaEventCalls = 0,
                        const char* metaEventCallIds[] = NULL,
                        UtlBoolean assumeFocusIfNoInfocusCall = TRUE);

    void doConnect(const char* callId,
                   const char* addressUrl,
                   const char* szDesiredConnectionCallId,
                   ContactId contactId = 0,
                   const void* pDisplay = NULL,
                   const bool sendPAIheader = 0 );

    void doSendInfo(const char* callId,
                           const char* szContentType,
                           UtlString   sContent);
    //:Called by the handleMessage method, this method posts a message to the Call object's
    //:message handler, passing along the content type and content.

    void doEnableStun(const char* szStunServer,
                      int iKeepAlivePeriodSecs,
                      int stunOptions,
                      OsNotification* pNotification) ;
    //:Enable STUN for NAT/Firewall traversal

    void releaseEvent(const char* callId,
                     OsProtectEventMgr* eventMgr,
                     OsProtectedEvent* dtmfEvent);

           CallManager(const CallManager& rCallManager);
     //:Copy constructor (disabled)

           CallManager& operator=(const CallManager& rhs);
     //:Assignment operator (disabled)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _CallManager_h_

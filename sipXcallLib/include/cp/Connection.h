//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _Connection_h_
#define _Connection_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <os/OsMutex.h>
#include <tapi/sipXtapiEvents.h>
#include <tapi/sipXtapiInternal.h>
#include <net/SipContactDb.h>


// DEFINES
#	define LOCAL_ONLY 0
#	define LOCAL_AND_REMOTE 1

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class CpCallManager;
class CpCall;
class CpMediaInterface;
class OsDatagramSocket;
class SdpCodec;
class SipSession;
class OsMsg;
class TaoObjectMap;
class TaoReference;
class OsServerTask;
class Url;

//:logical Connection within a call
// The Connection encapsulates the call setup protocol and state
// information for the leg of a call to a particular address.
class Connection: public UtlString
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
	enum connectionStates
	{
		CONNECTION_IDLE,
		CONNECTION_QUEUED,
		CONNECTION_OFFERING,
		CONNECTION_ALERTING,
		CONNECTION_ESTABLISHED,
		CONNECTION_FAILED,
		CONNECTION_DISCONNECTED,
        CONNECTION_UNKNOWN,
        CONNECTION_INITIATED,
        CONNECTION_DIALING,
        CONNECTION_NETWORK_REACHED,
        CONNECTION_NETWORK_ALERTING
	};

 	enum termConnectionStates
 	{
 		TERMCONNECTION_NONE,
 		TERMCONNECTION_HOLDING,
 		TERMCONNECTION_HELD,
 		TERMCONNECTION_TALKING
 	};


	enum connectionType
	{
		CONNECTION_REMOTE	= 0,
		CONNECTION_LOCAL	= 1
	};

	//
	// WARNING: The order/values of these constants are used in java.  Please
	//          do not change these without updating the java world.
	//

    enum connectionStateCauses
    {
        // General causes:
        CONNECTION_CAUSE_NORMAL,
        CONNECTION_CAUSE_UNKNOWN,

        // CONNECTION_IDLE causes:
		// CONNECTION_QUEUED causes:

		// CONNECTION_OFFERING causes:
        CONNECTION_CAUSE_REDIRECTED,

		// CONNECTION_ALERTING causes:
        //CONNECTION_CAUSE_CALL_NOT_ANSWERED,
        CONNECTION_CAUSE_CALL_PICKUP,

        //CONNECTION_CAUSE_LOCAL_RINGBACK = CONNECTION_CAUSE_NONE,
        //CONNECTION_CAUSE_REMOTE_RINGBACK = CONNECTION_CAUSE_UNKNOWN,

		// CONNECTION_ESTABLISHED causes:
		CONNECTION_CAUSE_UNHOLD,

		// CONNECTION_FAILED causes:
        CONNECTION_CAUSE_BUSY,
        CONNECTION_CAUSE_NETWORK_CONGESTION,
		CONNECTION_CAUSE_RESOURCES_NOT_AVAILABLE,
        CONNECTION_CAUSE_NETWORK_NOT_OBTAINABLE,
        CONNECTION_CAUSE_DEST_NOT_OBTAINABLE,
        CONNECTION_CAUSE_INCOMPATIBLE_DESTINATION,
		    CONNECTION_CAUSE_SERVICE_UNAVAILABLE,
        CONNECTION_CAUSE_NOT_ALLOWED, // unauthorized by endpoint
        CONNECTION_CAUSE_NETWORK_NOT_ALLOWED, // unauthorized by network/servers
        CONNECTION_CAUSE_CANCELLED,

		// CONNECTION_DISCONNECTED causes:
        CONNECTION_CAUSE_TRANSFER
    };

    enum offeringModes
 	{
 		IMMEDIATE = 0,
 		NEVER = -1
 	};

    enum lineAvailableBehaviors
    {
        RING = 100,
        FAKE_BUSY,
        RING_SILENT,
        AUTO_ANSWER,
        FORWARD_UNCONDITIONAL,
		FORWARD_ON_NO_ANSWER
    };
    //: Incoming call lineAvailableBehaviors
    // The following define the different behaviors for an incoming call when the
    // phone (and resources) is available to take a call.
    //
    //!enumcode: RING - make the phone ring to alert the user
    //!enumcode: FAKE_BUSY - indicate to the caller that the phone is busy, do not alert the callee
    //!enumcode: RING_SILENT - indicate to the caller that the phone is ringing, do not alert the callee
    //!enumcode: AUTO_ANSWER - automatically answer the call, alert the callee and enable the speaker phone
    //!enumcode: FORWARD_UNCONDITIONAL - forward the call even though the phone is available

    enum lineBusyBehaviors
    {
        BUSY = 200,
        FAKE_RING,
        FORCED_ANSWER,
        FORWARD_ON_BUSY,
        QUEUE_SILENT,
        QUEUE_ALERT
    };
    //: Incoming call lineBusyBehaviors
    // The following define the different behaviors for an incoming call when the
    // phone (and resources) is busy.
    //
    //!enumcode: BUSY - indicate to the caller that the phone is busy
    //!enumcode: FAKE_RING - indicate to the caller that the phone is ringing, do not alert the callee
    //!enumcode: FORCED_ANSWER - put the active call on hold, automatically answer the call, alert the callee and enable the speaker phone
    //!enumcode: FORWARD_ON_BUSY - forward the call
    //!enumcode: QUEUE_SILENT - indicate to the caller that the call is being queued, do not alert the callee
    //!enumcode: QUEUE_ALERT - indicate to the caller that the call is being queued, alert the callee

/* ============================ CREATORS ================================== */

   Connection(CpCallManager* callMgr = NULL,
              CpCall* call = NULL,
              CpMediaInterface* mediaInterface = NULL,
              int offeringDelayMilliSeconds = IMMEDIATE,
              int availableBehavior = RING,
              const char* forwardUnconditionalUrl = NULL,
              int busyBehavior = BUSY,
              const char* forwardOnBusyUrl = NULL,
              int fowardOnNoAnswerSeconds = -1);
     //:Default constructor


   virtual
   ~Connection();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual void prepareForSplit() ;
   virtual void prepareForJoin(CpCall* pNewCall, const char* szLocalAddress, CpMediaInterface* pNewMediaInterface) ;

   virtual void forceHangUp(int connectionState = CONNECTION_DISCONNECTED)
   {
	   setState(connectionState, CONNECTION_REMOTE);
      fireSipXEvent(CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE) ;
   }

   virtual UtlBoolean dequeue(UtlBoolean callInFocus) = 0;

   virtual UtlBoolean dial(const char* dialString,
						  const char* callerId,
						  const char* callId,
                          const char* callController = NULL,
                          const char* originalCallConnection = NULL,
                          UtlBoolean requestQueuedCall = FALSE,
                          const void* pDisplay = NULL,
						  const char* originalCallId = NULL,
						  const char* paiAddress = NULL) = 0;
   //! param: requestQueuedCall - indicates that the caller wishes to have the callee queue the call if busy

   virtual UtlBoolean originalCallTransfer(UtlString& transferTargetAddress,
                                           const char* transferControllerAddress,
                                           const char* targetCallId,
                                           bool       remoteHoldBeforeTransfer = true
                                           ) = 0;
   // Initiate transfer on transfer controller connection in
   // the original call.
   // If fromAddress or toAddress are NULL it is assumed to
   // be a blind transfer.

   virtual UtlBoolean targetCallBlindTransfer(const char* transferTargetAddress,
						           const char* transferControllerAddress) = 0;
   // Communicate blind transfer on transfer controller connection in
   // the target call.  This is signaled by the transfer controller in the
   // original call.

   virtual UtlBoolean transfereeStatus(int connectionState, int response) = 0;
   // Method to communicate status to original call on transferee side

   virtual UtlBoolean transferControllerStatus(int connectionState, int response) = 0;
   // Method to communicate status to target call on transfer
   // controller side


   virtual UtlBoolean answer(const void* hWnd = NULL) = 0;

   virtual UtlBoolean hangUp() = 0;

   virtual UtlBoolean hold() = 0;

   virtual UtlBoolean offHold() = 0;

   virtual UtlBoolean renegotiateCodecs() = 0;

   virtual UtlBoolean accept(int forwardOnNoAnswerTimeOut) = 0;

   virtual UtlBoolean reject(int errorCode, const char* szErrorText) = 0;

   virtual UtlBoolean redirect(const char* forwardAddress) = 0;

   virtual UtlBoolean processMessage(OsMsg& eventMessage,
                                    UtlBoolean callInFocus, UtlBoolean onHook) = 0;

   virtual UtlBoolean sendInfo(UtlString contentType, UtlString sContent){ return false; }
   //:Virtual method signature and default implementation for sendInfo - this should be overridden by
   //:SipConnection.

	OsStatus addTaoListener(OsServerTask* pListener,
									char* callId = NULL,
									int ConnectId = 0,
									int mask = 0);

	void setLocalAddress(const char* address);

    void unimplemented(const char* methodName) const;

    void markForDeletion() ;
      //: Is this connection marked for deletion?

    void setMediaInterface(CpMediaInterface* pMediaInterface) ;
      //:Set the media interface for this connection

    CpMediaInterface* getMediaInterfacePtr();
      //:Gets the media interface pointer for this connection.

    UtlBoolean validStateTransition(SIPX_CALLSTATE_EVENT eFrom, SIPX_CALLSTATE_EVENT eTo) ;
    void fireSipXEvent(SIPX_CALLSTATE_EVENT eMajor, SIPX_CALLSTATE_CAUSE eMinor, void *pEventData=NULL) ;

/* ============================ ACCESSORS ================================= */

	void getLocalAddress(UtlString* address);
	int getState(int isLocal = 0) const;
	int getState(int isLocal, int& cause) const;
	int getTerminalState(int isLocal) const;

   int getLocalState() const ;
     // Get the local state for this connection
   int getRemoteState() const ;
     // Get the remote state for this connection

   static void getStateString(int state, UtlString* stateLabel);

   void getCallId(UtlString* callId);
   // Call specific

   void getCallerId(UtlString* callerId);
   // Caller in a call specific

   virtual UtlBoolean getRemoteAddress(UtlString* remoteAddress) const = 0;
   //: get Connection address
   //! returns: TRUE/FALSE if the connection has an address.  The connection may not have an address assigned yet (i.e. if it is not fully setup).

   virtual UtlBoolean getRemoteAddress(UtlString* remoteAddress, UtlBoolean leaveFieldParametersIn) const = 0;
   //: get Connection address
   //! returns: TRUE/FALSE if the connection has an address.  The connection may not have an address assigned yet (i.e. if it is not fully setup).

 	virtual UtlBoolean getSession(SipSession& sessioon) = 0;

	int getResponseCode() { return mResponseCode; };

	void getResponseText(UtlString& responseText);

	int getConnectionId() { return mConnectionId; };

   OsStatus getDeleteAfter(OsTime& time) ;
     //: Get the time after which this connection can be deleted.  This
     //: timespan is relative to boot.

/* ============================ INQUIRY =================================== */

    UtlBoolean isRemoteCallee();

    UtlBoolean remoteRequestedHold();

    virtual UtlBoolean willHandleMessage(OsMsg& eventMessage) const = 0;

	static UtlBoolean isStateTransitionAllowed(int newState, int oldState);

    virtual UtlBoolean isConnection(const char* callId,
                                  const char* toTag,
                                  const char* fromTag,
                                  UtlBoolean  strictCompare) const = 0;

    virtual UtlBoolean isSameRemoteAddress(Url& remoteAddress) const = 0;

    UtlBoolean isMarkedForDeletion() const ;
      //: Determines if this connection has been marked for deletion and
      //: should be purged from the call.

    UtlBoolean isHeld() const ;
      //:Is this connection on remote hold (not sending or receiving audio)

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

	//: Set the connection state and notify observer of state change
	void setState(int newState,
					int isLocal,
					int cause = CONNECTION_CAUSE_NORMAL,
					int termState = -1);

 	//: Set the terminal connection state and notify observer of state change
 	void setTerminalConnectionState(int newState, int isLocal, int cause = CONNECTION_CAUSE_NORMAL);

    virtual void setOfferingTimer(int milliSeconds);
    virtual void setRingingTimer(int seconds);

	void postTaoListenerMessage(int state, int newCause, int isLocal = 0);

	//: Behavior of progression from Offering to next state
	// The connections will:
	// IMMEDIATE, PAUSE, NEVER go to Alerting
	// Default is IMMEDIATE (offeringDelay = 0) proceed to Alerting
	// PAUSE waits upto offeringDealy milliseconds
	// NEVER (offeringDelay = -1)waits forever in Offering state
	// If PAUSE or NEVER are set, the connection may be told to
	// proceed to Failed stated with reject() or redirect() or
	// to Alerting state with accept(), while the connection is in
	// Offering state
	int mOfferingDelay;

    // The use and values of the following four members are as defined in Call
    int mLineAvailableBehavior;
    UtlString mForwardUnconditional;
    int mLineBusyBehavior;
    UtlString mForwardOnBusy;
    int mForwardOnNoAnswerSeconds;

    void setCallId(const char* callId);
    void setCallerId(const char* callId);

    CpCallManager* mpCallManager;
    CpCall* mpCall;
    CpMediaInterface* mpMediaInterface;
    int mConnectionId;

	UtlBoolean mRemoteIsCallee;
	//UtlBoolean mLocalHeld;
	UtlBoolean mRemoteRequestedHold;

	UtlString remoteRtpAddress;
	int remoteRtpPort;
    int remoteRtcpPort;
	int sendCodec;
	int receiveCodec;

	int mLocalConnectionState;
	int mRemoteConnectionState;
    int mConnectionStateCause;
	int mTerminalConnState;
//	int mLocalTerminalConnState;
//	int mRemoteTerminalConnState;
	int mFarEndHoldState;
	int mResponseCode;		// response code obtained at processResponse, passed through events to upper layer
	UtlString mResponseText;	// response text obtained at processResponse

	TaoObjectMap*		mpListeners;
	TaoReference*		mpListenerCnt;

	UtlString mLocalAddress;
    UtlString mOriginalCallConnectionAddress;
    UtlString mTargetCallConnectionAddress;
    UtlString mTargetCallId;


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

	//Private as it must be locked and accessed outside this thread
	UtlString connectionCallId;
    UtlString connectionCallerId;
	OsMutex  callIdMutex;
    OsTime   mDeleteAfter ;    // Instructs the call to delete this connection
                               // after this time period (time since boot)
    SIPX_CALLSTATE_EVENT m_eLastMajor ;
    SIPX_CALLSTATE_CAUSE m_eLastMinor ;
    SIPX_CALLSTATE_EVENT m_eLastAudioMajor;
    SIPX_CALLSTATE_CAUSE m_eLastAudioMinor;


	Connection(const Connection& rConnection);
     //:Copy constructor (disabled)
	Connection& operator=(const Connection& rhs);
     //:Assignment operator (disabled)

	int terminalConnectionState(int connState);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _Connection_h_

//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _CpCall_h_
#define _CpCall_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsServerTask.h>
#include <os/OsRWMutex.h>
#include "os/OsUtil.h"
#include "os/OsLockingList.h"
#include <net/SdpCodec.h>
#include <ptapi/PtEvent.h>
#include <ptapi/PtConnection.h>
#include <ptapi/PtTerminalConnection.h>
#include <cp/CallManager.h>
#include "tao/TaoObjectMap.h"

// DEFINES
#define MINIMUM_DTMF_LENGTH 50
#define MINIMUM_DTMF_SILENCE 50

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
struct DtmfEvent {
    int     event;
    int interdigitSecs;
    int timeoutSecs;
    int ignoreKeyUp;
    UtlBoolean enabled;
};

// TYPEDEFS
// FORWARD DECLARATIONS
class CpCallManager;
class CpMediaInterface;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class CpCall : public OsServerTask
{
    /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    enum eventType
    {
        CONNECTION_STATE = 0,
        TERMINAL_CONNECTION_STATE,
        CALL_STATE
    };

    enum metaEventState
    {
        METAEVENT_START = 0,
        METAEVENT_INPROGRESS,
        METAEVENT_END
    };

    enum callTypes
    {
        CP_NORMAL_CALL,

        // There are three types of parties involved with a transfer:
        // Transfer Controller -
        // Transferee -
        // Transfer Target -
        // There are two calls involved with a transfer:
        // Original call - between the transfer controller and transferee(s)
        // Target Call - between all parties, the transfer controller and
        //     transfer target connect on this call first.  If this is
        //     consultative the transfer controller does not drop out
        //     immediately
        CP_TRANSFER_CONTROLLER_ORIGINAL_CALL,
        CP_TRANSFER_CONTROLLER_TARGET_CALL,
        CP_TRANSFEREE_ORIGINAL_CALL,
        CP_TRANSFEREE_TARGET_CALL,
        //CP_TRANSFER_TARGET_ORIGINAL_CALL,  // do not know if this is needed
        CP_TRANSFER_TARGET_TARGET_CALL
    };

    // The following enumeration defines the degree of willingness
    // that a call has for handling a message.  It is not always
    // a binary decision, so this allows a weighting as to the amount
    // of willingness.
    enum handleWillingness
    {
        // Does not match by any means
        CP_WILL_NOT_HANDLE = 0,

        // Will handle if a better match is not found
        // (i.e. check all the remaining calls first).
        CP_MAY_HANDLE,

        // Is a definite match no need to search any further
        CP_DEFINITELY_WILL_HANDLE
    };
    /* ============================ CREATORS ================================== */

    CpCall(CpCallManager* manager = NULL,
        CpMediaInterface* callMediaInterface = NULL,
        int callIndex = -1,
        const char* callId = NULL,
        int holdType = CallManager::NEAR_END_HOLD);
    //:Default constructor

    virtual
        ~CpCall();
    //:Destructor

    /* ============================ MANIPULATORS ============================== */

    void setDropState(UtlBoolean state);

    void postMetaEvent(int state, int remoteIsCallee = -1);  // remoteIsCallee = -1 means not set

    void postTaoListenerMessage(int responseCode,
        UtlString responseText,
        TaoEventId eventId,
        int type,
        int cause = PtEvent::CAUSE_NORMAL,
        int remoteIsCallee = 1,
        UtlString remoteAddress = "",
        int isRemote = 0,
        UtlString targetCallId = OsUtil::NULL_OS_STRING);

    virtual OsStatus addTaoListener(OsServerTask* pListener,
        char* callId = NULL,
        int ConnectId = 0,
        int mask = 0,
        intptr_t pEv = 0);
    //:Register as a listener for call and connection events.

    void setCallState(int responseCode, UtlString responseText, int state, int cause = PtEvent::CAUSE_NORMAL);

    virtual void inFocus(int talking = 1);
    virtual void outOfFocus();

    //virtual void hold();
    //virtual void offHold();
    virtual void localHold();
    virtual void hangUp(UtlString callId, int metaEventId);
    //virtual void blindTransfer() = 0;
    //virtual void conferenceAddParty() = 0;

    virtual void getLocalAddress(char* address, int len);

    virtual void getLocalTerminalId(char* terminal, int len);

    virtual void getCallId(UtlString& callId);
    //: Gets the main call Id for this call
    // Note: a call may have many callIds (i.e. one for each connection)
    virtual void setCallId(const char* callId);
    //: Sets the main call Id for this call

    void setLocalConnectionState(int newState);
    //: Sets the local connection state for this call

    int getLocalConnectionState() { return mLocalConnectionState; };
    //: Sets the local connection state for this call

    void addToneListenerToFlowGraph(intptr_t pListener, Connection* connection);
    void removeToneListenerFromFlowGraph(intptr_t pListener, Connection* connection);

    OsStatus ezRecord(int ms, int silenceLength, const char* fileName, double& duration, int& dtmfterm);
    virtual OsStatus stopRecord();
    /* ============================ ACCESSORS ================================= */
    static int getCallTrackingListCount();
    //returns the number of call tasks currently outstanding.

    int getCallIndex();

    int getCallState();

    int getDropState();    ///> For CallManager use ONLY. See mCallMgrPostedDrop/
    void setDropState();   ///> For CallManager use ONLY. See mCallMgrPostedDrop/

    virtual void printCall(int showHistory = 1);

    // This should go away
    void enableDtmf();

    static void getStateString(int state, UtlString* stateLabel);

    // Meta Event Utilities
    // For the meta events, the first callId (index=0) is the new
    // call, the subsequent callIds (index = 1 through numCalls)
    // are the old calls
    virtual void startMetaEvent(int metaEventId, int metaEventType,
        int numCalls, const char* metaEventCallIds[], int remoteIsCallee = -1); // remoteIsCallee = -1 means not set

    virtual void setMetaEvent(int metaEventId, int metaEventType,
        int numCalls, const char* metaEventCallIds[]);

    void getMetaEvent(int& metaEventId, int& metaEventType,
        int& numCalls, const UtlString* metaEventCallIds[]) const;

    virtual void stopMetaEvent(int remoteIsCallee = -1); // remoteIsCallee = -1 means not set

    void setCallType(int callType);
    int getCallType() const;

    void setTargetCallId(const char* targetCallId);
    void getTargetCallId(UtlString& targetCallId) const;
    void setIdOfOrigCall(const char* idOfOriginalCall);
    void getIdOfOrigCall(UtlString& idOfOriginalCall) const;

    int getLocalConnectionState(int state);

    unsigned long getElapsedTime(void);     // xecs-1698 hack


    /* ============================ INQUIRY =================================== */

    virtual UtlBoolean hasCallId(const char* callId) = 0;

    virtual enum handleWillingness willHandleMessage(const OsMsg& eventMessage) = 0;

    virtual UtlBoolean isQueued();

    virtual UtlBoolean isCallIdSet();

    virtual UtlBoolean isLocalHeld();

    virtual UtlBoolean canDisconnectConnection(Connection* pConnection) = 0;

    /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    virtual UtlBoolean handleMessage(OsMsg& eventMessage);
    virtual UtlBoolean handleCallMessage(OsMsg& eventMessage) = 0;
    virtual void onHook() = 0;

    virtual UtlBoolean getConnectionState(const char* remoteAddress, int& state) = 0;

    virtual UtlBoolean getTermConnectionState(const char* address,
        const char* terminal,
        int& state) = 0;

    void addHistoryEvent(const char* messageLogString);
    void addHistoryEvent(const int msgSubType,
        const CpMultiStringMessage* multiStringMessage);

    OsStatus addListener(OsServerTask* pListener,
        TaoListenerDb** pListeners,
        int& listenerCnt,
        char* callId = NULL,
        int ConnectId = 0,
        int mask = 0,
        intptr_t pEv = 0);

    CpCallManager* mpManager;
    UtlString mCallId;
    //: the original call-id of THIS call
    UtlBoolean mCallInFocus;
    UtlBoolean mRemoteDtmf;
    UtlBoolean mDtmfEnabled;
    OsRWMutex mCallIdMutex;
    CpMediaInterface*   mpMediaInterface;
    int mCallIndex;
    int mCallState;
    int mHoldType;
    int mLocalConnectionState;
    int mLocalTermConnectionState;
    UtlBoolean mLocalHeld;

    UtlBoolean mDropping;
    int mMetaEventId;
    int mMetaEventType;
    int mNumMetaEventCalls;
    UtlString* mpMetaEventCallIds;

    TaoListenerDb**                 mpListeners;
    int                             mListenerCnt;
    int                             mMaxNumListeners;

    TaoListenerDb*                  mpToneListeners[MAX_NUM_TONE_LISTENERS];
    int                                             mToneListenerCnt;

    int mMessageEventCount;
    UtlString mCallHistory[CP_CALL_HISTORY_LENGTH];

    OsRWMutex                           mDtmfQMutex;
    int                                             mDtmfQLen;
    DtmfEvent                               mDtmfEvents[MAX_NUM_TONE_LISTENERS];

    /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    static OsLockingList *spCallTrackingList;
    //: maintains a list of the Call-nnn names


    static OsStatus addToCallTrackingList(UtlString &rCallTaskName);
    //: function used to add call names to the tracking list

    static OsStatus removeFromCallTrackingList(UtlString &rCallTaskName);
    //: function used to remove call names from the tracking list

    int mCallType;
    //: initially set to CP_NORMAL_CALL
    //: can be set to
    //:     - CP_TRANSFER_CONTROLLER_ORIGINAL_CALL for blind(16) or consult(17) transfer
    //:     - CP_TRANSFER_CONTROLLER_TARGET_CALL for transfer connection(18)
    //:     - CP_TRANSFER_TARGET_TARGET_CALL for incoming invite with replaces,replaced-by, referred-by
    //:     - CP_TRANSFEREE_ORIGINAL_CALL for incoming REFER or BYE with 'also' header

    UtlString mIdOfOrigCall;
    //: transfer use only, contains call-id of the original call
    //: this is NOT the original call-id of THIS call

    UtlString mTargetCallId;

    CpCall& operator=(const CpCall& rhs);
    //:Assignment operator (disabled)
    CpCall(const CpCall& rCpCall);
    //:Copy constructor (disabled)

    int tcStateFromEventId(int eventId);

    // utility function used to remove ev from mDtmfEvents.
    void removeFromDtmfEventList(int ev);

    // utility function used to check if ev exists in mDtmfEvents.
    int dtmfEventExists(int ev);

    OsTime  mCallTimeStart;   // xecs-1698 hack

    bool mCallMgrPostedDrop;
    ///< the relationship between this object and the call manager
    /// This flag is to eliminate a race for messages which are posted
    /// to CallManager after CP_DROP but before CP_CALL_EXITED.
    /// CallManager should not send any such messages to the CpCall object.
    /// Only constructor should set to FALSE.
    /// Only CallManager should check or write to it outside the constructor.
    /// CM can only set this to TRUE.  CM should set before posting CP_DROP.
    /// If TRUE, CM will not post any message to the Call object.

};

/* ============================ INLINE METHODS ============================ */

#endif  // _CpCall_h_

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
#include <assert.h>

// APPLICATION INCLUDES
#include <cp/CpGhostConnection.h>
#include <net/Url.h>
#include <net/SipContactDb.h>
#include <cp/CpMultiStringMessage.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

//#define TEST_PRINT

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpGhostConnection::CpGhostConnection(CpCallManager* callMgr,
                                     CpCall* call,
                                     const char* callId) :
Connection(callMgr, call)
{
   if(callId) setCallId(callId);

   mRemoteIsCallee = TRUE;

}

// Copy constructor
CpGhostConnection::CpGhostConnection(const CpGhostConnection& rCpGhostConnection)
{
}

// Destructor
CpGhostConnection::~CpGhostConnection()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
CpGhostConnection&
CpGhostConnection::operator=(const CpGhostConnection& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

UtlBoolean CpGhostConnection::dequeue(UtlBoolean callInFocus)
{
    unimplemented("CpGhostConnection::dequeue");
    return(FALSE);
}
UtlBoolean CpGhostConnection::dial(const char* dialString,
                                   const char* callerId,
                                   const char* callId,
                                   const char* callController,
                                   const char* originalCallConnection,
                                   UtlBoolean requestQueuedCall,
                                   const void* pDisplay,
                                   const char* originalCallId,
                                   const char* paiAddress)
{
    unimplemented("CpGhostConnection::dial");
    return(FALSE);
}

UtlBoolean CpGhostConnection::originalCallTransfer(UtlString& transferTargetAddress,
                                                   const char* transferControllerAddress,
                                                   const char* targetCallId,
                                                   bool        remoteHoldBeforeTransfer)
{
    unimplemented("CpGhostConnection::originalCallBlindTransfer");
    return(FALSE);
}

UtlBoolean CpGhostConnection::targetCallBlindTransfer(const char* transferTargetAddress,
                                                           const char* transferControllerAddress)
{
#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpGhostConnection::targetCallBlindTransfer ");
#endif
    mRemoteAddress = transferTargetAddress;
    setState(CONNECTION_DIALING, CONNECTION_REMOTE, CONNECTION_CAUSE_TRANSFER);
    setState(CONNECTION_ESTABLISHED, CONNECTION_LOCAL, CONNECTION_CAUSE_TRANSFER);
    /** SIPXTAPI: TBD **/
    return(TRUE);
}

void CpGhostConnection::disconnectForSipXTapi()
{
    // sipXtapi listeners do not know about this ghost call so if there is a
    // tapi listener, we must tell CallManager to clean up here.
    // BUT this ghost connection does not know if there are any sipxtapi listeners,
    // fireSipXEvent has to tell us.
    // This code may not cause problems if BOTH tapi and tao listeners are allowed.
    bool bNotifyCallManager = FALSE;

    fireSipXEvent(CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_GHOST, (void*)(&bNotifyCallManager)) ;
#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpGhostConnection::tearDownForSipXTapi "
                  "returns %d", bNotifyCallManager);
#endif
    if (bNotifyCallManager == TRUE)
    {
        // there is a tapi listener, but it doesn't know this ghost-connected call
        UtlString ghostCallId;
        getCallId(&ghostCallId);
        CpMultiStringMessage callMessage(CpCallManager::CP_DROP, ghostCallId.data());
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CpGhostConnection::tearDownForSipXTapi "
                      "post drop for call %s",
                      ghostCallId.data());
        mpCallManager->postMessage(callMessage);
    }
}

UtlBoolean CpGhostConnection::transfereeStatus(int connectionState, int response)
{
    unimplemented("CpGhostConnection::transfereeStatus");
    return(FALSE);
}
UtlBoolean CpGhostConnection::transferControllerStatus(int connectionState, int cause)
{
#ifdef TEST_PRINT
    UtlString connState;
    getStateString(connectionState, &connState);
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpGhostConnection::transferControllerStatus "
                  "state: %s cause: %d\n",
                  connState.data(), cause);
    connState.remove(0);
#endif
    setState(connectionState, CONNECTION_REMOTE, cause);
    /** SIPXTAPI: TBD **/

    // If the transfer suceeded, we need to clean up the pseudo(ghost) call.
    if(connectionState == CONNECTION_ESTABLISHED)
    {
        // taoListeners need to see DISCONNECTED state to clean out all related objects
        setState(CONNECTION_DISCONNECTED, CONNECTION_REMOTE, CONNECTION_CAUSE_TRANSFER);

        /** SIPXTAPI related **/
        disconnectForSipXTapi();

        // Go immediately to UNKNOWN as we will NOT receive further state change notifications
        setState(CONNECTION_UNKNOWN, CONNECTION_REMOTE, CONNECTION_CAUSE_TRANSFER);

    }

    return(TRUE);
}

UtlBoolean CpGhostConnection::answer(const void* hWnd)
{
    unimplemented("CpGhostConnection::answer");
    return(FALSE);
}

UtlBoolean CpGhostConnection::hangUp()
{
    setState(CONNECTION_DISCONNECTED, CONNECTION_REMOTE, CONNECTION_CAUSE_TRANSFER);
    /** SIPXTAPI: TBD **/
    setState(CONNECTION_DISCONNECTED, CONNECTION_LOCAL, CONNECTION_CAUSE_TRANSFER);
    /** SIPXTAPI: TBD **/

    disconnectForSipXTapi();

    return(FALSE);
}
UtlBoolean CpGhostConnection::hold()
{
    unimplemented("CpGhostConnection::hold");
    return(FALSE);
}
UtlBoolean CpGhostConnection::reject(int errorCode, const char* szErrorText)
{
    unimplemented("CpGhostConnection::reject");
    return(FALSE);
}
UtlBoolean CpGhostConnection::redirect(const char* forwardAddress)
{
    unimplemented("CpGhostConnection::redirect");
    return(FALSE);
}
UtlBoolean CpGhostConnection::offHold()
{
    unimplemented("CpGhostConnection::offHold");
    return(FALSE);
}

UtlBoolean CpGhostConnection::renegotiateCodecs()
{
    unimplemented("CpGhostConnection::renegotiateCodecs");
    return(FALSE);
}

UtlBoolean CpGhostConnection::accept(int forwardOnNoAnswerSeconds)
{
    unimplemented("CpGhostConnection::accept");
    return(FALSE);
}

UtlBoolean CpGhostConnection::processMessage(OsMsg& eventMessage,
                                    UtlBoolean callInFocus, UtlBoolean onHook)
{
    return(FALSE);
}

/* ============================ ACCESSORS ================================= */

UtlBoolean CpGhostConnection::getRemoteAddress(UtlString* remoteAddress) const
{
    *remoteAddress = mRemoteAddress;
    return(!mRemoteAddress.isNull());
}

UtlBoolean CpGhostConnection::getRemoteAddress(UtlString* remoteAddress, UtlBoolean leaveFieldParametersIn) const
{
    unimplemented("CpGhostConnection::getRemoteAddress");
    return(FALSE);
}

UtlBoolean CpGhostConnection::getSession(SipSession& session)
{
    unimplemented("CpGhostConnection::getSession");
    return(OS_NOT_FOUND);
}


// Enumerate possible contact addresses
void CpGhostConnection::getLocalContactAddresses( ContactAddress contacts[],
                                                  size_t nMaxContacts,
                                                  size_t& nActualContacts)
{
    unimplemented("CpGhostConnection::getLocalContactAddresses");
}


/* ============================ INQUIRY =================================== */

UtlBoolean CpGhostConnection::willHandleMessage(OsMsg& eventMessage) const
{
    unimplemented("CpGhostConnection::willHandleMessage");
    return(FALSE);
}

UtlBoolean CpGhostConnection::isConnection(const char* callId,
                                  const char* toTag,
                                  const char* fromTag,
                                  UtlBoolean strictCompare) const
{
    unimplemented("CpGhostConnection::isConnection");
     return(FALSE);
}

UtlBoolean CpGhostConnection::isSameRemoteAddress(Url& remoteAddress) const
{
    UtlString address;
    remoteAddress.toString(address);
    return(address.compareTo(mRemoteAddress) == 0);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

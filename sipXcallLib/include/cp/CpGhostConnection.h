//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _CpGhostConnection_h_
#define _CpGhostConnection_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <cp/Connection.h>
#include <net/SipContactDb.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


//:logical Connection within a call
// The Connection encapsulates the call setup protocol and state
// information for the leg of a call to a particular address.
// WARNING: CpGhostConnection's are known to taoListener's but they
// are not known to sipXtapiListeners.  This forces different behavior
// for the two cases.
// In general, a single CpGhostConnection should not be associated
// with both types of listeners.
//
class CpGhostConnection: public Connection
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   CpGhostConnection(CpCallManager* callMgr = NULL,
              CpCall* call = NULL, const char* callId = NULL);
     //:Default constructor


   virtual
   ~CpGhostConnection();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual UtlBoolean dequeue(UtlBoolean callInFocus);

   virtual UtlBoolean dial(const char* dialString,
                           const char* callerId,
                           const char* callId,
                           const char* callController = NULL,
                           const char* originalCallConnection = NULL,
                           UtlBoolean requestQueuedCall = FALSE,
                           const void* pDisplay = NULL,
                           const char* originalCallId = NULL,
                           const char* paiAddress = NULL);
   //! param: requestQueuedCall - indicates that the caller wishes to have the callee queue the call if busy

   virtual UtlBoolean originalCallTransfer(UtlString& transferTargetAddress,
                                           const char* transferControllerAddress,
                                           const char* targetCallId,
                                           bool       remoteHoldBeforeTransfer = true
                                           );
   // Initiate blind transfer on transfer controller connection in
   // the original call.

   virtual UtlBoolean targetCallBlindTransfer(const char* transferTargetAddress,
                                                           const char* transferControllerAddress);
   // Communicate blind transfer on transfer controller connection in
   // the target call.  This is signaled by the transfer controller in the
   // original call.

   virtual UtlBoolean transfereeStatus(int connectionState, int response);
   // Method to communicate status to original call on transferee side

   virtual UtlBoolean transferControllerStatus(int connectionState, int cause);
   // Method to communicate status to target call on transfer
   // controller side

   void disconnectForSipXTapi();

   virtual UtlBoolean answer(const void* hWnd = NULL);

   virtual UtlBoolean hangUp();

   virtual UtlBoolean hold();

   virtual UtlBoolean reject(int errorCode, const char* errorText);

   virtual UtlBoolean redirect(const char* forwardAddress);

   virtual UtlBoolean offHold();

   virtual UtlBoolean renegotiateCodecs();

   virtual UtlBoolean accept(int forwardOnNoAnswerSeconds);

   virtual UtlBoolean processMessage(OsMsg& eventMessage,
                                    UtlBoolean callInFocus, UtlBoolean onHook);

/* ============================ ACCESSORS ================================= */

   virtual UtlBoolean getRemoteAddress(UtlString* remoteAddress) const;
   //: get Connection address
   //! returns: TRUE/FALSE if the connection has an address.  The connection may not have an address assigned yet (i.e. if it is not fully setup).

   virtual UtlBoolean getRemoteAddress(UtlString* remoteAddress, UtlBoolean leaveFieldParametersIn) const;
   //: get Connection address
   //! returns: TRUE/FALSE if the connection has an address.  The connection may not have an address assigned yet (i.e. if it is not fully setup).

           virtual UtlBoolean getSession(SipSession& session);

    /**
     * Enumerate possible contact addresses
     */
    virtual void getLocalContactAddresses( ContactAddress contacts[],
                                           size_t nMaxContacts,
                                           size_t& nActualContacts) ;


/* ============================ INQUIRY =================================== */

   virtual UtlBoolean willHandleMessage(OsMsg& eventMessage) const;

   virtual UtlBoolean isConnection(const char* callId,
                                  const char* toTag,
                                  const char* fromTag,
                                  UtlBoolean strictCompare) const;

   virtual UtlBoolean isSameRemoteAddress(Url& remoteAddress) const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:



/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

        CpGhostConnection(const CpGhostConnection& rCpGhostConnection);
     //:Copy constructor (disabled)
        CpGhostConnection& operator=(const CpGhostConnection& rhs);
     //:Assignment operator (disabled)

    UtlString mRemoteAddress;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _CpGhostConnection_h_

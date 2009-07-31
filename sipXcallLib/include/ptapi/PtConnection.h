//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtConnection_h_
#define _PtConnection_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <ptapi/PtDefs.h>
#include "ptapi/PtSessionDesc.h"
#include <os/OsBSem.h>
#include "os/OsProtectEventMgr.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PtCall;
class PtTerminalConnection;
class PtAddress;
class PtProviderListener;
class PtTerminal;
class TaoClientTask;
class TaoServerTask;
class TaoReference;
class TaoObjectMap;



//:A PtConnection represents a link, or association, between a PtCall
//:object and a PtAddress object.
// The purpose of a PtConnection object is to describe the relationship
// between a PtCall object and a PtAddress object. A PtConnection object
// exists if the PtAddress is a part of the telephone call. Each PtConnection
// has a state that describes the particular stage of the relationship
// between the call and address. These states and their meanings are described
// below. Applications use the PtConnection.getCall() and
// PtConnection.getAddress() methods respectively to obtain the call and
// address associated with this connection.
// <p>
// From one perspective, an application may view a call only in terms of the
// PtAddress/PtConnection objects that are part of the call. This is termed
// a logical view of the call because it ignores the details provided by the
// PtTerminal and PtTerminalConnection objects that are also associated with
// a call. In many instances, simple applications (such as an outcall program)
// may only need to concern themselves with the logical view. In this logical
// view, a telephone call is viewed as two or more endpoint addresses in
// communication. The PtConnection object describes the state of each of these
// endpoint addresses with respect to the call.
//
// <H3>Calls and Addresses</H3>
// PtConnection objects are immutable in terms of their PtCall and PtAddress
// references. In other words, the association of a PtCall and PtAddress
// object to the PtConnection does not change throughout the lifetime of the
// PtConnection object instance. The same PtConnection object cannot be used
// in another telephone call. The existence of a PtConnection implies that
// its address is associated with its call in the manner described by the
// PtConnection's state.<br>
// <br>
// Although a PtConnection's PtAddress and PtCall references remain valid
// throughout the lifetime of the PtConnection object, the same is not true
// for the PtCall and PtAddress object's references to this PtConnection.
// Particularly, when a PtConnection moves into the
// PtConnection::DISCONNECTED state, it is no longer listed by the
// PtCall.getConnections() and PtAddress.getConnections() methods.
//
// <H3>PtTerminalConnections</H3>
// PtConnection objects are containers for zero or more PtTerminalConnection
// objects. PtConnection objects represent the relationship between the PtCall
// and the PtAddress, whereas PtTerminalConnection objects represent the
// relationship between the PtConnection and the PtTerminal. The relationship
// between the PtCall and the PtAddress may be viewed as a logical view of the
// call. The relationship between a PtConnection and a PtTerminal represents
// the physical view of the call, i.e., at which PtTerminal the telephone call
// terminates. For more information on PtTerminalConnections see the
// specification for that class.
//
// <H3>Connection States</H3>
// Descriptions in real world terms of each PtConnection state follow.
// These real world descriptions have no bearing on the specifications of
// methods, but serve to provide a more intuitive understanding of what
// is going on.<p>
// <dl>
// <dt><b>PtConnection::IDLE</b><br>
// <dd>The initial state for all new connections. Connections
// which are in the PtConnection::IDLE state are not actively part of a
// telephone call, yet their references to the PtCall and PtAddress objects
// are valid. Connections typically do not stay in the PtConnection::IDLE
// state for long; they transition quickly to other states.<br>
// <br>
// <dt><b>PtConnection::OFFERED</b><br>
// <dd>Indicates that an incoming call is being offered to the
// PtAddress associated with the PtConnection.  Typically, applications must
// either accept or reject this offered call before the called party is
// alerted to the incoming telephone call.  Alternatively, the application
// may use PtAddress.setOfferedTimeout() to specify how long the connection
// should remain in the PtConnection::OFFERED state before automatically
// transitioning to the PtConnection::ALERTING state.<br>
// <br>
// <dt><b>PtConnection::QUEUED</b><br>
// <dd>Indicates that a PtConnection is queued at the particular
// PtAddress associated with the PtConnection.  For example, some telephony
// platforms permit the "queuing" of an incoming telephone call to a
// PtAddress when the PtAddress is busy.<br>
// <br>
// <dt><b>PtConnection::NETWORK_REACHED</b><br>
// <dd>Indicates that an outgoing telephone call has reached the
// network.  Applications may not receive further events about this leg of
// the telephone call, depending upon the ability of the telephone network
// to provide additional progress information. Applications must decide
// whether to treat this as a connected telephone call.<br>
// <br>
// <dt><b>PtConnection::NETWORK_ALERTING</b><br>
// <dd>Indicates that an outgoing telephone call is alerting at the
// destination end, which was previously only known to have reached the
// network. Typically, PtConnections transition into this state from the
// PtConnection::NETWORK_REACHED state. This state results from additional
// progress information being sent from the telephone network.<br>
// <br>
// <dt><b>PtConnection::ALERTING</b><br>
// <dd>Implies that the address is being notified of an incoming
// call.<br>
// <br>
// <dt><b>PtConnection::INITIATED</B><br>
// <dd>Indicates the originating end of a telephone call has begun
// the process of placing a telephone call, but the dialing of the destination
// telephone address has not yet begun.  Typically, a telephone associated
// with the PtAddress has gone "off hook."<br>
// <br>
// <dt><b>PtConnection::DIALING</b><br>
// <dd>Indicates the originating end of a telephone call has begun
// the process of dialing a destination telephone address, but has not yet
// completed dialing.<br>
// <br>
// <dt><b>PtConnection::ESTABLISHED</B><br>
// <dd>Implies that a connection and its address are actively part of
// a telephone call. For example, two people talking to one another are
// represented by two connections in the PtConnection::ESTABLISHED state.<br>
// <br>
// <dt><b>PtConnection::DISCONNECTED</B><br>
// <dd>Implies it is no longer part of the telephone call, although
// its references to PtCall and PtAddress still remain valid. A connection in
// this state is interpreted as once previously belonging to this telephone
// call.<br>
// <br>
// <dt><b>PtConnection::FAILED</B><br>
// <dd>Indicates that a connection to that end of the call has failed
// for some reason. One reason why a connection would be in the
// PtConnection::FAILED state is because the party was busy.<br>
// <br>
// <dt><b>PtConnection::UNKNOWN</B><br>
// <dd>Implies that the implementation is unable to determine the
// current state of the connection. Typically, methods are invalid on
// connections which are in this state. Connections may move in and out of
// the PtConnection::UNKNOWN state at any time.
// </dl>
// <H3>Listeners and Events</H3>
// All events pertaining to the PtConnection object are reported via the
// PtConnectionListener objects for the PtCall object associated with this
// connection.  Events are reported to a PtConnectionListener when a new
// connection is created and whenever a connection changes state.  Listeners
// are added to PtCall objects via the PtCall.addCallListener() method and
// more indirectly via the PtAddress.addCallListener() and
// PtTerminal.addCallListener() methods.
class PtConnection
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /*
    * WARNING: These values are used by java.  Please also change these values
    *          in PtConnection.java
    */
   enum ConnectionState
   {
      IDLE                                              = 0x50,
      OFFERED                                   = 0x51,
      QUEUED                                    = 0x52,
      ALERTING                                  = 0x53,
      INITIATED                             = 0x54,
      DIALING                                   = 0x55,
      NETWORK_REACHED               = 0x56,
      NETWORK_ALERTING              = 0x57,
      ESTABLISHED                               = 0x58,
      DISCONNECTED                          = 0x59,
      FAILED                                    = 0x5A,
      UNKNOWN                                   = 0x5B
   };

/* ============================ CREATORS ================================== */
        PtConnection();
         //:Default constructor

        PtConnection(TaoClientTask *pClient, const char *address, const char *callId);

        PtConnection(const PtConnection& rPtConnection);
         //:Copy constructor

        PtConnection& operator=(const PtConnection& rhs);
         //:Assignment operator

        PtConnection(const char* address, const char* callId);

        virtual
        ~PtConnection();
         //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual PtStatus accept(void);
     //:Accepts a telephone call incoming to a PtAddress.
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_INVALID_STATE - Connection was not in either the OFFERED or ALERTING states.
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus disconnect(void);
     //:Drops this connection from an active telephone call.
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_INVALID_STATE - Connection was not in an appropriate state for this call.
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus park(char* destinationURL, PtConnection& rNewConnection);
     //:Parks a PtConnection at the indicated destination telephone address.
     // Parking a PtConnection at a destination address drops this
     // PtConnection from the PtCall and sets <i>rpNewConnection</i> to point
     // to a new PtConnection at the specified destination address.  The new
     // PtConnection will be in the PtConnection::QUEUED state.
     //!param: (in) destinationURL - The destination URL where the call should be parked.
     //!param: (out) rpNewConnection - Set to point to the PtConnection representing the destination where the call was parked.
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_INVALID_STATE - Connection was not in an appropriate state for this call.
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus redirect(char* destinationURL, PtConnection& rNewConnection);
     //:Redirects an incoming telephone call to another telephone address.
     // This method is very similar to the transfer feature, however,
     // applications may invoke this method before first answering the
     // telephone call. This method redirects the telephone call to another
     // destination address provided as the argument to this method.  Redirecting
     // a PtConnection to a destination address drops this PtConnection from the
     // PtCall and sets <i>rpNewConnection</i> to point to a new PtConnection at
     // the specified destination address.
     //!param: (in) destinationURL - The destination URL where the call should be redirected.
     //!param: (out) rpNewConnection - Set to point to the PtConnection representing the redirect destination.
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_INVALID_STATE - Connection was not in either the OFFERED or ALERTING states.
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus reject(void);
     //:Rejects a telephone call incoming to a PtAddress.
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_INVALID_STATE - Connection was not in either the OFFERED or ALERTING states.
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

/* ============================ ACCESSORS ================================= */

   virtual PtStatus getAddress(PtAddress& rAddress);
     //:Sets <i>rpAddress</i> to point to the PtAddress corresponding to this
     //:connection.
     //!param: (out) rpAddress - Pointer to the address object corresponding to this connection
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getCall(PtCall& rCall);
     //:Sets <i>rpCall</i> to point to the PtCall corresponding to this
     //:connection.
     //!param: (out) rpCall - Pointer to the call object corresponding to this connection
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getSessionInfo(PtSessionDesc& rSession);
     //:Sets <i>rSession</i> to the current session of the connection.
     //!param: (out) rSession - Set to the current session of the connection
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getState(int& rState);
     //:Sets <i>rState</i> to the current state of the connection, either
     //:ALERTING, DIALING, DISCONNECTED, ESTABLISHED, FAILED, IDLE,
     //:INITIATED, NETWORK_REACHED, NETWORK_ALERTING, OFFERED, QUEUED, or
     //:UNKNOWN.
     //!param: (out) rState - Set to the current state of the connection
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getTerminalConnections(PtTerminalConnection termConnections[],
                                   int size, int& nItems);
     //:Returns an array of pointers to PtTerminalConnection objects currently
     //:associated with this connection.
     // The caller provides an array that can hold up to <i>size</i>
     // PtTerminalConnection pointers.  This method will fill in the
     // <i>termConnections</i> array with up to <i>size</i> pointers.  The
     // actual number of pointers filled in is passed back via the
     // <i>nItems</i> argument.
     //!param: (out) termConnections - The array of PtTerminalConnection pointers
     //!param: (in) size - The number of elements in the <i>termConnections</i> array
     //!param: (out) nItems - The number of items assigned
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_MORE_DATA - There are more than <i>size</i> terminal connections
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus numTerminalConnections(int& count);
     //:Returns the number of terminal connections associated with this
     //:connection.
     //!param: (out) count - The number of terminal connections associated with this connection
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

  virtual PtStatus getToField(char* pName, int len);
        //:Returns the SIP To field, upto len bytes
    //!param: (in) len - length of the string to store the To field

  virtual PtStatus getFromField(char* pName, int len);
        //:Returns the SIP From field, upto len bytes
    //!param: (in) len - length of the string to store the From field

/* ============================ INQUIRY =================================== */
friend class PtConnectionEvent;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

        UtlString mAddress;
        UtlString mCallId;
        int        mState;

    static OsBSem           semInit ;
      //: Binary Semaphore used to guard initialiation and tear down
        static TaoReference             *mpTransactionCnt;
        static unsigned int              mRef;

        PtConnection            *mpConnection;
        TaoClientTask           *mpClient;

        OsTime                          mTimeOut;
        void initialize();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        OsProtectEventMgr *mpEventMgr;


};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtConnection_h_

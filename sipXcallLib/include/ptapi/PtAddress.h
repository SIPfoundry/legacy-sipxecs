//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PtAddress_h_
#define _PtAddress_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <ptapi/PtDefs.h>
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
class PtAddressForwarding;
class PtAddressListener;
class PtCallListener;
class PtConnection;
class PtProvider;
class PtTerminal;
class TaoObjectMap;
class TaoReference;
class PtAddress;
class PtCall;
class PtProviderListener;
class TaoClientTask;
class TaoServerTask;

//:A PtAddress object represents what we commonly think of as a phone number.
// However, for calls to IP telephony endpoints, the "telephone number" is
// a URL or IP address identifying the endpoint.
// <p>
// Address objects may be classified into two categories: local and remote.
// <br>
// <ul>
// <li><b>Local address objects</b> are those addresses that are part of the
// provider's local domain. All of the provider's currently known local
// addresses are reported via the PtProvider.getAddresses() method. </li>
// <p>
// <li><B>Remote address objects</B> are those outside of the provider's domain
// that the provider learns about during its lifetime through various happenings
// (for example, an incoming call from a currently unknown address). Remote
// addresses are not reported via the PtProvider.getAddresses() method. </li>
// </ul><p>
// Note that applications may not create PtAddress objects. The PtProvider
// begins with knowledge of certain PtAddress objects within its local domain.
// This list is quasi-static, changing only as logical endpoints are added
// and removed in the phone system.
//
// <H3>Address and Terminal Objects</H3>
// PtAddress and PtTerminal objects exist in a many-to-many relationship. A
// PtAddress object may have zero or more PtTerminals associated with it.
// Each PtTerminal associated with a PtAddress must reflect its association
// with the PtAddress. The PtTerminals associated with a PtAddress are given
// by the PtAddress.getTerminals() method.
// <p>
// An association between a PtAddress and a PtTerminal indicates that the
// terminal is addressable via that address. In many instances, a telephone
// set (represented by a PtTerminal object) has only one telephone number
// (represented by a PtAddress object) associated with it. In more complex
// configurations, telephone sets may have several telephone numbers
// associated with them. Likewise, a telephone number may appear on more
// than one telephone set.
//
// <H3>Address and Call Objects</H3>
// PtAddress objects represent the logical endpoints of a telephone call. A
// logical view of a telephone call views the call as originating from one
// PtAddress endpoint and terminating at another PtAddress endpoint.
// <p>
// PtAddress objects are related to PtCall objects via the PtConnection
// object. The PtConnection object has a state that describes the current
// relationship between the call and the address. Each PtAddress object may
// be part of more than one telephone call, and in each case, is represented
// by a separate PtConnection object. The PtAddress.getConnections() method
// returns all PtConnection objects currently associated with the call.
// <p>
// An address is associated with a call until the connection moves into the
// PtConnection::DISCONNECTED state. At that time, the connection is no longer
// reported by the PtAddress.getConnections() method. Therefore, the
// PtAddress.getConnections() method will never report a connection in the
// PtConnection::DISCONNECTED state.
//
// <H3>Address Listeners and Events</H3>
// All changes in a PtAddress object are reported via PtAddressListener
// objects. Applications instantiate an object which is derived from the
// PtAddressListener class and use the PtAddress.addAddressListener() method
// to begin the delivery of events. All address-related events reported via
// a PtAddressListener are instances of either the PtAddressEvent class
// or its descendants.  Applications receive events on a listener until the
// listener is removed via the PtAddress.removeAddressListener() method, or
// until the PtAddress is no longer observable. In these instances, each
// AddressListener receives an ADDRESS_EVENT_TRANSMISSION_ENDED event as its
// final event.
//
// <H3>Call Listeners</H3>
// At times, applications may want to monitor a particular address for all
// calls that come to that address. For example, a customer service agent
// application may only be interested in telephone calls that are associated
// with a particular agent address. To achieve this sort of address-based call
// monitoring, applications may add PtCallListeners to a PtAddress via the
// PtAddress.addCallListener() method.
// <p>
// When a PtCallListener is added to a PtAddress, this listener instance is
// immediately added to all calls at this address, and is added to all calls
// which come to this address in the future. These listeners remain on the
// telephone call as long as the address is associated with the telephone
// call.
//
// <H3>Address Forwarding</H3>
// This class supports methods which permit applications to modify and query
// the forwarding characteristics of a PtAddress. The forwarding
// characteristics determine how incoming telephone calls to this PtAddress
// should be handled, if any special handling is desired.
// <p>
// Each PtAddress may have zero or more forwarding instructions. Each
// instruction describes how the switching domain should handle incoming
// telephone calls to a PtAddress under different circumstances. Examples of
// forwarding instructions are "forward all calls to x9999" or "forward all
// calls to x7777 when no one answers."  Each forwarding instruction is
// represented by an instance of the PtAddressForwarding class.
// <p>
// Applications assign a list of forwarding instructions via the
// PtAddress.setForwarding() method. To obtain the current,
// effective forwarding instructions, applications invoke the
// PtAddress.getForwarding() method. To cancel all forwarding
// instructions, applications use the PtAddress.cancelForwarding()
// method.
//
// <H3>Do-not-disturb and Message-waiting</H3>
// The <i>do-not-disturb</i> feature gives the means to notify the telephony
// platform that an address does not want to receive incoming telephone calls.
// That is, if this feature is activated, the underlying telephony platform
// will <b>not</b> alert this PtAddress to incoming telephone calls. Applications
// use the PtAddress.setDoNotDisturb() method to activate and deactivate this
// feature, and the PtAddress.getDoNotDisturb() method to return the current
// state of this attribute.
// <p>
// Note that the PtTerminal interface also has a <i>do-not-disturb</i>
// attribute. This gives the ability to control the <i>do-not-disturb</i>
// property at either the PtAddress level (for example, a phone number) or
// at the PtTerminal level (for example, an individual phone.)
// <p>
// The <i>message-waiting</i> attribute indicates whether there are messages
// waiting for a human user of the address. These messages may be maintained
// either by an application or by some telephony platform. Applications
// inform the phone set of the message waiting status, and typically
// the phone set displays a visible indicator (such as an LED) to users.
// Applications use the PtAddress.setMessageWaiting() method to activate and
// deactivate this feature, and the PtAddress.getMessageWaiting() method to
// return the current state of this attribute.
//
// <H3>Offered Timeout</H3>
// The offered timeout attribute for a PtAddress indicates how long the
// connection for an incoming call will stay in the PtConnection::OFFERED
// state before it transitions to the PtConnection::ALERTING state.  By
// default, the offered timeout for a PtAddress is 0.  This means that as
// soon as the connection for the incoming call is offered on a PtAddress,
// the PtTerminals for that address will begin alerting (by ringing, for
// example).
// <p>
// To implement services such as call screening or find-me-follow-me,
// applications must be allowed to explicitly accept, reject or
// redirect connections that are offered to a PtAddress.  Setting the
// offered timeout for a PtAddress to a positive number causes the
// PtConnection to stay in the PtConnection::OFFERED state until the
// PtCall is either explicitly accepted, rejected, or redirected, or until the
// offered timeout expires, whichever comes first.  If the offered timeout
// expires before the call is explicitly accepted, rejected, or
// redirected, the PtConnection transitions to the PtConnection::ALERTING
// state.
// <p>
// The timer-based transition of the PtConnection from
// PtConnection::OFFERED to PtConnection::ALERTING may be disabled by
// setting the offered timeout for the PtAddress to -1.

class PtAddress
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

        PtAddress();
         //:Default constructor

        PtAddress(TaoClientTask *pClient, const char* name);

        PtAddress(PtProvider *pProvider, const char* address);

        PtAddress(const PtAddress& rPtAddress);
         //:Copy constructor (not implemented for this class)

        PtAddress(const char* address);

        virtual
        ~PtAddress();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtAddress& operator=(const PtAddress& rhs);
     //:Assignment operator (not implemented for this class)

   virtual PtStatus addAddressListener(PtAddressListener& rAddressListener);
     //:Adds a listener to this address.
     //!param: (in) rAddressListener - The listener to add to this address
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_EXISTS - <i>rAddressListener</i> is already registered
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus addCallListener(PtCallListener& rCallListener);
     //:Adds a listener to a PtCall object when this PtAddress object first
     //:becomes part of that PtCall.
     //!param: (in) rCallListener - The listener to add to calls associated with this address
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_EXISTS - <i>rCallListener</i> is already registered
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus cancelForwarding(void);
     //:Cancels all of the forwarding instructions on this address.
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus cancelForwarding(PtAddressForwarding forwards[], int size);
     //:Cancels forwarding instructions in forwards on this address.
     //!param: (in) forwards - the forwarding instructions to remove
     //!param: (in) size - the number of forwarding instructions to remove
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available


   virtual PtStatus removeCallListener(PtCallListener& rCallListener);
     //:Removes the indicated PtCallListener from this PtAddress.
     // This method removes a PtCallListener which was added via the
     // PtAddress.addCallListener() method. If successful, the listener will
     // no longer be added to new calls which are presented to this address,
     // however it does not affect PtCallListeners which have already been
     // added to a call.
     //!param: (in) rCallListener - the listener to remove
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_NOT_FOUND - <i>rCallListener</i> not registered
     //!retcode: PT_PROVIDER_UNAVAILABLE - the provider is not available

   virtual PtStatus removeAddressListener(PtAddressListener& rAddressListener);
     //:Removes the indicated listener from this PtAddress.
     //!param: (in) rAddressListener - the listener to remove from this address
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_NOT_FOUND - <i>rAddressListener</i> not registered
     //!retcode: PT_PROVIDER_UNAVAILABLE - the provider is not available

   virtual PtStatus setDoNotDisturb(PtBoolean flag);
     //:Specifies whether the <i>do-not-disturb</i> feature should be
     //:activated or deactivated for this address.
     //!param: (in) flag - TRUE ==> enable, FALSE ==> disable
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - the provider is not available

   virtual PtStatus setForwarding(PtAddressForwarding forwards[], int size);
     //:Sets the forwarding characteristics for this address.
     //!param: (in) forwards - the array of call forwarding instructions
     //!param: (in) size - the number of forwarding instructions in the <i>forwards</i> array
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - the provider is not available

   virtual PtStatus setMessageWaiting(PtBoolean flag);
     //:Specifies whether the <i>message-waiting</i> indicator should be
     //:activated or deactivated for this address.
     //!param: (in) flag - TRUE ==> on, FALSE ==> off
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - the provider is not available

   virtual PtStatus setOfferedTimeout(int milliSecs);
     //:Specifies the timeout value for transitioning from the
     //:PtConnection::OFFERED state to the PtConnection::ALERTING state for
     //:an incoming call to this address.
     // If <i>milliSecs</i> is 0, a PtConnection in the PtConnection::OFFERED
     // state will immediately transition to the PtConnection::ALERTING state.
     // If <i>milliSecs</i> is greater than 0, the connection will stay in
     // the PtConnection::OFFERED state for the indicated number of
     // milliseconds or until the connection is explicitly handled (by calling
     // PtConnection.accept(), PtConnection.reject() or
     // PtConnection.redirect()). whichever comes first.  If the timeout
     // expires before the connection is explicitly handled, the connection
     // will then transition into the PtConnection::ALERTING state.  Setting
     // <i>milliSecs</i> to -1, is equivalent to requesting an infinite
     // timeout.
     //!param: (in) milliSecs - number of milliseconds to delay
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - the provider is not available

/* ============================ ACCESSORS ================================= */

   virtual PtStatus getAddressListeners(PtAddressListener* addrListeners[],
                                int size, int& nItems);
     //:Returns an array of PtAddressListener pointers for all of the
     //:address listeners presently associated with this address.
     // The caller provides an array that can hold up to <i>size</i>
     // PtAddressListener pointers.  This method will fill in the
     // <i>addrListeners</i> array with up to <i>size</i> pointers.  The
     // actual number of pointers filled in is passed back via the
     // <i>nItems</i> argument.
     //!param: (out) addrListeners - the array of pointers to known address listeners
     //!param: (in) size - the number of elements in the <i>addrListeners</i> array
     //!param: (out) nItems - the number of items assigned
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_MORE_DATA - there are more than <i>size</i> listeners
     //!retcode: PT_PROVIDER_UNAVAILABLE - the provider is not available

   virtual PtStatus getCallListeners(PtCallListener* callListeners[], int size,
                             int& nItems);
     //:Returns an array of PtCallListener pointers for all of the call
     //:listeners presently associated with this address.
     // The caller provides an array that can hold up to <i>size</i>
     // PtCallListener pointers.  This method will fill in the
     // <i>callListeners</i> array with up to <i>size</i> pointers.  The
     // actual number of pointers filled in is passed back via the
     // <i>nItems</i> argument.
     //!param: (out) callListeners - the array of pointers to known call listeners
     //!param: (in) size - the number of elements in the <i>callListeners</i> array
     //!param: (out) nItems - the number of items assigned
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_MORE_DATA - there are more than <i>size</i> listeners
     //!retcode: PT_PROVIDER_UNAVAILABLE - the provider is not available

   virtual PtStatus getConnections(PtConnection connections[], int size,
                           int& nItems);
     //:Returns an array of pointers to PtConnection objects currently
     //:associated with this address.
     // The caller provides an array that can hold up to <i>size</i>
     // PtConnection pointers.  This method will fill in the
     // <i>connections</i> array with up to <i>size</i> pointers.  The
     // actual number of pointers filled in is passed back via the
     // <i>nItems</i> argument.
     //!param: (out) connections - the array of PtConnection pointers
     //!param: (in) size - the number of elements in the <i>connections</i> array
     //!param: (out) nItems - the number of items assigned
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_MORE_DATA - there are more than <i>size</i> connections
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getDoNotDisturb(PtBoolean& rFlag);
     //:Returns the current setting for the <i>do-not-disturb</i> feature.
     //!param: (out) rFlag - TRUE ==> enabled, FALSE ==> disabled
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getForwarding(PtAddressForwarding forwards[], int size,
                          int& nItems);
     //:Returns an array of forwarding instructions currently associated with
     //:this address.
     // The caller provides an array of PtAddressForwarding objects containing
     // <i>size</i> elements.  This method will fill in the <i>forwards</i>
     // array with up to <i>size</i> objects.  The actual number of items
     // filled in is passed back via the <i>nItems</i> argument.
     //!param: (out) forwards - The array of PtAddressForwarding objects
     //!param: (in) size - The number of elements in the <i>forwards</i> array
     //!param: (out) nItems - The number of items assigned
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_MORE_DATA - There are more than <i>size</i> forwards
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getMessageWaiting(PtBoolean& rFlag);
     //:Returns the current setting for the <i>message waiting</i> indicator.
     //!param: (out) rFlag - TRUE ==> on, FALSE ==> off
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   PtStatus getName(char* rpName, int len);
     //:Returns the name associated with this PtAddress.
     //!param: (out) rpName - The reference used to return the name
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getOfferedTimeout(int& rMilliSecs);
     //:Sets <i>rMilliSecs</i> to the timeout value on this PtAddress for
     //:transitioning from the PtConnection::OFFERED state to the
     //:PtConnection::ALERTING state for an incoming call to this address.
     // See the description of PtAddress.setOfferedTimeout() for further
     // information on the offered timeout mechanism.
     //!param: (out) rMilliSecs - Set to the offered timeout value
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getProvider(PtProvider& rpProvider);
     //:Returns a pointer to the PtProvider associated with this PtAddress.
     //!param: (out) rpProvider - a pointer to the PtProvider object associated with this address
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getTerminals(PtTerminal terms[], int size, int& nItems);
     //:Returns an array of pointers to PtTerminal objects currently
     //:associated with this address.
     // The caller provides an array that can hold up to <i>size</i>
     // PtTerminal pointers.  This method will fill in the <i>terms</i> array
     // with up to <i>size</i> pointers.  The actual number of pointers
     // filled in is passed back via the <i>nItems</i> argument.
     //!param: (out) terms - The array of PtTerminal pointers
     //!param: (in) size - The number of elements in the <i>terms</i> array
     //!param: (out) nItems - The number of items assigned
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_MORE_DATA - There are more than <i>size</i> terminals
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus numAddressListeners(int& count);
     //:Returns the number of address listeners associated with this address.
     //!param: (out) count - The number of address listeners associated with this address
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus numCallListeners(int& count);
     //:Returns the number of call listeners associated with this address.
     //!param: (out) count - The number of call listeners associated with this address
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus numConnections(int& count);
     //:Returns the number of connections associated with this address.
     //!param: (out) count - The number of connections associated with this address
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus numForwards(int& count);
     //:Returns the number of forwarding instructions associated with this
     //:address.
     //!param: (out) count - The number of forwarding instructions associated with this address
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus numTerminals(int& count);
     //:Returns the number of terminals associated with this address.
     //!param: (out) count - The number of terminals associated with this address
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    static OsBSem           semInit ;
      //: Binary Semaphore used to guard initialiation and tear down
        static TaoReference             *mpTransactionCnt;
        static unsigned int     mRef;

        TaoClientTask   *mpClient;
        UtlString                       mAddress;

        OsTime          mTimeOut;

        void initialize();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        OsProtectEventMgr *mpEventMgr;

        TaoReference            *mpAddressListenerCnt;
        TaoObjectMap            *mpAddressListenerDb;

    static OsBSem                                mAddressForwardDbSem;
        static PtAddressForwarding      *mpAddressForwards;
        static int                                       mAddressForwardCnt;

        static PtBoolean        mbMessageWaiting;
        static PtBoolean        mbDoNotDisturb;
        static PtBoolean        mOfferedTimeout;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtAddress_h_

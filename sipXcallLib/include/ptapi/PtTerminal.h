//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtTerminal_h_
#define _PtTerminal_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "ptapi/PtDefs.h"
#include "tao/TaoDefs.h"
#include "os/OsBSem.h"
#include "os/OsProtectEventMgr.h"

// DEFINES
#define PTTERMINAL_MAX_NAME_LENGTH      128
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PtCallListener;
class PtTerminalListener;
class PtTerminalConnection;
class PtAddress;
class PtComponent;
class PtConfigDb;
class PtProvider;
class PtGatewayInterface;
class PtComponentGroup;
class TaoClientTask;
class TaoServerTask;
class TaoReference;
class TaoObjectMap;


//:A PtTerminal represents a physical hardware endpoint connected to the
//:telephony domain (that is, a computer workstation or a Pingtel phone
//:acting as a physical endpoint in a telephony network).
// A PtTerminal object has a string name that is unique for all PtTerminal
// objects . The PtTerminal does not attempt to interpret this string in any
// way. This name is first assigned when the PtTerminal is created and does
// not change throughout the lifetime of the object. The method
// PtTerminal.getName() returns the name of the PtTerminal object. The name
// of the PtTerminal may not have any real-world interpretation. Typically,
// PtTerminals are chosen from a list of PtTerminals obtained from a
// PtAddress object.
// <p>
// Like PtAddresses, PtTerminal objects may be classified into two
// categories: local and remote.
// <p>
// <ul>
// <li> <b>Local terminal objects</b> are those terminals that are part of the
// provider's local domain. These terminal objects are created by the
// implementation of
// the provider object when it is first instantiated.  All of the provider's
// currently known local terminals are reported via the
// PtProvider.getTerminals() method. </li><br>
// <li>
// <b>Remote terminal objects</b> are those outside
// of the provider's domain which the provider learns about during its
// lifetime through various happenings (such as an incoming call from a
// currently unknown address). Remote terminal objects are not reported via
// the PtProvider.getTerminals() method. </li>
// </ul>
// <p>Note that applications may not
// create PtTerminal objects. The PtProvider begins with knowledge of certain
// PtTerminal objects within its local domain. This list is quasi-static,
// changing only as physical endpoints are added and removed in the phone
// system.
//
// <H3>PtTerminal and PtAddress objects</H3>
// PtAddress and PtTerminal objects exist in a many-to-many relationship. A
// PtAddress object may have zero or more PtTerminals associated with it. For
// each PtTerminal associated with a PtAddress, that PtTerminal must also
// reflect its association with the PtAddress. The PtTerminals associated with
// a PtAddress are given by the PtAddress.getTerminals() method.<br>
// <br>
// An association between a PtAddress and PtTerminal object indicates that
// the PtTerminal contains the PtAddress object as one of its telephone number
// addresses. In many instances, a telephone set (represented by a PtTerminal
// object) has only one telephone number (represented by a PtAddress object)
// associated with it. In more complex configurations, telephone sets may have
// several telephone numbers associated with them. Likewise, a telephone
// number may appear on more than one telephone set. For example, feature
// phones in PBX environments may exhibit this configuration.
//
// <H3>PtTerminal and PtCall objects</H3>
// PtTerminal objects represent the physical endpoints of a telephone call.
// With respect to a single PtAddress endpoint on a PtCall, multiple physical
// PtTerminal endpoints may exist. PtTerminal objects are related to PtCall
// objects via the PtTerminalConnection object. PtTerminalConnection objects
// are associated with the PtCall indirectly via PtConnections. A PtTerminal
// may be associated with a PtCall only if one of its addresses is associated
// with the call. The PtTerminalConnection object has a state that describes
// the current relationship between the PtConnection and the PtTerminal. Each
// PtTerminal object may be part of more than one telephone call, and in each
// case, is represented by a separate PtTerminalConnection object. The
// PtTerminal.getTerminalConnections() method returns all PtTerminalConnection
// objects currently associated with the PtTerminal.<br>
// <br>
// A PtTerminal object is associated with a PtConnection until the
// PtTerminalConnection moves into the PtTerminalConnection::DROPPED state.
// At that time, the PtTerminalConnection is no longer reported via the
// PtTerminal.getTerminalConnections() method. Therefore, the
// PtTerminal.getTerminalConnections() method never reports a
// PtTerminalConnection in the PtTerminalConnection::DROPPED state.
//
// <H3>PtTerminal Components</H3>
// Applications may query each PtTerminal for the components of which it is
// made and may monitor and control certain attributes of these components.
// The component types that are presently supported include speakerphone,
// microphone, display, buttons, ringer, handset, hookswitch, and lamp. Each
// component type is derived from the PtComponent base class and exports a
// standard interface to control its attributes. For example, the ringer
// permits applications to control its volume, while the buttons permit
// applications to simulate pressing buttons.
//
// <H3>Terminal Listeners and Events</H3>
// All changes in a PtTerminal object are reported via PtTerminalListener
// objects. Applications instantiate an object which is derived from the
// PtTerminalListener class and use the PtTerminal.addTerminalListener()
// method to begin the delivery of events. All terminal-related events
// reported via a PtTerminalListener are instances of either the
// PtTerminalEvent class or its descendants.  Applications receive events on
// a listener until the
// listener is removed via the PtTerminal.removeTerminalListener() method or
// until the PtTerminal is no longer observable. In these instances, each
// PtTerminalListener receives a TERMINAL_EVENT_TRANSMISSION_ENDED event as
// its final event.<br>
// <br>
// To receive event reports regarding the PtTerminal's components (such as
// hookswitch state changes or button presses), applications instantiate
// an object which is derived from the PtTerminalComponentListener class
// and use the PtTerminal.addTerminalListener() method to begin the delivery
// of events.  Since the PtTerminalComponentListener is a descendant of the
// PtTerminalListener class, it receives event reports for changes in the
// PtTerminal object as well as changes in the PtTerminal's component objects.
//
// <H3>Call Listeners</H3>
// At times, applications may want to monitor a particular PtTerminal for all
// calls which come to that terminal. For example, a desktop telephone
// application may only be interested in telephone calls associated with a
// particular agent terminal. To achieve this sort of terminal-based call
// monitoring, applications may add PtCallListeners to a PtTerminal via the
// PtTerminal.addCallListener() method.<br>
// <br>
// When a PtCallListener is added to a PtTerminal, this listener instance
// is immediately added to all calls at this terminal, and is added to all
// calls which come to this terminal in the future. These listeners remain on
// the telephone call as long as the terminal is associated with the
// telephone call.
//
// <H3>Do Not Disturb</H3>
// The PtTerminal class defines the <i>do-not-disturb</i> attribute. The
// <i>do-not-disturb</i> attribute indicates to the telephony platform that
// this terminal does not want to be bothered with incoming telephone calls.
// That is, if this feature is activated, the underlying telephone platform
// will not ring this terminal for incoming telephone calls. Applications use
// the PtTerminal.setDoNotDisturb() method to activate and deactivate this
// feature and the PtTerminal.getDoNotDisturb() method to return the current
// state of this attribute.<br>
// <br>
// Note that the PtAddress class also carries the <i>do-not-disturb</i>
// attribute. The attributes associated with each class are maintained
// independently. Maintaining a separate <i>do-not-disturb</i> attribute at
// both the terminal and address allows for control over the
// <i>do-not-disturb</i> feature at either the terminal or address level.
class PtTerminal
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

        PtTerminal();
         //:Default constructor

        PtTerminal(const char* terminalName, TaoClientTask *pClient = NULL);

        virtual
        ~PtTerminal();
         //:Destructor

        PtTerminal(const PtTerminal& rPtTerminal);
         //:Copy constructor (not implemented for this class)

        PtTerminal& operator=(const PtTerminal& rhs);
         //:Assignment operator (not implemented for this class)

/* ============================ MANIPULATORS ============================== */

   virtual PtStatus addCallListener(PtCallListener& rCallListener);
     //:Adds a listener to a PtCall object when this PtTerminal object first
     //:becomes part of that PtCall.
     //!param: (in) rCallListener - the listener to add to calls associated with this terminal
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_EXISTS - <i>rCallListener</i> is already registered
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus addTerminalListener(PtTerminalListener& rTerminalListener);
     //:Adds a listener to this terminal.
     //!param: (in) rTerminalListener - The listener to add to this terminal
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_EXISTS - <i>rTerminalListener</i> is already registered
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   PtStatus setGatewayInterface(PtGatewayInterface& rGatewayInterface);
     //: Indicate that this terminal is a gateway and that audio media
     //: streams capabilities and setup is performed via this interface.
     //!param: (in) rGatewayInterface - reference to the gateway interface on which media setup methods are invoked during in/out going call setup for this terminal.
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_EXISTS - <i>rGatewayInterface</i> is already registered on this terminal.
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus pickup(PtAddress& rPickupAddress, PtAddress& rTerminalAddress,
                   PtTerminalConnection*& rpNewTermConnection);
     //:This method "picks up" a PtCall at this PtTerminal.
     // Picking up a PtCall is analogous to answering a PtCall at this
     // PtTerminal (i.e., PtTerminalConnection.answer()), except that the
     // PtCall is typically not ringing at this PtTerminal. For example,
     // this method is used to answer a "queued" PtCall or a PtCall that is
     // ringing at another PtTerminal across the room.<br>
     // <br>
     // The <i>rPickupAddress</i> represents the destination end of the
     // telephone call to be picked up.  The corresponding PtConnection must
     // be in either the PtConnection::QUEUED state or the
     // PtConnection.ALERTING state. The <i>rTerminalAddress</i> argument
     // chooses the PtAddress associated with this PtTerminal on which to
     // pick up the PtCall. If this method succeeds, <i>rpNewTermConnection</i>
     // is set to point to a new PtTerminalConnection that is in the
     // PtTerminalConnection::TALKING state.
     //!param: rPickupAddress - The PtAddress that is to be picked up
     //!param: rTerminalAddress - The PtAddress associated with this terminal on which to pick up the call
     //!param: rpNewTermConnection - Set to point to the PtTerminalConnection corresponding to the picked up PtConnection.
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_INVALID_STATE - There is no connection for <i>rTerminalAddress</i> in either the QUEUED or ALERTING state.
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus removeCallListener(PtCallListener& rCallListener);
     //:Removes the indicated PtCallListener from the PtTerminal.
     // This method removes a PtCallListener that was added via the
     // PtTerminal.addCallListener() method. If successful, the listener will
     // no longer be added to new calls which are presented to this terminal,
     // however it does not affect PtCallListeners that have already been
     // added to a call.
     //!param: (in) rCallListener - The listener to remove
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_NOT_FOUND - <i>rCallListener</i> not registered
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus removeTerminalListener(PtTerminalListener& rTerminalListener);
     //:Removes the indicated listener from the PtTerminal.
     //!param: (in) rTerminalListener - The listener to remove from this terminal
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_NOT_FOUND - <i>rTerminalListener</i> not registered
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus setDoNotDisturb(PtBoolean flag);
     //:Specifies whether the <i>do-not-disturb</i> feature should be
     //:activated or deactivated for this terminal.
     //!param: (in) flag - TRUE ==> enable, FALSE ==> disable
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus setCodecCPULimit(int limit);
     //:Sets the codec CPU limit level for inbound calls.
     //!param (in) limit - The codec/CPU limit for this call.  The value can
     //       be set to LOW (0) or HIGH (1).  If set to LOW, only LOW CPU
     //       intensive codecs are allowed.  If set to HIGH, both LOW and
     //       HIGH CPU intensive codes are allowed.
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_BUSY - Unable to communicate with call processing


/* ============================ ACCESSORS ================================= */

   virtual PtStatus getAddresses(PtAddress addresses[], int size, int& nItems);
     //:Returns an array of PtAddress objects associated with this
     //:terminal.
     // The caller provides an array that can hold up to <i>size</i>
     // PtAddresses.  This method will fill in the <i>aAddresses</i>
     // array with up to <i>size</i> pointers.  The actual number of items
     // filled in is passed back via the <i>nItems</i> argument.
     //!param: (out) aAddresses - The array of PtAddress's
     //!param: (in) size - The number of elements in the <i>addresses</i> array
     //!param: (out) nItems - The number of items assigned
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_MORE_DATA - There are more than <i>size</i> addresses
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getCallListeners(PtCallListener aCallListeners[], int size,
                             int& nItems);
     //:Returns an array of PtCallListener,s for all of the call
     //:listeners presently associated with this terminal.
     // The caller provides an array that can hold up to <i>size</i>
     // PtCallListener's.  This method will fill in the
     // <i>aCallListeners</i> array with up to <i>size</i> PtCallListener's. The
     // actual number items filled in is passed back via the
     // <i>nItems</i> argument.
     //!param: (out) aCallListeners - The array of known call listeners
     //!param: (in) size - The number of elements in the <i>callListeners</i> array
     //!param: (out) nItems - The number of items assigned
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_MORE_DATA - There are more than <i>size</i> listeners
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getComponent(const char* componentName, PtComponent& rComponent);
     //:Sets <i>rpComponent</i> to refer to the PtComponent object that
     //:corresponds to <i>componentName</i> or NULL if there is no
     //:component with that name.
     //!param: (in) componentName - The name of the desired component
     //!param: (out) rpComponent - Set to point to the component that corresponds to <i>componentName</i>
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getComponents(PtComponent* components[], int size, int& nItems);
     //:Returns an array of PtComponent's for all of the objects used
     //:to model the telephony hardware associated with this terminal.
     // The caller provides an array that can hold up to <i>size</i>
     // PtComponent's. This method will fill in the <i>components</i>
     // array with up to <i>size</i> objects.  The actual number of objects
     // filled in is passed back via the <i>nItems</i> argument.
     //!param: (out) components - The array of PtComponent's
     //!param: (in) size - The number of elements in the <i>components</i> array
     //!param: (out) nItems - The number of items assigned
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_MORE_DATA - There are more than <i>size</i> components
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

        virtual PtStatus getComponentGroups(PtComponentGroup componentGroup[], int size, int& nItems);
     //:Returns an array of ComponentGroup objects available on the Terminal. A
     //:ComponentGroup object is composed of a number of Components.
     //:Examples of Component objects include headsets, handsets,
     // speakerphones, and buttons. ComponentGroup objects group Components
     // together.
     //!param: (out) pComponentGroup - The array of PtComponetGroups
     //!param: (in) size - The number of elements in the <i>components</i> array
     //!param: (out) nItems - The number of items assigned
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_MORE_DATA - There are more than <i>size</i> components
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getConfiguration(PtConfigDb& rpConfigDb);
     //:Sets <i>rpConfigDb</i> to refer to the configuration object for
     //:this PtTerminal.
     // See the PtConfigDb class for information on getting and setting
     // configuration information.
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getDoNotDisturb(PtBoolean& rFlag);
     //:Returns the current setting for the <i>do-not-disturb</i> feature.
     //!param: (out) rFlag - TRUE ==> enabled, FALSE ==> disabled
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   PtStatus getName(char* rpName, int maxLen);
     //:Returns the name associated with this PtTerminal.
     //!param: (out) rpName - The reference used to return the name
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getTerminalListeners(PtTerminalListener aTermListeners[],
                                int size, int& nItems);
     //:Returns an array of PtTerminalListener's for all of the
     //:terminal listeners presently associated with this terminal.
     // The caller provides an array that can hold up to <i>size</i>
     // PtTerminalListener's.  This method will fill in the
     // <i>termListeners</i> array with up to <i>size</i> objects. The
     // actual number of pointers filled in is passed back via the
     // <i>nItems</i> argument.
     //!param: (out) aTermListeners - The array of known terminal listeners
     //!param: (in) size - The number of elements in the <i>termListeners</i> array
     //!param: (out) nItems - The number of items assigned
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_MORE_DATA - There are more than <i>size</i> listeners
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getProvider(PtProvider& rpProvider);
     //:Returns a reference to the PtProvider associated with this PtTerminal.
     //!param: (out) rpProvider - a reference to the PtProvider object associated with this terminal
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getTerminalConnections(PtTerminalConnection termConnections[],
                                   int size, int& nItems);
     //:Returns an array of PtTerminalConnection's for all of the
     //:terminal connections currently associated with this terminal.
     // The caller provides an array that can hold up to <i>size</i>
     // PtTerminalConnection's.  This method will fill in the
     // <i>termConnections</i> array with up to <i>size</i> objects.  The
     // actual number of items filled in is passed back via the <i>nItems</i>
     // argument.
     //!param: (out) aTermConnections - The array of PtTerminalConnection objects
     //!param: (in) size - The number of elements in the <i>termConnections</i> array
     //!param: (out) nItems - The number of items assigned
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_MORE_DATA - There are more than <i>size</i> terminal connections
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus numAddresses(int& count);
     //:Returns the number of addresses associated with this terminal.
     //!param: (out) count - The number of addresses associated with this terminal
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus numCallListeners(int& count);
     //:Returns the number of call listeners associated with this terminal.
     //!param: (out) count - The number of call listeners associated with this terminal
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus numComponents(int& count);
     //:Returns the number of components associated with this terminal.
     //!param: (out) count - The number of components associated with this terminal
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus numTerminalConnections(int& count);
     //:Returns the number of terminal connections associated with this terminal.
     //!param: (out) count - The number of terminal connections associated with this terminal
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus numTerminalListeners(int& count);
     //:Returns the number of terminal listeners associated with this terminal.
     //!param: (out) count - The number of terminal listeners associated with this terminal
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

friend class PtProvider;
friend class PtTerminalConnection;
friend class PtTerminalEvent;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
protected:
        OsTime          mTimeOut;

        void initialize(const char *name);

    static OsBSem           semInit ;
      //: Binary Semaphore used to guard initialiation and tear down
        static TaoReference             *mpTransactionCnt;
        static TaoObjectMap             *mpComponents;  // the database for all existing components
        static TaoObjectMap             *mpComponentGroups;     // the database for all existing components
        static int                              mRef;

   char mTerminalName[PTTERMINAL_MAX_NAME_LENGTH + 1];
   TaoClientTask *mpClient;

   enum PtComponentId
        {
                HEAD_SET = 1,
                HAND_SET,
                PHONE_SET,
                SPEAKER_PHONE,
                OTHER,
                BUTTON,
                DISPLAY,
                GRAPHIC_DISPLAY,
                HOOKSWITCH,
                LAMP,
                MICROPHONE,
                RINGER,
                SPEAKER,
                EXTERNAL_SPEAKER
        };

private:
        OsProtectEventMgr *mpEventMgr;

    void setName(const char *name);


};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtTerminal_h_

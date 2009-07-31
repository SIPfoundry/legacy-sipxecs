//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtTerminalConnectionListener_h_
#define _PtTerminalConnectionListener_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <ptapi/PtConnectionListener.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PtTerminalConnectionEvent;

//:The PtTerminalConnectionListener is used to register with PtAddress,
//:PtTerminal, and PtCall objects to receive events from associated PtCall,
//:PtConnection, and PtTerminalConnection objects.

class PtTerminalConnectionListener : public PtConnectionListener
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   PT_CLASS_INFO_MEMBERS

/* ============================ CREATORS ================================== */

   PtTerminalConnectionListener(PtEventMask* pMask = NULL);
     //:Default constructor
     //!param: (in) pMask - Event mask defining events the listener is interested in.  This must be a subset of the events that the listener supports.  The mask may be NULL where it is assumed that all events applicable to the derived listener are of interest.

   virtual
   ~PtTerminalConnectionListener();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

  virtual void terminalConnectionCreated(const PtTerminalConnectionEvent& rEvent);
     //:Method invoked on listener for event id =
     //:TERMINAL_CONNECTION_CREATED
     //:indicating that a new PtTerminalConnection object has been created.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void terminalConnectionIdle(const PtTerminalConnectionEvent& rEvent);
     //:Method invoked on listener for event id =
     //:TERMINAL_CONNECTION_IDLE
     //:indicating that the state of the PtTerminalConnection object has
     //:changed to PtTerminalConnection::IDLE.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - reference to the PtEvent containing the specific event information.

  virtual void terminalConnectionRinging(const PtTerminalConnectionEvent& rEvent);
     //:Method invoked on listener for event id =
     //:TERMINAL_CONNECTION_RINGING
     //:indicating that the state of the PtTerminalConnection object has
     //:changed to PtTerminalConnection::RINGING.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void terminalConnectionDropped(const PtTerminalConnectionEvent& rEvent);
     //:Method invoked on listener for event id =
     //:TERMINAL_CONNECTION_DROPPED
     //:indicating that the state of the PtTerminalConnection object has
     //:changed to PtTerminalConnection::DROPPED.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void terminalConnectionUnknown(const PtTerminalConnectionEvent& rEvent);
     //:Method invoked on listener for event id =
     //:TERMINAL_CONNECTION_UNKNOWN
     //:indicating that the state of the PtTerminalConnection object has
     //:changed to PtTerminalConnection::UNKNOWN.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void terminalConnectionHeld(const PtTerminalConnectionEvent& rEvent);
     //:Method invoked on listener for event id =
     //:TERMINAL_CONNECTION_HELD
     //:indicating that the state of the PtTerminalConnection object has
     //:changed to PtTerminalConnection::HELD.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void terminalConnectionTalking(const PtTerminalConnectionEvent& rEvent);
     //:Method invoked on listener for event id =
     //:TERMINAL_CONNECTION_TALKING
     //:indicating that the state of the PtTerminalConnection object has
     //:changed to PtTerminalConnection::TALKING.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void terminalConnectionInUse(const PtTerminalConnectionEvent& rEvent);
     //:Method invoked on listener for event id =
     //:TERMINAL_CONNECTION_IN_USE
     //:indicating that the state of the PtTerminalConnection object has
     //:changed to PtTerminalConnection::IN_USE.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - reference to the PtEvent containing the specific event information.

/* ============================ ACCESSORS ================================= */

   static const char* className();
     //:Returns the name of this class
     //!returns: Returns the string representation of the name of this class

/* ============================ INQUIRY =================================== */

   virtual PtBoolean isClass(const char* pClassName);
     //:Determines if this object if of the specified type.
     //!param: (in) pClassName - the string to compare with the name of this class.
     //!retcode: TRUE - if the given string contains the class name of this class.
     //!retcode: FALSE - if the given string does not match that of this class

   virtual PtBoolean isInstanceOf(const char* pClassName);
     //:Determines if this object is either an instance of or is derived from
     //:the specified type.
     //!param: (in) pClassName - the string to compare with the name of this class.
     //!retcode: TRUE - if this object is either an instance of or is derived from the specified class.
     //!retcode: FALSE - if this object is not an instance of the specified class.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   PtTerminalConnectionListener(const PtTerminalConnectionListener& rPtTerminalConnectionListener);
     //:Copy constructor

   PtTerminalConnectionListener& operator=(const PtTerminalConnectionListener& rhs);
     //:Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtTerminalConnectionListener_h_

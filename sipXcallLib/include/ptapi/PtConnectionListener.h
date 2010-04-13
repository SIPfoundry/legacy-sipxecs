//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtConnectionListener_h_
#define _PtConnectionListener_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <ptapi/PtCallListener.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PtConnectionEvent;

//:The PtConnectionListener is used to register with PtAddress, PtTerminal,
//:and PtCall objects to receive events from associated PtCall and
//:PtConnection objects.

class PtConnectionListener : public PtCallListener
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   PT_CLASS_INFO_MEMBERS

/* ============================ CREATORS ================================== */

   PtConnectionListener(PtEventMask* pMask = NULL);
     //:Default constructor
     //!param: (in) pMask - Event mask defining events the listener is interested in.  This must be a subset of the events that the listener supports.  The mask may be NULL where it is assumed that all events applicable to the derived listener are of interest.

   virtual
   ~PtConnectionListener();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

  virtual void connectionCreated(const PtConnectionEvent& rEvent);
     //:Method invoked on listener for event id =
     //:CONNECTION_CREATED
     //:indicating that a new PtConnection object has been created.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void connectionAlerting(const PtConnectionEvent& rEvent);
     //:Method invoked on listener for event id =
     //:CONNECTION_ALERTING
     //:indicating that the state of the PtConnection object has changed to
     //:PtConnection::ALERTING.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void connectionDisconnected(const PtConnectionEvent& rEvent);
     //:Method invoked on listener for event id =
     //:CONNECTION_DISCONNECTED
     //:indicating that the state of the PtConnection object has changed to
     //:PtConnection::DISCONNECTED.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void connectionFailed(const PtConnectionEvent& rEvent);
     //:Method invoked on listener for event id =
     //:CONNECTION_FAILED
     //:indicating that the state of the PtConnection object has changed to
     //:PtConnection::FAILED.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void connectionUnknown(const PtConnectionEvent& rEvent);
     //:Method invoked on listener for event id =
     //:CONNECTION_UNKNOWN
     //:indicating that the state of the PtConnection object has changed to
     //:PtConnection::UNKNOWN.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void connectionDialing(const PtConnectionEvent& rEvent);
     //:Method invoked on listener for event id =
     //:CONNECTION_DIALING
     //:indicating that the state of the PtConnection object has changed to
     //:PtConnection::DIALING.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void connectionEstablished(const PtConnectionEvent& rEvent);
     //:Method invoked on listener for event id =
     //:CONNECTION_ESTABLISHED
     //:indicating that the state of the PtConnection object has changed to
     //:PtConnection::ESTABLISHED.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void connectionInitiated(const PtConnectionEvent& rEvent);
     //:Method invoked on listener for event id =
     //:CONNECTION_INITIATED
     //:indicating that the state of the PtConnection object has changed to
     //:PtConnection::INITIATED.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void connectionNetworkAlerting(const PtConnectionEvent& rEvent);
     //:Method invoked on listener for event id =
     //:CONNECTION_NETWORK_ALERTING
     //:indicating that the state of the PtConnection object has changed to
     //:PtConnection::NETWORK_ALERTING.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void connectionNetworkReached(const PtConnectionEvent& rEvent);
     //:Method invoked on listener for event id =
     //:CONNECTION_NETWORK_REACHED
     //:indicating that the state of the PtConnection object has changed to
     //:PtConnection::NETWORK_REACHED.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void connectionOffered(const PtConnectionEvent& rEvent);
     //:Method invoked on listener for event id =
     //:CONNECTION_OFFERED
     //:indicating that the state of the PtConnection object has changed to
     //:PtConnection::OFFERED.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void connectionQueued(const PtConnectionEvent& rEvent);
     //:Method invoked on listener for event id =
     //:CONNECTION_QUEUED
     //:indicating that the state of the PtConnection object has changed to
     //:PtConnection::QUEUED.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

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

   PtConnectionListener(const PtConnectionListener& rPtConnectionListener);
     //:Copy constructor

   PtConnectionListener& operator=(const PtConnectionListener& rhs);
     //:Assignment operator
};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtConnectionListener_h_

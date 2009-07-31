//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtTerminalListener_h_
#define _PtTerminalListener_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "PtEventListener.h"
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PtTerminalEvent;

//: The PtTerminalListener is used to register with and receive events from
//: PtTerminal objects.

class PtTerminalListener  : public PtEventListener
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   PT_CLASS_INFO_MEMBERS

/* ============================ CREATORS ================================== */

   PtTerminalListener(const char * pTerminalName = NULL, PtEventMask* pMask = NULL);
     //:Default constructor
     //!param: (in) pMask - Event mask defining events the listener is interested.  This must be a subset of the events that the listener supports.  The mask may be NULL where it is assumed that all events applicable to the derived listener are of interest.

   virtual
   ~PtTerminalListener();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

  void terminalEventTransmissionEnded(const PtTerminalEvent& rEvent);
     //: Method invoked on listener for event id =
     //: TERMINAL_EVENT_TRANSMISSION_ENDED
     //: indicating that the application will no longer receive Terminal events on the instance of the PtTerminalListener.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  PtStatus setTerminalName(const char* name);

/* ============================ ACCESSORS ================================= */

        PtStatus getTerminalName(char* name, int len);


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
        char * mpTerminalName;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   PtTerminalListener(const PtTerminalListener& rPtTerminalListener);
     //:Copy constructor

   PtTerminalListener& operator=(const PtTerminalListener& rhs);
     //:Assignment operator


};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtTerminalListener_h_

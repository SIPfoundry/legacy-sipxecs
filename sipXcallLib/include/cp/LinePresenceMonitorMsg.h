//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _LinePresenceMonitorMsg_h_
#define _LinePresenceMonitorMsg_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsSysLog.h>
#include <os/OsMsg.h>
#include <os/OsEvent.h>
#include <net/StateChangeNotifier.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class LinePresenceMonitor;

//
// LinePresenceMonitorMsg
//
class LinePresenceMonitorMsg : public OsMsg {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum eLinePresenceMonitorMsgSubTypes {
      SUBSCRIBE_DIALOG,
      UNSUBSCRIBE_DIALOG,
      SUBSCRIBE_PRESENCE,
      UNSUBSCRIBE_PRESENCE,
      SET_STATUS
   };

/* ============================ CREATORS ================================== */

   // Constructor for UPDATE_STATE messages
   LinePresenceMonitorMsg(eLinePresenceMonitorMsgSubTypes type,
                          //< Operation the message is to perform.
                          LinePresenceBase* line,
                          /**< The LinePresenceBase object that is the argument
                           *   for the operation.
                           */
                          OsEvent* event = NULL
                          /**< Event to signal when caller may proceed.
                           *   (Used by unsubscribe operations only, otherwise
                           *   NULL.)
                           */
                          );
   /// Constructor for SET_STATUS messages.
   LinePresenceMonitorMsg(const UtlString* contact,
                          //< The contact address whose status to update.
                          StateChangeNotifier::Status value
                          //< Status change event.
                          );

   // Copy constructor
   LinePresenceMonitorMsg(const LinePresenceMonitorMsg& rLinePresenceMonitorMsg);

   // Create a copy of this msg object (which may be of a derived type)
   OsMsg* createCopy(void) const;

   // Destructor
   virtual ~LinePresenceMonitorMsg();

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

   // Get the associated line
   LinePresenceBase* getLine(void) const { return mLine; }

   // Get the event
   OsEvent* getEvent(void) const { return mEvent; }

   // Get the contact
   const UtlString* getContact(void) const { return mContact; }

   // Get the state change code
   StateChangeNotifier::Status getStateChange(void) const { return mStateChange; }

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   LinePresenceBase* mLine;
   OsEvent* mEvent;
   const UtlString* mContact;
   StateChangeNotifier::Status mStateChange;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _LinePresenceMonitorMsg_h_

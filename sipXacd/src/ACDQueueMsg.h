//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ACDQueueMsg_h_
#define _ACDQueueMsg_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsSysLog.h>
#include <os/OsMsg.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class ACDCall;
class ACDAgent;

//
// ACDQueueMsg
//
class ACDQueueMsg : public OsMsg {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum eACDQueueMsgSubTypes {
      ADD_CALL,
      REMOVE_CALL,
      CALL_CONNECTED,
      UPDATE_ROUTE_STATE,
      MAX_WAIT_TIME,
      AGENT_AVAILABLE
   };

/* ============================ CREATORS ================================== */

   // Constructor for ADD_CALL, REMOVE_CALL, CALL_CONNECTED & MAX_WAIT_TIME messages
   ACDQueueMsg(eACDQueueMsgSubTypes type, ACDCall* pCallRef);

   // Constructor for UPDATE_ROUTE_STATE message
   ACDQueueMsg(eACDQueueMsgSubTypes type, ACDCall* pCallRef, ACDCallRouteState::eRouteState state);

   // Constructor for AGENT_AVAILABLE messages
   ACDQueueMsg(eACDQueueMsgSubTypes type, ACDAgent* pAgentRef);

   // Copy constructor
   ACDQueueMsg(const ACDQueueMsg& rACDQueueMsg);

   // Create a copy of this msg object (which may be of a derived type)
   OsMsg* createCopy(void) const;

   // Destructor
   virtual ~ACDQueueMsg();

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

   // Get the associated ACDCall reference
   ACDCall* getCallReference(void) const { return mpACDCallReference; }

   // Get the associated ACDCall route state
   ACDCallRouteState::eRouteState getCallRouteState(void) const { return mACDCallRouteState; }

   // Get the associated ACDAgent reference
   ACDAgent* getAgentReference(void) const { return mpACDAgentReference; }

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   ACDCall*         mpACDCallReference;
   ACDCallRouteState::eRouteState mACDCallRouteState;
   ACDAgent*        mpACDAgentReference;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _ACDQueueMsg_h_

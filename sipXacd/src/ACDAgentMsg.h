//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ACDAgentMsg_h_
#define _ACDAgentMsg_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsSysLog.h>
#include <os/OsMsg.h>
#include <cp/LinePresenceBase.h>

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
// ACDAgentMsg
//
class ACDAgentMsg : public OsMsg {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum eACDAgentMsgSubTypes {
      UPDATE_STATE,
      DROP_CALL
   };

/* ============================ CREATORS ================================== */

   // Constructor for UPDATE_STATE messages
   ACDAgentMsg(eACDAgentMsgSubTypes type, LinePresenceBase::ePresenceStateType stateType, bool state);

   // Constructor for DROP_CALL messages
   ACDAgentMsg(eACDAgentMsgSubTypes type, bool rna);

   // Copy constructor
   ACDAgentMsg(const ACDAgentMsg& rACDAgentMsg);

   // Create a copy of this msg object (which may be of a derived type)
   OsMsg* createCopy(void) const;

   // Destructor
   virtual ~ACDAgentMsg();

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

   // Get the associated PresenceStateType
   LinePresenceBase::ePresenceStateType getPresenceStateType(void) const { return mPresenceStateType; }

   // Get the associated PresenceState
   bool getPresenceState(void) const { return mPresenceState; }

   // Get the associated rna
   bool getRna(void) const { return mRna; }

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   LinePresenceBase::ePresenceStateType mPresenceStateType;
   bool             mPresenceState;
   bool             mRna;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _ACDAgentMsg_h_

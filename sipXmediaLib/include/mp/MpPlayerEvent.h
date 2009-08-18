//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MpPlayerEvent_h_
#define _MpPlayerEvent_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/StreamDefs.h"
#include "mp/MpPlayer.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpPlayer ;

//:Event encapsulating a change of player state.
class MpPlayerEvent
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   MpPlayerEvent() ;
     //:Default Constructor

   MpPlayerEvent(MpPlayer* pPlayer,
                 void* pUserData,
                 PlayerState state) ;
     //:Constructor requiring a player, user data, and state

   MpPlayerEvent(const MpPlayerEvent& rMpPlayerEvent);
     //:Copy constructor

   virtual
   ~MpPlayerEvent();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   MpPlayerEvent& operator=(const MpPlayerEvent& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */
   MpPlayer* getPlayer() const ;
     //: Gets the player responsible for firing this event

   void* getUserData() const ;
     //: Gets the user supplied data supplied when adding the listener

   PlayerState getState() const ;
     //: Gets the state of player snapshotted when the event was fired.

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   MpPlayer* mpPlayer ;    // Reference to the Stream Player
   void* mpUserData ;      // Reference to the user data
   PlayerState mState ;    // State snapshot when fired
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpPlayerEvent_h_

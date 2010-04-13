//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpPlayerEvent.h"
#include "mp/MpPlayer.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor requiring a player, user data, and state
MpPlayerEvent::MpPlayerEvent()
{
   mpPlayer = NULL ;
   mpUserData = NULL ;
   mState = PlayerUnrealized ;
}


// Constructor
MpPlayerEvent::MpPlayerEvent(MpPlayer* pPlayer,
                             void* pUserData,
                             PlayerState state)
{
   mpPlayer = pPlayer ;
   mpUserData = pUserData ;
   mState = state ;
}

// Copy constructor
MpPlayerEvent::MpPlayerEvent(const MpPlayerEvent& rMpPlayerEvent)
{
   mpPlayer = rMpPlayerEvent.mpPlayer ;
   mpUserData = rMpPlayerEvent.mpUserData ;
   mState = rMpPlayerEvent.mState ;
}

// Destructor
MpPlayerEvent::~MpPlayerEvent()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
MpPlayerEvent&
MpPlayerEvent::operator=(const MpPlayerEvent& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   mpPlayer = rhs.mpPlayer ;
   mpUserData = rhs.mpUserData ;
   mState = rhs.mState ;

   return *this;
}

/* ============================ ACCESSORS ================================= */

// Gets the player responsible for firing this event
MpPlayer* MpPlayerEvent::getPlayer() const
{
   return mpPlayer ;
}


// Gets the user supplied data supplied when adding the listener
void* MpPlayerEvent::getUserData() const
{
   return mpUserData ;
}


// Gets the state of player snapshotted when the event was fired.
PlayerState MpPlayerEvent::getState() const
{
   return mState ;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

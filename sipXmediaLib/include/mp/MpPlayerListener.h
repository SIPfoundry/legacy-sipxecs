//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MpPlayerListener_h_
#define _MpPlayerListener_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpPlayerEvent.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpPlayer;


//:Listener interface for the MpPlayer object.
class MpPlayerListener
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   virtual ~MpPlayerListener()
   {
   }

/* ============================ MANIPULATORS ============================== */

   virtual void playerRealized(MpPlayerEvent& event) = 0;
     //: The player has been realized

   virtual void playerPrefetched(MpPlayerEvent& event) = 0;
     //: The player's data source has been prefetched

   virtual void playerPlaying(MpPlayerEvent& event) = 0;
     //: The player has begun playing.

   virtual void playerPaused(MpPlayerEvent& event) = 0;
     //: The player has been paused

   virtual void playerStopped(MpPlayerEvent& event) = 0;
     //: The player has been stopped

   virtual void playerFailed(MpPlayerEvent& event) = 0;
     //: The player has failed

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpPlayerListener_h_

//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MpQueuePlayerListener_h_
#define _MpQueuePlayerListener_h_

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
class MpQueuePlayerListener
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   virtual ~MpQueuePlayerListener()
   {
   }

/* ============================ MANIPULATORS ============================== */

   virtual void queuePlayerStarted() = 0 ;
     //:Called when a queue player has started playing its playlist.

   virtual void queuePlayerStopped() = 0 ;
     //:Called when a queue player has stopped playing its playlist.
     // This event will occur after the play list completes or when aborted.

   virtual void queuePlayerAdvanced() = 0 ;
     //:Called when the queue player advances to a new playlist element.
     // This method is called before the new playlist element is played and
     // may occur multiple times before a queuePlayerStopped.

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpQueuePlayerListener_h_

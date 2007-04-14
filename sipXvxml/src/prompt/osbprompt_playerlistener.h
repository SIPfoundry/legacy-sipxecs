//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _osbprompt_playerlistener_h_
#define _osbprompt_playerlistener_h_

// SYSTEM INCLUDES
#include "os/OsBSem.h"
#include "os/OsDefs.h"
#include "os/OsTime.h"
#include "os/OsQueuedEvent.h"

// APPLICATION INCLUDES
#include "mp/MpPlayerListener.h"

// DEFINES
#define MAX_NUM_LISTENERS 10

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

// Class used to wait for state changes
class OSBPlayerListener : public MpPlayerListener
{
  public:
   OSBPlayerListener(int iTimeoutSecs = 5);

   virtual
      ~OSBPlayerListener();
   //:Destructor

   virtual void clearState();

   virtual UtlBoolean waitForState(PlayerState state);

   virtual UtlBoolean isState(PlayerState state);

   virtual void addListeningEvent(OsQueuedEvent* promptEvent);

   virtual void removeListeningEvent(OsQueuedEvent* promptEvent);

   virtual void playerRealized(MpPlayerEvent& event) {}
     //: The player has been realized

   virtual void playerPrefetched(MpPlayerEvent& event) {}
     //: The player's data source has been prefetched

   virtual void playerPlaying(MpPlayerEvent& event);
   //:Called when a player has started playing its playlist.

   virtual void playerPaused(MpPlayerEvent& event) {}
     //: The player has been paused

   virtual void playerStopped(MpPlayerEvent& event);
   //:Called when a player has stopped playing its playlist.
   // This event will occur after the play list completes or when aborted.

   virtual void playerFailed(MpPlayerEvent& event) {}
     //: The player has failed

   virtual int addRef();
   //:Called when the rec module gets a handle of the listener.

   virtual int release();
   //:Called when the rec and prompt modules' EndSession.
   // when return value (mRef) is <= 0, caller should delete this listener.


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:
   int mEventQLen;
   OsQueuedEvent*  mPromptEvents[MAX_NUM_LISTENERS];

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:
   int       miTimeoutSec ;
   UtlBoolean mStates[16] ;
   OsBSem    mSemStateChange;
   OsBSem    mSemGuard ;

   int       mShutdown;
   int       mRef;

#ifdef TEST
   static bool sIsTested;
   //:Set to true after the tests for this class have been executed once

   void test();
   //:Verify assertions for this class

   // Test helper functions
   void testCreators();
   void testManipulators();
   void testAccessors();
   void testInquiry();

#endif //TEST
};

/* ============================ INLINE METHODS ============================ */

#endif  // _osbprompt_playerlistener_h_

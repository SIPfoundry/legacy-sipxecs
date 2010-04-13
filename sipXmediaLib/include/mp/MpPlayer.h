//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MpPlayer_h_
#define _MpPlayer_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/StreamDefs.h"
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsMutex.h"

// DEFINES
#define MAX_PLAYER_LISTENERS  16  // Max number of player listeners


// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

//:Definition of states used by audio players
typedef enum
{
   PlayerUnrealized,
   PlayerRealized,
   PlayerPrefetched,
   PlayerPlaying,
   PlayerPaused,
   PlayerStopped,
   PlayerAborted,
   PlayerFailed,
   PlayerDestroyed,
} PlayerState ;
//!enumcode PlayerUnrealized - Data is unrealized (uninitialized) and no
//          resources have been allocated.
//!enumcode PlayerRealized - Data has been realized and resource allocated
//!enumcode PlayerPrefetched - Data has been prefetched.  Prefetch may fetch
//          the entire data source or just enough to ensure smooth playback.
//!enumcode PlayerPlaying - The player has begun playing media.
//!enumcode PlayerPaused - The player has been paused.
//!enumcode PlayerStopped - The player has stopped playing media.
//!enumcode PlayerFailed - The player has failed.
//!enumcode PlayerDestroyed - The player has been destroyed
//!enumcode PlayerAborted - Indicates that the player was stop explicitly by
//          a call to stop() (as opposed to normal playing).

// FORWARD DECLARATIONS
class MpPlayerListener ;


//:Defines a stream player control that allows users to realize, start, stop,
//:and pause an audio source.
//
// <pre>
//                              +-----------------------------------+
//                             \ /                                  |
//    ------------        ------------         ---------        ---------
//   | Unrealized | ---> | Prefetched | <---> | Playing | ---> | Stopped |
//    ------------        ------------         ---------        ---------
//                                                / \                         .
//                                                 |
//                                                \ /
//    --------                                 --------
//   | Failed | <--*                          | Paused |
//    --------                                 --------
//
//    -----------
//   | Destroyed | <--*
//    -----------
//
//    ---------
//   | Aborted |  <--*
//    ---------

// </pre>
class MpPlayer
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum playerType  // Type of players
   {
      STREAM_PLAYER,
      STREAM_PLAYLIST_PLAYER,
      STREAM_QUEUE_PLAYER
   };

/* ============================ CREATORS ================================== */
   MpPlayer();
     //:Default Constructor

   virtual ~MpPlayer();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus realize(UtlBoolean bBlock = TRUE) = 0;
     //: Realizes the player by initiating a connection to the target,
     //: allocates buffers, etc.
     //
     //!param bBlock - TRUE if the method should block until completion,
     //       otherwise FALSE.

   virtual OsStatus prefetch(UtlBoolean bBlock = TRUE) = 0 ;
     //: Prefetch enough of the data source to ensure a smooth playback.
     //
     //!param bBlock - TRUE if the method should block until completion,
     //       otherwise FALSE.

   virtual OsStatus play(UtlBoolean bBlock = TRUE) = 0 ;
     //: Plays the media stream.
     //
     //!param bBlock - TRUE if the method should block until completion,
     //       otherwise FALSE.

   virtual OsStatus pause() = 0 ;
     //: Pauses the media stream temporarily.

   virtual OsStatus stop() = 0 ;
     //: Stops playing the media stream and resources used for buffering
     //: and streaming.

   virtual OsStatus destroy() = 0 ;
     //: Marks the player as destroy and frees all allocated resources
     //  in media processing.

   OsStatus addListener(MpPlayerListener* pListener, void* pUserData = NULL) ;
     //:Adds a player listener to receive notifications when this player
     //:changes state

   OsStatus removeListener(MpPlayerListener* pListener) ;
     //:Removes a previously added player listener.  This listener will
     // cease to receive state change notifications.


/* ============================ ACCESSORS ================================= */

   virtual OsStatus getState(PlayerState& state) = 0 ;
     //: Gets the player state

/* ============================ INQUIRY =================================== */

/* ============================ TESTING =================================== */

#ifdef MP_STREAM_DEBUG /* [ */
static const char* getEventString(PlayerState event);
#endif /* MP_STREAM_DEBUG ] */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   void fireEvent(PlayerState state);
     //:Fires an event to all registered listeners

   UtlBoolean isValidStateChange(PlayerState oldState, PlayerState newState) ;
     //:Is the transition from oldState to newState valid?

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   struct PlayerListenerDb   // Data structure used to maintain listeners
   {
      UtlBoolean inUse ;             // Is the entry in use?
      MpPlayerListener* pListener ; // Reference to listener
      void* pUserData;              // User data specified when added
   } ;


   PlayerListenerDb mListenerDb[MAX_PLAYER_LISTENERS] ;     // DB of listeners
   OsMutex          mListenerLock ;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpPlayer_h_

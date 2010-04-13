//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MpStreamQueuePlayer_h_
#define _MpStreamQueuePlayer_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpPlayer.h"
#include "mp/MpPlayerListener.h"
#include "mp/StreamDefs.h"
#include "net/Url.h"
#include "os/OsBSem.h"
#include "os/OsDefs.h"
#include "os/OsMsgQ.h"
#include "os/OsServerTask.h"
#include "os/OsStatus.h"
#include "os/OsRWMutex.h"
#include "os/OsQueuedEvent.h"

// DEFINES
#define DEFAULT_QUEUE_LENGTH      64    // Default length of queue
#define EXPAND_QUEUE_LENGTH       16    // Queue grows by this much

// Player impotence- If define, the queue player doesn't actually do
// anything, however, will still fire off events, etc.
#undef  PLAYER_STUBS

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlString ;
class MpStreamPlayer ;
class MpQueuePlayerListener ;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class MpStreamQueuePlayer : public OsServerTask, protected MpPlayerListener
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum SourceType
   {
      SourceUrl,
      SourceBuffer,
   } ;

/* ============================ CREATORS ================================== */

   MpStreamQueuePlayer(OsMsgQ* pMsgQ, const char* pTarget = NULL);
     //:Constructor accepting a msgQ

   virtual
   ~MpStreamQueuePlayer();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus add(Url& url, int flags) ;
     //:Queues a URL for playing
     //
     //!param url - Url identifing the source data stream
     //!param flags - Playing flags (see StreamDefs.h)


   virtual OsStatus add(UtlString* pBuffer, int flags) ;
     //:Queues a UtlString for playing
     //
     //!param pBuffer - Net Buffer containing buffered audio data.  The
     //       MpStreamPlayer resource will delete the pBuffer upon destruction
     //       of itself.
     //!param flags - Playing flags (see StreamDefs.h)

   virtual OsStatus play() ;
     //:Begins playing any queued streams

   virtual OsStatus reset() ;
     //: Resets the queue player state by stopping and removing all
     //: playing entries.

   virtual OsStatus destroy() ;
     //: Marks the player as destroy and frees all allocated resources
     //  in media processing.

   virtual OsStatus clear() ;
     //: Clears any queued entries that have not yet been scheduled for play.
     //  These entries include streams added after invoking "play()".

   virtual OsStatus wait(const OsTime& rTimeout = OsTime::OS_INFINITY) ;
     //: Wait until all play list items are finished playing

   OsStatus addListener(MpQueuePlayerListener* pListener) ;
     //:Adds a player listener to receive notifications when this player
     //:changes state

   OsStatus removeListener(MpQueuePlayerListener* pListener) ;
     //:Removes a previously added player listener.  This listener will
     // cease to receive state change notifications.


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

   UtlBoolean isPlaying() ;
     //:Is the Queue player playing (or about to play)


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   MpStreamQueuePlayer(const MpStreamQueuePlayer& rMpStreamQueuePlayer);
     //:Copy constructor

   MpStreamQueuePlayer& operator=(const MpStreamQueuePlayer& rhs);
     //:Assignment operator

   virtual UtlBoolean handleMessage(OsMsg& rMsg) ;
     //:Handles an incoming message
     // If the message is not one that the object is prepared to process,
     // the handleMessage() method in the derived class should return FALSE
     // which will cause the OsMessageTask::handleMessage() method to be
     // invoked on the message.

   void handleReset() ;
   void handlePlayNext() ;
   void handleDequeue() ;
   void handleRemoveFailed() ;

   UtlBoolean isPlayingStream(MpPlayer* pPlayer) ;
     //:Is the specified player the active/playing stream?

   void setFailedPlayer(MpPlayer* pPlayer) ;
     //:Designates the the player as failed

   virtual void playerRealized(MpPlayerEvent& event) ;
     //: The player has been realized

   virtual void playerPrefetched(MpPlayerEvent& event) ;
     //: The player's data source has been prefetched

   virtual void playerPlaying(MpPlayerEvent& event) ;
     //: The player has begun playing.

   virtual void playerPaused(MpPlayerEvent& event) ;
     //: The player has been paused

   virtual void playerStopped(MpPlayerEvent& event) ;
     //: The player has been stopped

   virtual void playerFailed(MpPlayerEvent& event) ;
     //: The player has failed

   void fireQueuePlayerStarted() ;
   void fireQueuePlayerStopped() ;
   void fireQueuePlayerAdvanced() ;



/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   enum
   {
      EVENT_DEQUEUE,
      EVENT_PLAY_NEXT,
      EVENT_REMOVE_FAILED,
      EVENT_RESET,
   } ;

   struct PlaylistQueue             // Definition for a playlist entry
   {
      MpStreamPlayer* pPlayer ;
      UtlBoolean       bFailed ;
   } ;

   OsMsgQ*  mpMsgQ ;                // Queue to deliever MpStreamMsg commands
   UtlString mTarget ;               // Target Id (CallId)
   OsBSem   mSemQueueChange;        // Guard for queue changes
   OsBSem   mSemWaitSynch;          // Gives some for block on for waiters

   OsQueuedEvent* mpQueueEvent;     // Used for dequeuing

   struct PlaylistQueue* mToPlayQueue;   // db of entries
   int    mToPlayQueueLength ;      // Physical size of queue
   int    mNumToPlayElements ;      // Current number of queued elements
   struct PlaylistQueue* mPlayingQueue;   // db of entries
   int    mPlayingQueueLength ;
   int    mNumPlayingElements ;      // Current number of queued elements
   UtlBoolean mbFatalError ;          // Something REALLY bad has happened (can't create thread)

   int expandQueue(struct PlaylistQueue*& queue, int currentLength, int desiredLength) ;
   void swapQueues(struct PlaylistQueue*& queue1, int& queueLength1,
                   struct PlaylistQueue*& queue2, int& queueLength2) ;

   struct PlayerListenerDb   // Data structure used to maintain listeners
   {
      UtlBoolean inUse ;                  // Is the entry in use?
      MpQueuePlayerListener* pListener ; // Reference to listener
   } ;

   PlayerListenerDb mListenerDb[MAX_PLAYER_LISTENERS] ;     // DB of listeners
   OsRWMutex 	    mListenerMutex;   // Used to make it thread-safe when adding/removing and using listeners
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpStreamQueuePlayer_h_

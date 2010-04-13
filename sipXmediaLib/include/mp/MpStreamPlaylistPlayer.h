//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MpStreamPlaylistPlayer_h_
#define _MpStreamPlaylistPlayer_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpPlayer.h"
#include "mp/StreamDefs.h"
#include "net/Url.h"
#include "os/OsBSem.h"
#include "os/OsEvent.h"
#include "os/OsDefs.h"
#include "os/OsServerTask.h"
#include "os/OsStatus.h"
#include "os/OsQueuedEvent.h"
#include "utl/UtlContainableAtomic.h"
#include "utl/UtlSList.h"
#include "utl/UtlSListIterator.h"

// DEFINES
#define REALIZE_TIMEOUT          15    // Timeout after 15 seconds
#define PREFETCH_TIMEOUT         30    // Timeout after 30 seconds
#define PLAY_TIMEOUT            180    // Timeout after 180 seconds
#define REWIND_TIMEOUT           15    // Timeout after 15 seconds
#define STOP_TIMEOUT             15    // Timeout after 15 seconds
#define DESTROY_TIMEOUT          30    // Timeout after 30 seconds

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlString;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class MpStreamPlaylistPlayer : public OsServerTask, public MpPlayer
{
// FORWARD DECLARATIONS
   class PlayListEntry;

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum SourceType
   {
      SourceUrl,
      SourceBuffer,
   } ;

/* ============================ CREATORS ================================== */

   MpStreamPlaylistPlayer(OsMsgQ* pMsgQ, const char* pTarget = NULL);
     //:Constructor accepting a flow graph


   virtual
   ~MpStreamPlaylistPlayer();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus add(Url& url, int flags) ;
     //:Adds a url to the playlist
     //
     //!param url - Url identifing the source data stream
     //!param flags - Playing flags (see StreamDefs.h)


   virtual OsStatus add(UtlString* pBuffer, int flags) ;
     //:Adds a buffer to the playlist
     //
     //!param pBuffer - Net Buffer containing buffered audio data.  The
     //       MpStreamPlayer resource will delete the pBuffer upon destruction
     //       of itself.
     //!param flags - Playing flags (see StreamDefs.h)


   virtual OsStatus realize(UtlBoolean bBlock = TRUE) ;
     //:Realizes the player by initiating a connection to the target,
     //:allocates buffers, etc.
     //
     //!param bBlock - TRUE if the method should block until completion,
     //       otherwise FALSE.

   virtual OsStatus prefetch(UtlBoolean bBlock = TRUE) ;
     //:Prefetch enough of the data source to ensure a smooth playback.
     //
     //!param bBlock - TRUE if the method should block until completion,
     //       otherwise FALSE.

   virtual OsStatus play(UtlBoolean bBlock = TRUE);
     //:Plays the media stream.  This will play all play lists from start
     //:to finish.
     //
     //!param bBlock - TRUE if the method should block until completion,
     //       otherwise FALSE.

   virtual OsStatus wait(const OsTime& rTimeout = OsTime::OS_INFINITY);
     //:Waits for the media stream(s) to finish playing.  This will block
     // the caller until all play lists have finished or rTimeout is reached.
     //
     //!param rTimeout - Optional timeout.  If not specified, wait forever.

   virtual OsStatus rewind(UtlBoolean bBlock = TRUE);
     //: Rewinds a previously played media stream.  In some cases this
     //  may result in a re-connect/refetch.
     //
     //!param bBlock - TRUE if the method should block until completion,
     //       otherwise FALSE.

   virtual OsStatus reset() ;
     //: Resets the playlist player state by stopping and removing all
     //: entries.

   virtual OsStatus stop() ;
     //:Stops play the media stream and resources used for buffering
     //:and streaming.

   virtual OsStatus destroy() ;
     //: Marks the player as destroy and frees all allocated resources
     //  in media processing.

   virtual OsStatus pause() ;
     //: Pauses the media stream temporarily.


/* ============================ ACCESSORS ================================= */

   virtual OsStatus getCount(int& count) const ;
     //:Gets the number of play list entries

   virtual OsStatus getSourceType(int index, int& type) const ;
     //:Gets the source type for playlist entry 'index'.

   virtual OsStatus getSourceUrl(int index, Url url) const ;
     //:Gets the source url for playlist entry 'index'.

   virtual OsStatus getSourceBuffer(int index, UtlString*& netBuffer) const ;
     //:Gets the source buffer for playlist entry 'index'.

   virtual OsStatus getSourceState(int index, PlayerState& state) const ;
     //:Gets the state for the playlist entry 'index'.

   virtual OsStatus getCurrentIndex(int& iIndex) const ;
     //: Gets the current playing index if playing or the next index to play
     //: if playNext() was invoked.

   virtual OsStatus getState(PlayerState& state)  ;
     //: Gets the aggregate playerlist player state

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   MpStreamPlaylistPlayer(const MpStreamPlaylistPlayer& rMpStreamPlaylistPlayer);
     //:Copy constructor

   MpStreamPlaylistPlayer& operator=(const MpStreamPlaylistPlayer& rhs);
     //:Assignment operator

   virtual OsStatus first() ;
     //:Selects the first playlist entry as the next index to play.
     // If an entry was playing, it will be stopped

   virtual OsStatus last() ;
     //:Selects the last playlist entry as the next index to play.
     // If an entry was playing, it will be stopped

   virtual OsStatus playNext(UtlBoolean bBlock = TRUE) ;
     //:Plays the next playlist entry without wrapping.

   virtual OsStatus playPrevious(UtlBoolean bBlock = TRUE) ;
     //:Plays the previous playlist entry without wrapping.

   void setEntryState(PlayListEntry *e, PlayerState iState) ;
     //:Sets the state for a specific entry.

   OsStatus playEntry(int iEntry, UtlBoolean bBlock = TRUE);
     //:Starts playing a specific entry

   OsStatus rewindEntry(PlayListEntry *e, UtlBoolean bBlock = TRUE);
     //:Rewinds a specific entry

   OsStatus stopEntry(PlayListEntry *e, UtlBoolean bBlock = TRUE);
     //:Stops playing a specific entry

   OsStatus pauseEntry(PlayListEntry *e);
     //:Pauses a specific entry

   OsStatus destroyEntry(PlayListEntry *e, UtlBoolean bBlockAndClean = TRUE);
     //:Destroys a specific entry

   virtual UtlBoolean handleMessage(OsMsg& rMsg) ;
     //:Handle messages directed to this server task.

   void handleRealizedState(PlayerState oldState, PlayerState newState);
     //:Handles processing for the realized state

   void handlePrefetchedState(PlayerState oldState, PlayerState newState);
     //:Handles processing for the prefetched state

   void handlePlayingState(PlayerState oldState, PlayerState newState);
     //:Handles processing for the playing state

   void handlePausedState(PlayerState oldState, PlayerState newState);
     //:Handles processing for the paused state

   void handleStoppedState(PlayerState oldState, PlayerState newState);
     //:Handles processing for the stopped state

   void handleFailedState(PlayerState oldState, PlayerState newState);
     //:Handles processing for the failed state

   const char* getFeederEventString(int iEvent);
     // :Handles converting Feeder events into human readable form

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   // Definition for a playlist entry
   class PlayListEntry : public UtlContainableAtomic
   {
   public:
      int            sourceType ;   // Source type (url or buffer)
      Url            url ;          // url if source type url
      UtlString*     pBuffer ;      // buffer if source type buffer
      StreamHandle   handle ;       // handle of the feeder
      PlayerState    state ;        // state of the entry
      int            flags ;        // flags for the entry
      OsQueuedEvent* pQueuedEvent ; // queued event for notifications
      int            index;         // Entry index (0 based)

      // Constructor
      PlayListEntry() {
         sourceType = 0;
         pBuffer = NULL;
         handle = NULL;
         state = PlayerUnrealized;
         flags = 0;
         pQueuedEvent = NULL;
      }

      // Needed for UtlContainable
      virtual UtlContainableType getContainableType() const {
         return "PlaylistEntry";
      }
   }  ;


   int mCurrentElement ;            // next item to  play
   int mPlayingElement ;            // current playing item

   OsQueuedEvent* mpQueueEvent;     // event for notifications
   OsBSem mSemStateChange;          // used to block for state changes
   OsMsgQ* mpMsgQ;                  // MsgQ to send commands
   UtlString mTarget;               // target used for MsgQ receive to help dispatch
   OsEvent mWaitEvent;              // used to block until player completes
   OsTime mRealizeTimeout;          // Timeout for Realize operation
   OsTime mPrefetchTimeout;         // Timeout for Prefetch operation
   OsTime mPlayTimeout;             // Timeout for Play operation
   OsTime mRewindTimeout;           // Timeout for Rewind operation
   OsTime mStopTimeout;             // Timeout for Stop operation
   OsTime mDestroyTimeout;          // Timeout for Destroy operation

   UtlBoolean mbAutoAdvance;        // used to play playlist lists

   UtlSList* mPlayListDb;           // db of entries
   PlayerState mAggregateState ;    // Aggregate state of the player
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpStreamPlaylistPlayer_h_

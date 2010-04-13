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
#include "mp/MpStreamPlaylistPlayer.h"
#include "mp/MpStreamMsg.h"
#include "os/OsEventMsg.h"
#include "os/OsEvent.h"
#include "os/OsQueuedEvent.h"
#include "os/OsMsgQ.h"
#include "os/OsSysLog.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Contructor accepting a flow graph
MpStreamPlaylistPlayer::MpStreamPlaylistPlayer(OsMsgQ* pMsgQ, const char* pTarget)
   : OsServerTask("PlaylistPlay-%d")
   , MpPlayer()
   , mSemStateChange(OsBSem::Q_PRIORITY, OsBSem::EMPTY)
   , mWaitEvent(0)
   , mRealizeTimeout(REALIZE_TIMEOUT, 0)
   , mPrefetchTimeout(PREFETCH_TIMEOUT, 0)
   , mPlayTimeout(PLAY_TIMEOUT, 0)
   , mRewindTimeout(REWIND_TIMEOUT, 0)
   , mStopTimeout(STOP_TIMEOUT, 0)
   , mDestroyTimeout(DESTROY_TIMEOUT, 0)

{
   mpMsgQ = pMsgQ;
   if (pTarget != NULL)
      mTarget = pTarget;
   mpQueueEvent = NULL;

   mPlayListDb = new UtlSList();
   mCurrentElement = 0;
   mPlayingElement = -1;
   mbAutoAdvance = FALSE;
   mAggregateState = PlayerUnrealized;

   start();

   // Wait for the stream player to startup.
   while (!isStarted())
   {
      OsTask::yield();
   }
}


// Destructor
MpStreamPlaylistPlayer::~MpStreamPlaylistPlayer()
{
   reset();
   delete mPlayListDb ;

   if (mpQueueEvent != NULL)
   {
      delete mpQueueEvent;
   }
}

/* ============================ MANIPULATORS ============================== */

// Adds a url to the playlist
OsStatus MpStreamPlaylistPlayer::add(Url& url, int flags)
{
   PlayListEntry* e = new PlayListEntry();
   int index = mPlayListDb->entries();

   e->sourceType = SourceUrl;
   e->url = url;
   e->flags = flags;
   e->pQueuedEvent = new OsQueuedEvent(*getMessageQueue(), (void*)index);
   e->index = index ;

   mPlayListDb->append(e);

   return OS_SUCCESS;
}


// Adds a buffer to the playlist
OsStatus MpStreamPlaylistPlayer::add(UtlString* pBuffer, int flags)
{
   PlayListEntry* e = new PlayListEntry();
   int index = mPlayListDb->entries();

   e->sourceType = SourceBuffer;
   e->pBuffer = pBuffer;
   e->flags = flags;
   e->pQueuedEvent = new OsQueuedEvent(*getMessageQueue(), (void*)index);
   e->index = index ;

   mPlayListDb->append(e);

   return OS_SUCCESS;
}


// Realizes the player by initiating a connection to the target, allocates
// buffers, etc.
OsStatus MpStreamPlaylistPlayer::realize(UtlBoolean bBlock)
{
   OsStatus status = OS_FAILED;
   PlayListEntry* e;

   if (mAggregateState == PlayerFailed)
   {
      OsSysLog::add(FAC_MP, PRI_ERR, "MpStreamPlaylistPlayer::realize failure, mAggregateState == PlayerFailed");
      return status;
   }

   // Start prefetching all of the elements
   UtlSListIterator playListDbIterator(*mPlayListDb) ;
   while((e = (PlayListEntry*)playListDbIterator()))
   {
//      OsSysLog::add(FAC_MP, PRI_DEBUG, "MpStreamPlaylistPlayer::realize entry[%d] state %d", e->index, e->state);

      if (e->state == PlayerUnrealized)
      {
         OsEvent eventHandle;

         // Realize the stream
         if (e->sourceType == SourceUrl)
         {
            MpStreamMsg msg(MpStreamMsg::STREAM_REALIZE_URL,
                            mTarget,
                            NULL,
                            &eventHandle,
                            e->pQueuedEvent,
                            e->flags,
                            (intptr_t) new Url(e->url));
            status = mpMsgQ->send(msg);
            if (status != OS_SUCCESS)
            {
               setEntryState(e, PlayerFailed);
               e->handle = NULL;
               OsSysLog::add(FAC_MP, PRI_ERR, "MpStreamPlaylistPlayer::realize failed on send of MpStreamMsg::STREAM_REALIZE_URL message");
            }
         }
         else if (e->sourceType == SourceBuffer)
         {
            MpStreamMsg msg(MpStreamMsg::STREAM_REALIZE_BUFFER,
                            mTarget,
                            NULL,
                            &eventHandle,
                            e->pQueuedEvent,
                            e->flags,
                            (intptr_t) e->pBuffer);
            status = mpMsgQ->send(msg);
            if (status != OS_SUCCESS)
            {
               setEntryState(e, PlayerFailed);
               e->handle = NULL;
               delete e->pBuffer;
               e->pBuffer = NULL;
               OsSysLog::add(FAC_MP, PRI_ERR, "MpStreamPlaylistPlayer::realize failed on send of MpStreamMsg::STREAM_REALIZE_BUFFER message");
            }
         }

         if (status == OS_SUCCESS)
         {
            // Wait for a response
            intptr_t eventData;
            status = eventHandle.wait(mRealizeTimeout);
            if (status == OS_SUCCESS)
               status = eventHandle.getEventData(eventData);
            if (status == OS_SUCCESS)
            {
               e->handle = (StreamHandle) eventData;
            }
            else
            {
               setEntryState(e, PlayerFailed);
               e->handle = NULL;
               if (e->sourceType == SourceBuffer)
               {
                  delete e->pBuffer;
                  e->pBuffer = NULL;
               }
               OsSysLog::add(FAC_MP, PRI_ERR, "MpStreamPlaylistPlayer::realize STREAM_REALIZE_ request failed");
            }
         }
      }
   }

   // Block if requested
   playListDbIterator.reset();
   if ((status == OS_SUCCESS) && bBlock)
   {
      while((e = (PlayListEntry*)playListDbIterator()) != NULL &&
               (mAggregateState != PlayerFailed))
      {
         while (e->state == PlayerUnrealized)
         {
            status = mSemStateChange.acquire(mRealizeTimeout);
            if (status == OS_WAIT_TIMEOUT)
            {
               setEntryState(e, PlayerFailed);
               OsSysLog::add(FAC_MP, PRI_ERR, "MpStreamPlaylistPlayer::realize timed out waiting for Realize to complete");
               break;
            }
         }
      }
   }

   return status;
}

// Prefetch enough of the data source to ensure a smooth playback.
OsStatus MpStreamPlaylistPlayer::prefetch(UtlBoolean bBlock /*= TRUE*/)
{
   OsStatus status = OS_FAILED;
   PlayListEntry* e;

   // Start prefetching all of the elements
   UtlSListIterator playListDbIterator(*mPlayListDb) ;
   while((mAggregateState != PlayerFailed) &&
         (e = (PlayListEntry*)playListDbIterator()))
   {
      if (e->state == PlayerRealized)
      {

         MpStreamMsg msg(MpStreamMsg::STREAM_PREFETCH, mTarget, e->handle);
         status = mpMsgQ->send(msg);
         if (status != OS_SUCCESS)
         {
            setEntryState(e, PlayerFailed);
            OsSysLog::add(FAC_MP, PRI_ERR, "MpStreamPlaylistPlayer::prefetch failed on send of MpStreamMsg::STREAM_PREFETCH message");
         }
      }
   }

   // Block if requested
   if ((status == OS_SUCCESS) && bBlock)
   {
      playListDbIterator.reset();
      while((mAggregateState != PlayerFailed) &&
            (e = (PlayListEntry*)playListDbIterator()))
      {
         while (e->state == PlayerRealized)
         {
            status = mSemStateChange.acquire(mPrefetchTimeout);
            if (status == OS_WAIT_TIMEOUT)
            {
               setEntryState(e, PlayerFailed);
               OsSysLog::add(FAC_MP, PRI_ERR, "MpStreamPlaylistPlayer::prefetch timed out waiting for Prefetch to complete");
               break ;
            }
         }
      }
   }

   return status;
}



// Plays the media stream.  This will play all play lists from start to
// finish.
OsStatus MpStreamPlaylistPlayer::play(UtlBoolean bBlock /*= TRUE*/)
{
   OsStatus status = OS_FAILED;

   // If the player is in a failed or unrealized state, abort.
   if ((mAggregateState == PlayerFailed) || (mAggregateState == PlayerUnrealized))
   {
      OsSysLog::add(FAC_MP, PRI_ERR, "MpStreamPlaylistPlayer::play request failed due to player being in invalid state");
      return OS_INVALID_STATE;
   }

   if (bBlock)
   {
      first();
      do
      {
         mbAutoAdvance = FALSE;
         status = playNext();
      }
      while ((status == OS_SUCCESS) && (mCurrentElement < (int)mPlayListDb->entries()));
   }
   else
   {
      mbAutoAdvance = TRUE;
      if ((mAggregateState == PlayerPaused) && (mPlayingElement != -1))
      {
         status = playEntry(mPlayingElement, bBlock);
      }
      else
         status = playNext(FALSE);
   }

   return status;
}


// Wait until all play list items are finished playing
OsStatus MpStreamPlaylistPlayer::wait(const OsTime& rTimeout)
{
   OsStatus status = OS_SUCCESS;

   while (  (mAggregateState == PlayerRealized)
         || (mAggregateState == PlayerPrefetched)
         || (mAggregateState == PlayerPlaying)
         || (mAggregateState == PlayerPaused))
   {
      status = mWaitEvent.wait(rTimeout);
      if (status != OS_SUCCESS)
         break;
   }

   return status;
}


// Plays the media stream.  This will play all play lists from start to
// finish.
OsStatus MpStreamPlaylistPlayer::rewind(UtlBoolean bBlock /*= TRUE*/)
{
   OsStatus status = OS_SUCCESS;
   PlayListEntry* e;

   stop();
   mAggregateState = PlayerUnrealized;

   // rewind all of the elements
   UtlSListIterator playListDbIterator(*mPlayListDb) ;
   while((e = (PlayListEntry*)playListDbIterator()))
   {
      rewindEntry(e, bBlock);
   }

   mCurrentElement = 0;
   mPlayingElement = -1;

   return status;
}


// Resets the playlist player state by stopping and removing all entries.
OsStatus MpStreamPlaylistPlayer::reset()
{
   OsStatus status = OS_SUCCESS;
   PlayListEntry* e;

   OsSysLog::add(FAC_MP, PRI_DEBUG, "MpStreamPlaylistPlayer::reset");
   UtlSListIterator playListDbIterator(*mPlayListDb) ;
   while((e = (PlayListEntry*)playListDbIterator()))
   {
      destroyEntry(e);
   }

   // Remove PlayListEntries from the db.  Cannot do it until all the
   // entries are destroyed, as they are position dependent based on index,
   // and if we remove entry 0 before entry 1 is done, when the event for
   // entry 1 arrives everything will have moved down one and be messed up.
            // Remove the PlayListEntry from the Db
   playListDbIterator.reset();
   while((e = (PlayListEntry*)playListDbIterator()))
   {
      mPlayListDb->removeReference(e) ;
   }

   mCurrentElement = 0;
   mPlayingElement = -1;
   mbAutoAdvance = FALSE;
   mAggregateState = PlayerUnrealized;

   // Wake up anyone waiting on this class
   mWaitEvent.signal(0);

   // And set it up for the next time
   mWaitEvent.reset() ;

   return status;
}



// Stops play the media stream and resources used for buffering and streaming.
OsStatus MpStreamPlaylistPlayer::stop()
{
   OsStatus status = OS_SUCCESS;
   mbAutoAdvance = FALSE;
   PlayListEntry* e;

   mCurrentElement = mPlayListDb->entries();
   UtlSListIterator playListDbIterator(*mPlayListDb) ;
   while((e = (PlayListEntry*)playListDbIterator()))
   {
      stopEntry(e);
   }

   return status;
}



// Marks the player as destroy and frees all allocated resources in media
// processing.
OsStatus MpStreamPlaylistPlayer::destroy()
{
   OsStatus status = OS_SUCCESS;   // This is non-blocking, assume success.
   PlayListEntry* e;

   OsSysLog::add(FAC_MP, PRI_DEBUG, "MpStreamPlaylistPlayer::destroy");
   mCurrentElement = mPlayListDb->entries();
   UtlSListIterator playListDbIterator(*mPlayListDb) ;
   while((e = (PlayListEntry*)playListDbIterator()))
   {
      destroyEntry(e, false);
   }

   return status;
}


// Pauses the media stream temporarily.
OsStatus MpStreamPlaylistPlayer::pause()
{
   OsStatus status = OS_FAILED;

   int iPlayingElement = mPlayingElement;
   if (iPlayingElement != -1)
   {
      PlayListEntry* e = (PlayListEntry*)mPlayListDb->at(iPlayingElement);
      status = pauseEntry(e);
   }

   return status;
}


/* ============================ ACCESSORS ================================= */

// Gets the number of play list entries
OsStatus MpStreamPlaylistPlayer::getCount(int& count) const
{
   count = (int)mPlayListDb->entries();
   return OS_SUCCESS;
}


// Gets the source type for playlist entry 'index'.
OsStatus MpStreamPlaylistPlayer::getSourceType(int index, int& type) const
{
   OsStatus status = OS_INVALID_ARGUMENT;
   PlayListEntry* e = (PlayListEntry*)mPlayListDb->at(index) ;

   if (e != NULL)
   {
      type = e->sourceType;
      status = OS_SUCCESS;
   }

   return status;
}


// Gets the source url for playlist entry 'index'.
OsStatus MpStreamPlaylistPlayer::getSourceUrl(int index, Url url) const
{
   OsStatus status = OS_INVALID_ARGUMENT;
   PlayListEntry* e = (PlayListEntry*)mPlayListDb->at(index) ;

   if (e != NULL)
   {
      if (e->sourceType == SourceUrl)
      {
         url = e->url;
         status = OS_SUCCESS;
      }
   }

   return status;
}


// Gets the source buffer for playlist entry 'index'.
OsStatus MpStreamPlaylistPlayer::getSourceBuffer(int index, UtlString*& netBuffer) const
{
   OsStatus status = OS_INVALID_ARGUMENT;
   PlayListEntry* e = (PlayListEntry*)mPlayListDb->at(index) ;

   if (e != NULL)
   {
      if (e->sourceType == SourceBuffer)
      {
         netBuffer = e->pBuffer;
         status = OS_SUCCESS;
      }
   }

   return status;
}

// Gets the state for the playlist entry 'index'.
OsStatus MpStreamPlaylistPlayer::getSourceState(int index, PlayerState& state) const

{
   OsStatus status = OS_INVALID_ARGUMENT;
   PlayListEntry* e = (PlayListEntry*)mPlayListDb->at(index) ;

   if (e != NULL)
   {
      state = e->state;
      status = OS_SUCCESS;
   }

   return status;
}

// Gets the current playing index if playing or the next index to play if
// playNext() was invoked.
OsStatus MpStreamPlaylistPlayer::getCurrentIndex(int& index) const
{
   OsStatus status = OS_SUCCESS;

   if (mPlayingElement == -1)
      index = mCurrentElement;
   else
      index = mPlayingElement;

   return status;
}

// Gets the aggregate playerlist player state
OsStatus MpStreamPlaylistPlayer::getState(PlayerState& state)
{
   state = mAggregateState;

   return OS_SUCCESS;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */


// Copy constructor
MpStreamPlaylistPlayer::MpStreamPlaylistPlayer(const MpStreamPlaylistPlayer& rMpStreamPlaylistPlayer)
   : mSemStateChange(OsBSem::Q_PRIORITY, OsBSem::EMPTY)
   , mWaitEvent(0)
{
}

// Assignment operator
MpStreamPlaylistPlayer&
MpStreamPlaylistPlayer::operator=(const MpStreamPlaylistPlayer& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}


// Selects the first playlist entry as the next index to play.
OsStatus MpStreamPlaylistPlayer::first()
{
   if (mPlayingElement != -1)
      stop();

   mCurrentElement = 0;

   return OS_SUCCESS;
}

// Selects the last playlist entry as the next index to play.
OsStatus MpStreamPlaylistPlayer::last()
{
   stop();

   int n = mPlayListDb->entries() ;
   if (n > 0)
      mCurrentElement = n - 1;
   else
      mCurrentElement = 0;

   return OS_SUCCESS;
}


// Plays the next playlist entry without wrapping.
OsStatus MpStreamPlaylistPlayer::playNext(UtlBoolean bBlock /*= TRUE*/)
{
   OsStatus status = OS_LIMIT_REACHED;

   if (mCurrentElement < (int)mPlayListDb->entries())
   {
      int iPlayElement = mCurrentElement++;
      status = playEntry(iPlayElement, bBlock);
   }

   return status;
}


// Plays the previous playlist entry without wrapping.
OsStatus MpStreamPlaylistPlayer::playPrevious(UtlBoolean bBlock /*= TRUE*/)
{
   OsStatus status = OS_LIMIT_REACHED;

   if (mCurrentElement > 0)
   {
      mCurrentElement--;
      status = playEntry(mCurrentElement, bBlock);
      mCurrentElement++;
   }

   return status;
}


// Sets the state for a specific entry.
void MpStreamPlaylistPlayer::setEntryState(PlayListEntry* e, PlayerState newState)
{
//   OsSysLog::add(FAC_MP, PRI_DEBUG, "MpStreamPlaylistPlayer::setEntryState %p[%d] newState=%d", e, e?e->index:-1, newState);

   if (e == NULL)
   {
      // Update any blocking calls
      mSemStateChange.release();
      return ;
   }

   PlayerState oldState = e->state;

   if (oldState != newState)
   {
#ifdef MP_STREAM_DEBUG /* [ */
      osPrintf("MpStreamPlaylistPlayer::setEntryState(%p): Setting mPlayListDb[%d].state = %s\n",
               this, e->index, getEventString(newState));
#endif /* MP_STREAM_DEBUG ] */
      // Store the new state
      e->state = newState;

      // Updated aggregate state given the new entry state
      switch (newState)
      {
         case PlayerUnrealized:
            break;
         case PlayerRealized:
            handleRealizedState(oldState, newState);
            break;
         case PlayerPrefetched:
            handlePrefetchedState(oldState, newState);
            break;
         case PlayerPlaying:
            handlePlayingState(oldState, newState);
            break;
         case PlayerPaused:
            handlePausedState(oldState, newState);
            break;
         case PlayerStopped:
         case PlayerAborted:
            handleStoppedState(oldState, newState);
            break;
         case PlayerDestroyed:
            break;
         case PlayerFailed:
            handleFailedState(oldState, newState);
            break;
      }

      // Update any blocking calls
      mSemStateChange.release();
   }
}


// Starts playing a specific entry
OsStatus MpStreamPlaylistPlayer::playEntry(int iEntry, UtlBoolean bBlock /*= TRUE*/)
{
   OsStatus status = OS_INVALID_ARGUMENT;
   PlayListEntry* e = (PlayListEntry*)mPlayListDb->at(iEntry) ;

   if (e != NULL)
   {
      // Only proceed if we have a flow graph and the player is realized.
      // NOTE: The player doesn't need to be prefetched
      if (  (e->state == PlayerRealized) ||
            (e->state == PlayerPaused) ||
            (e->state == PlayerPrefetched))
      {
         mPlayingElement = iEntry;
         MpStreamMsg msg(MpStreamMsg::STREAM_PLAY, mTarget, e->handle);
         status = mpMsgQ->send(msg);
         if (status != OS_SUCCESS)
         {
            setEntryState(e, PlayerFailed);
            OsSysLog::add(FAC_MP, PRI_ERR, "MpStreamPlaylistPlayer::playEntry failed on send of MpStreamMsg::STREAM_PLAY message");
         }
         else
         {
            if (e->state == PlayerPaused)
               setEntryState(e, PlayerPlaying);

            // Block while waiting for play to complete (if requested)
            if ((status == OS_SUCCESS)  && bBlock)
            {
               while (  (e->state == PlayerRealized)
                     || (e->state == PlayerPrefetched)
                     || (e->state == PlayerPlaying)
                     || (e->state == PlayerPaused))
               {
                  status = mSemStateChange.acquire(mPlayTimeout);
                  if (status == OS_WAIT_TIMEOUT)
                  {
                     setEntryState(e, PlayerFailed);
                     OsSysLog::add(FAC_MP, PRI_ERR, "MpStreamPlaylistPlayer::playEntry timed out waiting for play to complete");
                     break;
                  }
               }
            }
         }
      }
      else
      {
         status = OS_INVALID_STATE;
         OsSysLog::add(FAC_MP, PRI_ERR, "MpStreamPlaylistPlayer::playEntry failed due to current state being invalid");
      }
   }
   return status;
}


// Rewinds a previously played media stream.  In some cases this may result
// in a re-connect/refetch.
OsStatus MpStreamPlaylistPlayer::rewindEntry(PlayListEntry* e, UtlBoolean bBlock /*= TRUE*/)
{
   OsStatus status = OS_INVALID_ARGUMENT;

   if (e != NULL)
   {
      if (e->state == PlayerPrefetched)
      {
         status = OS_SUCCESS;
      }
      else
      {
         // Only proceed if we have a flow graph and the player is realized.
         // NOTE: The player doesn't need to be prefetched
         if ((e->state == PlayerStopped) || (e->state == PlayerAborted))
         {
            MpStreamMsg msg(MpStreamMsg::STREAM_REWIND, mTarget, e->handle);
            status = mpMsgQ->send(msg);
            if (status != OS_SUCCESS)
            {
               setEntryState(e, PlayerFailed);
               OsSysLog::add(FAC_MP, PRI_ERR, "MpStreamPlaylistPlayer::rewindEntry failed on send of MpStreamMsg::STREAM_REWIND message");
            }
            else
            {
               // Block while waiting for rewind to complete (if requested)
               if (bBlock)
               {
                  while ((e->state == PlayerStopped) ||
                        (e->state == PlayerAborted)  ||
                        (e->state == PlayerRealized))
                  {
                     status = mSemStateChange.acquire(mRewindTimeout);
                     if (status == OS_WAIT_TIMEOUT)
                     {
                        setEntryState(e, PlayerFailed);
                        OsSysLog::add(FAC_MP, PRI_ERR, "MpStreamPlaylistPlayer::rewindEntry timed out waiting for Rewind to complete");
                        break;
                     }
                  }
               }
            }
         }
         else
         {
            status = OS_INVALID_STATE;
         }
      }
   }
   return status;
}


// Stops playing a specific entry
OsStatus MpStreamPlaylistPlayer::stopEntry(PlayListEntry* e, UtlBoolean bBlock /* = TRUE */)
{
   OsStatus status = OS_INVALID_ARGUMENT;

   if (e != NULL)
   {
      // Only proceed if we have a flow graph and the player is realized.
      // NOTE: The player doesn't need to be prefetched
      if (  (e->state == PlayerPrefetched) ||
            (e->state == PlayerPlaying) ||
            (e->state == PlayerPaused))
      {
         MpStreamMsg msg(MpStreamMsg::STREAM_STOP, mTarget, e->handle);
         status = mpMsgQ->send(msg);
         if (status != OS_SUCCESS)
         {
            setEntryState(e, PlayerFailed);
            OsSysLog::add(FAC_MP, PRI_ERR, "MpStreamPlaylistPlayer::stopEntry failed on send of MpStreamMsg::STREAM_STOP message");
         }
         else
         {
            if (bBlock)
            {
               while ((e->state != PlayerStopped) &&
                      (e->state != PlayerAborted))

               {
                  status = mSemStateChange.acquire(mStopTimeout);
                  if (status == OS_WAIT_TIMEOUT)
                  {
                     setEntryState(e, PlayerFailed);
                     OsSysLog::add(FAC_MP, PRI_ERR, "MpStreamPlaylistPlayer::stopEntry timed out waiting for Stop to complete");
                     break;
                  }
               }
            }
         }
      }
      else if ((e->state == PlayerStopped) ||
               (e->state == PlayerAborted))
         status = OS_SUCCESS;
   }
   return status;
}


// Pauses a specific entry
OsStatus MpStreamPlaylistPlayer::pauseEntry(PlayListEntry* e)
{
   OsStatus status = OS_INVALID_ARGUMENT;

   if (e != NULL)
   {
      status = OS_SUCCESS;

      // Only proceed if we have a flow graph and the player is prefetched or
      // playing.
      if ( (e->state == PlayerPrefetched) ||
           (e->state == PlayerPlaying))
      {
         MpStreamMsg msg(MpStreamMsg::STREAM_PAUSE, mTarget, e->handle);
         status = mpMsgQ->send(msg);
         if (status != OS_SUCCESS)
         {
            setEntryState(e, PlayerFailed);
            OsSysLog::add(FAC_MP, PRI_ERR, "MpStreamPlaylistPlayer::pauseEntry failed on send of MpStreamMsg::STREAM_PAUSE message");
         }
      }
   }

   return status;
}


// Pauses a specific entry
OsStatus MpStreamPlaylistPlayer::destroyEntry(PlayListEntry* e, UtlBoolean bBlockAndClean /*= TRUE*/)
{
   OsStatus status = OS_INVALID_ARGUMENT;

//   OsSysLog::add(FAC_MP, PRI_DEBUG, "MpStreamPlaylistPlayer::destroyEntry %p[%d]", e, e? e->index:-1);

   if (e != NULL)
   {
      status = OS_SUCCESS;

      // Only proceed if we have a flow graph and the player is prefetched or
      // playing.
      if (e->state != PlayerUnrealized)
      {
         int iState = e->state;
         if (  (iState != PlayerStopped) &&
               (iState != PlayerAborted) &&
               (iState != PlayerDestroyed)  )
         {
            MpStreamMsg msgStop(MpStreamMsg::STREAM_STOP, mTarget, e->handle);
            status = mpMsgQ->send(msgStop);
            if (status != OS_SUCCESS)
            {
               setEntryState(e, PlayerFailed);
               OsSysLog::add(FAC_MP, PRI_ERR, "MpStreamPlaylistPlayer::destroyEntry failed on send of MpStreamMsg::STREAM_STOP message");
            }
         }

         if ((iState != PlayerDestroyed) && (status == OS_SUCCESS))
         {
            MpStreamMsg msgDestroy(MpStreamMsg::STREAM_DESTROY, mTarget, e->handle);
            status = mpMsgQ->send(msgDestroy);
            if (status != OS_SUCCESS)
            {
               setEntryState(e, PlayerFailed);
               OsSysLog::add(FAC_MP, PRI_ERR, "MpStreamPlaylistPlayer::destroyEntry failed on send of MpStreamMsg::STREAM_DESTROY message");
            }
         }


         if (bBlockAndClean)
         {
            while (e->state != PlayerDestroyed)
            {
               status = mSemStateChange.acquire(mDestroyTimeout);
               if (status == OS_WAIT_TIMEOUT)
               {
                  OsSysLog::add(FAC_MP, PRI_ERR, "MpStreamPlaylistPlayer::destroyEntry timed out waiting for Delete to complete");
                  break;
               }
            }

            if (e->pQueuedEvent != NULL)
            {
               delete e->pQueuedEvent;
            }
            // Delete the PlayListEntry
            delete e ;
         }
      }
   }

   return status;
}


// Handle messages directed to this server task.
UtlBoolean MpStreamPlaylistPlayer::handleMessage(OsMsg& rMsg)
{
   switch (rMsg.getMsgType())
   {
      case OsMsg::OS_EVENT:
         OsEventMsg* pMsg = (OsEventMsg*) &rMsg;
         intptr_t status;
         int index;
	 void* indexVoid;

         pMsg->getUserData(indexVoid);
	 index = (int)(intptr_t)indexVoid;
         if (pMsg->getEventData(status) == OS_SUCCESS)
         {
            PlayListEntry* e = (PlayListEntry*)mPlayListDb->at(index) ;
#ifdef MP_STREAM_DEBUG /* [ */
            osPrintf("MpStreamPlaylistPlayer::handleMessage(%p): Received Feeder event: %s \n",
                     this, getFeederEventString(status));

            OsSysLog::add(FAC_MP, PRI_DEBUG,
               "MpStreamPlaylistPlayer::handleMessage(%p): Received Feeder event: %s index=%d e->index=%d",
                  this, getFeederEventString(status), index, e?e->index:-1);
#endif /* MP_STREAM_DEBUG ] */
            switch (status)
            {
               case FeederRealizedEvent:
                  setEntryState(e, PlayerRealized);
                  break;

               case FeederPrefetchedEvent:
                  setEntryState(e, PlayerPrefetched);
                  break;

               case FeederStoppedEvent:
                  if (mAggregateState != PlayerPlaying)
                  {
                     setEntryState(e, PlayerPrefetched);
                  }
                  break;

               case FeederRenderingEvent:
                  break;

               case FeederFailedEvent:
                  setEntryState(e, PlayerFailed);
                  break;

               case FeederStreamPlayingEvent:
                  setEntryState(e, PlayerPlaying);
                  break;

               case FeederStreamPausedEvent:
                  setEntryState(e, PlayerPaused);
                  break;

               case FeederStreamStoppedEvent:
                  setEntryState(e, PlayerStopped);
                  break;

               case FeederStreamDestroyedEvent:
                  setEntryState(e, PlayerDestroyed);
                  break;

               case FeederStreamAbortedEvent:
                  setEntryState(e, PlayerStopped);
                  break;

            }
         }
         break;
   }

   return TRUE;
}


// Handles processing for the realized state
void MpStreamPlaylistPlayer::handleRealizedState(PlayerState oldState, PlayerState newState)
{
   //
   // Updated the mAggregateState if all play list items are realized
   //
   if (mAggregateState == PlayerUnrealized)
   {
      UtlBoolean bAnyUnRealized = FALSE;
      UtlSListIterator playListDbIterator(*mPlayListDb) ;
      while(PlayListEntry* e = (PlayListEntry*)playListDbIterator())
      {
         if (e->state == PlayerUnrealized)
         {
            bAnyUnRealized = TRUE;
            break;
         }
      }

      // If everything is PlayerRealized or better, then updated the aggregate
      // state and fire off the event to listeners
      if (!bAnyUnRealized)
      {
#ifdef MP_STREAM_DEBUG /* [ */
         osPrintf("MpStreamPlaylistPlayer::handleRealizedState(%p): Changed from %s to PlayerRealized.\n",
                  this, getEventString(mAggregateState));
#endif /* ] */
         mAggregateState = PlayerRealized;
         fireEvent(PlayerRealized);
      }
   }
}


// Handles processing for the prefetched state
void MpStreamPlaylistPlayer::handlePrefetchedState(PlayerState oldState, PlayerState newState)
{
   //
   // Updated the mAggregateState if all play list items are prefetched
   //
   if ((mAggregateState == PlayerUnrealized) || (mAggregateState == PlayerRealized))
   {
      UtlBoolean bAllPrefetched = TRUE;
      UtlSListIterator playListDbIterator(*mPlayListDb) ;
      while(PlayListEntry* e = (PlayListEntry*)playListDbIterator())
      {
         if (  (e->state != PlayerPrefetched) &&
               (e->state != PlayerFailed))
         {
            bAllPrefetched = FALSE;
            break;
         }
      }

      // If everything is prefetched, then updated the aggregate state and
      // fire off the event to listeners
      if (bAllPrefetched)
      {
#ifdef MP_STREAM_DEBUG /* [ */
         osPrintf("MpStreamPlaylistPlayer::handlePrefetchedState(%p): Changed from %s to PlayerPrefetched.\n",
                  this, getEventString(mAggregateState));
#endif /* ] */
         mAggregateState = PlayerPrefetched;
         fireEvent(PlayerPrefetched);
      }
   }
}


// Handles processing for the playing state
void MpStreamPlaylistPlayer::handlePlayingState(PlayerState oldState, PlayerState newState)
{
   if (mAggregateState != PlayerPlaying)
   {
#ifdef MP_STREAM_DEBUG /* [ */
         osPrintf("MpStreamPlaylistPlayer::handlePlayingState(%p): Changed from %s to PlayerPlaying.\n",
                  this, getEventString(mAggregateState));
#endif /* ] */
      mAggregateState = PlayerPlaying;
      fireEvent(PlayerPlaying);
   }
}


// Handles processing for the paused state
void MpStreamPlaylistPlayer::handlePausedState(PlayerState oldState, PlayerState newState)
{
   if (mAggregateState != PlayerPaused)
   {
#ifdef MP_STREAM_DEBUG /* [ */
         osPrintf("MpStreamPlaylistPlayer::handlePausedState(%p): Changed from %s to PlayerPaused.\n",
                  this, getEventString(mAggregateState));
#endif /* ] */
         mAggregateState = PlayerPaused;
         fireEvent(PlayerPaused);
   }
}


// Handles processing for the stopped state
void MpStreamPlaylistPlayer::handleStoppedState(PlayerState oldState, PlayerState newState)
{
   if (  (mbAutoAdvance) &&
         (mCurrentElement < (int)mPlayListDb->entries()) &&
         (newState != PlayerAborted))
   {
      playNext(FALSE);
   }
   else if (newState == PlayerAborted)
   {
      if (mAggregateState != PlayerAborted)
      {
#ifdef MP_STREAM_DEBUG /* [ */
         osPrintf("MpStreamPlaylistPlayer::handleStoppedState(%p): Changed from %s to PlayerAborted.\n",
                  this, getEventString(mAggregateState));
#endif /* ] */
         mAggregateState = PlayerAborted;
         fireEvent(PlayerAborted);
         // Wake up anyone waiting on play completion.
         mWaitEvent.signal(0);
      }
   }
   else
   {
      if (mCurrentElement >= (int)mPlayListDb->entries())
      {
         mbAutoAdvance = FALSE;

         if (mAggregateState != PlayerStopped)
         {
#ifdef MP_STREAM_DEBUG /* [ */
            osPrintf("MpStreamPlaylistPlayer::handleStoppedState(%p): Changed from %s to PlayerStopped.\n",
                     this, getEventString(mAggregateState));
#endif /* ] */
            mAggregateState = PlayerStopped;
            fireEvent(PlayerStopped);
            // Wake up anyone waiting on play completion.
            mWaitEvent.signal(0);
         }
      }
   }
}


// Handles processing for the failed state
void MpStreamPlaylistPlayer::handleFailedState(PlayerState oldState, PlayerState newState)
{
#ifdef MP_STREAM_DEBUG /* [ */
   osPrintf("MpStreamPlaylistPlayer::handleFailedState(%p): Changed from %s to PlayerFailed.\n",
            this, getEventString(mAggregateState));
#endif /* ] */
   mAggregateState = PlayerFailed;
   // Wake up anyone waiting on play completion.
   mWaitEvent.signal(0);
}



/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

const char* MpStreamPlaylistPlayer::getFeederEventString(int iEvent)
{
   const char* szRC = "ERROR_UNKNOWN";

   switch (iEvent)
   {
      case FeederRealizedEvent:
         szRC = "FeederRealizedEvent";
         break;
      case FeederPrefetchedEvent:
         szRC = "FeederPrefetchedEvent";
         break;
      case FeederRenderingEvent:
         szRC = "FeederRenderingEvent";
         break;
      case FeederStoppedEvent:
         szRC = "FeederStoppedEvent";
         break;
      case FeederFailedEvent:
         szRC = "FeederFailedEvent";
         break;
      case FeederStreamPlayingEvent:
         szRC = "FeederStreamPlayingEvent";
         break;
      case FeederStreamPausedEvent:
         szRC = "FeederStreamPausedEvent";
         break;
      case FeederStreamAbortedEvent:
         szRC = "FeederStreamAbortedEvent";
         break;
      case FeederStreamStoppedEvent:
         szRC = "FeederStreamStoppedEvent";
         break;
      case FeederStreamDestroyedEvent:
         szRC = "FeederStreamDestroyedEvent";
         break;
   }
   return szRC;
}

/* ============================ FUNCTIONS ================================= */

//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "mp/MpStreamQueuePlayer.h"
#include "mp/MpQueuePlayerListener.h"
#include "mp/MpStreamPlayer.h"

#include "os/OsEventMsg.h"
#include "os/OsQueuedEvent.h"
#include "os/OsWriteLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */


// Contructor accepting a msg queue
MpStreamQueuePlayer::MpStreamQueuePlayer(OsMsgQ* pMsgQ, const char* pTarget)
   : OsServerTask("QueuePlay-%d")
   , mSemQueueChange(OsBSem::Q_PRIORITY, OsBSem::FULL)
   , mSemWaitSynch(OsBSem::Q_FIFO, OsBSem::EMPTY)
   , mbFatalError(FALSE)
   , mListenerMutex(OsMutex::Q_PRIORITY)
{
   mpMsgQ = pMsgQ ;
   mpQueueEvent =  new OsQueuedEvent(*getMessageQueue(), 0);

   // Initialize the Queues
   mToPlayQueue = NULL ;
   mToPlayQueueLength = expandQueue(mToPlayQueue, 0, DEFAULT_QUEUE_LENGTH) ;
   mNumToPlayElements = 0 ;
   mPlayingQueue = NULL ;
   mPlayingQueueLength = expandQueue(mPlayingQueue, 0, DEFAULT_QUEUE_LENGTH) ;
   mNumPlayingElements = 0 ;
   if (pTarget != NULL)
      mTarget = pTarget ;


   // Initialize Listener List
   for (int j=0; j<MAX_PLAYER_LISTENERS; j++)
   {
      mListenerDb[j].inUse = FALSE ;
      mListenerDb[j].pListener = NULL ;
   }

   if (start() == TRUE)
   {
      // Wait for us to startup
      while (!isStarted())
         OsTask::yield() ;
   }
   else
   {
      syslog(FAC_STREAMING, PRI_CRIT, "Failed to create thread for MpStreamQueuePlayer") ;
      mbFatalError = TRUE ;
   }
}


// Destructor
MpStreamQueuePlayer::~MpStreamQueuePlayer()
{
   int i ;

   /*
    * Issue a reset and clear to remove all the players.
    */
   reset() ;
   clear() ;

   // Drain the Event
   getMessageQueue()->flush() ;

   mSemQueueChange.acquire() ;
   // Next clear the queues
   if (mToPlayQueue != NULL)
   {
      for (i = 0; i < mToPlayQueueLength; i++)
      {
         if (mToPlayQueue[i].pPlayer != NULL)
         {
            delete mToPlayQueue[i].pPlayer;
            mToPlayQueue[i].pPlayer = NULL;
         }
      }
      free(mToPlayQueue) ;
      mToPlayQueueLength = 0 ;
      mToPlayQueue = NULL ;
      mNumToPlayElements = 0 ;
   }

   if (mPlayingQueue != NULL)
   {
      for (i = 0; i < mPlayingQueueLength; i++)
      {
         if (mPlayingQueue[i].pPlayer != NULL)
         {
            delete mPlayingQueue[i].pPlayer;
            mPlayingQueue[i].pPlayer = NULL;
         }
      }
      free(mPlayingQueue) ;
      mPlayingQueueLength = 0 ;
      mPlayingQueue = NULL ;
      mNumPlayingElements = 0 ;
   }
   mSemQueueChange.release() ;

   // Wake up anyone waiting on this class
   mSemWaitSynch.release() ;

   // Lastly wait until we have shutdown and kill our event queue
   waitUntilShutDown() ;
   if (mpQueueEvent != NULL)
   {
      delete mpQueueEvent ;
      mpQueueEvent = NULL ;
   }
}


/* ============================ MANIPULATORS ============================== */

// Adds a url to the playlist
OsStatus MpStreamQueuePlayer::add(Url& url, int flags)
{
   if (mbFatalError)
      return OS_FAILED ;

#ifdef PLAYER_STUBS
   return OS_SUCCESS ;
#else
   OsStatus        status = OS_LIMIT_REACHED ;  // return code
   MpStreamPlayer* pNewPlayer = NULL ;          // newly created/added player

   mSemQueueChange.acquire() ;

   // Grow queue length if needed
   if (mNumToPlayElements == mToPlayQueueLength)
   {
      mToPlayQueueLength = expandQueue(mToPlayQueue,
            mToPlayQueueLength, mToPlayQueueLength + EXPAND_QUEUE_LENGTH) ;
   }

   // Only add if we are under the max length
   if (mNumToPlayElements < mToPlayQueueLength)
   {
      int index = mNumToPlayElements ;
      mNumToPlayElements++ ;

      // Create the player and set playing in motion by invoking "realize"
      pNewPlayer = new MpStreamPlayer(mpMsgQ, url, flags, mTarget) ;
      mToPlayQueue[index].pPlayer = pNewPlayer ;
      mToPlayQueue[index].bFailed = FALSE ;
      mToPlayQueue[index].pPlayer->addListener(this) ;
   }
   mSemQueueChange.release() ;

   if (pNewPlayer)
   {
      // Handle API Failures (different from rendering event failures)
      status = pNewPlayer->realize(TRUE) ;
      if ((status != OS_SUCCESS) && (pNewPlayer != NULL))
      {
         setFailedPlayer(pNewPlayer) ;
      }
   }

   return status ;
#endif

}


// Adds a buffer to the playlist
OsStatus MpStreamQueuePlayer::add(UtlString* pBuffer, int flags)
{
   if (mbFatalError)
      return OS_FAILED ;

#ifdef PLAYER_STUBS
   return OS_SUCCESS ;
#else
   OsStatus        status = OS_LIMIT_REACHED ;  // return code
   MpStreamPlayer* pNewPlayer = NULL ;          // newly created/added player

   mSemQueueChange.acquire() ;

   // Grow queue length if needed
   if (mNumToPlayElements == mToPlayQueueLength)
   {
      mToPlayQueueLength = expandQueue(mToPlayQueue,
            mToPlayQueueLength, mToPlayQueueLength + EXPAND_QUEUE_LENGTH) ;
   }

   // Only add if we are under the max length
   if (mNumToPlayElements < mToPlayQueueLength)
   {
      int index = mNumToPlayElements ;
      mNumToPlayElements++ ;

      // Create the player and set playing in motion by invoking "realize"
      pNewPlayer = new MpStreamPlayer(mpMsgQ, pBuffer, flags, mTarget) ;
      mToPlayQueue[index].pPlayer = pNewPlayer ;
      mToPlayQueue[index].bFailed = FALSE ;
      mToPlayQueue[index].pPlayer->addListener(this) ;
   }

   mSemQueueChange.release() ;

   if (pNewPlayer)
   {
      // Handle API Failures (different from rendering event failures)
      status = pNewPlayer->realize(TRUE) ;
      if ((status != OS_SUCCESS) && (pNewPlayer != NULL))
      {
         setFailedPlayer(pNewPlayer) ;
      }
   }
   return status ;
#endif
}


OsStatus MpStreamQueuePlayer::play()
{
   if (mbFatalError)
      return OS_FAILED ;

#ifdef PLAYER_STUBS
   mpQueueEvent->signal(EVENT_PLAY_NEXT) ;
   return OS_SUCCESS ;
#else
   OsStatus status = OS_SUCCESS ;

   // First stop anything that was playing
   if (isPlaying())
      reset() ;

   // Copy the queue list to the playing list
   mSemQueueChange.acquire() ;

   swapQueues(mPlayingQueue, mPlayingQueueLength, mToPlayQueue, mToPlayQueueLength) ;
   mNumPlayingElements = mNumToPlayElements ;
   mNumToPlayElements = 0 ;

   if (mNumPlayingElements > 0)
   {
      // Inform listeners that we will begin playing
      fireQueuePlayerStarted() ;

      // Start Playing
      mpQueueEvent->signal(EVENT_PLAY_NEXT) ;
   }
   mSemQueueChange.release() ;


   // Wake up anyone waiting for a change in the queue state
   mSemWaitSynch.release() ;

   return status ;
#endif
}


// Resets the queue player state by stopping and removing all playing entries.
OsStatus MpStreamQueuePlayer::reset()
{
   if (mbFatalError)
      return OS_FAILED ;

#ifdef PLAYER_STUBS
   mpQueueEvent->signal(EVENT_RESET) ;
   return OS_SUCCESS ;
#else
   OsStatus status = OS_SUCCESS ;

   mpQueueEvent->signal(EVENT_RESET) ;
   wait() ;

   return status ;
#endif
}

// Resets the queue player state by stopping and removing all playing entries.
OsStatus MpStreamQueuePlayer::destroy()
{
   if (mbFatalError)
      return OS_FAILED ;

   return reset() ;
}



// Clears any queued entries that have not yet been scheduled for play.
OsStatus MpStreamQueuePlayer::clear()
{
   if (mbFatalError)
      return OS_FAILED ;

#ifdef PLAYER_STUBS
   return OS_SUCCESS ;
#else
   MpStreamPlayer** pDeleteList ;
   int              iToDelete = 0 ;
   int              i ;
   OsStatus         status = OS_SUCCESS ;

   mSemQueueChange.acquire() ;

   iToDelete = mNumToPlayElements ;
   pDeleteList = new MpStreamPlayer*[iToDelete] ;
   for (i=0; i<iToDelete; i++)
   {
      // Clean up the player
      pDeleteList[i] = mToPlayQueue[i].pPlayer ;
      mToPlayQueue[i].pPlayer = NULL ;
      mToPlayQueue[i].bFailed = FALSE ;
   }
   mNumToPlayElements = 0 ;

   mSemQueueChange.release() ;

   for (i=0; i<iToDelete; i++)
   {
      pDeleteList[i]->removeListener(this) ;
      pDeleteList[i]->stop() ;
      delete pDeleteList[i] ;
      pDeleteList[i] = NULL ;
   }
   delete[] pDeleteList ;

   mSemWaitSynch.release() ;

   return status ;
#endif
}


// Wait until all play list items are finished playing
OsStatus MpStreamQueuePlayer::wait(const OsTime& rTimeout)
{
   if (mbFatalError)
      return OS_FAILED ;

#ifdef PLAYER_STUBS
   // Delay for 1 second
   OsTask::delay(1000) ;
   return OS_SUCCESS ;
#else
   OsStatus status = OS_SUCCESS ;

   while (isPlaying())
   {
      status = mSemWaitSynch.acquire(rTimeout) ;
      if (status != OS_SUCCESS)
         break ;
   }

   return status ;
#endif
}


// Adds a player listener to receive notifications when this player changes
// state
OsStatus MpStreamQueuePlayer::addListener(MpQueuePlayerListener* pListener)
{
   if (mbFatalError)
      return OS_FAILED ;

   OsWriteLock lock(mListenerMutex);
   OsStatus status = OS_LIMIT_REACHED ;

   for (int i=0; i<MAX_PLAYER_LISTENERS; i++)
   {
      if (!mListenerDb[i].inUse)
      {
         mListenerDb[i].inUse = TRUE ;
         mListenerDb[i].pListener = pListener ;

         status = OS_SUCCESS ;
         break;
      }
   }

   return status ;
}


// Removes a previously added player listener.  This listener will cease to
// receive state change notifications.
OsStatus MpStreamQueuePlayer::removeListener(MpQueuePlayerListener* pListener)
{
   if (mbFatalError)
      return OS_FAILED ;

   OsWriteLock lock(mListenerMutex);
   OsStatus status = OS_NOT_FOUND ;

   for (int i=0; i<MAX_PLAYER_LISTENERS; i++)
   {
      if ((mListenerDb[i].inUse) && (mListenerDb[i].pListener == pListener))
      {
         mListenerDb[i].inUse = FALSE ;
         mListenerDb[i].pListener = NULL ;

         status = OS_SUCCESS ;
      }
   }
   return status ;
}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

// Is the Queue player playing (or about to play)
UtlBoolean MpStreamQueuePlayer::isPlaying()
{
   if (mbFatalError)
      return FALSE ;

   UtlBoolean bIsPlaying = FALSE ;   // Are we playing?

   mSemQueueChange.acquire() ;

   if ((mPlayingQueue != NULL) && (mPlayingQueue[0].pPlayer != NULL))
      bIsPlaying = TRUE ;

   mSemQueueChange.release() ;

   return bIsPlaying ;
}


/* //////////////////////////// PROTECTED ///////////////////////////////// */


// Copy constructor
MpStreamQueuePlayer::MpStreamQueuePlayer(const MpStreamQueuePlayer& rMpStreamQueuePlayer)
   : mSemQueueChange(OsBSem::Q_PRIORITY, OsBSem::FULL)
   , mSemWaitSynch(OsBSem::Q_FIFO, OsBSem::EMPTY)
   , mListenerMutex(OsMutex::Q_PRIORITY)
{
   assert(FALSE) ;
}

// Assignment operator
MpStreamQueuePlayer&
MpStreamQueuePlayer::operator=(const MpStreamQueuePlayer& rhs)
{
   assert(FALSE) ;

   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}


// Handles OS server task events/messages
UtlBoolean MpStreamQueuePlayer::handleMessage(OsMsg& rMsg)
{
   UtlBoolean bHandled = FALSE ;

   switch (rMsg.getMsgType())
   {
      case OsMsg::OS_EVENT:
         OsEventMsg* pMsg = (OsEventMsg*) &rMsg ;
         intptr_t action ;
         if (pMsg->getEventData(action) == OS_SUCCESS)
         {
            switch (action)
            {
               case EVENT_DEQUEUE:
                  handleDequeue() ;
                  break ;
               case EVENT_PLAY_NEXT:
#ifdef PLAYER_STUBS
                  fireQueuePlayerStarted() ;
                  OsTask::delay(1000) ;
                  fireQueuePlayerStopped() ;
#else
                  handlePlayNext() ;
#endif
                  break ;
               case EVENT_REMOVE_FAILED:
#ifdef PLAYER_STUBS
                 fireQueuePlayerStopped() ;
#else
                  handleRemoveFailed() ;
#endif
                  break ;
               case EVENT_RESET:
#ifdef PLAYER_STUBS
                  fireQueuePlayerStopped() ;
#else
                  handleReset() ;
#endif
                  break ;
            }
         }
         bHandled = TRUE ;
         break ;
   }

   return bHandled ;
}


void MpStreamQueuePlayer::handleReset()
{
   MpStreamPlayer** pDeleteList = NULL;
   int             iToDelete = 0 ;
   int             i ;

   // Remove all of the entries from the list
   mSemQueueChange.acquire() ;
   iToDelete = mNumPlayingElements ;
   if (iToDelete)
   {
      pDeleteList = new MpStreamPlayer*[iToDelete] ;
      for (i=0; i<iToDelete; i++)
      {
         // Clean up the player
         pDeleteList[i] = mPlayingQueue[i].pPlayer ;

         mPlayingQueue[i].pPlayer = NULL ;
         mPlayingQueue[i].bFailed = FALSE ;
      }
      mNumPlayingElements = 0 ;

      mSemQueueChange.release() ;
   }
   else
   {
      mSemQueueChange.release() ;
   }

   // Delete them
   if (pDeleteList != NULL)
   {
      for (i=0; i<iToDelete; i++)
      {
         pDeleteList[i]->removeListener(this) ;
         pDeleteList[i]->destroy() ;
         delete pDeleteList[i] ;
         pDeleteList[i] = NULL ;
      }
      delete[] pDeleteList ;
   }

   mSemWaitSynch.release() ;

   // Inform listeners that the player has stopped
   if (iToDelete > 0)
   {
      fireQueuePlayerStopped() ;
   }
}


// Plays the next available stream
void MpStreamQueuePlayer::handlePlayNext()
{
   MpStreamPlayer* pPlayer = NULL ;    // next player to start
   UtlBoolean       bFailed ;           // Has it failed already?

   mSemQueueChange.acquire() ;

   if (mPlayingQueue && mPlayingQueue)
   {
      pPlayer = mPlayingQueue[0].pPlayer ;
      bFailed = mPlayingQueue[0].bFailed ;

      // Inform listeners of playlist advancement
      mSemQueueChange.release() ;
      fireQueuePlayerAdvanced() ;

      if ((pPlayer != NULL) && !bFailed)
      {
         OsStatus status = pPlayer->play(FALSE) ;

         // Handle API Failures (different rendering event failures)
         if (status != OS_SUCCESS)
         {
            setFailedPlayer(pPlayer) ;
         }
      }

      // If we had a valid player it was failed
      if ((pPlayer != NULL) && bFailed)
      {
         handleDequeue() ;
         mpQueueEvent->signal(EVENT_PLAY_NEXT) ;
      }

      mSemWaitSynch.release() ;
   }
   else
   {
      mSemQueueChange.release() ;
   }
}


void MpStreamQueuePlayer::handleDequeue()
{
   MpStreamPlayer* pDeletePlayer = NULL ;
   UtlBoolean       bFireStopped = FALSE ;

   mSemQueueChange.acquire() ;

   if ((mNumPlayingElements > 0) && mPlayingQueue)
   {
      // Remove previously playing entry
      if (mPlayingQueue[0].pPlayer != NULL)
      {
         pDeletePlayer = mPlayingQueue[0].pPlayer ;
         mPlayingQueue[0].pPlayer = NULL ;
      }

      // Update the Queue
      for (int i=0;i<(mNumPlayingElements-1); i++)
      {
         mPlayingQueue[i].pPlayer = mPlayingQueue[i+1].pPlayer ;
         mPlayingQueue[i].bFailed = mPlayingQueue[i+1].bFailed ;
      }
      mNumPlayingElements-- ;
      mPlayingQueue[mNumPlayingElements].pPlayer = NULL ;
      mPlayingQueue[mNumPlayingElements].bFailed = FALSE ;

      if (mNumPlayingElements == 0)
         bFireStopped = TRUE ;
   }
   mSemQueueChange.release() ;

   if (pDeletePlayer != NULL)
   {
      pDeletePlayer->removeListener(this) ;
      pDeletePlayer->stop() ;
      delete pDeletePlayer ;
   }

   mSemWaitSynch.release() ;

   if (bFireStopped)
   {
      // Inform listener that the player has stopped
      fireQueuePlayerStopped() ;
   }
}


UtlBoolean MpStreamQueuePlayer::isPlayingStream(MpPlayer* pPlayer)
{
   UtlBoolean bIsPlaying = FALSE ;

   mSemQueueChange.acquire() ;
   if ((mPlayingQueue != NULL) && (mPlayingQueue[0].pPlayer == pPlayer))
      bIsPlaying = TRUE ;
   mSemQueueChange.release() ;

   return bIsPlaying ;
}


// Removes a failed player from the queue and pushes the queue forward if
// necessary.
void MpStreamQueuePlayer::handleRemoveFailed()
{
   MpStreamPlayer *pDeletePlayer = NULL ;

   UtlBoolean bPlayNext = FALSE ;

   // We need to perform surgery in the play list
   mSemQueueChange.acquire() ;

   UtlBoolean bFound = FALSE ;
   for (int i=0; i<mNumPlayingElements; i++)
   {
      if (!bFound)
      {
         if (mPlayingQueue[i].bFailed)
         {
            // Clean up the player
            pDeletePlayer = mPlayingQueue[i].pPlayer ;
            mPlayingQueue[i].pPlayer = NULL ;
            bFound = TRUE ;

            if (i == 0)
               bPlayNext = TRUE ;
         }
      }
      else
      {
         mPlayingQueue[i-1].pPlayer = mPlayingQueue[i].pPlayer ;
         mPlayingQueue[i-1].bFailed = mPlayingQueue[i].bFailed ;
      }
   }

   if (bFound)
   {
      mNumPlayingElements-- ;
      mPlayingQueue[mNumPlayingElements].pPlayer = NULL ;
      mPlayingQueue[mNumPlayingElements].bFailed = FALSE ;
   }
   mSemQueueChange.release() ;

   if (pDeletePlayer != NULL)
   {
      pDeletePlayer->removeListener(this) ;
      pDeletePlayer->stop() ;
      delete pDeletePlayer;
   }

   // If we no longer have anything to play, then fire a stop
   if (bFound && mNumPlayingElements == 0)
   {
      fireQueuePlayerStopped() ;
      mSemWaitSynch.release() ;
   }
   else
   {
      if (bPlayNext)
         handlePlayNext() ;
   }
}


//Designates the the player as failed
void MpStreamQueuePlayer::setFailedPlayer(MpPlayer* pPlayer)
{
   UtlBoolean bFireRemovedEvent = false ;
   int       i ;

   mSemQueueChange.acquire() ;
   // Mark in playing list
   for (i=0; i<mNumPlayingElements; i++)
   {
      if (mPlayingQueue[i].pPlayer == pPlayer)
      {
         mPlayingQueue[i].bFailed = TRUE ;
         bFireRemovedEvent = true ;
         break ;
      }
   }

   // Mark in to play list
   for (i=0; i<mNumToPlayElements; i++)
   {
      if (mToPlayQueue[i].pPlayer == pPlayer)
      {
         mToPlayQueue[i].bFailed = TRUE ;
         break ;
      }
   }
   mSemQueueChange.release() ;

   if (bFireRemovedEvent)
      mpQueueEvent->signal(EVENT_REMOVE_FAILED) ;
}


// The player has been realized
void MpStreamQueuePlayer::playerRealized(MpPlayerEvent& event)
{
   MpPlayer* pPlayer = event.getPlayer() ;

   // There is a race where we can still receive events while in the
   // destructor of this class.  These events are generally harmless,
   // however, the playing queues have been deleted.
   if (mPlayingQueue == NULL)
   {
      mSemWaitSynch.release() ;  // Wake up anyone waiting
      return ;
   }

   OsStatus status = pPlayer->prefetch(FALSE) ;

   // Handle API Failures (different from rendering event failures)
   if (status != OS_SUCCESS)
   {
      setFailedPlayer(pPlayer) ;
   }
}


// The player's data source has been prefetched
void MpStreamQueuePlayer::playerPrefetched(MpPlayerEvent& event)
{
   MpPlayer* pPlayer = event.getPlayer() ;

   // There is a race where we can still receive events while in the
   // destructor of this class.  These events are generally harmless,
   // however, the playing queues have been deleted.
   if (mPlayingQueue == NULL)
   {
      mSemWaitSynch.release() ;  // Wake up anyone waiting
      return ;
   }

   if (isPlayingStream(pPlayer))
   {
      mpQueueEvent->signal(EVENT_PLAY_NEXT) ;
   }
}

// The player has begun playing
void MpStreamQueuePlayer::playerPlaying(MpPlayerEvent& event)
{
   // Not interesting, we are keying off stop/failed/setup
}

// The player has been paused
void MpStreamQueuePlayer::playerPaused(MpPlayerEvent& event)
{
   // Should never happen, we haven't exposed pause via this player
}


// The player has stopped
void MpStreamQueuePlayer::playerStopped(MpPlayerEvent& event)
{
   MpPlayer* pPlayer = event.getPlayer() ;   // player that caused this event

   // There is a race where we can still receive events while in the
   // destructor of this class.  These events are generally harmless,
   // however, the playing queues have been deleted.
   if (mPlayingQueue == NULL)
   {
      mSemWaitSynch.release() ;  // Wake up anyone waiting
      return ;
   }

   if (event.getState() == PlayerAborted)
   {
      mpQueueEvent->signal(EVENT_RESET) ;
   }
   else
   {
       if (isPlayingStream(pPlayer))
       {
          mpQueueEvent->signal(EVENT_DEQUEUE) ;
          if (isPlaying())
          {
             mpQueueEvent->signal(EVENT_PLAY_NEXT) ;
          }
       }
   }
}


// The player has failed
void MpStreamQueuePlayer::playerFailed(MpPlayerEvent& event)
{
   MpPlayer* pPlayer = event.getPlayer() ;   // player that caused this event

   // There is a race where we can still receive events while in the
   // destructor of this class.  These events are generally harmless,
   // however, the playing queues have been deleted.
   if (mPlayingQueue == NULL)
   {
      mSemWaitSynch.release() ;  // Wake up anyone waiting
      return ;
   }

   if (isPlayingStream(pPlayer))
   {
      mpQueueEvent->signal(EVENT_DEQUEUE) ;
      if (isPlaying())
      {
         mpQueueEvent->signal(EVENT_PLAY_NEXT) ;
      }
   }
}


void MpStreamQueuePlayer::fireQueuePlayerStarted()
{
   OsWriteLock lock(mListenerMutex);
   for (int i=0; i<MAX_PLAYER_LISTENERS; i++)
   {
      if ((mListenerDb[i].inUse) && (mListenerDb[i].pListener))
      {
         mListenerDb[i].pListener->queuePlayerStarted() ;
      }
   }
}


void MpStreamQueuePlayer::fireQueuePlayerStopped()
{
   OsWriteLock lock(mListenerMutex);
   for (int i=0; i<MAX_PLAYER_LISTENERS; i++)
   {
      if ((mListenerDb[i].inUse) && (mListenerDb[i].pListener))
      {
         mListenerDb[i].pListener->queuePlayerStopped() ;
      }
   }
}


void MpStreamQueuePlayer::fireQueuePlayerAdvanced()
{
   OsWriteLock lock(mListenerMutex);
   for (int i=0; i<MAX_PLAYER_LISTENERS; i++)
   {
      if ((mListenerDb[i].inUse) && (mListenerDb[i].pListener))
      {
         mListenerDb[i].pListener->queuePlayerAdvanced() ;
      }
   }
}




/* //////////////////////////// PRIVATE /////////////////////////////////// */

int MpStreamQueuePlayer::expandQueue(struct PlaylistQueue*& pQueue,
                                     int currentLength,
                                     int desiredLength)
{
   int rc = currentLength ;  // return code: queue length after all
   int i ;

   if (desiredLength > currentLength)
   {
      struct PlaylistQueue* pNewQueue = (struct PlaylistQueue*)
             malloc(sizeof(struct PlaylistQueue) * desiredLength) ;

      if (pNewQueue != NULL)
      {
         if (pQueue != NULL)
         {
            // Copy existing entries
            for (i=0; i<currentLength; i++)
            {
               pNewQueue[i].pPlayer = pQueue[i].pPlayer ;
               pNewQueue[i].bFailed = pQueue[i].bFailed ;
            }

            // Clean up memory for existing queue
            free(pQueue) ;
         }

         // Init new Entries
         for (i=currentLength; i<desiredLength; i++)
         {
            pNewQueue[i].pPlayer = NULL ;
            pNewQueue[i].bFailed = FALSE ;
         }

         // Set new Queue
         pQueue = pNewQueue ;
         rc = desiredLength ;
      }
   }

   return rc ;
}


void MpStreamQueuePlayer::swapQueues(struct PlaylistQueue*& pQueue1,
                                     int& queueLength1,
                                     struct PlaylistQueue*& pQueue2,
                                     int& queueLength2)
{
    struct PlaylistQueue* pTempQueue ;

    // Verify the size are equal
    if (queueLength1 > queueLength2)
    {
        queueLength2 = expandQueue(pQueue2, queueLength2, queueLength1) ;
    }
    else if (queueLength1 < queueLength2)
    {
        queueLength1 = expandQueue(pQueue1, queueLength1, queueLength2) ;
    }

    // Swap queues (code above guarentees the same length)
    pTempQueue = pQueue1 ;
    pQueue1 = pQueue2 ;
    pQueue2 = pTempQueue ;
}


/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

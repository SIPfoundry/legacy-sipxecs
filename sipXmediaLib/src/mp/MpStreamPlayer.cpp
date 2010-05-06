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
#include "mp/MpStreamPlayer.h"
#include "mp/MpStreamMsg.h"
#include "os/OsEventMsg.h"
#include "os/OsEvent.h"
#include "os/OsQueuedEvent.h"
#include "os/OsTime.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define MAX_REALIZE_WAIT   60   // Max time to wait for realize to succeed

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpStreamPlayer::MpStreamPlayer(OsMsgQ* pMsg, Url url, int flags, const char *pTarget)
   : OsServerTask("StreamPlay-%d")
   , MpPlayer()
   , mSemStateChange(OsBSem::Q_PRIORITY, OsBSem::EMPTY)
   , mSemStateGuard(OsBSem::Q_PRIORITY, OsBSem::FULL)
   , mbRealized(FALSE)
{
   mpMsgQ = pMsg ;
   mUrl = url ;
   mState = PlayerUnrealized ;
   mHandle = NULL ;
   mSourceType = SourceUrl ;
   mFlags = flags ;
   mpQueueEvent = NULL ;
   mpBuffer = NULL ;
   miLoopCount = 1;
   miTimesAlreadyLooped = 0;
   if (pTarget != NULL)
      mTarget = pTarget ;

   // Complain if the MsgQ is null
   if (mpMsgQ == NULL)
   {
      syslog(FAC_STREAMING, PRI_ERR,
            "Null MsgQ passed to MpSteamPlayer\nurl=%s\nflags=%08X\ntarget=%s",
            url.toString().data(), mFlags, mTarget.data()) ;
   }

}

// Constructs a stream player given a msgq, net buffer, and playing
// flags.
MpStreamPlayer::MpStreamPlayer(OsMsgQ* pMsgQ, UtlString* pBuffer, int flags, const char *pTarget)
   : OsServerTask("MpStreamPlayer-%d")
   , MpPlayer()
   , mSemStateChange(OsBSem::Q_PRIORITY, OsBSem::EMPTY)
   , mSemStateGuard(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
   mpMsgQ = pMsgQ ;
   mState = PlayerUnrealized ;
   mHandle = NULL ;
   mSourceType = SourceBuffer ;
   mFlags = flags ;
   mpQueueEvent = NULL ;
   miLoopCount = 1;
   miTimesAlreadyLooped = 0;
   if (pBuffer)
      mpBuffer = new UtlString(*pBuffer) ;   // To be deleted in StreamBufferDataSource
   else
      mpBuffer = NULL;

   if (pTarget != NULL)
      mTarget = pTarget ;

   // Complain if the MsgQ is null
   if (mpMsgQ == NULL)
   {
      syslog(FAC_STREAMING, PRI_ERR,
            "Null MsgQ passed to MpSteamPlayer\nbuffer=%p\nflags=%08X\ntarget=%s",
            mpBuffer, mFlags, mTarget.data()) ;
   }
}


// Destructor
MpStreamPlayer::~MpStreamPlayer()
{
   destroy() ;

   waitForDestruction() ;
   waitUntilShutDown() ;

   if (mpQueueEvent != NULL)
   {
      delete mpQueueEvent ;
      mpQueueEvent = NULL ;
   }
}

/* ============================ MANIPULATORS ============================== */

// Realizes the player by initiating a connection to the target, allocates
// buffers, etc.
OsStatus MpStreamPlayer::realize(UtlBoolean bBlock /* = TRUE */)
{
   OsStatus status = OS_FAILED ;
   OsEvent eventHandle ;
   intptr_t eventData ;

   // Only proceed if we have a flow graph and the player is unrealized.
   if (getState() == PlayerUnrealized)
   {
      // Create an mpQueueEvent object to signal state changes in from
      // the MpStreamFeeder
      mpQueueEvent =  new OsQueuedEvent(*getMessageQueue(), 0);

      // Realize the stream
      if (mSourceType == SourceUrl)
      {
         if (mpMsgQ != NULL)
         {
            MpStreamMsg msg(MpStreamMsg::STREAM_REALIZE_URL, mTarget, NULL,
                  &eventHandle, mpQueueEvent, mFlags, (intptr_t) new Url(mUrl)) ;
            status = mpMsgQ->send(msg) ;
         }
      }
      else if (mSourceType == SourceBuffer)
      {

         if (mpMsgQ != NULL)
         {
            MpStreamMsg msg(MpStreamMsg::STREAM_REALIZE_BUFFER, mTarget, NULL,
                  &eventHandle, mpQueueEvent, mFlags, (intptr_t) mpBuffer) ;
            status = mpMsgQ->send(msg) ;
         }
      }

      if (status == OS_SUCCESS)
      {
         // Wait for a response
         status = eventHandle.wait(OsTime(MAX_REALIZE_WAIT, 0)) ;
         if (status == OS_SUCCESS)
         {
            if (eventHandle.getEventData(eventData) == OS_SUCCESS)
            {
               mHandle = (StreamHandle) eventData ;
			   if (mHandle != 0)
			      mbRealized = TRUE ;
            }
            else
            {
               mHandle = NULL ;
            }
         }
         else
         {
            mHandle = NULL ;
         }
      }
   }

   if (mHandle == 0)
   {
      mState = PlayerDestroyed ;
      status = OS_FAILED ;
      mSemStateChange.release() ;
   }

   if (status == OS_SUCCESS)
   {
      // Start Server task if successfull
      if (start() == TRUE)
      {
         // Block while waiting for prefetch (if requested)
         if (bBlock)
         {
            while (getState() == PlayerUnrealized)
            {
               mSemStateChange.acquire();
            }
         }
         else
         {
            // Wait for task to startup
            while (!isStarted())
            {
               OsTask::yield() ;
            }
         }
      }
      else
      {
         syslog(FAC_STREAMING, PRI_CRIT, "Failed to create thread for MpStreamPlayer") ;

         // Unable to create thread; attempt to clean up
         status = OS_FAILED ;

         MpStreamMsg msgStop(MpStreamMsg::STREAM_STOP, mTarget, mHandle);
         mpMsgQ->send(msgStop) ;
         MpStreamMsg msgDestroy(MpStreamMsg::STREAM_DESTROY, mTarget, mHandle);
         mpMsgQ->send(msgDestroy) ;

         // YIKES: This is hard to recover from, we don't have a message queue
         // to wait for a response from the lower layers.  If someone deletes
         // this immediately after this call, the lower layers could choke
         // on a now-deleted mpQueueEvent.  There are two options that I can
         // think of: 1) garbage collect the player after some long period of
         // time, 2) block the thread context for some reasonable amount of
         // time.  I'm going with #2 for now...
         OsTask::delay(1000) ;

         mbRealized = FALSE ;
         mState = PlayerDestroyed ;
         mSemStateChange.release() ;
      }
   }

   return status ;
}


// Prefetch enough of the data source to ensure a smooth playback.
OsStatus MpStreamPlayer::prefetch(UtlBoolean bBlock /*= TRUE */)
{
   OsStatus status = OS_FAILED ;

   // Only proceed if we have a flow graph and the player is realized.
   if (getState() == PlayerRealized)
   {
      if (mpMsgQ != NULL)
      {
         MpStreamMsg msg(MpStreamMsg::STREAM_PREFETCH, mTarget, mHandle);
         status = mpMsgQ->send(msg) ;

         // Block while waiting for prefetch (if requested)
         if ((status == OS_SUCCESS) && bBlock)
         {
            while ((getState() == PlayerUnrealized) || (getState() == PlayerRealized))
            {
               mSemStateChange.acquire();
            }
         }
      }
   }

   return status ;
}


// Plays the media stream.
OsStatus MpStreamPlayer::play(UtlBoolean bBlock /*= TRUE*/) //, int iPlayCount /* 1 */)
{
   OsStatus status = OS_FAILED ;
   miTimesAlreadyLooped = 0;

   // Only proceed if we have a flow graph and the player is realized.
   // NOTE: The player doesn't need to be prefetched
   if (  (getState() == PlayerRealized) ||
         (getState() == PlayerPrefetched) ||
         (getState() == PlayerPaused))
   {

      if (mpMsgQ != NULL)
      {
         MpStreamMsg msg(MpStreamMsg::STREAM_PLAY, mTarget, mHandle);
         status = mpMsgQ->send(msg) ;

         // Block while waiting for play to complete(if requested)
         if ((status == OS_SUCCESS) && bBlock)
         {
            while (  (getState() == PlayerRealized)
                  || (getState() == PlayerPrefetched)
                  || (getState() == PlayerPlaying)
                  || (getState() == PlayerPaused))
            {
               mSemStateChange.acquire();
            }
         }
      }
   }

   return status ;
}


// Rewinds a previously played media stream.  In some cases this may result
// in a re-connect/refetch.
OsStatus MpStreamPlayer::rewind(UtlBoolean bBlock /*= TRUE*/)
{
   OsStatus status = OS_FAILED ;

   // Only proceed if we have a flow graph and the player is realized.
   // NOTE: The player doesn't need to be prefetched
   if ((getState() == PlayerStopped) || (getState() == PlayerAborted))
   {
      if (mpMsgQ != NULL)
      {
         MpStreamMsg msg(MpStreamMsg::STREAM_REWIND, mTarget, mHandle);
         status = mpMsgQ->send(msg) ;

         // Block while waiting for play to complete(if requested)
         if ((status == OS_SUCCESS) && bBlock)
         {
            while ((getState() == PlayerStopped) ||
                  (getState() == PlayerAborted) ||
                  (getState() == PlayerRealized))
            {
               mSemStateChange.acquire();
            }

            if ((getState() == PlayerFailed) || (getState() == PlayerDestroyed))
               status = OS_FAILED ;
         }
      }
   }
   return status ;
}

// sets the loop count.
//default is 1. -1 means infinite loop.
// 0 is invalid.
OsStatus MpStreamPlayer::setLoopCount(int iLoopCount)
{
   OsStatus status = OS_SUCCESS ;
   if( iLoopCount == 0 )
      status = OS_FAILED;
   else
      miLoopCount = iLoopCount;
   return status;
}


// Pauses the media stream temporarily.
OsStatus MpStreamPlayer::pause()
{
   OsStatus status = OS_FAILED ;

   // Only proceed if we have a flow graph and the player is prefetched or
   // playing.
   if (  (getState() == PlayerPrefetched) ||
         (getState() == PlayerPlaying))
   {
      if (mpMsgQ != NULL)
      {
         MpStreamMsg msg(MpStreamMsg::STREAM_PAUSE, mTarget, mHandle);
         status = mpMsgQ->send(msg) ;
      }
   }

   return status ;
}


// Stops play the media stream and resources used for buffering and streaming.
OsStatus MpStreamPlayer::stop()
{
   OsStatus status = OS_FAILED ;

   // Make sure we disable looping before sending stop, otherwise it is
   // possible for the stop request to be lost (because of auto-replay)
   miLoopCount = 0 ;

   if (mbRealized && (getState() != PlayerStopped) && (getState() != PlayerAborted))
   {
      if (mpMsgQ != NULL)
      {
         MpStreamMsg msg(MpStreamMsg::STREAM_STOP, mTarget, mHandle);
         status = mpMsgQ->send(msg) ;

         // Block while waiting for play to complete
         if (status == OS_SUCCESS )
         {
            while (  (getState() != PlayerStopped)
                  && (getState() != PlayerFailed)
                  && (getState() != PlayerAborted)
                  && (getState() != PlayerDestroyed))
            {
               mSemStateChange.acquire();
            }
         }
      }
   }
   else
   {
      status = OS_SUCCESS ;   // already stopped
   }

   return status ;
}


OsStatus MpStreamPlayer::destroy()
{
   OsStatus status = OS_SUCCESS ; // Assume since this is non-blocking.  Only
                                  // a failure to send will cause error

   // Make sure we disable looping before sending stop, otherwise it is
   // possible for the stop request to be lost (because of auto-replay)
   miLoopCount = 0 ;

   if ((mpMsgQ != NULL) && (mHandle != NULL))
   {
      int iState = getState() ;

      if (  mbRealized
	        && (iState != PlayerStopped)
            && (iState != PlayerAborted)
            && (iState != PlayerDestroyed)  )
      {
         MpStreamMsg msgStop(MpStreamMsg::STREAM_STOP, mTarget, mHandle);
         status = mpMsgQ->send(msgStop) ;
      }

      if (	mbRealized
	        && (iState != PlayerDestroyed)
		    && (status == OS_SUCCESS))
      {
         MpStreamMsg msgDestroy(MpStreamMsg::STREAM_DESTROY, mTarget, mHandle);
         status = mpMsgQ->send(msgDestroy) ;
      }
   }

   return status ;
}


// Blocks until the the lower layer stream player is destroyed
void MpStreamPlayer::waitForDestruction()
{
   // Wait for player to shutdown
   if (mbRealized)
   {
      while (getState() != PlayerDestroyed)
      {
         mSemStateChange.acquire() ;
      }
   }
   else
   {
      setState(PlayerDestroyed) ;
   }
}

/* ============================ ACCESSORS ================================= */

// Gets the player state
OsStatus MpStreamPlayer::getState(PlayerState& state)
{
   mSemStateGuard.acquire() ;
   state = mState ;
   mSemStateGuard.release() ;

   return OS_SUCCESS ;
}


// Gets the source type for this player (SourceUrl or SourceBuffer)
OsStatus MpStreamPlayer::getSourceType(int& iType) const
{
   iType = mSourceType ;

   return OS_SUCCESS ;
}


// Gets the url if the source type is a SourceUrl
OsStatus MpStreamPlayer::getSourceUrl(Url& url) const
{
   OsStatus status = OS_INVALID_STATE ;

   if (mSourceType == SourceUrl)
   {
      url = mUrl ;
      status = OS_SUCCESS ;
   }

   return status ;
}


// Gets the source buffer if the source type is a SourceBuffer
OsStatus MpStreamPlayer::getSourceBuffer(UtlString*& pBuffer)  const
{
   OsStatus status = OS_INVALID_STATE ;

   if (mSourceType == SourceBuffer)
   {
      pBuffer = mpBuffer ;
      status = OS_SUCCESS ;
   }

   return status ;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Copy constructor not implemented

// Assignment operator not implemented

#ifdef MP_STREAM_DEBUG /* [ */
const char* getFeederEventString(int iEvent)
{
   const char* szRC = "ERROR_UNKNOWN" ;

   switch (iEvent)
   {
      case FeederRealizedEvent:
         szRC = "FeederRealizedEvent" ;
         break ;
      case FeederPrefetchedEvent:
         szRC = "FeederPrefetchedEvent" ;
         break ;
      case FeederRenderingEvent:
         szRC = "FeederRenderingEvent" ;
         break ;
      case FeederStoppedEvent:
         szRC = "FeederStoppedEvent" ;
         break ;
      case FeederFailedEvent:
         szRC = "FeederFailedEvent" ;
         break ;
      case FeederStreamPlayingEvent:
         szRC = "FeederStreamPlayingEvent" ;
         break ;
      case FeederStreamPausedEvent:
         szRC = "FeederStreamPausedEvent" ;
         break ;
      case FeederStreamAbortedEvent:
         szRC = "FeederStreamAbortedEvent" ;
         break ;
      case FeederStreamStoppedEvent:
         szRC = "FeederStreamStoppedEvent" ;
         break ;
      case FeederStreamDestroyedEvent:
         szRC = "FeederStreamDestroyedEvent" ;
         break ;
   }
   return szRC ;
}

#endif


// Handles OS server task events/messages
UtlBoolean MpStreamPlayer::handleMessage(OsMsg& rMsg)
{
   switch (rMsg.getMsgType())
   {
      case OsMsg::OS_EVENT:
         OsEventMsg* pMsg = (OsEventMsg*) &rMsg ;
         intptr_t status ;
         if (pMsg->getEventData(status) == OS_SUCCESS)
         {
#ifdef MP_STREAM_DEBUG /* [ */
            osPrintf("MpStreamPlayer(%08X): received event: %s \n",
                  this, getFeederEventString(status)) ;
#endif /* MP_STREAM_DEBUG ] */

            switch (status)
            {
               case FeederRealizedEvent:
                  setState(PlayerRealized) ;
                  break ;
               case FeederPrefetchedEvent:
                  if (getState() != PlayerPlaying)
                     setState(PlayerPrefetched) ;
                  break ;
               case FeederRenderingEvent:
                  break ;
               case FeederStoppedEvent:
                  break ;
               case FeederFailedEvent:
                  setState(PlayerFailed) ;
                  break ;
               case FeederStreamPlayingEvent:
                  setState(PlayerPlaying) ;
                  break ;
               case FeederStreamPausedEvent:
                  setState(PlayerPaused) ;
                  break ;
               case FeederStreamAbortedEvent:
                  if (getState() != PlayerDestroyed)
                     setState(PlayerAborted) ;
                  break;
               case FeederStreamStoppedEvent:
                  //rewind and play again if miLoopCount >1 or -1
                  //flag is on.
                  if ( (miLoopCount > 1 || miLoopCount ==-1) &&
                       (getState() != PlayerAborted) &&
                       ((miLoopCount ==-1)?true:miTimesAlreadyLooped < miLoopCount) )
                  {
                     MpStreamMsg msg(MpStreamMsg::STREAM_REWIND, mTarget, mHandle);
                     status = mpMsgQ->send(msg) ;
                     MpStreamMsg msg2(MpStreamMsg::STREAM_PLAY, mTarget, mHandle);
                     mpMsgQ->send(msg2) ;
                     miTimesAlreadyLooped++;
                  }
                  else
                  {
                     if ((getState() != PlayerAborted) && (getState() != PlayerDestroyed))
                     {
                        setState(PlayerStopped) ;
                     }
                  }
                  break ;

               case FeederStreamDestroyedEvent:
                  setState(PlayerDestroyed) ;
                  break ;
            }
         }
         break ;
   }

   return TRUE ;
}


// Sets the internal state for this resource
void MpStreamPlayer::setState(PlayerState iState)
{
   UtlBoolean bShouldFire = FALSE ;

   mSemStateGuard.acquire() ;
   if (isValidStateChange(mState, iState))
   {
      bShouldFire = TRUE ;
#ifdef MP_STREAM_DEBUG /* [ */
      osPrintf("MpStreamPlayer::setState changed from %s to %s.\n\n",
			getEventString(mState), getEventString(iState)) ;
#endif /* ] */

      mState = iState ;
   }
#ifdef MP_STREAM_DEBUG /* [ */
   else if (mState != iState)
   {
      osPrintf("** WARNING MpStreamPlayer(%08X): invalid state change (%s to %s)\n",
            this, getEventString(mState), getEventString(iState)) ;
   }
#endif /* MP_STREAM_DEBUG ] */

   mSemStateGuard.release() ;

   if (bShouldFire)
      fireEvent(iState) ;
   mSemStateChange.release() ;
}


PlayerState MpStreamPlayer::getState()
{
   PlayerState state ;
   getState(state) ;

   return state ;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

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
#include "mp/MpPlayer.h"
#include "mp/MpPlayerEvent.h"
#include "mp/MpPlayerListener.h"
#include "mp/StreamDefs.h"
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpPlayer::MpPlayer()
   : mListenerLock(OsMutex::Q_FIFO)
{
   for (int i=0; i<MAX_PLAYER_LISTENERS; i++)
   {
      mListenerDb[i].inUse = FALSE ;
      mListenerDb[i].pListener = NULL ;
      mListenerDb[i].pUserData = NULL ;
   }
}

// Destructor
MpPlayer::~MpPlayer()
{
}


/* ============================ MANIPULATORS ============================== */

// Adds a player listener to receive notifications when this player changes
// state
OsStatus MpPlayer::addListener(MpPlayerListener* pListener, void* pUserData)
{
   OsLock lock(mListenerLock) ;
   OsStatus status = OS_LIMIT_REACHED ;

#ifdef MP_STREAM_DEBUG /* [ */
   osPrintf("MpPlayer addListener (player=%08X, listener=%08X, data=%08X)\n",
         this, pListener, pUserData) ;
#endif /* MP_STREAM_DEBUG ] */

   for (int i=0; i<MAX_PLAYER_LISTENERS; i++)
   {
      if (!mListenerDb[i].inUse)
      {
         mListenerDb[i].inUse = TRUE ;
         mListenerDb[i].pListener = pListener ;
         mListenerDb[i].pUserData = pUserData ;

         status = OS_SUCCESS ;
         break;
      }
   }

   return status ;
}


// Removes a previously added player listener.  This listener will cease to
// receive state change notifications.
OsStatus MpPlayer::removeListener(MpPlayerListener* pListener)
{
   OsLock lock(mListenerLock) ;
   OsStatus status = OS_NOT_FOUND ;

#ifdef MP_STREAM_DEBUG /* [ */
   osPrintf("MpPlayer removeListener (player=%08X, listener=%08X)\n",
         this, pListener) ;
#endif /* MP_STREAM_DEBUG ] */


   for (int i=0; i<MAX_PLAYER_LISTENERS; i++)
   {
      if ((mListenerDb[i].inUse) && (mListenerDb[i].pListener == pListener))
      {
         mListenerDb[i].inUse = FALSE ;
         mListenerDb[i].pListener = NULL ;
         mListenerDb[i].pUserData = NULL ;
         status = OS_SUCCESS ;
      }
   }
   return status ;
}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* ============================ TESTING =================================== */

#ifdef MP_STREAM_DEBUG /* [ */
const char* MpPlayer::getEventString(PlayerState event)
{
   switch (event)
   {
      case PlayerUnrealized:
         return "PlayerUnrealized" ;
         break ;
      case PlayerRealized:
         return "PlayerRealized" ;
         break ;
      case PlayerPrefetched:
         return "PlayerPrefetched" ;
         break ;
      case PlayerPlaying:
         return "PlayerPlaying" ;
         break ;
      case PlayerPaused:
         return "PlayerPaused" ;
         break ;
      case PlayerStopped:
         return "PlayerStopped" ;
         break ;
      case PlayerAborted:
         return "PlayerAborted" ;
         break ;
      case PlayerFailed:
         return "PlayerFailed" ;
         break ;
      case PlayerDestroyed:
         return "PlayerDestroyed" ;
         break ;
      // default:
         // return "PlayerUnknown" ;
         // break ;
   }
}
#endif /* MP_STREAM_DEBUG ] */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Delievers a MpPlayerEvent to all interested parties.
void MpPlayer::fireEvent(PlayerState state)
{
   OsLock lock(mListenerLock) ;

#ifdef MP_STREAM_DEBUG /* [ */
   osPrintf("MpPlayer: %s\n", getEventString(state)) ;
#endif /* MP_STREAM_DEBUG ] */

   // Notify Listeners
   for (int i=0; i<MAX_PLAYER_LISTENERS; i++)
   {
      if ((mListenerDb[i].inUse) && (mListenerDb[i].pListener))
      {
         MpPlayerEvent event(this, mListenerDb[i].pUserData, state) ;

         switch (state)
         {
            case PlayerUnrealized:
            case PlayerDestroyed:
               break ;
            case PlayerRealized:
               mListenerDb[i].pListener->playerRealized(event) ;
               break ;
            case PlayerPrefetched:
               mListenerDb[i].pListener->playerPrefetched(event) ;
               break ;
            case PlayerPlaying:
               mListenerDb[i].pListener->playerPlaying(event) ;
               break ;
            case PlayerPaused:
               mListenerDb[i].pListener->playerPaused(event) ;
               break ;
            case PlayerStopped:
            case PlayerAborted:
               mListenerDb[i].pListener->playerStopped(event) ;
               break ;
            case PlayerFailed:
               mListenerDb[i].pListener->playerFailed(event) ;
               break ;
         }
      }
   }
}

// Is the transition from oldState to newState valid?
UtlBoolean MpPlayer::isValidStateChange(PlayerState oldState, PlayerState newState)
{
   UtlBoolean bValid = TRUE ;

   if ((oldState == newState) || (oldState == PlayerDestroyed))
      bValid = FALSE ;

   return bValid ;
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#include "osbprompt_playerlistener.h"
#include "os/OsLock.h"
#include "os/OsSysLog.h"
#ifdef DMALLOC
#include <dmalloc.h>
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OSBPlayerListener::OSBPlayerListener(int iTimeoutSecs)
      : mSemStateChange(OsBSem::Q_PRIORITY, OsBSem::EMPTY)
      , mSemGuard(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
   mRef = 1;
   miTimeoutSec = iTimeoutSecs  ;
   mEventQLen = 0;
   for (int i = 0; i < MAX_NUM_LISTENERS; i++)
      mPromptEvents[i] = 0;

   clearState();
}

OSBPlayerListener::~OSBPlayerListener()
{
   if (mRef > 0)
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                    "SBPlayerListener::destructor mRef=%d, - ERROR trying to delete a not released listener %p!\n",
                    mRef, this);
   }
}

int OSBPlayerListener::addRef()
{
   OsLock lock(mSemGuard);
   if (mRef <= 0)
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                    "SBPlayerListener::addRef mRef=%d, - ERROR trying to use a released listener %p!\n",
                    mRef, this);
   }
   mRef++;
   return mRef;
}

int OSBPlayerListener::release()
{
   OsLock lock(mSemGuard);
   mRef--;
   if (mRef < 0)
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                    "SBPlayerListener::release mRef=%d, - ERROR trying to release a released listener %p!\n",
                    mRef, this);
   }

   return mRef;
}

void OSBPlayerListener::clearState()
{
   for (int i=0; i<16; i++)
      mStates[i] = FALSE ;
}

UtlBoolean OSBPlayerListener::isState(PlayerState state)
{
   UtlBoolean bRetrieved = FALSE;
   OsLock lock(mSemGuard) ;

   bRetrieved = mStates[state] ;

   return bRetrieved;
}

void OSBPlayerListener::addListeningEvent(OsQueuedEvent* promptEvent)
{
   OsLock lock(mSemGuard);
   int found = 0;
   for (int i = 0; i < mEventQLen; i++)
   {
      if (mPromptEvents[i] == promptEvent)
      {
         found = 1;
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                       "SBPlayerListener::addListeningEvent %p already added for listener %p!\n",
                       promptEvent, this);
         break;
      }
   }
   if (!found && mEventQLen < MAX_NUM_LISTENERS)
   {
      mPromptEvents[mEventQLen] = promptEvent;
      mEventQLen++;
   }
}


void OSBPlayerListener::removeListeningEvent(OsQueuedEvent* promptEvent)
{
   OsLock lock(mSemGuard) ;
   for (int i = 0; i < mEventQLen; i++)
   {
      if (mPromptEvents[i] == promptEvent)
      {
         for (int j = i; j < (mEventQLen - 1); j++)
         {
            mPromptEvents[j] = mPromptEvents[j + 1];
         }
         mEventQLen--;
         mPromptEvents[mEventQLen] = NULL;
         break;
      }
   }
}

UtlBoolean OSBPlayerListener::waitForState(PlayerState state)
{
   OsLock lock(mSemGuard);
   UtlBoolean bRetrieved = mStates[state];

   while (bRetrieved == FALSE)
   {
      OsStatus status = mSemStateChange.acquire(OsTime(miTimeoutSec, 0)) ;
      if (status == OS_SUCCESS)
      {
         bRetrieved = mStates[state] ;
         mSemStateChange.release();
      }
      else
      {
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                       "OSBPlayerListener::waitForState - Timeout waiting for state %d\n", state) ;
         break ;
      }
   }

   return bRetrieved ;
}

void OSBPlayerListener::playerPlaying(MpPlayerEvent& event)
{
   OsLock lock(mSemGuard);

   clearState();
   mStates[PlayerPlaying] = TRUE ;

   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "PlayerPlaying\n");

   for (int i = 0; i < mEventQLen; i++)
   {
      if (mPromptEvents[i])
         mPromptEvents[i]->signal(((PlayerPlaying << 16) | 1));
   }
   mSemStateChange.release() ;
}

void OSBPlayerListener::playerStopped(MpPlayerEvent& event)
{
   OsLock lock(mSemGuard);

   clearState();
   mStates[PlayerStopped] = TRUE ;

   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "PlayerStopped\n");

   for (int i = 0; i < mEventQLen; i++)
   {
      if (mPromptEvents[i])
         mPromptEvents[i]->signal(((PlayerStopped << 16) | 1));
   }
   mSemStateChange.release() ;
}

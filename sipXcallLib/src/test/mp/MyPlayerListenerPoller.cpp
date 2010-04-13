//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include "test/mp/MyPlayerListenerPoller.h"

#include "os/OsBSem.h"
#include "os/OsDefs.h"
#include "mp/MpMisc.h"

// Class used to wait for state changes
MyPlayerListenerPoller::MyPlayerListenerPoller(int iTimeoutSecs)
  : mSemStateChange(OsBSem::Q_PRIORITY, OsBSem::EMPTY)
  , mSemGuard(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
  miTimeoutSec = iTimeoutSecs  ;
  clearState();
}

MyPlayerListenerPoller::~MyPlayerListenerPoller()
{
}

void MyPlayerListenerPoller::clearState()
{
  mSemGuard.acquire() ;
  for (int i=0; i<MAX_STATES; i++)
     mStates[i] = FALSE ;
  mSemGuard.release() ;
}

UtlBoolean MyPlayerListenerPoller::waitForState(PlayerState state)
{
  mSemGuard.acquire() ;
  UtlBoolean bRetrieved = mStates[state] ;
  mSemGuard.release() ;

  while (bRetrieved == FALSE)
  {
     OsStatus status = mSemStateChange.acquire(OsTime(miTimeoutSec, 0)) ;
     if (status == OS_SUCCESS)
     {
        mSemGuard.acquire() ;
        bRetrieved = mStates[state] ;
        mSemGuard.release() ;
     }
     else
     {
        osPrintf("Timeout waiting for state %d\n", state) ;
        for (int i=0; i<MAX_STATES; i++)
        {
           osPrintf("\tState %2d: %d\n", i, mStates[i]) ;
        }
        break ;
     }
  }

  return bRetrieved ;
}

void MyPlayerListenerPoller::playerRealized(MpPlayerEvent& event)
{
  mStates[event.getState()] = TRUE ;
  mSemStateChange.release() ;
}

void MyPlayerListenerPoller::playerPrefetched(MpPlayerEvent& event)
{
  mStates[event.getState()] = TRUE ;
  mSemStateChange.release() ;
}

void MyPlayerListenerPoller::playerPlaying(MpPlayerEvent& event)
{
  mStates[event.getState()] = TRUE ;
  mSemStateChange.release() ;
}

void MyPlayerListenerPoller::playerPaused(MpPlayerEvent& event)
{
  mStates[event.getState()] = TRUE ;
  mSemStateChange.release() ;
}

void MyPlayerListenerPoller::playerStopped(MpPlayerEvent& event)
{
  mStates[event.getState()] = TRUE ;
  mSemStateChange.release() ;
}

void MyPlayerListenerPoller::playerFailed(MpPlayerEvent& event)
{
  mStates[event.getState()] = TRUE ;
  mSemStateChange.release() ;
}

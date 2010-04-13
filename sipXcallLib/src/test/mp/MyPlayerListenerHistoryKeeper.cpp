//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include "test/mp/MyPlayerListenerHistoryKeeper.h"
#include "mp/MpPlayerListener.h"
#include "mp/MpPlayerEvent.h"
#include "os/OsDefs.h"
#include "os/OsTask.h"

MyPlayerListenerHistoryKeeper::~MyPlayerListenerHistoryKeeper()
{
}

void MyPlayerListenerHistoryKeeper::playerRealized(MpPlayerEvent& event)
{
  char szString[64] ;
  sprintf(szString, "%d-%p,", event.getState(), event.getUserData()) ;
  mHistory.append(szString) ;
}


void MyPlayerListenerHistoryKeeper::playerPrefetched(MpPlayerEvent& event)
{
  char szString[64] ;
  sprintf(szString, "%d-%p,", event.getState(), event.getUserData()) ;
  mHistory.append(szString) ;
}


void MyPlayerListenerHistoryKeeper::playerPlaying(MpPlayerEvent& event)
{
  char szString[64] ;
  sprintf(szString, "%d-%p,", event.getState(), event.getUserData()) ;
  mHistory.append(szString) ;
}


void MyPlayerListenerHistoryKeeper::playerPaused(MpPlayerEvent& event)
{
  char szString[64] ;
  sprintf(szString, "%d-%p,", event.getState(), event.getUserData()) ;
  mHistory.append(szString) ;
}

void MyPlayerListenerHistoryKeeper::playerStopped(MpPlayerEvent& event)
{
  char szString[64] ;
  sprintf(szString, "%d-%p,", event.getState(), event.getUserData()) ;
  mHistory.append(szString) ;
}

void MyPlayerListenerHistoryKeeper::playerFailed(MpPlayerEvent& event)
{
  char szString[64] ;
  sprintf(szString, "%d-%p,", event.getState(), event.getUserData()) ;
  mHistory.append(szString) ;
}

const char* MyPlayerListenerHistoryKeeper::getHistory()
{
  return mHistory.data() ;
}


UtlBoolean MyPlayerListenerHistoryKeeper::matchesHistory(void* userData, int* pPlayerStates)
{
  // HACK: We are adding two different listeners and the order that events
  //       are fired to listeners matters.  This delay allows for events
  //       to be delievered to both the state poller and event recorder.
  OsTask::delay(100) ;


  mExpectedHistory.remove(0) ;

  while (*pPlayerStates != -1)
  {
     char szString[256] ;
     sprintf(szString, "%d-%p,", *pPlayerStates, userData) ;
     mExpectedHistory.append(szString) ;

     pPlayerStates++ ;
  }

  if (mExpectedHistory != mHistory)
  {
     osPrintf("Expected History: %s\n", mExpectedHistory.data()) ;
     osPrintf("  Actual History: %s\n", mHistory.data()) ;

     return FALSE ;
  }
  else
  {
     return TRUE ;
  }
}

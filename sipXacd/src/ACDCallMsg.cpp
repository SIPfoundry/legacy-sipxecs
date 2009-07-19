//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlSListIterator.h>
#include "ACDCallMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallMsg::ACDCallMsg
//
//  SYNOPSIS:
//
//  DESCRIPTION: Constructor for SET_QUEUE message
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDCallMsg::ACDCallMsg(eACDCallMsgSubTypes type, ACDQueue* pRequestingQueue, int timeout)
: OsMsg(USER_START, type)
{
   mpRequestingQueue = pRequestingQueue;
   mTimeout          = timeout;

   mhCallHandle = SIPX_CALL_NULL;
   mCallEvent   = 0;
   mCallCause   = 0;
   mConnectionScheme = 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallMsg::ACDCallMsg
//
//  SYNOPSIS:
//
//  DESCRIPTION: Constructor for UPDATE_STATE message
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDCallMsg::ACDCallMsg(eACDCallMsgSubTypes type, SIPX_CALL hCallHandle, int event, int cause)
: OsMsg(USER_START, type)
{
   mhCallHandle = hCallHandle;
   mCallEvent   = event;
   mCallCause   = cause;

   mpRequestingQueue = NULL;
   mConnectionScheme = 0;
   mTimeout = 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallMsg::ACDCallMsg
//
//  SYNOPSIS:
//
//  DESCRIPTION: Constructor for ROUTE_CALL message
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDCallMsg::ACDCallMsg(eACDCallMsgSubTypes type, UtlSList* pTargetAgentList, int connectionScheme, int timeout)
: OsMsg(USER_START, type)
{
   mConnectionScheme = connectionScheme;
   mTimeout          = timeout;

   mhCallHandle = SIPX_CALL_NULL;
   mCallEvent   = 0;
   mCallCause   = 0;
   mpRequestingQueue = NULL;

   // Make a copy of the UtlSList
   UtlSListIterator listIterator(*pTargetAgentList);
   UtlContainable* pElement;
   while ((pElement = listIterator()) != NULL) {
      mTargetAgentList.append(pElement);
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallMsg::ACDCallMsg
//
//  SYNOPSIS:
//
//  DESCRIPTION: Constructor for ROUTE_ADD message
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDCallMsg::ACDCallMsg(eACDCallMsgSubTypes type, ACDAgent* pAgentRef)
: OsMsg(USER_START, type)
{
   mhCallHandle = SIPX_CALL_NULL;
   mCallEvent   = 0;
   mCallCause   = 0;
   mpRequestingQueue = NULL;
   mConnectionScheme = 0;
   mTimeout = 0;
   mpTargetAgent = pAgentRef;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallMsg::ACDCallMsg
//
//  SYNOPSIS:
//
//  DESCRIPTION: Constructor for ANSWER_CALL message
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDCallMsg::ACDCallMsg(eACDCallMsgSubTypes type, UtlString* pAudio, bool bargeIn)
: OsMsg(USER_START, type)
{
   mhCallHandle = SIPX_CALL_NULL;
   mCallEvent   = 0;
   mCallCause   = 0;
   mpRequestingQueue = NULL;
   mConnectionScheme = 0;
   mTimeout = 0;
   mAudio1  = pAudio->data();
   mBargeIn = bargeIn;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallMsg::ACDCallMsg
//
//  SYNOPSIS:
//
//  DESCRIPTION: Constructor for PLAY_AUDIO message
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDCallMsg::ACDCallMsg(eACDCallMsgSubTypes type, UtlString* pQueueAudio, int queueAudioInterval, UtlString* pBackgroundAudio)
: OsMsg(USER_START, type)
{
   mhCallHandle = SIPX_CALL_NULL;
   mCallEvent   = 0;
   mCallCause   = 0;
   mpRequestingQueue = NULL;
   mConnectionScheme = 0;
   mAudio1  = pQueueAudio->data();
   mTimeout = queueAudioInterval;
   mAudio2  = pBackgroundAudio->data();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallMsg::ACDCallMsg
//
//  SYNOPSIS:
//
//  DESCRIPTION: Constructor for STOP_AUDIO & ABORT_ROUTE messages
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDCallMsg::ACDCallMsg(eACDCallMsgSubTypes type)
: OsMsg(USER_START, type)
{
   mhCallHandle = SIPX_CALL_NULL;
   mCallEvent   = 0;
   mCallCause   = 0;
   mpRequestingQueue = NULL;
   mConnectionScheme = 0;
   mTimeout = 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallMsg::ACDCallMsg
//
//  SYNOPSIS:
//
//  DESCRIPTION: Constructor for DROP_CALL message
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDCallMsg::ACDCallMsg(eACDCallMsgSubTypes type, int terminationToneDuration, UtlString* pTerminationAudio)
: OsMsg(USER_START, type)
{
   mhCallHandle = SIPX_CALL_NULL;
   mCallEvent   = 0;
   mCallCause   = 0;
   mpRequestingQueue = NULL;
   mConnectionScheme = 0;
   mTimeout = terminationToneDuration;
   mAudio1  = pTerminationAudio->data();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallMsg::ACDCallMsg
//
//  SYNOPSIS:
//
//  DESCRIPTION: Copy constructor
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDCallMsg::ACDCallMsg(const ACDCallMsg& rACDCallMsg)
: OsMsg(rACDCallMsg)
{
   mhCallHandle = SIPX_CALL_NULL;
   mCallEvent   = 0;
   mCallCause   = 0;
   mpRequestingQueue = NULL;
   mConnectionScheme = 0;
   mTimeout = 0;

   if (getMsgSubType() == SET_QUEUE) {
      mpRequestingQueue = rACDCallMsg.mpRequestingQueue;
      mTimeout          = rACDCallMsg.mTimeout;
   }
   else if (getMsgSubType() == UPDATE_STATE) {
      mhCallHandle = rACDCallMsg.mhCallHandle;
      mCallEvent   = rACDCallMsg.mCallEvent;
      mCallCause   = rACDCallMsg.mCallCause;
   }
   else if (getMsgSubType() == ROUTE_CALL) {
      mConnectionScheme = rACDCallMsg.mConnectionScheme;
      mTimeout          = rACDCallMsg.mTimeout;
      // Make a copy of the UtlSList
      UtlSListIterator listIterator(rACDCallMsg.mTargetAgentList);
      UtlContainable* pElement;
      while ((pElement = listIterator()) != NULL) {
         mTargetAgentList.append(pElement);
      }
   }
   else if (getMsgSubType() == ROUTE_ADD) {
      mpTargetAgent = rACDCallMsg.mpTargetAgent;
   }
   else if (getMsgSubType() == ANSWER_CALL) {
      mAudio1  = rACDCallMsg.mAudio1;
      mBargeIn = rACDCallMsg.mBargeIn;
   }
   else if (getMsgSubType() == PLAY_AUDIO) {
      mAudio1  = rACDCallMsg.mAudio1;
      mTimeout = rACDCallMsg.mTimeout;
      mAudio2  = rACDCallMsg.mAudio2;
   }
   else if (getMsgSubType() == DROP_CALL) {
      mTimeout = rACDCallMsg.mTimeout;
      mAudio1  = rACDCallMsg.mAudio1;
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallMsg::createCopy
//
//  SYNOPSIS:
//
//  DESCRIPTION: Create a copy of this msg object (which may be of a derived type)
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

OsMsg* ACDCallMsg::createCopy(void) const
{
   return new ACDCallMsg(*this);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallMsg::~ACDCallMsg
//
//  SYNOPSIS:
//
//  DESCRIPTION: Destructor
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDCallMsg::~ACDCallMsg()
{
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

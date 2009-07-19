//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ACDCallMsg_h_
#define _ACDCallMsg_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsSysLog.h>
#include <os/OsMsg.h>
#include <utl/UtlString.h>
#include <utl/UtlSList.h>
#include <tapi/sipXtapi.h>
#include "ACDQueue.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class ACDAgent;

//
// ACDCallMsg
//
class ACDCallMsg : public OsMsg {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum eACDCallMsgSubTypes {
      SET_QUEUE        = 1,
      UPDATE_STATE     = 2,
      ROUTE_CALL       = 3,
      ROUTE_ADD        = 4,
      ANSWER_CALL      = 5,
      PLAY_AUDIO       = 6,
      STOP_AUDIO       = 7,
      ABORT_ROUTE      = 8,
      DROP_CALL        = 9,
      CALL_PICKUP      = 10
   };

/* ============================ CREATORS ================================== */

   // Constructor for SET_QUEUE message
   ACDCallMsg(eACDCallMsgSubTypes type, ACDQueue* pRequestingQueue, int timeout);

   // Constructor for UPDATE_STATE message
   ACDCallMsg(eACDCallMsgSubTypes type, SIPX_CALL hCallHandle, int event, int cause);

   // Constructor for ROUTE_CALL message
   ACDCallMsg(eACDCallMsgSubTypes type, UtlSList* pTargetAgentList, int connectionScheme, int timeout);

   // Constructor for ROUTE_ADD message and CALL_PICKUP message
   ACDCallMsg(eACDCallMsgSubTypes type, ACDAgent* pAgentRef);

   // Constructor for ANSWER_CALL message
   ACDCallMsg(eACDCallMsgSubTypes type, UtlString* pAudio, bool bargeIn);

   // Constructor for PLAY_AUDIO message
   ACDCallMsg(eACDCallMsgSubTypes type, UtlString* pQueueAudio, int queueAudioInterval, UtlString* pBackgroundAudio);

   // Constructor for STOP_AUDIO & ABORT_ROUTE messages
   ACDCallMsg(eACDCallMsgSubTypes type);

   // Constructor for DROP_CALL message
   ACDCallMsg(eACDCallMsgSubTypes type, int terminationToneDuration, UtlString* pTerminationAudio);

   // Copy constructor
   ACDCallMsg(const ACDCallMsg& rACDCallMsg);

   // Create a copy of this msg object (which may be of a derived type)
   OsMsg* createCopy(void) const;

   // Destructor
   virtual ~ACDCallMsg();

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

   // Get the associated ACDCall handle
   SIPX_CALL getCallHandle(void) const { return mhCallHandle; }

   // Get the associated ACDCall event type
   int getCallEvent(void) const { return mCallEvent; }

   // Get the associated ACDCall cause type
   int getCallCause(void) const { return mCallCause; }

   // Get the associated ACDCall requesting queue for this message
   ACDQueue* getRequestingQueue(void) { return mpRequestingQueue; }

   // Get the associated ACDCall target agent list
   UtlSList* getTargetAgentList(void) { return &mTargetAgentList; }

   // Get the associated ACDCall target agent
   ACDAgent* getTargetAgent(void) { return mpTargetAgent; }

   // Get the associated ACDCall call connection scheme
   int getConnectionScheme(void) const { return mConnectionScheme; }

   // Get the associated ACDCall call timeout
   int getTimeout(void) const { return mTimeout; }

   // Get the associated ACDCall Audio
   UtlString* getWelcomeAudio(void) { return &mAudio1; }

   // Get the associated ACDCall barge-in flag
   bool getBargeIn(void) { return mBargeIn; }

   // Get the associated ACDCall Audio
   UtlString* getQueueAudio(void) { return &mAudio1; }

   // Get the associated ACDCall Queue Audio Interval
   int getQueueAudioInterval(void) { return mTimeout; }

   // Get the associated ACDCall Audio
   UtlString* getBackgroundAudio(void) { return &mAudio2; }

   // Get the associated ACDCall Audio
   UtlString* getTerminationAudio(void) { return &mAudio1; }

   // Get the associated ACDCall Termination Tone Duration
   int getTerminationToneDuration(void) { return mTimeout; }

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   SIPX_CALL mhCallHandle;
   int       mCallEvent;
   int       mCallCause;
   ACDQueue* mpRequestingQueue;
   UtlSList  mTargetAgentList;
   ACDAgent* mpTargetAgent;
   int       mConnectionScheme;
   int       mTimeout;
   bool      mBargeIn;
   UtlString mAudio1;
   UtlString mAudio2;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _ACDCallMsg_h_

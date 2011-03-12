//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ACDRtRecord_h
#define _ACDRtRecord_h

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <os/OsMutex.h>

// DEFINES
#define RT_LOG_FILE               "sipxacd_events.log"
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class ACDAgent;
class ACDCall;

class ACDRtRecord {

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   enum eEventTypes {
      EVENT_CALL = 1,
      EVENT_AGENT,
      EVENT_TRANSFER,
      EVENT_ACD,
      EVENT_MAX = EVENT_ACD,
      EVENT_MIN = EVENT_CALL
   };
   enum eCallEvents {
      ENTER_QUEUE = 1,
      PICK_UP,
      TERMINATE,
      CEVENT_MAX = TERMINATE
   };
   enum eAgentEvents {
      SIGNED_IN_AGENT = 1,
      SIGNED_OUT_AGENT,
      AEVENT_MAX = SIGNED_OUT_AGENT
   };
   enum eAcdEvents {
      START_ACD  = 1,
      STOP_ACD,
      ACDEVENT_MAX = STOP_ACD
   };
   enum eTransferEvents {
      TRANSFER  = 1,
      TERMINATE_TRANSFER,
      TRANSFEREVENT_MAX = TERMINATE_TRANSFER
   };

/* ============================ CREATORS ================================== */

   ACDRtRecord(UtlString& rLogDir);

   // Destructor
   ~ACDRtRecord();

/* ============================ MANIPULATORS ============================== */

   void appendCallEvent(int event,
                        UtlString queueString,
                        ACDCall* pCallRef,
                        bool AgentRec = FALSE);

   void appendTransferCallEvent(int event,
                                ACDCall* pCallRef);

   void appendAgentEvent(int event, UtlString* pQueueListString, ACDAgent* pAgentRef);

   void appendAcdEvent(int event);

   UtlString getEventString(int event_type, int event);

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsMutex       mLock;                   // Lock used for atomic access
   ACDRtRecord*  pACDRtRecord;
   FILE*         mFp;
   UtlString     mEventFile;              // Log filename including the path.

};

#endif  // _ACDRtRecord_h

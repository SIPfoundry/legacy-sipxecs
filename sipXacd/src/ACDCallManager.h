//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ACDCallManager_h_
#define _ACDCallManager_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsMutex.h>
#include <tapi/sipXtapi.h>
#include <tapi/sipXtapiEvents.h>
#include <utl/UtlHashMap.h>
#include "ACDCall.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class ACDServer;
class ACDLineManager;
class ACDAudioManager;

bool ACDCallManager_EventCallback(SIPX_EVENT_CATEGORY category,
                                  void* pInfo,
                                  void* pUserData);

class ACDCallManager {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */

   ACDCallManager(ACDServer* pAcdServer,
                  const int udpPort        = DEFAULT_UDP_PORT,
                  const int tcpPort        = DEFAULT_TCP_PORT,
                  const int tlsPort        = DEFAULT_TLS_PORT,
                  const int rtpPortStart   = DEFAULT_RTP_START_PORT,
                  const int maxConnections = DEFAULT_CONNECTIONS,
                  const char* pIdentity    = DEFAULT_IDENTITY,
                  const char* pBindToAddr  = DEFAULT_BIND_ADDRESS,
                  bool useSequentialPorts  = false);

   // Destructor
   ~ACDCallManager();


/* ============================ MANIPULATORS ============================== */

   OsStatus initialize(void);

   OsStatus start(void);

   void destroyACDCall(ACDCall* pCallRef);

   void addAssociationCallHandleAndAgent(ACDCall* pCall);

   void removeAssociationCallHandleAndAgent(ACDCall* pCall);

   void addMapTransferAgentCallHandleToCall(SIPX_CALL hCall, ACDCall* pCallRef);

   void removeMapTransferAgentCallHandleToCall(SIPX_CALL hCall);

   void addDeadCallToMap(SIPX_CALL hCall, UtlContainable* pCall);
   void addMapAgentCallHandleToCall(SIPX_CALL hCall, ACDCall* pCallRef);

   void removeMapAgentCallHandleToCall(SIPX_CALL hCall);

/* ============================ ACCESSORS ================================= */

   SIPX_INST getAcdCallManagerHandle(void);

   ACDAudioManager* getAcdAudioManager(void);

   int getSipPort(void);

   bool isThereActiveCalls(void);

   // CML: Required for call pickup
   ACDCall* getAcdCallByCallId(UtlString);

   ACDServer* getAcdServer(void) {return mpAcdServer;};

/* ============================ INQUIRY =================================== */
   bool verifyACDCallExists(ACDCall *pCallRef);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   bool eventCallback(SIPX_EVENT_CATEGORY category, void* pInfo);

   OsStatus createACDCall(ACDLine* pLineRef, SIPX_CALL hCallHandle);

   void removeCallFromMaps(ACDCall *pCallRef);
   void removeCallFromMap(ACDCall *pCallRef, UtlHashMap *pMap, char *mapName);
   bool verifyCallInMap(ACDCall *pCallRef, UtlHashMap *pMap, char *mapName);

   OsStatus updateCallState(SIPX_CALLSTATE_INFO* pCallInfo);

   void updateTransferCallState(SIPX_CALLSTATE_INFO* pCallInfo);

   bool validateTransferToLine(SIPX_CALLSTATE_INFO* pCallInfo);


   OsMutex           mLock;                 // Lock used for atomic access
   ACDServer*        mpAcdServer;           // Reference to the parent ACDServer
   int               mSipPort;              // The port number that the UA is bound to
   SIPX_INST         mAcdCallManagerHandle; // sipXtapi handle for UA
   ACDLineManager*   mpAcdLineManager;      // reference to associated Line Manager object
   ACDAudioManager*  mpAcdAudioManager;     // Reference to the peer AudioManager object
   UtlHashMap        mCallHandleMap;        // Handle Map for Call objects
   UtlHashMap        mAgentCallHandleMap;   // Handle Map for Call objects
   UtlHashMap        mTransferCallHandleMap; // Handle Map for Call objects
   UtlHashMap        mDeadCallHandleMap;    // Handle Map for Dead Call objects
   UtlHashMap        mACDCallExistsMap;     // Map for ACDCall objects
   int               mTotalCalls;           // Total number of calls received by AD server

/* ============================ INLINE METHODS ============================ */

/* ============================ FRIEND METHODS ============================ */

   friend bool ACDCallManager_EventCallback(SIPX_EVENT_CATEGORY category,
                                            void* pInfo,
                                            void* pUserData);

};

#endif  // _ACDCallManager_h_

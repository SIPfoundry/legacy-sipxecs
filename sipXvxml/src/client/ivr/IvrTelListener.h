// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _IvrTelListener_h_
#define _IvrTelListener_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsServerTask.h"
#include "os/OsRWMutex.h"
#include "os/OsBSem.h"
#include "os/OsEvent.h"
#include "cp/CallManager.h"
#include <VXIlog.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
struct  ListenerDB {
   UtlString       mName;
   UtlString       mAltName;
   UtlString mRemoteAddesss;
   OsBSem   *mpSemStateChange;
   int             mSemState;
   int             mpListenerPtr;
   int             mRef;
   int             mId;
   int             mIntData;
};

struct  TransferredCalls {
   UtlString       mCallId;
   UtlString       mTgtCallId;
   int             mRef;
};

struct  CleanupDB {
   UtlString       mName;
   int             mRef;
};

// ENUMS

// TYPEDEFS
// FORWARD DECLARATIONS
class OsTimer;
class OsEvent;
class TaoMessage;
class TaoString;

// The IvrTelListener handles all of the syslog processing
class IvrTelListener : public OsServerTask
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   enum SemState 
   {
      IDLE = 0,
      IN_USE,
      REQUEST_REMOVE
   };
                                
   /* ============================ CREATORS ================================== */

   IvrTelListener(CallManager* pCallMgr,
                  int timeoutSec = 60,
                  const UtlString& name = "IvrTelListener-%d",
                  int maxRequestQMsgs = 1000);
   //:Default constructor

   virtual ~IvrTelListener();
   //:Destructor

   /* ============================ MANIPULATORS ============================== */

   static IvrTelListener* getTelListener(CallManager* pCallMgr);

   virtual UtlBoolean handleMessage(OsMsg& eventMessage);
   //:Handles all incoming requests

   virtual int waitForFinalState(const char* callId, char* remoteAddress);

   virtual UtlBoolean stopWaitForFinalState(const char* callId);

   virtual void addListener(const char* callId);
   virtual OsStatus removeListener(const char* callId);

   UtlBoolean removeFromCleanupInProgress(const char* callId);

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */
                
   UtlBoolean isCleanupInProgress(const char* callId);

   /* //////////////////////////// PROTECTED ///////////////////////////////// */

  protected:
   CallManager* mpCallManager;
   OsRWMutex mRWMutex;           // Guards log data

   int       mTimeoutSec;

   UtlBoolean processTaoEvent(TaoMessage* pTaoMsg);

   UtlBoolean processTaoMessage(TaoMessage* pTaoMsg);

   void handleDisconnectCall(TaoString& arg);

   OsBSem                  mListenerSem;
   ListenerDB**    mpListeners;
   int                             mListenerCnt;
   int                             mMaxNumListeners;

   OsBSem                  mCleanupSem;
   CleanupDB**             mpCleanupInProgress;
   int                             mCleanupCnt;
   int                             mMaxNumCleanups;

   OsBSem                                  mTransferSem;
   TransferredCalls**      mTransferredCalls;
   int                                             mTransferredCallCnt;
   int                                             mMaxNumTransfers;

   void addTransferCallIds(UtlString* callIds, int numOldCalls);


   /* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:
   static IvrTelListener*    spInstance;    // pointer to the single instance of
   //  the IvrTelListener class
   static OsBSem     sLock;         

   IvrTelListener(const IvrTelListener& rIvrTelListener);
   //:Copy constructor

   IvrTelListener& operator=(const IvrTelListener& rhs);
   //:Assignment operator   

   OsBSem *findStateChangeSemaphore(const char* callId);

   void doCleanUpCall(const char* callId);
   void addToCleanupInProgress(const char* callId);

   void removeTransferCallId(int index);

   int findIndexForListener(const char* callId);

#ifdef TEST
   static bool sIsTested;
   //:Set to true after the tests for this class have been executed once

   void test();
   //:Verify assertions for this class

   // Test helper functions
   void testCreators();
   void testManipulators();
   void testAccessors();
   void testInquiry();

#endif /* TEST */
};

/* ============================ INLINE METHODS ============================ */

#endif  /* _IvrTelListener_h_ */

// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _IvrCallListener_h_
#define _IvrCallListener_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "tao/TaoAdaptor.h"
#include "tao/TaoString.h"
#include "OSBclient.h"
#include "IvrDtmfListener.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

typedef struct ActiveCallIds 
{
   int       mRef;
   UtlString  mCallId;
} ActiveCallIds;

// FORWARD DECLARATIONS
class CallManager;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class IvrCallListener : public TaoAdaptor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/* ============================ CREATORS ================================== */

   IvrCallListener(CallManager* callManager = NULL, 
                   VXIplatform *platform = NULL,
                   int channelNum = 0, 
                   const UtlString& name = "IvrCallListener-%d");
   //:Default constructor

   IvrCallListener(const IvrCallListener& rIvrCallListener);
   //:Copy constructor

   virtual
      ~IvrCallListener();
   //:Destructor

/* ============================ MANIPULATORS ============================== */

   UtlBoolean handleMessage(OsMsg& rMsg);

   void setPlatform(VXIplatform *platform);

   void addListener(OsServerTask* pListener);


/* ============================ ACCESSORS ================================= */

   CallManager*         getCallManager() { return mpCallManager; };

   VXIplatform*         getPlatform() { return mpPlatform; };

/* ============================ INQUIRY =================================== */

   int running() { return mRunning; };

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

   virtual void handleAcceptCall(TaoMessage& rMsg, TaoString& arg);

   virtual void handleStartVXISession(TaoMessage& rMsg, TaoString& arg);

   virtual void handleDisconnectCall(TaoMessage& rMsg, TaoString& arg);
  
   virtual void handleConnectCall(TaoMessage& rMsg, TaoString& arg);
  

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   ActiveCallIds       **mActiveCallIds;
   int            mCurrentActiveCalls;
   int            mMaxActiveCalls;
   OsBSem         mSemCallIds;

   IvrDtmfListener *mpDtmfListener;
   CallManager*          mpCallManager;
   VXIplatform*          mpPlatform;
   OsServerTask*   mpListener;
   int                                   mChannelNum;
   int         mRunning;

   int getRemoteAddress(TaoMessage& rMsg, UtlString& rAddress);

   UtlBoolean isActiveCallId(const char* callId);
   OsStatus addCallId(const char* callId);
   OsStatus removeCallId(const char* callId);

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

#endif //TEST
};

/* ============================ INLINE METHODS ============================ */

#endif  // _IvrCallListener_h_

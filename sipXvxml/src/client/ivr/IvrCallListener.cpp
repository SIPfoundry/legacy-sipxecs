// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>
#if defined(_WIN32)
#   include <io.h>
#elif defined(_VXWORKS)
#   include <unistd.h>
#   include <dirent.h>
#elif defined(__pingtel_on_posix__)
#   include <unistd.h>
#   include <stdlib.h>
#   define O_BINARY 0 // There is no notion of a "not binary" file under POSIX,
                      // so we just set O_BINARY used below to no bits in the mask.
#else
#   error Unsupported target platform.
#endif
#include <fcntl.h> 

#ifdef TEST
#include "utl/UtlMemCheck.h"
#endif // TEST

// APPLICATION INCLUDES
#include "clientMain.h"
#include "IvrCallListener.h"
#include "os/OsLock.h"
#include "tao/TaoMessage.h"
#include "cp/CallManager.h"
#include "cp/CpCall.h"
#include "net/Url.h"
#include "net/SipSession.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define INITIAL_MAX_ACTIVE_CALLS 150
#define ADDITIONAL_ACTIVE_CALLS 20

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
IvrCallListener::IvrCallListener(CallManager* callManager, 
                                 VXIplatform *platform,
                                 int channelNum,
                                 const UtlString& name) :
TaoAdaptor(name, 1000),
mSemCallIds(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
#ifdef TEST
   if (!sIsTested)
   {
      sIsTested = true;
      test();
   }
#endif //TEST

   mpCallManager = callManager;
   mpPlatform = platform;
   mChannelNum = channelNum;
   mpDtmfListener = new IvrDtmfListener(mpPlatform);
   mpDtmfListener->start();
   mpListener = NULL;

   mCurrentActiveCalls = 0;
   mMaxActiveCalls = INITIAL_MAX_ACTIVE_CALLS;

   mActiveCallIds = (ActiveCallIds**) malloc(sizeof(ActiveCallIds *)*mMaxActiveCalls);

   if (!mActiveCallIds)
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, 
                    "IvrCallListener::_ ***** ERROR ALLOCATING mActiveCallIds IN IvrCallListener **** \n");
   }

   for (int i = 0; i < mMaxActiveCalls; i++)
      mActiveCallIds[i] = 0;

}

// Copy constructor
IvrCallListener::IvrCallListener(const IvrCallListener& rIvrCallListener)
: mSemCallIds(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
   mpCallManager = rIvrCallListener.mpCallManager;
   mpPlatform = rIvrCallListener.mpPlatform;
   mChannelNum = rIvrCallListener.mChannelNum;
   mpListener = rIvrCallListener.mpListener;
}

// Destructor
IvrCallListener::~IvrCallListener()
{
   waitUntilShutDown();
   if (mpDtmfListener)
   {
      mpDtmfListener->requestShutdown();
      delete mpDtmfListener;
      mpDtmfListener = NULL;
   }

   if (mActiveCallIds)
   {
      for (int i = 0; i < mMaxActiveCalls; i++)
      {
         if (mActiveCallIds[i])
         {
            delete mActiveCallIds[i];
            mActiveCallIds[i] = NULL;
         }
      }

      free(mActiveCallIds);
      mActiveCallIds = NULL;
   }

}

/* ============================ MANIPULATORS ============================== */

UtlBoolean IvrCallListener::handleMessage(OsMsg& rMsg)
{
   if(rMsg.getMsgSubType()== TaoMessage::EVENT)
   {
      TaoMessage* taoMessage = (TaoMessage*)&rMsg;

      TaoEventId taoEventId = taoMessage->getTaoObjHandle();
      UtlString argList(taoMessage->getArgList());

      TaoString arg(argList, TAOMESSAGE_DELIMITER);
      int argc = arg.getCnt();

      UtlString callId = arg[0];

      UtlBoolean localConnection = atoi(arg[6]);
      UtlBoolean remoteIsCallee = atoi(arg[3]);
      int metaCode = 0;
      if (argc > 9) 
         metaCode = atoi(arg[9]);

#ifdef TEST_PRINT /* [ */
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, 
                    "IvrCallListener::handleMessage: type     %d\n subtype  %d \n", 
                    taoMessage->getMsgType(), taoMessage->getMsgSubType());
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, 
                    "IvrCallListener::handleMessage: event id %d\n", 
                    taoMessage->getTaoObjHandle());
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, 
                    "IvrCallListener::handleMessage: localconnection %d\n", 
                    localConnection);
      for (int i = 0; i < argc; i++)
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "          %s\n", arg[i]);
#endif /* TEST_PRINT ] */

#ifdef TEST_PRINT /* [ */
      UtlString connStr;
      CpCall::getStateString(taoMessage->getTaoObjHandle(), &connStr);
#endif /* TEST_PRINT ] */
      if(taoEventId == PtEvent::CONNECTION_OFFERED
         && !remoteIsCallee && localConnection)
      {
#ifdef TEST_PRINT /* [ */
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, 
                       "IvrCallListener::handleMessage: Receiving new message %s %s\n", 
                       callId.data(), connStr.data());
#endif /* TEST_PRINT ] */
         handleAcceptCall((TaoMessage&)rMsg, arg);
      } 
      else if((taoEventId == PtEvent::TERMINAL_CONNECTION_TALKING) 
              && localConnection 
              && metaCode != PtEvent::META_CALL_TRANSFERRING
              && metaCode != PtEvent::META_CALL_REPLACING)
      {
#ifdef TEST_PRINT /* [ */
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, 
                       "IvrCallListener::handleMessage: Receiving new message %s %s\n", 
                       callId.data(), connStr.data());
#endif /* TEST_PRINT ] */
         handleStartVXISession((TaoMessage&)rMsg, arg);
      }
      else if ((taoEventId == PtEvent::CONNECTION_DISCONNECTED &&
                !localConnection) ||
               taoEventId == PtEvent::CONNECTION_FAILED)
      {
         // CONNECTION_DISCONNECTED causes the call to be torn down if
         // it is remote.  (If we see a local CONNECTION_DISCONNECTED,
         // we will eventually see a remote one.)
         // CONNECTION_FAILED cause the call to be town regardless of
         // which end it is reported for.
#ifdef TEST_PRINT /* [ */
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, 
                       "IvrCallListener::handleMessage: Receiving new message %s %s\n", 
                       callId.data(), connStr.data());
#endif /* TEST_PRINT ] */
         // Calling handleDisconnectCall will remove this Call-Id from its
         // list of calls to listen for, so we will never call it twice
         // for the same call.
         handleDisconnectCall((TaoMessage&)rMsg, arg);
      }
      else if((taoEventId == PtEvent::CONNECTION_CONNECTED    || 
               taoEventId == PtEvent::CONNECTION_ESTABLISHED  ||
               taoEventId == PtEvent::CONNECTION_OFFERED) && 
              !localConnection)
      {
#ifdef TEST_PRINT /* [ */
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, 
                       "IvrCallListener::handleMessage: Receiving new message %s %s\n", 
                       callId.data(), connStr.data());
#endif /* TEST_PRINT ] */
         handleConnectCall((TaoMessage&)rMsg, arg);
      }
      else if(taoEventId == PtEvent::MULTICALL_META_TRANSFER_STARTED)
      {
#ifdef TEST_PRINT /* [ */
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, 
                       "IvrCallListener::handleMessage: Receiving new message %s %s\n", 
                       callId.data(), connStr.data());
#endif /* TEST_PRINT ] */
         handleConnectCall((TaoMessage&)rMsg, arg);
      }
   }

   return(TRUE);
}

void IvrCallListener::setPlatform(VXIplatform *platform)
{
   mpPlatform = platform;
}

void IvrCallListener::addListener(OsServerTask* pListener)
{
   mpListener = pListener;
}
/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
void IvrCallListener::handleAcceptCall(TaoMessage& rMsg, TaoString& arg)
{            
   UtlString callId = arg[0];
   UtlString address = arg[2];
   mpCallManager->acceptConnection(callId.data(), address.data());
   mpCallManager->answerTerminalConnection(callId.data(), address.data(), "mediaserver");
   mpCallManager->addToneListener(callId.data(), mpDtmfListener);

   // Add this call to the list so we can drop it later.
   OsStatus res = addCallId(callId.data());
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, 
                 "IvrCallListener::handleAcceptCall %s %s addCallId=%d\n", 
                 callId.data(), address.data(), res);
}

void IvrCallListener::handleStartVXISession(TaoMessage& rMsg, TaoString& arg)
{            
   UtlString address;
   getRemoteAddress((TaoMessage&)rMsg, address);

   UtlString callId = arg[0];
   SipSession ses;
   UtlString requestUriStr;
   Url fromUrl;
   Url toUrl;
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                 "IvrCallListener::handleStartVXISession callId = '%s', address = '%s'",
                 callId.data(), address.data());
   if (mpCallManager->getSession(
          callId.data(),         // call id
          address.data(), // remote address
          ses) == OS_SUCCESS)
   {
      ses.getFromUrl(fromUrl);
      ses.getToUrl(toUrl);
      ses.getRemoteRequestUri(requestUriStr);
   }

   Url requestUrl(requestUriStr);

   UtlString domain;
   UtlString userId;
   UtlString vxmlDoc;

   // voicexml parameter is RFC-4240 
   // try that first
   requestUrl.getFieldParameter("voicexml", vxmlDoc);
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
              "IvrCallListener::handleStartVXISession voicexml vxmlDoc = '%s'",
              vxmlDoc.data());
   if (vxmlDoc.isNull())
   {
      // Didn't find it.  Try play parameter (older, proprietary)
      requestUrl.getFieldParameter("play", vxmlDoc);
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                 "IvrCallListener::handleStartVXISession play vxmlDoc = '%s'",
                 vxmlDoc.data());
   }

   requestUrl.getFieldParameter("user", userId);
   requestUrl.getFieldParameter("domain", domain);
   if (userId.isNull()) requestUrl.getUserId(userId);

   UtlString callerAddress;
   UtlString targetAddress;
   toUrl.toString(callerAddress);
   fromUrl.toString(targetAddress);

   if (vxmlDoc.isNull())
   {
#ifdef HANDLE_NO_PLAY_URL /* [ */
      if (userId.length() == 0) userId = "default";
      vxmlDoc = userId + UtlString(".vxml");   // get base url from the config file OSBclient.cfg
      HttpMessage::unescape(vxmlDoc);
#else
      // This is a transfer target call, and will be dropped when the original call is dropped.
      // Remove this call from the list so we will not drop it twice.
      OsStatus res = removeCallId(callId.data());
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, 
                    "IvrCallListener::handleStartVXISession-%s: no play url specified, skip. removeCallId=%d \n", 
                    callId.data(), res);
      return;
#endif /* HANDLE_NO_PLAY_URL ] */
   }
   else
   {
      HttpMessage::unescape(vxmlDoc);

      if (!callerAddress.isNull())
      {
         UtlString tobeEscaped("=&");
         HttpMessage::escapeChars(callerAddress, tobeEscaped);
         HttpMessage::escape(callerAddress);
         vxmlDoc += "&from=" + callerAddress;
      }
      UtlString tobeEscaped("@+");
      HttpMessage::escapeChars(vxmlDoc, tobeEscaped);
   }

   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, 
                 "IvrCallListener::handleStartVXISession %s %s %s \n", 
                 callId.data(), vxmlDoc.data(), callerAddress.data());
   VXIProcessUrl(this,           
                 mChannelNum,
                 callId.data(),          // callid
                 callerAddress.data(),
                 targetAddress.data(),
                 vxmlDoc.data());                // url
}

void IvrCallListener::handleDisconnectCall(TaoMessage& rMsg, TaoString& arg)
{
   int numConns = 0;
   UtlString callId = arg[0];
   mpCallManager->getNumConnections(callId.data(), numConns);
   if (numConns <= 2)
   {
      mpCallManager->removeToneListener(callId.data(), mpDtmfListener);
      UtlBoolean res = isActiveCallId(callId.data());
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, 
                    "IvrCallListener::handleDisconnectCall %s active=%d \n", callId.data(), res);
      if (mpListener && res) 
      {
         mpListener->postMessage(rMsg);

         // Remove this call from the list so we will not drop it twice.
         removeCallId(callId.data());
      }
   }
   else
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, 
                    "IvrCallListener::handleDisconnectCall %s has %d connections", 
                    callId.data(), numConns);
   }
}

void IvrCallListener::handleConnectCall(TaoMessage& rMsg, TaoString& arg)
{

   if (mpListener) mpListener->postMessage(rMsg);
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

int IvrCallListener::getRemoteAddress(TaoMessage& rMsg, UtlString& rAddress)
{
   TaoString arg = TaoString(rMsg.getArgList(), TAOMESSAGE_DELIMITER); 
   UtlString callId = arg[0]; 
   UtlString address = arg[2]; 

   int numConnections = 0;
   mpCallManager->getNumConnections(callId.data(), numConnections);

   if (numConnections > 10) numConnections = 10;
   UtlString addresses[10];

   if (addresses)
   {
      int maxConnections = numConnections;
      mpCallManager->getConnections(callId.data(), maxConnections, numConnections, addresses);
      if (numConnections)
         rAddress = addresses[numConnections - 1];
   }

   return 1;
}

UtlBoolean IvrCallListener::isActiveCallId(const char* callId)
{   
   OsLock lock(mSemCallIds);
   UtlBoolean res = FALSE;

   if (callId == NULL)
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, "IvrCallListener::isActiveCallId - callId is NULL, return FALSE.\n");
      return res;
   }

   for (int i = 0; i < mCurrentActiveCalls; i++)
   {
#ifdef TEST_PRINT
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, 
                    "IvrCallListener::isActiveCallId - %p %s for %s\n", 
                    mActiveCallIds[i], mActiveCallIds[i]->mCallId.data(), callId);
#endif
      if (mActiveCallIds[i] && (mActiveCallIds[i]->mCallId.compareTo(callId) == 0))
      {
         res = TRUE;
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, 
                       "IvrCallListener::isActiveCallId - TRUE for %s\n", callId);
         break;
      }
   }

   return res;
}

OsStatus IvrCallListener::addCallId(const char* callId)
{
   OsLock lock(mSemCallIds);

   OsStatus res = OS_FAILED;

   if (callId == NULL)
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, "IvrCallListener::addCallId - callId is NULL, return OS_FAILED.\n");
      return res;
   }

   for (int i = 0; i < mCurrentActiveCalls; i++)
   {
      if (mActiveCallIds[i] && (mActiveCallIds[i]->mCallId.compareTo(callId) == 0))
      {
         mActiveCallIds[i]->mRef++;
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, 
                       "IvrCallListener::addCallId - call already exists for call %s\n", callId);
         return res;
      }
   }

   if (mCurrentActiveCalls == mMaxActiveCalls)
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, "IvrCallListener::addCallId - add %d more listeners\n", ADDITIONAL_ACTIVE_CALLS);
      //make more of em.
      mMaxActiveCalls += ADDITIONAL_ACTIVE_CALLS;
      mActiveCallIds = (ActiveCallIds **)realloc(mActiveCallIds, sizeof(ActiveCallIds *)*mMaxActiveCalls);
      if (!mActiveCallIds)
      {
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,   
                       "IvrCallListener::addCallId - out of memory for call %s\n", callId);
         OsSysLog::flush();
         res = OS_NO_MEMORY;
         return res;
      }
      for (int loop = mCurrentActiveCalls; loop < mMaxActiveCalls; loop++)
         mActiveCallIds[loop] = 0 ;
   }

   ActiveCallIds *pCallId = new ActiveCallIds;
   if (pCallId)
   {
      pCallId->mRef = 1;
      pCallId->mCallId = callId;
      mActiveCallIds[mCurrentActiveCalls++] = pCallId;
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, 
                    "IvrCallListener::addCallId - added for %s cnt=%d\n", callId, mCurrentActiveCalls);
      res = OS_SUCCESS;
   }
        
   return res;
}

OsStatus IvrCallListener::removeCallId(const char* callId)
{
   OsLock lock(mSemCallIds);

   OsStatus res = OS_NOT_FOUND;

   if (callId == NULL)
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, "IvrCallListener::removeCallId - callId is NULL, return OS_NOT_FOUND.\n");
      return res;
   }

   for (int i = 0; i < mCurrentActiveCalls; i++)
   {
      if (mActiveCallIds[i] && (mActiveCallIds[i]->mCallId.compareTo(callId) == 0))
      {
         mActiveCallIds[i]->mRef--;
         if (mActiveCallIds[i]->mRef == 0)
         {
            delete mActiveCallIds[i];
            mActiveCallIds[i] = NULL;
            mCurrentActiveCalls--;
            for (int j = i; j < mCurrentActiveCalls; j++)
            {
               mActiveCallIds[j] = mActiveCallIds[j+1];
            }
            mActiveCallIds[mCurrentActiveCalls] = NULL;
            res = OS_SUCCESS;
         }
         else
         {
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                          "IvrCallListener::removeCallId - mRef=%d for call %s\n", 
                          mActiveCallIds[i]->mRef, callId);
         }
         break;
      }
   }

   return res;
}


/* ============================ TESTING =================================== */

#ifdef TEST

// Set to true after the tests have been executed once
bool IvrCallListener::sIsTested = false;

// Test this class by running all of its assertion tests
void IvrCallListener::test()
{
   UtlMemCheck* pMemCheck = 0;
   pMemCheck = new UtlMemCheck();      // checkpoint for memory leak check

   testCreators();
   testManipulators();
   testAccessors();
   testInquiry();

   assert(pMemCheck->delta() == 0);    // check for memory leak
   delete pMemCheck;
}

// Test the creators (and destructor) methods for the class
void IvrCallListener::testCreators()
{
   UtlMemCheck* pMemCheck  = 0;

   pMemCheck = new UtlMemCheck();      // checkpoint for memory leak check

   // test the default constructor (if implemented)
   // test the copy constructor (if implemented)
   // test other constructors (if implemented)
   //    if a constructor parameter is used to set information in an ancestor
   //       class, then verify it gets set correctly (i.e., via ancestor
   //       class accessor method.
   // test the destructor
   //    if the class contains member pointer variables, verify that the 
   //    pointers are getting scrubbed.

   assert(pMemCheck->delta() == 0);    // check for memory leak
   delete pMemCheck;
}

// Test the manipulator methods
void IvrCallListener::testManipulators()
{
   UtlMemCheck* pMemCheck  = 0;

   pMemCheck = new UtlMemCheck();      // checkpoint for memory leak check

   // test the assignment method (if implemented)
   // test the other manipulator methods for the class

   assert(pMemCheck->delta() == 0);    // check for memory leak
   delete pMemCheck;
}

// Test the accessor methods for the class
void IvrCallListener::testAccessors()
{
   UtlMemCheck* pMemCheck  = 0;

   pMemCheck = new UtlMemCheck();      // checkpoint for memory leak check

   // body of the test goes here

   assert(pMemCheck->delta() == 0);    // check for memory leak
   delete pMemCheck;
}

// Test the inquiry methods for the class
void IvrCallListener::testInquiry()
{
   UtlMemCheck* pMemCheck  = 0;

   pMemCheck = new UtlMemCheck();      // checkpoint for memory leak check

   // body of the test goes here

   assert(pMemCheck->delta() == 0);    // check for memory leak
   delete pMemCheck;
}

#endif //TEST

/* ============================ FUNCTIONS ================================= */

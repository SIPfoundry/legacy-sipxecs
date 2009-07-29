//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <listener.h>
#include <CallObject.h>
#include "os/OsWriteLock.h"
#include <tao/TaoMessage.h>
#include <tao/TaoString.h>
#include <cp/CallManager.h>
#include <net/Url.h>
#include <os/OsFS.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define TAO_OFFER_PARAM_CALLID             0
#define TAO_OFFER_PARAM_ADDRESS            2
#define TAO_OFFER_PARAM_LOCAL_CONNECTION   6

//#define DEBUGGING 1

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
Listener::Listener(CallManager* callManager, UtlString playfile)
   : mPlayfile(playfile),
     mRWMutex(OsRWMutex::Q_PRIORITY)
{
   mpCallManager = callManager;
   mTotalCalls = 0;
}


//Destructor
Listener::~Listener()
{
}


/* ============================ MANIPULATORS ============================== */

UtlBoolean Listener::handleMessage(OsMsg& rMsg)
{
   // React to telephony events
   if(rMsg.getMsgSubType()== TaoMessage::EVENT)
   {
      TaoMessage* taoMessage = (TaoMessage*)&rMsg;

      TaoEventId taoEventId = taoMessage->getTaoObjHandle();
      UtlString argList(taoMessage->getArgList());
      TaoString arg(argList, TAOMESSAGE_DELIMITER);

#ifdef DEBUGGING
      dumpTaoMessageArgs(taoEventId, arg) ;
#endif
      UtlBoolean localConnection = atoi(arg[TAO_OFFER_PARAM_LOCAL_CONNECTION]);
      UtlString  callId = arg[TAO_OFFER_PARAM_CALLID] ;
      UtlString  address = arg[TAO_OFFER_PARAM_ADDRESS] ;

      switch (taoEventId)
      {
         case PtEvent::CONNECTION_OFFERED:
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "Call arrived: callId %s address %s\n",
                          callId.data(), address.data());

            mpCallManager->acceptConnection(callId, address);
            mpCallManager->answerTerminalConnection(callId, address, "*");

            break;
         case PtEvent::CONNECTION_ESTABLISHED:
            if (localConnection)
            {
               OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "Call connected: callId %s\n", callId.data());

               CallObject* pThisCall = new CallObject(mpCallManager, callId, mPlayfile);

               // Create a player and start to play out the file
               if (pThisCall->playAudio() == OS_SUCCESS)
               {
                  // Put it in a sorted list
                  insertEntry(callId, pThisCall);
               }
               else
               {
                  // Drop the call
                  mpCallManager->drop(callId);
                  OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_WARNING, "Listener::handleMessage - drop callId %s due to failure of playing audio\n",
                                callId.data());

                  delete pThisCall;
               }
            }

            break;

         case PtEvent::CONNECTION_DISCONNECTED:
            if (!localConnection)
            {
               OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "Call Dropped: %s\n", callId.data());

               // Remove the call from the pool and clean up the call
               CallObject* pDroppedCall = removeEntry(callId);
               if (pDroppedCall)
               {
                  pDroppedCall->cleanUp();
                  delete pDroppedCall;

                  // Drop the call
                  mpCallManager->drop(callId);
               }
               else
               {
                  OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "Lisenter::handleMessage - no callId %s founded in the active call list\n",
                                callId.data());
               }
            }

            break;

         case PtEvent::CONNECTION_FAILED:
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_WARNING, "Dropping call: %s\n", callId.data());

            mpCallManager->drop(callId);

            break;
      }
   }
   return(TRUE);
}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */


void Listener::dumpTaoMessageArgs(unsigned char eventId, TaoString& args)
{
   osPrintf("===>\nMessage type: %d args:\n\n", eventId) ;

   int argc = args.getCnt();
   for(int argIndex = 0; argIndex < argc; argIndex++)
   {
      osPrintf("\targ[%d]=\"%s\"\n", argIndex, args[argIndex]);
   }
}


void Listener::insertEntry(UtlString& rKey,
                           CallObject* newObject)
{
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "Listener::insertEntry - Putting %p into the call pool for callId %s\n",
                 newObject, rKey.data());

   OsWriteLock lock(mRWMutex);
   ActiveCall  tempEntry(rKey, newObject);
   ActiveCall* pOldEntry;
   unsigned int       i;
   i = mCalls.index(&tempEntry);
   if (i != UTL_NOT_FOUND)
   {                             // we already have an entry with this key
      pOldEntry = (ActiveCall *)mCalls.at(i);
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_WARNING, "Listener::insertEntry - FOUND %s\n", rKey.data());
   }
   else
   {
      ActiveCall* pNewEntry = new ActiveCall(rKey, newObject);
      mCalls.insert(pNewEntry);

      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "Listener::insertEntry - inserted %s\n", rKey.data());

      mTotalCalls++;
   }
}


CallObject* Listener::removeEntry(UtlString& rKey)
{
   OsWriteLock lock(mRWMutex);
   ActiveCall  lookupPair(rKey);
   ActiveCall* pEntryToRemove;
   unsigned int i = mCalls.index(&lookupPair);
   if (i == UTL_NOT_FOUND)
   {
      return NULL;
   }
   else
   {
      pEntryToRemove = (ActiveCall *)mCalls.at(i);
      CallObject* pObject = pEntryToRemove->getCallObject();
      mCalls.removeAt(i);
      delete pEntryToRemove;

      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "Listener::remove - removed CallObject %p\n", pObject);

      return pObject;
   }
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

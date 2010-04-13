//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdlib.h>

// APPLICATION INCLUDES
#include "OrbitListener.h"
#include "ParkedCallObject.h"
#include <tao/TaoMessage.h>
#include <tao/TaoString.h>
#include <cp/CallManager.h>
#include <net/Url.h>
#include <os/OsFS.h>
#include <os/OsDateTime.h>
#include "xmlparser/tinyxml.h"
#include "xmlparser/ExtractContent.h"
#include <utl/UtlHashMapIterator.h>
#include "os/OsMsg.h"
#include "os/OsEventMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define TAO_OFFER_PARAM_CALLID             0
#define TAO_OFFER_PARAM_ADDRESS            2
#define TAO_OFFER_PARAM_LOCAL_CONNECTION   6
#define TAO_OFFER_PARAM_NEW_CALLID        11
// This is the argument in a MULTICALL_META_TRANSFER_ENDED event that
// contains the callId of the call that was being transferred.
// (TA_OFFER_PARAM_CALLID is the callId of the new call created to
// represent the transfer operation.)  I don't know if this location
// is truly fixed by the design, but this value works consistently.
#define TAO_TRANSFER_ENDED_ORIGINAL_CALLID 12
// Ditto for the callId of the pseudo-call used to report the status
// of the transfer.
#define TAO_TRANSFER_ENDED_TRANSFER_CALLID 11

#define DEFAULT_MOH_AUDIO                  "file://"SIPX_VARDIR"/parkserver/music/default.wav"
#define MOH_USER_PART                      "moh"

#define CALL_RETRIEVE_URI_PARM             "operation"
#define CALL_RETRIEVE_URI_VALUE            "retrieve"

// STATIC VARIABLE INITIALIZATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OrbitListener::OrbitListener(CallManager* callManager,
                             int lifetime,
                             int blindXferWait,
                             int keepAliveTime,
                             int consXferWait) :
   TaoAdaptor("OrbitListener-%d"),
   mpCallManager(callManager),
   mLifetime(lifetime, 0),
   mBlindXferWait(blindXferWait, (intptr_t)0),
   mKeepAliveTime(keepAliveTime, 0),
   mConsXferWait(consXferWait, 0)
{
   // Assemble the full file name of the orbit file and initialize the reader.
   UtlString f = SIPX_CONFDIR + OsPathBase::separator + ORBIT_CONFIG_FILENAME;
   mOrbitFileReader.setFileName(&f);

   // Start the dummy listener, so that it will process messages (and its queue
   // will not fill up).
   mListener.start();
}


// Destructor
OrbitListener::~OrbitListener()
{
   // We should dispose of all the parked calls.  But what is the correct
   // method?  Should we just drop them, or try to transfer them back to
   // their parkers?
   // :TODO:  Not done yet because sipXpark never destroys its OrbitListener.
}


/* ============================ MANIPULATORS ============================== */

UtlBoolean OrbitListener::handleMessage(OsMsg& rMsg)
{
   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "OrbitListener::handleMessage - "
                 "message type %d subtype %d",
                 rMsg.getMsgType(), rMsg.getMsgSubType());

   // React to telephony events
   if (rMsg.getMsgType() == OsMsg::TAO_MSG &&
       rMsg.getMsgSubType() == TaoMessage::EVENT)
   {
      TaoMessage* taoMessage = (TaoMessage*)&rMsg;

      TaoObjHandle taoEventId = taoMessage->getTaoObjHandle();
      UtlString argList(taoMessage->getArgList());
      TaoString argTao(argList, TAOMESSAGE_DELIMITER);

      if (OsSysLog::willLog(FAC_PARK, PRI_DEBUG))
      {
         // Dump the contents of the Tao message.
         dumpTaoMessageArgs(taoEventId, argTao) ;
      }
      UtlBoolean localConnectionTao = atoi(argTao[TAO_OFFER_PARAM_LOCAL_CONNECTION]);
      UtlString  callIdTao = argTao[TAO_OFFER_PARAM_CALLID] ;
      UtlString  addressTao = argTao[TAO_OFFER_PARAM_ADDRESS] ;

      switch (taoEventId)
      {
         case PtEvent::CONNECTION_OFFERED:
         {
            OsSysLog::add(FAC_PARK, PRI_DEBUG,
                          "OrbitListener::handleMessage - %d "
                          "Call offered: callId: '%s', address: '%s'",
                          (int) taoEventId, callIdTao.data(), addressTao.data());
            if (mTransferCalls.find(&callIdTao))
            {
               OsSysLog::add(FAC_PARK, PRI_DEBUG,
                             "OrbitListener::handleMessage - %d "
                             "callId: '%s', address: '%s' found in mTransferCalls",
                             (int) taoEventId, callIdTao.data(), addressTao.data());
            }

            // Validate the orbit and get the information about the orbit.
            UtlString orbit;
            UtlString audio;
            int timeout, keycode, capacity;
            if (validateOrbit(callIdTao, addressTao, orbit, audio,
                              timeout, keycode, capacity) != OS_SUCCESS)
            {
               OsSysLog::add(FAC_PARK, PRI_DEBUG,
                             "OrbitListener::handleMessage - %d "
                             " call '%s', invalid orbit '%s'",
                             (int) taoEventId, callIdTao.data(), orbit.data());

               mpCallManager->rejectConnection(callIdTao.data(), addressTao.data(),
                                               SIP_NOT_FOUND_CODE, "Invalid Park Orbit");
            }
            else
            {
               // Is this a retrieval call?
               if (isCallRetrievalInvite(callIdTao.data(), addressTao.data()))
               {
                  // Verify that there is at least one call in the orbit available for retrieval.
                  int calls_in_orbit = getNumCallsInOrbit(orbit, TRUE);
                  if (calls_in_orbit >= 1)
                  {
                     // Accept the call.
                     OsSysLog::add(FAC_PARK, PRI_DEBUG,
                                   "OrbitListener::handleMessage - %d "
                                   "call retrieval call '%s' accepted",
                                   (int) taoEventId, callIdTao.data());
                     mpCallManager->acceptConnection(callIdTao, addressTao);
                     mpCallManager->answerTerminalConnection(callIdTao, addressTao, "*");
                     // We don't have to add a DTMF listener, as this call will
                     // be turned into a transfer as soon as it is established.
                  }
                  else
                  {
                     OsSysLog::add(FAC_PARK, PRI_DEBUG,
                                   "OrbitListener::handleMessage - %d "
                                   "call retrieval call '%s', orbit '%s', "
                                   "rejected due to no calls to retrieve",
                                   (int) taoEventId, callIdTao.data(), orbit.data());

                     mpCallManager->rejectConnection(callIdTao.data(), addressTao.data(),
                                                     SIP_NOT_FOUND_CODE,
                                                     "No parked calls to retrieve");
                  }
               }
               else
               {
                  // Check that the orbit is below the maximum number of calls
                  // allowed.
                  int calls_in_orbit = getNumCallsInOrbit(orbit, FALSE);
                  if (calls_in_orbit >= capacity)
                  {
                     OsSysLog::add(FAC_PARK, PRI_DEBUG,
                                   "OrbitListener::handleMessage - %d "
                                   "not call retrieval call '%s', orbit '%s' full",
                                   (int) taoEventId, callIdTao.data(), orbit.data());

                     mpCallManager->rejectConnection(callIdTao.data(), addressTao.data(),
                                                     SIP_BUSY_CODE, "Park orbit is full");
                  }
                  else
                  {
                     OsSysLog::add(FAC_PARK, PRI_DEBUG,
                                   "OrbitListener::handleMessage - %d "
                                   "not call retrieval call '%s', accept",
                                   (int) taoEventId, callIdTao.data());
                     // Accept the call.
                     mpCallManager->acceptConnection(callIdTao, addressTao);
                     mpCallManager->answerTerminalConnection(callIdTao, addressTao,
                                                             "*");

                     // Set up the data structures for this dialog.
                     setUpParkedCallOffered(callIdTao, addressTao,
                                            orbit, audio, timeout, keycode, capacity,
                                            argTao);
                  }
               }
            }
         }
         break;

         case PtEvent::CONNECTION_ESTABLISHED:
         {
             if (localConnectionTao)
             {
                OsSysLog::add(FAC_PARK, PRI_DEBUG,
                              "OrbitListener::handleMessage - %d "
                              "Call established: callId '%s', address: '%s'",
                              (int) taoEventId, callIdTao.data(), addressTao.data());
                if (mTransferCalls.find(&callIdTao))
                {
                   // found call-id in transfer hashmap, get origi call-id value (if any)
                   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                                 "OrbitListener::handleMessage - %d "
                                 "callId: '%s', address: '%s' found in mTransferCalls",
                                 (int) taoEventId, callIdTao.data(), addressTao.data());
                    UtlString* idOfOrigCallTxfr =
                      dynamic_cast <UtlString*> (mTransferCalls.findValue(&callIdTao));
                   if (idOfOrigCallTxfr)
                   {
                      // If we have a ParkedCallObject for the originalCallId,
                      // remove it from the hashmap and re-insert it with the new callId
                      // Then update "originalCallId" in PCO to new callid.
                      OsSysLog::add(FAC_PARK, PRI_DEBUG,
                                    "OrbitListener::handleMessage - %d "
                                    "transfer pair - callId: '%s', address: '%s', originalCallId: '%s'",
                                    (int) taoEventId, callIdTao.data(), addressTao.data(),
                                    idOfOrigCallTxfr->data());

                      // See if the originalCallId is one of our calls.
                      ParkedCallObject* pCall =
                         dynamic_cast <ParkedCallObject *> (mCalls.findValue(idOfOrigCallTxfr));
                      if (pCall == NULL)
                      {
                         OsSysLog::add(FAC_PARK, PRI_DEBUG,
                                       "OrbitListener::handleMessage - %d "
                                       "Original callId '%s' not found in the parked call list",
                                       (int) taoEventId, idOfOrigCallTxfr->data());
                      }
                      else
                      {
                         OsSysLog::add(FAC_PARK, PRI_DEBUG,
                                       "OrbitListener::handleMessage - %d "
                                       "found parked call object %p for '%s'",
                                       (int) taoEventId, pCall, callIdTao.data());

                         OsSysLog::add(FAC_PARK, PRI_DEBUG,
                                       "OrbitListener::handleMessage - %d "
                                       "replacing parked call(%p) object key "
                                       "was %s changes to '%s'",
                                       (int) taoEventId, pCall, idOfOrigCallTxfr->data(), callIdTao.data());

                         mCalls.remove(idOfOrigCallTxfr);
                         mCalls.insertKeyAndValue(new UtlString(callIdTao),pCall);
                         pCall->setCurrentCallId(callIdTao);
                      }
                   }    // end found original call-id as value in mTransferCalls hashmap
                }   // end found call-id in mTransferCalls hashmap


                // CallId at this point is either the callId of a normal call or the original callId
                // of a second leg of a transfer.

                // Test if this is a call retrieval operation
                bool bIsRetrieve = !isCallRetrievalInvite(callIdTao.data(), addressTao.data());
                OsSysLog::add(FAC_PARK, PRI_DEBUG,
                              "OrbitListener::handleMessage - %d "
                              "is %scall retrieval call '%s'",
                              (int) taoEventId,
                              (bIsRetrieve ? "" : "not "),
                              callIdTao.data());

                if (bIsRetrieve)
                {
                  ParkedCallObject* pThisCall =
                      dynamic_cast <ParkedCallObject *> (mCalls.findValue(&callIdTao));
                  if (pThisCall != NULL)
                  {
                     // Set up the call state for this dialog.
                     setUpParkedCallEstablished(callIdTao, addressTao, argTao);
                  }
                }
                else
                {
                   // Do the work for a call-retrieval call.
                   setUpRetrievalCall(callIdTao, addressTao);
                }
             }  // end is localConnection
             else
             {
                ParkedCallObject* pThisCall =
                    dynamic_cast <ParkedCallObject *> (mCalls.findValue(&callIdTao));
                if (pThisCall != NULL)
                {
                   if(pThisCall->getEstablished())
                   {
                      OsSysLog::add(FAC_PARK, PRI_DEBUG,
                                    "OrbitListener::handleMessage - %d "
                                    "remote setUpParkedCallEstablished",
                                    (int) taoEventId);
                      // Set up the call state for this dialog.
                      setUpParkedCallEstablished(callIdTao, addressTao, argTao);
                   }

                 }
             }  // end is not localConnection
         }
         break;


         case PtEvent::CONNECTION_DISCONNECTED:
         {
             if (!localConnectionTao)
             {
                OsSysLog::add(FAC_PARK, PRI_DEBUG,
                              "OrbitListener::handleMessage - %d "
                              "Call disconnected: callId '%s', address: '%s'",
                              (int) taoEventId, callIdTao.data(), addressTao.data());
                // See if the callId is in our list and if the address matches.
                ParkedCallObject* pDroppedCall =
                   dynamic_cast <ParkedCallObject *> (mCalls.findValue(&callIdTao));
                if (pDroppedCall == NULL)
                {
                   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                                 "OrbitListener::handleMessage - %d "
                                 "callId '%s' not found in the parked call list"
                                 , (int) taoEventId, callIdTao.data());
                }
                else if (addressTao.compareTo(pDroppedCall->getCurrentAddress()) == 0)
                {
                   // Addresses match, which means that this event
                   // indicates that the current leg of the call is ending, and
                   // so the logical call is ending.
                   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                                 "OrbitListener::handleMessage - %d "
                                 "found call object %p for '%s'",
                                 (int) taoEventId, pDroppedCall, callIdTao.data());
                   // Remove the call from the pool and clean up the call.
                   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                                 "OrbitListener::handleMessage - %d "
                                 "remove call object %p for '%s'",
                                 (int) taoEventId, pDroppedCall, callIdTao.data());
                   mCalls.destroy(&callIdTao);

                   // Drop the call
                   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                                 "OrbitListener::handleMessage - %d "
                                 "Dropping callId '%s' because received CONNECTION_DISCONNECTED",
                                 (int) taoEventId, callIdTao.data());
                   mpCallManager->drop(callIdTao);
                }
                else
                {
                   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                                 "OrbitListener::handleMessage - %d "
                                 "Address mismatch passed in '%s' / stored '%s'",
                                 (int) taoEventId, addressTao.data(),
                                 pDroppedCall->getCurrentAddress());
                }
             }
         }
         break;

         case PtEvent::CONNECTION_FAILED:
         {
            OsSysLog::add(FAC_PARK, PRI_WARNING,
                          "OrbitListener::handleMessage - %d "
                          "Connection failed: '%s'",
                          (int) taoEventId, callIdTao.data());

            // If this is a CONNECTION_FAILED on a transfer
            // pseudo-call, it means that a blind transfer failed.
            // We must unlock the call we were trying to transfer (so
            // that it can be picked up), and delete this transfer
            // call from mTransferCalls, as it no longer exists.
            {
               // Limit the scope of originalCallId, as it will become invalid
               // once we do mTransferCalls.destroy(&callId).
               UtlString* originalCallId =
                  dynamic_cast <UtlString*> (mTransferCalls.findValue(&callIdTao));
               if (originalCallId)
               {
                  OsSysLog::add(FAC_PARK, PRI_DEBUG,
                                "OrbitListener::handleMessage - %d "
                                "transfer - Call failed: "
                                "callId: '%s', address: '%s', originalCallId: '%s'",
                                (int) taoEventId, callIdTao.data(), addressTao.data(),
                                originalCallId->data());

                  // See if the originalCallId is one of our calls.
                  ParkedCallObject* pCall =
                     dynamic_cast <ParkedCallObject *> (mCalls.findValue(originalCallId));
                  if (pCall == NULL)
                  {
                     OsSysLog::add(FAC_PARK, PRI_DEBUG,
                                   "OrbitListener::handleMessage - %d "
                                   "No callId '%s' found in the parked call list",
                                   (int) taoEventId, originalCallId->data());
                  }
                  else
                  {
                     OsSysLog::add(FAC_PARK, PRI_DEBUG,
                                   "OrbitListener::handleMessage - %d "
                                   "found call object %p for '%s'",
                                   (int) taoEventId, pCall, callIdTao.data());
                     // Clear the transfer-in-progress indicator, because
                     // an attempt to transfer it has failed.
                     pCall->clearTransfer();
                  }
               }
            }

            // Delete the transfer call (if it is one).
            // This deletes *originalCallId, so do it after we are
            // done with originalCallId.
            OsSysLog::add(FAC_PARK, PRI_DEBUG,
                          "OrbitListener::handleMessage - %d "
                          "callId: '%s', address: '%s' removed from mTransferCalls",
                          (int) taoEventId, callIdTao.data(), addressTao.data());
            mTransferCalls.destroy(&callIdTao);

            // If this is a real call, remove it from our list.
            ParkedCallObject* pDroppedCall =
               dynamic_cast <ParkedCallObject *> (mCalls.findValue(&callIdTao));
            if (pDroppedCall)
            {
               // Remove the call from the pool and clean up the call
               OsSysLog::add(FAC_PARK, PRI_DEBUG,
                             "OrbitListener::handleMessage - %d "
                             "remove call object %p for '%s' from the list "
                             "due to failed connection",
                             (int) taoEventId, pDroppedCall, callIdTao.data());
               mCalls.destroy(&callIdTao);
            }

            OsSysLog::add(FAC_PARK, PRI_DEBUG,
                          "OrbitListener::handleMessage - %d "
                          "Dropping callId '%s' because received CONNECTION_DISCONNECTED",
                          (int) taoEventId, callIdTao.data());
            mpCallManager->drop(callIdTao);
         }
         break;


         case PtEvent::MULTICALL_META_TRANSFER_STARTED:
         case PtEvent::MULTICALL_META_REPLACE_STARTED:
         {
                // A transfer operation is starting.  Record the call-Id
                // of the transfer pseudo-call.
                if (localConnectionTao)
                {
                   UtlString transferCallIdTao = argTao[TAO_OFFER_PARAM_NEW_CALLID];
                   UtlString originalCallIdTao = argTao[TAO_TRANSFER_ENDED_ORIGINAL_CALLID];
                   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                                 "OrbitListener::handleMessage - %d "
                                 "transfer - started: "
                                 "add callId '%s' to mTransferCalls, original callid: '%s'",
                                 (int) taoEventId, transferCallIdTao.data(), originalCallIdTao.data());
                   // Record it.
                   mTransferCalls.insertKeyAndValue(new UtlString(transferCallIdTao),
                                                    new UtlString(originalCallIdTao));
                }
         }
         break;

         case PtEvent::MULTICALL_META_TRANSFER_ENDED:
         {
                // A MULTICALL_META_TRANSFER_ENDED is given on the
                // transfer pseudo-call immediately after the transfer is
                // initiated.  If the transfer succeeds, another
                // MULTICALL_META_TRANSFER_ENDED is given on the
                // *original* call.  We detect the second type of
                // MULTICALL_META_TRANSFER_ENDED and use it to remove the
                // transfer pseudo-call from our list of calls.  (Since
                // the transfer was successful, we don't need to unlock
                // the real call -- it will soon be terminated.)
                if (localConnectionTao)
                {
                   UtlString transferCallIdTao = argTao[TAO_TRANSFER_ENDED_TRANSFER_CALLID];
                   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                                 "OrbitListener::handleMessage - %d "
                                 "transfer - Transfer ended: callId '%s', "
                                 "transfer callid: '%s'",
                                 (int) taoEventId, callIdTao.data(), transferCallIdTao.data());

                   if ( !callIdTao.isEqual(&transferCallIdTao)
                       && mTransferCalls.find(&transferCallIdTao))
                   {
                      OsSysLog::add(FAC_PARK, PRI_DEBUG,
                                    "OrbitListener::handleMessage - %d "
                                    "transfer - Remove from mTransferCalls '%s'",
                                    (int) taoEventId, transferCallIdTao.data());
                      // Remove this call ID from the list of transfer pseudo-calls we
                      // are monitoring.
                      mTransferCalls.destroy(&transferCallIdTao);

                      // start hy
                      ParkedCallObject* pCall =
                          dynamic_cast <ParkedCallObject *> (mCalls.findValue(&callIdTao));

                      if(pCall)
                      {
                         // Transfer is complete; clearTransfer flag.
                         pCall->clearTransfer();
                      }
                      else
                      {
                          OsSysLog::add(FAC_PARK, PRI_DEBUG,
                                        "OrbitListener::handleMessage - %d "
                                        "Dropping callId '%s' on event MULTICALL_META_TRANSFER_ENDED"
                                        "without matching ParkedCallObject",
                                        (int) taoEventId, callIdTao.data());
                          mpCallManager->drop(callIdTao);
                      }
                   }
                }
         }
         break;

         case PtEvent::MULTICALL_META_REPLACE_ENDED:
         {
                // Only one MULTICALL_META_REPLACE_ENDED will occur, since
                // only one CpCall exists to hold the metaEvent.
                // The metaEvent is stopped(and removed) at the first of:
                // 1- ACK recieved for the new call (INVITE with REPLACES)
                // 2- 200 OK recieved for BYE to replaced dialog
                //
                // Either case sets transfer call id the new dialog's value.
                // All transfer state and info must be cleared here.
                // No call needs to be dropped, the replaced dialog is
                // terminated from SipConnection::processInviteRequest
                // and the CpCall continues to exist, with a new dialog.
                if (localConnectionTao)
                {
                   UtlString transferCallIdTao = argTao[TAO_TRANSFER_ENDED_TRANSFER_CALLID];
                   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                                 "OrbitListener::handleMessage - %d "
                                 "transfer - Transfer ended: callId '%s', "
                                 "transfer callid: '%s'",
                                 (int) taoEventId, callIdTao.data(), transferCallIdTao.data());

                   if ( mTransferCalls.find(&transferCallIdTao))
                   {
                      OsSysLog::add(FAC_PARK, PRI_DEBUG,
                                    "OrbitListener::handleMessage - %d "
                                    "transfer - Remove from mTransferCalls '%s'",
                                    (int) taoEventId, transferCallIdTao.data());
                      // Remove this call ID from the list of transfer pseudo-calls we
                      // are monitoring.
                      mTransferCalls.destroy(&transferCallIdTao);
                           // start hy
                      ParkedCallObject* pCall =
                          dynamic_cast <ParkedCallObject *> (mCalls.findValue(&callIdTao));

                      if(pCall)
                      {
                         // Transfer is complete; clearTransfer flag.
                         pCall->clearTransfer();
                         //mpCallManager->drop(transferCallIdTao);
                      }
                      // end hy
                   }
                   else
                   {
                       OsSysLog::add(FAC_PARK, PRI_DEBUG,
                                     "OrbitListener::handleMessage - %d "
                                     "transfer - '%s' not found in mTransferCalls ",
                                     (int) taoEventId, transferCallIdTao.data());
                   }
                }
          }
          break;
      } // end tao event switch
   } else if (rMsg.getMsgType() == OsMsg::OS_EVENT &&
              rMsg.getMsgSubType() == OsEventMsg::NOTIFY)
   {
      // An event notification.

      // The userdata of the original OsQueuedEvent was copied by
      // OsQueuedEvent::signal() into the userdata of this OsEventMsg.
      // The userdata is the mSeqNo of the ParkedCallObject, plus an
      // "enum notifyCodes" value indicating what sort of event it was.
      intptr_t userData;
      OsEventMsg* pEventMsg = dynamic_cast <OsEventMsg*> (&rMsg);
      pEventMsg->getUserData((void*&)userData);
      int seqNo;
      enum ParkedCallObject::notifyCodes type;
      ParkedCallObject::splitUserData(userData, seqNo, type);
      ParkedCallObject* pParkedCallObject = findBySeqNo(seqNo);

      // If we could locate the call this event applies to, act on the event.
      // (Otherwise, the call was dropped before we processed this event.)
      if (pParkedCallObject)
      {
         switch (type)
         {
         case ParkedCallObject::DTMF:
         {
            // Get the keycode.
            intptr_t keycode;
            pEventMsg->getEventData(keycode);
            keycode >>= 16;

            // Check that it is a keyup event.
            if ((keycode & 0x8000) != 0)
            {
               keycode &= 0x7FFF;
               OsSysLog::add(FAC_PARK, PRI_DEBUG,
                             "OrbitListener::handleMessage "
                             "DTMF keyup "
                             "keycode %" PRIdPTR "d, ParkedCallObject = %p",
                             keycode, pParkedCallObject);

               // Call the ParkedCallObject to process the keycode, which may
               // start a transfer..
               pParkedCallObject->keypress(keycode);
            }
            else
            {
               keycode &= 0x7FFF;
               OsSysLog::add(FAC_PARK, PRI_DEBUG,
                             "OrbitListener::handleMessage "
                             "DTMF keydown (ignored) "
                             "keycode %" PRIdPTR "d",
                             keycode);
               // Ignore it.
            }
         }
         break;

         case ParkedCallObject::PARKER_TIMEOUT:
            OsSysLog::add(FAC_PARK, PRI_DEBUG,
                          "OrbitListener::handleMessage "
                          "PARKER_TIMEOUT ParkedCallObject = %p",
                          pParkedCallObject);

            // Call the ParkedCallObject to start the transfer.
            pParkedCallObject->startBlindTransfer();
            break;

         case ParkedCallObject::MAXIMUM_TIMEOUT:
            OsSysLog::add(FAC_PARK, PRI_DEBUG,
                          "OrbitListener::handleMessage MAXIMUM_TIMEOUT "
                          "ParkedCallObject = %p",
                          pParkedCallObject);

            // Drop the call.
            mpCallManager->drop(pParkedCallObject->getCurrentCallId());
            break;

         case ParkedCallObject::TRANSFER_TIMEOUT:
            OsSysLog::add(FAC_PARK, PRI_DEBUG,
                          "OrbitListener::handleMessage TRANSFER_TIMEOUT "
                          "ParkedCallObject = %p",
                          pParkedCallObject);

            // Since the transfer methods of CallManager are "fire and forget",
            // a transfer that fails will leave the call in the same state
            // as it was originally.  So abandoning a transfer attempt requires
            // no action on our part.  But we do need to unlock the
            // ParkedCallObject to allow further transfer attempts.
            pParkedCallObject->clearTransfer();
            break;

         case ParkedCallObject::KEEPALIVE_TIMEOUT:
         {
            OsSysLog::add(FAC_PARK, PRI_DEBUG,
                          "OrbitListener::handleMessage KEEPALIVE_TIMEOUT "
                          "ParkedCallObject = %p",
                          pParkedCallObject);

            SipDialog dialog;
            // Check if the call manager knows about this callId, remove from list if not.
            // Another way to check if this callId is valid is to test the returned callId
            // in the SipDialog.
            // Sometimes getSipDialog does not fail but returns with an empty callId.
            // Note that getSipDialog looks up using the current Call-ID, not the original
            // Call-ID.
            OsStatus ret =
               mpCallManager->getSipDialog(pParkedCallObject->getCurrentCallId(),
                                           pParkedCallObject->getCurrentAddress(), dialog);
            UtlString tCallId;
            dialog.getCallId(tCallId);

            if (ret == OS_SUCCESS && !tCallId.isNull())
            {
               // Send a keep alive signal back to the caller
               pParkedCallObject->sendKeepAlive(MOH_USER_PART);;
            }
            else
            {
               OsSysLog::add(FAC_PARK, PRI_ERR,
                             "OrbitListener::handleMessage - "
                             "ParkedCallObject::KEEPALIVE_TIMEOUT Unknown callId '%s', "
                             "remove from list", pParkedCallObject->getCurrentCallId());

               UtlString tKey(pParkedCallObject->getCurrentCallId());
               mCalls.destroy(&tKey);
            }
            break;
         }

         }
      }
      else
      {
         OsSysLog::add(FAC_PARK, PRI_INFO,
                       "OrbitListener::handleMessage "
                       "ParkedCallObject(seqNo = %d) "
                       "deleted while message in queue",
                       seqNo);
      }
   }

   if (OsSysLog::willLog(FAC_PARK, PRI_DEBUG))
   {
       dumpCallsAndTransfers();
   }

   return TRUE;
}


void OrbitListener::dumpCallsAndTransfers()
{
   // Dump mCalls and mTransferCalls.
   {
      UtlHashMapIterator iterator(mCalls);
      UtlString *pKey;
      while ((pKey = dynamic_cast <UtlString*> (iterator())))
      {
         ParkedCallObject* pCall = dynamic_cast <ParkedCallObject*> (iterator.value());
         if (pCall)
         {
            const char *o = pCall->getOriginalCallId();
            const char *c = pCall->getCurrentCallId();
            OsSysLog::add(FAC_PARK, PRI_DEBUG,
                          "OrbitListener::dumpCallsAndTransfers "
                          "D ParkedCallObject(originalCallId = '%s', "
                          "currentCallId = '%s', isPickupCall = %d, seqNo = %d) ",
                          o,
                          strcmp(o, c) == 0 ? "(same)" : c,
                          pCall->isPickupCall(), pCall->getSeqNo());
         }
      }
   }
   {
      UtlHashMapIterator iterator(mTransferCalls);
      UtlString *pKey;
      while ((pKey = dynamic_cast <UtlString*> (iterator())))
      {
         OsSysLog::add(FAC_PARK, PRI_DEBUG,
                       "OrbitListener::dumpCallsAndTransfers D mTransferCalls('%s', '%s')",
                       pKey->data(),
                       (dynamic_cast <UtlString*> (mTransferCalls.findValue(pKey)))->data());
      }
   }
   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "OrbitListener::dumpCallsAndTransfers D end");
}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */


OsStatus OrbitListener::validateOrbit(const UtlString& callId,
                                      const UtlString& address,
                                      UtlString& orbit,
                                      UtlString& audio,
                                      int& timeout,
                                      int& keycode,
                                      int& capacity)
{
   // Retrieve the request URI
   SipDialog dialog;
   mpCallManager->getSipDialog(callId,
                               address,
                               dialog);
   UtlString request;
   dialog.getRemoteRequestUri(request);

   // If the call is initiated by the orbit
   // get the local user Id.
   if(request.isNull())
   {
      Url localIdentity;
      dialog.getLocalField(localIdentity);
      localIdentity.getUserId(orbit);
   }
   else
   {
      // Get orbit from request Uri.
      Url requestUri(request, TRUE);
      requestUri.getUserId(orbit);
   }

   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "OrbitListener::validateOrbit "
                 "request URI '%s', orbit '%s'",
                 request.data(), orbit.data());

   audio = "";

   // See if this is a music-on-hold call or an orbit call

   if (orbit == MOH_USER_PART)
   {
      mOrbitFileReader.getMusicOnHoldFile(audio);
      timeout = OrbitData::NO_TIMEOUT;
      keycode = OrbitData::NO_KEYCODE;
      capacity = OrbitData::UNLIMITED_CAPACITY;
   }
   else
   {
      OrbitData* orbit_data = mOrbitFileReader.findInOrbitList(orbit);
      if (orbit_data)
      {

         audio = orbit_data->mAudio;
         timeout = orbit_data->mTimeout;
         keycode = orbit_data->mKeycode;
         capacity = orbit_data->mCapacity;
      }

   }

   if (audio != "")
   {
      return OS_SUCCESS;
   }
   else
   {
      // Check if this is for MOH.  If so, use the default audio file.
      if (orbit == MOH_USER_PART)
      {
         audio = DEFAULT_MOH_AUDIO;
         return OS_SUCCESS;
      }
      else
      {
         return OS_FAILED;
      }
   }
}


ParkedCallObject* OrbitListener::getOldestCallInOrbit(const UtlString& orbit,
                                                      UtlString& callId,
                                                      UtlString& address)
{
   ParkedCallObject* pCall;
   ParkedCallObject* pReturn = NULL;
   UtlString *pKey;
   UtlString oldestKey("");
   UtlString oldestAddress("");
   OsTime oldest;
   OsTime parked;

   OsDateTime::getCurTime(oldest);

   UtlHashMapIterator iterator(mCalls);
   // Loop through the call list to find the oldest call.
   // Remove calls that are unknown by the Call Manager, in case we lost
   // track of them.
   while ((pKey = dynamic_cast <UtlString*> (iterator())))
   {
      pCall = dynamic_cast <ParkedCallObject*> (iterator.value());
      if (pCall)
      {
         // Found a call, make sure that the orbit matches, that it's older than anything seen
         // so far, that it is not a retrieval call, and that it's not being transferred
         // back to its parker.
         pCall->getTimeParked(parked);
         OsSysLog::add(FAC_PARK, PRI_DEBUG,
                       "OrbitListener::getOldestCallInOrbit "
                       "Entry for callId '%s', orbit '%s', time %ld.%06ld, "
                       "isPickupCall %d, transferInProgress %d",
                       pKey->data(), pCall->getOrbit(),
                       (long) parked.seconds(), (long) parked.usecs(),
                       pCall->isPickupCall(), pCall->transferInProgress());
         if (orbit.compareTo(pCall->getOrbit()) == 0 &&
             parked <= oldest &&
             !pCall->isPickupCall() &&
             !pCall->transferInProgress())
         {
            SipDialog dialog;
            // Check if the call manager knows about this callId, remove from list if not.
            // Another way to check if this callId is valid is to test the returned callId
            // in the SipDialog.
            // Sometimes getSipDialog does not fail but returns with an empty callId.
            // Note that getSipDialog looks up using the current Call-ID, not the original
            // Call-ID.
            OsStatus ret =
               mpCallManager->getSipDialog(pCall->getCurrentCallId(),
                                           pCall->getCurrentAddress(), dialog);
            UtlString tCallId;
            dialog.getCallId(tCallId);

            if (ret == OS_SUCCESS && !tCallId.isNull())
            {
               // Copy 'new' oldest data
               oldest = parked;
               oldestKey = *pKey;
               oldestAddress = pCall->getCurrentAddress();
               pReturn = pCall;

               OsSysLog::add(FAC_PARK, PRI_DEBUG,
                             "OrbitListener::getOldestCallInOrbit "
                             "Valid callId '%s', address '%s'",
                             oldestKey.data(), oldestAddress.data());
            }
            else
            {
               OsSysLog::add(FAC_PARK, PRI_ERR,
                             "OrbitListener::getOldestCallInOrbit "
                             "Unknown callId '%s', remove from list",
                             pKey->data());
               mCalls.destroy(pKey);
            }
         }
      }
      else
      {
         OsSysLog::add(FAC_PARK, PRI_DEBUG,
                       "OrbitListener::getOldestCallInOrbit "
                       "No call object found for callId '%s'",
                       pKey->data());
      }
   }
   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "OrbitListener::getOldestCallInOrbit "
                 "Returning pCall %p, callId '%s', address '%s'",
                 pReturn, oldestKey.data(), oldestAddress.data());
   callId = oldestKey;
   address = oldestAddress;
   return pReturn;
}


int OrbitListener::getNumCallsInOrbit(const UtlString& orbit,
                                      UtlBoolean onlyAvailable)
{
   ParkedCallObject* pCall;
   UtlString *pKey;
   int callsInOrbit = 0;

   UtlHashMapIterator iterator(mCalls);
   // Loop through the call list to count calls in this orbit.
   // Remove calls that are unknown by the Call Manager, in case we lost
   // track of them.
   while ((pKey = dynamic_cast <UtlString*> (iterator())))
   {
      pCall = dynamic_cast <ParkedCallObject*> (iterator.value());
      if (pCall)
      {
         OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::getNumCallsInOrbit "
                       "Entry for callId '%s', orbit '%s'",
                       pKey->data(), pCall->getOrbit());
         // Found a call, make sure that the orbit matches and
         // that it is not a retrieval call.
         // If onlyAvailable, check that transferInProgress() is FALSE.
         if (orbit.compareTo(pCall->getOrbit()) == 0 &&
             !pCall->isPickupCall() &&
             (!onlyAvailable || !pCall->transferInProgress()))
         {
            SipDialog dialog;
            // Check if the call manager knows about this callId, remove from list if not.
            // Another way to check if this callId is valid is to test the returned callId
            // in the SipDialog.
            // Sometimes getSipDialog does not fail but returns with an empty callId.
            // Note that getSipDialog looks up using the current Call-ID, not the original
            // Call-ID.
            OsStatus ret =
               mpCallManager->getSipDialog(pCall->getCurrentCallId(),
                                           pCall->getCurrentAddress(), dialog);
            UtlString tCallId;
            dialog.getCallId(tCallId);

            if (ret == OS_SUCCESS && !tCallId.isNull())
            {
               // Increment the count of calls in this orbit.
               callsInOrbit++;
               OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::getNumCallsInOrbit "
                             "Valid callId '%s', address '%s'",
                             pKey->data(), pCall->getCurrentAddress());
            }
            else
            {
               OsSysLog::add(FAC_PARK, PRI_ERR, "OrbitListener::getNumCallsInOrbit "
                             "Unknown callId '%s', remove from list",
                             pKey->data());
               mCalls.destroy(pKey);
            }
         }
      }
      else
      {
         OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::getNumCallsInOrbit "
                       "No call object found for callId '%s'",
                       pKey->data());
      }
   }
   OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::getNumCallsInOrbit - Returning %d",
                 callsInOrbit);
   return callsInOrbit;
}


ParkedCallObject* OrbitListener::findBySeqNo(int seqNo)
{
   UtlString *pKey;
   ParkedCallObject* pCall;
   ParkedCallObject* ret;
   UtlBoolean found;

   UtlHashMapIterator iterator(mCalls);
   // Loop through the call list.
   ret = NULL;
   found = FALSE;
   while (!found &&
          (pKey = dynamic_cast <UtlString*> (iterator())))
   {
      pCall = dynamic_cast <ParkedCallObject*> (iterator.value());
      if (pCall)
      {
         // Found a call, check if the seqNo matches.
         if (seqNo == pCall->getSeqNo())
         {
            ret = pCall;
            found = TRUE;
         }
      }
   }

   return ret;
}


bool OrbitListener::isCallRetrievalInvite(const char* callId,
                                          const char* address)
{
   bool bRet = false;

   // Retrieve the request URI
   SipDialog dialog;
   mpCallManager->getSipDialog(callId,
                               address,
                               dialog);
   UtlString request;
   dialog.getRemoteRequestUri(request);

   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "OrbitListener::isCallRetrievalInvite "
                 "remote request URI '%s'", request.data());

   Url requestURI(request, TRUE);
   UtlString urlParam;

   if (requestURI.getUrlParameter(CALL_RETRIEVE_URI_PARM, urlParam))
   {
      if (urlParam.compareTo(CALL_RETRIEVE_URI_VALUE, UtlString::ignoreCase) == 0)
      {
         bRet = true;
         OsSysLog::add(FAC_PARK, PRI_DEBUG,
                       "OrbitListener::isCallRetrievalInvite "
                       "URL parameter '%s=%s' found ",
                       CALL_RETRIEVE_URI_PARM, CALL_RETRIEVE_URI_VALUE);
      }
   }
   else
   {
      OsSysLog::add(FAC_PARK, PRI_DEBUG,
                    "OrbitListener::isCallRetrievalInvite "
                    "URL parameter '%s' not found ",
                    CALL_RETRIEVE_URI_PARM);
   }
   return bRet;
}


void OrbitListener::dumpTaoMessageArgs(TaoObjHandle eventId, TaoString& args)
{
   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "OrbitListener::dumpTaoMessageArgs Tao event id: %ld local: %s args: %d",
                 eventId, args[TAO_OFFER_PARAM_LOCAL_CONNECTION], args.getCnt()) ;

   int argc = args.getCnt();
   for(int argIndex = 0; argIndex < argc; argIndex++)
   {
      OsSysLog::add(FAC_PARK, PRI_DEBUG, "\targ[%d]=\"%s\"", argIndex, args[argIndex]);
   }
}


// Set up the data structures for a new call that isn't a call retrieval call.
// This routine does the operations activated by CONNECTION_OFFERED.
// The call may be a new call to be parked, or a new dialog that
// replaces a dialog that was parked.
void OrbitListener::setUpParkedCallOffered(const UtlString& callId,
                                           const UtlString& address,
                                           const UtlString& orbit,
                                           const UtlString& audio,
                                           int timeout,
                                           int keycode,
                                           int capacity,
                                           const TaoString& arg)
{
   // Look up callId in list of calls to see if this is
   // a replacement of a call we already have parked.
   ParkedCallObject* pThisCall =
      dynamic_cast <ParkedCallObject *> (mCalls.findValue(&callId));
   if (pThisCall == NULL)
   {
      //
      // This is a new call.
      //
      OsSysLog::add(FAC_PARK, PRI_DEBUG,
                    "OrbitListener::setUpParkedCallOffered - New call");
      pThisCall = new ParkedCallObject(orbit, mpCallManager,
                                       callId, address, audio, false,
                                       this->getMessageQueue(), mLifetime,
                                       mBlindXferWait, mKeepAliveTime);
      // Put it in the list of parked calls
      mCalls.insertKeyAndValue(new UtlString(callId), pThisCall);
      OsSysLog::add(FAC_PARK, PRI_DEBUG,
                    "OrbitListener::setUpParkedCallOffered "
                    "inserting ParkedCallObject for new call %p for '%s'",
                    pThisCall, callId.data());
   }
   else
   {
      // This is a new leg of an existing parked call, which should not
      // have a CONNECTION_OFFERED event.
      OsSysLog::add(FAC_PARK, PRI_ERR,
                    "OrbitListener::setUpParkedCallOffered "
                    "CONNECTION_OFFERED seen for replacing dialog '%s'",
                    callId.data());
   }
}


// Set up the call state for a new call that isn't a call retrieval call.
// (The data structures have been set up by setUpParkedCallOffered.)
// This routine does the operations activated by CONNECTION_ESTABLISHED.
// Note that a dialog that replaces another dialog generates a
// CONNECTION_ESTABLISHED message but not a CONNECTION_OFFERED message.
// The call may be a new call to be parked, or a new dialog that replaces a dialog
// that was parked.
void OrbitListener::setUpParkedCallEstablished(const UtlString& callId,
                                               const UtlString& address,
                                               const TaoString& arg)
{
   // Add a dummy DTMF listener to the call.
   // This listener doesn't process DTMF, but the
   // addToneListener() seems to be needed to
   // activate DTMF listening via enableDtmfEvent().
   mpCallManager->addToneListener(callId.data(), &mListener);

   // Get a copy of the INVITE that we received or sent
   // to start this dialog.  Look for a Referred-By
   // header, which indicates that this is a parked
   // dialog and needs to have a timer set.  (If there
   // is no Referred-By, the caller may have gotten
   // here by direct dialing, or this may be an initial
   // leg in a consultative transfer.)
   SipMessage invite;
   UtlString parker;
   OsStatus s = mpCallManager->getInvite(callId.data(),
                                         address.data(),
                                         invite);
   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "OrbitListener::setUpParkedCallEstablished "
                 "getInvite returned %d",
                 s);
   if (s == OS_SUCCESS)
   {
      if (OsSysLog::willLog(FAC_PARK, PRI_DEBUG))
      {
         UtlString text;
         ssize_t length;
         invite.getBytes(&text, &length);
         OsSysLog::add(FAC_PARK, PRI_DEBUG,
                       "OrbitListener::setUpParkedCallEstablished "
                       "getInvite: INVITE is '%s'",
                       text.data());
      }
      invite.getReferredByField(parker);
      OsSysLog::add(FAC_PARK, PRI_DEBUG,
                    "OrbitListener::setUpParkedCallEstablished "
                    "parker = '%s'",
                    parker.data());
   }
   // If !isNull.parker(), then this dialog has been parked by
   // the user with URI 'parker'.

   // Look up callId in list of calls to see if this is
   // a replacement of a call we already have parked.
   ParkedCallObject* pThisCall =
      dynamic_cast <ParkedCallObject *> (mCalls.findValue(&callId));
   if (pThisCall != NULL)
   {
      if (!pThisCall->getEstablished())
      {
         // pThisCall has not received CONNECTION_ESTABLISHED before, which
         // means that it was created by the current call.  Finish setting
         // up the parked call.
         OsSysLog::add(FAC_PARK, PRI_DEBUG,
                       "OrbitListener::setUpParkedCallEstablished "
                       "- New call");

         pThisCall->setEstablished();

         // Get the orbit information.
         // Since the call was accepted, the orbit should be valid.
         UtlString orbit;
         UtlString audio;
         int timeout, keycode, capacity;
         validateOrbit(callId, address, orbit, audio,
                       timeout, keycode, capacity);

         // Create a player and start to play out the file
         if (pThisCall->playAudio() != OS_SUCCESS)
         {
            OsSysLog::add(FAC_PARK, PRI_WARNING,
                          "OrbitListener::setUpParkedCallEstablished "
                          "No audio for callId '%s' because playAudio() failed",
                          callId.data());
         }
         // Start the time-out timer if necessary.
         pThisCall->startEscapeTimer(parker, timeout, keycode);
      }
      else
      {
         // The current call is an additional leg of a call being transferred
         // (by the other end of the call, not the Park Server),
         // and pThisCall was created by the previously existing call.
         OsSysLog::add(FAC_PARK, PRI_DEBUG,
                       "OrbitListener::setUpParkedCallEstablished "
                       "change the call address from '%s' to '%s'",
                       pThisCall->getCurrentAddress(), address.data());
         // Update the remote address of the dialog
         pThisCall->setCurrentAddress(address);
         OsSysLog::add(FAC_PARK, PRI_DEBUG,
                       "OrbitListener::setUpParkedCallEstablished - dump pThisCall: "
                       "current address '%s', original address '%s'",
                       pThisCall->getCurrentAddress(),
                       pThisCall->getOriginalAddress());


         // Audio continues playing from the previous dialog's media processing.

         // Check if Tao message has enough parameters to contain a new call Id.
         if (arg.getCnt() > TAO_OFFER_PARAM_NEW_CALLID)
         {
            // Update the current call-ID of the call.
            pThisCall->setCurrentCallId(arg[TAO_OFFER_PARAM_NEW_CALLID]);
         }
         else
         {
            OsSysLog::add(FAC_PARK, PRI_ERR,
                          "OrbitListener::setUpParkedCallEstablished "
                          "Tao message does not contain new callId for call '%s'",
                          callId.data());
         }

         UtlString orbit;
         UtlString audio;
         int timeout, keycode, capacity;
         validateOrbit(callId, address, orbit, audio,
                       timeout, keycode, capacity);
         if (pThisCall->playAudio() != OS_SUCCESS)
         {
            OsSysLog::add(FAC_PARK, PRI_WARNING,
                          "OrbitListener::setUpParkedCallEstablished "
                          "No audio for callId= %s because playAudio() failed",
                          callId.data());
         }
         pThisCall->startEscapeTimer(parker, timeout, keycode);
      }
   }
   else
   {
      // No recorded call could be found with the (original) Call-Id.
      // This should never happen, as the original call should have gotten
      // a CONNECTION_OFFERED.
      OsSysLog::add(FAC_PARK, PRI_ERR,
                    "OrbitListener::setUpParkedCallEstablished "
                    "No recorded call for callId '%s'",
                    callId.data());
   }
}


// Do the work for a call-retrieval call.
void OrbitListener::setUpRetrievalCall(const UtlString& callId,
                                       const UtlString& address)
{
   // This is a call retrieval
   // During offering we checked that the orbit was OK and that there was
   // a call to retrieve.
   UtlString orbit, audio;
   int timeout, keycode, capacity;

   // Get Orbit out of the requestUri and use that to find the oldest call.
   validateOrbit(callId, address, orbit, audio, timeout,
                 keycode, capacity);

   // Find oldest call for the orbit.
   UtlString retrievedCallId;
   UtlString retrievedAddress;
   ParkedCallObject* pCallToRetrieve =
      getOldestCallInOrbit(orbit, retrievedCallId, retrievedAddress);
   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "OrbitListener::setUpRetrievalCall "
                 "retrieving parked call from orbit '%s', call '%s', address '%s'",
                 orbit.data(), retrievedCallId.data(), retrievedAddress.data());

   if (pCallToRetrieve)
   {
      OsSysLog::add(FAC_PARK, PRI_DEBUG,
                    "OrbitListener::setUpRetrievalCall "
                    "transferring original callId '%s', address '%s' "
                    "to callId '%s', address '%s'",
                    retrievedCallId.data(), retrievedAddress.data(),
                    callId.data(), address.data());

      mpCallManager->transfer(retrievedCallId, retrievedAddress,
                              callId, address,
                              false /* do not hold before transfer */
                              );
      // Add this call to the park list so it can be cleaned up correctly, mark it as
      // a call retrieval,
      ParkedCallObject *pExecutingCall =
         new ParkedCallObject(orbit, mpCallManager,
                              callId, address, "", true,
                              this->getMessageQueue(), mLifetime, mBlindXferWait, mKeepAliveTime);
      mCalls.insertKeyAndValue(new UtlString(callId), pExecutingCall);
      OsSysLog::add(FAC_PARK, PRI_DEBUG,
                    "OrbitListener::setUpRetrievalCall "
                    "inserting ParkedCallObject for new call %p for '%s'",
                    pExecutingCall, callId.data());
      // Start the deadman timer on the call being retrieved, for a
      // relatively short time.
      pCallToRetrieve->markTransfer(mConsXferWait);
   }
   else
   {
      // Drop the call
      OsSysLog::add(FAC_PARK, PRI_WARNING,
                    "OrbitListener::setUpRetrievalCall "
                    "Drop callId '%s' due to not finding a parked call",
                    callId.data());
      mpCallManager->drop(callId);
   }
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */


// Implementation of the dummy DTMF listener.

// This is used to provide a listener for CallManager::addToneListener.
// It does noting, but the addToneListener call seems to be necessary to
// make CallManager::enableDtmfEvent work.

// Constructor
DummyListener::DummyListener() :
   TaoAdaptor("DummyListener-%d")
{
}

// Copy constructor
DummyListener::DummyListener(const DummyListener& rDummyListener)
{
}

// Destructor
DummyListener::~DummyListener()
{
}

// Assignment operator
DummyListener&
DummyListener::operator=(const DummyListener& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

UtlBoolean DummyListener::handleMessage(OsMsg& rMsg)
{
   // Do nothing.
   return(TRUE);
}

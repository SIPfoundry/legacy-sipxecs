//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <cp/PresenceDialInServer.h>
#include <utl/UtlHashMapIterator.h>
#include <utl/UtlVoidPtr.h>
#include <tao/TaoMessage.h>
#include <tao/TaoString.h>
#include <cp/CallManager.h>
#include <net/SipDialog.h>
#include <os/OsFS.h>
#include <xmlparser/tinyxml.h>
#include <os/OsDateTime.h>
#include <os/OsTimer.h>
#include <os/OsEventMsg.h>
#include "ConfirmationTone.h"
#include "DialTone.h"
#include "BusyTone.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define TAO_OFFER_PARAM_CALLID             0
#define TAO_OFFER_PARAM_ADDRESS            2
#define TAO_OFFER_PARAM_LOCAL_CONNECTION   6

#define SECONDS_DELAY 3

#define CONFIG_SETTING_SIGN_IN_CODE        "SIP_PRESENCE_SIGN_IN_CODE"
#define CONFIG_SETTING_SIGN_OUT_CODE       "SIP_PRESENCE_SIGN_OUT_CODE"
#define CONFIG_SETTING_SIGN_IN_AUDIO       "SIP_PRESENCE_SIGN_IN_CONFIRMATION_AUDIO"
#define CONFIG_SETTING_SIGN_OUT_AUDIO      "SIP_PRESENCE_SIGN_OUT_CONFIRMATION_AUDIO"
#define CONFIG_SETTING_ERROR_AUDIO         "SIP_PRESENCE_ERROR_AUDIO"


//#define DEBUGGING 1

// STATIC VARIABLE INITIALIZATIONS

// Private class to contain an incoming call for given callId
class CallContainer : public UtlString
{
public:
    CallContainer(UtlString& callId, UtlString& address, OsMsgQ* incomingQ);

    virtual ~CallContainer();

    // parent UtlString contains the resourceId and eventTypeKey
    UtlString mCallId;
    UtlString mAddress;
    OsQueuedEvent* mQueuedEvent;
    OsTimer* mTimer;

private:
    //! DISALLOWED accendental copying
    CallContainer(const CallContainer& rCallContainer);
    CallContainer& operator=(const CallContainer& rhs);

};

CallContainer::CallContainer(UtlString& callId, UtlString& address, OsMsgQ* incomingQ)
{
   mCallId = callId;
   mAddress = address;

   mQueuedEvent = new OsQueuedEvent(*incomingQ, (void*)this);
   mTimer = new OsTimer(*mQueuedEvent);
}

CallContainer::~CallContainer()
{
   if (mQueuedEvent)
   {
      delete mQueuedEvent;
   }

   if (mTimer)
   {
      delete mTimer;
   }
}

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PresenceDialInServer::PresenceDialInServer(CallManager* callManager,
                                           OsConfigDb* configFile)
   : mLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
   mpCallManager = callManager;

   if (configFile->get(CONFIG_SETTING_SIGN_IN_CODE, mSignInFC) != OS_SUCCESS)
   {
      mSignInFC  = DEFAULT_SIGNIN_FEATURE_CODE;
   }

   if (configFile->get(CONFIG_SETTING_SIGN_OUT_CODE, mSignOutFC) != OS_SUCCESS)
   {
      mSignOutFC = DEFAULT_SIGNOUT_FEATURE_CODE;
   }

   if (configFile->get(CONFIG_SETTING_SIGN_IN_AUDIO, mSignInConfirmationAudio) != OS_SUCCESS)
   {
      mSignInConfirmationAudio = NULL;
   }

   if (configFile->get(CONFIG_SETTING_SIGN_OUT_AUDIO, mSignOutConfirmationAudio) != OS_SUCCESS)
   {
      mSignOutConfirmationAudio = NULL;
   }

   if (configFile->get(CONFIG_SETTING_ERROR_AUDIO, mErrorAudio) != OS_SUCCESS)
   {
      mErrorAudio = NULL;
   }

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "PresenceDialInServer:: configuration for PresenceDialIn:");
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "PresenceDialInServer:: signInFeatureCode = %s", mSignInFC.data());
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "PresenceDialInServer:: signOutFeatureCode = %s", mSignOutFC.data());
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "PresenceDialInServer:: signInConfirmationAudio = %s",
                 (mSignInConfirmationAudio == NULL ? "confirmation tone" : mSignInConfirmationAudio.data()));
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "PresenceDialInServer:: signOutConfirmationAudio = %s",
                 (mSignOutConfirmationAudio == NULL ? "dial tone" : mSignOutConfirmationAudio.data()));
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "PresenceDialInServer:: errorAudio = %s",
                 (mErrorAudio == NULL ? "busy tone" : mErrorAudio.data()));

   mpIncomingQ = getMessageQueue();
}


//Destructor
PresenceDialInServer::~PresenceDialInServer()
{
   if (!mStateChangeNotifiers.isEmpty())
   {
      mStateChangeNotifiers.destroyAll();
   }
}


/* ============================ MANIPULATORS ============================== */

UtlBoolean PresenceDialInServer::handleMessage(OsMsg& rMsg)
{
   SipDialog sipDialog;
   UtlString sipDialogContent;
   Url requestUrl;
   UtlString entity;
   UtlString featureCode;
   Url contactUrl;
   UtlString contact;

    int msgType = rMsg.getMsgType();
    int msgSubType = rMsg.getMsgSubType();

   // React to telephony events
   if(msgSubType == TaoMessage::EVENT)
   {
      TaoMessage* taoMessage = dynamic_cast <TaoMessage*> (&rMsg);

      int taoEventId = taoMessage->getTaoObjHandle();
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

            mpCallManager->acceptConnection(callId, address);
            mpCallManager->answerTerminalConnection(callId, address, "*");

            if (mpCallManager->getSipDialog(callId, address, sipDialog) ==
                OS_SUCCESS)
            {
#ifdef DEBUGGING
               sipDialog.toString(sipDialogContent);
               OsSysLog::add(FAC_SIP, PRI_DEBUG, "PresenceDialInServer::handleMessage sipDialog = %s",
                             sipDialogContent.data());
#endif
               sipDialog.getRemoteRequestUri(entity);

               OsSysLog::add(FAC_SIP, PRI_DEBUG, "PresenceDialInServer::handleMessage Call arrived: callId %s address %s requestUrl %s",
                             callId.data(), address.data(), entity.data());

               if (entity.isNull())
               {
                  OsSysLog::add(FAC_SIP, PRI_WARNING, "PresenceDialInServer::handleMessage Call arrived: callId %s address %s without requestUrl",
                                callId.data(), address.data());
               }
            }
            else
            {
               OsSysLog::add(FAC_SIP, PRI_ERR,
                             "PresenceDialInServer::handleMessage CONNECTION_OFFERED Could not find dialog callId %s address %s",
                             callId.data(), address.data());
            }

            break;

         case PtEvent::CONNECTION_ESTABLISHED:
            if (localConnection)
            {
               if (mpCallManager->getSipDialog(callId, address, sipDialog) ==
                   OS_SUCCESS)
               {
#ifdef DEBUGGING
                  sipDialog.toString(sipDialogContent);
                  OsSysLog::add(FAC_SIP, PRI_DEBUG, "PresenceDialInServer::handleMessage sipDialog = %s",
                                sipDialogContent.data());
#endif
                  sipDialog.getRemoteRequestUri(entity);

                  OsSysLog::add(FAC_SIP, PRI_DEBUG, "Call connected: callId %s address %s with request %s",
                                callId.data(), address.data(), entity.data());

                  if (entity.isNull())
                  {
                     OsSysLog::add(FAC_SIP, PRI_WARNING, "PresenceDialInServer::handleMessage Call connected: callId %s address %s without requestUrl",
                                   callId.data(), address.data());
                  }
                  else
                  {
                     // Create a CallContainer object and insert it into the call list
                     CallContainer* thisCall = new CallContainer(callId, address, mpIncomingQ);
                     mCalls.insertKeyAndValue(new UtlString(callId), thisCall);

                     // Get the feature code from the request URI.
                     requestUrl = Url(entity);
                     requestUrl.getUserId(featureCode);
                     contactUrl = Url(address);
                     contactUrl.getIdentity(contact);
                     OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                   "PresenceDialInServer::handleMessage contact '%s' request feature code '%s'",
                                   contact.data(), featureCode.data());

                     if (featureCode.compareTo(mSignInFC) == 0)
                     {
                        if (notifyStateChange(contact, true))
                        {
                           if (mSignInConfirmationAudio == NULL)
                           {
                              // Play built-in default sign-in confirmation audio tone
                              mpCallManager->bufferPlay(callId,
                                                        (char*)confirmationTone,
                                                        confirmationToneLength,
                                                        RAW_PCM_16,
                                                        FALSE, FALSE, TRUE);
                           }
                           else
                           {
                              // Play user specified sign-in confirmation audio
                              mpCallManager->audioPlay(callId,
                                                       mSignInConfirmationAudio,
                                                       FALSE, FALSE, TRUE);
                           }
                        }
                        else
                        {
                           if (mErrorAudio == NULL)
                           {
                              // Play built-in default error audio tone
                              mpCallManager->bufferPlay(callId,
                                                        (char*)busyTone,
                                                        busyToneLength,
                                                        RAW_PCM_16,
                                                        FALSE, FALSE, TRUE);
                           }
                           else
                           {
                              // Play user specified error audio
                              mpCallManager->audioPlay(callId,
                                                       mErrorAudio,
                                                       FALSE, FALSE, TRUE);
                           }
                           OsSysLog::add(FAC_SIP, PRI_DEBUG, "PresenceDialInServer::handleMessage contact %s has already signed in",
                                         contact.data());
                        }
                     }

                     if (featureCode.compareTo(mSignOutFC) == 0)
                     {
                        if (notifyStateChange(contact, false))
                        {
                           if (mSignOutConfirmationAudio == NULL)
                           {
                              // Play built-in default sign-out confirmation audio tone
                              mpCallManager->bufferPlay(callId,
                                                        (char*)dialTone,
                                                        dialToneLength,
                                                        RAW_PCM_16,
                                                        FALSE, FALSE, TRUE);
                           }
                           else
                           {
                              // Play user specified sign-out confirmation audio
                              mpCallManager->audioPlay(callId, mSignOutConfirmationAudio, FALSE, FALSE, TRUE);
                           }
                        }
                        else
                        {
                           if (mErrorAudio == NULL)
                           {
                              // Play built-in default error audio tone
                              mpCallManager->bufferPlay(callId,
                                                        (char*)busyTone,
                                                        busyToneLength,
                                                        RAW_PCM_16,
                                                        FALSE, FALSE, TRUE);
                           }
                           else
                           {
                              // Play user specified error audio
                              mpCallManager->audioPlay(callId,
                                                       mErrorAudio,
                                                       FALSE, FALSE, TRUE);
                           }
                           OsSysLog::add(FAC_SIP, PRI_DEBUG, "PresenceDialInServer::handleMessage contact %s has already signed out",
                                         contact.data());
                        }
                     }

                     OsTime timerTime(SECONDS_DELAY, 0);
                     thisCall->mTimer->oneshotAfter(timerTime);
                  }
               }
               else
               {
                  OsSysLog::add(FAC_SIP, PRI_ERR,
                                "PresenceDialInServer::handleMessage CONNECTION_ESTABLISHED Could not find dialog callId %s address %s",
                                callId.data(), address.data());
               }
            }
            break;

         case PtEvent::CONNECTION_DISCONNECTED:
            if (!localConnection)
            {
               mpCallManager->getSipDialog(callId, address, sipDialog);
               if (mpCallManager->getSipDialog(callId, address, sipDialog) ==
                   OS_SUCCESS)
               {
#ifdef DEBUGGING
                  sipDialog.toString(sipDialogContent);
                  OsSysLog::add(FAC_SIP, PRI_DEBUG, "PresenceDialInServer::handleMessage sipDialog = %s",
                                sipDialogContent.data());
#endif
                  sipDialog.getLocalContact(requestUrl);
                  requestUrl.getIdentity(entity);

                  OsSysLog::add(FAC_SIP, PRI_DEBUG, "PresenceDialInServer::handleMessage Call dropped: %s address %s with entity %s",
                                callId.data(), address.data(), entity.data());

                  if (entity.isNull())
                  {
                     OsSysLog::add(FAC_SIP, PRI_WARNING, "PresenceDialInServer::handleMessage Call dropped: callId %s address %s without requestUri",
                                   callId.data(), address.data());
                  }

                  // We have to drop the call from this end, also.
                  mpCallManager->drop(callId);
               }
               else
               {
                  OsSysLog::add(FAC_SIP, PRI_ERR,
                                "PresenceDialInServer::handleMessage CONNECTION_DISCONNECTED Could not find dialog callId %s address %s",
                                callId.data(), address.data());
               }

               // Remove the CallContainer object from the call list
               mCalls.destroy(&callId);
            }

            break;

         case PtEvent::CONNECTION_FAILED:
            OsSysLog::add(FAC_SIP, PRI_WARNING, "Connection failed on call: %s", callId.data());

            break;
      }
   }
   else if(msgType == OsMsg::OS_EVENT && msgSubType == OsEventMsg::NOTIFY)
   {
      CallContainer* thisCall = NULL;
      ((OsEventMsg&)rMsg).getUserData((void*&)thisCall);
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "PresenceDialInServer::handleMessage dropping call %s",
                    thisCall->mCallId.data());
      mpCallManager->drop(thisCall->mCallId.data());
   }

   return(TRUE);
}


/* ============================ ACCESSORS ================================= */
void PresenceDialInServer::addStateChangeNotifier(const char* fileUrl, StateChangeNotifier* notifier)
{
   mLock.acquire();
   UtlString* name = new UtlString(fileUrl);
   UtlVoidPtr* value = new UtlVoidPtr(notifier);
   mStateChangeNotifiers.insertKeyAndValue(name, value);
   mLock.release();
}

void PresenceDialInServer::removeStateChangeNotifier(const char* fileUrl)
{
   mLock.acquire();
   UtlString name(fileUrl);
   mStateChangeNotifiers.destroy(&name);
   mLock.release();
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */


void PresenceDialInServer::dumpTaoMessageArgs(unsigned char eventId, TaoString& args)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "===>Message type: %d args:\n", eventId) ;

   int argc = args.getCnt();
   for(int argIndex = 0; argIndex < argc; argIndex++)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "\targ[%d]=\"%s\"", argIndex, args[argIndex]);
   }
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Returns TRUE if the requested state is different from the current state.
bool PresenceDialInServer::notifyStateChange(UtlString& contact, bool signIn)
{
   bool result = false;

   // Loop through the notifier list
   UtlString* listUri;
   StateChangeNotifier* notifier;
   UtlVoidPtr* container;
   Url contactUrl(contact);
   mLock.acquire();
   UtlHashMapIterator iterator(mStateChangeNotifiers);
   while ((listUri = dynamic_cast <UtlString *> (iterator())) != NULL)
   {
      container = dynamic_cast <UtlVoidPtr *> (mStateChangeNotifiers.findValue(listUri));
      notifier = (StateChangeNotifier *) container->getValue();

      OsSysLog::add(FAC_SIP, PRI_INFO,
                    "PresenceDialInServer::notifyStateChange contact '%s' ==> %s",
                    contact.data(),
                    signIn ? "SIGN_IN" : "SIGN_OUT");
      result = notifier->setStatus(contactUrl,
                                   signIn ?
                                   StateChangeNotifier::PRESENT :
                                   StateChangeNotifier::AWAY);
      OsSysLog::add(FAC_SIP, PRI_INFO,
                    "PresenceDialInServer::notifyStateChange notifier->setStatus returned %d",
                    result);
   }
   mLock.release();

   return result;
}


/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

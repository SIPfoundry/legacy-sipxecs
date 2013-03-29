//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include "os/OsLogger.h"
#include "os/OsLock.h"
#include "utl/UtlSListIterator.h"
#include "AppearanceGroupSet.h"
#include "AppearanceGroup.h"
#include "ResourceListMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// Resubscription period.
#define RESUBSCRIBE_PERIOD 3600

// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType AppearanceGroupSet::TYPE = "AppearanceGroupSet";


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
AppearanceGroupSet::AppearanceGroupSet(AppearanceAgent* appearanceAgent) :
   mAppearanceAgent(appearanceAgent),
   mVersion(0),
   _appearanceTimers(appearanceAgent->getAppearanceAgentTask().getMessageQueue())
{
   Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                 "AppearanceGroupSet:: this = %p",
                 this);
}

// Destructor
AppearanceGroupSet::~AppearanceGroupSet()
{
   Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                 "AppearanceGroupSet::~ this = %p",
                 this);

   _appearanceTimers.stop();
}


/* ============================ MANIPULATORS ============================== */

bool AppearanceGroupSet::addAppearanceByTimer(
        const UtlString& callidContact,
        UtlContainable* handler,
        const OsTime& offset)
{
    Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
            "AppearanceGroupSet::addAppearanceByTimer "
            "this = %p, callidContact = '%s', handler = '%p', offset = '%d'",
                  this, callidContact.data(), handler, offset.cvtToMsecs());

    OsStatus ret = _appearanceTimers.scheduleOneshotAfter(
                         new AppearanceMsg(handler, callidContact),
                         offset);
    if (OS_SUCCESS != ret)
    {
        Os::Logger::instance().log(FAC_SAA, PRI_ERR,
                "AppearanceGroupSet::addAppearanceByTimer failed "
                "this = %p, callidContact = '%s', handler = '%p', offset = '%d'",
                      this, callidContact.data(), handler, offset.cvtToMsecs());
    }

    return (OS_SUCCESS == ret);
}

// Create and add an Appearance Group.
void AppearanceGroupSet::addAppearanceGroup(const char* user)
{
   Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                 "AppearanceGroupSet::addAppearanceGroup this = %p, user = '%s'",
                 this, user);

   // Serialize access to the AppearanceGroupSet.
   mutex_write_lock lock(_listMutex);

   // Check to see if there is already a group with this name.
   if (!findAppearanceGroup(user))
   {
      // Create the appearance group.
      AppearanceGroup* appearanceGroup = new AppearanceGroup(this, user/*, userCons*/);

      // Add the appearance group to the set.
      mAppearanceGroups.append(appearanceGroup);

      Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                    "AppearanceGroupSet::addAppearanceGroup added AppearanceGroup '%s', mVersion = %d",
                    user, mVersion);
   }
   else
   {
      Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                    "AppearanceGroupSet::addAppearanceGroup AppearanceGroup '%s' already exists",
                    user);
   }
}

// Remove an Appearance Group.
void AppearanceGroupSet::removeAppearanceGroup(const char* user)
{
   Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                 "AppearanceGroupSet::removeAppearanceGroup this = %p, user = '%s'",
                 this, user);

   // Serialize access to the AppearanceGroupSet.
   mutex_write_lock lock(_listMutex);

   // Check to see if there is a group with this name.
   AppearanceGroup* ag;
   if (!(ag = findAppearanceGroup(user)))
   {
      Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                    "AppearanceGroupSet::removeAppearanceGroup  AppearanceGroup '%s' does not exist",
                    user);
   }
   else
   {
      mAppearanceGroups.removeReference(ag);
      delete ag;
      Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                    "AppearanceGroupSet::removeAppearanceGroup removed AppearanceGroup '%s'",
                    user);
      // Now unpublish to terminate subscriptions to this no-longer-shared line.
      mAppearanceAgent->getEventPublisher().unpublish(
            user,
            DIALOG_SLA_EVENT_TYPE, //eventTypeKey
            DIALOG_EVENT_TYPE,     //eventType
            // Tell subscriber that SA events are no longer available for this user.
            SipSubscribeServer::terminationReasonNoresource
            );

   }
}

// Delete all Appearance Groups.
void AppearanceGroupSet::deleteAllAppearanceGroups()
{
   Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                 "AppearanceGroupSet::deleteAllAppearanceGroups this = %p",
                 this);

   // Gradually remove elements from the AppearanceGroups and delete them.
   AppearanceGroup* ag;
   int changeDelay = getAppearanceAgent()->getChangeDelay();
   do
   {
      // Serialize access to the AppearanceGroupSet.
       mutex_write_lock lock(_listMutex);

      // Get pointer to the first AppearanceGroup.
      ag = dynamic_cast <AppearanceGroup*> (mAppearanceGroups.first());

      if (ag)
      {
         mAppearanceGroups.removeReference(ag);
         delete ag;
      }

      // Delay to allow the consequent processing to catch up.
      OsTask::delay(changeDelay);
   } while (ag);
}

// Get a list of all Appearance Groups.
void AppearanceGroupSet::getAllAppearanceGroups(UtlSList& list)
{
   Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                 "AppearanceGroupSet::getAllAppearanceGroups this = %p",
                 this);

   // Serialize access to the AppearanceGroupSet.
   mutex_read_lock lock(_listMutex);

   // Iterate through the Appearance Groups.
   UtlSListIterator appearanceGroupItor(mAppearanceGroups);
   AppearanceGroup* appearanceGroup;
   while ((appearanceGroup = dynamic_cast <AppearanceGroup*> (appearanceGroupItor())))
   {
      list.append(new UtlString(appearanceGroup->getUser()));
   }
}

// Callback routine for subscription state events.
// Called as a callback routine.
void AppearanceGroupSet::subscriptionEventCallbackAsync(
   SipSubscribeClient::SubscriptionState newState,
   const char* earlyDialogHandle,
   const char* dialogHandle,
   void* applicationData,
   int responseCode,
   const char* responseText,
   long expiration,
   const SipMessage* subscribeResponse
   )
{
   // earlyDialogHandle may be NULL for some termination callbacks.
   if (!earlyDialogHandle)
   {
      earlyDialogHandle = "";
   }
   // dialogHandle may be NULL for some termination callbacks.
   if (!dialogHandle)
   {
      dialogHandle = "";
   }
   Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                 "AppearanceGroupSet::subscriptionEventCallbackAsync newState = %d, applicationData = %p, earlyDialogHandle = '%s', dialogHandle = '%s'",
                 newState, applicationData, earlyDialogHandle, dialogHandle);

   // The AppearanceGroupSet concerned is applicationData.
   AppearanceGroupSet* appearanceGroupSet = (AppearanceGroupSet*) applicationData;

   // Determine the subscription state.
   // Currently, this is only "active" or "terminated", which is not
   // very good.  But the real subscription state is carried in the
   // NOTIFY requests. :TODO: Handle subscription set correctly.
   const char* subscription_state;
   if (subscribeResponse)
   {
      int expires;
      subscribeResponse->getExpiresField(&expires);
      subscription_state = expires == 0 ? "terminated" : "active";
   }
   else
   {
      subscription_state = "active";
   }

   // Send a message to the AppearanceGroupTask.
   appearanceGroupSet->getAppearanceAgent()->getAppearanceAgentTask().
      postMessageP(
         new SubscriptionCallbackMsg(earlyDialogHandle, dialogHandle,
                                     newState, subscription_state));
}

// Callback routine for subscription state events.
// Called by AppearanceGroupTask.
void AppearanceGroupSet::subscriptionEventCallbackSync(
   const UtlString* earlyDialogHandle,
   const UtlString* dialogHandle,
   SipSubscribeClient::SubscriptionState newState,
   const UtlString* subscriptionState
   )
{
   Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                 "AppearanceGroupSet::subscriptionEventCallbackSync earlyDialogHandle = '%s', dialogHandle = '%s', newState = %d, subscriptionState = '%s'",
                 earlyDialogHandle->data(), dialogHandle->data(), newState,
                 subscriptionState->data());

   // Serialize access to the appearance group set.
   recursive_mutex_read_lock lock(_subscriptionMutex);

   // Look up the ResourceSubscriptionReceiver to notify based on the
   // earlyDialogHandle.
   /* To call the handler, we dynamic_cast the object to
    * (ResourceSubscriptionReceiver*).  Whether this is strictly
    * conformant C++ I'm not sure, since UtlContainable and
    * ResourceSubscriptionReceiver are not base/derived classes of
    * each other.  But it seems to work in GCC as long as the dynamic
    * type of the object is a subclass of both UtlContainable and
    * ResourceSubscriptionReceiver.
    */
   ResourceSubscriptionReceiver* receiver =
      dynamic_cast <ResourceSubscriptionReceiver*>
         (mSubscribeMap.findValue(earlyDialogHandle));

   if (receiver)
   {
      receiver->subscriptionEventCallback(earlyDialogHandle,
                                          dialogHandle,
                                          newState,
                                          subscriptionState);
   }
   else
   {
      Os::Logger::instance().log(FAC_SAA, PRI_ERR,
                    "AppearanceGroupSet::subscriptionEventCallbackSync this = %p, no ResourceSubscriptionReceiver found for earlyDialogHandle '%s'",
                    this, earlyDialogHandle->data());
   }
}

// Callback routine for NOTIFY events.
// Called as a callback routine.
bool AppearanceGroupSet::notifyEventCallbackAsync(const char* earlyDialogHandle,
                                               const char* dialogHandle,
                                               void* applicationData,
                                               const SipMessage* notifyRequest)
{
   Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                 "AppearanceGroupSet::notifyEventCallbackAsync "
                 "applicationData = %p, earlyDialogHandle = '%s', dialogHandle = '%s'",
                 applicationData, earlyDialogHandle, dialogHandle);

   // The AppearanceGroupSet concerned is applicationData.
   AppearanceGroupSet* appearanceGroupSet = (AppearanceGroupSet*) applicationData;

   // Send a message to the AppearanceGroupTask.  The handler owns the SipMessage pointer, unless queue insert fails.
   SipMessage* qnotifyRequest = new SipMessage(*notifyRequest);
   if (appearanceGroupSet->getAppearanceAgent()->getAppearanceAgentTask().
                                    postMessageP(new NotifyCallbackMsg(dialogHandle, qnotifyRequest))
       != OS_SUCCESS)
   {  
       // postMessageP deletes the new NotifyCallbackMsg for this failure path
       delete qnotifyRequest;
   }

   // Do NOT send an OK response; the callback handler is responsible for this.
   return false;
}

// Callback routine for NOTIFY events.
// Called by AppearanceGroupTask.
// This callback MUST send a response, as we told the SipSubscribeClient not to.
void AppearanceGroupSet::notifyEventCallbackSync(const UtlString* dialogHandle,
                                              const SipMessage* msg)
{
   Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                 "AppearanceGroupSet::notifyEventCallbackSync dialogHandle = '%s'",
                 dialogHandle->data());

   // Serialize access to the appearance group set.
   mutex_read_lock lock(_notifyMutex);

   // Look up the ResourceNotifyReceiver to notify based on the dialogHandle.
   /* To call the handler, we dynamic_cast the object to
    * (ResourceNotifyReceiver*).  Whether this is strictly
    * conformant C++ I'm not sure, since UtlContainanble and
    * ResourceNotifyReceiver are not base/derived classes of
    * each other.  But it seems to work in GCC as long as the dynamic
    * type of the object is a subclass of both UtlContainable and
    * ResourceNotifyReceiver.
    */
   ResourceNotifyReceiver* receiver =
      dynamic_cast <ResourceNotifyReceiver*>
         (mNotifyMap.findValue(dialogHandle));

   if (receiver)
   {
      // the callback MUST respond to the NOTIFY
      receiver->notifyEventCallback(dialogHandle, msg);
   }
   else
   {
      Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                    "AppearanceGroupSet::notifyEventCallbackSync this = %p, no ResourceNotifyReceiver found for dialogHandle '%s'",
                    this, dialogHandle->data());
      // Acknowledge the NOTIFY, even though we won't process it.
      SipMessage response;
      response.setOkResponseData(msg, NULL);
      getAppearanceAgent()->getServerUserAgent().send(response);
   }
   delete msg;
}

/** Add a mapping for an early dialog handle to its handler for
 *  subscription events.
 */
void AppearanceGroupSet::addSubscribeMapping(UtlString* earlyDialogHandle,
                                          UtlContainable* handler)
{
    recursive_mutex_write_lock lock(_subscriptionMutex);

   Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                 "AppearanceGroupSet::addSubscribeMapping this = %p, earlyDialogHandle = '%s', handler = %p",
                 this, earlyDialogHandle->data(), handler);

   mSubscribeMap.insertKeyAndValue(earlyDialogHandle, handler);
}

/** Delete a mapping for an early dialog handle.
 */
void AppearanceGroupSet::deleteSubscribeMapping(UtlString* earlyDialogHandle)
{
    recursive_mutex_write_lock lock(_subscriptionMutex);

   Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                 "AppearanceGroupSet::deleteSubscribeMapping this = %p, earlyDialogHandle = '%s'",
                 this, earlyDialogHandle->data());

   mSubscribeMap.remove(earlyDialogHandle);
}

/** Add a mapping for a dialog handle to its handler for
 *  NOTIFY events.
 */
void AppearanceGroupSet::addNotifyMapping(const UtlString* d,
                                       UtlContainable* handler)
{
    mutex_write_lock lock(_notifyMutex);

   /* The machinery surrounding dialog handles is broken in that it
    * does not keep straight which tag is local and which is remote,
    * and the dialogHandle for notifyEventCallback has the tags
    * reversed relative to the tags in the dialogHandle for
    * subscriptionEventCallback.  So we reverse the tags in dialogHandle
    * before inserting it into mNotifyMap, to match what
    * notifyEventCallback will receive.  Yuck.  Ideally, we would fix
    * the problem (XSL-146), but there are many places in the code
    * where it is sloppy about tracking whether a message is incoming
    * or outgoing when constructing a dialogHandle, and this is
    * circumvented by making the lookup of dialogs by dialogHandle
    * insensitive to reversing the tags.  (See SipDialog::isSameDialog.)
    */
   /* Correction:  Sometimes the NOTIFY tags are reversed, and
    * sometimes they aren't.  So we have to file both handles in
    * mNotifyMap.  Yuck.
    */
   if ( mNotifyMap.findValue(d) )
   {
      Os::Logger::instance().log(FAC_SAA, PRI_ERR,
                    "AppearanceGroupSet::addNotifyMapping already exists for this = %p, dialogHandle = '%s', handler = %p",
                    this, d->data(),
                    handler);
      return;
   }
   UtlString* dialogHandle = new UtlString(*d);
   mNotifyMap.insertKeyAndValue(dialogHandle, handler);

   UtlString* swappedDialogHandleP = new UtlString;
   swapTags(*dialogHandle, *swappedDialogHandleP);

   Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                 "AppearanceGroupSet::addNotifyMapping this = %p, dialogHandle = '%s', swappedDialogHandle = '%s', handler = %p",
                 this, dialogHandle->data(), swappedDialogHandleP->data(),
                 handler);

   // Note that we have allocated *swappedDialogHandleP.  Our caller
   // owns *dialogHandle and will deallocate it after calling
   // deleteNotifyMapping, but deleteNotify must remember to deallocate
   // *swappedDialogHandleP, the key object in mNotifyMap.  Yuck.
   mNotifyMap.insertKeyAndValue(swappedDialogHandleP, handler);
}

/** Delete a mapping for a dialog handle.
 */
void AppearanceGroupSet::deleteNotifyMapping(const UtlString* dialogHandle)
{
   // See comment in addNotifyMapping for why we do this.
   UtlString swappedDialogHandle;
   swapTags(*dialogHandle, swappedDialogHandle);

   Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                 "AppearanceGroupSet::deleteNotifyMapping this = %p, dialogHandle = '%s', swappedDialogHandle = '%s'",
                 this, dialogHandle->data(), swappedDialogHandle.data());

   // Delete the un-swapped mapping.
   UtlContainable* value;
   UtlContainable* keyString = mNotifyMap.removeKeyAndValue(dialogHandle, value);
   if (keyString)
   {
      // Delete the key object.
      delete keyString;
   }

   // Delete the swapped mapping.
   keyString = mNotifyMap.removeKeyAndValue(&swappedDialogHandle, value);
   if (keyString)
   {
      // Delete the key object.
      delete keyString;
   }
}


/* ============================ ACCESSORS ================================= */

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ INQUIRY =================================== */

/**
 * Get the ContainableType for a UtlContainable-derived class.
 */
UtlContainableType AppearanceGroupSet::getContainableType() const
{
   return AppearanceGroupSet::TYPE;
}

// Dump the object's internal state.
void AppearanceGroupSet::dumpState()
{
   // indented 2
   Os::Logger::instance().log(FAC_SAA, PRI_INFO,
                 "\t  AppearanceGroupSet %p", this);
   UtlSListIterator appearanceGroupItor(mAppearanceGroups);
   AppearanceGroup* appearanceGroup;
   while ((appearanceGroup = dynamic_cast <AppearanceGroup*> (appearanceGroupItor())))
   {
      appearanceGroup->dumpState();
   }
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Search for an Appearance Group with a given uri.
AppearanceGroup* AppearanceGroupSet::findAppearanceGroup(const char* user)
{
   AppearanceGroup* ret = 0;

   UtlSListIterator appearanceGroupItor(mAppearanceGroups);
   AppearanceGroup* appearanceGroup;
   while (!ret &&
          (appearanceGroup = dynamic_cast <AppearanceGroup*> (appearanceGroupItor())))
   {
      if (appearanceGroup->getUser().compareTo(user) == 0)
      {
         ret = appearanceGroup;
      }
   }

   return ret;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

// Swap the tags in a dialog handle.
void AppearanceGroupSet::swapTags(const UtlString& dialogHandle,
                               UtlString& swappedDialogHandle)
{
   // Find the commas in the dialogHandle.
   ssize_t index1 = dialogHandle.index(',');
   ssize_t index2 = dialogHandle.index(',', index1+1);

   // Copy the call-Id and the first comma.
   swappedDialogHandle.remove(0);
   swappedDialogHandle.append(dialogHandle,
                              index1+1);

   // Copy the second tag.
   swappedDialogHandle.append(dialogHandle,
                              index2+1,
                              dialogHandle.length() - (index2+1));

   // Copy the first comma and the first tag.
   swappedDialogHandle.append(dialogHandle,
                              index1,
                              index2-index1);
}

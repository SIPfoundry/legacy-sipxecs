//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include "ResourceListSet.h"
#include "ResourceList.h"
#include "SubscriptionSet.h"
#include "ResourceNotifyReceiver.h"
#include "ResourceSubscriptionReceiver.h"
#include "ResourceListMsg.h"
#include <os/OsSysLog.h>
#include <os/OsLock.h>
#include <os/OsEventMsg.h>
#include <utl/XmlContent.h>
#include <utl/UtlSListIterator.h>
#include <net/SipDialogEvent.h>
#include <net/NameValueTokenizer.h>
#include <net/NameValuePair.h>
#include <net/HttpMessage.h>
#include <net/SipMessage.h>
#include <xmlparser/tinyxml.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// URN for the xmlns attribute for Resource List Meta-Information XML.
#define RLMI_XMLNS "urn:ietf:params:xml:ns:rlmi"
// MIME information for RLMI XML.
#define RLMI_CONTENT_TYPE "application/rlmi+xml"

// Resubscription period.
#define RESUBSCRIBE_PERIOD 3600

// STATIC VARIABLE INITIALIZATIONS

const UtlContainableType ResourceListSet::TYPE = "ResourceListSet";
const int ResourceListSet::sSeqNoIncrement = 4;
const int ResourceListSet::sSeqNoMask = 0x3FFFFFFC;


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
ResourceListSet::ResourceListSet(ResourceListServer* resourceListServer) :
   mResourceListServer(resourceListServer),
   mSemaphore(OsBSem::Q_PRIORITY, OsBSem::FULL),
   mResourceCache(this),
   mNextSeqNo(0),
   mSuspendPublishingCount(0),
   mPublishingTimer(getResourceListServer()->getResourceListTask().
                    getMessageQueue(),
                    (void*)ResourceListSet::PUBLISH_TIMEOUT),
   mPublishOnTimeout(FALSE),
   mVersion(0)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListSet:: this = %p",
                 this);
}

// Destructor
ResourceListSet::~ResourceListSet()
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListSet::~ this = %p",
                 this);
}

// Flag to indicate publish on timeout
UtlBoolean ResourceListSet::publishOnTimeout()
{
   return mPublishOnTimeout;
}

// Set the gap timeout.
void ResourceListSet::setGapTimeout()
{
   // RFC 4235 specifies a maximum of one RLMI notification per second.
   // After publishing create a 1 second delay before publishing again.
   mPublishingTimer.oneshotAfter(OsTime(1,0));
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListSet::setGapTimeout mPublishingTimer.oneshotAfter(1 sec)");

   // Don't publish on the gap timeout.
   mPublishOnTimeout = FALSE;
}

// Delete all ResourceList's and stop the publishing timer.
void ResourceListSet::finalize()
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListSet::finalize this = %p",
                 this);

   // Make sure the ResourceList's are destroyed so that all
   // references to the ResourceCached's are removed, before
   // destroying the ResourceCache.
   deleteAllResourceLists();

   // Make sure the publishing timer is stopped before the ResourceListTask
   // is destroyed, because the timer posts messages to ResourceListTask.
   mPublishingTimer.stop();
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListSet::finalize mPublishingTimer.stop()");
}

/* ============================ MANIPULATORS ============================== */

// Create and add a resource list.
bool ResourceListSet::addResourceList(const char* user,
                                      const char* userCons,
                                      const char* nameXml)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListSet::addResourceList this = %p, user = '%s', userCons = '%s', nameXml = '%s'",
                 this, user, userCons, nameXml);

   // Serialize access to the ResourceListSet.
   OsLock lock(mSemaphore);

   // Check to see if there is already a list with this name.
   bool ret = !findResourceList(user);
   if (ret)
   {
      // Create the resource list.
      ResourceList* resourceList = new ResourceList(this, user, userCons);

      // Update the version number for consolidated events if it is too
      // small for an existing subscription to this URI.
      int v =
         getResourceListServer()->getSubscriptionMgr().
         getNextAllowedVersion(*resourceList->getResourceListUriCons());
      if (v > mVersion)
      {
         mVersion = v;
      }

      // Add the resource list to the set.
      mResourceLists.append(resourceList);

      OsSysLog::add(FAC_RLS, PRI_DEBUG,
                    "ResourceListSet::addResourceList added ResourceList, mVersion = %d",
                    mVersion);
   }
   else
   {
      OsSysLog::add(FAC_RLS, PRI_DEBUG,
                    "ResourceListSet::addResourceList ResourceList '%s' already exists",
                    user);
   }

   return ret;
}

// Delete all resource lists.
void ResourceListSet::deleteAllResourceLists()
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListSet::deleteAllResourceLists this = %p",
                 this);

   // Gradually remove elements from the ResourceLists and delete them.
   ResourceList* rl;
   int changeDelay = getResourceListServer()->getChangeDelay();
   do {
      {
         // Serialize access to the ResourceListSet.
         OsLock lock(mSemaphore);

         // Get pointer to the first ResourceList.
         rl = dynamic_cast <ResourceList*> (mResourceLists.first());

         // If one exists, shrink it.
         if (rl) {
            if (rl->shrink()) {
               // The ResourceList is empty, and so can be removed and deleted.
               mResourceLists.removeReference(rl);
               delete rl;
            }
         }
      }

      // Delay to allow the consequent processing to catch up.
      OsTask::delay(changeDelay);
   } while (rl);
}

// Delete all resources from a resource list.
void ResourceListSet::deleteAllResources(const char* user)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListSet::deleteAllResources this = %p, user = '%s'",
                 this, user);

   // Serialize access to the ResourceListSet.
   OsLock lock(mSemaphore);

   ResourceList* resourceList = findResourceList(user);
   if (resourceList)
   {
      resourceList->deleteAllResources();
      OsSysLog::add(FAC_RLS, PRI_DEBUG,
                    "ResourceListSet::deleteAllResources done");
   }
   else
   {
      OsSysLog::add(FAC_RLS, PRI_DEBUG,
                    "ResourceListSet::deleteAllResources ResourceList '%s' not found",
                    user);
   }
}

// Get a list of the user-parts of all resource lists.
void ResourceListSet::getAllResourceLists(UtlSList& list)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListSet::getAllResourceLists this = %p",
                 this);

   // Serialize access to the ResourceListSet.
   OsLock lock(mSemaphore);

   // Iterate through the resource lists.
   UtlSListIterator resourceListItor(mResourceLists);
   ResourceList* resourceList;
   while ((resourceList = dynamic_cast <ResourceList*> (resourceListItor())))
   {
      list.append(new UtlString(*resourceList->getUserPart()));
   }
}

//! Create and add a resource to the resource list.
//  Returns the generated Resource object.
bool ResourceListSet::addResource(const char* user,
                                  const char* uri,
                                  const char* nameXml,
                                  const char* display_name)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListSet::addResource this = %p, user = '%s', uri = '%s', nameXml = '%s', display_name = '%s'",
                 this, user, uri, nameXml, display_name);

   // Serialize access to the resource list.
   OsLock lock(mSemaphore);

   ResourceList* resourceList = findResourceList(user);
   bool ret;
   if (resourceList)
   {
      ret = resourceList->addResource(uri, nameXml, display_name);
      OsSysLog::add(FAC_RLS, PRI_DEBUG,
                    "ResourceListSet::addResource resource added");
   }
   else
   {
      ret = false;
      OsSysLog::add(FAC_RLS, PRI_DEBUG,
                    "ResourceListSet::addResource ResourceList '%s' not found",
                    user);
   }

   return ret;
}

// Callback routine for subscription state events.
// Called as a callback routine.
void ResourceListSet::subscriptionEventCallbackAsync(
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
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListSet::subscriptionEventCallbackAsync newState = %d, applicationData = %p, earlyDialogHandle = '%s', dialogHandle = '%s'",
                 newState, applicationData, earlyDialogHandle, dialogHandle);

   // The ResourceListSet concerned is applicationData.
   ResourceListSet* resourceListSet = (ResourceListSet*) applicationData;

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

   // Send a message to the ResourceListTask.
   resourceListSet->getResourceListServer()->getResourceListTask().
      postMessageP(
         new SubscriptionCallbackMsg(earlyDialogHandle, dialogHandle,
                                     newState, subscription_state));
}

// Callback routine for subscription state events.
// Called by ResourceListTask.
void ResourceListSet::subscriptionEventCallbackSync(
   const UtlString* earlyDialogHandle,
   const UtlString* dialogHandle,
   SipSubscribeClient::SubscriptionState newState,
   const UtlString* subscriptionState
   )
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListSet::subscriptionEventCallbackSync earlyDialogHandle = '%s', dialogHandle = '%s', newState = %d, subscriptionState = '%s'",
                 earlyDialogHandle->data(), dialogHandle->data(), newState,
                 subscriptionState->data());

   // Serialize access to the resource list set.
   OsLock lock(mSemaphore);

   // Look up the ResourceSubscriptionReceiver to notify based on the
   // earlyDialogHandle.
   /* To call the handler, we dynamic_cast the object to
    * (ResourceSubscriptionReceiver*).  Whether this is strictly
    * conformant C++ I'm not sure, since UtlContainanble and
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
      OsSysLog::add(FAC_RLS, PRI_WARNING,
                    "ResourceListSet::subscriptionEventCallbackSync this = %p, no ResourceSubscriptionReceiver found for earlyDialogHandle '%s'",
                    this, earlyDialogHandle->data());
   }
}

// Callback routine for NOTIFY events.
// Called as a callback routine.
bool ResourceListSet::notifyEventCallbackAsync(const char* earlyDialogHandle,
                                               const char* dialogHandle,
                                               void* applicationData,
                                               const SipMessage* notifyRequest)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListSet::notifyEventCallbackAsync applicationData = %p, earlyDialogHandle = '%s', dialogHandle = '%s'",
                 applicationData, earlyDialogHandle, dialogHandle);

   // The ResourceListSet concerned is applicationData.
   ResourceListSet* resourceListSet = (ResourceListSet*) applicationData;

   // Get the NOTIFY content.
   const char* b;
   ssize_t l;
   const HttpBody* body = notifyRequest->getBody();
   if (body)
   {
      body->getBytes(&b, &l);
   }
   else
   {
      b = NULL;
      l = 0;
   }

   // Send a message to the ResourceListTask.
   resourceListSet->getResourceListServer()->getResourceListTask().
      postMessageP(new NotifyCallbackMsg(dialogHandle, b, l));

   return true;
}

// Callback routine for NOTIFY events.
// Called by ResourceListTask.
void ResourceListSet::notifyEventCallbackSync(const UtlString* dialogHandle,
                                              const UtlString* content)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListSet::notifyEventCallbackSync dialogHandle = '%s'",
                 dialogHandle->data());

   // Serialize access to the resource list set.
   OsLock lock(mSemaphore);

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
      receiver->notifyEventCallback(dialogHandle, content);
   }
   else
   {
      OsSysLog::add(FAC_RLS, PRI_WARNING,
                    "ResourceListSet::notifyEventCallbackSync this = %p, no ResourceNotifyReceiver found for dialogHandle '%s'",
                    this, dialogHandle->data());
   }
}

/** Add a mapping for an early dialog handle to its handler for
 *  subscription events.
 */
void ResourceListSet::addSubscribeMapping(UtlString* earlyDialogHandle,
                                          UtlContainable* handler)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListSet::addSubscribeMapping this = %p, earlyDialogHandle = '%s', handler = %p",
                 this, earlyDialogHandle->data(), handler);

   mSubscribeMap.insertKeyAndValue(earlyDialogHandle, handler);
}

/** Delete a mapping for an early dialog handle.
 */
void ResourceListSet::deleteSubscribeMapping(UtlString* earlyDialogHandle)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListSet::deleteSubscribeMapping this = %p, earlyDialogHandle = '%s'",
                 this, earlyDialogHandle->data());

   mSubscribeMap.remove(earlyDialogHandle);
}

/** Add a mapping for a dialog handle to its handler for
 *  NOTIFY events.
 */
void ResourceListSet::addNotifyMapping(const UtlString& dialogHandle,
                                       UtlContainable* handler)
{
   /* The machinery surrounding dialog handles is broken in that it
    * does not keep straight which tag is local and which is remote,
    * and the order of the tags in dialogHandle is not consistent.
    * Ideally, we would fix the problem (XSL-146), but there are many
    * places in the code where it is sloppy about tracking whether a
    * message is incoming or outgoing when constructing a
    * dialogHandle.  We circumvent this by making the lookup of
    * dialogs by dialogHandle insensitive to reversing the tags.  (See
    * SipDialog::isSameDialog.)
    */

   // If we already have a different mapping, report an error, as this
   // addNotifyMapping() should be a duplicate of the mapping we
   // already have.
   UtlContainable* current_handler = mNotifyMap.find(&dialogHandle);
   if (current_handler)
   {
      if (current_handler != handler)
      {
      OsSysLog::add(FAC_RLS, PRI_ERR,
                    "ResourceListSet::addNotifyMapping Adding a different handler for an existing mapping: dialogHandle = '%s', current handler = %p, new handler = %p",
                    dialogHandle.data(), current_handler, handler);
      }
      // Remove the previous mapping in preparation for the new mapping.
      deleteNotifyMapping(&dialogHandle);
   }

   // Construct our copies of the dialog handle and the swapped dialog handle.
   UtlString* dialogHandleP = new UtlString(dialogHandle);
   UtlString* swappedDialogHandleP = new UtlString;
   swapTags(dialogHandle, *swappedDialogHandleP);

   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListSet::addNotifyMapping this = %p, dialogHandle = '%s', swappedDialogHandle = '%s', handler = %p",
                 this,
                 dialogHandleP->data(), swappedDialogHandleP->data(),
                 handler);

   // Make entries in mNotifyMap for both forms of the handle.
   mNotifyMap.insertKeyAndValue(dialogHandleP, handler);
   mNotifyMap.insertKeyAndValue(swappedDialogHandleP, handler);
}

/** Delete a mapping for a dialog handle.
 */
void ResourceListSet::deleteNotifyMapping(const UtlString* dialogHandle)
{
   // See comment in addNotifyMapping for why we have two entries, one for
   // the dialog handle and one for the swapped dialog handle.
   UtlString swappedDialogHandle;
   swapTags(*dialogHandle, swappedDialogHandle);

   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListSet::deleteNotifyMapping this = %p, dialogHandle = '%s', swappedDialogHandle = '%s'",
                 this, dialogHandle->data(), swappedDialogHandle.data());

   // We have to get a pointer to the key objects, as our caller won't
   // free them.  Otherwise, we could use UtlHashMap::remove().
   UtlContainable* value;
   UtlContainable* keyString = mNotifyMap.removeKeyAndValue(dialogHandle, value);
   if (keyString)
   {
      // Delete the key object.
      delete keyString;
   }
   keyString = mNotifyMap.removeKeyAndValue(&swappedDialogHandle, value);
   if (keyString)
   {
      // Delete the swapped key object.
      delete keyString;
   }
}

// Get the next sequence number for objects for the parent ResourceListServer.
int ResourceListSet::getNextSeqNo()
{
   // Update mNextSeqNo.
   mNextSeqNo = (mNextSeqNo + sSeqNoIncrement) & sSeqNoMask;

   // Return the new value.
   return mNextSeqNo;
}

// Returns TRUE if publish() should not have any effect.
UtlBoolean ResourceListSet::publishingSuspended()
{
   return mSuspendPublishingCount > 0;
}

// Suspend the effect of publish().
void ResourceListSet::suspendPublishing()
{
   // Serialize access to the resource list set.
   OsLock lock(mSemaphore);

   // Increment mSuspendPublishingCount.
   mSuspendPublishingCount++;

   // Stop the publishing timer, asynchronously.  This is to prevent
   // it from firing, so ResourceListTask doesn't bother trying to
   // publish the resource lists when it will have no effect, but also
   // so that when publishing is resumed, the publishing timer will be
   // started and eventually fire.
   mPublishingTimer.stop(FALSE);
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListSet::suspendPublishing mPublishingTimer.stop()");

   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListSet::suspendPublishing mSuspendPublishingCount now = %d",
                 mSuspendPublishingCount);
}

// Resume the effect of publish().
void ResourceListSet::resumePublishing()
{
   // Serialize access to the resource list set.
   OsLock lock(mSemaphore);

   // Decrement mSuspendPublishingCount if > 0.
   if (mSuspendPublishingCount > 0)
   {
      mSuspendPublishingCount--;

      OsSysLog::add(FAC_RLS, PRI_DEBUG,
                    "ResourceListSet::resumePublishing mSuspendPublishingCount now = %d",
                    mSuspendPublishingCount);

      // If mSuspendPublishingCount is now 0, publish all the lists.
      if (mSuspendPublishingCount == 0)
      {
         schedulePublishing();
      }
   }
   else
   {
      OsSysLog::add(FAC_RLS, PRI_ERR,
                    "ResourceListSet::resumePublishing called when mSuspendPublishingCount = 0");
   }
}

// Declare that some content has changed and needs to be published.
void ResourceListSet::schedulePublishing()
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListSet::schedulePublishing this = %p",
                 this);

   // If publishing has been suspended, do not start the timer --
   // it will be started when publishing is resumed.
   if (!publishingSuspended())
   {
      OsTime pubDelay = getResourceListServer()->getPublishingDelay();

      // Check if waiting for the gap timeout (rather than the publishing timeout)
      if (mPublishOnTimeout == FALSE)
      {
         OsTimer::OsTimerState tmrState;
         OsTimer::Time tmrExpiresAt;
         UtlBoolean tmrPeriodic;
         OsTimer::Interval tmrPeriod;
         mPublishingTimer.getFullState(tmrState, tmrExpiresAt, tmrPeriodic, tmrPeriod);

         // Check if the timer is currently running.
         if (tmrState == OsTimer::STARTED)
         {
            // Calculate the amount of time before the gap timer expires (in seconds and microseconds).
            OsTimer::Time timeDelta = tmrExpiresAt - OsTimer::now();
            OsTime pubGap(timeDelta/1000000, timeDelta%1000000);

            // If the remaining gap timeout is less than the pubDelay
            // then we need to wait for pubDelay before publishing.
            if (pubGap < pubDelay)
            {
               // Cancel the current gap timeout so that oneshotAfter can restart the timer.
               mPublishingTimer.stop();
               OsSysLog::add(FAC_RLS, PRI_DEBUG,
                             "ResourceListSet::schedulePublishing mPublishingTimer.stop()");
            }
         }
      }

      // Start the timer with the publishing timeout if the timer is not already started.
      // If it is already started, OsTimer::oneshotAfter() does nothing.
      mPublishingTimer.oneshotAfter(pubDelay);
      OsSysLog::add(FAC_RLS, PRI_DEBUG,
                    "ResourceListSet::schedulePublishing mPublishingTimer.oneshotAfter(%d.%06d)",
                    pubDelay.seconds(), pubDelay.usecs());

      // Publish once the publishing timer expires.
      mPublishOnTimeout = TRUE;
   }
}

// Publish all ResourceList's that have changes.
void ResourceListSet::publish()
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListSet::publish this = %p",
                 this);

   // Serialize access to the resource list set.
   OsLock lock(mSemaphore);

   // If publishing has been suspended, do nothing --
   // publish() will be called again after publishing is resumed.
   if (!publishingSuspended())
   {
      // Iterate through the resource lists.
      UtlSListIterator resourceListItor(mResourceLists);
      ResourceList* resourceList;
      while ((resourceList =
              dynamic_cast <ResourceList*> (resourceListItor())))
      {
         resourceList->publishIfNecessary();
      }

      // Purge dialogs with terminated state and terminated resource
      // instances, now that we have published the fact that they've
      // terminated (and their termination reasons).
      getResourceCache().purgeTerminated();
   }
}

/* ============================ ACCESSORS ================================= */

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ INQUIRY =================================== */

/**
 * Get the ContainableType for a UtlContainable-derived class.
 */
UtlContainableType ResourceListSet::getContainableType() const
{
   return ResourceListSet::TYPE;
}

// Split a userData value into the seqNo and "enum notifyCodes".
void ResourceListSet::splitUserData(int userData,
                                    int& seqNo,
                                    enum notifyCodes& type)
{
   seqNo = userData & sSeqNoMask;
   type = (enum notifyCodes) (userData & ~sSeqNoMask);
}

// Retrieve an entry from mEventMap and delete it.
UtlContainable* ResourceListSet::retrieveObjectBySeqNoAndDeleteMapping(int seqNo)
{
   // Serialize access to the resource list set.
   OsLock lock(mSemaphore);

   // Search for and possibly delete seqNo.
   UtlInt search_key(seqNo);
   UtlContainable* value;
   UtlContainable* key = mEventMap.removeKeyAndValue(&search_key, value);

   if (key)
   {
      OsSysLog::add(FAC_RLS, PRI_DEBUG,
                    "ResourceListSet::retrieveObjectBySeqNoAndDeleteMapping seqNo = %d, value = %p",
                    seqNo, value);
      delete key;
   }
   else
   {
      OsSysLog::add(FAC_RLS, PRI_WARNING,
                    "ResourceListSet::retrieveObjectBySeqNoAndDeleteMapping seqNo = %d not found",
                    seqNo);
   }

   return value;
}

// Add a mapping for a ResourceCached's sequence number.
void ResourceListSet::addResourceSeqNoMapping(int seqNo,
                                              ResourceCached* resource)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListSet::addResourceSeqNoMappping seqNo = %d, instanceName = '%s'",
                 seqNo, resource->getUri()->data());

   // Allocate a UtlInt to hold the sequence number.
   UtlInt* i = new UtlInt(seqNo);

   // Put the pair in mEventMap.
   mEventMap.insertKeyAndValue(i, resource);
}

//! Delete a mapping for a Resource's sequence number.
void ResourceListSet::deleteResourceSeqNoMapping(int seqNo)
{
   // Search for and possibly delete seqNo.
   UtlInt search_key(seqNo);
   UtlContainable* value;
   UtlContainable* key = mEventMap.removeKeyAndValue(&search_key, value);

   if (key)
   {
      OsSysLog::add(FAC_RLS, PRI_DEBUG,
                    "ResourceListSet::deleteResourceSeqNoMapping seqNo = %d, instanceName = '%s'",
                    seqNo,
                    (dynamic_cast <ResourceCached*> (value))->getUri()->data());
      delete key;
   }
   else
   {
      OsSysLog::add(FAC_RLS, PRI_WARNING,
                    "ResourceListSet::deleteResourceSeqNoMapping seqNo = %d not found",
                    seqNo);
   }
}

// Dump the object's internal state.
void ResourceListSet::dumpState()
{
   // Serialize access to the resource list set.
   OsLock lock(mSemaphore);

   // indented 2

   OsSysLog::add(FAC_RLS, PRI_INFO,
                 "\t  ResourceListSet %p mSuspendPublishingCount = %d",
                 this, mSuspendPublishingCount);

   UtlSListIterator i(mResourceLists);
   ResourceList* rl;
   while ((rl = dynamic_cast <ResourceList*> (i())))
   {
      rl->dumpState();
   }

   mResourceCache.dumpState();
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Search for a resource list with a given name (user-part).
ResourceList* ResourceListSet::findResourceList(const char* user)
{
   ResourceList* ret = 0;

   // Iterate through the resource lists.
   UtlSListIterator resourceListItor(mResourceLists);
   ResourceList* resourceList;
   while (!ret &&
          (resourceList = dynamic_cast <ResourceList*> (resourceListItor())))
   {
      if (resourceList->getUserPart()->compareTo(user) == 0)
      {
         ret = resourceList;
      }
   }

   return ret;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

// Swap the tags in a dialog handle.
void ResourceListSet::swapTags(const UtlString& dialogHandle,
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

//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include "SubscriptionSet.h"
#include "ResourceInstance.h"
#include <os/OsSysLog.h>
#include <utl/UtlSListIterator.h>
#include <net/SipMessage.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

const UtlContainableType SubscriptionSet::TYPE = "SubscriptionSet";


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SubscriptionSet::SubscriptionSet(ResourceCached* resource,
                                 UtlString& uri) :
   ResourceSubscriptionReceiver(),
   mResource(resource),
   mUri(uri)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "SubscriptionSet:: this = %p, resource = %p, mUri = '%s'",
                 this, mResource, mUri.data());

   // Start the subscription for dialog events.
   UtlBoolean ret;
   UtlString mUriNameAddr = "<" + mUri + ">";
   ret = getResourceListServer()->getSubscribeClient().
      addSubscription(mUri.data(),
                      getResourceListServer()->getEventType(),
                      getResourceListServer()->getContentType(),
                      getResourceListServer()->getClientFromURI(),
                      mUriNameAddr.data(),
                      getResourceListServer()->getClientContactURI(),
                      getResourceListServer()->getResubscribeInterval(),
                      getResourceListSet(),
                      ResourceListSet::subscriptionEventCallbackAsync,
                      ResourceListSet::notifyEventCallbackAsync,
                      mSubscriptionEarlyDialogHandle);
   if (ret)
   {
      OsSysLog::add(FAC_RLS, PRI_DEBUG,
                    "SubscriptionSet:: addSubscription for '%s' succeeded",
                    mUri.data());
      // Add this SubscriptionSet to mSubscribeMap.
      getResourceListSet()->addSubscribeMapping(&mSubscriptionEarlyDialogHandle,
                                                this);
   }
   else
   {
      OsSysLog::add(FAC_RLS, PRI_WARNING,
                    "SubscriptionSet:: addSubscription for '%s' failed",
                    mUri.data());
   }
}

// Destructor
SubscriptionSet::~SubscriptionSet()
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "SubscriptionSet::~ mUri = '%s'",
                 mUri.data());

   // Delete this SubscriptionSet from mSubscribeMap.
   getResourceListSet()->deleteSubscribeMapping(&mSubscriptionEarlyDialogHandle);

   // Terminate the master subscription.
   UtlBoolean ret;
   ret = getResourceListServer()->getSubscribeClient().
      endSubscriptionGroup(mSubscriptionEarlyDialogHandle.data());
   OsSysLog::add(FAC_RLS,
                 ret ? PRI_DEBUG : PRI_WARNING,
                 "SubscriptionSet::~ endSubscriptionGroup %s mUri = '%s', mSubscriptionEarlyDialogHandle = '%s'",
                 ret ? "succeeded" : "failed",
                 mUri.data(),
                 mSubscriptionEarlyDialogHandle.data());

   // Delete all the child subscriptions.

   // First, send their termination content.
   UtlSListIterator itor(mSubscriptions);
   ResourceInstance* inst;
   while ((inst = dynamic_cast <ResourceInstance*> (itor())))
   {
      // Set the state of the resource instance to terminated.
      // We do not know what their termination reason is.
      // (:TODO: But it might be possible to determine that.)
      inst->terminateContent("terminated");
   }

   // Since we are about to delete the ResourceInstances, we have to
   // publish their terminated state now or forgo doing so.
   getResourceCached()->setToBePublished(TRUE);

   // Delete the ResourceInstance objects.
   mSubscriptions.destroyAll();

   // Record that the content has been changed again and needs to be
   // published.
   getResourceCached()->setToBePublished();
}

/* ============================ MANIPULATORS ============================== */

void SubscriptionSet::subscriptionEventCallback(
   const UtlString* earlyDialogHandle,
   const UtlString* dialogHandle,
   SipSubscribeClient::SubscriptionState newState,
   const UtlString* subscriptionState)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "SubscriptionSet::subscriptionEventCallback newState = %d, earlyDialogHandle = '%s', dialogHandle = '%s', subscriptionState = '%s'",
                 newState, mSubscriptionEarlyDialogHandle.data(),
                 dialogHandle->data(), subscriptionState->data());

   switch (newState)
   {
   case SipSubscribeClient::SUBSCRIPTION_INITIATED:
      break;
   case SipSubscribeClient::SUBSCRIPTION_SETUP:
   {
      // There may be duplicate 'active' subscription callbacks, so first
      // check whether we already know of the subscription.
      if (!getInstance(dialogHandle->data()))
      {
         // Put the subscription into pending state, as we have no
         // content for it yet.
         addInstance(dialogHandle->data(), "pending");
      }
   }
   break;
   case SipSubscribeClient::SUBSCRIPTION_TERMINATED:
   {
      deleteInstance(dialogHandle->data(), "terminated",
                     subscriptionState->data());
   }
   break;
   }
}

// Insert a subscription into the set.
void SubscriptionSet::addInstance(const char* instanceName,
                                  const char* subscriptionState)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "SubscriptionSet::addInstance instanceName = '%s', subscriptionState = '%s'",
                 instanceName, subscriptionState);

   // Check that we don't have too many instances.
   if (mSubscriptions.entries() <
       getResourceListServer()->getMaxResInstInCont())
   {
      // Add the instance to the set.
      ResourceInstance* inst =
         new ResourceInstance(this, instanceName, subscriptionState);
      mSubscriptions.append(inst);
   }
   else
   {
      OsSysLog::add(FAC_RLS, PRI_ERR,
                    "SubscriptionSet::addInstance cannot add ResourceInstance with name '%s', already %zu in SubscriptionSet '%s'",
                    instanceName, mSubscriptions.entries(),
                    mUri.data());
   }
}

// Find a subscription in the set.
ResourceInstance* SubscriptionSet::getInstance(const char* instanceName)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "SubscriptionSet::getInstance instanceName = '%s'",
                 instanceName);

   // Search for the resource instance in question.
   UtlSListIterator itor(mSubscriptions);
   ResourceInstance* inst;
   UtlBoolean found = FALSE;
   while (!found && (inst = dynamic_cast <ResourceInstance*> (itor())))
   {
      if (inst->getInstanceName()->compareTo(instanceName) == 0)
      {
         found = TRUE;
      }
   }

   return inst;
}

// Delete a subscription from the set.
void SubscriptionSet::deleteInstance(const char* instanceName,
                                     const char* subscriptionState,
                                     const char* resourceSubscriptionState)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "SubscriptionSet::deleteInstance instanceName = '%s', subscriptionState = '%s'",
                 instanceName, subscriptionState);

   // Search for the resource instance in question.
   UtlSListIterator itor(mSubscriptions);
   ResourceInstance* inst;
   UtlBoolean found = FALSE;
   while (!found && (inst = dynamic_cast <ResourceInstance*> (itor())))
   {
      if (inst->getInstanceName()->compareTo(instanceName) == 0)
      {
         found = TRUE;
         // Set the state of the resource instance to terminated.
         // This call sets the containing ResourceList's to publish,
         // eventually.
         inst->terminateContent(subscriptionState);
         // We do not remove the element from mSubscriptions, but rather
         // let it be removed by ResourceCache::purgeTerminated().
      }
   }
   if (!found)
   {
      OsSysLog::add(FAC_RLS, PRI_WARNING,
                    "SubscriptionSet::deleteInstance instanceName = '%s' not found",
                    instanceName);
   }
}

// Remove dialogs in terminated state and terminated resource instances.
void SubscriptionSet::purgeTerminated()
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "SubscriptionSet::purgeTerminated mUri = '%s'",
                 mUri.data());

   // Iterate through the instances of the resource.
   UtlSListIterator instancesItor(mSubscriptions);
   ResourceInstance* instance;
   while ((instance =
           dynamic_cast <ResourceInstance*> (instancesItor())))
   {
      if (instance->isSubscriptionStateTerminated())
      {
         // If the instance itself is terminated, destroy it.
         mSubscriptions.destroy(instance);
      }
      else
      {
         // Purge any terminated dialogs in the instance.
         instance->purgeTerminatedDialogs();
      }
   }
}

// Add to the HttpBody the current state of the resource instances.
void SubscriptionSet::generateBody(UtlString& rlmi,
                                   HttpBody& body,
                                   UtlBoolean consolidated,
                                   const UtlString& displayName) const
{
   // Iterate through the instances of the resource.
   UtlSListIterator instancesItor(mSubscriptions);
   ResourceInstance* instance;
   while ((instance =
           dynamic_cast <ResourceInstance*> (instancesItor())))
   {
      instance->generateBody(rlmi, body, consolidated, displayName);
   }
}

/* ============================ ACCESSORS ================================= */

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ INQUIRY =================================== */

// Dump the object's internal state.
void SubscriptionSet::dumpState() const
{
   // indented 10

   OsSysLog::add(FAC_RLS, PRI_INFO,
                 "\t          SubscriptionSet %p mUri = '%s', mSubscriptionEarlyDialogHandle = '%s'",
                 this, mUri.data(), mSubscriptionEarlyDialogHandle.data());

   UtlSListIterator itor(mSubscriptions);
   ResourceInstance* ri;
   while ((ri = dynamic_cast <ResourceInstance*> (itor())))
   {
      ri->dumpState();
   }
}

/**
 * Get the ContainableType for a UtlContainable-derived class.
 */
UtlContainableType SubscriptionSet::getContainableType() const
{
   return SubscriptionSet::TYPE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

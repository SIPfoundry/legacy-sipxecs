// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

#include <stdlib.h>

// APPLICATION INCLUDES

#include "ResourceCached.h"
#include "ContactSet.h"
#include <os/OsSysLog.h>
#include <os/OsTimer.h>
#include <os/OsLock.h>
#include <utl/XmlContent.h>
#include <utl/UtlHashBagIterator.h>
#include <net/SipDialogEvent.h>
#include <net/NameValueTokenizer.h>
#include <net/NameValuePair.h>
#include <net/HttpMessage.h>
#include <net/SipMessage.h>
#include <net/SipDialogEvent.h>
#include <xmlparser/tinyxml.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

const UtlContainableType ResourceCached::TYPE = "ResourceCached";

// Random number generator.
UtlRandom ResourceCached::sRandom;


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
ResourceCached::ResourceCached(ResourceCache* resourceCache,
                               const char* uri) :
   UtlString(uri),
   mResourceCache(resourceCache),
   mContactSetP(0),
   mSeqNo(getResourceListSet()->getNextSeqNo()),
   mRefreshTimer(getResourceListServer()->getResourceListTask().
                    getMessageQueue(),
                 (void*)(mSeqNo + ResourceListSet::REFRESH_TIMEOUT))
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceCached:: this = %p, URI = '%s'",
                 this, data());

   // Add the sequence number mapping.
   getResourceListSet()->addResourceSeqNoMapping(mSeqNo, this);

   // Start subscriptions.
   startSubscriptions();
}

// Destructor
ResourceCached::~ResourceCached()
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceCached::~ this = %p, URI = '%s'",
                 this, data());

   // Terminate any existing subscriptions.
   terminateSubscriptions();

   // Delete the sequence number mapping.
   getResourceListSet()->deleteResourceSeqNoMapping(mSeqNo);
}

/* ============================ MANIPULATORS ============================== */

// Add a ResourceReference to the set of references.
void ResourceCached::addReference(ResourceReference* resourceReference)
{
   mReferences.insert(resourceReference);
}

// Remove a ResourceReference from the set of references.
void ResourceCached::deleteReference(ResourceReference* resourceReference)
{
   mReferences.removeReference(resourceReference);
}

// Determine of there are any references in the set of references.
UtlBoolean ResourceCached::hasReferences()
{
   return !mReferences.isEmpty();
}

// Refresh the resource's subscriptions.
void ResourceCached::refresh()
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceCached::refresh URI = '%s'",
                 data());

   // Reinitialize subscription machinery.
   terminateSubscriptions();

   // Start subscriptions.
   startSubscriptions();
}

//! Start subscriptions for this resource.
void ResourceCached::startSubscriptions()
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceCached::startSubscriptions URI = '%s'",
                 data());

   // Create the ContactSet.
   mContactSetP = new ContactSet(this,
                                 // The URI, which is the UtlString-nature
                                 // of a RecourceCached.
                                 *(static_cast <UtlString*> (this))
      );
   // Start the refresh timer.
   // Choose a random time between 1/2 and 1 times
   // ResourceListServer::getRefreshInterval().
   int refresh_time =
      (int) ((1.0 + ((float) sRandom.rand()) / RAND_MAX) / 2.0 *
             getResourceListServer()->getRefreshInterval());
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceCached::startSubscriptions refresh_time = %d",
                 refresh_time);
   OsTime rt(refresh_time, 0);
   mRefreshTimer.oneshotAfter(rt);
}

// Terminate any existing subscriptions for this resource.
void ResourceCached::terminateSubscriptions()
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceCached::terminateSubscriptions URI = '%s'",
                 data());

   // Stop the refresh timer, asynchronously so it cannot block.
   mRefreshTimer.stop(FALSE);

   // Terminate any existing subscriptions.
   if (mContactSetP)
   {
      delete mContactSetP;
      mContactSetP = 0;
   }
}

// Declare that the contents have changed and need to be published.
void ResourceCached::setToBePublished(UtlBoolean publishNow)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceCached::setToBePublished URI = '%s'",
                 data());

   // Iterate through all our ResourceReference's and mark their
   // containing ResourceList's as needing to be published.
   UtlHashBagIterator itor(mReferences);
   ResourceReference* reference;
   while ((reference = dynamic_cast <ResourceReference*> (itor())))
   {
      reference->getResourceList()->setToBePublished();
   }

   // If publishing is suppressed, do nothing further.
   if (!getResourceListSet()->publishingSuspended())
   {
      if (publishNow)
      {
         // If publishNow is specified, loop through the
         // ResourceList's again and publish them.  (Doing this in a
         // second loop avoids publishing a list twice if it contains
         // this resource twice.)
         UtlHashBagIterator itor(mReferences);
         ResourceReference* reference;
         while ((reference = dynamic_cast <ResourceReference*> (itor())))
         {
            reference->getResourceList()->publishIfNecessary();
         }
      }
      else
      {
         // In the more usual case, we just set the publishing timer.
         getResourceListSet()->schedulePublishing();
      }
   }
}

// Add to the HttpBody the current state of the resource.
void ResourceCached::generateBody(UtlString& rlmi,
                                  HttpBody& body,
                                  UtlBoolean consolidated,
                                  const UtlString& nameXml,
                                  const UtlString& displayName) const
{
   // Generate the preamble for the resource.
   rlmi += "  <resource uri=\"";
   XmlEscape(rlmi, *(static_cast <const UtlString*> (this)));
   rlmi += "\">\r\n";
   if (!nameXml.isNull())
   {
      rlmi += "    ";
      rlmi += nameXml;
   }

   if (consolidated)
   {
      // If consolidating resource instances, generate the XML for the
      // unified resource instance.
      rlmi += "    <instance id=\"consolidated\" state=\"active\"";

      UtlString contentBodyPartCid;
      // Use the count of parts in 'body' to generate a unique identifier for
      // each part.
      contentBodyPartCid.appendNumber(body.getMultipartCount());
      contentBodyPartCid += "@";
      contentBodyPartCid += getResourceListServer()->getDomainName();

      rlmi += " cid=\"";
      rlmi += contentBodyPartCid;
      rlmi += "\"";

      // Now add the <...> and use it in the header.
      contentBodyPartCid.prepend("<");
      contentBodyPartCid.append(">");

      // Create a single HttpBody to contain the unified dialog event.
      UtlString dialog_event;

      // XML declaration is optional, but Broadworks uses it.
      dialog_event += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      dialog_event += BEGIN_DIALOG_INFO;
      dialog_event += VERSION_EQUAL;
      dialog_event += "\"";
      // The consolidated dialog events need to have persistent
      // version numbers, as they all have the same instance id.
      // So we use a global version number in the ResourceListSet.
      dialog_event.appendNumber(getResourceListSet()->getVersion());
      dialog_event += "\"";
      dialog_event += STATE_EQUAL;
      dialog_event += "\"full\"";
      dialog_event += ENTITY_EQUAL;
      dialog_event += "\"";
      dialog_event += *(static_cast <const UtlString*> (this));
      dialog_event += "\">\r\n";
      // Save the length of dialog_event, so we can tell later if
      // any <dialog>s have been added to it.
      unsigned int preamble_length = dialog_event.length();

      // Call the ContactSet to generate the consolidated dialog event body.
      if (mContactSetP)
      {
         mContactSetP->generateBody(dialog_event, body, consolidated,
                                    displayName);
      }

      // If no <dialog>s have been added, we have to add a dummy
      // <dialog> to carry the display name.
      if (dialog_event.length() == preamble_length)
      {
         dialog_event +=
            "<dialog id=\";\"><state>terminated</state><local><identity display=\"";
         XmlEscape(dialog_event, displayName);
         dialog_event += "\">";
         XmlEscape(dialog_event, *(static_cast <const UtlString*> (this)));
         dialog_event += "</identity></local></dialog>\r\n";
      }

      dialog_event += END_DIALOG_INFO;

      // Insert the consolidated dialog event body into the multiplart body.
      HttpBody content_body(dialog_event.data(),
                            dialog_event.length(),
                            DIALOG_EVENT_CONTENT_TYPE);
      UtlDList content_body_parameters;
      content_body_parameters.append(
         new NameValuePair(HTTP_CONTENT_ID_FIELD,
                           contentBodyPartCid));
      body.appendBodyPart(content_body, content_body_parameters);
      content_body_parameters.destroyAll();

      // Finish the <instance> element.
      rlmi += "/>\r\n";
   }
   else
   {
      // Call the ContactSet to do the work.
      if (mContactSetP)
      {
         mContactSetP->generateBody(rlmi, body, consolidated, displayName);
      }
   }   
   
// Generate the postamble for the resource.
   rlmi += "  </resource>\r\n";
}

// Remove dialogs in terminated state and terminated resource instances.
void ResourceCached::purgeTerminated()
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceCached:: this = %p, URI = '%s'",
                 this, data());

   // Call the ContactSet to do the work.
   if (mContactSetP)
   {
      mContactSetP->purgeTerminated();
   }
}

/* ============================ ACCESSORS ================================= */

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ INQUIRY =================================== */

// Dump the object's internal state.
void ResourceCached::dumpState()
{
   // indented 6

   OsTimer::OsTimerState state;
   OsTimer::Time expiresAt;
   UtlBoolean periodic;
   OsTimer::Interval period;
   mRefreshTimer.getFullState(state, expiresAt, periodic, period);
   OsSysLog::add(FAC_RLS, PRI_INFO,
                 "\t      ResourceCached %p UtlString = '%s', mSeqNo = %d, mRefreshTimer = %s/%+d/%s/%d",
                 this, data(), mSeqNo,
                 state == OsTimer::STARTED ? "STARTED" : "STOPPED",
                 (int) ((expiresAt - OsTimer::now()) / 1000000),
                 periodic ? "periodic" : "one-shot",
                 (int) period);
   mContactSetP->dumpState();
}

/**
 * Get the ContainableType for a UtlContainable-derived class.
 */
UtlContainableType ResourceCached::getContainableType() const
{
   return ResourceCached::TYPE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

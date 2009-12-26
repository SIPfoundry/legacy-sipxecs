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


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
ResourceCached::ResourceCached(ResourceCache* resourceCache,
                               const char* uri) :
   UtlString(uri),
   mResourceCache(resourceCache),
   mContactSetP(0),
   mSeqNo(getResourceListSet()->getNextSeqNo())
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
}

// Terminate any existing subscriptions for this resource.
void ResourceCached::terminateSubscriptions()
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceCached::terminateSubscriptions URI = '%s'",
                 data());

   // Terminate any existing subscriptions.
   if (mContactSetP)
   {
      delete mContactSetP;
      mContactSetP = 0;
   }
}

// Declare that the contents have changed and need to be published.
void ResourceCached::setToBePublished(UtlBoolean publishNow,
                                      const UtlString* chgUri)
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
      reference->getResourceList()->setToBePublished(chgUri);
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

   // Remove any suffix from the resource's URI (which is the
   // UtlString-nature of *this).  For example it removes:
   // ";sipx-noroute=VoiceMail;sipx-userforward=false"
   Url temp_url(*(static_cast <const UtlString*> (this)),
                TRUE);    // addr-spec format, i.e., URI
   temp_url.removeUrlParameters();
   UtlString rlmi_uri;
   temp_url.getUri(rlmi_uri);       // addr-spec format, i.e., URI

   // Generate the preamble for the resource.
   rlmi += "  <resource uri=\"";
   XmlEscape(rlmi, rlmi_uri);
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
      dialog_event += rlmi_uri;
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
         XmlEscape(dialog_event, rlmi_uri);
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
                 "ResourceCached::purgeTerminated this = %p, URI = '%s'",
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

   OsSysLog::add(FAC_RLS, PRI_INFO,
                 "\t      ResourceCached %p UtlString = '%s', mSeqNo = %d",
                 this, data(), mSeqNo);
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

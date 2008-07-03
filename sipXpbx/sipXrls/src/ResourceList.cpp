// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include "ResourceList.h"
#include "ResourceReference.h"
#include <os/OsSysLog.h>
#include <os/OsLock.h>
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

const UtlContainableType ResourceList::TYPE = "ResourceList";


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
ResourceList::ResourceList(ResourceListSet* resourceListSet,
                           const char* userPart,
                           const char* userPartCons) :
   mUserPart(userPart),
   mUserPartCons(userPartCons),
   mVersion(0),
   mResourceListSet(resourceListSet)
{
   // Compose the resource list names.
   mResourceListName = "sip:";
   mResourceListName.append(mUserPart);
   mResourceListName.append("@");
   mResourceListName.append(getResourceListServer()->getDomainName());

   mResourceListNameCons = "sip:";
   mResourceListNameCons.append(mUserPartCons);
   mResourceListNameCons.append("@");
   mResourceListNameCons.append(getResourceListServer()->getDomainName());

   // Compose the resource list URIs as they will appear in the SUBSCRIBE.
   // Because we now use a Route header to send SUBSCRIBEs to the RLS,
   // and leave the request-URI unchanged, the resource list URIs are
   // the same as the resource list names.
   mResourceListUri = mResourceListName;
   mResourceListUriCons = mResourceListNameCons;

   // Initialize mVersion by looking up the next allowed version
   // that is recorded in any subscription for mResourceListUri.
   mVersion =
      getResourceListServer()->getSubscriptionMgr().
      getNextAllowedVersion(mResourceListUri);

   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceList::_ this = %p, mUserPart = '%s', mResourceListName = '%s', mResourceListUri = '%s', mUserPartCons = '%s', mResourceListNameCons = '%s', mResourceListUriCons = '%s', mVersion = %d",
                 this,
                 mUserPart.data(),
                 mResourceListName.data(), mResourceListUri.data(),
                 mUserPartCons.data(),
                 mResourceListNameCons.data(), mResourceListUriCons.data(),
                 mVersion);

   // Publish the new list.
   setToBePublished();
}

// Destructor
ResourceList::~ResourceList()
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceList::~ mUserPart = '%s'",
                 mUserPart.data());

   mResources.destroyAll();
   // Do not need to publish, as deletion of the resource instances
   // has triggered publishing.
}

/* ============================ MANIPULATORS ============================== */

// Refresh the subscriptions of all resources.
void ResourceList::refreshAllResources()
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceList::refreshAllResources mUserPart = '%s'",
                 mUserPart.data());

   // Iterate through the resources.
   UtlSListIterator resourcesItor(mResources);
   ResourceReference* resource;
   while ((resource = dynamic_cast <ResourceReference*> (resourcesItor())))
   {
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceList::refreshAllResources resource = '%s'",
                 resource->getUri()->data());
      resource->refresh();
   }
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceList::refreshAllResources done");
}

// Delete all ResourceReference's in the resource list.
void ResourceList::deleteAllResources()
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceList::deleteAllResources mUserPart = '%s'",
                 mUserPart.data());

   mResources.destroyAll();
}

// Create and add a resource to the resource list.
void ResourceList::addResource(const char* uri,
                               const char* nameXml,
                               const char* display_name)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceList::addResource mUserPart = '%s', uri = '%s', nameXml = '%s', display_name = '%s'",
                 mUserPart.data(), uri, nameXml, display_name);

   mResources.append(new ResourceReference(this, uri, nameXml, display_name));
}

// Declare that the contents have changed and need to be published.
void ResourceList::setToBePublished()
{
    OsSysLog::add(FAC_RLS, PRI_DEBUG,
                  "ResourceList::setToBePublished mUserPart = '%s'",
                  mUserPart.data());

   mChangesToPublish = TRUE;
}

// Publish the contents if necessary and clear the publishing indicator.
void ResourceList::publishIfNecessary()
{
    OsSysLog::add(FAC_RLS, PRI_DEBUG,
                  "ResourceList::publishIfNecessary mUserPart = '%s'",
                  mUserPart.data());

   // Do nothing if publishing is suspended.
   if (!getResourceListSet()->publishingSuspended())
   {
      if (mChangesToPublish)
      {
         mChangesToPublish = FALSE;
         publish();
      }
   }
}

// Generate and publish the content for the resource list.
void ResourceList::publish()
{
   UtlBoolean publishingSuspended =
      getResourceListSet()->publishingSuspended();
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceList::publish mUserPart = '%s', publishingSuspended = %d",
                 mUserPart.data(), publishingSuspended);

   // Do the publishing if it's not suppressed.
   if (!publishingSuspended)
   {
      // Generate the notice body.
      HttpBody* body = generateBody(FALSE);
      // Publish the notice.
      getResourceListServer()->getEventPublisher().
         publish(mResourceListUri.data(),
                 getResourceListServer()->getEventType(),
                 getResourceListServer()->getEventType(),
                 1, &body, &mVersion);
      // Increment the version number for events..
      mVersion++;

      // Generate the consolidated notice body.
      body = generateBody(TRUE);
      // Publish the notice.
      getResourceListServer()->getEventPublisher().
         publish(mResourceListUriCons.data(),
                 getResourceListServer()->getEventType(),
                 getResourceListServer()->getEventType(),
                 1, &body, &mVersion);
   }
}

/* ============================ ACCESSORS ================================= */

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ INQUIRY =================================== */

//! Generate the HttpBody for the current state of the resource list.
HttpBody* ResourceList::generateBody(UtlBoolean consolidated) const
{
   // Construct the multipart body.
   // We add the <...> here, as they are used in all the contexts where
   // rlmiBodyPartCid appears.
   UtlString rlmiBodyPartCid;
   rlmiBodyPartCid += "<rlmi@";
   rlmiBodyPartCid += getResourceListServer()->getDomainName();
   rlmiBodyPartCid += ">";

   UtlString content_type(CONTENT_TYPE_MULTIPART_RELATED
                          ";type=\"" RLMI_CONTENT_TYPE "\""
                          ";start=\"");
   content_type += rlmiBodyPartCid;
   content_type += "\"";
   HttpBody* body = new HttpBodyMultipart(content_type);

   // This is the Resource List Meta-Information, XML describing the resources
   // and their instances.  It is the main part of the NOTIFY body.
   UtlString rlmi;

   // Generate the initial part of the RLMI.
   rlmi += "<?xml version=\"1.0\"?>\r\n";
   rlmi += "<list xmlns=\"" RLMI_XMLNS "\" uri=\"";
   XmlEscape(rlmi,
             consolidated ? mResourceListNameCons : mResourceListName);
   rlmi += "\" version=\"";
   rlmi.appendNumber(mVersion);
   rlmi += "\" fullState=\"true\">\r\n";

   // If we implemented names for resource lists, <name> elements would go here.

   // Iterate through the resources.
   UtlSListIterator resourcesItor(mResources);
   ResourceReference* resource;
   while ((resource = dynamic_cast <ResourceReference*> (resourcesItor())))
   {
      // Add the content for the resource.
      resource->generateBody(rlmi, *body, consolidated);
   }

   // Generate the postamble for the resource list.
   rlmi += "</list>\r\n";

   // Construct the RLMI body part.
   HttpBody rlmi_body(rlmi.data(), rlmi.length(), RLMI_CONTENT_TYPE);
   UtlDList rlmi_body_parameters;
   rlmi_body_parameters.append(new NameValuePair(HTTP_CONTENT_ID_FIELD,
                                                 rlmiBodyPartCid));

   // Attach the RLMI.
   body->appendBodyPart(rlmi_body, rlmi_body_parameters);

   // Clean up the parameter list.
   rlmi_body_parameters.destroyAll();

   return body;
}

// Dump the object's internal state.
void ResourceList::dumpState()
{
   // indented 4

   OsSysLog::add(FAC_RLS, PRI_INFO,
                 "\t    ResourceList %p mUserPart='%s', mResourceListName = '%s', "
                 "mResourceListUri = '%s', mResourceListNameCons = '%s', "
                 "mResourceListUriCons = '%s', mVersion = %d, mChangesToPublish = %d",
                 this, mUserPart.data(), mResourceListName.data(),
                 mResourceListUri.data(), mResourceListNameCons.data(),
                 mResourceListUriCons.data(), mVersion,
                 mChangesToPublish);

   UtlSListIterator i(mResources);
   ResourceReference* resource;
   while ((resource = dynamic_cast <ResourceReference*> (i())))
   {
      resource->dumpState();
   }
}

/**
 * Get the ContainableType for a UtlContainable-derived class.
 */
UtlContainableType ResourceList::getContainableType() const
{
   return ResourceList::TYPE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

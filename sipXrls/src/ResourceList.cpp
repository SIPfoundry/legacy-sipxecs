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

   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceList::_ this = %p, mUserPart = '%s', mResourceListName = '%s', mResourceListUri = '%s', mUserPartCons = '%s', mResourceListNameCons = '%s', mResourceListUriCons = '%s'",
                 this,
                 mUserPart.data(),
                 mResourceListName.data(), mResourceListUri.data(),
                 mUserPartCons.data(),
                 mResourceListNameCons.data(), mResourceListUriCons.data());

   // Publish the new list.
   setToBePublished();
}

// Destructor
ResourceList::~ResourceList()
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceList::~ mUserPart = '%s'",
                 mUserPart.data());

   mResourcesList.destroyAll();
   mChangesList.destroyAll();
   // Do not need to publish, as deletion of the resource instances
   // has triggered publishing.
}

/* ============================ MANIPULATORS ============================== */

// Delete all ResourceReference's in the resource list.
void ResourceList::deleteAllResources()
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceList::deleteAllResources mUserPart = '%s'",
                 mUserPart.data());

   mResourcesList.destroyAll();
   mChangesList.destroyAll();
}

void ResourceList::getAllResourceReferences(UtlSList& list)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceList::getAllResourceReferences this = %p",
                 this);

   // Iterate through the resource references.
   UtlSListIterator resourceListItor(mResourcesList);
   ResourceReference* resourceReference;
   while ((resourceReference = dynamic_cast <ResourceReference*> (resourceListItor())))
   {
      list.append(new UtlString(*resourceReference->getUri()));
   }
}

// Create and add a resource to the resource list.
bool ResourceList::addResource(const char* uri,
                               const char* nameXml,
                               const char* display_name,
                               const char* previous_uri)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceList::addResource mUserPart = '%s', uri = '%s', nameXml = '%s', display_name = '%s', previous_uri = '%s'",
                 mUserPart.data(), uri, nameXml, display_name, previous_uri);

   // See if 'uri' is already in the list of ResourceReference's.
   bool ret = !findResourceReference(uri);
   ResourceReference* previous = findResourceReference(previous_uri);

   if (ret)
   {
      // See if at the beginning of list or not
      if(previous)
      {
         ssize_t after_index = mResourcesList.index(previous) + 1;
         mResourcesList.insertAt(after_index, new ResourceReference(this,
                                                                    uri,
                                                                    nameXml,
                                                                    display_name));

         OsSysLog::add(FAC_RLS, PRI_DEBUG,
                       "ResourceList::addResource Added '%s' into the list after '%s' at index %lu in the ResourceList",
                       display_name, previous->getDisplayName().data(), (unsigned long)(mResourcesList.index(previous) + 1));

      }
      else
      {
         /// Added to the list but the previous node was not found, so add at the end.
         if(previous_uri)
         {
            mResourcesList.append(new ResourceReference(this,
                                                        uri,
                                                        nameXml,
                                                        display_name));

            OsSysLog::add(FAC_RLS, PRI_DEBUG,
                          "ResourceList::addResource Added at the end of the list: mUserPart = '%s', uri = '%s', nameXml = '%s', display_name = '%s'",
                          mUserPart.data(), uri, nameXml, display_name);
         }
         /// Added as the head of the list, since this node has no previous node to add after.
         else
         {
            mResourcesList.insertAt(0, new ResourceReference(this,
                                                             uri,
                                                             nameXml,
                                                             display_name));

            OsSysLog::add(FAC_RLS, PRI_DEBUG,
                          "ResourceList::addResource Added at the front of the list: mUserPart = '%s', uri = '%s', nameXml = '%s', display_name = '%s'",
                          mUserPart.data(), uri, nameXml, display_name);
         }
      }
   }
   else
   {
      OsSysLog::add(FAC_RLS, PRI_WARNING,
                    "ResourceList::addResource Resource URI '%s' is already in resource list '%s'",
                    uri, mUserPart.data());
   }

   return ret;
}

//! Checks if there are any changes in a resource.
//  Check if this resource is new to the list, or
//  if the display_name, nameXml, and the order is different.
bool ResourceList::resourceChanged(const char* uri,
                                   const char* nameXml,
                                   const char* display_name,
                                   const char* previous_uri)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceList::resourceChanged mUserPart = '%s', uri = '%s', nameXml = '%s', display_name = '%s', previous_uri = '%s'",
                 mUserPart.data(), uri, nameXml, display_name, previous_uri);

   // Gets the ResourceReference from the list.
   ResourceReference* resource_reference = findResourceReference(uri);
   ResourceReference* previous_resource_reference = findResourceReference(previous_uri);
   bool ret = false;

   bool correct_index = true;

   //! Sees if it can find the previous node in the RLS.
   //  Only 2 reasons it cannot find the previous node in the RLS
   //     1. The previous node could not be found based off of
   //        the previous uri because the previous_uri was a duplicate
   //        of another node and was not added or any weird glitches.
   //     2. The current node is the head node and the previous_uri is null.
   if(previous_resource_reference)
   {
      OsSysLog::add(FAC_RLS, PRI_DEBUG,
                    "ResourceList::resourceChanged Getting index location: current{'%s'} = '%lu', previous{'%s'} = '%lu'",
                    resource_reference->getDisplayName().data(), (unsigned long)mResourcesList.index(resource_reference), previous_resource_reference->getDisplayName().data(), (unsigned long)mResourcesList.index(previous_resource_reference));

      // Checks if the previous node is really before the current node for order checking.
      correct_index = mResourcesList.index(resource_reference) == mResourcesList.index(previous_resource_reference) + 1;
   }
   else
   {
      // Sees if a previous_uri exists
      if(previous_uri)
      {
         OsSysLog::add(FAC_RLS, PRI_ERR,
                       "ResourceList::resourceChanged The previous_uri could not be found.");

         // The previous node was not found therefore might be in the wrong index.
         correct_index = false;
      }
      else
      {
         OsSysLog::add(FAC_RLS, PRI_DEBUG,
                       "ResourceList::resourceChanged This is a head node");

         // Check if it is the first of the list
         correct_index = mResourcesList.index(resource_reference) == 0;
      }
   }

   // Checks if the current node is found and with the correct index
   if (resource_reference && correct_index)
   {
      // Compare the display_name and the nameXml from the XML to the resource_reference.
      if(resource_reference->compareNameXml(nameXml) != 0 || resource_reference->compareDisplayName(display_name) != 0)
      {
         // Has been changed.
         ret = true;

         OsSysLog::add(FAC_RLS, PRI_DEBUG,
                       "ResourceList::resourceChanged mUserPart = '%s', uri = '%s', nameXml => {old = '%s', new = '%s'}, display_name => {old = '%s', new = '%s'} changed",
                       mUserPart.data(), uri, resource_reference->getNameXml().data(), nameXml, resource_reference->getDisplayName().data(), display_name);
      }
      else
      {
         OsSysLog::add(FAC_RLS, PRI_DEBUG,
                       "ResourceList::resourceChanged mUserPart = '%s', uri = '%s', nameXml = '%s', display_name = '%s' not changed",
                       mUserPart.data(), uri, nameXml, display_name);
      }
   }
   else
   {
    // ResourceReference was not found or has wrong index therefore has been changed.
      ret = true;

      // Current node could not be found
      if(!resource_reference)
      {
         OsSysLog::add(FAC_RLS, PRI_DEBUG,
                       "ResourceList::resourceChanged Resource URI '%s' was not found so it is changed",
                       uri);
      }
      // Current node is found but has the wrong index
      if(!correct_index && resource_reference)
     {
   if(previous_resource_reference)
         {
            OsSysLog::add(FAC_RLS, PRI_DEBUG,
                          "ResourceList::resourceChanged Index is wrong: current = '%lu' should be = '%lu'",
                          (unsigned long)(mResourcesList.index(resource_reference)), (unsigned long)(mResourcesList.index(previous_resource_reference) + 1));
         }
         else
         {
            OsSysLog::add(FAC_RLS, PRI_DEBUG,
                          "ResourceList::resourceChanged Index is wrong because previous node could not be found or this is the head node");
         }
      }
   }

   return ret;
}

// Remove a resource from the resource list.
bool ResourceList::removeResource(const char* uri)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceList::removeResource mUserPart = '%s', uri = '%s'",
                 mUserPart.data(), uri);

   // See if 'uri' is in the list of ResourceReference's.
   bool ret = findResourceReference(uri);
   ResourceReference* obj = findResourceReference(uri);

   if (ret)
   {
      mResourcesList.remove(obj);
   }
   else
   {
      OsSysLog::add(FAC_RLS, PRI_WARNING,
                    "ResourceList::removeResource Resource URI '%s' is not found in resource list '%s' to delete",
                    uri, mUserPart.data());
   }

   return ret;
}

// Declare that the contents have changed and need to be published.
void ResourceList::setToBePublished(const UtlString* chgUri)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceList::setToBePublished chgUri = '%s', mUserPart = '%s'",
                 chgUri ? chgUri->data() : "[null]",
                 mUserPart.data());

   // Add this resource URI to mChangesList (if it is not already).
   if (chgUri != NULL && !chgUri->isNull())
   {
      if (!mChangesList.find(chgUri))
      {
         mChangesList.append(new UtlString(*chgUri));
      }
   }

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
      // Generate and publish the RFC 4662 ("full") notice body.
      genAndPublish(FALSE, mResourceListUri);

      // Generate and publish the consolidated notice body.
      genAndPublish(TRUE, mResourceListUriCons);

      // Flush the list of partial updates.
      mChangesList.destroyAll();
   }
}

// Incrementally remove and delete one component of the ResourceList.
UtlBoolean ResourceList::shrink()
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceList::shrink mUserPart = '%s'",
                 mUserPart.data());

   // Incrementally remove element from the ResourceReferences and delete it.
   // Get pointer to the first ResourceReference.
   ResourceReference* rr =
      dynamic_cast <ResourceReference*> (mResourcesList.first());

   // If one exists, delete it.
   if (rr) {
      mResourcesList.removeReference(rr);
      // A ResourceReference can be deleted outright because it contains
      // no lists.
      // It may cause deletion of the ResourceCached that it points to.
      delete rr;
   }

   // mChangesList is not handled here, as it is just a list of UtlStrings.

   return rr == NULL;
}

/* ============================ ACCESSORS ================================= */

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ INQUIRY =================================== */

//! Generate the HttpBody for the current state of the resource list.
HttpBody* ResourceList::generateRlmiBody(UtlBoolean consolidated,
                                         UtlBoolean fullRlmi,
                                         UtlSList& listToSend)
{
   if (OsSysLog::willLog(FAC_RLS, PRI_DEBUG))
   {
      UtlString l;

      UtlSListIterator resourcesItor(listToSend);
      ResourceReference* resource;
      while ((resource = dynamic_cast <ResourceReference*> (resourcesItor())))
      {
         l.append(*resource->getUri());
         l.append(",");
      }
      if (!l.isNull())
      {
         l.remove(l.length() - 1);
      }

      OsSysLog::add(FAC_RLS, PRI_DEBUG,
                    "ResourceList::generateRlmiBody cons=%d URI='%s' full=%d listToSend='%s'",
                    consolidated,
                    (consolidated ? mResourceListNameCons.data() : mResourceListName.data()),
                    fullRlmi,
                    l.data());
   }

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
   // Placeholder for version from SIP stack.
   rlmi += "\" version=\"" VERSION_PLACEHOLDER "\" ";

   // Generate either the full or the partial RLMI.
   if (fullRlmi)
   {
      rlmi += "fullState=\"true\">\r\n";
   }
   else
   {
      rlmi += "fullState=\"false\">\r\n";
   }

   // If we implemented names for resource lists, <name> elements would be added here.

   // Iterate through the resources.
   UtlSListIterator resourcesItor(listToSend);
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
                 "mResourceListUriCons = '%s', mChangesToPublish = %d",
                 this, mUserPart.data(), mResourceListName.data(),
                 mResourceListUri.data(), mResourceListNameCons.data(),
                 mResourceListUriCons.data(),
                 mChangesToPublish);

   UtlSListIterator i(mResourcesList);
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

// Search for a resource with a given reference (user-uri)
ResourceReference* ResourceList::findResourceReference(const char* uri)
{
   ResourceReference* ret = 0;

   // Iterate through the resource reference lists.
   UtlSListIterator resourceReferenceListItor(mResourcesList);
   ResourceReference* resourceReference;
   while (!ret &&
          (resourceReference = dynamic_cast <ResourceReference*> (resourceReferenceListItor())))
   {
      if (resourceReference->getUri()->compareTo(uri) == 0)
      {
         ret = resourceReference;
      }
   }

   return ret;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Generate the partial RLMI list.
// Generate list of ResourceReference's corresponding to the URIs in mResourcesList.
// Returns true if partialList is non-empty.
UtlBoolean ResourceList::genPartialList(UtlSList& partialList)
{
   UtlSListIterator resourcesItor(mResourcesList);
   ResourceReference* resource;
   UtlBoolean any_found = FALSE;

   // Check the resource list for changed items.
   while ((resource = dynamic_cast <ResourceReference*> (resourcesItor())))
   {
      UtlSListIterator changesItor(mChangesList);
      UtlString* changedRsrc;
      UtlBoolean found = FALSE;

      while ((changedRsrc = dynamic_cast <UtlString*> (changesItor())))
      {
         // Send a partial RLMI (only for resources that changed).
         if (strcmp(resource->getUri()->data(), changedRsrc->data()) == 0)
         {
            found = TRUE;
            any_found = TRUE;
         }
      }

      if (found)
      {
         partialList.append(resource);
      }
   }

   return any_found;
}

// Generate and publish the full and partial RLMI for the specified URI
// (full/consolidated) of a resource list.
// Both the Full and the Partial RLMI are sent to the SIP Subscribe Server.
// The Partial RLMI will then be sent out right away and the Full RLMI
// will be stored in the Subscribe Server to be sent on any initial
// SUBSCRIBEs and re-SUBSCRIBEs.
void ResourceList::genAndPublish(UtlBoolean consolidated, UtlString resourceListUri)
{
   const UtlBoolean RLMI_FULL = TRUE;
   const UtlBoolean RLMI_PARTIAL = FALSE;
   HttpBody* body;
   UtlSList partialList;

   // Generate and publish the fullState=TRUE notice body.
   // Note that the full-state publish() must be done before the partial-state
   // publish() to avoid race conditions with regard to starting a new
   // subscription.
   body = generateRlmiBody(consolidated, RLMI_FULL, mResourcesList);
   getResourceListServer()->getEventPublisher().
      publish(resourceListUri.data(),
              getResourceListServer()->getEventType(),
              getResourceListServer()->getEventType(),
              1, &body,
              RLMI_FULL,
              // Suppress generating notifications for this call of
              // SipPublishContentMgr::publish, because the call below
              // will generate notifications for the same subscribed-to
              // URIs.
              TRUE);

   // Generate and publish the fullState=FALSE notice body.
   // If there are no URIs in mChangesList, then there have been no changes
   // and publish() does not need to be called to trigger notifications.
   if (genPartialList(partialList))
   {
      body = generateRlmiBody(consolidated, RLMI_PARTIAL, partialList);
      getResourceListServer()->getEventPublisher().
         publish(resourceListUri.data(),
                 getResourceListServer()->getEventType(),
                 getResourceListServer()->getEventType(),
                 1, &body,
                 RLMI_PARTIAL,
                 // This call to SipPublishContentMgr::publish triggers
                 // notification.
                 FALSE);
   }
}


/* ============================ FUNCTIONS ================================= */

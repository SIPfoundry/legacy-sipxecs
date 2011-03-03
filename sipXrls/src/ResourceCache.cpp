//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include "ResourceCache.h"
#include "ResourceListSet.h"
#include "ResourceList.h"
#include "ResourceCached.h"
#include <os/OsSysLog.h>
#include <os/OsLock.h>
#include <os/OsEventMsg.h>
#include <utl/XmlContent.h>
#include <utl/UtlSListIterator.h>
#include <utl/UtlHashBagIterator.h>
#include <net/SipDialogEvent.h>
#include <net/NameValueTokenizer.h>
#include <net/NameValuePair.h>
#include <net/HttpMessage.h>
#include <net/SipMessage.h>
#include <xmlparser/tinyxml.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

const UtlContainableType ResourceCache::TYPE = "ResourceCache";


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
ResourceCache::ResourceCache(ResourceListSet* resourceListSet) :
   mResourceListSet(resourceListSet)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceCache:: this = %p",
                 this);
}

// Destructor
ResourceCache::~ResourceCache()
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceCache::~ this = %p",
                 this);

   // This assumes that all ResourceReference's have already been destroyed.
   mResources.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

// Construct a ResourceReference and tell whether it caused a ResourceCached
// to be created.
void ResourceCache::createResourceReference(ResourceList* resourceList,
                                            const char* uri,
                                            const char* nameXml,
                                            const char* display_name,
                                            ResourceReference*& rr,
                                            bool& resourceCreated)
{
   // Create the ResourceReference.
   rr = new ResourceReference(resourceList, uri, nameXml, display_name);

   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceCache::createResourceReference "
                 "rr = %p, resourceList = %p, uri = '%s', nameXml = '%s', display_name = '%s'",
                 rr, resourceList, uri, nameXml, display_name);

   // Get a ResourceCached for it.
   ResourceCached* rc;
   addReferenceToResource(rr,
                          uri,
                          rc,
                          resourceCreated);
   // Point ResourceReference to ResourceCached.
   rr->setResourceCached(rc);
}

// Get a pointer to the ResourceCached for a URI, creating it if necessary.
void ResourceCache::addReferenceToResource(ResourceReference* rr,
                                           const char* uri,
                                           ResourceCached*& rc,
                                           bool& resourceCreated)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceCache::addReferenceToResource this = %p, rr = %p, uri = '%s'",
                 this, rr, uri);

   // See if there is already a ResourceCached for the URI.
   UtlString uri_s(uri);
   rc = dynamic_cast <ResourceCached*> (mResources.find(&uri_s));
   // If not, create one.
   resourceCreated = rc == NULL;
   if (resourceCreated)
   {
      rc = new ResourceCached(this, uri);
      mResources.insert(rc);
   }
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceCache::addReferenceToResource this = %p, uri = '%s', ResourceCached = %p",
                 this, uri, rc);

   // Now add the ResourceReference to the ResourceCached's set.
   rc->addReference(rr);
}

// Destroy a ResourceReference and tell whether or not the ResourceCached was
// deleted.
bool ResourceCache::destroyResourceReference(ResourceReference* rr)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceCache::destroyResourceReference "
                 "rr = %p, rr->getUri() = '%s'",
                 rr, rr->getUri()->data());

   // Tell the ResourceCache that we no longer have a reference to the
   // ResourceCached.
   ResourceCached* rc = rr->getResourceCached();
   bool ret = deleteReferenceToResource(rr, rc);
   
   // Destroy the ResourceReference.
   delete rr;

   return ret;
}

/** Delete a ResourceReference for a ResourceCached, and delete the
 *  ResourceCached if it has no more references.
 */
bool ResourceCache::deleteReferenceToResource(ResourceReference* resourceReference,
                                              ResourceCached* resourceCached)
{
   // Remove pointer from ResourceReference to ResourceCached.
   resourceReference->setResourceCached(NULL);

   // Remove the ResourceReference from the ResourceCached's set.
   resourceCached->deleteReference(resourceReference);
   // If the ResourceCached has no ResourceReference's, delete it.
   bool ret = !resourceCached->hasReferences();
   if (ret)
   {
      mResources.removeReference(resourceCached);
      delete resourceCached;
   }
   return ret;
}

// Remove dialogs in terminated state and terminated resource instances.
void ResourceCache::purgeTerminated()
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceCache::purgeTerminated this = %p",
                 this);

   // Iterate through the resources.
   UtlHashBagIterator resourceItor(mResources);
   ResourceCached* resource;
   while ((resource = dynamic_cast <ResourceCached*> (resourceItor())))
   {
      resource->purgeTerminated();
   }
}

/* ============================ ACCESSORS ================================= */

// Get the ancestor ResourceListServer.
ResourceListServer* ResourceCache::getResourceListServer() const
{
   return mResourceListSet->getResourceListServer();
}

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ INQUIRY =================================== */

// Dump the object's internal state.
void ResourceCache::dumpState()
{
   // indented 4

   OsSysLog::add(FAC_RLS, PRI_INFO,
                 "\t    ResourceCache %p", this);
   UtlHashBagIterator i(mResources);
   ResourceCached* resource;
   while ((resource = dynamic_cast <ResourceCached*> (i())))
   {
      resource->dumpState();
   }
}

/**
 * Get the ContainableType for a UtlContainable-derived class.
 */
UtlContainableType ResourceCache::getContainableType() const
{
   return ResourceCache::TYPE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

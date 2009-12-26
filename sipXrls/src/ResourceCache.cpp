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

// Get a pointer to the ResourceCached for a URI, creating it if necessary.
ResourceCached* ResourceCache::addReferenceToResource(
   /// The requesting ResourceReference
   ResourceReference* resourceReference,
   /// The URI of the reference
   const char* uri)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceCache::addReferenceToResource this = %p, resourceReference = %p, uri = '%s'",
                 this, resourceReference, uri);

   // See if there is already a ResourceCached for the URI.
   UtlString uri_s(uri);
   ResourceCached* r = dynamic_cast <ResourceCached*> (mResources.find(&uri_s));
   // If not, create one.
   if (!r)
   {
      r = new ResourceCached(this, uri);
      mResources.insert(r);
   }
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceCache::addReferenceToResource this = %p, uri = '%s', ResourceCached = %p",
                 this, uri, r);

   // Now add the ResourceReference to the ResourceCahced's set.
   r->addReference(resourceReference);

   return r;
}

/** Delete a ResourceReference for a ResourceCached, and delete the
 *  ResourceCached if it has no more references.
 */
void ResourceCache::deleteReferenceToResource(
   /// The ResourceReference being deleted
   ResourceReference* resourceReference,
   /// The ResourceCached that is no longer referenced
   ResourceCached* resourceCached)
{
   // Remove the ResourceReference from the ResourceCached's set.
   resourceCached->deleteReference(resourceReference);
   // If the ResourceCahced has no ResourceReference's, delete it.
   if (!resourceCached->hasReferences())
   {
      mResources.removeReference(resourceCached);
      delete resourceCached;
   }
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

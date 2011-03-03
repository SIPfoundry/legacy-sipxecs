//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ResourceCache_h_
#define _ResourceCache_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include <utl/UtlContainableAtomic.h>
#include <utl/UtlHashBag.h>
#include <utl/UtlString.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class ResourceListServer;
class ResourceListSet;
class ResourceList;
class ResourceReference;
class ResourceCached;


/**
 * ResourceCache is a child of a ResourceListSet.
 * It contains the ResourceCached objects for all the resources in all
 * the ResourceList's.  All ResourceList's that contain a resource URI
 * contain ResourceReference objects that point to the same
 * ResourceCached.
 */

class ResourceCache : public UtlContainableAtomic
{
  public:

/* //////////////////////////// PUBLIC //////////////////////////////////// */

   //! Constructor.
   ResourceCache(/// Parent ResourceListSet.
                 ResourceListSet* resourceListSet);

   virtual ~ResourceCache();

   //! Construct a ResourceReference and tell whether it caused a ResourceCached
   // to be created.
   void createResourceReference(/// containing ResourceList
                                ResourceList* resourceList,
                                /// URI
                                const char* uri,
                                /// XML for <name> elements
                                const char* nameXml,
                                /// Display name for consolidated events.
                                const char* display_name,
                                /// Returned ResourceReference*
                                ResourceReference*& rr,
                                /// Returned true if ResourceCached created
                                bool& resourceCreated);

   //! Get a pointer to the ResourceCached for a URI, creating it if necessary.
   void addReferenceToResource(/// The requesting ResourceReference
                               ResourceReference* resourceReference,
                               /// The URI of the reference
                               const char* uri,
                               /// Returned ResourceCached*
                               ResourceCached*& rc,
                               /// Returned true if ResourceCached was created
                               bool& resourceCreated);

   //! Destroy a ResourceReference and tell whether or not the ResourceCached was
   // deleted.
   bool destroyResourceReference(/// The ResourceReference*
                                 ResourceReference* rr);

   /** Delete a ResourceReference for a ResourceCached, and delete the
    *  ResourceCached if it has no more references.
    *  Return true if this caused the ResourceCached to be deleted.
    */
   bool deleteReferenceToResource(
      /// The ResourceReference being deleted
      ResourceReference* resourceReference,
      /// The ResourceCached that is no longer referenced
      ResourceCached* resourceCached);

   //! Get the parent ResourceListSet.
   ResourceListSet* getResourceListSet() const;
   //! Get the ancestor ResourceListServer.
   ResourceListServer* getResourceListServer() const;

   //! Remove dialogs in terminated state and terminated resource instances.
   //  To be called immediately after publishing all changed resource lists.
   void purgeTerminated();

   //! Dump the object's internal state.
   void dumpState();

   /**
    * Get the ContainableType for a UtlContainable-derived class.
    */
   virtual UtlContainableType getContainableType() const;

   static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   //! Parent ResourceListSet.
   ResourceListSet* mResourceListSet;

   /** UtlHashBag containing the ResourceCached (which are indexed by
    *  their URIs).
    */
   UtlHashBag mResources;

};

/* ============================ INLINE METHODS ============================ */

// Get the parent ResourceListSet.
inline ResourceListSet* ResourceCache::getResourceListSet() const
{
   return mResourceListSet;
}

#endif  // _ResourceCache_h_

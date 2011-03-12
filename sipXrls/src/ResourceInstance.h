//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ResourceInstance_h_
#define _ResourceInstance_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include "ResourceNotifyReceiver.h"
#include <utl/UtlContainableAtomic.h>
#include <utl/UtlString.h>
#include <utl/UtlSList.h>
#include <xmlparser/tinyxml.h>
#include <net/HttpBody.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class SubscriptionSet;
class ResourceCached;
class ResourceList;
class ResourceListSet;
class ResourceListServer;


//! Container for an instance of a Resource within a ResourceList.
/** The ResourceInstance does not have an independent life beyond the
 *  ResourceListSet that contains it.  Access to it is locked by the
 *  ResourceListSet containing it.
 *  A ResourceInstance is created when a subscription is created and
 *  is destroyed when the subscription ends (by a SubscriptionSet).
 *  Conversely, when a ResourceInstance is destroyed, the destructor
 *  terminates its subscription (if it has not already been terminated).
 *  A ResourceInstance is a child of the SubscriptionSet that was created
 *  for the SUBSCRIBE request that created its subscription.
 *  The constructor/destructor update the mNotifyMap of the ResourceListSet
 *  so this ResourceInstance can always be found by its dialog handle.
 *  The ResourceInstance processes NOTIFY callbacks for its subscription
 *  that have been dispatched by the ResourceListSet -- it stores the new
 *  content and calls the publishing functions of the containing ResourceList.
 */
class ResourceInstance : public UtlContainableAtomic,
                         public ResourceNotifyReceiver
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   //! Construct an instance
   ResourceInstance(SubscriptionSet* parent,
                    ///< the parent SubscriptionSet
                    const char* instanceName,
                    ///< the instance name (dialog handle of the subscription)
                    const char* subscriptionState
                    ///< the initial subscription state
      );

   //! Destructor
   virtual ~ResourceInstance();

   //! Get the parent SubscriptionSet.
   SubscriptionSet* getSubscriptionSet() const;
   //! Get the ancestor ResourceCached.
   ResourceCached* getResourceCached() const;
   //! Get the ancestor ResourceListSet.
   ResourceListSet* getResourceListSet() const;
   //! Get the ancestor ResourceListServer.
   ResourceListServer* getResourceListServer() const;

   //! Get the instance name.
   const UtlString* getInstanceName() const;

   //! Return TRUE if the subscription state is "terminated".
   UtlBoolean isSubscriptionStateTerminated();

   //! Process a notify event callback.
   virtual void notifyEventCallback(const UtlString* dialogHandle,
                                    const UtlString* content);

   /** Process termination of the subscription of this ResourceInstance:
    *  Set the subscription state and remove the content.
    *  This cannot be included in the destructor, because it needs the
    *  subscriptionState argument, and the ResourceInstance is deleted much
    *  later, when ResourceCache::purgeTerminated() is called.
    */
   void terminateContent(const char* subscriptionState);

   //! Remove any dialogs in the terminated state.
   void purgeTerminatedDialogs();

   //! Add to the HttpBody the current state of the resource.
   void generateBody(/// the RLMI XML to be appended to
                     UtlString& rlmi,
                     /// the HttpBody to which to add a part for this resource
                     HttpBody& body,
                     /// True if resource instances are to be consolidated.
                     UtlBoolean consolidated,
                     /// The local display name (if consolidated = TRUE)
                     const UtlString& displayName) const;

   //! Dump the object's internal state.
   void dumpState() const;

   /**
    * Get the ContainableType for a UtlContainable-derived class.
    */
   virtual UtlContainableType getContainableType() const;

   static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   //! Destroy the contents of mXmlDialogs.
   void destroyXmlDialogs();

   //! Terminate the non-Terminated contents of mXmlDialogs.
   void terminateXmlDialogs();

   //! Parent SubscriptionSet.
   SubscriptionSet* mSubscriptionSet;

   //! The instance name, which is also the subscription dialog handle.
   UtlString mInstanceName;
   //! The subscription state.
   UtlString mSubscriptionState;

   //! Whether content is present for the resource.
   UtlBoolean mContentPresent;
   //! The current content for the resource (valid only if mContentPresent is true).
   UtlString mContent;

   /** A hash map that maps the dialog id's into UtlVoidPtr's containing
    *  TiXmlElement*'s for the XML of the dialog.  The XML is preprocessed
    *  to make it fast to dump it into consolidated resource list events.
    */
   UtlHashMap mXmlDialogs;

   //! Disabled copy constructor
   ResourceInstance(const ResourceInstance& rResourceInstance);

   //! Disabled assignment operator
   ResourceInstance& operator=(const ResourceInstance& rhs);

};

/* ============================ INLINE METHODS ============================ */

// Put #include of SubscriptionSet down here to avoid circular
// include problems.
#include "SubscriptionSet.h"

// Get the parent SubscriptionSet.
inline SubscriptionSet* ResourceInstance::getSubscriptionSet() const
{
   return mSubscriptionSet;
}

// Get the ancestor ResourceCached.
inline ResourceCached* ResourceInstance::getResourceCached() const
{
   return mSubscriptionSet->getResourceCached();
}

// Get the ancestor ResourceListSet.
inline ResourceListSet* ResourceInstance::getResourceListSet() const
{
   return mSubscriptionSet->getResourceListSet();
}

// Get the ancestor ResourceListServer.
inline ResourceListServer* ResourceInstance::getResourceListServer() const
{
   return mSubscriptionSet->getResourceListServer();
}

// Get the instance name.
inline const UtlString* ResourceInstance::getInstanceName() const
{
   return &mInstanceName;
}

#endif  // _ResourceInstance_h_

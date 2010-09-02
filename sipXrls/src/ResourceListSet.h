//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ResourceListSet_h_
#define _ResourceListSet_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include "ResourceCache.h"
#include <utl/UtlContainableAtomic.h>
#include <utl/UtlString.h>
#include <utl/UtlSList.h>
#include <utl/UtlHashMap.h>
#include <net/HttpBody.h>
#include <net/SipPublishContentMgr.h>
#include <net/SipSubscribeClient.h>
#include <os/OsBSem.h>
#include <os/OsTimer.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class ResourceListServer;
class ResourceList;
class ResourceCached;
class SubscriptionSet;
class ResourceInstance;


/**
 * This class is a set of ResourceList's.
 * In principle, ResourceListSet's can exist autonomously, although
 * the "publish" methods of subordinate objects assume that the ResourceListSet
 * is the child of a ResourceListServer.  :TODO: adjusting the code so that
 * ResourceListSet's can be entirely independent.
 * The ResourceListSet has a lock for all modifications.  A small number of
 * methods are allowed to be "externally called" by threads that do not
 * already possess the lock.
 * The ResourceListSet "externally callable" methods are responsible for
 * calling ResourceList::publish() when new content is available for a
 * resource list.
 */

class ResourceListSet : public UtlContainableAtomic
{
  public:

   // Enum to differentiate NOTIFY messages.
   // Max value must be less than sSeqNoIncrement.
   enum notifyCodes
   {
      // Publish any changed ResourceList's.
      PUBLISH_TIMEOUT,
   };

/* //////////////////////////// PUBLIC //////////////////////////////////// */

   //! Construct a resource list.
   ResourceListSet(/// Parent ResourceListServer.
                   ResourceListServer* resourceListServer);

   virtual ~ResourceListSet();

   //! Flag to indicate publish on timeout.
   UtlBoolean publishOnTimeout();

   //! Set the gap timeout.
   void setGapTimeout();

   /** Delete all ResourceList's and stop the publishing timer, so it
    *  is safe to destroy the other permanent components of ResourceListServer.
    */
   void finalize();

   //! Get the parent ResourceListServer.
   ResourceListServer* getResourceListServer() const;

   //! Get the contained ResourceCache.
   ResourceCache& getResourceCache();

   //! Create and add a resource list.
   //  Returns true if resource list was added, returns false if 'user'
   //  duplicates the name of an existing resource list, and hence,
   //  no new list was created.
   //  May be called externally.
   bool addResourceList(/// The user-part of the resource list URI for "full" events.
                        const char* user,
                        /// The user-part of the resource list URI for "consolidated" events.
                        const char* userCons,
                        /// The XML for the name of the resource list.
                        const char* nameXml);

   //! Delete all resource lists.
   //  May be called externally.
   void deleteAllResourceLists();

   //! Deletes a resource list.
   //  May be called externally.
   void deleteResourcesList(/// The user-part of the resource list URI to delete.
                           const char* user);

   //! Delete all resources from a resource list.
   //  May be called externally.
   void deleteAllResources(/// The user-part of the resource list URI.
                           const char* user);

   //! Deletes a single resource from a resource list.
   void deleteResource(/// The user-part of the resource list URI.
                       const char* user,
                       ///  The uri-part of the resource reference to delete from the resource list.
                       const char* uri);

   //! Get a list of the user-parts of all resource lists.
   //  May be called externally.
   //  The UtlString's added to 'list' are owned by 'list'.
   void getAllResourceLists(/// The list to add the user-parts to.
                            UtlSList& list);

   //! Get a list of the uri-parts of all resource references.
   //  for the user.
   //  May be called externally.
   //  The UtlString's added to 'list' are owned by 'list'.
   void getResourceReferences(/// The user to get the list for.
                              const char* user,
                              /// The list to add the uri-parts to.
                              UtlSList& list);

   //! Create and add a resource to a resource list.
   //  based on the previous_uri.
   //  Returns true if resource 'URI' was added, returns false if resource
   //  was not added.
   //  Adding can fail because 'user' is not the name of a resource list.
   //  Adding can fail because 'URI' duplicates (is string-equal to)
   //  the name of an existing resource.  (Duplicating resource URIs
   //  is forbidden because it makes it impossible to unambiguously
   //  process partial-state updates.)
   //  May be called externally.
   bool addResource(/// The user-part of the resource list URI.
                    const char* user,
                    /// The resource URI.
                    const char* uri,
                    /// The XML for the name of the resource.
                    const char* nameXml,
                    /// The display name for consolidated event notices
                    const char* display_name,
                    //! The uri of the previous node to add after
                    //  or if this is null then add as head node
                    //  or if it is not found then add as tail node
                    const char* previous_uri);

   //! Find the resource list in the RLS based in the user-part from the XML
   //  to compare the resource in the XML and the resource in the RLS
   //  based on the URI, nameXml, display_name, and previous_uri.
   //  If any changes were found then it returns true, false otherwise.
   //  It also returns false because 'user' is not the name of a resource list.
   //  May be called externally.
   bool resourceChanged(/// The user-part of the resource list URI.
                        const char* user,
                        /// The resource URI.
                        const char* URI,
                        /// The XML for the name of the resource.
                        const char* nameXml,
                        /// The display name for consolidated event notices
                        const char* display_name,
                        /// The uri from the previous resource for checking order
                        const char* previous_uri);

   //! Callback routine for subscription state events.
   //  May be called externally.
   //  Queues a message for the ResourceListTask to do the work.
   //  Can be called as a callback routine.
   static void subscriptionEventCallbackAsync(
      SipSubscribeClient::SubscriptionState newState,
      const char* earlyDialogHandle,
      const char* dialogHandle,
      void* applicationData,
      int responseCode,
      const char* responseText,
      long expiration,
      const SipMessage* subscribeResponse);

   //! Callback routine for subscription state events.
   //  May be called externally.
   //  Calls subscription methods, so cannot be called as a callback routine.
   void subscriptionEventCallbackSync(const UtlString* earlyDialogHandle,
                                      const UtlString* dialogHandle,
                                      SipSubscribeClient::SubscriptionState newState,
                                      const UtlString* subscriptionState);

   //! Callback routine for notify events.
   //  May be called externally.
   //  Queues a message for the ResourceListTask to do the work.
   //  Can be called as a callback routine.
   static bool notifyEventCallbackAsync(const char* earlyDialogHandle,
                                        const char* dialogHandle,
                                        void* applicationData,
                                        const SipMessage* notifyRequest);

   //! Callback routine for notify events.
   //  May be called externally.
   //  Calls subscription methods, so cannot be called as a callback routine.
   void notifyEventCallbackSync(const UtlString* dialogHandle,
                                const UtlString* content);

   /** Add a mapping for an early dialog handle to its handler for
    *  subscription events.
    *  Note that the handler is UtlContainable, not ResourceSubscriptionReceiver.
    *  Neither argument becomes owned by mSubscribeMap.
    */
   void addSubscribeMapping(UtlString* earlyDialogHandle,
                            UtlContainable* handler);

   /** Delete a mapping for an early dialog handle.
    */
   void deleteSubscribeMapping(UtlString* earlyDialogHandle);

   /** Add a mapping for a dialog handle to its handler for
    *  NOTIFY events.
    *  Note that the handler is UtlContainable, not ResourceNotifyReceiver,
    *  in parallel to addSubscribeMapping().
    *  Neither argument becomes owned by mSubscribeMap.
    */
   void addNotifyMapping(const UtlString& dialogHandle,
                         UtlContainable* handler);

   /** Delete a mapping for a dialog handle.
    *  May be either the dialog handle or the reversed dialog handle.
    *  Frees the UtlString objects that are the keys for mNotifyMap,
    *  but does not free the 'handler' that they are mapped to.
    */
   void deleteNotifyMapping(const UtlString* dialogHandle);

   /** Get the next sequence number for objects for the parent
    *  ResourceListServer.
    *  May only be called by a thread holding ResourceListSet::mSemaphore.
    */
   int getNextSeqNo();

   //! Split a userData value into the seqNo and "enum notifyCodes".
   static void splitUserData(int userData, int& seqNo, enum notifyCodes& type);

   //! Queue a request to call SipSubscribeClient::endSubscriptionGroup.
   void queueEndSubscription(/// the handle of the subscription
                             //  '*handle' is copied and caller does not need
                             //  to save it.
                             const char* handle);

   //! Retrieve an entry from mEventMap and delete it.
   //  May be called externally.
   //  Caller is responsible for freeing the value, but this method frees
   //  the UtlInt that is the key.
   UtlContainable* retrieveObjectBySeqNoAndDeleteMapping(int seqNo);

   //! Add a mapping for a ResourceCached's sequence number.
   void addResourceSeqNoMapping(/// the sequence number
                                int seqNo,
                                /// the ResourceCached
                                ResourceCached* resource);

   //! Delete a mapping for a ResourceCached's sequence number.
   void deleteResourceSeqNoMapping(/// the sequence number
                                   int seqNo);

   // The RLS attempts to publish all changes to resource lists promptly,
   // without publishing several updates to the same resource list
   // back-to-back.  Most changes are published by setting mChangesToPublish
   // in the ResourceList whose content has changed, and then starting
   // ResourceListSet.mPublishingTimer.oneshotAfter(ResourceListServer::mPublishingDelay).
   //
   // That timer (like all timers in the RLS) queues a message to
   // ResourceListTask, which calls ResourceListSet::publish, which calls
   // ResourceList::publishIfNecessary on every ResourceList.
   // ResourceList::publishIfNecessary, if mChangesToPublish set, calls
   // ResourceList::publish to publish the new contents (and reset
   // mChangesToPublish).
   //
   // Since reported dialogs that have terminated are supposed to be
   // published with their "terminated" state (and their termination
   // reasons, if possible) shown before they vanish from the resource
   // instance, terminated ResourceInstance's are not deleted immediately,
   // but rather are left to be published as such.
   // ResourceListSet::publish, after publishing the new content, calls
   // ResourceCached::purgeTerminated to destroy the ResourceInstance's.
   //
   // This is further complicated by the mechanism to suppress publishing
   // when there are large transient changes being made to the content.
   // Currently, this mechanism is used only by ResourceListFileReader when
   // it clears and reloads the ResourceListSet when resource-lists.xml
   // changes.
   //
   // Publishing is suppressed by calling ResourceListSet::suspendPublishing
   // and resumed by calling ResourceListSet::resumePublishing.  While
   // suspension is in effect, no publishing is done, and the
   // mChangesToPublish's are not reset.  When suspension ends, the usual
   // publishing is resumed.  Suspending publishing prevents publishing of
   // terminated dialogs, which is theoretically required.
   //
   //
   // The various changes that can be made to a resource list set and how
   // they get published:
   //
   // ResourceListSet
   //     add ResourceList
   //         need to publish the new resource list
   //
   //         ResourceList:: calls
   //         ResourceList::setToBePublished
   //
   //     delete ResourceList
   //         there is no way to publish the absence of a resource list
   //
   // ResourceList
   //     add Resource
   //         need to publish the new resource list
   //
   //         ResourceReference:: calls
   //         ResourceList::setToBePublished
   //
   //     delete Resource
   //         need to publish the new resource list
   //
   //         ResourceReference:: calls
   //         ResourceList::setToBePublished
   //
   // Resource (ResourceReference/ResourceCached)
   //     add SubscriptionSet (contact)
   //         no visible change to published resource lists
   //
   //     delete SubscriptionSet (contact)
   //         no visible change, but since the container
   //         of ResourceInstance's is being deleted, we
   //         have to publish the changes immediately, before
   //         the SubscriptionSet is destroyed
   //
   //         SubscriptionSet::~ sets all the resources to terminated state,
   //         then calls
   //         ResourceCached::setToBePublished(TRUE) to force immediate
   //         publication (which does nothing if publication is suppressed),
   //         then destroys the ResourceInstance's, then calls
   //         ResourceCached::setToBePublished(FALSE) so that the resource's
   //         state without the destroyed instances will be published (eventually)
   //
   // SubscriptionSet (contact)
   //     add ResourceInstance
   //         need to publish the new state of the resource lists
   //
   //         ResourceInstance:: calls
   //         ResourceCached::setToBePublished, which calls
   //         ResourceListSet::setToBePublished
   //
   //     delete ResourceInstance
   //         need to make sure the R.I. is published with terminated state
   //         before it vanishes
   //
   //         SubscriptionSet::deleteInstance calls
   //         ResourceCached::setToBePublished, which calls
   //         ResourceListSet::setToBePublished
   //
   //         ResourceInstance is reaped by the publishing process,
   //         after publishing the R.I. with terminated state:
   //         ResourceListSet::publish calls
   //         ResourceListCache::purgeTerminated
   //
   // ResourceInstance
   //     add dialog
   //         need to publish the new state of the resource lists containing
   //         this ResourceInstance
   //
   //         ResourceInstance::notifyEventCallback calls
   //         ResourceCached::setToBePublished, which calls
   //         ResourceListSet::setToBePublished
   //
   //     dialog terminated
   //         need to publish the new state of the resource lists
   //
   //         ResourceInstance::notifyEventCallback calls
   //         ResourceCached::setToBePublished, which calls
   //         ResourceListSet::setToBePublished
   //
   //         dialog is reaped by the publishing process,
   //         after publishing the R.I. with terminated state:
   //         ResourceListSet::publish calls
   //         ResourceListCache::purgeTerminated
   //
   //     dialog content changed
   //         need to publish the new state of the resource lists
   //
   //         ResourceInstance::notifyEventCallback calls
   //         ResourceCached::setToBePublished, which calls
   //         ResourceListSet::setToBePublished

   //! Suspend the effect of publish().
   //  May be called externally.
   //  Must be matched by a later call of resumePublishing.
   //  Multiple suspendPublishing() calls can be current at the same time.
   void suspendPublishing();

   //! Resume the effect of publish().
   //  May be called externally.
   //  Cancels one previous call of suspendPublishing.
   void resumePublishing();

   //! Returns TRUE if publish() should not be called.
   //  May NOT be called externally.
   UtlBoolean publishingSuspended();

   //! Declare that some content has changed and needs to be published.
   //  May NOT be called externally.
   void schedulePublishing();

   //! Publish all ResourceList's that have changes.
   //  May be called externally.
   void publish();

   /** Get the version number for the current consolidated RLMI being
    *  generated (or next to be generated).  Increments
    *  ResourceListSet::mVersion.
    */
   int getVersion() const;

   //! Dump the object's internal state.
   void dumpState();

   /**
    * Get the ContainableType for a UtlContainable-derived class.
    */
   virtual UtlContainableType getContainableType() const;

   static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

   //! Search for a resource list with a given name (user-part).
   ResourceList* findResourceList(const char* user);

   //! Swap the tags in a dialog handle.
   //  Part of the work-around for XSL-146.
   static void swapTags(const UtlString& dialogHandle,
                        UtlString& swappedDialogHandle);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   //! Parent ResourceListServer.
   ResourceListServer* mResourceListServer;

   /** Reader/writer lock for synchronization of the ResourceList and its
    *  contained Resources.
    */
   mutable OsBSem mSemaphore;

   /** The ResourceCache for holding the ResourceCached objects
    *  representing the resources of all member ResourceList's.
    */
   ResourceCache mResourceCache;

   /** List of ResourceList objects for all the resource lists in the
    *  ResourceListSet.
    */
   UtlSList mResourceLists;

   //! Map from early dialog handles to the objects that handle their events.
   //  The values are instances of subclasses of ResourceSubscriptionReceiver.
   //  The keys are UtlString's owned by the value objects.
   //  Thus, neither keys nor values are owned by mSubscribeMap.
   UtlHashMap mSubscribeMap;

   //! Map from dialog handles to the objects that handle their events.
   //  The values are instances of subclasses of ResourceNotifyReceiver.
   //  The keys are UtlString's owned by the value objects.
   UtlHashMap mNotifyMap;

   /** Map from UtlInt's containing sequence numbers to the objects they
    *  designate.
    */
   // The keys are UtlInt's which are allocated and freed when entries are
   // added and deleted from the map.  The handling of the objects differs
   // depending on the event type of the key sequence number.
   UtlHashMap mEventMap;

   /** Next sequence number to be assigned to objects for the parent
    *  ResourceListServer.
    */
   int mNextSeqNo;
   //! Amount to increment successive seqNo's.
   // Must be lowest 1 bit of seqNoMask.
   static const int sSeqNoIncrement;
   //! Mask to cause seqNo's to wrap around.
   static const int sSeqNoMask;

   /** Number of times suspendPublishing has been called without
    *  a corresponding call of resumePublishing.
    */
   unsigned int mSuspendPublishingCount;

   //! Timer to schedule the next publishing.
   OsTimer mPublishingTimer;

   //! Flag to indicate that we're about to publish something.
   UtlBoolean mPublishOnTimeout;

   //! version number for consolidated RLMI
   mutable int mVersion;

   //! Disabled copy constructor
   ResourceListSet(const ResourceListSet& rResourceListSet);

   //! Disabled assignment operator
   ResourceListSet& operator=(const ResourceListSet& rhs);

};

/* ============================ INLINE METHODS ============================ */

// Put #include's of ResourceListServer and ResourceCache down here to
// avoid circular include problems.
#include "ResourceListServer.h"

// Get the parent ResourceListServer.
inline ResourceListServer* ResourceListSet::getResourceListServer() const
{
   return mResourceListServer;
}

// Get the contained ResourceCache.
inline ResourceCache& ResourceListSet::getResourceCache()
{
   return mResourceCache;
}

// Get the version number for the current consolidated RLMI being generated (or next to be generated).
inline int ResourceListSet::getVersion() const
{
   return mVersion++;
}

#endif  // _ResourceListSet_h_

//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _AppearanceGroupSet_h_
#define _AppearanceGroupSet_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include "net/SipSubscribeClient.h"
#include "os/OsBSem.h"
#include "utl/UtlContainableAtomic.h"
#include "utl/UtlHashMap.h"
#include "utl/UtlString.h"
#include "utl/UtlSList.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class AppearanceAgent;
class AppearanceGroup;


/**
 * This class is a set of AppearanceGroup's.
 * In principle, AppearanceGroupSet's can exist autonomously, although
 * the "publish" methods of subordinate objects assume that the AppearanceGroupSet
 * is the child of a AppearanceAgent.  :TODO: adjusting the code so that
 * AppearanceGroupSet's can be entirely independent.
 * The AppearanceGroupSet has a lock for all modifications.  A small number of
 * methods are allowed to be "externally called" by threads that do not
 * already possess the lock.
 * The AppearanceGroupSet "externally callable" methods are responsible for
 * calling AppearanceGroup::publish() when new content is available for a
 * resource list.
 */

class AppearanceGroupSet : public UtlContainableAtomic
{
  public:

   // Enum to differentiate NOTIFY messages.
   // Max value must be less than sSeqNoIncrement.
   enum notifyCodes
   {
      // Refresh subscriptions for a Resource.
      REFRESH_TIMEOUT,
      // Publish any changed AppearanceGroup's.
      PUBLISH_TIMEOUT,
   };

/* //////////////////////////// PUBLIC //////////////////////////////////// */

   //! Construct a resource list.
   AppearanceGroupSet(/// Parent AppearanceAgent.
                   AppearanceAgent* appearanceAgent);

   virtual ~AppearanceGroupSet();


   //! Get the parent AppearanceAgent.
   AppearanceAgent* getAppearanceAgent() const;

   //! Create and add an Appearance Group.
   //  May be called externally.
   void addAppearanceGroup(/// The uri of the Appearance Group.
                        const char* user
                        );

   //! Create and add an Appearance Group.
   //  May be called externally.
   void removeAppearanceGroup(/// The uri of the Appearance Group.
                        const char* user
                        );

   //! Delete all Appearance Groups.
   //  May be called externally.
   void deleteAllAppearanceGroups();

   //! Get a list of the uris of all Appearance Groups.
   //  May be called externally.
   //  The UtlString's added to 'list' are owned by 'list'.
   void getAllAppearanceGroups(/// The list to add the uris to.
                            UtlSList& list);


   //! Callback routine for subscription state events.
   //  May be called externally.
   //  Queues a message for the AppearanceGroupTask to do the work.
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
   //  Queues a message for the AppearanceGroupTask to do the work.
   //  Can be called as a callback routine.
   static bool notifyEventCallbackAsync(const char* earlyDialogHandle,
                                        const char* dialogHandle,
                                        void* applicationData,
                                        const SipMessage* notifyRequest);

   //! Callback routine for notify events.
   //  May be called externally.
   //  Calls subscription methods, so cannot be called as a callback routine.
   void notifyEventCallbackSync(const UtlString* dialogHandle,
                                const SipMessage* msg);

   /** Add a mapping for an early dialog handle to its handler for
    *  subscription events.
    *  Note that the handler is UtlContainable, not ResourceSubscriptionReceiver.
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
    */
   void addNotifyMapping(const UtlString* dialogHandle,
                         UtlContainable* handler);

   /** Delete a mapping for a dialog handle.
    */
   void deleteNotifyMapping(const UtlString* dialogHandle);


   //! Search for a resource list with a given name (user-part).
   AppearanceGroup* findAppearanceGroup(const char* user);

   //! Swap the tags in a dialog handle.
   static void swapTags(const UtlString& dialogHandle,
                        UtlString& swappedDialogHandle);

   //! Dump the object's internal state.
   void dumpState();

   /**
    * Get the ContainableType for a UtlContainable-derived class.
    */
   virtual UtlContainableType getContainableType() const;

   static const UtlContainableType TYPE;    //< Class type used for runtime checking

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:



/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   //! Parent AppearanceAgent.
   AppearanceAgent* mAppearanceAgent;

   /** Reader/writer lock for synchronization of the AppearanceGroup and its
    *  contained Appearances.
    */
   mutable OsBSem mSemaphore;

   /** List of AppearanceGroup objects in the AppearanceGroupSet.
    */
   UtlSList mAppearanceGroups;

   //! Map from early dialog handles to the objects that handle their events.
   //  The values are instances of subclasses of ResourceSubscriptionReceiver.
   //  The keys are UtlString's owned by the value objects.
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

   //! version number for consolidated RLMI
   mutable int mVersion;

   //! Disabled copy constructor
   AppearanceGroupSet(const AppearanceGroupSet& rAppearanceGroupSet);

   //! Disabled assignment operator
   AppearanceGroupSet& operator=(const AppearanceGroupSet& rhs);

};

/* ============================ INLINE METHODS ============================ */

// Put #include's of AppearanceAgent down here to
// avoid circular include problems.
#include "AppearanceAgent.h"

// Get the parent AppearanceGroupServer.
inline AppearanceAgent* AppearanceGroupSet::getAppearanceAgent() const
{
   return mAppearanceAgent;
}

#endif  // _AppearanceGroupSet_h_

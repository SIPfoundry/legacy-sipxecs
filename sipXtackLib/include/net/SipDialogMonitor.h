//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _SIPDIALOGMONITOR_H_
#define _SIPDIALOGMONITOR_H_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsBSem.h>
#include <net/StateChangeNotifier.h>
#include <net/SipUserAgent.h>
#include <net/SipDialogMgr.h>
#include <net/SipSubscriptionMgr.h>
#include <net/SipSubscribeServerEventHandler.h>
#include <net/SipSubscribeServer.h>
#include <net/SipRefreshManager.h>
#include <net/SipSubscribeClient.h>
#include <net/SipPublishContentMgr.h>
#include <net/SipDialogEvent.h>
#include <utl/UtlSList.h>
#include <utl/UtlHashMap.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * A SipDialogMonitor object monitors the on/off hook status of all the SIP user
 * agents for a set of URIs by subscribing to dialog events for the URIs and
 * processing the resulting dialog events.
 *
 * The URI is considered to be off-hook if there are any active
 * dialogs (dialogs with state != 'terminated') on any of the UAs that send
 * NOTIFYs in response to a SUBSCRIBE to the URI.
 *
 * When status is updated, the information will be reported to all the
 * notifiers that have been registered using addStateChangeNotifier.
 *
 * Also, if toBePublished (mToBePublished) is set, the dialog status of
 * any groups of URIs containing the URI that has changed will be published
 * to any subscribers via a SipPublishContentMgr.
 * (This is not completely implemented.)
 */

class SipDialogMonitor
{
  public:

   SipDialogMonitor(SipUserAgent* userAgent, /**<
                                               * Sip user agent for sending out
                                               * SUBSCRIBEs and receiving NOTIFYs
                                               */
                    UtlString& domainName,   ///< sipX domain name
                    int hostPort,            ///< Host port
                    int refreshTimeout,      ///< refresh timeout for SUBSCRIBEs
                    bool toBePublished);     ///< option to publish for other subscriptions

   virtual ~SipDialogMonitor();

   /** URIs are added and deleted from groups, which are identified by
    *  names.  Each group has independent membership, and they may
    * overlap (which results in multiple subscriptions to that URI).
    * Groups are not evident in the events presented to notifiers
    * (except that a URI in multiple groups will be reported to a
    * notifier multiply), but they are central to published status, as
    * the groups are the "resource lists" which this object will be a
    * subscription server for.
    *
    * Note:  Groups should not overlap, as mDialogHandleList is indexed
    * by AOR only, and not group-and-AOR.
    */

   /// Add a URI to a group to be monitored
   // Return true if successful, false if not.
   bool addExtension(UtlString& groupName, Url& contactUrl);

   /// Remove a URI from a group to be monitored
   // Note:  Only removes the URI from the group specified.
   // Return true if successful, false if not.
   bool removeExtension(UtlString& groupName, Url& contactUrl);

   /// Register a StateChangeNotifier
   // notifier() will be called for every change in status of any URI that
   // has been added with addExtension().
   // Only one notifier is allowed per listUri.
   void addStateChangeNotifier(const char* listUri,
                               StateChangeNotifier* notifier);

   /// Unregister a StateChangeNotifier
   void removeStateChangeNotifier(const char* listUri);

  protected:
   friend class SipDialogMonitorTest;

   /// Add 'dialogEvent' as the last dialog event for AOR 'contact'.
   void addDialogEvent(UtlString& contact,
                       SipDialogEvent* dialogEvent,
                       const char* earlyDialogHandle,
                       const char* dialogHandle);

   /// Publish the dialog event package to the resource list
   void publishContent(UtlString& contact, SipDialogEvent* dialogEvent);

   /// Send the state change to the notifier
   void notifyStateChange(UtlString& contact, StateChangeNotifier::Status);

   /// Callback to handle notification of changes in the states of subscriptions.
   static void subscriptionStateCallback(SipSubscribeClient::SubscriptionState newState,
                                         const char* earlyDialogHandle,
                                         const char* dialogHandle,
                                         void* applicationData,
                                         int responseCode,
                                         const char* responseText,
                                         long expiration,
                                         const SipMessage* subscribeResponse);

   /// Callback to handle incoming NOTIFYs.
   static bool notifyEventCallback(const char* earlyDialogHandle,
                                   const char* dialogHandle,
                                   void* applicationData,
                                   const SipMessage* notifyRequest);

   /// Non-static callback to handle incoming NOTIFYs.
   void handleNotifyMessage(const SipMessage* notifyMessage,
                            const char* earlyDialogHandle,
                            const char* dialogHandle);

   /// Merge information from a dialogEvent into mDialogState.
   // Return StateChangeNotifier::ON_HOOK/OFF_HOOK depending on whether there
   // are any active dialogs for the subscription.
   // If the earlyDialogHandle is not in mDialogState, ignore the event, as
   // this is a NOTIFY due to an un-SUBSCRIBE.
   StateChangeNotifier::Status mergeEventInformation(SipDialogEvent* dialogEvent,
                                                     const char* earlyDialogHandle,
                                                     const char* dialogHandle);

   /// Create the dialog event state record for the SUBSCRIBE earlyDialogHandle.
   void createDialogState(UtlString* earlyDialogHandle);

   /// Delete the dialog event state record for the SUBSCRIBE earlyDialogHandle.
   void destroyDialogState(UtlString* earlyDialogHandle);

  private:

   // User agent to send SUBSCRIBEs and receive NOTIFYs.
   SipUserAgent* mpUserAgent;
   // The SIP domain used to construct the identity URI for the user agent.
   UtlString mDomainName;
   // The Contact URI for the user agent.
   UtlString mContact;
   // The (maximum) subscription refresh time for our subscriptions.
   int mRefreshTimeout;
   bool mToBePublished;

   OsBSem mLock;

   SipDialogMgr mDialogManager;
   SipRefreshManager* mpRefreshMgr;
   SipSubscribeClient* mpSipSubscribeClient;

   SipDialogMgr mDialogMgr;
   SipSubscriptionMgr* mpSubscriptionMgr;
   SipSubscribeServerEventHandler mPolicyHolder;
   SipPublishContentMgr mSipPublishContentMgr;
   SipSubscribeServer* mpSubscribeServer;

   // UtlHashMap mapping group names to SipResourceList's of URIs in
   // the groups.
   UtlHashMap mMonitoredLists;
   // The last dialogEvent received for each AOR that we are watching.
   UtlHashMap mDialogEventList;
   UtlHashMap mDialogHandleList;
   UtlHashMap mStateChangeNotifiers;
   // UtlHashMap mapping SUBSCRIBEs (via early dialog handles) to
   // UtlHashBag's that list the identifiers of all non-terminated dialogs
   // on the UAs for the subscribed-to URI.
   // Dialogs are identified by the string
   // "<dialog id (as given in the dialog event)><ctrl-A><dialog handle>".
   UtlHashMap mDialogState;

   /// Disabled copy constructor
   SipDialogMonitor(const SipDialogMonitor& rSipDialogMonitor);

   /// Disabled assignment operator
   SipDialogMonitor& operator=(const SipDialogMonitor& rhs);
};

#endif // _SIPDIALOGMONITOR_H_

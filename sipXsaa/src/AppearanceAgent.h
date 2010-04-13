//
//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _AppearanceAgent_h_
#define _AppearanceAgent_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include "net/SipDialogEvent.h"
#include "net/SipDialogMgr.h"
#include "net/SipPublishContentMgr.h"
#include "net/SipSubscribeClient.h"
#include "net/SipSubscribeServer.h"
#include "net/SipUserAgent.h"
#include "os/OsBSem.h"
#include "os/OsTime.h"
#include "persist/SipPersistentSubscriptionMgr.h"
#include "utl/UtlContainableAtomic.h"
#include "utl/UtlString.h"
#include "utl/UtlSList.h"
#include "AppAgentSubscribePolicy.h"
#include "AppAgentTask.h"
#include "AppearanceGroupFileReader.h"
#include "AppearanceGroupSet.h"

// DEFINES
#define SLA_EVENT_TYPE    DIALOG_EVENT_TYPE ";sla"    // for draft-anil-sipping-bla-02
#define ML_EVENT_TYPE     DIALOG_EVENT_TYPE ";ma"     // for draft-anil-sipping-bla-04
#define SHARED_EVENT_TYPE DIALOG_EVENT_TYPE ";shared" // for draft-ieft-bliss-shared-appearances

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


/**
 * The AppearanceAgent class holds the machinery for generating shared appearance events,
 * as described in draft-anil-sipping-bla-02.  It is known as the Shared Appearance Agent (SAA).
 * It contains an AppearanceGroupSet, which holds the list of shared AORs.
 * Each AppearanceGroup holds the content returned by subscriptions to those AORs.
 * As sets register to the shared AOR, Appearance objects are created to manage
 * the tracking of dialogs at that set.
 * It contains numerous SIP processing objects to handle SIP requests.
 * It contains a AppearanceAgentTask, which receives timer messages and executes
 * the appropriate changes on the AppearanceGroupSet.
 */

class AppearanceAgent : public UtlContainableAtomic
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   //! Construct a Shared Appearance Agent.
   AppearanceAgent(/** The host-part of the canonical form of the appearance AORs,
                        *  which is the sipX domain. */
                      const UtlString& domainName,
                      /// The realm used for authentication
                      const UtlString& realm,
                      /// Container for user definitions and their credentials.
                      SipLineMgr* lineMgr,
                      /// The TCP port to listen on.
                      int tcpPort,
                      /// The UDP port to listen on.
                      int udpPort,
                      /// The TLS port to listen on.
                      int tlsPort,
                      /// IP address to bind on.
                      const UtlString& bindIp,
                      /// The file describing the appearance groups.
                      UtlString* appearanceGroupFile,
                      /// Refresh interval for reinitializing connection to resource URIs, in seconds.
                      int refreshInterval,
                      /// The maximum resubscribe interval to request when making subscriptions.
                      int resubscribeInterval,
                      /// The minimum resubscribe interval.
                      int minResubscribeInterval,
                      /// Expiration to use while line is "seized"
                      int seizedResubscribeInterval,
                      /// Publishing delay, in msec.
                      int publishingDelay,
                      /// The maximum number of reg subscriptions per group.
                      int maxRegSubscInGroup,
                      /// Minimum expiration to grant incoming SUBSCRIBEs.
                      int serverMinExpiration,
                      /// Default expiration to grant incoming SUBSCRIBEs.
                      int serverDefaultExpiration,
                      /// Maximum expiration to grant incoming SUBSCRIBEs.
                      int serverMaxExpiration,
                      /// Name of the subscription DB to use (for testing purposes)
                      const UtlString&  subscriptionDbName = "subscription",
                      /// Name of the credentials DB to use (for testing purposes)
                      const UtlString&  credentialsDbName = "credential"
      );

   /// Applications should call shutdown() before calling the destructor
   virtual ~AppearanceAgent();

   //! Start the call processing components.
   void start();

   //! Shut down the call processing components.
   void shutdown();

   //! Dump the object's internal state.
   void dumpState();

   //! Get the canonical SIP domain name.
   // Return value is valid as long as the AppearanceAgent exists.
   const char* getDomainName() const;

   //! Get the From URI to be used for SUBSCRIBEs.
   const char* getServerFromURI() const;

   //! Get the Contact URI to be used for SUBSCRIBEs.
   const char* getServerContactURI() const;

   //! Get the event publisher.
   SipPublishContentMgr& getEventPublisher();

   //! Get the subscribe client.
   SipSubscribeClient& getSubscribeClient();

   // Get the subscribe server manager.
   SipSubscriptionMgr& getSubscriptionMgr();

   //! Get the AppearanceGroupSet.
   AppearanceGroupSet& getAppearanceGroupSet();

   //! Get the AppearanceAgentTask.
   AppearanceAgentTask& getAppearanceAgentTask();

   //! Get the AppearanceGroupFileReader.
   AppearanceGroupFileReader& getAppearanceGroupFileReader();

   //! Get the refresh interval.
   int getRefreshInterval() const;

   //! Get the resubscribe interval.
   int getResubscribeInterval() const;

   //! Get the resubscribe interval to use while line is seized.
   int getSeizedResubscribeInterval() const;

   //! Get the publishing delay.
   const OsTime& getPublishingDelay() const;

   //! Get the maximum number of reg subscriptions allowed for a group.
   size_t getMaxRegSubscInGroup() const;

   //! Get the server user agent.
   SipUserAgent& getServerUserAgent();

   //! Get the bulk add/delete delay (in msec).
   static int getChangeDelay();

   /**
    * Get the ContainableType for a UtlContainable-derived class.
    */
   virtual UtlContainableType getContainableType() const;

   static const UtlContainableType TYPE;    //< Class type used for runtime checking

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   //! SIP domain name.
   UtlString mDomainName;
   //! From URI for mServerUserAgent.
   UtlString mServerFromURI;
   //! Contact URI for mServerUserAgent.
   UtlString mServerContactURI;
   //! Name of the file containing the shared appearance AORs.
   UtlString mAppearanceGroupFile;
   //! Refresh interval for reinitializing connection to resource URIs, in seconds.
   int mRefreshInterval;
   //! Max resubscription interval to request whem making subscriptions, in seconds.
   int mResubscribeInterval;
   //! Minimum resubscription interval for subscriptions, in seconds.
   int mMinResubscribeInterval;
   //! Resubscription interval to use while line is "seized", in seconds.
   int mSeizedResubscribeInterval;
   //! Interval to delay after a content change before publishing, in msec.
   OsTime mPublishingDelay;
   //! The maximum number of reg subscriptions allowed for a group.
   size_t mMaxRegSubscInGroup;

   // The call processing objects.

   //! The SipUserAgent for making and receiving subscriptions.
   SipUserAgent mServerUserAgent;

   //! The SipPublishContentMgr.
   // This will contain the event content for every shared appearance URI
   // that the Appearance Agent services.
   SipPublishContentMgr mEventPublisher;

   //! Component for holding the subscription data.
   SipPersistentSubscriptionMgr mSubscriptionMgr;

   //! Component for granting subscription rights
   AppAgentSubscribePolicy mPolicyHolder;

   //! The SIP Subscribe Server.
   SipSubscribeServer mSubscribeServer;

   //! Support objects for SipSubscribeClient.
   SipDialogMgr mDialogManager;
   SipRefreshManager mRefreshMgr;

   //! The SipSubscribeClient for subscriptions we make to resources.
   SipSubscribeClient mSubscribeClient;

   /// The AppearanceAgentTask, which processes asynchronous events.
   AppearanceAgentTask mAppearanceAgentTask;

   //! The AppearanceGroupSet.
   AppearanceGroupSet mAppearanceGroupSet;

   //! The AppearanceGroupFileReader.
   AppearanceGroupFileReader mAppearanceGroupFileReader;

   //! Milliseconds of delay to allow between calls that add or delete resources
   static const int sChangeDelay;

   //! Disabled copy constructor
   AppearanceAgent(const AppearanceAgent& rAppearanceAgent);

   //! Disabled assignment operator
   AppearanceAgent& operator=(const AppearanceAgent& rhs);

   friend class AppearanceAgentTest;
};

/* ============================ INLINE METHODS ============================ */

//! Get the SIP domain name.
inline const char* AppearanceAgent::getDomainName() const
{
   return mDomainName.data();
}

//! Get the server From URI.
inline const char* AppearanceAgent::getServerFromURI() const
{
   return mServerFromURI.data();
}

//! Get the server Contact URI.
inline const char* AppearanceAgent::getServerContactURI() const
{
   return mServerContactURI.data();
}

//! Get the event publisher.
inline SipPublishContentMgr& AppearanceAgent::getEventPublisher()
{
   return mEventPublisher;
}

//! Get the subscribe client.
inline SipSubscribeClient& AppearanceAgent::getSubscribeClient()
{
   return mSubscribeClient;
}

/// Get the subscribe server manager.
inline SipSubscriptionMgr& AppearanceAgent::getSubscriptionMgr()
{
   return mSubscriptionMgr;
}

/// Get the AppearanceGroupSet.
inline AppearanceGroupSet& AppearanceAgent::getAppearanceGroupSet()
{
   return mAppearanceGroupSet;
}

/// Get the AppearanceAgentTask.
inline AppearanceAgentTask& AppearanceAgent::getAppearanceAgentTask()
{
   return mAppearanceAgentTask;
}

/// Get the AppearanceGroupFileReader.
inline AppearanceGroupFileReader& AppearanceAgent::getAppearanceGroupFileReader()
{
   return mAppearanceGroupFileReader;
}

// Get the refresh interval.
inline int AppearanceAgent::getRefreshInterval() const
{
   return mRefreshInterval;
}

// Get a resubscribe interval to request when making a subscription.
// Spread between mMinResubscribeInterval and mResubscribeInterval.
inline int AppearanceAgent::getResubscribeInterval() const
{
   return ( ( rand() % (mResubscribeInterval - mMinResubscribeInterval) ) + mMinResubscribeInterval);
}

// Get the resubscribe interval to request when the line is "seized".
// Not spread, as there is no burst of these.
inline int AppearanceAgent::getSeizedResubscribeInterval() const
{
   return mSeizedResubscribeInterval;
}

// Get the publishing delay.
inline const OsTime& AppearanceAgent::getPublishingDelay() const
{
   return mPublishingDelay;
}

// Get the maximum number of reg subscriptions allowed for a group.
inline size_t AppearanceAgent::getMaxRegSubscInGroup() const
{
   return mMaxRegSubscInGroup;
}

// Get the server user agent.
inline SipUserAgent& AppearanceAgent::getServerUserAgent()
{
   return mServerUserAgent;
}

// Get the bulk add/delete delay (in msec).
inline int AppearanceAgent::getChangeDelay()
{
   return sChangeDelay;
}

#endif  // _AppearanceAgent_h_

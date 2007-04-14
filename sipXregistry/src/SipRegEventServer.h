// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _SipRegEventServer_h_
#define _SipRegEventServer_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include <utl/UtlContainableAtomic.h>
#include <utl/UtlString.h>
#include <utl/UtlSList.h>
#include <net/SipPublishContentMgr.h>
#include <net/SipSubscribeClient.h>
#include <net/SipSubscribeServer.h>
#include <net/SipDialogMgr.h>
#include <net/SipUserAgent.h>
#include <net/SipSubscribeServerEventHandler.h>
#include <net/Url.h>
#include <persist/SipPersistentSubscriptionMgr.h>
#include <os/OsBSem.h>
#include <sipdb/RegistrationDB.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


/**
 * A SipRegEventServer contains the machinery for servicing
 * subscriptions to registration events, as described in RFC 3680.
 */

class SipRegEventServer
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   //! Construct a resource list.
   SipRegEventServer(/** The host-part of the canonical form of the resource list
                        *  URIs, which is the sipX domain. */
                     const UtlString& domainName,
                     /// The TCP port to listen on.
                     int tcpPort,
                     /// The UDP port to listen on.
                     int udpPort,
                     /// The TLS port to listen on.
                     int tlsPort);

   virtual ~SipRegEventServer();

   //! Generate and publish content for reg events for an AOR.
   void generateAndPublishContent(/// AOR as a string
                                  const UtlString& aorString,
                                  /// AOR as a Uri
                                  const Url& aorUri);

   //! Generate (but not publish) content for reg events for an AOR.
   void generateContent(/// AOR as a string
                        const UtlString& aorString,
                        /// AOR as a Uri
                        const Url& aorUri,
                        /// Returned pointer to HttpBody to publish.
                        HttpBody*& body,
                        /// Returned XML version number.
                        int& version);

   //! Get the SIP domain name for the resources.
   const UtlString* getDomainName();

   //! Get the Registration IMDB DB instance pointer.
   RegistrationDB* getRegistrationDBInstance();

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:
   
/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   //! SIP domain name.
   UtlString mDomainName;
   //! The local host-part.
   UtlString mLocalHostPart;
   //! Event type.
   UtlString mEventType;
   //! Outgoing address.
   UtlString mOutgoingAddress;
   //! The next event notice version number to use.
   //  Yes, this is global for all notices for all resource.
   //  This is contrary to the spec, but it works in all known cases.
   //  We need to rearchitect the SipSubscriptionMgr
   //  to allow the version attribute for each subscriber to be
   //  different.
   int mVersion;

   //! Pointer to the Registration DB instance.
   RegistrationDB* mpRegistrationDBInstance;

   // The call processing objects.

   //! The SipUserAgent.
   SipUserAgent mUserAgent;

   //! The SipPublishContentMgr.
   // This will contain the event content for every URI
   // and event type that the RLS services.
   SipPublishContentMgr mEventPublisher;

   //! Component for holding the subscription data.
   SipPersistentSubscriptionMgr mSubscriptionMgr; 

   //! Component for granting subscription rights
   SipSubscribeServerEventHandler mPolicyHolder;

   //! The SIP Subscribe Server.
   SipSubscribeServer mSubscribeServer;

   //! Disabled copy constructor
   SipRegEventServer(const SipRegEventServer& rSipRegEventServer);

   //! Disabled assignment operator
   SipRegEventServer& operator=(const SipRegEventServer& rhs);

};

/* ============================ INLINE METHODS ============================ */

inline const UtlString* SipRegEventServer::getDomainName()
{
   return &mDomainName;
}

inline RegistrationDB* SipRegEventServer::getRegistrationDBInstance()
{
   return mpRegistrationDBInstance;
}

#endif  // _SipRegEventServer_h_

//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _RegisterEventServer_h_
#define _RegisterEventServer_h_

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
 * A RegisterEventServer contains the machinery for servicing
 * subscriptions to registration events, as described in RFC 3680.
 */

class RegisterEventServer
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   //! Construct a resource list.
   RegisterEventServer(/** The host-part of the canonical form of the resource list
                        *  URIs, which is the sipX domain. */
                       const UtlString& domainName,
                       /// The TCP port to listen on.
                       int tcpPort,
                       /// The UDP port to listen on.
                       int udpPort,
                       /// The TLS port to listen on.
                       int tlsPort,
                       // Local IP address to bind on
                       const UtlString& bindIp);

   virtual ~RegisterEventServer();

   //! Generate and publish content for reg events for an AOR/instrument.
   //  Note that content will be published under the AOR, and if
   //  instrument is non-empty, under the appropriate ~~in~ URIs.
   void generateAndPublishContent(/// AOR as a string
                                  const UtlString& aorString,
                                  /// AOR as a Uri
                                  const Url& aorUri,
                                  /// instrument tag value
                                  const UtlString& instrument);

   //! Generate (but not publish) content for reg events for an AOR.
   void generateContentUser(/// The entity URI string to incorporate into the body.
                            const char* entity,
                            /// AOR as a string
                            const UtlString& aorString,
                            /// AOR as a Uri
                            const Url& aorUri,
                            /// Returned pointer to HttpBody to publish.
                            HttpBody*& body);

   //! Generate (but not publish) content for reg events for an instrument URI.
   void generateContentInstrument(/// The entity URI string to incorporate into the body.
                                  const char* entity,
                                  /// instrument tag value
                                  const UtlString& instrument,
                                  /// Returned pointer to HttpBody to publish.
                                  HttpBody*& body);

   //! Generate (but not publish) content for reg events for a user/instrument URI.
   void generateContentUserInstrument(/// The entity URI string to incorporate into the body.
                                      const char* entity,
                                      /// AOR as a string
                                      const UtlString& aorString,
                                      /// AOR as a Uri
                                      const Url& aorUri,
                                      /// instrument tag value
                                      const UtlString& instrument,
                                      /// Returned pointer to HttpBody to publish.
                                      HttpBody*& body);

   //! Generate (but not publish) content for reg events based on a ResultSet.
   void generateContent(/// The entity URI string to incorporate into the body.
                        const char* entityString,
                        /// The ResultSet containing the registrations to show
                        ResultSet& rs,
                        /// Returned pointer to HttpBody to publish.
                        HttpBody*& body);

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
   RegisterEventServer(const RegisterEventServer& rRegisterEventServer);

   //! Disabled assignment operator
   RegisterEventServer& operator=(const RegisterEventServer& rhs);

};

/* ============================ INLINE METHODS ============================ */

inline const UtlString* RegisterEventServer::getDomainName()
{
   return &mDomainName;
}

inline RegistrationDB* RegisterEventServer::getRegistrationDBInstance()
{
   return mpRegistrationDBInstance;
}

#endif  // _RegisterEventServer_h_

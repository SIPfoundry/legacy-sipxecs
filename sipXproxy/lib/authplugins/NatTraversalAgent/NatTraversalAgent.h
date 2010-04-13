//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _NATTRAVERSALAGENT_H_
#define _NATTRAVERSALAGENT_H_

// SYSTEM INCLUDES
#include "os/OsRWMutex.h"
#include "os/OsTimer.h"
#include "os/OsProcess.h"
#include "os/OsNotification.h"
#include "utl/UtlHashMap.h"
#include "net/SipOutputProcessor.h"
#include "sipdb/RegistrationDB.h"
#include "NatTraversalRules.h"

// APPLICATION INCLUDES
#include "AuthPlugin.h"

// DEFINES
#ifdef _nat_unit_tests_
#define CLEAN_UP_TIMER_IN_SECS               (1)
#else
#define CLEAN_UP_TIMER_IN_SECS               (60)
#endif

// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class NatTraversalRules;
class NatMaintainer;
class MediaRelay;
class RegistrationDB;
class CallTracker;

extern "C" AuthPlugin* getAuthPlugin(const UtlString& name);

/**
 * Auth Plugin responsible for implementing the nANT traversal feature.
 * The uses two hooks to examine the traffic passing by.  First,
 * the AuthPlugin::authorizeAndModify() method is used to see all
 * the requests that traverse the system.  The implementation of that
 * method is mainly responsible for allocating CallTracker objects to
 * track new calls made in through the  system.  Second,
 * the SipOutputProcessor::handleOutputMessage() is used to monitor  all
 * requests and responses exiting the system and notify the appropriate
 * CallTracker so that they can analyze and modify them as required to
 * facilitate NAT Traversal.
 */
class NatTraversalAgent : public AuthPlugin, SipOutputProcessor, OsNotification
{
  public:

   /// destructor
   virtual ~NatTraversalAgent();

   /// Called for any request - enforces the restrictions specified by authrules.
   virtual
      AuthResult authorizeAndModify(const UtlString& id, /**< The authenticated identity of the
                                                          *   request originator, if any (the null
                                                          *   string if not).
                                                          *   This is in the form of a SIP uri
                                                          *   identity value as used in the
                                                          *   credentials database (user@domain)
                                                          *   without the scheme or any parameters.
                                                          */
                                    const Url&  requestUri, ///< parsed target Uri
                                    RouteState& routeState, ///< the state for this request.
                                    const UtlString& method,///< the request method
                                    AuthResult  priorResult,///< results from earlier plugins.
                                    SipMessage& request,    ///< see AuthPlugin regarding modifying
                                    bool bSpiralingRequest, ///< request spiraling indication
                                    UtlString&  reason      ///< rejection reason
                                    );

   /// Called when SIP messages are about to be sent by proxy
   virtual void handleOutputMessage( SipMessage& message,
                                     const char* address,
                                     int port );

   /// Read (or re-read) the authorization rules.
   virtual void readConfig( OsConfigDb& configDb /**< a subhash of the individual configuration
                                                  * parameters for this instance of this plugin. */
                           );

   virtual void announceAssociatedSipRouter( SipRouter* sipRouter );

   // OsNotification virtual method implementation
   virtual OsStatus signal(intptr_t eventData);

  protected:
     // header manipulation routines for NAT traversal
     void adjustViaForNatTraversal( SipMessage& message, const char* address, int port );
     void adjustRecordRouteForNatTraversal( SipMessage& message, const char* address, int port );
     void adjustReferToHeaderForNatTraversal( SipMessage& message, const char* address, int port );
     bool restoreOriginalContact( SipMessage& request );

     // Call Tracker Manipulation methods
     CallTracker* createCallTrackerAndAddToMap( const UtlString& callId , ssize_t trackerHandle );
     CallTracker* getCallTrackerForMessage( const SipMessage& sipMessage );
     CallTracker* getCallTrackerFromCallId( const UtlString& callId );

  private:
   bool              mbNatTraversalFeatureEnabled;
   bool              mbOutputProcessorRegistrrationDone;
   UtlHashMap        mCallTrackersMap;
   NatTraversalRules mNatTraversalRules;
   SipRouter*        mpSipRouter;
   OsRWMutex         mMessageProcessingMutex;
   MediaRelay*       mpMediaRelay;
   NatMaintainer*    mpNatMaintainer;
   OsTimer           mCleanupTimer;
   RegistrationDB*   mpRegistrationDB;
   bool              mbConnectedToRegistrationDB;
   ssize_t           mNextAvailableCallTrackerHandle;

   friend AuthPlugin* getAuthPlugin(const UtlString& name);
   friend class NatTraversalAgentTest;

   // removes any modifications the NAT traversal feature has done to the Request-URI.
   // This ensures that when the request is received by the target, it will bear
   // the Request-URI it is expecting.
   void UndoChangesToRequestUri( SipMessage& message );

   /// Constructor - private so that only the factory can call it.
   NatTraversalAgent(const UtlString& instanceName ///< the configured name for this plugin instance
                    );

// @cond INCLUDENOCOPY

   /// There is no copy constructor.
   NatTraversalAgent(const NatTraversalAgent& nocopyconstructor);

   /// There is no assignment operator.
   NatTraversalAgent& operator=(const NatTraversalAgent& noassignmentoperator);
// @endcond INCLUDENOCOPY
};

#endif // _NATTRAVERSALAGENT_H_

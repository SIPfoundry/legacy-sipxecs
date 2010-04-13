//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _NATMANINTAINER_H_
#define _NATMANINTAINER_H_

// SYSTEM INCLUDES
#include <vector>

// APPLICATION INCLUDES
#include <os/OsTask.h>
#include <net/SipMessage.h>
#include <utl/UtlString.h>
#include "sipdb/RegistrationDB.h"
#include "sipdb/SubscriptionDB.h"
#include "NatTraversalAgentDataTypes.h"

// DEFINES
#define MAX_IP_ADDRESS_STRING_SIZE (15)
// NAMESPACES
using namespace std;
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipRouter;
class NatTraversalRules;

/** When a User Agent behind a NAT/FW sends a message to sipXecs, it creates
 *  a pinhole that allows sipXecs to communicate back to it.  This pinhole
 *  is maintained open by the NAT/FW until the traffic flowing through it stops.
 *  sipXecs requires this pinhole to be maintained open so that it can send
 *  SIP messages to the User Agent behind the NAT/FW.   To that end, the NAT
 *  Traversal feature includes a component that will periodically send a SIP
 *  message to every UA known to the system that is behind a NAT.
 *  Basically, sipXecs needs to maintain pinholes for every endpoint behind a
 *  NAT/FW that it may send messages to.  These endpoints are:
 *    1. REMOTE_NATED endpoints that are registered to sipXecs:  On an incoming
 *       call to such endpoints, sipXecs will have to send an INVITE to their
 *       public IP:port therefore requiring the pinhole to be open;
 *    2. REMOTE_NATED endpoints that have active subscriptions on the sipXecs:
 *       sipXecs will need to send NOTIFY messages to these endpoints’ public
 *       IP:port therefore requiring the pinhole to be open;
 *    3. REMOTE_NATED endpoints that are involved in a call:  While on a call,
 *       an endpoint behind a remote NAT may be the recipient of in-dialog
 *       requests which will be sent to their public IP:Port therefore requiring
 *       the pinhole to be open.
 *  In non-HA deployments, item #1 is a superset of items #2 and #3 however in
 *  HA deployments these items are not fully overlapping so to ensure full coverage,
 *  all three items must be considered by the pinhole-maintaining component.
 *  That NatMaintainer is a class implemented in code unit ‘NatMaintainer.cpp’ and
 *  is responsible for maintaining NAT pinholes alive.   An instance of the NatMaintainer
 *  class is created by NatTraveralAgent only when the NAT Traversal feature is enabled.
 *  The list of pinholes that it needs to maintain is built from three sources:
 *    1. Registration DB: NatMaintainer queries the registration DB looking for unexpired
 *       contacts that containing a ‘x-sipX-privcontact’ URL parameter.
 *    2. Subscription DB: NatMaintainer queries the subscription DB looking for unexpired
 *       contacts that containing a ‘x-sipX-privcontact’ URL parameter.
 *    3. External Pinhole List: NatMaintainer exposes methods to be invoked by components
 *       of the NAT Traversal feature to dynamically add and remove pinholes that need to
 *       be maintained.  Such methods are used by the CallTracker when REMOTE NATED
 *       endpoints initiate calls to ensure that their pinholes get maintained by
 *       NatMaintainer for the duration of the call.
 */
class NatMaintainer : public OsTask
{

public:
   NatMaintainer( SipRouter* sipRouter );
   virtual ~NatMaintainer();
   /// Start running the NatMaintainer task.
   virtual int run( void* runArg );
   /// Stop running the NatMaintainer task.
   virtual void requestShutdown( void );

   /** Add transport (protocol/address/port) to the list of endpoints to which
    *  we must send keepalives.
    */
   void addEndpointToKeepAlive   ( const TransportData& endpointToKeepAlive );
   /// Remove transport from the list of endpoints.
   void removeEndpointToKeepAlive( const TransportData& endpointKeptAlive );

private:
   /// Information that NatMaintainer keeps about an endpoint.
   struct KeepAliveEndpointDescriptor
   {
      KeepAliveEndpointDescriptor() :
         mLastRefreshRoundNumber( 0 ){ mIpAddress[0] = 0; }
      /// The last refresh round number that we sent a keepalive to this endpoint.
      uint32_t mLastRefreshRoundNumber;
      /// The address to which the keepalive was sent in round mLastRefreshRoundNumber.
      char     mIpAddress[ MAX_IP_ADDRESS_STRING_SIZE + 1 ];
   };

   uint32_t mRefreshRoundNumber;      ///< Counter of refresh rounds.
   RegistrationDB* mpRegistrationDB;  ///< The registration DB.
   SubscriptionDB* mpSubscriptionDB;  ///< The subscription DB.
   int             mNextSeqValue;     ///< Sequence number to use in next OPTIONS wave 
   UtlString       mBaseCallId;       ///< Call-id from which all the others will be derived
   
   OsMutex mTimerMutex;
   ///< Time-outs of mTimerMutex.acquire() provide the timing signal.
   SipRouter* mpSipRouter;            ///< SipRouter through which to send keepalives.
   /** Array, indexed by endpoint listening port, of addresses and what keepalive
    *  round we last sent keepalives to them.
    */
   KeepAliveEndpointDescriptor* mpEndpointsKeptAliveList;
   SipMessage* mpKeepAliveMessage;    ///< The keepalive message.
   /// List of TransportData's describing endpoints to which to send keepalives.
   UtlSList    mExternalKeepAliveList;
   OsMutex     mExternalKeepAliveListMutex; ///< mutex protecting mExternalKeepAliveList

   /// Send keepalives to the list of URIs (as UtlString's) in contactList.
   void sendKeepAliveToContactList( UtlSList& contactList );
   /// Send keepalives to the endpoints in mExternalKeepAliveList.
   void sendKeepAliveToExternalKeepAliveList( void );
   /// Send a keepalive to the specified address/port.  (Transport is always UDP.)
   void sendKeepAliveToEndpoint( const char* pIpAddress, uint16_t portNumber );

};

#endif // _NATMANINTAINER_H_

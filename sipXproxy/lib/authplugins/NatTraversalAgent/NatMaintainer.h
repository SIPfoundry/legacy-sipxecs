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
 *  The sipXecs requires this pinhole to be maintained open so that it can send 
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
 *       contacts that containing a ‘x-sipX-pubcontact’ URL parameter.
 *    2. Subscription DB: NatMaintainer queries the subscription DB looking for unexpired 
 *       contacts that containing a ‘x-sipX-pubcontact’ URL parameter.
 *    3. External Pinhole List: NatMaintainer exposes methods to be invoked by components 
 *       of the NAT Traversal feature to dynamically add and remove pinholes that need to 
 *       be maintained.  Such methods are used by the CallTracker when REMOTE NATED 
 *       endpoints initiate calls to ensure that their pinholes get maintained by 
 *       NatMaintainer for the duration of the call.
 */
class NatMaintainer : public OsTask
{

public:
   NatMaintainer( SipRouter* sipRouter, NatTraversalRules* pNatTraversalRules );
   virtual ~NatMaintainer();
   virtual int run( void* runArg );
   virtual void requestShutdown( void );

   void addEndpointToKeepAlive   ( const TransportData& endpointToKeepAlive );
   void removeEndpointToKeepAlive( const TransportData& endpointKeptAlive );

private:
   struct KeepAliveEndpointDescriptor
   {
      KeepAliveEndpointDescriptor() :
         mlastRefreshRoundNumber( 0 ){ mIpAddress[0] = 0; }
      uint32_t mlastRefreshRoundNumber;
      char     mIpAddress[ MAX_IP_ADDRESS_STRING_SIZE + 1 ];
   };

   uint32_t mRefreshRoundNumber;      //
   RegistrationDB* mpRegistrationDB;  //    
   SubscriptionDB* mpSubscriptionDB;  //    
   OsMutex mTimerMutex;               // mutex used to generate heartbeat;   
   SipRouter* mpSipRouter;
   NatTraversalRules* mpNatTraversalRules;
   KeepAliveEndpointDescriptor* mpEndpointsKeptAliveList;
   SipMessage* mpKeepAliveMessage;
   UtlSList    mExternalKeepAliveList;
   OsMutex     mExternalKeepAliveListMutex;
   
   void sendKeepAliveToContactList( UtlSList& contactList );   
   void sendKeepAliveToExternalKeepAliveList( void );   
   void sendKeepAliveToEndpoint( const char* pIpAddress, uint16_t portNumber );

};
   
#endif // _NATMANINTAINER_H_




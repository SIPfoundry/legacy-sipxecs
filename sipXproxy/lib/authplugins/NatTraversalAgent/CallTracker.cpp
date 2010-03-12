//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include "os/OsSysLog.h"

// APPLICATION INCLUDES
#include "SessionContext.h"
#include "NatMaintainer.h"
#include "NatTraversalRules.h"
#include "CallTracker.h"
#include "os/OsProcess.h"
#include "utl/UtlHashMapIterator.h"

// DEFINES
#define VIA_SESSION_HANDLE_TAG ("id")

const UtlContainableType CallTracker::TYPE = "CallTracker";
ssize_t CallTracker::sNextAvailableSessionContextHandle = 0;


CallTracker::CallTracker( ssize_t handle,
                          const NatTraversalRules* pNatRules,
                          MediaRelay* pMediaRelayToUse,
                          const RegistrationDB* pRegistrationDB,
                          NatMaintainer* pNatMaintainer,
                          UtlString instanceNameForRouteState ) :
   mHandle( handle ),
   mpNatTraversalRules( pNatRules ),
   mpMediaRelayToUse( pMediaRelayToUse ),
   mpRegistrationDB( pRegistrationDB ),
   mInstanceNameForRouteState( instanceNameForRouteState ),
   mNumberOfTicksWithoutActiveDialogs( 0 ),
   mpNatMaintainer( pNatMaintainer ),
   mpCallerPinholeInformation( 0 ),
   mpSavedOriginalSdpOfferCopy( 0 )
{
   mPid = OsProcess::getCurrentPID();
   OsSysLog::add(FAC_NAT, PRI_DEBUG, "+CallTracker tracker %p created; Handle=%zd+",
                                       this,
                                       mHandle );
}

CallTracker::~CallTracker()
{
   if( mpCallerPinholeInformation )
   {
      if( mpNatMaintainer )
      {
         mpNatMaintainer->removeEndpointToKeepAlive( *mpCallerPinholeInformation );
      }
      delete mpCallerPinholeInformation;
      mpCallerPinholeInformation = 0;
   }

   mSessionContextsMap.destroyAll();
   mBranchIdToSessionHandleMap.destroyAll();
   mSdpLibrary.destroyAll();
   delete mpSavedOriginalSdpOfferCopy;
   OsSysLog::add(FAC_NAT, PRI_DEBUG, "-CallTracker tracker %p deleted; Handle=%zd-",
                                       this,
                                       mHandle );
}

bool CallTracker::notifyIncomingDialogFormingInvite( SipMessage& request, RouteState& routeState, const EndpointDescriptor*& prCaller, const EndpointDescriptor*& prCallee )
{
   bool                bResult = false;
   UtlString           sessionContextHandle;
   SessionContext*     pSessionContext;

   // We need to process and incoming dialog-forming request.  In some scenarios, it is possible
   // for such a request to visit us twice.  If there is no session-id encoded in the RouteState,
   // it indicates that this is the first time we see it.
   if( !getSessionContextHandle( routeState, sessionContextHandle ) )
   {
      // we are not tracking this session yet.  Allocate a new session context to track
      // this particular fork and save a copy of the message's SDP body if not already done - this
      // may be useful if the request spirals back to us and we need to restore a patched
      // SDP to its original form.
      pSessionContext = createSessionContextAndSetHandle( request, routeState, true, sessionContextHandle );
      if( pSessionContext )
      {
         prCaller = &pSessionContext->getEndpointDescriptor( CALLER );
         prCallee = &pSessionContext->getEndpointDescriptor( CALLEE );
         if( !mpSavedOriginalSdpOfferCopy )
         {
            mpSavedOriginalSdpOfferCopy = const_cast<SdpBody*>( request.getSdpBody() );
         }
         bResult = true;
      }
      else
      {
         OsSysLog::add( FAC_NAT, PRI_ERR, "CallTracker[%zd]::notifyIncomingDialogFormingInvite[1] failed to create session context ",
                        mHandle );
      }
   }
   else
   {
      // the RouteState already has a session-id which means that we are already tracking this
      // session.  There are several scenarios that can lead up to that situation.  Consider this:
      //
      // User A--------sipXecs1-----------sipXecs2-------User C(call forwarded to User B)
      // User B---+
      // User B---+
      //
      // Users A & B are registered against sipXecs1 and User C is registered againt sipXecs2.
      // User B is registered from two endpoints.
      // User C has its sipXecs-based call forward all calls set to SetB@sipXecs1.
      // When User A calls User C, the INVITE takes the following path:
      //           UserA-->sipXecs1-->sipXecs2-->sipXecs1-->UserB
      //                                                 +->UserB
      // A SessionContext is created when sipXecs1 is visited the first time and its session-id
      // is encoded in the RouteState.  When sipXecs1 is revisited by the INVITE it sees that
      // the RouteState already carries a session-id which tells it that this is a revisiting
      // INVITE.

      // If no special handling is performed here, several problems can appear.
      //
      // Problem #1- callee's real location wasn't known when SessionContext was created on first visit
      // ==============================================================================================
      //     When the dialog-forming INVITE was first seen by sipXecs1, the target was UserC@sipxecs2.
      //     Given that the call was routed via a SIP trunk, sipXecs1 didn't know the real location of the
      //     callee and therefore pegged the callee a being at an UNKNOWN location and imposed the use
      //     of a media relay using the media relay's public IP adddress as the media connection address
      //     in the SDP presented to the callee.
      //     When the dialog-forming INVITE comes back to sipxecs1,  the request target of the INVITE, namely
      //     UserB@sipxecs1, is known to sipXecs1 and can therefore promote its location information
      //     from UNKNOWN to its precise location based on the information it collected from the set
      //     at registration time.  The SessionContext that was initially created by the CallTracker to
      //     handle the NAT traversal was created at the time when the location of the callee was unknown.
      //     Now that the location is known, the old SessionContext can be abandonned and a new one created
      //     that will choose whether or not to involve a media relay based on an accurate representation
      //     of the caller and callee's locations.
      //
      // Solution to Problem #1
      // ======================
      // - Restore the SDP to its original state (i.e. before any transformation) if it got changed by us.
      // - Allocate a new SessionContext that will be responsible handling all the dialogs forked off
      //   of this INVITE using the updated callee location information.
      // - Remove any 'id' param containing the handle of the original SessionContext in the Vias
      //   of the request.  This procedure will prevent the original SessionContext from handling
      //   responses pertaining to the newly created SessionContext.
      pSessionContext = getSessionContextFromHandle( sessionContextHandle );
      if( pSessionContext )
      {
         // First, allocate a new SessionContext that will take care of this new fork.
         UtlString handleOfnewSessionHandle;
         pSessionContext = createSessionContextAndSetHandle( request, routeState, true, handleOfnewSessionHandle );
         if( pSessionContext )
         {
            prCaller = &pSessionContext->getEndpointDescriptor( CALLER );
            prCallee = &pSessionContext->getEndpointDescriptor( CALLEE );
            bResult = true;

            // Second, restore the SDP to its original form as saved by the original SessionContext
            if( mpSavedOriginalSdpOfferCopy )
            {
               request.setBody( mpSavedOriginalSdpOfferCopy->copy() );
            }

            // third, remove the handle of the SessionContext being replaced in the Vias
            removeSessionHandleFromVias( request, sessionContextHandle );
         }
         else
         {
            OsSysLog::add( FAC_NAT, PRI_ERR, "CallTracker[%zd]::notifyIncomingDialogFormingInvite[2] failed to create session context ",
                           mHandle );
         }
      }

      // Problem #2- sipXecs2 may not know how to reach UserB directly
      // =============================================================
      //     As we can see in the example above, the sipXecs1 is visited twice but the SipRouter
      //     logic is such that a Record-Route is added the first time it sees the request and
      //     not the others.  This means that when the request arrives at UserB's sets it will
      //     have Record-route: <sip:sipXecs2>,<sip:sipXecs1>. That particular arrangement
      //     means that although sipXecs1 was the proxy that routed the dialog-forming
      //     request to UserB's sets, sipXecs2 will be responsible for routing all subsequent in-dialog
      //     requests.  Since UserB's sets are registered against sipXecs1 only, it is the only
      //     proxy that truly knows the public and private IP address information of UserB's sets
      //     and therefore the only one that can successfully deliver requests to them accross NATs.
      //     Even if sipXecs2 could somehow "learn" the sets public IP addresses, if UserB's set happened to
      //     be behind a non-full cone NAT  sipXecs2 will not have the ability to send requests to
      //     that set as they will be rejected by the NAT because pinholes exist between the set and sipXecs2
      //
      // Solution to Problem #2
      // ======================
      // To solve this problem, the following piece of logic will ask the RouteState to add a copy of the
      // Record-Route header when it is updated in cases where sipXproxy is not already at the top of the
      // route set.
      UtlString topRecordRoute;
      if( routeState.isFound() )
      {
         if( request.getRecordRouteUri( 0, &topRecordRoute ) )
         {
            Url topRecordRouteUrl( topRecordRoute );
            UtlString topRecordRouteHost;
            topRecordRouteUrl.getHostAddress( topRecordRouteHost );
            int topRecordRoutePort = topRecordRouteUrl.getHostPort();

            if( ( topRecordRouteHost != mpNatTraversalRules->getProxyTransportInfo().getAddress() ||
                  topRecordRoutePort != mpNatTraversalRules->getProxyTransportInfo().getPort() ) &&
                ( topRecordRouteHost != mpNatTraversalRules->getPublicTransportInfo().getAddress() ||
                  topRecordRoutePort != mpNatTraversalRules->getPublicTransportInfo().getPort() ) )
            {
               routeState.addCopy();
            }
         }
      }
   }

   // If the method successfully completed and if the caller is a remote worker then add it to the
   // list of endpoints whose NAT pinholes need to be kept alive for the duration of the call.
   // NOTE: the called party is already handled the NatMaintainer's RegDB lookups.
   if( bResult == true )
   {
      if( prCaller->getLocationCode() == REMOTE_NATED &&
          prCaller->getPublicTransportAddress().getTransportProtocol().compareTo( "udp", UtlString::ignoreCase ) == 0 &&
          !mpCallerPinholeInformation &&
          mpNatMaintainer )
      {
         mpCallerPinholeInformation = new TransportData( prCaller->getPublicTransportAddress().getAddress(),
                                                         prCaller->getPublicTransportAddress().getPort() );

         mpNatMaintainer->addEndpointToKeepAlive( *mpCallerPinholeInformation );
      }
   }
   return bResult;
}


bool CallTracker::handleRequest( SipMessage& message, const char* address, int port )
{
   // message is a request.  If this is a request we care about, it will contain
   // a SIP_SIPX_SESSION_CONTEXT_ID_HEADER header containing the value of the session context
   // tracking the session and a SIP_SIPX_FROM_CALLER_DIRECTION_HEADER if the request
   // is in the Caller->Callee direction.  For more on these headers, look for
   // 'OPTIMIZATION CODE' comments in NatTraversalAgent.cpp file.
   bool result = false;
   const char *pSessionHandle = 0;
   if( ( pSessionHandle = message.getHeaderValue( 0, SIP_SIPX_SESSION_CONTEXT_ID_HEADER ) ) )
   {
      UtlString sessionHandle( pSessionHandle );
      message.removeHeader( SIP_SIPX_SESSION_CONTEXT_ID_HEADER, 0 );

      bool bFromCallerToCallee = false;
      if( message.getHeaderValue( 0, SIP_SIPX_FROM_CALLER_DIRECTION_HEADER ) )
      {
         bFromCallerToCallee = true;
         message.removeHeader( SIP_SIPX_FROM_CALLER_DIRECTION_HEADER, 0 );
      }

      SessionContext* pSessionContext;
      pSessionContext = getSessionContextFromHandle( sessionHandle );
      if( pSessionContext )
      {
         // Responses to this request may not contain the RouteState which normally
         // provides the Session Handle used to retrieve the SessionContext.
         // To compensate for that, if the request is one that the SessionContext
         // wants to track, as indicated by a 'true' return value from
         // pSessionContext->handleRequest(), the session handle is encoded in the
         // Via header to allow SessionContext retrieval for
         // RouteState-less responses.
         result = true;
         if( pSessionContext->handleRequest( message, address, port, bFromCallerToCallee ) )
         {
            setSessionHandleInVia( message, 1, sessionHandle );
         }
      }
      else
      {
         OsSysLog::add(FAC_NAT, PRI_ERR, "CallTracker[%zd]::handleRequest failed to retrieve session context "
                                         "associated with handle '%s'", mHandle, sessionHandle.data() );
      }
   }
   else
   {
      // we can see that log entry in 'normal' cases when a dialog-forming INVITE we processed
      // is coming back to us and is spiraling, i.e. carries a X-Sipx-Spiral: header.
      OsSysLog::add(FAC_NAT, PRI_DEBUG, "CallTracker[%zd]::handleRequest did not find SIP_SIPX_SESSION_CONTEXT_ID_HEADER in message"
                                      , mHandle );
   }
   deleteSessionContextsReadyForDeletion();
   return result;
}

void CallTracker::handleResponse( SipMessage& message, const char* address, int port )
{
   // Outgoing SIP message is a response.  We cannot rely on the presence of RouteState
   // in responses.  Given that, some other way had to be devised to link this response
   // to a SessionContext object.  Use the SesionContext handle that was encoded in the
   // top Via.  That handle is used as a key to index the mSessionContextsMap which yields the
   // SessionContext object.
   //
   UtlString branchId;
   SessionContext* pSessionContext;

   if( ( pSessionContext = getSessionContextFromVia( message, 0 ) ) != 0  )
   {
      pSessionContext->handleResponse( message, address, port );
   }
   else
   {
      OsSysLog::add(FAC_NAT, PRI_DEBUG, "CallTracker[%zd]::handleResponse: no session context present in response", mHandle );
   }
   deleteSessionContextsReadyForDeletion();
}

bool CallTracker::handleCleanUpTimerTick( void )
{
   bool bPleaseDeleteMeNow = false;
   ssize_t numberOfTrackedDialogs = 0;
   // send a timer event to all instantiated SessionContext objects
   UtlHashMapIterator sessionContextIterator( mSessionContextsMap );

   while( sessionContextIterator() )
   {
      SessionContext *pSessionContext;
      pSessionContext = dynamic_cast<SessionContext*>( sessionContextIterator.value() );

      pSessionContext->handleCleanUpTimerTick();
      numberOfTrackedDialogs += pSessionContext->getNumberOfTrackedDialogs();
   }

   // check if have have been without an active dialog long enough
   // to delete ourself
   if( numberOfTrackedDialogs == 0 )
   {
      mNumberOfTicksWithoutActiveDialogs++;
      if( mNumberOfTicksWithoutActiveDialogs >= MAX_TIMER_TICK_COUNTS_BEFORE_CALL_TRACKER_CLEAN_UP )
      {
         bPleaseDeleteMeNow = true;
      }
   }
   else
   {
      mNumberOfTicksWithoutActiveDialogs = 0;
   }
   deleteSessionContextsReadyForDeletion();
   return bPleaseDeleteMeNow;
}

void CallTracker::deleteSessionContextsReadyForDeletion( void )
{
   ssize_t numberOfSessionContextsToDelete = mListOfSessionContextsReadyForDeletion.size();
   if( numberOfSessionContextsToDelete > 0 )
   {
      ssize_t index;
      UtlString handleForSessionContextsToDelete;

      for( index = 0; index < numberOfSessionContextsToDelete; index++ )
      {
         handleForSessionContextsToDelete = mListOfSessionContextsReadyForDeletion[ index ];
         mSessionContextsMap.destroy( &handleForSessionContextsToDelete );
      }
      mListOfSessionContextsReadyForDeletion.clear();
   }
}

void CallTracker::reportSessionContextReadyForDeletion( const UtlString& sessionContextHandle )
{
   // remove SessionContext from Map
   mListOfSessionContextsReadyForDeletion.push_back( sessionContextHandle );
}

bool CallTracker::getSessionHandleFromVia( const SipMessage& message, int viaIndex, UtlString& sessionHandle ) const
{
   bool bSessionHandleFound = false;
   UtlString viaValue;
   sessionHandle.remove( 0 );

   if( message.getViaFieldSubField( &viaValue, viaIndex ) )
   {
      // in most cases the Via will contain a proprietary tag called 'id' which carries
      // the handle of the SessionContext - this is the fastest way retrieve the SessionContext
      // handle so try it first.
      if( message.getViaTag( viaValue, VIA_SESSION_HANDLE_TAG, sessionHandle ) )
      {
         bSessionHandleFound = true;
      }
      else
      {
         // The Via didn't contain the information we required to find the SessionContext object.
         // This can happen in cases where the sipXproxy waits for the 'best' failure response to INVITEs.
         // In such cases, for reasons that remain unknown,  the Via headers contained in the
         // 'best' response selected by sipXproxy will be stripped of our special 'id' tag
         // which is normally used to find the correct SessionContext.
         // If we are dealing with a non-200 final response for INVITES then try to find the
         // SessionContext corresponding to the Via's topmost branchId using the information
         // contained in mBranchIdToSessionHandleMap.
         int seqNum;
         UtlString seqMethod;
         message.getCSeqField( &seqNum, &seqMethod );

         if( message.isResponse() &&
             message.getResponseStatusCode() >= SIP_3XX_CLASS_CODE &&
             seqMethod.compareTo( SIP_INVITE_METHOD ) == 0 )
         {
            UtlString branchId;
            if( message.getViaTag( viaValue, "branch", branchId ) )
            {
               UtlString* pSessionHandle;
               pSessionHandle = dynamic_cast<UtlString*>(mBranchIdToSessionHandleMap.findValue( &branchId ) );
               if( pSessionHandle )
               {
                  sessionHandle = *pSessionHandle;
                  bSessionHandleFound = true;
               }
            }
         }
      }
   }
   return bSessionHandleFound;
}

SessionContext* CallTracker::getSessionContextFromVia( const SipMessage& message, int viaIndex ) const
{
   SessionContext* pRetrievedSessionContext = 0;
   UtlString       sessionHandle;

   if( getSessionHandleFromVia( message, viaIndex, sessionHandle ) )
   {
      pRetrievedSessionContext = getSessionContextFromHandle( sessionHandle );
   }
   return pRetrievedSessionContext;
}

bool CallTracker::setSessionHandleInVia( SipMessage& message, int viaIndex, const UtlString& sessionHandle ) const
{
   return message.setViaTag( sessionHandle, VIA_SESSION_HANDLE_TAG, viaIndex );
}

bool CallTracker::removeSessionHandleFromVias( SipMessage& message, const UtlString& sessionHandleToRemove ) const
{
   bool bSessionHandleRemoved = false;
   UtlString viaValue;
   UtlString tempSessionHandle;
   int viaIndex = 0;

   while( message.getViaFieldSubField( &viaValue, viaIndex ) )
   {
      if( message.getViaTag( viaValue, VIA_SESSION_HANDLE_TAG, tempSessionHandle ) &&
          tempSessionHandle.compareTo( sessionHandleToRemove ) == 0 )
      {
         bSessionHandleRemoved = message.setViaTag( "nil", VIA_SESSION_HANDLE_TAG, viaIndex );
         break;
      }
      else
      {
         viaIndex++;
      }
   }
   return bSessionHandleRemoved;
}

bool CallTracker::getBranchId( const SipMessage& message, int viaIndex, UtlString& branchId ) const
{
   bool bBranchIdFound = false;
   UtlString viaValue;

   if( message.getViaFieldSubField( &viaValue, viaIndex ) && message.getViaTag( viaValue, "branch", branchId ) )
   {
      bBranchIdFound = true;
   }
   return bBranchIdFound;
}

SessionContext* CallTracker::createSessionContextAndSetHandle( const SipMessage& request,
                                                               RouteState& routeState,
                                                               bool bTrackBranchId,
                                                               UtlString& sessionContextHandle )
{
   SessionContext *pSessionContext = 0;

   if( generateSessionContextHandleFromRequest( sessionContextHandle ) )
   {
      OsSysLog::add(FAC_NAT, PRI_DEBUG, "CallTracker[%zd]::createSessionContextAndSetHandle generated handle: '%s' ",
                    mHandle, sessionContextHandle.data() );

      // session context handle successfully generated - allocate a new SessionContext
      // object to track the session and insert it into our mSessionContextsMap.
      UtlString* pMapKey = new UtlString( sessionContextHandle );
      pSessionContext = new SessionContext( request, mpNatTraversalRules, sessionContextHandle, mpMediaRelayToUse, mpRegistrationDB, this );
      if( pSessionContext && mSessionContextsMap.insertKeyAndValue( pMapKey, pSessionContext ) )
      {
         // Session Context successfully inserted into our mSessionContextsMap.  Encode
         // its handle in the RouteState to facilitate SessionContext look-ups while
         // processing in-dialog requests.
         setSessionContextHandle( routeState, sessionContextHandle );

         // map the branch id of the topmost via to the Session Context handle if requested
         if( bTrackBranchId )
         {
            UtlString branchId;
            if( getBranchId( request, 0, branchId ) )
            {
               OsSysLog::add(FAC_NAT, PRI_DEBUG, "CallTracker[%zd]::createSessionContextAndSetHandle now tracking branch Id '%s'", mHandle, branchId.data() );
               mBranchIdToSessionHandleMap.destroy( &branchId );
               mBranchIdToSessionHandleMap.insertKeyAndValue( new UtlString( branchId ), new UtlString( sessionContextHandle ) ) ;
            }
         }
      }
      else
      {
         delete pSessionContext;
         delete pMapKey;
         pSessionContext = 0;
         OsSysLog::add(FAC_NAT, PRI_ERR, "CallTracker[%zd]::createSessionContextAndSetHandle failed to insert "
                                           "new session context into map.  key : '%s'",
                                           mHandle, sessionContextHandle.data() );
      }
   }
   else
   {
      OsSysLog::add(FAC_NAT, PRI_ERR, "CallTracker[%zd]::createSessionContextAndSetHandle failed to generate new handle",
                                       mHandle );
   }
   return pSessionContext;
}

bool CallTracker::getSessionContextHandle( const RouteState& routeState, UtlString& handle ) const
{
   return routeState.getParameter( mInstanceNameForRouteState.data(), SESSION_CONTEXT_ID_PARAM, handle );
}

bool CallTracker::setSessionContextHandle( RouteState& routeState, const UtlString& handle ) const
{
   routeState.setParameter( mInstanceNameForRouteState.data(), SESSION_CONTEXT_ID_PARAM, handle );
   return true;
}

bool CallTracker::unsetSessionContextHandle( RouteState& routeState ) const
{
   routeState.unsetParameter( mInstanceNameForRouteState.data(), SESSION_CONTEXT_ID_PARAM );
   return true;
}

bool CallTracker::generateSessionContextHandleFromRequest( UtlString& handle )
{
   handle.remove(0);
   bool bHandleGenerated = true;
   char tempBuffer[50];
#ifdef _nat_unit_tests_
   sprintf( tempBuffer, "%u-%zd", 1234, sNextAvailableSessionContextHandle );
#else
   sprintf( tempBuffer, "%u-%zd", mPid, sNextAvailableSessionContextHandle );
#endif
   sNextAvailableSessionContextHandle++;
   handle = tempBuffer;
   return bHandleGenerated;
}

SessionContext* CallTracker::getSessionContextFromHandle( const UtlString& sessionContextHandle ) const
{
   SessionContext* pSessionContext = 0;
   pSessionContext = dynamic_cast<SessionContext*>(mSessionContextsMap.findValue( &sessionContextHandle ) );
   return pSessionContext;
}

UtlContainableType CallTracker::getContainableType( void ) const
{
   return CallTracker::TYPE;
}

unsigned CallTracker::hash() const
{
   return directHash();
}

int CallTracker::compareTo(UtlContainable const *rhsContainable ) const
{
   int result = -1;

   ssize_t otherHandle = static_cast<const CallTracker*>(rhsContainable)->mHandle;
   if ( rhsContainable->isInstanceOf( SessionContext::TYPE ) )
   {
      if( mHandle == otherHandle )
      {
         result = 0;
      }
      else
      {
         result = mHandle < otherHandle  ? -1 : 1;

      }
   }
   return result;
}

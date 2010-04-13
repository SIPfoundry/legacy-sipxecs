//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _CALLTRACKER_H_
#define _CALLTRACKER_H_

// SYSTEM INCLUDES
#include <vector>

// APPLICATION INCLUDES
#include "NatTraversalAgentDataTypes.h"
#include "utl/UtlContainable.h"
#include "utl/UtlString.h"
#include "os/OsProcess.h"

// DEFINES
#define SESSION_CONTEXT_ID_PARAM          "id"
#define CALLER_PUBLIC_TRANSPORT_PARAM     "CrT"
#define CALLEE_PUBLIC_TRANSPORT_PARAM     "CeT"
#define MAX_TIMER_TICK_COUNTS_BEFORE_CALL_TRACKER_CLEAN_UP (2)

// NAMESPACES
using namespace std;
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class NatTraversalRules;
class NatMaintainer;
class SessionContext;
class MediaRelay;

class CallTrackerInterfaceForSessionContext
{
public:
   /*!
    * Method to be invoked by a SessionContext instance to signal to its owner
    * that it is done and can be deleted.  The owner, upon receiving that callback
    * may delete the SessionContext on the spot therefore the calling instance must
    * not perform any further action after calling this method.
    *
    * \param [in] - handle of SessionContext object that is ready for deletion.
    */
   virtual void reportSessionContextReadyForDeletion( const UtlString& sessionContextHandle ) = 0;

   virtual ~CallTrackerInterfaceForSessionContext(){};
};

/**
 * Class used to track all the forks associated with a given call.  For
 * each new fork encountered, this class will allocate a SessionContext
 * that implements the state machine required to follow the media offers/
 * answers and intervene when required to allow NAT traversal.
 */
class CallTracker : public CallTrackerInterfaceForSessionContext, public UtlContainable
{
public:
   CallTracker( const ssize_t handle,
                const NatTraversalRules* pNatRules,
                MediaRelay* pMediaRelayToUse,
                const RegistrationDB* pRegistrationDB,
                NatMaintainer* pNatMaintainer,
                UtlString instanceNameForRouteState );

   virtual ~CallTracker();

   /*! Method to be called to notify the CallTracker that a dialog
    *  forming-request has been seen.  This method allows the
    *  CallTracker to initialize internal data structures to be ready to track
    *  that fork of the call.  It also returns EndpointDescriptors
    *  for both the Caller and the Callee back to the the method's caller.
    *
    * \note This method is used to let the CallTracker prepare itself to
    *       track a fork but this method does not affect the CallTracker's
    *       state machine.  The handleRequest() still needs to be
    *       called for that request.
    *
    * \param [in] request    - dialog-forming request
    * \param [in] routeState - route state information associated with the
    *                          request
    * \param [out] prCaller  - will hold a pointer to caller's EndpointDescriptor
    * \param [out] prCallee  - will hold a pointer to callee's EndpointDescriptor
    *
    * \return - true if request was processed properly.
    */
   bool notifyIncomingDialogFormingInvite( SipMessage& request, RouteState& routeState, const EndpointDescriptor*& prCaller, const EndpointDescriptor*& prCallee );

   // //////////////////// //
   // State machine events //
   // //////////////////// //

   /*********************************************
    * That serie of handle...() methods is      *
    * called by the owner of the CallTracker    *
    * instance to have it process an event      *
    * significant to the CallTracker.  These    *
    * notifications serve as inputs to the      *
    * CallTracker's state machines              *
    *********************************************/

   /*! Method to be called to notify the CallTracker that a request
    *  belonging to the call being tracked has arrived.
    *  This method must be called by the owner for
    *  *each and every* request of the tracked call.  The CallTracker
    *  will run the request through its state machines and adjust
    *  it for NAT traversal as required.
    *
    * \param [in] message - request to process
    * \param [in] address - ip address to which request will be sent
    * \param [in] port    - port to which request will be sent
    *
    * \return - true if request was processed properly.
    */
   bool handleRequest( SipMessage& message, const char* address, int port );

   /*! Method to be called to notify the CallTracker that a response
    *  belonging to the call being tracked by the CallTracker has
    *  arrived.  This method must be called by the owner for
    *  *each and every* response of the tracked call.  The CallTracker
    *  will run the response through its state machines and adjust
    *  it for NAT traversal as required.
    *
    * \param [in] message - response to process
    * \param [in] address - ip address to which response will be sent
    * \param [in] port    - port to which response will be sent
    */
   void handleResponse( SipMessage& message, const char* address, int port );

   /*! Method to notify that CallTracker that a cleanup timer has just ticked.
    *  The CallTracker will clean up stale resources if they have not
    *  been utilized for a while.
    *
    * \return - true when the CallTracker has finished its job and can be deleted.
    */
   bool handleCleanUpTimerTick( void );

   // ///////
   // Misc //
   // ///////

   /*! Method used to get the handle to a SessionContext based on the
    *  supplied route state.
    *
    * \param [in] routeState - route state to use to derive SessionContext handle
    * \param [out] handle - string that will receive handle if found
    *
    * \return - true if a handle was found
    */
   bool getSessionContextHandle( const RouteState& routeState, UtlString& handle ) const;

   // /////////////////////// //
   // UtlContainable virtuals //
   // /////////////////////// //

   UtlContainableType getContainableType( void ) const;
   unsigned hash() const;
   int compareTo(UtlContainable const *rhsContainable ) const;
   static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

   // ////////////////////////////////////////////// //
   // CallTrackerInterfaceForSessionContext virtuals //
   // ////////////////////////////////////////////// //

   //! note: see class CallTrackerInterfaceForSessionContext for details
   virtual void reportSessionContextReadyForDeletion( const UtlString& sessionContextHandle );


protected:
private:
   // ////////////////////////// //
   // Session context operations //
   // ////////////////////////// //

   /*! Generates a process-wide unique SessionContext handle
    * \param [out] handle - generated handle
    * \return - true if handle was generated successfully; otherwise false
    */
   bool generateSessionContextHandleFromRequest( UtlString& handle );

   /*! Retrieves the SessionContext object, provided its handle
    *  \param [in] sessionContextHandle - handle of SessionContext object to retrieve
    *  \return - SessionContext pointer if found, otherwise 0
    */
   SessionContext* getSessionContextFromHandle( const UtlString& sessionContextHandle ) const;

   /*! Instantiates a new SessionContext handle to track a new call fork and encode its
    *  handle in the provided routeState.
    *  \param [in] request - request that mandates the creation of the SessionContext
    *  \param [in] routeState - route state in which to encode the SessionContext handle
    *  \param [in] bTrackBranchId - indicates whether the branch Id of the request should be tracked -see not
    *  \param [out] sessionContextHandle - handle given to newly created SessionContext
    *  \return - SessionContext pointer if created successfully, otherwise 0
    *
    * Note: Normally, the CallTracker dispatches an incoming response
    * to the correct SessionContext handle by extracting the SessionContext handle it
    * carries in its topmost Via.  Unfortunately, in some cases where the sipXproxy
    * forks the dialog-forming INVITE and waits for the best response and that 'best
    * response' ends up being a final failure response, sipXproxy strips the
    * SessionContext handle contained in the Via which leaves CallTracker
    * without the information it needs to dispatch it to the correct
    * SessionContext.  One such scenario is as follows:  UserA calls
    * UserB.  UserA hangs up before UserB had a chance to answer.  The final
    * failure response sent to UserA to terminate the INVITE transaction will
    * not carry the SessionContext handle in the Via.  The root cause of this
    * behavior is still unknown but is suspected to be a bug inside SipTransaction.
    * To get around the problem, the CallTracker maintains a list that maps branch Id of
    * dialog-forming requests to Session Context handles that it will fallback to
    * whenever it cannot find SessionContext handles in the topmost Via.  The mapping
    * of branch Ids to SessionContext handles is governed by the bTrackBranchId flag.
    *
    */
   SessionContext* createSessionContextAndSetHandle( const SipMessage& request,
                                                     RouteState& routeState,
                                                     bool bTrackBranchId,
                                                     UtlString& sessionContextHandle );

   // /////////////////////////////// //
   // Operations on RouteState object //
   // /////////////////////////////// //

   /*! Encodes SessionContext handle in the provided RouteState object
    *  \param [in] routeState - RouteState object in which to encode the handle
    *  \param [in] handle - handle to encode in RouteState
    *  \return - true if operation succeeded
    */
   bool setSessionContextHandle( RouteState& routeState, const UtlString& handle ) const;

   /*! Removes an encoded SessionContext handle from a route state.  This method
    *  essentially undoes setSessionContextHandle().
    *  \param [in] routeState - RouteState object from which to remove handle
    *  \return - true if operation succeeded
    */
   bool unsetSessionContextHandle( RouteState& routeState ) const;

   // //////////////////////// //
   // Via manipulation methods //
   // //////////////////////// //

   /*! Adds an 'id' tag to the specified Via subfield containing the sessionHandle as a value
    *  \param [in] message - message that contains Via to add 'id' tag to.
    *  \param [in] viaIndex - index of Via subfield that will receive tag
    *  \param [in] sessionHandle- handle to use as value for 'id' tag.
    *  \return - true if tag was added; false otherwise.
    */
   bool setSessionHandleInVia( SipMessage& message, int viaIndex, const UtlString& sessionHandle ) const;

   /*! Gets the 'id' tag containing a sessionHandle from the specified Via subfield
    *  \param [in] message - message that contains Via
    *  \param [in] viaIndex - index of Via subfield from which 'id' tag will be taken
    *  \param [out] sessionHandle - will be set to handle value contained in Via's 'id' tag
    *  \return - true if sessionHandle was found; false otherwise.
    */
   bool getSessionHandleFromVia( const SipMessage& message, int viaIndex, UtlString& sessionHandle ) const;

   /*! Gets the SessionContext instance mapped to the sessionHandle contained in the specified
    *  Via's 'id' tag.
    *  \param [in] message - message that contains Via
    *  \param [in] viaIndex - index of Via subfield from which 'id' tag willbe taken
    *  \return - NULL if SessionContext instance could not be determined; pointer to instance otherwise.
    */
   SessionContext* getSessionContextFromVia( const SipMessage& message, int viaIndex ) const;

   /*! Removes the first encountered 'id' via tag conataining the specified session context handle.
    *  VIa's 'id' tag.
    *  \param [in] message - message that contains Vias
    *  \param [in] sessionHandleToRemove - as name says
    *  \return - true if Session Context handle could be removed.
    */
   bool removeSessionHandleFromVias( SipMessage& message, const UtlString& sessionHandleToRemove ) const;

   // //////////////////////// //
   // branchId-related methods //
   // //////////////////////// //

   /*! Extracts the branch Id from a Via
    *  \param [in] message - message that contains Via from which to extract branch Id
    *  \param [in] index - index of Via to extract branch Id from (0 means top Via)
    *  \param [out] branchId - string that will receive the branch Id if found
    *  \return - true if branch Id was found; false otherwise.
    */
   bool getBranchId( const SipMessage& message, int viaIndex, UtlString& branchId ) const;

   // ///////////////////////////////////////////// //
   // Session Contexts lifetime management routines //
   // ///////////////////////////////////////////// //

   /*! Enforces the deletion of the SessionContext objects whose handles are found
    *  in mListOfSessionContextsReadyForDeletion.
    */
   void deleteSessionContextsReadyForDeletion( void );

   /// CallTracker handle
   ssize_t                  mHandle;
   /// pointer to the NAT Traversal Rules object which holds the NAT traversal feature configuration information
   const NatTraversalRules* mpNatTraversalRules;
   /// Pointer to object that abstracts the Media Relay
   MediaRelay*              mpMediaRelayToUse;
   /// Pointer to registration DB used in some instances to resolve the location of certain endpoints
   const RegistrationDB*    mpRegistrationDB;
   /// Map that holds the SessionContext objects allocated by the CallTracker.  Map is indexed by SessionContextHandles
   UtlHashMap               mSessionContextsMap;
   /// Map that maps BranchIds to the handle of the SessionContext that can handle the request or response that bears the branchId
   UtlHashMap               mBranchIdToSessionHandleMap;
   /// Map that holds original SDP bodies before they got modified by the NAT traveral feature.  The map
   /// is indexed by SessionContext handles.
   UtlHashMap               mSdpLibrary;
   /// Auth Plugin instance name to use when encoding parameters in a RouteStata
   UtlString                mInstanceNameForRouteState;
   /// Counts the number of times we received a call to handleCleanUpTimerTick()
   /// while not having any allocated SessionContexts
   ssize_t                  mNumberOfTicksWithoutActiveDialogs;
   /// Static used to generate process-wide session context handles.
   static ssize_t           sNextAvailableSessionContextHandle;
   /// Pointer to NatMaintainer instance.  Will be used to report calls established by Remote NATed
   /// endpoints to ensure that pinholes to them are properly maintained.
   NatMaintainer*           mpNatMaintainer;
   /// remembers the public IP address and port number that we have asked the NatMaintainer to maintain open.
   /// Set to NULL if the caller's pinhole is not being maintained.
   TransportData*           mpCallerPinholeInformation;
   /// Id of currently running process
   PID                      mPid;
   // Copy of SDP contained in the dialog-forming INVITE.  This copy is used in scenarios that
   // require the SDP to be restored to its original value prior to processing.
   SdpBody*                 mpSavedOriginalSdpOfferCopy;
   // List of handle of SessionContexts that are ready to be deleted
   vector<UtlString>        mListOfSessionContextsReadyForDeletion;

   friend class CallTrackerTest;
   friend class NatTraversalAgentTest;
};

#endif // _CALLTRACKER_H_

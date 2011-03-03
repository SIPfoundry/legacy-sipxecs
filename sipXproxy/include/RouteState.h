// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _ROUTESTATE_H_
#define _ROUTESTATE_H_

// SYSTEM INCLUDES
#include <vector>

// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "utl/UtlSortedList.h"
#include "net/SipMessage.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class RouteStateTest;

/// Manipulate sipXauthproxy state information in Record-Route and Route headers.
/**
 * The sipXauthproxy uses this class to encode state information to be carried by
 * the request messages in a dialog.  Normally, this class is used only by an
 * AuthPlugin object in its implementation of AuthPlugin::authorizeAndModify.
 *
 * State is encoded as name/value pairs.
 * The name is composed of two parts, each of which may consist only of
 * A-Z, a-z, 0-9, underscore (_), and dash (-).:
 * - instanceName: The name of the AuthPlugin.  This name is a configuration
 *                 value: @see AuthPlugin::AuthPlugin).
 *                 Some parameters are designed to be shared among plugins;
 *                 these should be implemented as methods in RouteState, and
 *                 should pass the null string for instanceName (for an example,
 *                 see RouteState::directionIsCallerToCalled).
 * - parameterName: The name for this parameter chosen by the caller.
 * 
 * Obeying this naming convention ensures that an AuthPlugin will not
 * unintentionally access or modify state for some other AuthPlugin.  If
 * the AuthPlugin uses only one parameter, it may pass an empty string for
 * the parameterName.
 *
 * The state in a message is 'signed' by a hash value that incorporates a
 * secret (see setSecret); this signature binds the message to the call-id
 * and from-tag values from the dialog-forming request.  A state value
 * whose hash value does not match is logged, but is otherwise equivalent
 * to a message for which no state was received (no parameter names or
 * values are available).
 * 
 * State can only be created (encoded) in the initial dialog-forming
 * request message; in subsequent in-dialog requests and in responses, the
 * state can be decoded and read, but cannot be modified.  This is because
 * the state is encoded into a route inserted into a Record-Route: the
 * route set for the dialog is established by the Record-Route header(s) in
 * the request/response messages that create the dialog.  Subsequent
 * messages in the dialog carry those values in Route headers, but they
 * cannot be changed.  To determine whether state can be changed, call
 * isMutable.
 *
 * @Note  The SIP protocol allows a proxy to modify the Record-Route headers in the
 *        dialog-forming response so that the two ends of the dialog see different route sets.
 *        However, the current sipXtackLib design proxies responses before sending them
 *        to the application (sipXauthproxy) layer, so we must use symetric state.
 *        (There are some protocol problems with modifying responses anyway, since in the
 *        case of SUBSCRIBE or other requests that can create multiple dialogs, the
 *        requestor may not see all responses, but because of the stack limitation this
 *        is moot for us)
 *
 * To access the state in a message, construct a RouteState object, passing
 * the message and any Route headers removed by SipMessage::normalizeProxyRoutes to
 * the constructor.  SipRouter::proxyMessage uses the update method to put the updated
 * state back into the message.
 *
 * @Note  A dialog-forming request that passes through the same sipXauthproxy more than
 *        once (spirals) will not get multiple Record-Route headers.  The first spiral to
 *        call the 'update' method adds a Record-Route; subsequent calls modify the state
 *        information recorded in that existing header so that in-dialog requests only
 *        traverse this proxy once.
 *
 * @nosubgrouping
 */
class RouteState
{
  public:

   // ================================================================
   /** @name                  Decoding Operations
    *
    * These methods access the state in a message. 
    */
   ///@{

   /// Decode the route state for a received message.
   RouteState(const SipMessage& message,      ///< normalized incoming message
              const UtlSList&   removedRoutes,///< removed routes returned by normalizeProxyRoutes
              const UtlString   routeName     /**< The SIP address to use in the added route.
                                                *  It may include only:
                                                *  - scheme (optional, defaults to 'sip'),
                                                *  - host (required, may be name or address),
                                                *  - port (optional, defaults to unspecified)
                                                *  These values need not match those received in a
                                                *  spiraled request; if they do not, the new values
                                                *  are used.
                                                */
              );
   /**<
    * This method must be called after the received request has
    * been normalized by normalizeProxyRoutes.  
    */


   /// Extract value of a parameter saved in the route state.
   bool getParameter(const char* pluginInstance,
                     const char* parameterName, 
                     UtlString&  parameterValue  ///< output
                     ) const;
   /**<
    * @returns true if the parameter was found in the state, false if not.
    *
    * @Note  A parameter may be present but have a null (zero length) value;
    *        in this case, this returns true and sets parameterValue to the
    *        empty string.
    */

   /// Used to discover the request direction.
   bool directionIsCallerToCalled(const char* instanceName ///< used for logging - must not be null
                                  );

   /// Returns the from tag value of the dialog forming request
   bool originalCallerFromTagValue( const char* instanceName ///< used for logging - must not be null
                                   ,UtlString& fromTagValue
                                   );
   ///< Do not use as a substitute for directionIsCallerToCalled
   
   /// Adds an encoded parameter to a mutable RouteState indicating that the dialog is authorized.
   void markDialogAsAuthorized( void );
   /**<
    * Authorizating entities can test for the presence of such a parameter using the
    * RouteState::isDialogAuthorized() method when processing requests.
    */
   
   /// used to discover whether or not the dialog has already been authorized.
   bool isDialogAuthorized( void ); 
  
   /**<
    * Some AuthPlugin classes need to make different changes
    * to a request depending on whether it was sent from the
    * original caller or the called party.  For example, anything
    * that makes changes to the To or From headers needs to
    * know which is which because they will be swapped for an
    * in-dialog request sent by the called party.  This method
    * provides a uniform way to discover this.
    *
    * This caches the From tag from the original caller in
    * a shared parameter in the RouteState (a shared parameter
    * passes null string for the instanceName) so that once
    * it is computed by any plugin it is accessible to all.
    *
    * @returns true iff the From header is the original caller.
    */

   ///@}

   // ================================================================
   /** @name                  Encoding Operations
    *
    * These methods manipulate the state encoded into a dialog-forming request.
    */
   ///@{

   /// Is it possible to record state in this request?
   bool isMutable()
   {
   /**<
    * @returns false if this appears to be an in-dialog request.
    *
    * Since Record-Route headers in an in-dialog request are ignored by the UAs,
    * there is no point in adding them.  This attempts to detect this.
    */
      return mMayBeMutable;
   }
   
   bool isFound()
   {
   /**<
    * @returns true if the route set contains RouteState information, 
    * otherwise returns false.
    */
      return mFoundRouteState;
   }
   
   /// Stores the value of a parameter in the route state.
   void setParameter(const char*       pluginInstance,
                     const char*       parameterName,  
                     const UtlString&  parameterValue  
                     );
   /**<
    * Establish a new value for the named parameter to be included
    * when the updated route state is generated.
    *
    * @Note  A parameter may be set to the null (zero length) value.
    */

   /// Removes any value for a parameter from the route state.
   void unsetParameter(const char* pluginInstance,
                       const char* parameterName
                       );
   /**<
    * This removes the named parameter from those included when
    * the updated route state is recorded.
    */

   /** 
    * The addCopy method is used to add another Record-Route w/
    * a RouteState to a request that already has one.  The logic
    * here ensures that all copies of the RouteState contain
    * all the same parameters and that they remain synchronized.
    */
   void addCopy( void );
   
   /// Add or update the state in the Record-Route header.
   void update(SipMessage* request         ///< message to add state too, if state was modified
               );
   /**<
    * This has no effect other than logging an error if isMutable would return false.
    */

   /// Initialize the secret value used to sign RouteState information.
   static void setSecret(const char* secret /**< a null terminated string used as input to sign the
                                             *   state value.  This should be chosen such that it:
                                             * - is hard for an attacker to guess (includes at
                                             *   least 32 bits of cryptographicaly random data)
                                             * - ideally, is the same in replicated authproxies
                                             *   (this is not important yet, since we don't use
                                             *   SRV names in the Record-Route targets).
                                             */
                         );
   /**<
    * This must be called once at initialization time,
    * before any RouteState objects are created.
    *
    * It may be called after that, but doing so with a
    * new value will invalidate any outstanding route
    * state.
    */
   ///@}
   
   /// destructor
   ~RouteState();

  protected:

   friend class RouteStateTest; // to allow unit tests for protected members
   
   /// Encode and sign the state as a single SIP token
   void encode(UtlString& stateToken);

   /// Check the signature and parse the name/value pairs from a state token
   bool decode(const UtlString& stateToken);
   /**< @returns true iff the token was correctly signed and successfully parsed */
   
  private:
   static const char* UrlParameterName;
   
   UtlString  mRouteHostPort; /**< hostport value to use when adding Record-Route headers
                              */
   bool       mMayBeMutable; /**<
                              * true iff
                              * - There was no 'to' tag on the request, and
                              * - Any state in the request was in a Route header
                              *   (not a Record-Route header).
                              */
   std::vector<size_t> mRecordRouteIndices; /**<
                                             * Used only if mMayBeMutable == true
                                             * This is a list of  Record-Route header indices that 
                                             * contain a hostport matching the mRouteHostPort.  These
                                             * Record-Route headers may or may not contain
                                             * Route State parameters.
                                             */
   bool       mModified; /**<
                          * Used only if mMayBeMutable == true
                          * Tracks whether or not any changes have been made to the state,
                          * so that we know whether or not to rewrite it in the update method.
                          */
   
   bool       mFoundRouteState; /**<
                                 * Indicates if a RouteState has been found in the route set supplied
                                 * at construction time.
                                 */
   
   bool       mAddCopyRequested;
   
   UtlSortedList mValues; ///< contains NameValuePair objects

   UtlString  mCallId;
   UtlString  mFromTag;

   static UtlString mSignatureSecret;
   
   // @cond INCLUDENOCOPY
   // There is no copy constructor.
   RouteState(const RouteState& nocopyconstructor);

   // There is no assignment operator.
   RouteState& operator=(const RouteState& noassignmentoperator);
   // @endcond     
};

#endif // _ROUTESTATE_H_

// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>


// APPLICATION INCLUDES
#include "os/OsFS.h"
#include "os/OsConfigDb.h"
#include "os/OsSysLog.h"
#include "os/OsEventMsg.h"
#include "utl/UtlRandom.h"
#include "net/SignedUrl.h"
#include "net/SipUserAgent.h"
#include "net/SipXauthIdentity.h"
#include "sipdb/ResultSet.h"
#include "sipdb/CredentialDB.h"
#include "AuthPlugin.h"
#include "SipRouter.h"
#include "ForwardRules.h"
#include "sipXecsService/SipXecsService.h"
#include "sipXecsService/SharedSecret.h"
// DEFINES
//#define TEST_PRINT 1

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const char* AuthPlugin::Factory = "getAuthPlugin";
const char* AuthPlugin::Prefix  = "SIPX_PROXY";
// The period of time in seconds that nonces are valid, in seconds.
#define NONCE_EXPIRATION_PERIOD             (60 * 5)     // five minutes
#define SIPX_SPIRAL_HEADER                  ("X-SipX-Spiral")

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipRouter::SipRouter(SipUserAgent& sipUserAgent,
               ForwardRules& forwardingRules,
               OsConfigDb&   configDb
               )
   :OsServerTask("SipRouter-%d", NULL, 2000)
   ,mpSipUserAgent(&sipUserAgent)
   ,mAuthenticationEnabled(true)    
   ,mNonceExpiration(NONCE_EXPIRATION_PERIOD) // the period in seconds that nonces are valid
   ,mpForwardingRules(&forwardingRules)
   ,mAuthPlugins(AuthPlugin::Factory, AuthPlugin::Prefix)
{
   // Get Via info to use as defaults for route & realm
   UtlString dnsName;
   int       port;
   mpSipUserAgent->getViaInfo(OsSocket::UDP, dnsName, port);
   Url defaultUri;
   defaultUri.setHostAddress(dnsName.data());
   defaultUri.setHostPort(port);

   readConfig(configDb, defaultUri);
   
   
   // Get the secret to be used in the route recognition hash.
   // read the domain configuration
   OsConfigDb domainConfiguration;
   domainConfiguration.loadFromFile(SipXecsService::domainConfigPath());

   // get the shared secret for generating signatures
   mSharedSecret = new SharedSecret(domainConfiguration);   
   RouteState::setSecret(mSharedSecret->data());
   SipXauthIdentity::setSecret(mSharedSecret->data());
   SignedUrl::setSecret(mSharedSecret->data());

   // Register to get incoming requests
   OsMsgQ* queue = getMessageQueue();
   mpSipUserAgent->addMessageObserver(*queue,
                                      "",      // All methods
                                      TRUE,    // Requests,
                                      FALSE,   // Responses,
                                      TRUE,    // Incoming,
                                      FALSE,   // OutGoing,
                                      "",      // eventName,
                                      NULL,    // SipSession* pSession,
                                      NULL     // observerData
                                      );
}

void SipRouter::readConfig(OsConfigDb& configDb, const Url& defaultUri)
{
   UtlString authScheme;
   configDb.get("SIPX_PROXY_AUTHENTICATE_SCHEME", authScheme);
   if(authScheme.compareTo("none", UtlString::ignoreCase) == 0)
   {
      mAuthenticationEnabled = false;
      OsSysLog::add(FAC_SIP, PRI_WARNING,
                    "SIPX_PROXY_AUTHENTICATE_SCHEME : NONE\n"
                    "  Authentication is disabled: there is NO permissions enforcement"
                    );
   }
   else
   {
      UtlString algorithm;
      if (OS_SUCCESS != configDb.get("SIPX_PROXY_AUTHENTICATE_ALGORITHM", algorithm))
      {
         OsSysLog::add(FAC_SIP, PRI_INFO,
                       "SipRouter::readConfig "
                       "SIPX_PROXY_AUTHENTICATE_ALGORITHM not configured: using MD5"
                       );
         algorithm = "MD5";
      }

      if(algorithm.compareTo("MD5", UtlString::ignoreCase) == 0)
      {
         OsSysLog::add(FAC_SIP, PRI_INFO,
                       "SipRouter::readConfig "
                       "SIPX_PROXY_AUTHENTICATE_ALGORITHM : %s",
                       algorithm.data());
      }
      else if (algorithm.isNull())
      {
         OsSysLog::add(FAC_SIP, PRI_INFO,
                       "SipRouter::readConfig "
                       "SIPX_PROXY_AUTHENTICATE_ALGORITHM not set: using MD5"
                       );
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_WARNING,
                       "SipRouter::readConfig "
                       "Unknown authentication algorithm:\n"
                       "SIPX_PROXY_AUTHENTICATE_ALGORITHM : %s\n"
                       "   using MD5",
                       algorithm.data());
      }
   
      configDb.get("SIPX_PROXY_AUTHENTICATE_REALM", mRealm);
      if(mRealm.isNull())
      {
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "SipRouter::readConfig "
                       "SIPX_PROXY_AUTHENTICATE_REALM not specified\n"
                       "   Phones must be configured with the correct default to authenticate."
                       );
         defaultUri.toString(mRealm);
      }
      OsSysLog::add(FAC_SIP, PRI_NOTICE,
                    "SipRouter::readConfig "
                    "SIPX_PROXY_AUTHENTICATE_REALM : %s", mRealm.data());
   }
   
   configDb.get("SIPX_PROXY_HOSTPORT", mRouteHostPort);
   if(mRouteHostPort.isNull())
   {
      OsSysLog::add(FAC_SIP, PRI_WARNING,
                    "SipRouter::readConfig "
                    "SIPX_PROXY_HOSTPORT not specified\n"
                    "   This may cause some peers to make a non-optimal routing decision."
                    );
      defaultUri.toString(mRouteHostPort);
   }
      
   // this should really be redundant with the existing aliases,
   // but it's better to be safe and add it to ensure that it's
   // properly recognized (the alias db prunes duplicates anyway)
   mpSipUserAgent->setHostAliases(mRouteHostPort);

   OsSysLog::add(FAC_SIP, PRI_INFO,
                 "SipRouter::readConfig "
                 "SIPX_PROXY_HOSTPORT : %s", mRouteHostPort.data());

   configDb.get("SIPX_PROXY_DOMAIN_NAME", mDomainName);
   if (mDomainName.isNull())
   {
      OsSysLog::add(FAC_SIP, PRI_ERR, "SipRouter::readConfig "
                    "SIPX_PROXY_DOMAIN_NAME not configured.");
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_INFO, "SipRouter::readConfig "
                    "SIPX_PROXY_DOMAIN_NAME : %s", mDomainName.data());
   }
    
   // Load and configure all authorization plugins
   mAuthPlugins.readConfig(configDb);
}

// Destructor
SipRouter::~SipRouter()
{
   // Remove the message listener from the SipUserAgent, if there is one.
   if (mpSipUserAgent)
   {
      mpSipUserAgent->removeMessageObserver(*getMessageQueue());
   }
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean
SipRouter::handleMessage( OsMsg& eventMessage )
{
   int msgType = eventMessage.getMsgType();

   // Timer event
   if ( msgType == OsMsg::PHONE_APP )
   {
      SipMessageEvent* sipMsgEvent = dynamic_cast<SipMessageEvent*>(&eventMessage);

      int messageType = sipMsgEvent->getMessageStatus();
      if ( messageType == SipMessageEvent::TRANSPORT_ERROR )
      {
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "SipRouter::handleMessage received transport error message") ;
      }
      else
      {
         SipMessage* sipRequest = const_cast<SipMessage*>(sipMsgEvent->getMessage());
         if(sipRequest)
         {
             if ( sipRequest->isResponse() )
             {
                OsSysLog::add(FAC_AUTH, PRI_CRIT, "SipRouter::handleMessage received response");
                /*
                 * Responses have already been proxied by the stack,
                 * so we don't need to do anything with them.
                 */
             }
             else
             {
                SipMessage sipResponse;
                switch (proxyMessage(*sipRequest, sipResponse))
                {
                case SendRequest:
                   // sipRequest may have been rewritten entirely by proxyMessage().
                   // clear timestamps, protocol, and port information
                   // so send will recalculate it
                   sipRequest->resetTransport();
                   mpSipUserAgent->send(*sipRequest);
                   break;

                case SendResponse:
                   sipResponse.resetTransport();
                   mpSipUserAgent->send(sipResponse);
                   break;

                case DoNothing:
                   // this message is just ignored
                   break;

                default:
                   OsSysLog::add(FAC_SIP, PRI_CRIT,
                                 "SipRouter::proxyMessage returned invalid action");
                   assert(false);
                }
             }
         }
         else 
         {
            // not a SIP message - should never happen
            OsSysLog::add(FAC_SIP, PRI_CRIT,
                          "SipRouter::handleMessage is not a sip message");
         }
      }
   }    // end PHONE_APP
   return(TRUE);
}

SipRouter::ProxyAction SipRouter::proxyMessage(SipMessage& sipRequest, SipMessage& sipResponse)
{
   ProxyAction returnedAction = SendRequest;

   bool bRequestShouldBeAuthorized         = true;
   bool bForwardingRulesShouldBeEvaluated  = true;
   bool bRequestHasProprietarySpiralHeader = true;

   /*
    * Check for a Proxy-Require header containing unsupported extensions
    */
   UtlString disallowedExtensions;      
   if( areAllExtensionsSupported(sipRequest, disallowedExtensions) )
   {
      // no unsupported extensions, so continue...
      // fix strict routes and remove any top route headers that go to myself      
      Url normalizedRequestUri;
      UtlSList removedRoutes;
      sipRequest.normalizeProxyRoutes(mpSipUserAgent,
                                      normalizedRequestUri, ///< returns normalized request uri
                                      &removedRoutes        // route headers popped 
                                      );
      
      // get any state from the record-route and route headers
      RouteState routeState(sipRequest, removedRoutes, mRouteHostPort); 
      removedRoutes.destroyAll(); // done with routes - discard them.
      
      if( !sipRequest.getHeaderValue( 0, SIPX_SPIRAL_HEADER ))
      {
         // Our custom spiraling header was NOT found indicating that the request
         // is not received as a result of spiraling. It could either be a 
         // dialog-forming request or an in-dialog request sent directly by the UAC
         if( !routeState.isFound() )
         {
            if(isPAIdentityApplicable(sipRequest))
            {
               Url fromUrl;
               UtlString userId;
               UtlString authTypeDB;
               UtlString passTokenDB;

               sipRequest.getFromUrl(fromUrl); 
          
               bRequestShouldBeAuthorized = false;

               // If the identity portion of the From header can be found in the
               // identity column of the credentials database, then a request
               // should be challenged for authentication and when authenticated
               // the PAI should be added by the proxy before passing it on to
               // other components.
               if(CredentialDB::getInstance()->getCredential(fromUrl,
                                                             mRealm,
                                                             userId,
                                                             passTokenDB,
                                                             authTypeDB))
               {
                  UtlString authUser;
                  if (!isAuthenticated(sipRequest,authUser))
                  {
                     // challenge the originator
                     authenticationChallenge(sipRequest, sipResponse);

                     OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipRouter::proxyMessage "
                                   " From '%s' is unauthenticated local user - challenge for PAI",
                                   fromUrl.toString().data());

                     returnedAction = SendResponse;
                     bRequestShouldBeAuthorized = false;
                     bForwardingRulesShouldBeEvaluated = false;
                  } 
                  else
                  {
                     // already authenticated
                     sipRequest.setHeaderValue(SIPX_SPIRAL_HEADER, "true", 0 );
                     bRequestShouldBeAuthorized        = false;
                     bForwardingRulesShouldBeEvaluated = true;

                     SipXauthIdentity pAssertedIdentity;
                     UtlString fromIdentity;
                     fromUrl.getIdentity(fromIdentity);
                     pAssertedIdentity.setIdentity(fromIdentity);
                     // Sign P-Asserted-Identity header  to prevent from forgery 
                     // and insert it into sipMessage
                     pAssertedIdentity.insert(sipRequest,
                                              SipXauthIdentity::PAssertedIdentityHeaderName);
                  }
               }
               else
               {
                  OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipRouter::proxyMessage "
                                " From '%s' not local to realm '%s' - do not challenge",
                                fromUrl.toString().data(), mRealm.data());
               }
            }
            else
            {
               // the request is not spiraling and does not bear a RouteState.  
               // Do not authorize the request right away.  Evaluate the 
               // Forwarding Rules and let the request spiral.   The request
               // will eventually get authorized as it spirals back to us.
               // Add proprietary header indicating that the request is 
               // spiraling.
               sipRequest.setHeaderValue(SIPX_SPIRAL_HEADER, "true", 0 );
               bForwardingRulesShouldBeEvaluated = true;
            }

            // the request is not spiraling and does not bear a RouteState.  
            // Do not authorize the request right away.  Evaluate the 
            // Forwarding Rules and let the request spiral.   The request
            // will eventually get authorized as it spirals back to us.
            // Add proprietary header indicating that the request is 
            // spiraling. 
            sipRequest.setHeaderValue( SIPX_SPIRAL_HEADER, "true", 0 );
            bRequestShouldBeAuthorized        = false;
            bForwardingRulesShouldBeEvaluated = true;

            // If the UA sending this request is located behind 
            // a NAT and the request is a REGISTER then add a
            // Path header to this proxy to make sure that all subsequent 
            // requests sent to the registering UA get funneled through
            // this proxy.
            addPathHeaderIfNATRegisterRequest( sipRequest );
         }
         else
         {
            // the request is not spiraling but it has a RouteState.
            // If the RouteState is not mutable, it indicates that
            // this request is an in-dialog one.  There is no need to
            // evaluate the Forwarding Rules on such requests unless the
            // final target that has been identified is in our own domain.
            // If the RouteState is mutable, this indicates that we are
            // still in an early dialog.  Such a condition can occur
            // when a UAS generates 302 Moved Temporarily in response
            // to an INVITE that we forked.  The processing of that 302 Moved
            // Temporarily generates an INVITE that carries a RecordRoute header
            // with a valid RouteState.  Such requests must be authenticated
            // and then spiraled to make sure they get forked according to the
            // forwarding rules. In such cases, our custom spiraling header
            // is added to make sure that that happens.
            if( routeState.isMutable() )
            {
               sipRequest.setHeaderValue( SIPX_SPIRAL_HEADER, "true", 0 );
               bRequestHasProprietarySpiralHeader = true;
               bRequestShouldBeAuthorized         = true;
               bForwardingRulesShouldBeEvaluated  = false;
            }
            else if (isLocalDomain(normalizedRequestUri))
            {
               // final target is in our own domain, need to check forwarding rules
                OsSysLog::add(FAC_SIP, PRI_DEBUG,
                         "SipRouter::proxyMessage domain is us");
               bRequestHasProprietarySpiralHeader = false;
               bRequestShouldBeAuthorized         = true;
               bForwardingRulesShouldBeEvaluated  = true;
            }
            else
            {
               bRequestHasProprietarySpiralHeader = false;
               bRequestShouldBeAuthorized         = true;
               bForwardingRulesShouldBeEvaluated  = false;
            }
         }
      }
      else
      {
         // Request is currently spiraling.  Continue to evaluate Forwarding Rules
         // to converge on the final target and do authorize the request to
         // make sure request is allowed to spiral further.
         bRequestShouldBeAuthorized        = true;
         bForwardingRulesShouldBeEvaluated = true;
      }

      if( bForwardingRulesShouldBeEvaluated )
      {
         bool bMatchingForwardingRuleFound = false;
         
         UtlString topRouteValue;
         if (sipRequest.getRouteUri(0, &topRouteValue)) 
         {
            /*
             * There is a top route that is not to this domain
             * (if the top route were to this domain, it would have been removed),
             * so let the authorization process decide whether or not it can go through
             */
            bRequestShouldBeAuthorized = true;
         }
         else // there is no Route header, so evaluate forwarding rules 
              // based on request's Request URI
         {
            UtlString mappedTo;
            UtlString routeType;               
            bool authRequired;
                        
            // see if we have a mapping for the normalized request uri
            if (   mpForwardingRules 
                && (mpForwardingRules->getRoute(normalizedRequestUri, sipRequest,
                                                mappedTo, routeType, authRequired)==OS_SUCCESS)
                )
            {
               bMatchingForwardingRuleFound = true;
               if (mappedTo.length() > 0)
               {
                  // Yes, so add a loose route to the mapped server
                  Url nextHopUrl(mappedTo);

                  // Check if the route points to the Registrar by
                  // testing for the preseonce of the
                  // 'x-sipx-routetoreg' custom URL parameter.  If the
                  // parameter is found, it indicates that the request
                  // is spiraling.
                  UtlString dummyString;
                  if( nextHopUrl.getUrlParameter( ROUTE_TO_REGISTRAR_MARKER_URI_PARAM, dummyString ) )
                  {
                     // bMessageIsSpiraling = true;
                     nextHopUrl.removeUrlParameter( ROUTE_TO_REGISTRAR_MARKER_URI_PARAM );
                  }
                  
                  nextHopUrl.setUrlParameter("lr", NULL);
                  UtlString routeString;
                  nextHopUrl.toString(routeString);
                  sipRequest.addRouteUri(routeString.data());
         
                  OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "SipRouter::proxyMessage fowardingrules added route type '%s' to: '%s'",
                             routeType.data(), routeString.data());
               }
               if (authRequired)
               {
                  // Forwarding rules specify that request should be authorized
                  bRequestShouldBeAuthorized = true;
               }
            }
            else
            {
               // the mapping rules didn't have any route for this,
               // so let the authorization process decide whether or not it can go through
               bRequestShouldBeAuthorized = true;
            }
         } 
         if(    !bMatchingForwardingRuleFound
             && bRequestHasProprietarySpiralHeader )
         {
            // No match found in forwarding rules meaning that spiraling is 
            // complete and that request will be sent towards its final destination. 
            // If the request contained our proprietary spiral header then remove it
            // since spiraling is complete.
            sipRequest.removeHeader( SIPX_SPIRAL_HEADER, 0 );
         }
      }
      
      if( bRequestShouldBeAuthorized )
      {
         UtlString callId;
         sipRequest.getCallIdField(&callId);  // for logging
         
         /*
          * Determine the authIdentity.
          */
         UtlString authUser;
         bool requestIsAuthenticated = false;
         
         // Use the identity found in the SipX-Auth-Identity header if found
         SipXauthIdentity sipxIdentity(sipRequest,SipXauthIdentity::AuthIdentityHeaderName,
             SipXauthIdentity::allowUnbound); 
         if ((requestIsAuthenticated = sipxIdentity.getIdentity(authUser)))
         {
            // found identity in request
            OsSysLog::add(FAC_AUTH, PRI_DEBUG, "SipRouter::proxyMessage "
                          " found valid sipXauthIdentity '%s' for callId %s",
                          authUser.data(), callId.data() 
                          );

            // Can't completely remove identity info, since it may be required
            // further if the request spirals. Normalize authIdentity to only leave
            // the most recent info in the request 
            SipXauthIdentity::normalize(sipRequest, SipXauthIdentity::AuthIdentityHeaderName);
         }
         else
         {
            // no SipX-Auth-Identity, so see if there is a Proxy-Authorization on the request
            requestIsAuthenticated = isAuthenticated(sipRequest, authUser);
         }

         /*
          * Determine whether or not this request is authorized.
          */
         AuthPlugin::AuthResult finalAuthResult;
         UtlString rejectReason;

         // handle special cases that are universal
         UtlString method;
         sipRequest.getRequestMethod(&method); // Don't authenticate ACK -- it is always allowed.
         if (  (method.compareTo(SIP_ACK_METHOD) == 0) 
             || sipRequest.isResponse())  // responses are always allowed (just in case)
         {
            finalAuthResult = AuthPlugin::ALLOW;
         }
         else
         {
            finalAuthResult = AuthPlugin::CONTINUE; // let the plugins decide
         }
         
         // call each plugin
         PluginIterator authPlugins(mAuthPlugins);
         AuthPlugin* authPlugin;
         UtlString authPluginName;
         AuthPlugin::AuthResult pluginResult;
         while ((authPlugin = dynamic_cast<AuthPlugin*>(authPlugins.next(&authPluginName))))
         {
            pluginResult = authPlugin->authorizeAndModify(this,
                                                          authUser,
                                                          normalizedRequestUri,
                                                          routeState,
                                                          method,
                                                          finalAuthResult,
                                                          sipRequest,
                                                          rejectReason
                                                          );

            OsSysLog::add(FAC_AUTH, PRI_DEBUG,
                          "SipProxy::proxyMessage plugin %s returned %s for %s",
                          authPluginName.data(),
                          AuthPlugin::AuthResultStr(pluginResult),
                          callId.data()
                          );

            // the first plugin to return something other than CONTINUE wins
            if (AuthPlugin::CONTINUE == finalAuthResult)
            {
               finalAuthResult = pluginResult;
               OsSysLog::add(FAC_AUTH, PRI_INFO,
                             "SipProxy::proxyMessage authorization %s by %s for %s",
                             AuthPlugin::AuthResultStr(finalAuthResult),
                             authPluginName.data(),
                             callId.data()
                             );
            }
         }

         // Based on the authorization decision, either proxy the request or send a response.
         switch (finalAuthResult)
         {
         case AuthPlugin::DENY:
         {
            // Either not authenticated or not authorized
            if (requestIsAuthenticated)
            {
               // Rewrite sipRequest as the authorization-needed response so our caller
               // can send it.
               sipResponse.setResponseData(&sipRequest,
                                           SIP_FORBIDDEN_CODE,
                                           rejectReason.data());
            }
            else
            {
               // There was no authentication, so challenge
               authenticationChallenge(sipRequest, sipResponse);
            }
            returnedAction = SendResponse;
         }
         break;
         
         case AuthPlugin::CONTINUE: // be permissive - if nothing says DENY, then treat as ALLOW
         case AuthPlugin::ALLOW:
         {
           // Request is sufficiently authorized, so proxy it.
           // Plugins may have modified the state - if allowed, put that state into the message
            if (routeState.isMutable())
            {
               routeState.update(&sipRequest);
            }
         }
         break;

         default:
            OsSysLog::add(FAC_SIP, PRI_CRIT,
                          "SipRouter::proxyMessage plugin returned invalid result %d",
                          finalAuthResult);
            break;
         }
      }     // end should be authorized
      else
      {
         // In order to guantantee symmetric signaling, this proxy has to 
         // Record-Route all incoming requests.  The RouteState mechanism
         // utilized by the authorization process does add a Record-Route
         // to the requests it evaluates.  We get into this branch of the
         // code when the authorization process is skipped, i.e. request
         // did not get Record-Routed by the authorization process.  In order  
         // to ensure that each and every request gets Record-Routed, we 
         // manually add a Record-Route to ourselves here.
         if( sipRequest.isRecordRouteAccepted() )
         {
            // Generate the Record-Route string to be used by proxy to Record-Route requests 
            // based on the route name
            UtlString recordRoute;
            Url route(mRouteHostPort);

            route.setUrlParameter("lr",NULL);
            route.toString(recordRoute);
            sipRequest.addRecordRouteUri(recordRoute);
         }
      }
   }        // end all extensions are supported
   else
   {
      // The request has a Proxy-Require that we don't support; return an error
      sipResponse.setRequestBadExtension(&sipRequest, disallowedExtensions.data());
      returnedAction = SendResponse;
   }
   
   switch ( returnedAction )
   {
   case SendRequest:
      // Decrement max forwards
      int maxForwards;
      if ( sipRequest.getMaxForwards(maxForwards) )
      {
         maxForwards--;
      }
      else
      {
         maxForwards = mpSipUserAgent->getMaxForwards();
      }
      sipRequest.setMaxForwards(maxForwards);
      break;

   case SendResponse:
      mpSipUserAgent->setServerHeader(sipResponse);
      break;

   case DoNothing:
   default:
      break;
   }
   
   return returnedAction;
}

// Get the canonical form of our SIP domain name
void SipRouter::getDomain(UtlString& canonicalDomain) const
{
   canonicalDomain = mDomainName;
}

// @returns true iff the authority in url is a valid form of the domain name for this proxy.
bool SipRouter::isLocalDomain(const Url& url ///< a url to be tested
                           ) const
{
   UtlString urlDomain;
   url.getHostAddress(urlDomain);

   return (   (0 == mDomainName.compareTo(urlDomain, UtlString::ignoreCase))
           || (mpSipUserAgent->isMyHostAlias(url))
           );
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
bool SipRouter::addPathHeaderIfNATRegisterRequest( SipMessage& sipRequest ) const 
{
   bool bMessageModified = false;
   UtlString method;

   sipRequest.getRequestMethod(&method);
   if( method.compareTo(SIP_REGISTER_METHOD) == 0 )
   {
      // Check if top via header has a 'received' parameter.  Presence of such
      // a header would indicate that the registering user is located behind
      // a NAT.
      UtlString  privateAddress, protocol;
      int        privatePort;
      UtlBoolean bReceivedSet;
      
      sipRequest.getTopVia( &privateAddress, &privatePort, &protocol, NULL, &bReceivedSet );
      if( bReceivedSet )
      {
         // Add Path header to the message
         Url pathUri( mRouteHostPort );
         SignedUrl::sign( pathUri );
         UtlString pathUriString;      
         pathUri.toString( pathUriString );
         sipRequest.addPathUri( pathUriString );
         bMessageModified = true;         
      }
   }
   return bMessageModified;
}

bool SipRouter::areAllExtensionsSupported( const SipMessage& sipRequest, 
                                           UtlString& disallowedExtensions ) const
{
   bool bAllExtensionsSupported = true;
    
   UtlString extension;
   for (int extensionIndex = 0;
        sipRequest.getProxyRequireExtension(extensionIndex, &extension);
        extensionIndex++
        )
   {
      if(!mpSipUserAgent->isExtensionAllowed(extension.data()))
      {
         bAllExtensionsSupported = false; 
         if(!disallowedExtensions.isNull())
         {
            disallowedExtensions.append(SIP_MULTIFIELD_SEPARATOR);
            disallowedExtensions.append(SIP_SINGLE_SPACE);
         }
         disallowedExtensions.append(extension.data());
      }
   }
   return bAllExtensionsSupported;
}

bool SipRouter::isAuthenticated(const SipMessage& sipRequest,
                             UtlString& authUser
                             )
{
   UtlBoolean authenticated = FALSE;
   UtlString requestUser;
   UtlString requestRealm;
   UtlString requestNonce;
   UtlString requestUri;
   int requestAuthIndex;
   UtlString callId;
   Url fromUrl;
   UtlString fromTag;
   OsTime time;
   OsDateTime::getCurTimeSinceBoot(time);
   long nonceExpires = mNonceExpiration;

   authUser.remove(0);
    
   sipRequest.getCallIdField(&callId);
   sipRequest.getFromUrl(fromUrl);
   fromUrl.getFieldParameter("tag", fromTag);

   // loop through all credentials in the request
   for ( ( authenticated = FALSE, requestAuthIndex = 0 );
         (   ! authenticated
          && sipRequest.getDigestAuthorizationData(&requestUser,
                                                   &requestRealm,
                                                   &requestNonce,
                                                   NULL,
                                                   NULL,
                                                   &requestUri,
                                                   HttpMessage::PROXY,
                                                   requestAuthIndex)
          );
         requestAuthIndex++
        )
   {
      if ( mRealm.compareTo(requestRealm) == 0 ) // case sensitive
      {
         OsSysLog::add(FAC_AUTH, PRI_DEBUG, "SipRouter:isAuthenticated: checking user '%s'",
                       requestUser.data());

         // Ignore this credential if it is not a current valid nonce
         if (mNonceDb.isNonceValid(requestNonce, callId, fromTag,
                                   mRealm, nonceExpires))
         {
            Url userUrl;
            UtlString authTypeDB;
            UtlString passTokenDB;

            // then get the credentials for this user and realm
            if(CredentialDB::getInstance()->getCredential(requestUser,
                                                          mRealm,
                                                          userUrl,
                                                          passTokenDB,
                                                          authTypeDB)
               )
            {
#                   ifdef TEST_PRINT
               // THIS SHOULD NOT BE LOGGED IN PRODUCTION
               // For security reasons we do not want to put passtokens into the log.
               OsSysLog::add(FAC_AUTH, PRI_DEBUG,
                             "SipRouter::isAuthenticated found credential "
                             "user: \"%s\" passToken: \"%s\"",
                             requestUser.data(), passTokenDB.data());
#                   endif
               authenticated = sipRequest.verifyMd5Authorization(requestUser.data(),
                                                                 passTokenDB.data(),
                                                                 requestNonce,
                                                                 requestRealm.data(),
                                                                 requestUri,
                                                                 HttpMessage::PROXY );

               if ( authenticated )
               {
                  userUrl.getIdentity(authUser);
                  OsSysLog::add(FAC_AUTH, PRI_DEBUG,
                                "SipRouter::isAuthenticated(): authenticated as '%s'",
                                authUser.data());
               }
               else
               {
                  OsSysLog::add(FAC_AUTH, PRI_DEBUG,
                                "SipRouter::isAuthenticated() authentication failed as '%s'",
                                requestUser.data());
               }
            }
            // Did not find credentials in DB
            else
            {
               OsSysLog::add(FAC_AUTH, PRI_INFO,
                             "SipRouter::isAuthenticated() No credentials found for user: '%s'",
                             requestUser.data());
            }
         }
         else // Is not a valid nonce
         {
            OsSysLog::add(FAC_AUTH, PRI_INFO,
                          "SipRouter::isAuthenticated() "
                          "Invalid NONCE: %s found "
                          "call-id: %s from tag: %s uri: %s realm: %s expiration: %ld",
                          requestNonce.data(), callId.data(), fromTag.data(),
                          requestUri.data(), mRealm.data(), nonceExpires);
         }
      }
      else
      {
         // wrong realm - meant for some other proxy on the path, so ignore it
      }
   } // looping through credentials

   return(authenticated);
}

/// Create an authentication challenge.
void SipRouter::authenticationChallenge(const SipMessage& sipRequest, ///< message to be challenged. 
                                     SipMessage& challenge         ///< challenge response.
                                     )
{
   UtlString newNonce;

   UtlString callId;
   sipRequest.getCallIdField(&callId);

   Url fromUrl;
   sipRequest.getFromUrl(fromUrl);
   UtlString fromTag;
   fromUrl.getFieldParameter("tag", fromTag);
   
   mNonceDb.createNewNonce(callId,
                           fromTag,
                           mRealm,
                           newNonce);

   challenge.setRequestUnauthorized(&sipRequest,
                                    HTTP_DIGEST_AUTHENTICATION,
                                    mRealm,
                                    newNonce, // nonce
                                    NULL, // opaque - not used
                                    HttpMessage::PROXY);
}

// P-Asserted-Identity header is only applicable for INVITE, REFER, BYE,
// OPTIONS, and SUBSCRIBE 
bool SipRouter::isPAIdentityApplicable(const SipMessage& sipRequest) 
                                     
{
   bool result = false;
   
   // Currently we only support SIP uri in P-Asserted-Identity, 
   // therefore only 1 P-Asserted-Identity is allowed.
   if(!sipRequest.getHeaderValue(0, SipXauthIdentity::PAssertedIdentityHeaderName))
   { 
       UtlString method;

       sipRequest.getRequestMethod(&method);

       if (0==method.compareTo(SIP_INVITE_METHOD, UtlString::ignoreCase))
       {
           UtlString toTag;
           Url toUrl;

           sipRequest.getToUrl(toUrl);
           toUrl.getFieldParameter("tag", toTag);

           if (toTag.isNull())
            {
                result = true;
            }
       }else if (0==method.compareTo(SIP_REFER_METHOD, UtlString::ignoreCase))
       {
            result = true;
       }
       else if (0==method.compareTo(SIP_BYE_METHOD, UtlString::ignoreCase))
       {
            result = true;
       }
       else if (0==method.compareTo(SIP_OPTIONS_METHOD, UtlString::ignoreCase))
       {
            result = true;
       }
       else if (0==method.compareTo(SIP_SUBSCRIBE_METHOD, UtlString::ignoreCase))
       {
            result = true;
       }
       else if (0==method.compareTo(SIP_NOTIFY_METHOD, UtlString::ignoreCase))
       {
            result = true;
       }

   }
   
   return result;
   
}

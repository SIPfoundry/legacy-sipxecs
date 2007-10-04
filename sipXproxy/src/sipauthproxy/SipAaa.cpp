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
#include "net/SipUserAgent.h"
#include "net/SipXauthIdentity.h"
#include "sipdb/ResultSet.h"
#include "sipdb/CredentialDB.h"
#include "AuthPlugin.h"
#include "SipAaa.h"
#include "sipXecsService/SipXecsService.h"
#include "sipXecsService/SharedSecret.h"

// DEFINES
//#define TEST_PRINT 1

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const char* AuthPlugin::Factory = "getAuthPlugin";
const char* AuthPlugin::Prefix  = "SIP_AUTHPROXY";
// The period of time in seconds that nonces are valid, in seconds.
#define NONCE_EXPIRATION_PERIOD    (60 * 5)     // five minutes

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipAaa::SipAaa(SipUserAgent& sipUserAgent,
               OsConfigDb&   configDb
               )
   :OsServerTask("SipAaa-%d", NULL, 2000)
   ,mpSipUserAgent(&sipUserAgent)
   ,mAuthenticationEnabled(true)    
   ,mNonceExpiration(NONCE_EXPIRATION_PERIOD) // the period in seconds that nonces are valid
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

   OsMsgQ* queue = getMessageQueue();
    
   // Register to get incoming requests
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

   // all is in readyness... let the proxying begin...
   mpSipUserAgent->start();
}

void SipAaa::readConfig(OsConfigDb& configDb, const Url& defaultUri)
{
   UtlString authScheme;
   configDb.get("SIP_AUTHPROXY_AUTHENTICATE_SCHEME", authScheme);
   if(authScheme.compareTo("none", UtlString::ignoreCase) == 0)
   {
      mAuthenticationEnabled = false;
      OsSysLog::add(FAC_SIP, PRI_WARNING,
                    "SIP_AUTHPROXY_AUTHENTICATE_SCHEME : NONE\n"
                    "  Authentication is disabled: there is NO permissions enforcement"
                    );
   }
   else
   {
      UtlString algorithm;
      if (OS_SUCCESS != configDb.get("SIP_AUTHPROXY_AUTHENTICATE_ALGORITHM", algorithm))
      {
         algorithm = "MD5";
      }
      if(authScheme.compareTo("MD5", UtlString::ignoreCase) == 0)
      {
         OsSysLog::add(FAC_SIP, PRI_INFO,
                       "SIP_AUTHPROXY_AUTHENTICATE_ALGORITHM : %s",
                       algorithm.data());
      }
      else if (algorithm.isNull())
      {
         OsSysLog::add(FAC_SIP, PRI_INFO,
                       "SIP_AUTHPROXY_AUTHENTICATE_ALGORITHM not set: using MD5"
                       );
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_WARNING,
                       "Unknown authentication algorithm:\n"
                       "SIP_AUTHPROXY_AUTHENTICATE_ALGORITHM : %s\n"
                       "   using MD5",
                       algorithm.data());
      }
   
      configDb.get("SIP_AUTHPROXY_AUTHENTICATE_REALM", mRealm);
      if(mRealm.isNull())
      {
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "SIP_AUTHPROXY_AUTHENTICATE_REALM not specified\n"
                       "   Phones must be configured with the correct default to authenticate."
                       );
         defaultUri.toString(mRealm);
      }
      OsSysLog::add(FAC_SIP, PRI_NOTICE, "SIP_AUTHPROXY_AUTHENTICATE_REALM : %s", mRealm.data());
   }
   
   configDb.get("SIP_AUTHPROXY_ROUTE_NAME", mRouteName);
   if(mRouteName.isNull())
   {
      OsSysLog::add(FAC_SIP, PRI_WARNING,
                    "SIP_AUTHPROXY_ROUTE_NAME not specified\n"
                    "   This may cause some peers to make a non-optimal routing decision."
                    );
      defaultUri.toString(mRouteName);
   }
   // this should really be redundant with the existing aliases,
   // but it's better to be safe and add it to ensure that it's
   // properly recognized (the alias db prunes duplicates anyway)
   mpSipUserAgent->setHostAliases(mRouteName);

   OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_AUTHPROXY_ROUTE_NAME : %s", mRouteName.data());

   configDb.get("SIP_AUTHPROXY_DOMAIN_NAME", mDomainName);
   if (mDomainName.isNull())
   {
      OsSysLog::add(FAC_SIP, PRI_ERR, "SIP_AUTHPROXY_DOMAIN_NAME not configured.");
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_AUTHPROXY_DOMAIN_NAME : %s", mDomainName.data());
   }
    
   // Load and configure all authorization plugins
   mAuthPlugins.readConfig(configDb);
}

// Destructor
SipAaa::~SipAaa()
{
   // Remove the message listener from the SipUserAgent, if there is one.
   if (mpSipUserAgent)
   {
      mpSipUserAgent->removeMessageObserver(*getMessageQueue());
   }
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean
SipAaa::handleMessage( OsMsg& eventMessage )
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
                       "SipAaa::handleMessage received transport error message") ;
      }
      else
      {
         SipMessage* sipRequest = const_cast<SipMessage*>(sipMsgEvent->getMessage());

         if ( sipRequest->isResponse() )
         {
            OsSysLog::add(FAC_AUTH, PRI_CRIT, "SipAaa::handleMessage received response");
            /*
             * Responses have already been proxied by the stack,
             * so we don't need to do anything with them.
             */
         }
         else
         {
            if (proxyMessage(*sipRequest))
            {
               // sipRequest may have been rewritten entirely by proxyMessage().
               // clear timestamps, protocol, and port information
               // so send will recalculate it
               sipRequest->resetTransport();
               mpSipUserAgent->send(*sipRequest);
            }
         }
      }
   }
   return(TRUE);
}

bool SipAaa::proxyMessage(SipMessage& sipRequest)
{
   bool requestShouldBeProxied = true;
   
   /*
    * Check for a Proxy-Require header containing unsupported extensions
    */
   UtlString extension;
   UtlString disallowedExtensions;
   for (int extensionIndex = 0;
        sipRequest.getProxyRequireExtension(extensionIndex, &extension);
        extensionIndex++
        )
   {
      if(!mpSipUserAgent->isExtensionAllowed(extension.data()))
      {
         if(!disallowedExtensions.isNull())
         {
            disallowedExtensions.append(SIP_MULTIFIELD_SEPARATOR);
            disallowedExtensions.append(SIP_SINGLE_SPACE);
         }
         disallowedExtensions.append(extension.data());
      }
   }

   if (disallowedExtensions.isNull())
   {
      // no unsupported extensions, so continue...
      
      UtlString callId;
      sipRequest.getCallIdField(&callId); // for logging

      // fix strict routes and remove any top route headers that go to myself
      Url normalizedRequestUri;
      UtlSList removedRoutes;
      sipRequest.normalizeProxyRoutes(mpSipUserAgent,
                                      normalizedRequestUri, ///< returns normalized request uri
                                      &removedRoutes        // route headers popped 
                                      );

      // get any state from the record-route and route headers
      RouteState routeState(sipRequest, removedRoutes); 
      removedRoutes.destroyAll(); // done with routes - discard them.
      
      /*
       * Determine the authIdentity.
       */
      UtlString authUser;
      bool requestIsAuthenticated = false;
      
      // Use the identity found in the SipX-Auth-Identity header if found
      SipXauthIdentity sipxIdentity(sipRequest, SipXauthIdentity::allowUnbound); 
      if ((requestIsAuthenticated = sipxIdentity.getIdentity(authUser)))
      {
         // found identity in request
         OsSysLog::add(FAC_AUTH, PRI_DEBUG, "SipAaa::proxyMessage "
                       " found valid sipXauthIdentity '%s' for callId %s",
                       authUser.data(), callId.data() 
                       );

         // Can't completely remove identity info, since it may be required
         // further if the request spirals. Normalize authIdentity to only leave
         // the most recent info in the request 
         SipXauthIdentity::normalize(sipRequest);
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
                       "SipAaa::proxyMessage plugin %s returned %s for %s",
                       authPluginName.data(),
                       AuthPlugin::AuthResultStr(pluginResult),
                       callId.data()
                       );

         // the first plugin to return something other than CONTINUE wins
         if (AuthPlugin::CONTINUE == finalAuthResult)
         {
            finalAuthResult = pluginResult;
            OsSysLog::add(FAC_AUTH, PRI_INFO,
                          "SipAaa::proxyMessage authorization %s by %s for %s",
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
         SipMessage authResponse;
         mpSipUserAgent->setServerHeader(authResponse);

         if (requestIsAuthenticated)
         {
            // Rewrite sipRequest as the authorization-needed response so our caller
            // can send it.
            authResponse.setResponseData(&sipRequest,
                                         SIP_FORBIDDEN_CODE,
                                         rejectReason.data());
         }
         else
         {
            // There was no authentication, so challenge to see if authenticated request would work
            authenticationChallenge(sipRequest, authResponse);
         }
         mpSipUserAgent->send(authResponse);

         requestShouldBeProxied = false;
      }
      break;

      case AuthPlugin::CONTINUE: // be permissive - if nothing says DENY, then treat as ALLOW
      case AuthPlugin::ALLOW:
      {
         // Request is sufficiently authorized, so proxy it.
         
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

         // Plugins may have modified the state - if allowed, put that state into the message
         if (routeState.isMutable())
         {
            routeState.update(&sipRequest, mRouteName);
         }
      }
      break;

      default:
         OsSysLog::add(FAC_SIP, PRI_CRIT, "SipAaa::proxyMessage plugin returned invalid result %d",
                       finalAuthResult);
         break;
      }
   }
   else
   {
      // The request has a Proxy-Require that we don't support; return an error
      requestShouldBeProxied = false;
      
      SipMessage response;
      response.setRequestBadExtension(&sipRequest, disallowedExtensions.data());
      mpSipUserAgent->setServerHeader(response);
      mpSipUserAgent->send(response);
   }

   return requestShouldBeProxied;
}

// @returns true iff the authority in url is a valid form of the domain name for this proxy.
bool SipAaa::isLocalDomain(const Url& url ///< a url to be tested
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

bool SipAaa::isAuthenticated(const SipMessage& sipRequest,
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
         OsSysLog::add(FAC_AUTH, PRI_DEBUG, "SipAaa:isAuthenticated: checking user '%s'",
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
                             "SipAaa::isAuthenticated found credential "
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
                                "SipAaa::isAuthenticated(): authenticated as '%s'",
                                authUser.data());
               }
               else
               {
                  OsSysLog::add(FAC_AUTH, PRI_DEBUG,
                                "SipAaa::isAuthenticated() authentication failed as '%s'",
                                requestUser.data());
               }
            }
            // Did not find credentials in DB
            else
            {
               OsSysLog::add(FAC_AUTH, PRI_INFO,
                             "SipAaa::isAuthenticated() No credentials found for user: '%s'",
                             requestUser.data());
            }
         }
         else // Is not a valid nonce
         {
            OsSysLog::add(FAC_AUTH, PRI_INFO,
                          "SipAaa::isAuthenticated() "
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
void SipAaa::authenticationChallenge(const SipMessage& sipRequest, ///< message to be challenged. 
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

/* ============================ FUNCTIONS ================================= */

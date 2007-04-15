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
#include "os/OsSysLog.h"
#include "net/SipUserAgent.h"
#include "SipRouter.h"
#include "ForwardRules.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipRouter::SipRouter(SipUserAgent& sipUserAgent, 
                     ForwardRules& forwardingRules,
                     bool          useAuthServer,
                     const char*   authServer,
                     bool          shouldRecordRoute) :
   OsServerTask("SipRouter-%d", NULL, 2000),
   mpSipUserAgent(&sipUserAgent),
   mpForwardingRules(&forwardingRules),
   mShouldRecordRoute(shouldRecordRoute),
   mAuthEnabled(useAuthServer)
{
   if ( mAuthEnabled )
   {
      if ( authServer && *authServer != '\000' )
      {
         // Construct the loose route URI to be used
         Url nextHopUrl(authServer);
         // Add a loose route to the mapped server
         nextHopUrl.setUrlParameter("lr", NULL);
         nextHopUrl.toString(mAuthRoute);
         OsSysLog::add(FAC_SIP, PRI_INFO, "SipRouter: auth server route '%s'", mAuthRoute.data());
      }
      else
      {
         mAuthEnabled = false;
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "SipRouter::SipRouter auth server enabled but no address set");
      }
   }

   if ( shouldRecordRoute )
   {
      UtlString uriString;
      int port;

      mpSipUserAgent->getViaInfo(OsSocket::UDP, uriString, port);
      Url routeUrl;
      routeUrl.setHostAddress(uriString.data());
      routeUrl.setHostPort(port);
      routeUrl.setUrlParameter("lr", NULL);

      routeUrl.toString(mRecordRoute);

      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipRouter Record-Route address: '%s'",
                    mRecordRoute.data());
   }

   // Register to get incoming requests
   OsMsgQ* queue = getMessageQueue();
   sipUserAgent.addMessageObserver(*queue,
                                   "", // All methods
                                   TRUE, // Requests,
                                   FALSE, //Responses,
                                   TRUE, //Incoming,
                                   FALSE, //OutGoing,
                                   "", //eventName,
                                   NULL, //   SipSession* pSession,
                                   NULL); //observerData)

}

// Destructor
SipRouter::~SipRouter()
{
   // Remove our message observer from *mpSipUserAgent, as it may
   // outlive us.
   mpSipUserAgent->removeMessageObserver(*getMessageQueue(),
                                         NULL);
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean SipRouter::handleMessage(OsMsg& eventMessage)
{
	if(OsMsg::PHONE_APP == eventMessage.getMsgType())
	{
		SipMessageEvent* sipMsgEvent = dynamic_cast<SipMessageEvent*>(&eventMessage);

      int messageType = sipMsgEvent->getMessageStatus();
      if(messageType == SipMessageEvent::TRANSPORT_ERROR)
      {
         OsSysLog::add(FAC_SIP, PRI_CRIT,
                       "SipRouter::handleMessage received transport error message");
      }
		else
		{
         SipMessage* sipRequest = const_cast<SipMessage*>(sipMsgEvent->getMessage());
         if(sipRequest)
         {
            if(!sipRequest->isResponse())
            {
               if (proxyMessage(*sipRequest))
               {
                  // clear timestamps, protocol, and port information so send will recalculate it
                  sipRequest->resetTransport();
                  mpSipUserAgent->send(*sipRequest);
               }
            }
            else 
            {
               // got a response - should never happen
               OsSysLog::add(FAC_SIP, PRI_CRIT,
                             "SipRouter::handleMessage received response");
            }
         }
         else 
         {
            // not a SIP message - should never happen
            OsSysLog::add(FAC_SIP, PRI_CRIT,
                          "SipRouter::handleMessage is not a sip message");
         }
      }
   }

   return TRUE;
}

bool SipRouter::proxyMessage(SipMessage& sipRequest)
{
   bool proxyMessage = false;
   
   // TODO - [XPR-183] Check for loops here.

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
   if (!disallowedExtensions.isNull())
   {
      SipMessage response;
      response.setRequestBadExtension(&sipRequest, disallowedExtensions.data());
      mpSipUserAgent->setServerHeader(response);
      mpSipUserAgent->send(response);
   }
   else
   {
   /*
    * Check the request URI and the topmost route
    *   - Detect and correct for any strict router upstream
    *     as specified by RFC 3261 section 16.4 Route Information Preprocessing
    *   - Pop off the topmost route until it is not me
    */
   Url requestUri;
   sipRequest.normalizeProxyRoutes(mpSipUserAgent, requestUri);
   // requestUri is set to the target uri from the request line after normalization
   
   UtlString topRouteValue;
   if (sipRequest.getRouteUri(0, &topRouteValue)) 
   {
      /*
       * There is a top route that is not to this domain
       * (if the top route were to this domain, it would have been removed),
       * so let the authproxy decide whether or not it can go through
       */
      addAuthRoute(sipRequest);
   }
   else // there is no Route header, so route on the Request URI
   {
      UtlString mappedTo;
      UtlString routeType;               
                  
      // see if we have a mapping for the normalized request uri
      if (   mpForwardingRules 
          && (mpForwardingRules->getRoute(requestUri, sipRequest, mappedTo, routeType)==OS_SUCCESS)
          )
      {
         // Yes, so add a loose route to the mapped server
         Url nextHopUrl(mappedTo);
         nextHopUrl.setUrlParameter("lr", NULL);
         UtlString routeString;
         nextHopUrl.toString(routeString);
         sipRequest.addRouteUri(routeString.data());

         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipRouter fowardingrules added route type '%s' to: '%s'",
                       routeType.data(), routeString.data());
      }
      else
      {
         // the mapping rules didn't have any route for this,
         // so let the authproxy decide whether or not it can go through
         addAuthRoute(sipRequest);
      }
   }

   if(mShouldRecordRoute) // If record route is configureed
   {
      sipRequest.addRecordRouteUri(mRecordRoute);
   }

   // Decrement max forwards
   //   we don't need to check for zero - the stack would already have rejected it.
   int maxForwards;
   if (sipRequest.getMaxForwards(maxForwards))
   {
      maxForwards--;
   }
   else
   {
      maxForwards = mpSipUserAgent->getMaxForwards();
   }

   sipRequest.setMaxForwards(maxForwards);
   proxyMessage = true;
   }
   
   return proxyMessage;
}

void SipRouter::addAuthRoute(SipMessage& request)
{
   if (mAuthEnabled)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipRouter routing through auth proxy");
      // Add a loose route to the auth server
      request.addRouteUri(mAuthRoute.data());
   }
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


//
//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsSysLog.h>
#include <utl/UtlString.h>
#include <os/OsSysLog.h>
#include <net/SipSubscribeServerEventHandler.h>
#include <net/SipPublishContentMgr.h>
#include <net/SipMessage.h>
#include <net/Url.h>
#include "AppAgentSubscribePolicy.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// Expiration time for nonces (in seconds):  5 minutes
#define RLS_SUBSCRIBE_NONCE_EXPIRATION	(5 * 60)

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
AppAgentSubscribePolicy::AppAgentSubscribePolicy(UtlString defaultDomain,
                                       UtlString realm,
                                       UtlString credentialDbName)
   : mRealm(realm),
     mNonceExpiration(RLS_SUBSCRIBE_NONCE_EXPIRATION),
     mDefaultDomain(defaultDomain),
     mCredentialDbName(credentialDbName)
{
}

// Copy constructor NOT IMPLEMENTED

// Destructor
AppAgentSubscribePolicy::~AppAgentSubscribePolicy()
{
    // Iterate through and delete all the dialogs
    // TODO:
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean AppAgentSubscribePolicy::getKeys(const SipMessage& subscribeRequest,
                                                   UtlString& resourceId,
                                                   UtlString& eventTypeKey,
                                                   UtlString& eventType)
{
    // default resourceId is the identity, but we want it from the From URI
    UtlString uriString;
    subscribeRequest.getFromUri(&uriString);
    Url uri(uriString);
    uri.getIdentity(resourceId);
    // Make the resourceId be a proper URI by prepending "sip:".
    resourceId.prepend("sip:");

    // Default event key is the event type with no parameters, but we want the full thing (dialog;sla)
    subscribeRequest.getEventField(eventTypeKey);
    // Event type is the same.
    eventType = eventTypeKey;

    return(TRUE);
}

UtlBoolean AppAgentSubscribePolicy::isAuthorized(const SipMessage& subscribeRequest,
                                            SipMessage& subscribeResponse)
{
   UtlBoolean ret = true;

   return ret;
}


UtlBoolean AppAgentSubscribePolicy::isAuthenticated(const SipMessage & subscribeRequest,
                                               SipMessage & subscribeResponse)
{
   UtlBoolean isAuthorized = FALSE;

   UtlString callId;
   subscribeRequest.getCallIdField(&callId);

   Url fromNameAddr;
   UtlString fromTag;
   subscribeRequest.getFromUrl(fromNameAddr);
   fromNameAddr.getFieldParameter("tag", fromTag);

   UtlString authNonce, authRealm, authUser, uriParam;

   // Iterate through Authorization and Proxy-Authorization credentials,
   // looking for one that shows this request is authenticated.
   for (int authIndex = 0;
           ! isAuthorized
        && subscribeRequest.getDigestAuthorizationData(
           &authUser, &authRealm, &authNonce,
           NULL, NULL, &uriParam,
           HttpMessage::SERVER, authIndex);
        authIndex++
      )
   {
      OsSysLog::add(FAC_AUTH, PRI_DEBUG, "Message Authorization received: "
                    "reqRealm='%s', reqUser='%s'", authRealm.data() , authUser.data());

      if (mRealm.compareTo(authRealm) == 0) // case sensitive check that realm is correct
      {
         OsSysLog::add(FAC_AUTH, PRI_DEBUG,
                       "AppAgentSubscribePolicy::isAuthenticated Realm Matches");

         UtlString authTypeDB;
         UtlString passTokenDB;

         // validate the nonce
         if (mNonceDb.isNonceValid(authNonce, callId, fromTag,
                                   mRealm, mNonceExpiration))
         {
            // build the "authorization identity" from the auth credentials
            Url authIdentity;

            // then get the credentials for this user & realm
            if (CredentialDB::getInstance(mCredentialDbName)->getCredential( authUser
                                                            ,authRealm
                                                            ,authIdentity
                                                            ,passTokenDB
                                                            ,authTypeDB
                   ))
            {
               // only DIGEST is used, so the authTypeDB above is ignored
               if ((isAuthorized =
                    subscribeRequest.verifyMd5Authorization(authUser.data(),
                                                            passTokenDB.data(),
                                                            authNonce,
                                                            authRealm.data(),
                                                            uriParam)
                      ))
               {
                  OsSysLog::add(FAC_AUTH, PRI_DEBUG,
                                "AppAgentSubscribePolicy::isAuthenticated "
                                "response auth hash matches");
               }
               else
               {
                  UtlString identity;
                  authIdentity.getIdentity(identity);
                  OsSysLog::add(FAC_AUTH, PRI_ERR,
                                "Response auth hash does not match (bad password?)"
                                " authIdentity='%s' authUser='%s'",
                                identity.data(), authUser.data());
               }
            }
            else // failed to get credentials
            {
               UtlString identity;
               authIdentity.getIdentity(identity);
               OsSysLog::add(FAC_AUTH, PRI_ERR,
                             "Unable to get credentials for realm='%s', user='%s'",
                             mRealm.data(), authUser.data());
            }
         }
         else // nonce is not valid
         {
            OsSysLog::add(FAC_AUTH, PRI_INFO,
                          "Invalid nonce: nonce='%s', callId='%s'",
                          authNonce.data(), callId.data());
         }
      }
   } //end for

   if ( !isAuthorized )
   {
      // Generate a new challenge
      UtlString newNonce;
      UtlString opaque;

      mNonceDb.createNewNonce(callId,
                              fromTag,
                              mRealm,
                              newNonce);

      subscribeResponse.setRequestUnauthorized(&subscribeRequest,
                                               HTTP_DIGEST_AUTHENTICATION,
                                               mRealm,
                                               newNonce,
                                               NULL // opaque
         );
   }

   return isAuthorized;
}

/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

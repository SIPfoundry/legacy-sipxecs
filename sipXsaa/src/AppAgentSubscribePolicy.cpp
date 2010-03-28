//
//
// Copyright (C) 2009, 2010 Avaya, Inc., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsSysLog.h"
#include "net/SipMessage.h"
#include "net/Url.h"
#include "sipdb/CredentialDB.h"
#include "AppAgentSubscribePolicy.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// Expiration time for nonces (in seconds):  5 minutes
#define SAA_SUBSCRIBE_NONCE_EXPIRATION	(5 * 60)

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
AppAgentSubscribePolicy::AppAgentSubscribePolicy(UtlString defaultDomain,
                                       UtlString realm,
                                       UtlString credentialDbName)
   : mRealm(realm),
     mNonceExpiration(SAA_SUBSCRIBE_NONCE_EXPIRATION),
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
    // Event type is the event type with no parameters.
    subscribeRequest.getEventFieldParts(&eventType);

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

   UtlString authNonce;
   UtlString authRealm;
   UtlString authUser;
   UtlString authUserBase;
   UtlString uriParam;
   UtlString authCnonce;
   UtlString authNonceCount;
   UtlString authQop;

   // Iterate through Authorization and Proxy-Authorization credentials,
   // looking for one that shows this request is authenticated.
   for (int authIndex = 0;
        ! isAuthorized
        && subscribeRequest.getDigestAuthorizationData(&authUser,
                                                       &authRealm,
                                                       &authNonce,
                                                       NULL, NULL, 
                                                       &uriParam,
                                                       &authCnonce,
                                                       &authNonceCount,
                                                       &authQop,
                                                       HttpMessage::SERVER, 
                                                       authIndex,
                                                       &authUserBase);
        authIndex++
      )
   {
      OsSysLog::add(FAC_AUTH, PRI_DEBUG, 
                    "AppAgentSubscribePolicy::isAuthenticated "
                    "Message Authorization received: "
                    "reqRealm='%s', reqUser='%s', reqUserBase='%s'",
                    authRealm.data(), authUser.data(), authUserBase.data());

      UtlString qopType;

      if (mRealm.compareTo(authRealm) ) // case sensitive check that realm is correct
      {
         OsSysLog::add(FAC_AUTH, PRI_DEBUG,
                       "AppAgentSubscribePolicy::isAuthenticated "
                       "Realm does not match");
      }

      // validate the nonce
      else if (!mNonceDb.isNonceValid(authNonce, callId, fromTag, mRealm, mNonceExpiration))
      {
          OsSysLog::add(FAC_AUTH, PRI_INFO,
                        "AppAgentSubscribePolicy::isAuthenticated -"
                        "Invalid nonce: nonce='%s', callId='%s'",
                        authNonce.data(), callId.data());
      }

      // verify that qop,cnonce, nonceCount are compatible
      else if (subscribeRequest.verifyQopConsistency(authCnonce.data(),
                                                     authNonceCount.data(),
                                                     &authQop,
                                                     qopType)
               >= HttpMessage::AUTH_QOP_NOT_SUPPORTED)
      {
          OsSysLog::add(FAC_AUTH, PRI_INFO,
                        "AppAgentSubscribePolicy::isAuthenticated -"
                        "Invalid combination of QOP('%s'), cnonce('%s') and nonceCount('%s')",
                        authQop.data(), authCnonce.data(), authNonceCount.data());
      }

      else // realm, nonce and qop are all ok
      {    
         // build the "authorization identity" from the auth credentials
         Url authIdentity;
         UtlString authTypeDB;
         UtlString passTokenDB;

         // then get the credentials for this user & realm
         if (CredentialDB::getInstance(mCredentialDbName)->getCredential( authUserBase
                                                                          ,authRealm
                                                                          ,authIdentity
                                                                          ,passTokenDB
                                                                          ,authTypeDB ))
         {
            // only DIGEST is used, so the authTypeDB above is ignored
            if ((isAuthorized =
                 subscribeRequest.verifyMd5Authorization(authUser.data(),
                                                         passTokenDB.data(),
                                                         authNonce.data(),
                                                         authRealm.data(),
                                                         authCnonce.data(),
                                                         authNonceCount.data(),
                                                         authQop.data(),
                                                         uriParam.data())
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
                             "AppAgentSubscribePolicy::isAuthenticated "
                             "Response auth hash does not match (bad password?)"
                             " authIdentity='%s' authUser='%s' authUserBase='%s'",
                             identity.data(), authUser.data(), authUserBase.data());
            }
         }
         else // failed to get credentials
         {
            UtlString identity;
            authIdentity.getIdentity(identity);
            OsSysLog::add(FAC_AUTH, PRI_ERR,
                          "AppAgentSubscribePolicy::isAuthenticated "
                          "Unable to get credentials for realm='%s', user='%s', userBase = '%s'",
                          mRealm.data(), authUser.data(), authUserBase.data());
         }
      }     // end DB check
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

// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
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
#include "RlsSubscribePolicy.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// Expiration time for nonces (in seconds):  5 minutes
#define RLS_SUBSCRIBE_NONCE_EXPIRATION	(5 * 60)

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
RlsSubscribePolicy::RlsSubscribePolicy(UtlString defaultDomain, 
                                       UtlString realm)
   : mRealm(realm),
     mNonceExpiration(RLS_SUBSCRIBE_NONCE_EXPIRATION),
     mDefaultDomain(defaultDomain)
{
}

// Copy constructor NOT IMPLEMENTED

// Destructor
RlsSubscribePolicy::~RlsSubscribePolicy()
{
    // Iterate through and delete all the dialogs
    // TODO:
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean RlsSubscribePolicy::isAuthorized(const SipMessage& subscribeRequest,
                                            const UtlString& resourceId,
                                            const UtlString& eventTypeKey,
                                            SipMessage& subscribeResponse)
{
   // SUBSCRIBE is authorized if "eventlist" is supported.
   UtlBoolean ret = subscribeRequest.isInSupportedField("eventlist");

   // If we return false, we must construct a failure response.
   if (!ret)
   {
      // 421 Extension Required
      // Require: eventlist"
      subscribeResponse.setResponseData(&subscribeRequest,
                                        SIP_EXTENSION_REQUIRED_CODE,
                                        SIP_EXTENSION_REQUIRED_TEXT);
      subscribeResponse.addRequireExtension("eventlist");
   }
   
   return ret;
}


UtlBoolean RlsSubscribePolicy::isAuthenticated(const SipMessage & subscribeRequest, 
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
                       "RlsSubscribePolicy::isAuthenticated Realm Matches");

         UtlString authTypeDB;
         UtlString passTokenDB;

         // validate the nonce
         if (mNonceDb.isNonceValid(authNonce, callId, fromTag,
                                   mRealm, mNonceExpiration))
         {
            // build the "authorization identity" from the auth credentials
            Url authIdentity;
                
            // then get the credentials for this user & realm
            if (CredentialDB::getInstance()->getCredential( authUser
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
                                "RlsSubscribePolicy::isAuthenticated "
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

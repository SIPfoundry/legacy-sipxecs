//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

/**
 * SIP Registrar Implied Subscriptions
 *
 * The SipImpliedSubscriptions::doImpliedSubscriptions method is invoked by
 *   the Registrar whenever a REGISTER request succeeds.  This object determines
 *   whether or not the register needs to generate any SUBSCRIBE requests on behalf
 *   of the originator of the REGISTER, and if so, creates and sends those SUBSCRIBE
 *   requests.
 *
 * Configuration is in the registrar-config file.  Directives that begin with the
 *   ConfigPrefix (const below) specify a regular expression to be checked against
 *   the User-Agent value in the REGISTER request.  When it matches, this module
 *   generates a SUBSCRIBE for message waiting indication on behalf of the phone.
 */


// SYSTEM INCLUDES
// :TODO: #include <stdlib.h>

// APPLICATION INCLUDES
#include "os/OsLock.h"
#include "os/OsDateTime.h"
#include "os/OsSysLog.h"
#include "os/OsConfigDb.h"
#include "utl/UtlString.h"
#include "utl/UtlRegex.h"
#include "net/NetMd5Codec.h"
#include "sipdb/CredentialDB.h"
#include "SipImpliedSubscriptions.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
// GLOBAL VARIABLES

extern "C" RegisterPlugin* getRegisterPlugin(const UtlString& name)
{
   OsLock lock(*SipImpliedSubscriptions::mpSingletonLock);

   RegisterPlugin* thePlugin;

   if (!SipImpliedSubscriptions::mpSingleton)
   {
      SipImpliedSubscriptions::mpSingleton = new SipImpliedSubscriptions(name);
      thePlugin = dynamic_cast<RegisterPlugin*>(SipImpliedSubscriptions::mpSingleton);
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_ERR,
                    "SipImpliedSubscriptions plugin may not be configured twice.\n"
                    "   First configured instance is %s.\n"
                    "   Second instance [%s] not created.",
                    SipImpliedSubscriptions::mpSingleton->mLogName.data(), name.data()
                    );

      thePlugin = NULL;
   }

   return thePlugin;
}

OsBSem* SipImpliedSubscriptions::mpSingletonLock = new OsBSem( OsBSem::Q_PRIORITY
                                                              ,OsBSem::FULL
                                                              );
SipImpliedSubscriptions* SipImpliedSubscriptions::mpSingleton;

// ImpliedSubscriptionUserAgent - private configuration class
//    Each of these records one implied subscription directive
//      from the registrar configuration file.
//    The parent UtlString value is the UA identifier from the configuration directive
//    These are created as members of the configuredUserAgents list, below.
class ImpliedSubscriptionUserAgent : public UtlString
{
public:
  ImpliedSubscriptionUserAgent( const UtlString& name
                               ,const UtlString& recognizer
                               ,const UtlString& logName
                               ) :
     UtlString(name)
  {
     mUserAgentRegEx = NULL;
     try
     {
        mUserAgentRegEx = new RegEx(recognizer.data());
     }
     catch(const char* compileError)
     {
        OsSysLog::add( FAC_SIP, PRI_ERR
                      ,"%s: Invalid recognizer expression '%s' for '%s%s': %s"
                      ,logName.data()
                      ,recognizer.data()
                      ,SipImpliedSubscriptions::ConfigPrefix
                      ,data()
                      ,compileError
                      );
     }
  }

  ~ImpliedSubscriptionUserAgent()
  {
     if (mUserAgentRegEx)
     {
        delete mUserAgentRegEx;
     }
  }

  UtlBoolean matchesRecognizer(const UtlString& rcvdUA, const UtlString& logName) const
  {
    UtlBoolean matched = FALSE;
    if (mUserAgentRegEx) // NULL if recognizer did not compile
    {
#      if 1 // :TODO: DEBUG
       OsSysLog::add( FAC_SIP, PRI_DEBUG
                     ,"%s checking %s: %s"
                     ,logName.data(), data(), rcvdUA.data()
                     );
#      endif
       matched = mUserAgentRegEx->Search(rcvdUA);
    }

    return matched;
  }

private:

  RegEx*   mUserAgentRegEx;
};

// ConfiguredUserAgents - private class
//    This is the list of ImpliedSubscriptionUserAgent objects;
//      one per configuration directive.
class ConfiguredUserAgents
{
public:
  ConfiguredUserAgents()
  {
  }

  ~ConfiguredUserAgents()
  {
     reset();
  }


  void add( const UtlString& name
           ,const UtlString& recognizer
           ,const UtlString& logName
           )
  {
    mConfiguredUserAgents.append(new ImpliedSubscriptionUserAgent( name, recognizer, logName ));
  }

  void reset()
  {
    mConfiguredUserAgents.destroyAll();
  }

  ImpliedSubscriptionUserAgent* configurationName( const UtlString& name
                                                  ,const UtlString& logName
                                                  )
  {
     ImpliedSubscriptionUserAgent* foundAgent = NULL;
     ImpliedSubscriptionUserAgent* agent;

     UtlSListIterator nextAgent(mConfiguredUserAgents) ;
     for (agent = dynamic_cast<ImpliedSubscriptionUserAgent*>(nextAgent());
          !foundAgent && agent;
          agent = dynamic_cast<ImpliedSubscriptionUserAgent*>(nextAgent())
          )
     {
        if (agent->matchesRecognizer(name, logName))
        {
           foundAgent = agent;
        }
     }

     return foundAgent ;
  }

private:
  UtlSList mConfiguredUserAgents ;

};

// the only instance of ConfiguredUserAgents
static ConfiguredUserAgents configuredUserAgents;

// the 'SIP_REGISTRAR.<instancename>.' has been stripped by the time we see it...
const char SipImpliedSubscriptions::ConfigPrefix[] = "UA.";

SipImpliedSubscriptions::SipImpliedSubscriptions(const UtlString& name) :
   RegisterPlugin(name)
{
   mLogName.append("[");
   mLogName.append(name);
   mLogName.append("] SipImpliedSubscriptions");
}

SipImpliedSubscriptions::~SipImpliedSubscriptions()
{
}

void SipImpliedSubscriptions::readConfig( OsConfigDb& configDb )
{
  OsConfigDb impliedSubscriptionConfig;
  OsStatus   found;
  UtlString   key;
  UtlString   name;
  UtlString   recognizer;

  configuredUserAgents.reset();

  // extract the database of implied message waiting subscriptions
  configDb.getSubHash( ConfigPrefix
                      ,impliedSubscriptionConfig
                      );
  for ( key = "", found = impliedSubscriptionConfig.getNext( key
                                                            ,name
                                                            ,recognizer
                                                            );
        found == OS_SUCCESS;
        key = name, found = impliedSubscriptionConfig.getNext( key
                                                              ,name
                                                              ,recognizer
                                                              )
       )
    {
      OsSysLog::add( FAC_SIP, PRI_INFO
                    ,"%s::readConfig name=\"%s\" recognizer=\"%s\""
                    ,mLogName.data(), name.data(), recognizer.data()
                    );
      configuredUserAgents.add( name, recognizer, mLogName );
    }
}


/**
 * doImpliedSubscriptions checks for characteristics in the REGISTER message
 *    that imply that a subscription needs to be requested on behalf of the
 *    party doing the registration, and then invokes the appropriate subscription
 *    request method.
 */

void SipImpliedSubscriptions::takeAction(
    const SipMessage&   registerMessage  ///< the successful registration
   ,const unsigned int  registrationDuration /**< the actual allowed
                                              * registration time (note
                                              * that this may be < the
                                              * requested time). */
   ,SipUserAgent*       sipUserAgent     /**< to be used if the plugin
                                          *   wants to send any SIP msg */
                                         )
{

   if ( needsImpliedSubscription( registerMessage ) )
   {
      OsSysLog::add( FAC_SIP, PRI_DEBUG
                    ,"%s requesting mwi subscription duration=%d"
                    ,mLogName.data(), registrationDuration
                    );

      // This phone - accepts message waiting notifies,
      // but doesn't subscribe to them, so we'll do it for them.
      SipMessage subscribeRequest;
      UtlString  callId;
      UtlString  fromTag;
      UtlString  fromUri;

      buildSubscribeRequest( registerMessage, registrationDuration
                            ,subscribeRequest, callId, fromTag, fromUri
                            );

      // If credentials aren't found, Authorization header won't be added
      addAuthorization( registerMessage, subscribeRequest, callId, fromTag, fromUri );

      // send SUBSCRIBE with or without Authorization header
      sipUserAgent->send( subscribeRequest, NULL, NULL );
   }
}

bool SipImpliedSubscriptions::needsImpliedSubscription( const SipMessage& registerMessage )
{
   bool configuredForSubscription = false;

   UtlString userAgent;
   ImpliedSubscriptionUserAgent* configured;

   registerMessage.getUserAgentField( &userAgent );

   OsSysLog::add( FAC_SIP, PRI_DEBUG
                 ,"%s checking User-Agent \"%s\""
                 ,mLogName.data(), userAgent.data()
                 );

   configured = configuredUserAgents.configurationName( userAgent, mLogName );

   if ( configured ) // ? did we find a configuration name whose recognizer matched ?
   {
      configuredForSubscription = true;
      OsSysLog::add( FAC_SIP, PRI_INFO
                    ,"%s User-Agent \"%s\" matched rule \"%s%s\""
                    ,mLogName.data()
                    ,userAgent.data()
                    ,ConfigPrefix
                    ,configured->data()
                    );
   }

   return configuredForSubscription;
}

void SipImpliedSubscriptions::buildSubscribeRequest( const SipMessage& registerMessage
                           ,int duration
                           ,SipMessage& subscribeRequest
                           ,UtlString&  callId
                           ,UtlString&  fromTag
                           ,UtlString&  fromUri
                           )
{
   UtlString registrationValue;
   UtlString tagNameValuePair;
   UtlString contactUri;
   int      sequenceNumber = 0;

   // Get the From URL, and change the tag
   Url fromUrl;
   registerMessage.getFromUrl( fromUrl );
   fromUrl.removeFieldParameter("tag"); // discard from tag from REGISTER
   registerMessage.getFromUri( &fromUri );
   (void) registerMessage.getContactUri(0, &contactUri);
   (void) registerMessage.getCSeqField(&sequenceNumber, &registrationValue);

   Url toUrl;
   registerMessage.getToUrl( toUrl );
   toUrl.removeFieldParameter("tag");

   UtlString toUri;
   registerMessage.getToUri( &toUri );

   registerMessage.getCallIdField( &callId );
   callId.prepend("implied-mwi-");

   // Build a from tag for the SUBSCRIBE
   //   - hash the call id so that it will be the same on each refresh
   UtlString callIdHash;
   NetMd5Codec::encode( callId.data(), callIdHash );
   fromUrl.setFieldParameter("tag", callIdHash.data() );
   fromTag = callIdHash; // for constructing the nonce

   subscribeRequest.setVoicemailData( fromUrl.toString() // From:
                                     ,toUrl.toString()   // To:
                                     ,toUri.data()       // request URI
                                     ,contactUri.data()  // taken from registration
                                     ,callId.data()
                                     ,++sequenceNumber
                                     ,duration
                                     );

   /*
    * Rewrite the event field to add our extension parameter to
    * ensure that the registration and subscription are synchronized.
    */
   const char* standardEventHeader = subscribeRequest.getHeaderValue(0, SIP_EVENT_FIELD);
   UtlString extendedEventHeader(standardEventHeader);
   extendedEventHeader.append(";" SIPX_IMPLIED_SUB "=");
   char durationString[12];
   sprintf(durationString, "%d", duration);
   extendedEventHeader.append(durationString);
   subscribeRequest.setHeaderValue(SIP_EVENT_FIELD, extendedEventHeader.data(), 0);
}


void SipImpliedSubscriptions::addAuthorization( const SipMessage& registerMessage
                                                ,SipMessage& subscribeRequest
                                                ,UtlString&  callId
                                                ,UtlString&  fromTag
                                                ,UtlString&  fromUri
                                                )
{
   // Construct authentication that the status server will accept
   // We need the user credentials, and a signed nonce like the one
   //    the status server would have generated to challenge this phone.
   UtlString user;
   UtlString userBase;
   UtlString realm;
   UtlString registrationNonce;
   UtlString opaque;
   UtlString response;
   UtlString authUri;

   // extract the identity from the authorization of the registration
   if ( registerMessage.getDigestAuthorizationData( &user, &realm       // the identity
                                                   ,NULL // request nonce not used
                                                   ,&opaque // passed through to aid debugging
                                                   ,NULL, NULL // response & authUri not used
                                                   ,HttpMessage::SERVER, 0
                                                   ,&userBase
                                                   )
       )
   {
      Url subscribeUser;
      UtlString passToken;
      UtlString authType;

      if (CredentialDB::getInstance()->getCredential( userBase, realm, subscribeUser
                                                     ,passToken, authType
                                                     )
          )
      {
         // Construct a nonce
         UtlString serverNonce;
         UtlString clientNonce;
         SipNonceDb* nonceDb = SharedNonceDb::get();

         nonceDb->createNewNonce( callId, fromTag, realm ,serverNonce );

         // generate a client nonce - doesn't matter what it is, really
         //   because the server doesn't validate this one;
         //   but change an input so that the two won't be the same
         //              UtlString dummyFromTag("different value");
         //              mRegistrarNonceDb.createNewNonce( callId, dummyFromTag, fromUri
         //                                               ,clientNonce
         //                               );

         // Construct A1
         UtlString a1Buffer;
         UtlString encodedA1;
         a1Buffer.append(user);
         a1Buffer.append(':');
         a1Buffer.append(realm);
         a1Buffer.append(':');
         a1Buffer.append(passToken);
         NetMd5Codec::encode(a1Buffer.data(), encodedA1);

         // Sign the message
         UtlString responseHash;
         HttpMessage::buildMd5Digest(encodedA1.data(),
                                     HTTP_MD5_ALGORITHM,
                                     serverNonce.data(),
                                     NULL, // client nonce
                                     1, // nonce count
                                     "",
                                     SIP_SUBSCRIBE_METHOD,
                                     fromUri.data(),
                                     NULL,
                                     &responseHash
                                     );

         subscribeRequest.removeHeader( HTTP_AUTHORIZATION_FIELD, 0);
         subscribeRequest.setDigestAuthorizationData(userBase.data(),
                                                     realm.data(),
                                                     serverNonce.data(),
                                                     fromUri.data(),
                                                     responseHash.data(),
                                                     HTTP_MD5_ALGORITHM,
                                                     NULL,//clientNonce.data(),
                                                     opaque.data(),
                                                     HTTP_QOP_AUTH,
                                                     1, // nonce count
                                                     HttpMessage::SERVER
                                                     );

      }
      else
      {
         OsSysLog::add( FAC_SIP, PRI_WARNING,
                       "%s implied subscription request not authenticated:\n"
                       "   no credentials found for \"%s\"",
                       mLogName.data(), userBase.data());
      }
   }
   else
   {
      OsSysLog::add( FAC_SIP, PRI_WARNING,
                    "%s implied subscription request not authenticated:\n"
                    "   no credentials in registration",
                    mLogName.data()
                    );
   }
}

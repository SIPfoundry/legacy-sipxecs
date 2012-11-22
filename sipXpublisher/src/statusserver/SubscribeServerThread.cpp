//
//
// Copyright (C) 2007, 2010 Avaya, Inc., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdlib.h>
#include <set>


// APPLICATION INCLUDES
#include "os/OsDateTime.h"
#include "os/OsLock.h"
#include "net/CallId.h"
#include "net/SipMessageEvent.h"
#include "net/SipUserAgent.h"
#include "net/NetMd5Codec.h"
#include "sipdb/ResultSet.h"
#include "sipdb/SubscribeDB.h"
#include "sipdb/EntityDB.h"
#include "statusserver/Notifier.h"
#include "statusserver/PluginXmlParser.h"
#include "statusserver/StatusServer.h"
#include "statusserver/SubscribeServerThread.h"
#include "statusserver/SubscribeServerPluginBase.h"
#include "statusserver/StatusPluginReference.h"
#include "utl/UtlSortedList.h"
#include "utl/UtlSortedListIterator.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
#define MIN_EXPIRES_TIME 300
#define DEFAULT_Q_VALUE " "

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
SubscribeServerThread::SubscribeServerThread(StatusServer& statusServer):
    OsServerTask("SubscribeServerThread", NULL, 2000),
    mStatusServer(statusServer),
    mpSipUserAgent(NULL),
    mIsStarted(FALSE),
    mDefaultSubscribePeriod( 24*60*60 ), //24 hours
    mPluginTable(NULL),
    mLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{}

SubscribeServerThread::~SubscribeServerThread()
{
    waitUntilShutDown();
    if( mpSipUserAgent )
        mpSipUserAgent = NULL;

    if( mPluginTable )
        mPluginTable = NULL;
}

///////////////////////////PUBLIC///////////////////////////////////
UtlBoolean
SubscribeServerThread::initialize (
    SipUserAgent* sipUserAgent,
    int defaultSubscribePeriod,
    const UtlString& minExpiresTime,
    const UtlString& defaultDomain,
    const UtlBoolean& useCredentialDB,
    const UtlString& realm,
    PluginXmlParser* pluginTable)
{
    if ( !mIsStarted )
    {
        //start the thread
        start();
    }

    if ( !minExpiresTime.isNull() )
    {
        mMinExpiresTimeStr.append(minExpiresTime);
        mMinExpiresTimeint = atoi(minExpiresTime.data());
    }

    mDefaultSubscribePeriod = defaultSubscribePeriod;
    if ( mMinExpiresTimeint > mDefaultSubscribePeriod )
    {
       Os::Logger::instance().log(FAC_SIP, PRI_ERR,
                     "SubscribeServerThread::initialize - minimum expiration time (%d)"
                     " > default/maximum expiration time (%d); reset to equal",
                     mMinExpiresTimeint,
                     mDefaultSubscribePeriod
          );
       mMinExpiresTimeint = mDefaultSubscribePeriod;
    }

    if ( !defaultDomain.isNull() )
    {
        mDefaultDomain.remove(0);
        mDefaultDomain.append( defaultDomain );
        Url defaultDomainUrl( mDefaultDomain );
        mDefaultDomainPort = defaultDomainUrl.getHostPort();

        // the isValidDomain compares the server's domain to
        // that in the incoming URI, sometimes the incoming
        // URI is a dotted IP address.
        UtlString ipOrFQDNHost;

        defaultDomainUrl.getHostAddress(ipOrFQDNHost);

        // sometimes the request URI will contain
        // an IP Address and at others it contains a FQDN
        if ( OsSocket::isIp4Address( ipOrFQDNHost.data() ) )
        {
            mDefaultDomainHostIP = ipOrFQDNHost;
            OsSocket::getDomainName( mDefaultDomainHostFQDN );
        } else
        {
            mDefaultDomainHostFQDN = ipOrFQDNHost;
            OsSocket::getHostIp( &mDefaultDomainHostIP );
        }
    }

    if ( !realm.isNull() )
    {
        mRealm.remove(0);
        mRealm.append(realm);
    }

    mIsCredentialDB = useCredentialDB;

    //get sip user agent
    if ( sipUserAgent )
    {
        mpSipUserAgent = sipUserAgent;
    } else
    {
        mpSipUserAgent = NULL;
        return FALSE;
    }

    //get Plugin table
    if ( pluginTable )
    {
        mPluginTable = pluginTable;
    } else
    {
        return FALSE;
    }
    return TRUE;
}


/// Insert a row in the subscription DB and schedule persisting the DB
UtlBoolean SubscribeServerThread::insertRow(
   const UtlString& uri,
   const UtlString& callid,
   const UtlString& contact,
   const int& expires,
   const int& subscribeCseq,
   const UtlString& eventTypeKey,
   const UtlString& eventType,
   const UtlString& id,
   const UtlString& to,
   const UtlString& from,
   const UtlString& key,
   const UtlString& recordRoute,
   const int& notifyCseq)
{
   UtlBoolean status = true;
   {
      // Critical Section here
      OsLock mutex(mLock);
      // (We must supply a dummy XML version number.)
      Os::Logger::instance().log(FAC_AUTH, PRI_DEBUG, "SubscribeServerThread::insertRow() calling SubscriptionDB::getInstance()->InsertRow()\n") ;
      StatusServer::getInstance()->getSubscribeDb()->upsert(
         SUBSCRIPTION_COMPONENT_STATUS,
         uri, callid, contact, expires, subscribeCseq, eventTypeKey, eventType,
         id, to, from, key, recordRoute, notifyCseq,
         CONTENT_TYPE_SIMPLE_MESSAGE_SUMMARY, 0);
   }

   return status;
}


/// Remove a row from the subscription DB and schedule persisting the DB
void SubscribeServerThread::removeRow(
   const UtlString& to,
   const UtlString& from,
   const UtlString& callid,
   const int& subscribeCseq )
{
   {
      StatusServer::getInstance()->getSubscribeDb()->remove(SUBSCRIPTION_COMPONENT_STATUS, to, from, callid, subscribeCseq);
   }
}

/// Remove an error row from the subscription DB and schedule persisting the DB
void SubscribeServerThread::removeErrorRow (
   const UtlString& to,
   const UtlString& from,
   const UtlString& callid )
{
   {
      StatusServer::getInstance()->getSubscribeDb()->removeError(SUBSCRIPTION_COMPONENT_STATUS, to, from, callid);
   }
}


//functions
UtlBoolean
SubscribeServerThread::handleMessage(OsMsg& eventMessage)
{
  std::string errorString;
  
  try
  {
    // Only handle SIP messages
    if (eventMessage.getMsgType() != OsMsg::PHONE_APP ||
        eventMessage.getMsgSubType() != SipMessage::NET_SIP_MESSAGE)
    {
       return FALSE ;
    }

    const SipMessage* message =
        ((SipMessageEvent&)eventMessage).getMessage();

    UtlString userKey;
    UtlString uri;
    SipMessage finalResponse;

    // Test for request/response processing code path
    if (!message->isResponse())
    {
        // this is a request, so authenticate and authorize the request
        if ( isValidDomain( message, &finalResponse ) )
        {
            UtlString eventPackage;
            UtlString id;
            UtlHashMap otherParams;

            message->getEventFieldParts(&eventPackage, &id, &otherParams);

            StatusPluginReference* pluginContainer =
                mPluginTable->getPlugin( eventPackage );

            if( pluginContainer )
            {
               //check in credential database if authentication needed
               UtlString authenticatedUser, authenticatedRealm;
               if( isAuthenticated ( message, &finalResponse, authenticatedUser, authenticatedRealm ) )
               {
                  if ( isAuthorized ( message, &finalResponse, pluginContainer ) )
                  {
                     // fetch the plugin
                     SubscribeServerPluginBase* plugin =
                        pluginContainer->getPlugin();

                     if (plugin)
                     {
                        int timeNow = (int)OsDateTime::getSecsSinceEpoch();
                        int grantedExpiration;
                        UtlString newToTag;

                        // add the subscription to the IMDB
                        SubscribeStatus isSubscriptionAdded
                           = addSubscription(timeNow,
                                             message,
                                             mDefaultDomain,
                                             eventPackage,
                                             eventPackage,
                                             id,
                                             otherParams,
                                             newToTag,
                                             grantedExpiration);

                        otherParams.destroyAll();

                        switch ( isSubscriptionAdded )
                        {
                        case STATUS_SUCCESS:
                           // create response - 202 Accepted Response
                           finalResponse.setResponseData( message,
                                                          SIP_ACCEPTED_CODE,
                                                          SIP_ACCEPTED_TEXT);
                           // Set the granted subscription time.
                           finalResponse.setExpiresField(grantedExpiration);

                           plugin->handleSubscribeRequest( *message,
                                                           finalResponse,
                                                           authenticatedUser.data(),
                                                           authenticatedRealm.data(),
                                                           mDefaultDomain.data());

                           // ensure that the contact returned will route back to here
                           // (the default supplied by SipUserAgent will not).
                           {
                              UtlString requestUri;
                              message->getRequestUri(&requestUri);
                              finalResponse.setContactField(requestUri);
                           }
                           break;

                        case STATUS_TO_BE_REMOVED:
                           // create response - 202 Accepted Response
                           finalResponse.setResponseData( message,
                                                          SIP_ACCEPTED_CODE,
                                                          SIP_ACCEPTED_TEXT);
                           // Set the granted subscription time.
                           finalResponse.setExpiresField(grantedExpiration);

                           plugin->handleSubscribeRequest( *message,
                                                           finalResponse,
                                                           authenticatedUser.data(),
                                                           authenticatedRealm.data(),
                                                           mDefaultDomain.data());

                           // ensure that the contact returned will route back to here
                           // (the default supplied by SipUserAgent will not).
                           {
                              UtlString requestUri;
                              message->getRequestUri(&requestUri);
                              finalResponse.setContactField(requestUri);
                           }
                           // Now that final NOTIFY has been sent, remove row
                           removeSubscription(message);

                           break;

                        case STATUS_LESS_THAN_MINEXPIRES:
                           // (already logged in addSubscription)

                           // send 423 Subscription Too Brief response
                           finalResponse.setResponseData(
                              message,
                              SIP_TOO_BRIEF_CODE,
                              SIP_TOO_BRIEF_TEXT );

                           finalResponse.setHeaderValue(
                              SIP_MIN_EXPIRES_FIELD,
                              mMinExpiresTimeStr,
                              0 );
                           break;

                        case STATUS_INVALID_REQUEST:
                           Os::Logger::instance().log(FAC_SIP, PRI_ERR,
                                         "SubscribeServerThread::handleMessage()"
                                         "Subscription Could Not Be Added "
                                         SIP_BAD_REQUEST_TEXT
                              );

                           finalResponse.setResponseData(
                              message,
                              SIP_BAD_REQUEST_CODE,
                              SIP_BAD_REQUEST_TEXT );
                           break;

                        case STATUS_FORBIDDEN:
                           Os::Logger::instance().log(FAC_SIP, PRI_ERR,
                                         "SubscribeServerThread::handleMessage()"
                                         "Subscription Could Not Be Added "
                                         SIP_FORBIDDEN_TEXT
                              );

                           finalResponse.setResponseData(
                              message,
                              SIP_FORBIDDEN_CODE,
                              SIP_FORBIDDEN_TEXT);
                           break;

                        case STATUS_NOT_FOUND:
                           Os::Logger::instance().log(FAC_SIP, PRI_ERR,
                                         "SubscribeServerThread::handleMessage()"
                                         "Subscription Could Not Be Added "
                                         SIP_NOT_FOUND_TEXT
                              );
                           finalResponse.setResponseData(
                              message,
                              SIP_NOT_FOUND_CODE,
                              SIP_NOT_FOUND_TEXT );
                           break;

                        case STATUS_BAD_SUBSCRIPTION:
                           // send 481 Subscription Does Not Exist response
                           Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                                         "SubscribeServerThread::handleMessage()"
                                         "Subscription to be renewed does not exist "
                                         SIP_BAD_SUBSCRIPTION_TEXT
                              );
                           finalResponse.setResponseData(
                              message,
                              SIP_BAD_SUBSCRIPTION_CODE,
                              SIP_BAD_SUBSCRIPTION_TEXT );
                           break;

                        case STATUS_INTERNAL_ERROR:
                        default:
                           Os::Logger::instance().log(FAC_SIP, PRI_ERR,
                                         "SubscribeServerThread::handleMessage()"
                                         "Subscription Could Not Be Added "
                                         "Status %d from addSubscription",
                                         isSubscriptionAdded
                              );
                           finalResponse.setResponseData(
                              message,
                              SIP_SERVER_INTERNAL_ERROR_CODE,
                              "Subscription database error" );
                        }

                        // Apply the new to-tag, if any, to the response.
                        if (!newToTag.isNull())
                        {
                           finalResponse.setToFieldTag(newToTag);
                        }
                     }
                     else
                     {
                        Os::Logger::instance().log(FAC_SIP, PRI_CRIT,
                                      "SubscribeServerThread::handleMessage()"
                                      " container->getPlugin failed for '%s'",
                                      eventPackage.data()
                           );
                        finalResponse.setResponseData(
                           message,
                           SIP_SERVER_INTERNAL_ERROR_CODE,
                           SIP_SERVER_INTERNAL_ERROR_TEXT );
                     }
                  }
                  else
                  {
                     // not authorized - the response was created in isAuthorized
                  }
               }
               else
               {
                  // not authenticated - the response was created in isAuthenticated
               }
            }
            else // no plugin found for this event type
            {
               Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                             "SubscribeServerThread::handleMessage()"
                             " Request denied - "
                             SIP_BAD_EVENT_TEXT
                  );
               finalResponse.setResponseData( message,
                                              SIP_BAD_EVENT_CODE,
                                              "Event type not supported" );
            }

            // send final response
            UtlString finalMessageStr;
            ssize_t finalMessageLen;
            finalResponse.getBytes(&finalMessageStr, &finalMessageLen);
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "\n----------------------------------\n"
                "Sending final response\n%s",finalMessageStr.data());
            mpSipUserAgent->setUserAgentHeader( finalResponse );
            mpSipUserAgent->send( finalResponse );
        }
        else // Invalid domain
        {
           const char* notFoundMsg = SIP_NOT_FOUND_TEXT " Invalid Domain";
           finalResponse.setResponseData(message,
                                         SIP_NOT_FOUND_CODE,
                                         notFoundMsg
                                         );
           mpSipUserAgent->setUserAgentHeader( finalResponse );
           mpSipUserAgent->send( finalResponse );
        }
    }
    else // response
    {
       // The server may send us back a "481" response, if it does we need
       // to remove the subscription from the SubscriptionDB as the callid
       // that it corresponds to is stale (probably the phone was rebooted)
       // In the above case, RFC 3265 says we MUST remove the subscription.
       // It also says (essentially) that any error that does not imply a retry
       // SHOULD remove the subscription.  We will interpret this to be any
       // 4xx code _except_ 408 timeout (because that may be a transient error).
       int responseCode = message->getResponseStatusCode();
       if (   responseCode >= SIP_4XX_CLASS_CODE
           && responseCode != SIP_REQUEST_TIMEOUT_CODE )
       {
          // remove the subscription
          removeErrorSubscription ( *message );
       }
    }
    return TRUE;
  }
#ifdef MONGO_assert
  catch (mongo::DBException& e)
  {
    errorString = "MWI - Mongo DB Exception";
    OS_LOG_ERROR( FAC_SIP, "SubscribeServerThread::handleMessage() Exception: "
             << e.what() );
  }
#endif
  catch (boost::exception& e)
  {
    errorString = "MWI - Boost Library Exception";
    OS_LOG_ERROR( FAC_SIP, "SubscribeServerThread::handleMessage() Exception: "
             << boost::diagnostic_information(e));
  }
  catch (std::exception& e)
  {
    errorString = "MWI - Standard Library Exception";
    OS_LOG_ERROR( FAC_SIP, "SubscribeServerThread::handleMessage() Exception: "
             << e.what() );
  }
  catch (...)
  {
    errorString = "MWI - Unknown Exception";
    OS_LOG_ERROR( FAC_SIP, "SubscribeServerThread::handleMessage() Exception: Unknown Exception");
  }

  //
  // If it ever get here, that means we caught an exception
  //
  if (eventMessage.getMsgType()  == OsMsg::PHONE_APP)
  {
    const SipMessage& message = *((SipMessageEvent&)eventMessage).getMessage();
    if (!message.isResponse())
    {
      SipMessage finalResponse;
      finalResponse.setResponseData(&message, SIP_5XX_CLASS_CODE, errorString.c_str());
      mpSipUserAgent->send(finalResponse);
    }
  }

  return(TRUE);
}

UtlBoolean
SubscribeServerThread::isAuthorized (
    const SipMessage* message,
    SipMessage *responseMessage,
    StatusPluginReference* pluginContainer)
{
    UtlBoolean retIsAuthorized = FALSE;
    UtlString  requestUser;
    Url       identityUrl;
    message->getUri(NULL, NULL, NULL, &requestUser);
    identityUrl.setUserId(requestUser);
    identityUrl.setHostAddress(mDefaultDomain);

    EntityDB* entityDb = StatusServer::getInstance()->getEntityDb();

    if( pluginContainer )
    {
        // if the plugin has permissions, we must match all these against the IMDB
        if( pluginContainer->hasPermissions() )
        {
            // permission required. Check for required permission in permission IMDB
            // All required permissions should match

            EntityRecord entity;
            entityDb->findByIdentity(identityUrl, entity);
            std::set<std::string> permissions = entity.permissions();

            int numDBPermissions = permissions.size();

            if( numDBPermissions > 0 )
            {
                UtlBoolean nextPermissionMatched = TRUE;

                UtlSListIterator* pluginPermissionIterator = pluginContainer->permissionsIterator();
                UtlString* pluginPermission;
                // Iterated through the plugin permissions matching
                // them one by one against the IMDB
                while(   (pluginPermission = (UtlString*)(*pluginPermissionIterator)())
                      && nextPermissionMatched
                      )
                {
                    //check againt all permissions in IMDB
                    nextPermissionMatched = FALSE;
                    UtlString identity, permission;
                    for ( std::set<std::string>::iterator iter = permissions.begin(); iter != permissions.end(); iter++ )

                    {

                        permission = iter->c_str();
                        if (pluginPermission->compareTo(permission, UtlString::ignoreCase ) == 0)
                        {
                            nextPermissionMatched = TRUE;
                            break;
                        }
                    }
                }
                delete pluginPermissionIterator;

                // after going thru all permissions find out if all matched or not
                if( nextPermissionMatched )
                {
                   Os::Logger::instance().log(FAC_AUTH, PRI_DEBUG, "SubscribeServerThread::isAuthorized() -"
                        " All permissions matched - request is AUTHORIZED");
                    retIsAuthorized = TRUE;
                }
                else
                {
                    Os::Logger::instance().log(FAC_AUTH, PRI_DEBUG, "SubscribeServerThread::isAuthorized() -"
                        " One or more Permissions did not match - request is UNAUTHORIZED");
                    retIsAuthorized = FALSE;
                }
            }
            else
            {
                // one or more permissions needed by plugin and none in IMDB => UNAUTHORIZED
                Os::Logger::instance().log(FAC_AUTH, PRI_DEBUG, "SubscribeServerThread::isAuthorized() -"
                    " No Permissions in IMDB - request is UNAUTHORIZED");
                retIsAuthorized = FALSE;
            }
        }
        else
        {
            Os::Logger::instance().log(FAC_AUTH, PRI_DEBUG, "SubscribeServerThread::isAuthorized() -"
                " No Permissions required - request is always AUTHORIZED");
            retIsAuthorized = TRUE;
        }
    }
    //set the error response message id unauthorized
    if(!retIsAuthorized)
    {
        responseMessage->setResponseData(message,SIP_FORBIDDEN_CODE, SIP_FORBIDDEN_TEXT);
    }
    return retIsAuthorized;
}

UtlBoolean
SubscribeServerThread::isAuthenticated (const SipMessage* message,
                                        SipMessage *responseMessage,
                                        UtlString& authenticatedUser,
                                        UtlString& authenticatedRealm )
{
    UtlBoolean retIsAuthenticated = FALSE;

    // if we are not using a database we must assume authenticated
    if ( !mIsCredentialDB )
    {
        Os::Logger::instance().log(FAC_AUTH, PRI_DEBUG, "SubscribeServerThread::isAuthenticated() "
            ":: No Credential DB - request is always AUTHENTICATED");
        retIsAuthenticated = TRUE;
    } else
    {
        // realm and auth type should be default for server
        // if URI not defined in DB, the user is not authorized to modify bindings -
        Os::Logger::instance().log( FAC_AUTH, PRI_DEBUG, "SubscribeServerThread::isAuthenticated():TRUE realm=\"%s\" ",
                mRealm.data());

        UtlString requestNonce;
        UtlString requestCnonce;
        UtlString requestNonceCount;
        UtlString requestQop;
        UtlString requestRealm;
        UtlString requestUser;
        UtlString requestUserBase;
        UtlString requestUriParam;
        int requestAuthIndex = 0;
        EntityDB* entityDb = StatusServer::getInstance()->getEntityDb();
        // can have multiple authorization / authorization-proxy headers
        // headers search for a realm match
        while ( message->getDigestAuthorizationData ( &requestUser,
                                                      &requestRealm,
                                                      &requestNonce,
                                                      NULL, // opaque
                                                      NULL, // response
                                                      &requestUriParam,
                                                      &requestCnonce,
                                                      &requestNonceCount,
                                                      &requestQop,
                                                      HttpMessage::SERVER,
                                                      requestAuthIndex,
                                                      &requestUserBase) )
        {
            Os::Logger::instance().log(FAC_AUTH, PRI_DEBUG, 
                          "SubscribeServerThread::isAuthenticated() "
                          "- Authorization header set in message, validate it.\n"
                          "- reqRealm=\"%s\", reqUser=\"%s\", reqUserBase=\"%s\"",
                          requestRealm.data(), requestUser.data(),
                          requestUserBase.data());

            UtlString qopType;
            UtlString callId;
            UtlString fromTag;
            long     nonceExpires = (5*60); // five minutes

            Url fromUrl;
            message->getFromUrl(fromUrl);
            fromUrl.getFieldParameter("tag", fromTag);
            UtlString reqUri;
            message->getRequestUri(&reqUri);
            Url mailboxUrl(reqUri);

            message->getCallIdField(&callId);

            if (mRealm.compareTo(requestRealm) ) // case sensitive check that realm is correct
            {
               Os::Logger::instance().log(FAC_AUTH, PRI_DEBUG,
                             "SubscribeServerThread::isAuthenticated() "
                             "Realm does not match");
            }

            // See if the nonce is valid - see net/SipNonceDb.cpp
            else if (!mNonceDb.isNonceValid(requestNonce, callId, fromTag, mRealm, nonceExpires))
            {
                Os::Logger::instance().log(FAC_AUTH, PRI_INFO,
                              "SubscribeServerThread::isAuthenticated() "
                              "Invalid NONCE: '%s' found for mailboxUrl '%s' "
                              "realm: '%s' user: '%s' "
                              "cnonce: '%s' nc: '%s' qop: '%s' "
                              "expiration: %ld",
                              requestNonce.data(), mailboxUrl.toString().data(),
                              mRealm.data(), requestUser.data(), 
                              requestCnonce.data(), requestNonceCount.data(), 
                              requestQop.data(), nonceExpires);
            }

            // verify that qop,cnonce, nonceCount are compatible
            else if (message->verifyQopConsistency(requestCnonce.data(),
                                                     requestNonceCount.data(),
                                                     &requestQop,
                                                     qopType)
                     >= HttpMessage::AUTH_QOP_NOT_SUPPORTED)
            {
                Os::Logger::instance().log(FAC_AUTH, PRI_INFO,
                              "SubscribeServerThread::isAuthenticated() "
                              "Invalid combination of QOP('%s'), cnonce('%s') and nonceCount('%s')",
                              requestQop.data(), requestCnonce.data(), requestNonceCount.data());
            }

            else // realm, nonce and qop are all ok
            {    
                UtlString authTypeDB;
                UtlString passTokenDB;

                // then get the credentials for this realm
                if (entityDb->getCredential(mailboxUrl, mRealm, requestUserBase, passTokenDB, authTypeDB))
                {
                    // the Digest Password is calculated from the request
                    // user, passtoken, nonce and request URI

                    retIsAuthenticated =
                       message->verifyMd5Authorization(requestUser.data(),
                                                       passTokenDB.data(),
                                                       requestNonce.data(),
                                                       requestRealm.data(),
                                                       requestCnonce.data(),
                                                       requestNonceCount.data(),
                                                       requestQop.data(),
                                                       requestUriParam.data());
                    if (retIsAuthenticated)
                    {
                        // can have multiple credentials for same realm so only break out
                        // when we have a positive match
                        Os::Logger::instance().log(FAC_AUTH, PRI_DEBUG, 
                                      "SubscribeServerThread::isAuthenticated() "
                                      "- request is AUTHENTICATED");
                        // copy the authenticated user/realm for subsequent authorization
                        authenticatedUser = requestUserBase;
                        authenticatedRealm = requestRealm;
                        break;
                    }
                    else
                    {
                        Os::Logger::instance().log(FAC_AUTH, PRI_DEBUG,
                                      "SubscribeServerThread::isAuthenticated() "
                                      "- digest authorization failed "
                                      "nonce \"%s\", cnonce \"%s\" for "
                                      "mailboxUrl=\"%s\", reqRealm=\"%s\", reqUser=\"%s\"",
                                      requestNonce.data(),
                                      requestCnonce.data(),
                                      mailboxUrl.toString().data(),
                                      requestRealm.data(),
                                      requestUser.data());
                    }
                }
                else
                {
                    Os::Logger::instance().log(FAC_AUTH, PRI_DEBUG, 
                                  "SubscribeServerThread::isAuthenticated() "
                                  "- No Credentials for mailboxUrl=\"%s\", reqRealm=\"%s\", reqUser=\"%s\"",
                        mailboxUrl.toString().data(),
                        requestRealm.data(),
                        requestUser.data());
                }  // end check credentials
            }
            requestAuthIndex++;
        } //end while

        if ( !retIsAuthenticated )
        {
            // Generate the 401 Unauthorized response to challenge for credentials
            // Use the SipNonceDB to generate a nonce
            UtlString newNonce;
            UtlString callId;
            UtlString fromTag;

            Url fromUrl;
            message->getFromUrl(fromUrl);
            fromUrl.getFieldParameter("tag", fromTag);

            message->getCallIdField(&callId);

            mNonceDb.createNewNonce(callId,
                                    fromTag,
                                    mRealm,
                                    newNonce);

            responseMessage->setRequestUnauthorized(message, HTTP_DIGEST_AUTHENTICATION,
                                                    mRealm, newNonce);
        }
    }  // end DB exists
    return retIsAuthenticated;
}

UtlBoolean
SubscribeServerThread::isValidDomain(
    const SipMessage* message,
    SipMessage* responseMessage )
{
    UtlString address;
    UtlString requestUri;
    message->getRequestUri(&requestUri);
    Url reqUri(requestUri);
    reqUri.getHostAddress(address);
    int port = reqUri.getHostPort();

    // Compare against either an IP address of the
    // local host or a FullyQalified Domain Name
    if ( ( (address.compareTo(mDefaultDomainHostIP.data(), UtlString::ignoreCase) == 0) ||
           (address.compareTo(mDefaultDomainHostFQDN.data(), UtlString::ignoreCase) == 0) )
         && ( (mDefaultDomainPort == PORT_NONE) || (port == mDefaultDomainPort) ) )
    {
        Os::Logger::instance().log(FAC_AUTH, PRI_DEBUG, "SubscribeServerThread::isValidDomain() - VALID Domain") ;
        return TRUE;
    }
    Os::Logger::instance().log(FAC_AUTH, PRI_DEBUG, "SubscribeServerThread::isValidDomain() - INVALID Domain") ;
    return FALSE;
}


SubscribeServerThread::SubscribeStatus SubscribeServerThread::addSubscription(
    const int timeNow,
    const SipMessage* subscribeMessage,
    const char* domain,
    const UtlString& eventTypeKey,
    const UtlString& eventType,
    const UtlString& eventId,
    const UtlHashMap& eventParams,
    UtlString& newToTag,
    int& grantedExpiration)
{
    SubscribeStatus returnStatus = STATUS_INTERNAL_ERROR;
    int subscribeCseqInt = 0;
    UtlString callId;
    UtlString contactEntry;
    UtlString to;
    UtlString from;
    UtlString route;
    UtlString key;
    UtlString method;
    Url identity;

    //  Construct the identity
    UtlString uriUser, requestUri;
    subscribeMessage->getUri( NULL, NULL, NULL, &uriUser );
    subscribeMessage->getRequestUri( &requestUri );
    identity.setUserId( uriUser );
    identity.setUrlType( "sip" );
    identity.setHostAddress( domain );

    subscribeMessage->getToField(&to);
    subscribeMessage->getFromField(&from);
    subscribeMessage->getCallIdField(&callId);
    subscribeMessage->getCSeqField(&subscribeCseqInt, &method);
    subscribeMessage->getContactEntry(0, &contactEntry);

    subscribeMessage->buildRouteField(&route);
    Url toUrl;
    subscribeMessage->getToUrl(toUrl);
    int commonExpires = -1;
    if ( subscribeMessage->getExpiresField( &commonExpires ) )
    {
       if( commonExpires > 0 ) // came from request
       {
          if (commonExpires < mMinExpiresTimeint)
          {
             returnStatus = STATUS_LESS_THAN_MINEXPIRES;
             Os::Logger::instance().log( FAC_SIP, PRI_ERR, "addSubscription: "
                            "Expires (%d) less than Minimum (%d)",
                           commonExpires, mMinExpiresTimeint);
             return returnStatus;
          }
          else if (commonExpires > mDefaultSubscribePeriod)
          {
             commonExpires = mDefaultSubscribePeriod;
          }
          else
          {
             // commonExpires is in the allowed range - use the requested value
          }
        }
        else if( commonExpires == 0 )
        {
            // remove subscription binding
            // remove all bindings  because one contact value is *
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,"SubscribeServerThread::addSubscription -"
                " subscription for url '%s' and event type key '%s' to be removed after sending NOTIFY",
                toUrl.toString().data(), eventTypeKey.data());

            returnStatus = STATUS_TO_BE_REMOVED;
            return returnStatus;
        }
        else if( commonExpires == -1) // no expires value in request
        {
            // Assume the default value
            commonExpires = mDefaultSubscribePeriod;
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,"SubscribeServerThread::addSubscription -"
                " No Expires Value, assigning default value (%d)", commonExpires);
        }
    }

    UtlString sipxImpliedParameter(SIPX_IMPLIED_SUB);
    UtlString* sipxImpliedDuration = NULL;

    int grantedExpirationTime;
    if ((  sipxImpliedDuration
         = dynamic_cast<UtlString*>(eventParams.findValue(&sipxImpliedParameter))))
    {
       /*
        * This request was generated by the registrar as an implied subscription;
        * its duration must therefore match that of the registration, which has
        * already been randomized.
        */
       grantedExpirationTime = atoi(sipxImpliedDuration->data());
    }
    else
    {
       /*
        * The request is for a new or refreshed subscription (other cases were handled above);
        * commonExpires is now the requested duration in the range:
        *    mMinExpiresTimeint <= commonExpires <= mDefaultSubscribePeriod
        * In order to distribute expirations smoothly, we actually grant a randomized duration
        */
       int spreadFloor = mMinExpiresTimeint*2;

       if ( commonExpires > spreadFloor )
       {
          // a normal (long) registration
          // - spread it between twice the min and the longest they asked for
          grantedExpirationTime = (  (rand() % (commonExpires - spreadFloor))
                                   + spreadFloor);
       }
       else if ( commonExpires > mMinExpiresTimeint )
       {
          // a short but not minimum registration
          // - spread it between the min and the longest they asked for
          grantedExpirationTime = (  (rand()
                                      % (commonExpires - mMinExpiresTimeint)
                                      )
                                   + mMinExpiresTimeint
                                   );
       }
       else // longestExpiration == mMinExpiresTimeint
       {
          // minimum - can't randomize because we can't shorten or lengthen it
          grantedExpirationTime = mMinExpiresTimeint;
       }
    }

    // Handle the to-tag:
    // If no to-tag, this is a new subscription, for which we must create
    // a to-tag.
    // If there is a to-tag, this is a renewal of a subscription, and we
    // must first check that the subscription exists.  (This is because if
    // the UA thinks there is a subscription, but we do not know of it, we
    // cannot reconstitute the subscription, as we do not know the NOTIFY CSeq
    // that the UA expects.  See XECS-406 for the ill consequences of
    // this problem.)

    // Clear the returned new to-tag.
    newToTag.remove(0);
    {
       UtlString x;
       bool r = toUrl.getFieldParameter("tag", x);
       Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,"SubscribeServerThread::addSubscription getting to-tag, return %d, value '%s'",
                     (int) r, x.data());
    }
    UtlString toTag;
    bool exists = false;
    if (toUrl.getFieldParameter("tag", toTag))
    {
       // Check to see if this subscription exists.
       // Critical Section here
       OsLock mutex(mLock);

       exists = StatusServer::getInstance()->getSubscribeDb()->subscriptionExists(
                SUBSCRIPTION_COMPONENT_STATUS, to, from, callId, OsDateTime::getSecsSinceEpoch());
       Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,"SubscribeServerThread::addSubscription subscriptionExists(..., '%s', '%s', '%s', %d) = %d",
                     to.data(), from.data(),
                     callId.data(), (int) OsDateTime::getSecsSinceEpoch(),
                     exists);

       if (!exists)
       {
          returnStatus = STATUS_BAD_SUBSCRIPTION;
          return returnStatus;
       }
    }
    else
    {
       // Generate a random to-tag.
       CallId::getNewTag(newToTag);
       // Add it to the remembered To header value.
       to.append(";tag=");
       to.append(newToTag);
       Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,"SubscribeServerThread::addSubscription generated to-tag '%s'",
                     newToTag.data());
    }

    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                  "SubscribeServerThread::addSubscription -"
                  " Adding/updating subscription for URI '%s' event type key '%s' duration %d to '%s'",
                  toUrl.toString().data(), eventTypeKey.data(), grantedExpirationTime, contactEntry.data());

    // trim the contact to just the uri
    Url contactUrl(contactEntry);
    contactUrl.getUri(contactEntry);

    // add bindings
    identity.getIdentity( key );

    // Insert or update the information for this subscription.
    // (Note that the "notify CSeq" parameter is ignored if there is
    // an existing SubscriptionDB row for this subscription.)
    if (insertRow(requestUri, // identity,
                  callId,
                  contactEntry,
                  grantedExpirationTime + timeNow,
                  subscribeCseqInt,
                  eventTypeKey,
                  eventType,
                  eventId,
                  to,
                  from,
                  key,                 // this will be searched for later
                  route,
                  !exists))    // initial notify cseq (sent to phone).  This will be false(0) if dialog already exists
    {
       grantedExpiration = grantedExpirationTime;
       returnStatus = STATUS_SUCCESS;
    }
    else
    {
       // log the error and send error indication to subscriber
       Os::Logger::instance().log(FAC_SIP, PRI_ERR,
                     "SubscribeServerThread::addSubscription -"
                     " Could not insert record in Database");

       returnStatus = STATUS_INTERNAL_ERROR;
    }

    return returnStatus;
}

SubscribeServerThread::SubscribeStatus SubscribeServerThread::removeSubscription(
    const SipMessage* subscribeMessage)
{
    int subscribeCseqInt = 0;
    UtlString callId;
    UtlString to;
    UtlString from;
    UtlString method;

    subscribeMessage->getToField(&to);
    subscribeMessage->getFromField(&from);
    subscribeMessage->getCallIdField(&callId);
    subscribeMessage->getCSeqField(&subscribeCseqInt, &method);
    Url toUrl;
    subscribeMessage->getToUrl(toUrl);
    // remove subscription binding
    // remove all bindings  because one contact value is *
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "SubscribeServerThread::removeSubscription -"
      " Removing subscription for url %s and callid %s", toUrl.toString().data(), callId.data());

    // note that the subscribe's csequence is used
    // as a remove filter here (all rows with CSEQ < this are removed)
    removeRow(to, from, callId, subscribeCseqInt);

    return STATUS_REMOVED;
}

int SubscribeServerThread::removeErrorSubscription (const SipMessage& sipMessage )
{
    int returnStatus = STATUS_SUCCESS;
    UtlString callId;
    UtlString to;
    UtlString from;
    sipMessage.getToField(&to);
    sipMessage.getFromField(&from);
    sipMessage.getCallIdField(&callId);

    Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                  "SubscribeServerThread::removeErrorSubscription %s",
                  callId.data());

    removeErrorRow(from, to, callId);
    return returnStatus;
}

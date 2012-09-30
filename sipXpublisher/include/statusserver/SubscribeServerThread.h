//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////// SubscribeServerThread.h: interface for the SubscribeServerThread class.
//
//////////////////////////////////////////////////////////////////////

#ifndef SUBSCRIBESERVERTHREAD_H
#define SUBSCRIBESERVERTHREAD_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsServerTask.h"
#include "sipXecsService/SipNonceDb.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipMessage;
class SipUserAgent;
class StatusServer;
class Notifier;
class PluginXmlParser;
class StatusPluginReference;
class SubscribeServerPluginBase;
class UtlHashMap;


class SubscribeServerThread : public OsServerTask
{
public:
    UtlBoolean initialize (
        SipUserAgent* userAgent,
        int defaultSubscribePeriod,
        const UtlString& minExpiresTime,
        const UtlString& defaultDomain,
        const UtlBoolean& useCredentialDB,
        const UtlString& realm,
        PluginXmlParser* pluginTable);

    SubscribeServerThread(StatusServer& statusServer);

    virtual ~SubscribeServerThread();

    /// Insert a row in the subscription DB
    UtlBoolean insertRow (
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
        const int& notifyCseq);

    /// Remove a row from the subscription DB
    void removeRow (
       const UtlString& to,
       const UtlString& from,
       const UtlString& callid,
       const int& subscribeCseq );

    /// Remove an error row from the subscription DB
    void removeErrorRow (
       const UtlString& to,
       const UtlString& from,
       const UtlString& callid );

    //==================================================================================

    typedef enum subcribeStatus
    {
       STATUS_SUCCESS = 0,      // successful - 200 response
       STATUS_LESS_THAN_MINEXPIRES, // Expires value too small - 423 response
       STATUS_INVALID_REQUEST,	// invalid request - 400 response
       STATUS_FORBIDDEN,	// forbidden - 403 response
       STATUS_NOT_FOUND,        // target not found - 404 response
       STATUS_INTERNAL_ERROR,   // internal error - 500 response
       STATUS_BAD_SUBSCRIPTION,  // renewal of non-existent subscription -
                                // 481 response
       STATUS_TO_BE_REMOVED,    // row exists but is to be removed once final NOTIFY sent
       STATUS_REMOVED           // successfully removed
    } SubscribeStatus;

    UtlBoolean handleMessage( OsMsg& eventMessage );
    
protected:
    StatusServer& mStatusServer;
    SipUserAgent* mpSipUserAgent;
    UtlBoolean mIsCredentialDB;
    UtlBoolean mIsStarted;
    UtlString mMinExpiresTimeStr;
    UtlString mRealm;
    int mDefaultSubscribePeriod;
    int mMinExpiresTimeint;
    int mDefaultDomainPort;
    UtlString mDefaultDomain;
    UtlString mDefaultDomainHostFQDN;
    UtlString mDefaultDomainHostIP;
    SipNonceDb mNonceDb;
    PluginXmlParser* mPluginTable;

    /// This semaphore mediates access to the subscription DB
    OsBSem mLock;

    //int addUserToDirectory(const int timeNow, const SipMessage* registerMessage);

    UtlBoolean isAuthenticated(
        const SipMessage* message,
        SipMessage *responseMessage,
        UtlString& authenticatedUser,
        UtlString& authenticatedRealm  );

    UtlBoolean isAuthorized(
        const SipMessage* message,
        SipMessage *responseMessage,
        StatusPluginReference* plugin = NULL );

    UtlBoolean isValidDomain(
        const SipMessage* message,
        SipMessage * responseMessage );

    // Process the message as a prospective database change.
    SubscribeStatus addSubscription(const int timeNow,
                                    const SipMessage* subscribeMessage, ///< request message
                                    const char* domain,
                                    const UtlString& eventTypeKey, ///< package name
                                    const UtlString& eventType, ///< package name
                                    const UtlString& eventId,   ///< event header id parameter (may be null)
                                    const UtlHashMap& eventParams,
                                    UtlString& newToTag,        ///< if not null, newly generated to-tag for response
                                    int& grantedExpiration      ///< if returns STATUS_SUCCESS, set to the granted expiration time
       );

    /// Remove the subscription
    SubscribeStatus removeSubscription(const SipMessage* subscribeMessage);

    /**
     *
     * @param sipMessage
     *
     * @return
     */
    int removeErrorSubscription(const SipMessage& sipMessage);

};
#endif // SUBSCRIBESERVERTHREAD_H

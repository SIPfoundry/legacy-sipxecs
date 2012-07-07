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
#include "os/OsStatus.h"
#include "os/OsDateTime.h"
#include "net/SipUserAgent.h"
#include "sipdb/ResultSet.h"
#include "sipdb/SubscribeDB.h"
#include "sipdb/EntityDB.h"
#include "statusserver/Notifier.h"
#include "statusserver/StatusServer.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
// STATIC INITIALIZERS

UtlString Notifier::sComponentKey ("component");
UtlString Notifier::sUriKey ("uri");
UtlString Notifier::sCallidKey ("callid");
UtlString Notifier::sContactKey ("contact");
UtlString Notifier::sExpiresKey ("expires");
UtlString Notifier::sSubscribecseqKey ("subscribecseq");
UtlString Notifier::sEventtypeKey ("eventtype");
UtlString Notifier::sIdKey ("id");
UtlString Notifier::sToKey ("toUri");
UtlString Notifier::sFromKey ("fromUri");
UtlString Notifier::sKeyKey ("key");
UtlString Notifier::sRecordrouteKey ("recordroute");
UtlString Notifier::sNotifycseqKey ("notifycseq");

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Notifier::Notifier(SipUserAgent* sipUserAgent)
{
    mpSipUserAgent = sipUserAgent;
    mpStaticSeq = 0;
}

Notifier::~Notifier()
{
    if( mpSipUserAgent )
    {
        mpSipUserAgent = NULL;
    }
}

/* ============================ PUBLIC ==================================== */

SipUserAgent* Notifier::getUserAgent()
{
    return(mpSipUserAgent);
}

void
Notifier::sendNotifyForSubscription (
    const char* key,
    const char* event,
    const SipMessage& subscribe,
    SipMessage& notify )
{
    // this is where we send back a single notify
    // rather than notifying all phones
    sendNotifyForeachSubscription (
        key,
        event,
        notify,
        &subscribe);
}

void 
SendTheNotify( SipMessage& notify,
               SipUserAgent* sipUserAgent,
               UtlString uri,
               UtlString contact,
               UtlString from,
               UtlString to,
               UtlString callid,
               int notifycseq,
               UtlString eventtype,
               UtlString id,
               UtlString subscriptionState,
               UtlString recordroute)
{
            // Make a copy of the message
            SipMessage notifyRequest(notify);

            // Set the NOTIFY request headers
            // swapping the to and from fields
            notifyRequest.setNotifyData (
                contact,        // uri (final destination where we send the message)
                from,           // fromField
                to,             // toField
                callid,         // callId
                notifycseq,     // already incremented
                eventtype,      // eventtype
                id,
                subscriptionState.data(),
                uri,            // contact
                recordroute);   // added the missing route field

            // Send the NOTIFY message via the user agent with
            // the incremented notify csequence value
            sipUserAgent->setUserAgentHeader( notifyRequest );
            sipUserAgent->send( notifyRequest );
}

void
Notifier::sendNotifyForeachSubscription (
    const char* key,
    const char* event,
    SipMessage& notify,
    const SipMessage* subscribe)
{
    SubscribeDB::Subscriptions subscriptions;

    int timeNow = (int)OsDateTime::getSecsSinceEpoch();

    // Get all subscriptions associated with this identity
    StatusServer* server = StatusServer::getInstance();
    SubscribeDB* subscribeDb = server->getSubscribeDb();
    subscribeDb->getUnexpiredSubscriptions(
           SUBSCRIPTION_COMPONENT_STATUS, key, event, timeNow, subscriptions );

    // Add the static configured contacts.
    UtlString userUri(key);
    UtlString eventType(event);
    UtlString userContact;
    UtlString userFromUri;
    UtlString userToUri;
    UtlString userCallid;
    EntityRecord entity;
    if (server->getEntityDb()->findByIdentity(std::string(key), entity))
    {
        std::vector<EntityRecord::StaticUserLoc> staticUserLoc = entity.staticUserLoc();
        if (staticUserLoc.size() > 0)
        {
            //staticUserLoc[0].event.c_str();
            userContact = staticUserLoc[0].contact.c_str();
            userFromUri = staticUserLoc[0].fromUri.c_str();
            userToUri = staticUserLoc[0].toUri.c_str();
            userCallid = staticUserLoc[0].callId.c_str();

             Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                           "Notifier::sendNotifyForeachSubscription configured contact %s", userUri.data());
             int notifycseq = 0;
             userCallid.append("-");
             userCallid.appendNumber((mpStaticSeq++ & 0xFFFF), "%04x");
             UtlString id("");
             UtlString subscriptionState("terminated");
             UtlString recordroute("");

             SendTheNotify(notify, mpSipUserAgent, userUri, userContact, userFromUri, userToUri, userCallid, notifycseq,
                           eventType, id, subscriptionState, recordroute);
        }
    }


    int numSubscriptions = subscriptions.size();

    if (  numSubscriptions > 0 )
    {
        Os::Logger::instance().log( FAC_SIP, PRI_INFO, "Notifier::sendNotifyForeachSubscription: "
                      " %d '%s' subscriptions for '%s'", numSubscriptions, event, key );

        UtlString subscribeCallid;
        if ( subscribe )
        {
           subscribe->getCallIdField(&subscribeCallid);
           Os::Logger::instance().log( FAC_SIP, PRI_INFO, "Notifier::sendNotifyForeachSubscription: "
                         " notify only '%s'", subscribeCallid.data() );
        }

        // There may be any number of subscriptions
        // for the same identity and event type!
        // send a notify to each
        for (SubscribeDB::Subscriptions::iterator iter = subscriptions.begin(); iter != subscriptions.end(); iter++ )
        {
            Subscription& record = *iter;

            UtlString uri        = record.uri().c_str();
            UtlString callid     = record.callId().c_str();
            UtlString contact    = record.contact().c_str();
            int timeNow = (int)OsDateTime::getSecsSinceEpoch();
            int expires          = record.expires() - timeNow;
            UtlString eventtype  = record.eventType().c_str();
            UtlString id         =  record.id().c_str();
            UtlString to         = record.toUri().c_str();
            UtlString from       = record.fromUri().c_str();
            if ( subscribe && subscribeCallid != callid  )
            {
               Os::Logger::instance().log( FAC_SIP, PRI_DEBUG, "Notifier::sendNotifyForeachSubscription: "
                             " skipping '%s'; notify only '%s'", callid.data(), subscribeCallid.data() );
               continue;
            }
            UtlString key        = record.key().c_str();
            UtlString recordroute= record.recordRoute().c_str();
            int notifycseq      = record.notifyCseq();

            // the recordRoute column in the database is optional
            // and can be set to null, the IMDB does not support null columns
            // so look to see if the field is set to "%" a special single character
            // reserved value we use to null columns in the IMDB
            if ( record.recordRoute().empty() )
            {
                recordroute.remove(0);
            }

            UtlString subscriptionState;
            int requestedExpires = -1;
            if ( subscribe )
            {
               subscribe->getExpiresField(&requestedExpires);
            }
            if ( requestedExpires == 0 )
            {
               subscriptionState = SIP_SUBSCRIPTION_TERMINATED;
               // the subscription should be removed from the database after this NOTIFY is sent
            }
            else
            {
               subscriptionState = SIP_SUBSCRIPTION_ACTIVE ";expires=";
               char expStr[16];
               sprintf(expStr, "%d", expires);
               subscriptionState.append(expStr);
            }

            // increment the outbound cseq sent with the notify
            // this will guarantee that duplicate messages are rejected
            notifycseq += 1;

            SendTheNotify(notify, mpSipUserAgent, uri, contact, to, from, callid, notifycseq, 
                          eventtype, id, subscriptionState, recordroute);


            // Update the Notify sequence number (CSeq) in the IMDB
            // (We must supply a dummy XML version number.)
            subscribeDb->updateNotifyUnexpiredSubscription (
               SUBSCRIPTION_COMPONENT_STATUS, to, from, callid,
               eventtype, id, timeNow, notifycseq, 0 );
        }
    }
    else
    {
       Os::Logger::instance().log( FAC_SIP, PRI_INFO, "Notifier::sendNotifyForeachSubscription: "
                     " no '%s' subscriptions for '%s'", event, key );
    }
}

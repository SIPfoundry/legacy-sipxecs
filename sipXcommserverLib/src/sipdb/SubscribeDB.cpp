/*
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */

#include <string>
#include <vector>
#include <mongo/client/connpool.h>
#include "sipdb/SubscribeDB.h"
#include "sipdb/SubscribeExpireThread.h"
#include "os/OsDateTime.h"

using namespace std;

const string SubscribeDB::NS("node.subscription");

void SubscribeDB::getAll(Subscriptions& subscriptions)
{
	mongo::BSONObj query;
	mongo::ScopedDbConnection conn(_info.getConnectionString());
	auto_ptr<mongo::DBClientCursor> pCursor = conn->query(_info.getNS(), query);
	while (pCursor.get() && pCursor->more()) {
		subscriptions.push_back(Subscription(pCursor->next()));
	}
	conn.done();
}

void SubscribeDB::upsert (
    const UtlString& component,
    const UtlString& uri,
    const UtlString& callId,
    const UtlString& contact,
    unsigned int expires,
    unsigned int subscribeCseq,
    const UtlString& eventTypeKey,
    const UtlString& eventType,
    const UtlString& id,
    const UtlString& toUri,
    const UtlString& fromUri,
    const UtlString& key,
    const UtlString& recordRoute,
    unsigned int notifyCseq,
    const UtlString& accept,
    unsigned int version)
{
    mongo::BSONObj query = BSON(
        "toUri" << toUri.str() <<
        "fromUri" << fromUri.str() <<
        "callId" << callId.str() <<
        "eventTypeKey" << eventTypeKey.str());

    mongo::BSONObj update = BSON("$set" << BSON(
        Subscription::component_fld() << component.str() <<
        Subscription::uri_fld() << uri.str() <<
        Subscription::callId_fld() << callId.str() <<
        Subscription::contact_fld() << contact.str() <<
        Subscription::expires_fld() << expires <<
        Subscription::subscribeCseq_fld() << subscribeCseq <<
        Subscription::eventTypeKey_fld() << eventTypeKey.str() <<
        Subscription::eventType_fld() << eventType.str() <<
        Subscription::id_fld() << id.str() <<
        Subscription::toUri_fld() << toUri.str() <<
        Subscription::fromUri_fld() << fromUri.str() <<
        Subscription::key_fld() << key.str() <<
        Subscription::recordRoute_fld() << recordRoute.str() <<
        Subscription::notifyCseq_fld() << notifyCseq <<
        Subscription::accept_fld() << accept.str() <<
        Subscription::version_fld() << version));

    mongo::ScopedDbConnection conn(_info.getConnectionString());
    conn->update(_info.getNS(), query, update, true, false);
    conn->ensureIndex("node.subscription",  BSON( "expires" << 1 ));
    conn->ensureIndex("node.subscription",  BSON( "key" << 1 ));
    conn->ensureIndex("node.subscription",  BSON( "toUri" << 1 ));
    conn.done();
}

//delete methods - delete a subscription session
void SubscribeDB::remove (
   const UtlString& component,
   const UtlString& to,
   const UtlString& from,
   const UtlString& callid,
   const int& subscribeCseq)
{
    mongo::BSONObj query = BSON(
        Subscription::toUri_fld() << to.str() <<
        Subscription::fromUri_fld() << from.str() <<
        Subscription::callId_fld() << callid.str() <<
        Subscription::subscribeCseq_fld() << BSON_LESS_THAN(subscribeCseq));

    mongo::ScopedDbConnection conn(_info.getConnectionString());
    conn->remove(_info.getNS(), query);
    conn.done();
}

void SubscribeDB::removeError (
    const UtlString& component,
    const UtlString& to,
    const UtlString& from,
    const UtlString& callid )
{
    mongo::BSONObj query = BSON(
        Subscription::toUri_fld() << to.str() <<
        Subscription::fromUri_fld() << from.str() <<
        Subscription::callId_fld() << callid.str());

    mongo::ScopedDbConnection conn(_info.getConnectionString());
    conn->remove(_info.getNS(), query);
    conn.done();
}

bool SubscribeDB::subscriptionExists (
   const UtlString& /*component*/,
   const UtlString& toUri,
   const UtlString& fromUri,
   const UtlString& callId,
   const int timeNow)
{

    mongo::BSONObj query = BSON(
              Subscription::toUri_fld() << toUri.str() <<
              Subscription::fromUri_fld() << fromUri.str() <<
              Subscription::callId_fld() << callId.str() <<
              Subscription::expires_fld() << BSON_GREATER_THAN_EQUAL(timeNow));

    mongo::ScopedDbConnection conn(_info.getConnectionString());
    auto_ptr<mongo::DBClientCursor> pCursor = conn->query(_info.getNS(), query);
    if (pCursor.get() && pCursor->more()) {
    	conn.done();
        return pCursor->itcount() > 0;
    }

    conn.done();
    return false;
}

//void SubscribeDB::removeRows(const UtlString& key)
//{
//    mongo::BSONObj query = BSON(Subscription::key_fld() << key.str());
//    mongo::ScopedDbConnection conn(_info.getConnectionString());
//    conn->remove(_info.getNS(), query);
//}

void SubscribeDB::removeExpired( const UtlString& component, const int timeNow )
{
    mongo::BSONObj query = BSON(
        Subscription::component_fld() << component.str() <<
        Subscription::expires_fld() << BSON_LESS_THAN(timeNow));
    mongo::ScopedDbConnection conn(_info.getConnectionString());
    conn->remove(_info.getNS(), query);
    conn.done();
}

void SubscribeDB::getUnexpiredSubscriptions (
    const UtlString& component,
    const UtlString& key,
    const UtlString& eventTypeKey,
    const int& timeNow,
    Subscriptions& subscriptions)
{
    removeExpired(component, timeNow);
    //query="key=",key,"and eventtypekey=",eventTypeKey;
     mongo::BSONObj query = BSON(
        Subscription::key_fld() << key.str() <<
        Subscription::eventTypeKey_fld() << eventTypeKey.str());

    mongo::ScopedDbConnection conn(_info.getConnectionString());
    auto_ptr<mongo::DBClientCursor> pCursor = conn->query(_info.getNS(), query);
    while (pCursor.get() && pCursor->more())
    {
        subscriptions.push_back(Subscription(pCursor->next()));
    }
    conn.done();
}

void SubscribeDB::getUnexpiredContactsFieldsContaining(
    UtlString& substringToMatch,
    const int& timeNow,
    std::vector<string>& matchingContactFields ) const
{
    mongo::BSONObj query = BSON(Subscription::expires_fld() << BSON_GREATER_THAN(timeNow));

    mongo::ScopedDbConnection conn(_info.getConnectionString());
    auto_ptr<mongo::DBClientCursor> pCursor = conn->query(_info.getNS(), query);
    while (pCursor.get() && pCursor->more())
    {
        string contact;
        mongo::BSONObj bsonObj = pCursor->next();
        if (bsonObj.hasField(Subscription::contact_fld()))
            contact = bsonObj.getStringField(Subscription::contact_fld());
        if (contact.find(substringToMatch.str()) != string::npos)
            matchingContactFields.push_back(contact);
    }
    conn.done();
}

void SubscribeDB::updateNotifyUnexpiredSubscription(
    const UtlString& component,
    const UtlString& to,
    const UtlString& from,
    const UtlString& callid,
    const UtlString& eventTypeKey,
    const UtlString& id,
    int timeNow,
    int updatedNotifyCseq,
    int version) const
{

    mongo::BSONObj query = BSON(
        Subscription::toUri_fld() << to.str() <<
        Subscription::callId_fld() << callid.str() <<
        Subscription::eventTypeKey_fld() << eventTypeKey.str() <<
        Subscription::id_fld() << id.str() );

    mongo::BSONObj update = BSON("$set" << BSON(
        Subscription::notifyCseq_fld() << updatedNotifyCseq <<
        Subscription::version_fld() << version));

    mongo::ScopedDbConnection conn(_info.getConnectionString());
    conn->update(_info.getNS(), query, update);
    conn->ensureIndex("node.subscription",  BSON( "expires" << 1 ));
    conn->ensureIndex("node.subscription",  BSON( "key" << 1 ));
    conn->ensureIndex("node.subscription",  BSON( "toUri" << 1 ));
    conn.done();
}

//void SubscribeDB::updateSubscribeUnexpiredSubscription (
//    const UtlString& component,
//    const UtlString& to,
//    const UtlString& from,
//    const UtlString& callid,
//    const UtlString& eventTypeKey,
//    const UtlString& id,
//    const int& timeNow,
//    const int& expires,
//    const int& updatedSubscribeCseq) const
//{
//    const_cast<SubscribeDB*>(this)->removeExpired(component, timeNow);
//
//    mongo::BSONObj query = BSON(
//        Subscription::toUri_fld() << to.str() <<
//        Subscription::fromUri_fld() << from.str() <<
//        Subscription::callId_fld() << callid.str() <<
//        Subscription::eventTypeKey_fld() << eventTypeKey.str() <<
//        Subscription::id_fld() << id.str() );
//
//    mongo::BSONObj update = BSON("$set" << BSON(
//        Subscription::expires_fld() << expires <<
//        Subscription::subscribeCseq_fld() << updatedSubscribeCseq));
//
//	mongo::ScopedDbConnection conn(_info.getConnectionString());
//    conn->update(_info.getNS(), query, update);
//
//
//    if (!ret)
//    {
//       // Add a new row.
//
//       // This call assumes that eventTypeKey (as set by the
//       // handler) is OK for use as the <eventtypekey>,
//       // and that the NOTIFY CSeq's will start at 1.  0 is used as
//       // the initial XML version.
//       ret = mDB.insertRow(
//          mComponent, requestUri, callId, contactEntry,
//          expires, subscribeCseq, eventTypeKey, eventType, "",
//          to, from, resourceId, route, 1, accept, 0);
//
//       if (!ret)
//       {
//          Os::Logger::instance().log(FAC_SIP, PRI_ERR,
//                        "SipPersistantSubscriptionMgr::addSubscription "
//                        "Could not update or insert record in database");
//       }
//    }
//
//}

void SubscribeDB::updateToTag(
   const UtlString& callid,
   const UtlString& fromtag,
   const UtlString& totag) const
{
    mongo::BSONObj query = BSON(Subscription::callId_fld() << callid.str());
    mongo::ScopedDbConnection conn(_info.getConnectionString());
    auto_ptr<mongo::DBClientCursor> pCursor = conn->query(_info.getNS(), query);
    while (pCursor.get() && pCursor->more())
    {
        mongo::BSONObj bsonObj = pCursor->next();
        if (bsonObj.hasField(Subscription::fromUri_fld()))
        {
            string fromUri = bsonObj.getStringField(Subscription::fromUri_fld());
            Url from_uri(fromUri.c_str(), FALSE);
            UtlString seen_tag;
            if (from_uri.getFieldParameter("tag", seen_tag) && seen_tag.compareTo(fromtag) == 0)
            {
                if (bsonObj.hasField(Subscription::toUri_fld()))
                {
                    string toUri = bsonObj.getStringField(Subscription::toUri_fld());
                    Url to_uri(toUri.c_str(), FALSE);
                    UtlString dummy;
                    if (!to_uri.getFieldParameter("tag", dummy))
                    {
                        to_uri.setFieldParameter("tag", totag);
                        to_uri.toString(dummy); // un-parse as name-addr

                        string oid = bsonObj.getStringField(Subscription::oid_fld());
                        mongo::BSONObj query = BSON(Subscription::oid_fld() << oid);
                        mongo::BSONObj update = BSON("$set" << BSON(Subscription::toUri_fld() << dummy.data()));
                        conn->update(_info.getNS(), query, update);
                        conn->ensureIndex("node.subscription",  BSON( "expires" << 1 ));
                        conn->ensureIndex("node.subscription",  BSON( "key" << 1 ));
			conn->ensureIndex("node.subscription",  BSON( "toUri" << 1 ));
                    }
                }
            }
        }
    }
    conn.done();
}

bool SubscribeDB::findFromAndTo(
   const UtlString& callid,
   const UtlString& fromtag,
   const UtlString& totag,
   UtlString& from,
   UtlString& to) const
{
    mongo::BSONObj query = BSON(Subscription::callId_fld() << callid.str());
    mongo::ScopedDbConnection conn(_info.getConnectionString());
    auto_ptr<mongo::DBClientCursor> pCursor = conn->query(_info.getNS(), query);
    while (pCursor.get() && pCursor->more())
    {
        Subscription row = pCursor->next();
        UtlBoolean r;
        UtlString seen_tag;

        // Get the tag on the URI in the "from" column.
        Url fromUri(row.fromUri().c_str(), FALSE);
        r = fromUri.getFieldParameter("tag", seen_tag);

        // If it matches...
        if (r && seen_tag.compareTo(fromtag) == 0)
        {
           // Get the tag on the URI in the "to" column.
           Url toUri(row.toUri().c_str(), FALSE);
           r = toUri.getFieldParameter("tag", seen_tag);

           // If it matches...
           if (r && seen_tag.compareTo(totag) == 0)
           {
              // We have found a match.  Record the full URIs.
              from = row.fromUri().c_str();
              to = row.toUri().c_str();
              conn.done();
              return true;
           }
        }
    }
    conn.done();
    return false;
}

int SubscribeDB::getMaxVersion(const UtlString& uri) const
{
    mongo::BSONObj query = BSON(Subscription::uri_fld() << uri.str());
    mongo::ScopedDbConnection conn(_info.getConnectionString());
    auto_ptr<mongo::DBClientCursor> pCursor = conn->query(_info.getNS(), query);
    unsigned int value = 0;
    while (pCursor.get() && pCursor->more())
    {
        Subscription row = pCursor->next();
        if (value < row.version())
            value = row.version();
    }
    conn.done();
    return value;
}

void SubscribeDB::removeAllExpired()
{
  int timeNow = (int) OsDateTime::getSecsSinceEpoch();
	mongo::BSONObj query = BSON(Subscription::expires_fld() << BSON_LESS_THAN_EQUAL(timeNow));
	mongo::ScopedDbConnection conn(_info.getConnectionString());
	conn->remove(_info.getNS(), query);
	conn.done();
}


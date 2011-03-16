/*
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
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

#include "sipdb/SubscribeDB.h"

std::string SubscribeDB::_defaultNamespace = "node.subscription";
const std::string& SubscribeDB::defaultNamespace()
{
    return SubscribeDB::_defaultNamespace;
}

MongoDB::Collection<SubscribeDB>& SubscribeDB::defaultCollection()
{
    static MongoDB::Collection<SubscribeDB> collection(SubscribeDB::_defaultNamespace);
    return collection;
}

SubscribeDB::SubscribeDB(
    MongoDB& db,
    const std::string& ns) :
    MongoDB::DBInterface(db, ns)
{
}

SubscribeDB::~SubscribeDB()
{
}

void SubscribeDB::getAllRows(Subscriptions& subscriptions)
{
  MongoDB::BSONObj query;
  std::string error;
  MongoDB::Cursor pCursor = _db.find(_ns, query, error);

  while (pCursor->more())
    subscriptions.push_back(Subscription(pCursor->next()));

  if (!error.empty())
  {
    SYSLOG_ERROR("mongoDB Exception: (SubscribeDB::getAllRows) - " << error);
  }
}

bool SubscribeDB::insertRow (
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
    MongoDB::BSONObj query = BSON(
              Subscription::toUri_fld() << toUri.str() <<
              Subscription::fromUri_fld() << fromUri.str() <<
              Subscription::callId_fld() << callId.str() <<
              Subscription::eventTypeKey_fld() << eventTypeKey.str() <<
              Subscription::id_fld() << id.str() <<
              Subscription::subscribeCseq_fld() << BSON_LESS_THAN(subscribeCseq));

    MongoDB::BSONObj update = BSON("$set" << BSON(
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

    std::string error;
    if (!_db.updateOrInsert(_ns, query, update, error))
    {
        SYSLOG_ERROR("mongoDB Exception: (SubscribeDB::insertRow) - " << error);
        return false;
    }
    return true;
}

//delete methods - delete a subscription session
void SubscribeDB::removeRow (
   const UtlString& component,
   const UtlString& to,
   const UtlString& from,
   const UtlString& callid,
   const int& subscribeCseq)
{
    MongoDB::BSONObj query = BSON(
        Subscription::toUri_fld() << to.str() <<
        Subscription::fromUri_fld() << from.str() <<
        Subscription::callId_fld() << callid.str() <<
        Subscription::subscribeCseq_fld() << BSON_LESS_THAN(subscribeCseq));

    std::string error;
    if (!_db.remove(_ns, query, error))
    {
        SYSLOG_ERROR("MongoDB Exception: (SubscribeDB::removeRow) - " << error);
    }
}

void SubscribeDB::removeErrorRow (
    const UtlString& component,
    const UtlString& to,
    const UtlString& from,
    const UtlString& callid )
{   
    MongoDB::BSONObj query = BSON(
        Subscription::toUri_fld() << to.str() <<
        Subscription::fromUri_fld() << from.str() <<
        Subscription::callId_fld() << callid.str());

    std::string error;
    if (!_db.remove(_ns, query, error))
    {
        SYSLOG_ERROR("MongoDB Exception: (SubscribeDB::removeErrorRow) - " << error);
    }
}

bool SubscribeDB::subscriptionExists (
   const UtlString& /*component*/,
   const UtlString& toUri,
   const UtlString& fromUri,
   const UtlString& callId,
   const int timeNow)
{

    MongoDB::BSONObj query = BSON(
              Subscription::toUri_fld() << toUri.str() <<
              Subscription::fromUri_fld() << fromUri.str() <<
              Subscription::callId_fld() << callId.str() <<
              Subscription::expires_fld() << BSON_GREATER_THAN_EQUAL(timeNow));

    std::string error;
    MongoDB::Cursor pCursor = _db.find(_ns, query, error);

    if (pCursor->more())
    {
        return pCursor->itcount() > 0;
    }

    if (!error.empty())
    {
        SYSLOG_ERROR("MongoDB Exception: (SubscribeDB::subscriptionExists) - " << error);
    }

    return false;
}

void SubscribeDB::removeRows(const UtlString& key)
{
    MongoDB::BSONObj query = BSON(Subscription::key_fld() << key.str());

    std::string error;
    if (!_db.remove(_ns, query, error))
    {
        SYSLOG_ERROR("MongoDB Exception: (SubscribeDB::removeRows) - " << error);
    }
}

void SubscribeDB::removeExpired( const UtlString& component, const int timeNow )
{
    MongoDB::BSONObj query = BSON(
        Subscription::component_fld() << component.str() <<
        Subscription::expires_fld() << BSON_LESS_THAN(timeNow));

    std::string error;
    if (!_db.remove(_ns, query, error))
    {
        SYSLOG_ERROR("MongoDB Exception: (SubscribeDB::removeExpired) - " << error);
    }
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
     MongoDB::BSONObj query = BSON(
        Subscription::key_fld() << key.str() <<
        Subscription::eventTypeKey_fld() << eventTypeKey.str());

    std::string error;
    MongoDB::Cursor pCursor = _db.find(_ns, query, error);

    if (!error.empty())
    {
        SYSLOG_ERROR("MongoDB Exception: (SubscribeDB::getUnexpiredSubscriptions) - " << error);
    }

    while (pCursor->more())
    {
        subscriptions.push_back(Subscription(pCursor->next()));
    }
}

void SubscribeDB::getUnexpiredContactsFieldsContaining(
    UtlString& substringToMatch,
    const int& timeNow,
    std::vector<std::string>& matchingContactFields ) const
{
    MongoDB::BSONObj query = BSON(Subscription::expires_fld() << BSON_LESS_THAN(timeNow));

    std::string error;
    MongoDB::Cursor pCursor = _db.find(_ns, query, error);

    if (!error.empty())
    {
        SYSLOG_ERROR("MongoDB Exception: (SubscribeDB::getUnexpiredSubscriptions) - " << error);
    }

    while (pCursor->more())
    {
        std::string contact;
        MongoDB::BSONObj bsonObj = pCursor->next();
        if (bsonObj.hasField(Subscription::contact_fld()))
            contact = bsonObj.getStringField(Subscription::contact_fld());
        if (contact.find(substringToMatch.str()) != std::string::npos)
            matchingContactFields.push_back(contact);
    }
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
    int version ) const
{

    MongoDB::BSONObj query = BSON(
        Subscription::toUri_fld() << to.str() <<
        Subscription::callId_fld() << callid.str() <<
        Subscription::eventTypeKey_fld() << eventTypeKey.str() <<
        Subscription::id_fld() << id.str() );

    MongoDB::BSONObj update = BSON("$set" << BSON(
        Subscription::notifyCseq_fld() << updatedNotifyCseq <<
        Subscription::version_fld() << version));
    

    std::string error;
    if (!_db.update(_ns, query, update, error))
    {
        SYSLOG_ERROR("MongoDB Exception: (SubscribeDB::updateNotifyUnexpiredSubscription) - " << error);
    }
}

bool SubscribeDB::updateSubscribeUnexpiredSubscription (
    const UtlString& component,
    const UtlString& to,
    const UtlString& from,
    const UtlString& callid,
    const UtlString& eventTypeKey,
    const UtlString& id,
    const int& timeNow,
    const int& expires,
    const int& updatedSubscribeCseq) const
{
    const_cast<SubscribeDB*>(this)->removeExpired(component, timeNow);

    MongoDB::BSONObj query = BSON(
        Subscription::toUri_fld() << to.str() <<
        Subscription::fromUri_fld() << from.str() <<
        Subscription::callId_fld() << callid.str() <<
        Subscription::eventTypeKey_fld() << eventTypeKey.str() <<
        Subscription::id_fld() << id.str() );

    MongoDB::BSONObj update = BSON("$set" << BSON(
        Subscription::expires_fld() << expires <<
        Subscription::subscribeCseq_fld() << updatedSubscribeCseq));


    std::string error;
    if (!_db.update(_ns, query, update, error))
    {
        SYSLOG_ERROR("MongoDB Exception: (SubscribeDB::updateSubscribeUnexpiredSubscription) - " << error);
        return false;
    }

    return true;;
}

void SubscribeDB::updateToTag(
   const UtlString& callid,
   const UtlString& fromtag,
   const UtlString& totag) const
{
    MongoDB::BSONObj query = BSON(Subscription::callId_fld() << callid.str());

    std::string error;
    MongoDB::Cursor pCursor = _db.find(_ns, query, error);

    if (!error.empty())
    {
        SYSLOG_ERROR("MongoDB Exception: (SubscribeDB::getUnexpiredSubscriptions) - " << error);
    }

    while (pCursor->more())
    {
        MongoDB::BSONObj bsonObj = pCursor->next();
        if (bsonObj.hasField(Subscription::fromUri_fld()))
        {
            std::string fromUri = bsonObj.getStringField(Subscription::fromUri_fld());
            Url from_uri(fromUri.c_str(), FALSE);
            UtlString seen_tag;
            if (from_uri.getFieldParameter("tag", seen_tag) && seen_tag.compareTo(fromtag) == 0)
            {
                if (bsonObj.hasField(Subscription::toUri_fld()))
                {
                    std::string toUri = bsonObj.getStringField(Subscription::toUri_fld());
                    Url to_uri(toUri.c_str(), FALSE);
                    UtlString dummy;
                    if (!to_uri.getFieldParameter("tag", dummy))
                    {
                        to_uri.setFieldParameter("tag", totag);
                        to_uri.toString(dummy); // un-parse as name-addr

                        std::string oid = bsonObj.getStringField(Subscription::oid_fld());
                        MongoDB::BSONObj query = BSON(Subscription::oid_fld() << oid);
                        MongoDB::BSONObj update = BSON("$set" << BSON(Subscription::toUri_fld() << dummy.data()));

                        std::string error;
                        if (!_db.update(_ns, query, update, error))
                        {
                            SYSLOG_ERROR("MongoDB Exception: (SubscribeDB::updateToTag) - " << error);
                        }
                    }
                }
            }
        }
    }
}

bool SubscribeDB::findFromAndTo(
   const UtlString& callid,
   const UtlString& fromtag,
   const UtlString& totag,
   UtlString& from,
   UtlString& to) const
{
    MongoDB::BSONObj query = BSON(Subscription::callId_fld() << callid.str());

    std::string error;
    MongoDB::Cursor pCursor = _db.find(_ns, query, error);

    if (!error.empty())
    {
        SYSLOG_ERROR("MongoDB Exception: (SubscribeDB::findFromAndTo) - " << error);
    }

    while (pCursor->more())
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
              return true;
           }
        }
    }
    return false;
}

int SubscribeDB::getMaxVersion(const UtlString& uri) const
{
    MongoDB::BSONObj query = BSON(Subscription::uri_fld() << uri.str());

    std::string error;
    MongoDB::Cursor pCursor = _db.find(_ns, query, error);

    if (!error.empty())
    {
        SYSLOG_ERROR("MongoDB Exception: (SubscribeDB::getMaxVersion) - " << error);
    }

    unsigned int value = 0;
    while (pCursor->more())
    {
        Subscription row = pCursor->next();
        if (value < row.version())
            value = row.version();
    }
    return value;
}



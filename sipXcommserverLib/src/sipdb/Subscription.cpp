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

#include "sipdb/Subscription.h"

using namespace std;

const char* Subscription::oid_fld(){ static std::string fld = "_id"; return fld.c_str(); }
const char* Subscription::component_fld(){ static std::string fld = "component"; return fld.c_str(); }
const char* Subscription::uri_fld(){ static std::string fld = "uri"; return fld.c_str(); }
const char* Subscription::callId_fld(){ static std::string fld = "callId"; return fld.c_str(); }
const char* Subscription::contact_fld(){ static std::string fld = "contact"; return fld.c_str(); }
const char* Subscription::notifyCseq_fld(){ static std::string fld = "notifyCseq"; return fld.c_str(); }
const char* Subscription::subscribeCseq_fld(){ static std::string fld = "subscribeCseq"; return fld.c_str(); }
const char* Subscription::eventTypeKey_fld(){ static std::string fld = "eventTypeKey"; return fld.c_str(); }
const char* Subscription::eventType_fld(){ static std::string fld = "eventType"; return fld.c_str(); }
const char* Subscription::id_fld(){ static std::string fld = "id"; return fld.c_str(); }
const char* Subscription::toUri_fld(){ static std::string fld = "toUri"; return fld.c_str(); }
const char* Subscription::fromUri_fld(){ static std::string fld = "fromUri"; return fld.c_str(); }
const char* Subscription::key_fld(){ static std::string fld = "key"; return fld.c_str(); }
const char* Subscription::recordRoute_fld(){ static std::string fld = "recordRoute"; return fld.c_str(); }
const char* Subscription::accept_fld(){ static std::string fld = "accept"; return fld.c_str(); }
const char* Subscription::file_fld(){ static std::string fld = "file"; return fld.c_str(); }
const char* Subscription::version_fld(){ static std::string fld = "version"; return fld.c_str(); }
const char* Subscription::expires_fld(){ static std::string fld = "expires"; return fld.c_str(); }

Subscription::Subscription() :
    _notifyCseq(0),
    _subscribeCseq(0),
    _version(0),
    _expires(0)
{
}

Subscription::Subscription(const Subscription& subscription)
{
    _oid = subscription._oid;
    _component = subscription._component;
    _uri = subscription._uri;
    _callId = subscription._callId;
    _contact = subscription._contact;
    _notifyCseq = subscription._notifyCseq;
    _subscribeCseq = subscription._subscribeCseq;
    _eventTypeKey = subscription._eventTypeKey;
    _eventType = subscription._eventType;
    _id = subscription._id;
    _toUri = subscription._toUri;
    _fromUri = subscription._fromUri;
    _key = subscription._key;
    _recordRoute = subscription._recordRoute;
    _accept = subscription._accept;
    _version = subscription._version;
    _expires = subscription._expires;
}

Subscription::Subscription(
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
    _component = component.str();
    _uri = uri.str();
    _callId = callId.str();
    _contact = contact.str();
    _notifyCseq = notifyCseq;
    _subscribeCseq = subscribeCseq;
    _eventTypeKey = eventTypeKey;
    _eventType = eventType;
    _id = id;
    _toUri = toUri;
    _fromUri = fromUri;
    _key = key;
    _recordRoute = recordRoute;
    _accept = accept;
    _version = version;
    _expires = expires;
}

Subscription::Subscription(const mongo::BSONObj& bson)
{
    operator=(bson);
}

Subscription::~Subscription()
{
}

Subscription& Subscription::operator=(const Subscription& subscription)
{
    Subscription clonable(subscription);
    swap(clonable);
    return *this;
}

Subscription& Subscription::operator=(const mongo::BSONObj& bsonObj)
{
    //For Subscription DB object id is of type mongo::OID and not a std::string
    mongo::BSONElement _id_field;
    if (true == bsonObj.getObjectID(_id_field))
    {
		_id_field.Val(_oid);
    }

	if (bsonObj.hasField(Subscription::component_fld()))
		_component = bsonObj.getStringField(Subscription::component_fld());

	if (bsonObj.hasField(Subscription::uri_fld()))
		_uri = bsonObj.getStringField(Subscription::uri_fld());

	if (bsonObj.hasField(Subscription::callId_fld()))
		_callId = bsonObj.getStringField(Subscription::callId_fld());

	if (bsonObj.hasField(Subscription::contact_fld()))
		_contact = bsonObj.getStringField(Subscription::contact_fld());

	if (bsonObj.hasField(Subscription::notifyCseq_fld()))
		_notifyCseq = bsonObj.getIntField(Subscription::notifyCseq_fld());

	if (bsonObj.hasField(Subscription::subscribeCseq_fld()))
		_subscribeCseq = bsonObj.getIntField(Subscription::subscribeCseq_fld());

	if (bsonObj.hasField(Subscription::eventTypeKey_fld()))
		_eventTypeKey = bsonObj.getStringField(Subscription::eventTypeKey_fld());

	if (bsonObj.hasField(Subscription::eventType_fld()))
		_eventType = bsonObj.getStringField(Subscription::eventType_fld());

	if (bsonObj.hasField(Subscription::id_fld()))
		_id = bsonObj.getStringField(Subscription::id_fld());

	if (bsonObj.hasField(Subscription::toUri_fld()))
		_toUri = bsonObj.getStringField(Subscription::toUri_fld());

	if (bsonObj.hasField(Subscription::fromUri_fld()))
		_fromUri = bsonObj.getStringField(Subscription::fromUri_fld());

	if (bsonObj.hasField(Subscription::key_fld()))
		_key = bsonObj.getStringField(Subscription::key_fld());

	if (bsonObj.hasField(Subscription::recordRoute_fld()))
		_recordRoute = bsonObj.getStringField(Subscription::recordRoute_fld());

	if (bsonObj.hasField(Subscription::accept_fld()))
		_accept = bsonObj.getStringField(Subscription::accept_fld());

	if (bsonObj.hasField(Subscription::file_fld()))
		_file = bsonObj.getStringField(Subscription::file_fld());

	if (bsonObj.hasField(Subscription::version_fld()))
		_version = bsonObj.getIntField(Subscription::version_fld());

	if (bsonObj.hasField(Subscription::expires_fld()))
		_expires = bsonObj.getIntField(Subscription::expires_fld());

    return *this;
}

void Subscription::swap_mongo_oid(mongo::OID& oid_src, mongo::OID& oid_dest)
{
    string oid_src_str = oid_src.str();
    string oid_dest_str = oid_dest.str();

    oid_src.init(oid_dest_str);
    oid_dest.init(oid_src_str);
}

void Subscription::swap(Subscription& subscription)
{
    swap_mongo_oid(_oid, subscription._oid);
    std::swap(_component, subscription._component);
    std::swap(_uri, subscription._uri);
    std::swap(_callId, subscription._callId);
    std::swap(_contact, subscription._contact);
    std::swap(_notifyCseq, subscription._notifyCseq);
    std::swap(_subscribeCseq, subscription._subscribeCseq);
    std::swap(_eventTypeKey, subscription._eventTypeKey);
    std::swap(_eventType, subscription._eventType);
    std::swap(_id, subscription._id);
    std::swap(_toUri, subscription._toUri);
    std::swap(_fromUri, subscription._fromUri);
    std::swap(_key, subscription._key);
    std::swap(_recordRoute, subscription._recordRoute);
    std::swap(_accept, subscription._accept);
    std::swap(_version, subscription._version);
    std::swap(_expires, subscription._expires);
}

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

#ifndef SUBSCRIBERECORD_H
#define	SUBSCRIBERECORD_H


#include <string>
#include <boost/shared_ptr.hpp>
#include "sipdb/MongoDB.h"
#include "utl/UtlString.h"


class Subscription
{
public:
    Subscription();
    Subscription(const Subscription& subscription);
    Subscription(const mongo::BSONObj& bson);
    Subscription(
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
        unsigned int version);

    ~Subscription();
    Subscription& operator=(const Subscription& subscription);
    Subscription& operator=(const mongo::BSONObj& bson);
    void swap(Subscription& subscription);
    std::string& oid();
    std::string& component();
    std::string& uri();
    std::string& callId();
    std::string& contact();
    std::string& eventTypeKey();
    std::string& eventType();
    std::string& id();
    std::string& toUri();
    std::string& fromUri();
    std::string& key();
    std::string& recordRoute();
    std::string& accept();
    std::string& file();
    unsigned int notifyCseq();
    unsigned int subscribeCseq();
    unsigned int version();  
    unsigned int expires();
    void getMongoOID(mongo::OID& oid) const;

    static const char* oid_fld();
    static const char* component_fld();
    static const char* uri_fld();
    static const char* callId_fld();
    static const char* contact_fld();
    static const char* notifyCseq_fld();
    static const char* subscribeCseq_fld();
    static const char* eventTypeKey_fld();
    static const char* eventType_fld();
    static const char* id_fld();
    static const char* toUri_fld();
    static const char* fromUri_fld();
    static const char* key_fld();
    static const char* recordRoute_fld();
    static const char* accept_fld();
    static const char* file_fld();
    static const char* version_fld();
    static const char* expires_fld();
    static const char* shardId_fld();

private:
    std::string  _oid;
    std::string _component;
    std::string _uri;
    std::string _callId;
    std::string _contact;
    std::string _eventTypeKey;
    std::string _eventType;
    std::string _id; // id param from event header
    std::string _toUri;
    std::string _fromUri;
    std::string _key;
    std::string _recordRoute;
    std::string _accept;
    std::string _file;
    unsigned int _notifyCseq;
    unsigned int _subscribeCseq;
    unsigned int _version;              // Version no inside generated XML.
    unsigned int _expires; // Absolute expiration time secs since 1/1/1970
};

//
// Inlines
//

inline std::string& Subscription::oid()
{
    return _oid;
}

inline void Subscription::getMongoOID(mongo::OID& oid) const
{
  oid.init(_oid);
}

inline std::string& Subscription::component()
{
    return _component;
}

inline std::string& Subscription::uri()
{
    return _uri;
}

inline std::string& Subscription::callId()
{
    return _callId;
}

inline std::string& Subscription::contact()
{
    return _contact;
}

inline std::string& Subscription::eventTypeKey()
{
    return _eventTypeKey;
}

inline std::string& Subscription::eventType()
{
    return _eventType;
}

inline std::string& Subscription::id()
{
    return _id;
}

inline std::string& Subscription::toUri()
{
    return _toUri;
}

inline std::string& Subscription::fromUri()
{
    return _fromUri;
}

inline std::string& Subscription::key()
{
    return _key;
}

inline std::string& Subscription::recordRoute()
{
    return _recordRoute;
}

inline std::string& Subscription::accept()
{
    return _accept;
}

inline unsigned int Subscription::version()
{
    return _version;
}

inline unsigned int Subscription::expires()
{
    return _expires;
}

inline std::string& Subscription::file()
{
    return _file;
}

inline unsigned int Subscription::notifyCseq()
{
    return _notifyCseq;
}

inline unsigned int Subscription::subscribeCseq()
{
    return _subscribeCseq;
}

#endif	/* SUBSCRIBERECORD_H */


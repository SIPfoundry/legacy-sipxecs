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

#ifndef SUBSCRIBEDB_H
#define	SUBSCRIBEDB_H

#include "utl/UtlString.h"
#include "net/Url.h"
#include "sipdb/Subscription.h"


// "component" value for sipXpublisher a/k/a the Status Server
#ifndef SUBSCRIPTION_COMPONENT_STATUS
#define SUBSCRIPTION_COMPONENT_STATUS "status"
#endif
// "component" value for sipXrls a/k/a the Resource List Server
#ifndef SUBSCRIPTION_COMPONENT_RLS
#define SUBSCRIPTION_COMPONENT_RLS "rls"
#endif
// "component" value for sipXregistry "reg" event subscriptions
#ifndef SUBSCRIPTION_COMPONENT_REG
#define SUBSCRIPTION_COMPONENT_REG "reg"
#endif
// "component" value for sipXpresence a/k/a the Presence Server
#ifndef SUBSCRIPTION_COMPONENT_PRESENCE
#define SUBSCRIPTION_COMPONENT_PRESENCE "presence"
#endif
// "component" value for sipXpark a/k/a the Park Server
#ifndef SUBSCRIPTION_COMPONENT_PARK
#define SUBSCRIPTION_COMPONENT_PARK "park"
#endif
// "component" value for sipXsaa a/k/a the Shared Appearance Agent
#ifndef SUBSCRIPTION_COMPONENT_SAA
#define SUBSCRIPTION_COMPONENT_SAA "saa"
#endif

class SubscribeDB : public MongoDB::DBInterface
{
public:
    typedef std::vector<Subscription> Subscriptions;
    typedef MongoDB::Collection<SubscribeDB> Collection;
    typedef boost::shared_ptr<SubscribeDB::Collection> Ptr;
    SubscribeDB(
        MongoDB& db,
        const std::string& ns = SubscribeDB::_defaultNamespace);

    ~SubscribeDB();

    void getAllRows(Subscriptions& subscriptions);

    bool insertRow (
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

    //delete methods - delete a subscription session
    void removeRow (
       const UtlString& component,
       const UtlString& to,
       const UtlString& from,
       const UtlString& callid,
       const int& subscribeCseq );

    void removeErrorRow (
       const UtlString& component,
       const UtlString& to,
       const UtlString& from,
       const UtlString& callid );

    bool subscriptionExists (
       const UtlString& component,
       const UtlString& to,
       const UtlString& from,
       const UtlString& callid,
       const int timeNow);

    void removeRows (const UtlString& key);

    void removeExpired( const UtlString& component, const int timeNow );

    void getUnexpiredSubscriptions (
        const UtlString& component,
        const UtlString& key,
        const UtlString& eventTypeKey,
        const int& timeNow,
        Subscriptions& subscriptions);

    void getUnexpiredContactsFieldsContaining(
        UtlString& substringToMatch,
        const int& timeNow,
        std::vector<std::string>& matchingContactFields ) const;

    void updateNotifyUnexpiredSubscription (
        const UtlString& component,
        const UtlString& to,
        const UtlString& from,
        const UtlString& callid,
        const UtlString& eventTypeKey,
        const UtlString& id,
        int timeNow,
        int updatedNotifyCseq,
        int version ) const;

    bool updateSubscribeUnexpiredSubscription (
        const UtlString& component,
        const UtlString& to,
        const UtlString& from,
        const UtlString& callid,
        const UtlString& eventTypeKey,
        const UtlString& id,
        const int& timeNow,
        const int& expires,
        const int& updatedSubscribeCseq) const;

    void updateToTag(
       const UtlString& callid,
       const UtlString& fromtag,
       const UtlString& totag) const;

    bool findFromAndTo(
       const UtlString& callid,
       const UtlString& fromtag,
       const UtlString& totag,
       UtlString& from,
       UtlString& to) const;

    int getMaxVersion(const UtlString& uri) const;

    static const std::string& defaultNamespace();

    static MongoDB::Collection<SubscribeDB>& defaultCollection();

private:
    static std::string _defaultNamespace;
};

//
// Inlines
//

#endif	/* SUBSCRIBEDB_H */


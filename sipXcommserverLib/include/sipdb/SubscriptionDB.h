//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef SUBSCRIPTIONDB_H
#define SUBSCRIPTIONDB_H

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMutex.h"

// DEFINES

// "component" value for sipXpublisher a/k/a the Status Server
#define SUBSCRIPTION_COMPONENT_STATUS "status"
// "component" value for sipXrls a/k/a the Resource List Server
#define SUBSCRIPTION_COMPONENT_RLS "rls"
// "component" value for sipXregistry "reg" event subscriptions
#define SUBSCRIPTION_COMPONENT_REG "reg"
// "component" value for sipXpresence a/k/a the Presence Server
#define SUBSCRIPTION_COMPONENT_PRESENCE "presence"
// "component" value for sipXpark a/k/a the Park Server
#define SUBSCRIPTION_COMPONENT_PARK "park"
// "component" value for sipXsaa a/k/a the Shared Appearance Agent
#define SUBSCRIPTION_COMPONENT_SAA "saa"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class dbDatabase;
class dbFieldDescriptor;
class UtlHashMap;
class TiXmlNode;
class ResultSet;
class UtlSList;

class SubscriptionDB
{
public:
    // Singleton Accessor
    static SubscriptionDB* getInstance(
        const UtlString& name = "subscription" );

    /// releaseInstance - cleans up the singleton (for use at exit)
    static void releaseInstance();

    //serialize methods
    OsStatus store();

    /// Count rows in table
    int getRowCount () const;

    //set methods
    UtlBoolean insertRow (
        const UtlString& component,
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
        const int& notifyCseq,
        const UtlString& accept,
        const int& version );

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

    /** Return true if a subscription with the specified identifiers exists
     *  (and has not yet expired). */
    UtlBoolean subscriptionExists (
       const UtlString& component,
       const UtlString& to,
       const UtlString& from,
       const UtlString& callid,
       const int timeNow
       );

    void removeRows ( const UtlString& uri );

    void removeAllRows ();

    /// Clean out any expired rows
    void removeExpired( const UtlString& component,
                        const int timeNow );

    // utility method for dumping all rows
    void getAllRows ( ResultSet& rResultSet ) const;

    void getUnexpiredSubscriptions (
        const UtlString& component,
        const UtlString& key,
        const UtlString& eventType,
        const int& timeNow,
        ResultSet& rResultSet );

    /// Return a list of contact fields that are unexpired and contain a specified substring.
    /// The caller is responsible for de-allocating the memory for the entries contained in the
    /// list.
    void getUnexpiredContactsFieldsContaining ( UtlString& substringToMatch
                                               ,const int& timeNow
                                               ,UtlSList& matchingContactFields ) const;

    // Updates the XML version ('version') and NOTIFY cseq ('updatedNotifyCseq')
    // in the subscription DB.
    // Does not have event and event-id parameters, because all subscriptions
    // in a dialog share the same CSeq value.
    void updateNotifyUnexpiredSubscription (
        const UtlString& component,
        const UtlString& to,
        const UtlString& from,
        const UtlString& callid,
        const UtlString& eventType,
        const UtlString& id,
        int timeNow,
        int updatedNotifyCseq,
        int version ) const;

    // Returns true if any rows are updated.
    UtlBoolean updateSubscribeUnexpiredSubscription (
        const UtlString& component,
        const UtlString& to,
        const UtlString& from,
        const UtlString& callid,
        const UtlString& eventType,
        const UtlString& id,
        const int& timeNow,
        const int& expires,
        const int& updatedSubscribeCseq) const;

    // Update the to-tag of entries containing a call-id and from-tag.
    void updateToTag(
       const UtlString& callid,
       const UtlString& fromtag,
       const UtlString& totag
       ) const;

    // Find the full From and To values given from- and to-tags.
    // Returns true if values are found.
    UtlBoolean findFromAndTo(
       const UtlString& callid,
       const UtlString& fromtag,
       const UtlString& totag,
       UtlString& from,
       UtlString& to) const;

    // Get the maximum of the <version> values whose <uri> matches
    // 'uri', or 0 if there are none.
    int getMaxVersion(
       const UtlString& uri) const;

    // ResultSet column Keys
    static const UtlString gComponentKey;
    static const UtlString gUriKey;
    static const UtlString gCallidKey;
    static const UtlString gContactKey;
    static const UtlString gNotifycseqKey;
    static const UtlString gSubscribecseqKey;
    static const UtlString gExpiresKey;
    static const UtlString gEventtypekeyKey; // sic
    static const UtlString gEventtypeKey;
    static const UtlString gIdKey;
    static const UtlString gToKey;
    static const UtlString gFromKey;
    static const UtlString gFileKey;
    static const UtlString gKeyKey;
    static const UtlString gRecordrouteKey;
    static const UtlString gAcceptKey;
    static const UtlString gVersionKey;
    static const UtlString sComponentStatus;
    static const UtlString sAcceptSimpleMessage;

    static const UtlString nullString;

    // The 'type' attribute of the top-level 'items' element.
    static const UtlString sType;

    // The XML namespace of the top-level 'items' element.
    static const UtlString sXmlNamespace;

    // Determine if the table is loaded from xml file
    bool isLoaded();

protected:
    // this is implicit now
    OsStatus load();

    // Singleton Constructor is private
    SubscriptionDB( const UtlString& name );

    // Added this to make load and store code identical
    // in all database implementations, One step closer
    // to a template version of the code
    UtlBoolean insertRow ( const UtlHashMap& nvPairs );

    /// Clean out any expired rows
    //  To be called by public methods.
    void removeExpiredInternal( const UtlString& component,
                                const int timeNow );

    // There is only one singleton in this design
    static SubscriptionDB* spInstance;

    // Fast DB instance
    static dbDatabase* spDBInstance;

    // Singleton and Serialization mutex
    static OsMutex sLockMutex;

    // Fast DB instance
    dbDatabase* m_pFastDB;

    // the persistent filename for loading/saving
    UtlString mDatabaseName;

    // boolean indicating table is loaded
    bool mTableLoaded;

private:
    virtual ~SubscriptionDB();

};

#endif //SUBSCRIPTIONDB_H

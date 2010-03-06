//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SUBSCRIPTIONROW_H
#define SUBSCRIPTIONROW_H
// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "fastdb/fastdb.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * The Subscription Base Schema
 */
class SubscriptionRow
{
public:
    // Identifies the component which is the server for this subscription.
    // Values are #define'd in SubscriptionDB.h.
    const char* component;
    const char* uri;
    const char* callid;
    const char* contact;
    int4 notifycseq;
    int4 subscribecseq;
    int4 expires; // Absolute expiration time secs since 1/1/1970
    const char* eventtypekey;
    const char* eventtype;
    const char* id; // id param from event header
    const char* toUri;
    const char* fromUri;
    const char* key;
    const char* recordroute;
    const char* accept;
    int4 version;              // Version no inside generated XML.

    TYPE_DESCRIPTOR (
      ( KEY(component, HASHED),
        KEY(toUri, HASHED),
        KEY(fromUri, HASHED),
        KEY(callid, HASHED),
        KEY(eventtype, HASHED),
        KEY(eventtypekey, HASHED),
        KEY(id, HASHED),
        KEY(key, HASHED),
        FIELD(subscribecseq),
        FIELD(uri),
        FIELD(contact),
        FIELD(expires),
        FIELD(recordroute),
        FIELD(notifycseq),
        FIELD(accept),
        FIELD(version) )
    );
};

#endif // SUBSCRIPTIONROW_H

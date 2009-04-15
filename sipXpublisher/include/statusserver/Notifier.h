// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////


#ifndef NOTIFIER_H
#define NOTIFIER_H

// SYSTEM INCLUDES


// APPLICATION INCLUDES
#include "net/SipMessage.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsServerTask;
class SipUserAgent;
class SubscriptionDB;

class Notifier 
{
public:
	Notifier(SipUserAgent* sipUserAgent);

	~Notifier();

    void sendNotifyForeachSubscription (
        const char* key,
        const char* event,
        SipMessage& notify );

    void sendNotifyForSubscription (
        const char* key,
        const char* event,
        SipMessage& notify );

    SipUserAgent* getUserAgent();

private:
    SipUserAgent*   mpSipUserAgent;
    SubscriptionDB* mpSubscriptionDB;

    static UtlString sComponentKey;
    static UtlString sUriKey;
    static UtlString sCallidKey;
    static UtlString sContactKey;
    static UtlString sExpiresKey;
    static UtlString sSubscribecseqKey;
    static UtlString sEventtypeKey;
    static UtlString sIdKey;
    static UtlString sToKey;
    static UtlString sFromKey;
    static UtlString sFileKey;
    static UtlString sKeyKey;
    static UtlString sRecordrouteKey;
    static UtlString sNotifycseqKey;

};

#endif // NOTIFIER_H

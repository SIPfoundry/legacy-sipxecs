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

class Notifier
{
public:
	Notifier(SipUserAgent* sipUserAgent);

	~Notifier();

    /// Send a NOTIFY on each subscription for the specified key and event.
    /** If a SUBSCRIBE message pointer is provided, then the NOTIFY is only sent
     *  to the subscribing phone.  NOTIFY content must be provided in the notify message.
     */
    void sendNotifyForeachSubscription (
        const char* key,
        const char* event,
        SipMessage& notify,
        const SipMessage* subscribe = NULL);

    void sendNotifyForSubscription (
        const char* key,
        const char* event,
        const SipMessage& subscribe,
        SipMessage& notify );

    SipUserAgent* getUserAgent();

private:
    SipUserAgent*   mpSipUserAgent;
    int             mpStaticSeq;

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

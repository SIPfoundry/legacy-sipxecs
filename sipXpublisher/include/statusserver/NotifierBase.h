//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef NOTIFIERBASE_H
#define NOTIFIERBASE_H

// SYSTEM INCLUDES
// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipMessage;

class NotifierBase
{
public:
    virtual void sendNotifyForeachSubscription(
        const char* key,
        const char* eventType,
        SipMessage& notify) = 0;

    virtual void sendNotifyForSubscription(
        const char* key,
        const char* eventType,
        SipMessage& notify) = 0;
};

#endif // NOTIFIERBASE_H

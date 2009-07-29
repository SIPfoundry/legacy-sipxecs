//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _SipXMessageObserver_h_
#define _SipXMessageObserver_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsServerTask.h"
#include "net/SipMessage.h"
#include "net/SipMessageEvent.h"


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS

/**
 *  Class that is an OsServerTask, and has a message queue that observes SIP messages.
 *  For example, it is used for looking for message responses, like the INFO response.
 */
class SipXMessageObserver : public OsServerTask
{
public:
/* ============================ CREATORS ================================== */

    SipXMessageObserver(void);
    virtual ~SipXMessageObserver(void);

/* ============================ MANIPULATORS ============================== */

    /**
     * Implementation of OsServerTask's pure virtual method
     */
    UtlBoolean handleMessage(OsMsg& rMsg);

    /**
     * FOR TEST PURPOSES ONLY - a response code to send back to the client
     */
    void setTestResponseCode(int code) { mTestResponseCode = code; }


private:
    bool handleIncomingInfoMessage(SipMessage* pMessage);
    bool handleIncomingInfoStatus(SipMessage* pMessage);
    /**
     * Special response code - for test purposes only.
     */
    int mTestResponseCode;

};

#endif

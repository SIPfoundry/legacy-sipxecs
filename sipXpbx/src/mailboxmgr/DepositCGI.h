// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef DEPOSITCGI_H
#define DEPOSITCGI_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "net/Url.h"
#include "mailboxmgr/CGICommand.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 *  DepositCGI Class
 *
 *  This is the entry point for depositing voicemail.
 *  This CGI retrieves the URL for the user's greeting to be played
 *  and sends it to the VXML script that takes care of the actual deposit use case.
 *
 *  @author Harippriya M Sivapatham
 *  @version 1.0
 */
class DepositCGI : public CGICommand
{
public:
    /**
     * Ctor
     */
    DepositCGI ( const Url& from, const UtlString& mailboxIdentity );

    /**
     * Virtual Dtor
     */
    virtual ~DepositCGI();

    /** This does the work */
    virtual OsStatus execute (UtlString* out = NULL);

protected:

private:
    const Url m_from;

    /** Fully qualified mailbox id */
    UtlString m_mailboxIdentity;
};

#endif //DEPOSITCGI_H


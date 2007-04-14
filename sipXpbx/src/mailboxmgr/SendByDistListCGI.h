// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SENDBYDISTLISTCGI_H
#define SENDBYDISTLISTCGI_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "net/Url.h"
#include "mailboxmgr/CGICommand.h"
#include "mailboxmgr/MailboxManager.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class SendByDistListCGI : public CGICommand
{
public:
    /**
     * Ctor
     */
    SendByDistListCGI ( const Url& from, 
                        const UtlString& identity, 
                        const UtlString& distlist, 
                        const UtlString& durationsecs,
                        const UtlString& timestamp,
                        const char* termchar, 
                        const char* data, 
                        int   datasize );
    /**
     * Virtual Destructor
     */
    virtual ~SendByDistListCGI();

    /** This does the work */
    virtual OsStatus execute (UtlString* out = NULL) ;

protected:

private:
    const Url m_from;
    const UtlString m_identity;
    const UtlString m_distlist;
    const UtlString m_duration;
    const UtlString m_timestamp;
    UtlString m_termchar;
    char* m_data;
    int   m_datasize;
};

#endif //SENDBYDISTLISTCGI_H

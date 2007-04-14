// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef TRANSFERCGI_H
#define TRANSFERCGI_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
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
 *  TransferToExtnCGI Class
 *
 *  CGI for taking the extension dialed by the user and converting it 
 *  into a fully qualified SIP URL and then transfering to that extension.
 *
 *  @author Harippriya M Sivapatham
 *  @version 1.0
 */
class TransferCGI : public CGICommand
{
public:
    /**
     * Ctor
     */
    TransferCGI ( const UtlString& conferenceurl );

    /**
     * Virtual Dtor
     */
    virtual ~TransferCGI();

    /** This does the work */
    virtual OsStatus execute (UtlString* out = NULL);

protected:

private:
    const UtlString mConferenceUrl;
};

#endif //TRANSFERCGI_H


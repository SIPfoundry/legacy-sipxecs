//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef LOGINCGI_H
#define LOGINCGI_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlString.h"
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
 * Mailbox Class
 *
 */
class LoginCGI : public CGICommand
{
public:
    /**
     * Ctor
     */
    LoginCGI ( const UtlString& contact, const UtlString& confid, const UtlString& accessCode );

    /**
     * Virtual Dtor
     */
    virtual ~LoginCGI();


    /** This does the work */
    virtual OsStatus execute ( UtlString* out = NULL );

protected:

private:
    const UtlString mContact;
    const UtlString mConfId;
    const UtlString mAccessCode;
};

#endif //LOGINCGI_H

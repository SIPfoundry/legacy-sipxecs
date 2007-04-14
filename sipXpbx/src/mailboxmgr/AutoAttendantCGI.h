// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef AUTOATTENDANTCGI_H
#define AUTOATTENDANTCGI_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "net/Url.h"
#include "mailboxmgr/CGICommand.h"

// DEFINES
#define DEFAULT_AA_NAME    "operator"

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
 * @author John P. Coffey
 * @version 1.0
 */
class AutoAttendantCGI : public CGICommand
{
public:
    /**
     * Ctor
     */
    AutoAttendantCGI ( const Url& from, const UtlString& name, const char* digits );

    /**
     * Virtual Dtor
     */
    virtual ~AutoAttendantCGI();

    /** This does the work */
    virtual OsStatus execute (UtlString* out = NULL);

protected:

private:
    const Url m_from;
    UtlString m_name;
    const char* m_digits;
};

#endif //AUTOATTENDANTCGI_H


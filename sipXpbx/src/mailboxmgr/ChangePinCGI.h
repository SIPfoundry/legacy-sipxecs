// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef CHANGEPINCGI_H
#define CHANGEPINCGI_H

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

class ChangePinCGI : public CGICommand
{
public:
    /**
     * Ctor
     */
    ChangePinCGI ( const UtlString& userid,
                   const UtlString& password,
                   const UtlString& newpassword);

    /**
     * Virtual Destructor
     */
    virtual ~ChangePinCGI();

    /** This does the work */
    virtual OsStatus execute ( UtlString* out = NULL);

protected:

private:
    const UtlString mUserId;
    const UtlString mOldPassword;
    const UtlString mNewPassword;
};

#endif //CHANGEPINCGI_H


// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SETACTIVEGREETING_H
#define SETACTIVEGREETING_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
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
 * SetActiveGreeting Class
 *
 * @author John P. Coffey
 * @version 1.0
 */
class SetActiveGreeting : public CGICommand
{
public:
    /**
     * Ctor
     */
    SetActiveGreeting();

    /**
     * Virtual Destructor
     */
    virtual ~SetActiveGreeting();

    /** This does the work */
    virtual OsStatus execute ();

protected:

private:
};

#endif //SETACTIVEGREETING_H


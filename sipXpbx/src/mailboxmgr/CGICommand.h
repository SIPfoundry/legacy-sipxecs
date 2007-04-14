// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef CGICOMMAND_H
#define CGICOMMAND_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsStatus.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlString;

/**
 * Abstract Command Interface.  Classic command pattern
 *
 * @author John P. Coffey
 * @version 1.0
 */
class CGICommand
{
public:
    /** Ctor */
    CGICommand () {};
    /**
     * Virtual Destructor
     */
    virtual ~CGICommand() {};

    /** Declare business method */
    virtual OsStatus execute (UtlString* out = NULL) = 0;
};

#endif //CGICOMMAND_H


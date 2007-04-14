// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef AUTHEXCEPTIONROW_H
#define AUTHEXCEPTIONROW_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "fastdb/fastdb.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * The Huntgroup Table Schema
 */
class AuthexceptionRow
{
public:
    const char* user; 

    TYPE_DESCRIPTOR((KEY(user, INDEXED)));
};

#endif //AUTHEXCEPTIONROW_H

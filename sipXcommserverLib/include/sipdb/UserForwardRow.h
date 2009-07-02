// 
// 
// Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef USERFORWARDROW_H
#define USERFORWARDROW_H

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
 * The Call Forward Table Schema
 */
class UserForwardRow
{
public:
    const char* identity;    // this is the call forward uri identity
    const char* cfwdtime;    // This is the call forward delay time

    TYPE_DESCRIPTOR(
       (KEY(identity, INDEXED),        
        FIELD(cfwdtime)));
};

#endif //USERFORWARDROW_H


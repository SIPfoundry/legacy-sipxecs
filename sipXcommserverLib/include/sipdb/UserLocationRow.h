// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef USERLOCATIONROW_H
#define USERLOCATIONROW_H

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
 * The UserLocation Schema
 */
class UserLocationRow
{
public:
    const char* identity;
    const char* location;
    TYPE_DESCRIPTOR((KEY(identity, INDEXED),
                     FIELD(location)));
};

#endif //USERLOCATIONROW

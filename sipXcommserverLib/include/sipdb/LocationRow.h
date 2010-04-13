// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef LOCATIONROW_H
#define LOCATIONROW_H

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
 * The Location Schema
 */
class LocationRow
{
public:
    const char* name;
    const char* description;
    const char* locationcode;
    const char* subnets;
    
    TYPE_DESCRIPTOR ( ( KEY(name, INDEXED),
                        FIELD(description),
                        FIELD(locationcode),
                        FIELD(subnets ) ) );
};

#endif //LOCATIONROW_H

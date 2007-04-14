// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef HUNTGROUPROW_H
#define HUNTGROUPROW_H

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
class HuntgroupRow
{
public:
    const char* identity; 

    TYPE_DESCRIPTOR((KEY(identity, INDEXED)));
};

#endif //HUNTGROUPIDENTITY_H

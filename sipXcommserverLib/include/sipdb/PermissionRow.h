//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef PERMISSIONROW_H
#define PERMISSIONROW_H

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
 * The Permission Schema
 */
class PermissionRow
{
public:
    const char* identity;
    const char* permission;
    TYPE_DESCRIPTOR((KEY(identity, INDEXED),
                     FIELD(permission)));
};

#endif //PERMISSIONROW_H

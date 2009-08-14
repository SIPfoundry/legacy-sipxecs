//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef DIALBYNAMEROW_H
#define DIALBYNAMEROW_H

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
 * The DialByName Table Schema
 */
class DialByNameRow
{
public:
    const char* np_identity;
    const char* np_contact;
    const char* np_digits;

    TYPE_DESCRIPTOR(
       (KEY(np_identity, INDEXED),      // non persistent AVL Tree
        KEY(np_digits, HASHED),    // non persistent Hashed
        FIELD(np_contact)));
};

#endif //DIALBYNAMEROW_H

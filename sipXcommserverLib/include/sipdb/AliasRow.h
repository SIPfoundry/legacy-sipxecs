//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef ALIASROW_H
#define ALIASROW_H

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
 * The Alias Table Schema
 */
class AliasRow
{
public:
    const char* identity;    // this is the alias uri identity
    const char* contact;     // This is the link to the CredentialsDB (full URI)
    const char* relation;    // The value of the 'relation' field.

    TYPE_DESCRIPTOR(
       (KEY(identity, INDEXED),
        FIELD(contact),
        FIELD(relation)));
};

#endif //ALIASROW_H

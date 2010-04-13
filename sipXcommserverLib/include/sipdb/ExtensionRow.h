//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef EXTENSIONROW_H
#define EXTENSIONROW_H

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
 * The Extension Table Schema
 */
class ExtensionRow
{
public:
    const char* np_identity;
    const char* uri;
    const char* extension;

    TYPE_DESCRIPTOR(
       (KEY(np_identity, INDEXED),
        KEY(uri, HASHED),
        FIELD(extension)));
};

#endif //EXTENSIONROW_H

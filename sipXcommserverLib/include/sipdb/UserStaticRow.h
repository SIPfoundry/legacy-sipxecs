// 
// 
// Copyright (C) 2009 Nortel Networks, certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef USERSTATICROW_H
#define USERSTATICROW_H

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
 * The Static Contact Table Schema
 */
class UserStaticRow
{
public:
    const char* identity;   // this is the static contact uri identity
    const char* event;      // This is the static contact event type
    const char* contact;    // This is the static contact field
    const char* from_uri;   // This is the static contact from URI
    const char* to_uri;     // This is the static contact to URI
    const char* callid;     // This is the static contact call identity

    TYPE_DESCRIPTOR(
       (KEY(identity, INDEXED),        
        KEY(event, HASHED),        
        FIELD(contact),
        FIELD(from_uri),
        FIELD(to_uri),
        FIELD(callid)));
};

#endif //USERSTATICROW_H


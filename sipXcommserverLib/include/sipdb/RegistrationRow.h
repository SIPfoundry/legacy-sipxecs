//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef REGISTRATIONROW_H
#define REGISTRATIONROW_H
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
 * The Registration Base Schema
 */
class RegistrationRow
{
public:
    const char* np_identity;
    const char* uri;
    const char* callid;
    const char* contact;
    const char* qvalue;
    const char* instance_id;
    const char* gruu;
    const char* path;
    int4 cseq;
    int4 expires;             // Absolute expiration time, seconds since 1/1/1970
    const char* primary;      // The name of the Primary Registrar for this registration
    db_int8 update_number;    // The DbUpdateNumber of the last modification to this entry
    const char* instrument;
    TYPE_DESCRIPTOR (
      ( KEY(np_identity, INDEXED),
        KEY(callid, HASHED),
        KEY(cseq, HASHED),
        KEY(primary, INDEXED),
        FIELD(uri),
        FIELD(contact),
        FIELD(qvalue),
        FIELD(expires),
        // In principle, we don't have to keep both the GRUU and Instance ID,
        // as we do not need the IID operationally, and the GRUU can
        // be calculated from it.  But it makes debugging a lot easier to log
        // both in registration.xml.
        FIELD(instance_id),
        FIELD(gruu),
        FIELD(path),
        FIELD(update_number),
        FIELD(instrument)
      )
    );
};

#endif //REGISTRATIONROW_H

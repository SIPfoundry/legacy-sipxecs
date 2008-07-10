// 
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _CgiValues_h_
#define _CgiValues_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <cgicc/Cgicc.h>
#include <string.h>
#include <vector>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * Wraps CGICC with helper routines.
 */
class CgiValues 
{
private:
    cgicc::Cgicc* m_cgi;

    std::vector<std::string> m_values;

public:

    CgiValues(cgicc::Cgicc *cgi);

    const char* valueOf(const char *name);
};

#endif // _CgiValues_h_

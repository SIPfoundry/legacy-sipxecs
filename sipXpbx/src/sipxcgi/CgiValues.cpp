// 
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#include <cgicc/Cgicc.h>
#include <string.h>
#include <sipxcgi/CgiValues.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */
                                                                                                             
CgiValues::CgiValues(cgicc::Cgicc *cgi)
{
    m_cgi = cgi;
}


const char* CgiValues::valueOf(const char *name)
{
    const char *cstrValue = NULL;
    cgicc::form_iterator i = m_cgi->getElement(name);
    if (i != m_cgi->getElements().end())
    {
        // save every returned value, caller expects values to remain
        // valid until this class instance is destructed
        std::string strValue = i->getValue();
        m_values.insert(m_values.end(), strValue);
        cstrValue = strValue.c_str();
    }

    return cstrValue;
}

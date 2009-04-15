// 
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////
#ifndef _XMLERRORMSG_H_
#define _XMLERRORMSG_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "xmlparser/tinyxml.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Get the TiXml error description with row and column numbers (and file name if available)
void XmlErrorMsg(const TiXmlDocument& parserDoc, UtlString& errorMsg);

#endif // _XMLERRORMSG_H_

// 
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "xmlparser/XmlErrorMsg.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

void XmlErrorMsg(const TiXmlDocument* parserDoc, UtlString& errorMsg)
{
   errorMsg.remove(0);
   errorMsg.append(parserDoc->ErrorDesc());

   int row = const_cast<TiXmlDocument*>(parserDoc)->ErrorRow();
   if (row)
   {
      errorMsg.append(" at line ");
      errorMsg.appendNumber(row);
      errorMsg.append(" column ");
      errorMsg.appendNumber(const_cast<TiXmlDocument*>(parserDoc)->ErrorCol());
   }

   // If available, add the file name from which the xml was loaded.
   const char* xmlDocumentName = parserDoc->Value();
   if (xmlDocumentName)
   {
      errorMsg.append(" in '");
      errorMsg.append(xmlDocumentName);
      errorMsg.append("'");
   }
}


//
//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
// SYSTEM INCLUDES
#ifndef _AlarmUtils_
#define _AlarmUtils_

// APPLICATION INCLUDES
#include "xmlparser/tinyxml.h"
#include "utl/UtlString.h"
#include "utl/UtlSList.h"


/// Assemble a message by substituting in the parameters for placeholders {n} in the formatStr
void assembleMsg(
      const UtlString& formatStr,          ///< template message with placeholders
      const UtlSList& params,              ///< list of UtlString runtime parameters
      UtlString& outMsg                    ///< formatted message with parameters
      );

/// Assemble a message by substituting in the parameter for placeholders {0} in the formatStr
void assembleMsg(
      const UtlString& formatStr,          ///< template message with placeholder
      const UtlString& param,              ///< runtime parameter
      UtlString& outMsg                    ///< formatted message with parameter
      );

/// Safely return the specified attribute from the element.  Return defValue if not configured.
int getIntAttribute(
      TiXmlElement *element,               ///< pointer to XML element possibly containing attr
      const char *attrStr,                 ///< attribute string to look for
      int defValue=0                       ///< default value to return if element or attr not present
      );

/// Safely return the specified attribute from the element.  Return defValue if not configured.
bool getBoolAttribute(
      TiXmlElement *element,               ///< pointer to XML element possibly containing attr
      const char *attrStr,                 ///< attribute string to look for
      bool defValue=false                  ///< default value to return if element or attr not present
      );

#endif //_AlarmUtils__


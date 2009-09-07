//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _XmlContent_h_
#define _XmlContent_h_

// SYSTEM INCLUDES
#include "os/OsDefs.h"

// APPLICATION INCLUDES
#include "utl/UtlString.h"

// DEFINES
#define XML_VERSION_1_0 "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"


// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * XmlContent provides conversion functions for escaping and unescaping UtlStrings
 * as appropriate for use in XML attribute and element content.
 *
 * At present, this makes no accomodation for character set differences; input is assumed
 * to be 8 bits.  The following characters are encoded using the mandatory character
 * entities:
 *   - < => &lt;
 *   - & => &amp;
 *   - > => &gt;
 *   - ' => &apos;
 *   - " => &quot;
 *
 * Other character values outside the range of valid 8-bit characters in XML:
 * - [pound]x09 | [pound]x0A | [pound]x0D | [[pound]x20-[pound]FF]
 * are encoded using the numeric entity encoding (&[pound]x??;).
 *
 * While this is not strictly XML conformant (in that it does not explicitly deal with
 * larger-size character encodings), it is symmetric (escaping and unescaping any string
 * with these routines will always produce the original string), and will interoperate
 * correctly for any 8 bit encoding.
 */

/// Append escaped source string onto destination string
bool XmlEscape(UtlString& destination, const UtlString& source);
/**<
 * The contents of the source string are appended to the destination string, with all
 * characters escaped as described above.
 * @returns true for success, false if an error was returned from any UtlString operation.
 */

/// Append escaped source string onto destination string
bool XmlEscape(UtlString& destination, const char* source);
/**<
 * The contents of the source string are appended to the destination string, with all
 * characters escaped as described above.
 * @returns true for success, false if an error was returned from any UtlString operation.
 */

/// Append unescaped source string onto destination string
bool XmlUnEscape(UtlString& destination, const UtlString& source);
/**<
 * The contents of the source string are appended to the destination string, with all
 * characters unescaped as described above.
 * @returns true for success, false if an error was returned from any UtlString operation.
 */

#endif    // _XmlContent_h_

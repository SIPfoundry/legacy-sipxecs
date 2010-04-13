//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _NetAttributeTokenizer_h_
#define _NetAttributeTokenizer_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <utl/UtlString.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlList;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class NetAttributeTokenizer
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   NetAttributeTokenizer(const char* parseString = NULL);
     //:Default constructor

   virtual
   ~NetAttributeTokenizer();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   UtlBoolean getNextAttribute(UtlString& attributeName, UtlString& attributeValue);
   // Retrieve the next attribute name and value if present

   UtlBoolean getAttributes(UtlList& attributeList);
   // Retrieve all of the attributes in the collection of NameValuePair(s)

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    UtlString attributeParseString;
    int parseIndex;

   NetAttributeTokenizer(const NetAttributeTokenizer& rNetAttributeTokenizer);
     //:Copy constructor (disabled)
   NetAttributeTokenizer& operator=(const NetAttributeTokenizer& rhs);
     //:Assignment operator (disabled)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _NetAttributeTokenizer_h_

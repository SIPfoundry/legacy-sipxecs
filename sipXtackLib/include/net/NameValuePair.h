//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _NameValuePair_h_
#define _NameValuePair_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlString.h"

#include <os/OsMutex.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//: Name Value pair
// for storing string values associated with a key or name
class NameValuePair : public UtlString
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   NameValuePair(const char* name, const char* value = NULL);
   //: Construct a pair
   // Data is copied not attached or freed
   //! param: name - the key for this object, null terminated string
   //! param: value - the data or value for this object, null
   //! param:   terminated string

   virtual
   ~NameValuePair();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   NameValuePair& operator=(const NameValuePair& rhs);
     //:Assignment operator

   NameValuePair(const NameValuePair& rNameValuePair);
     //: Copy constructor

/* ============================ ACCESSORS ================================= */

   const char* getValue();
   //: get value string
   //! returns: the null terminated string containing the value <br>
   //! Note: this should not be freed as it is part of this object

   void setValue(const char*);

/* ============================ INQUIRY =================================== */
public:
        static int count;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   char* valueString;

   NameValuePair();
     //: Hide Default constructor

        static OsMutex    mCountLock;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _NameValuePair_h_

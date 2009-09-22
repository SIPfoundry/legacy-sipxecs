//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _NameValuePairInsensitive_h_
#define _NameValuePairInsensitive_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "net/NameValuePair.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//: For storing string values associated with a key or name.
//  Functions like NameValuePair, but the name (key) is case-insensitive.
class NameValuePairInsensitive : public NameValuePair
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   NameValuePairInsensitive(const char* name, const char* value = NULL);
   //: Construct a pair
   // Data is copied, not attached or freed
   //! param: name - the key for this object, null terminated string
   //! param: value - the data or value for this object, null
   //! param:   terminated string

   virtual
   ~NameValuePairInsensitive();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   NameValuePairInsensitive& operator=(const NameValuePairInsensitive& rhs);
     //:Assignment operator

   NameValuePairInsensitive(const NameValuePairInsensitive& rNameValuePairInsensitive);
     //: Copy constructor

/* ============================ ACCESSORS ================================= */

   const char* getValue();
   //: get value string
   //! returns: the null terminated string containing the value <br>
   //! Note: this should not be freed as it is part of this object

   void setValue(const char*);

/* ============================ INQUIRY =================================== */
public:

   // Redefine all the container support operations to be case-insenstive,
   // rather than the case-sensitive operations on keys that NameValuePair
   // inherits from UtlString.

    /// Calculate a hash over the string contents.
    virtual unsigned hash() const;
    /**<
     * If the equals operator returns true for another object, then both
     * objects must return the same hashcode.
     */

    /// Return the unique type value for this class.
    virtual UtlContainableType getContainableType() const;

    /// Compare to any other UtlContainable
    virtual int compareTo(UtlContainable const *other) const;
    /**<
     * Compare this object to another containable object.
     * If the UtlContainableType of the other object is not the UtlString type,
     * this will return unequal.
     *
     * @returns 0 if equal, < 0 if less than, and > 0 if greater.
     */

    virtual UtlBoolean isEqual(UtlContainable const *) const;
    /**<
     * Test this object to another like-object for equality.  This method
     * returns false if unlike-objects are specified.
     */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    static UtlContainableType TYPE;    /** < Class type used for runtime checking */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _NameValuePairInsensitive_h_

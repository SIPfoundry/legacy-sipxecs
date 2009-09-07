//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsNameDb_h_
#define _OsNameDb_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsBSem.h"
#include "os/OsRWMutex.h"
#include "os/OsStatus.h"
#include "utl/UtlHashMap.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlString;

//:Name service maintaining mappings between string names and pointer values
// The OsNameDb is a singleton object that maintains a dictionary of
// mappings between string names and the associated pointer values.
// Duplicate names are not allowed.

class OsNameDb
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   static OsNameDb* getNameDb(void);
     //:Return a pointer to the singleton object, creating it if necessary

/* ============================ MANIPULATORS ============================== */

   OsStatus insert(const UtlString& rKey,
                   void* value);
     //:Add the key-value pair to the name database
     // Return OS_SUCCESS if successful, OS_NAME_IN_USE if the key is
     // already in the database.

   OsStatus remove(const UtlString& rKey,
                   void** pValue = NULL);
     //:Remove the indicated key-value pair from the name database
     // If pValue is non-NULL, the value for the key-value pair is returned
     // via pValue.<br>
     // Return OS_SUCCESS if the lookup is successful, return
     // OS_NOT_FOUND if there is no match for the specified key.

/* ============================ ACCESSORS ================================= */

   OsStatus lookup(const UtlString& rKey,
                   void** pValue = NULL);
     //:Retrieve the value associated with the specified key
     // If pValue is non-NULL, the value is returned via pValue. <br>
     // Return OS_SUCCESS if the lookup is successful, return
     // OS_NOT_FOUND if there is no match for the specified key.

   int numEntries(void);
     //: Return the number of key-value pairs in the name database

/* ============================ INQUIRY =================================== */

   UtlBoolean isEmpty(void);
     //:Return TRUE if the name database is empty

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   OsNameDb();
     //:Default constructor (only callable from within this class)
     // This class implements the singleton pattern and therefore the
     // class constructor will not be called from outside the class.
     // We identify this as a protected (rather than a private) method so
     // that gcc doesn't complain that the class only defines a private
     // constructor and has no friends.

   virtual
      ~OsNameDb();
   //:Destructor

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static OsNameDb* spInstance;     // pointer to the single instance of the
                                    //  OsNameDb class
   static OsBSem*   spLock;         // semaphore used to ensure that there is
                                    //  only one instance of this class
   UtlHashMap mDict;          // hash table used to store the name/value
                                    //  mappings
   OsRWMutex        mRWLock;        // R/W mutex used to coordinate access to
                                    //  the name database by multiple tasks
   OsNameDb(const OsNameDb& rOsNameDb);
     //:Copy constructor (not supported for this class)

   OsNameDb& operator=(const OsNameDb& rhs);
     //:Assignment operator (not supported for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsNameDb_h_

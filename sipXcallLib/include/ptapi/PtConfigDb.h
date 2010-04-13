//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtConfigDb_h_
#define _PtConfigDb_h_

// SYSTEM INCLUDES
#include <stdio.h>

// APPLICATION INCLUDES
#include "PtDefs.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Configuration database containing key/value pairs.
// This class maintains a dictionary of key/value pairs.

class PtConfigDb
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   PtConfigDb();
     //:Default constructor

   virtual
   ~PtConfigDb();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtStatus loadFromFile(FILE* fp);
     //:Load the configuration database from a file
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   PtStatus storeToFile(FILE* fp);
     //:Store the config database to a file
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   PtStatus remove(const char* pKey);
     //:Remove the key/value pair associated with pKey.
     //!param: pKey - The key for the key/value pair to be removed
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_NOT_FOUND - The key was not found
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   PtStatus set(const char* pKey, const char* pNewValue);
     //:Insert the key/value pair into the config database.
     // If the database already contains an entry for this key, then
     // set the value for the existing entry to pNewValue.
     //!param: pKey - The key for the key/value pair being added (or modified)
     //!param: pNewValue - The new value for the key/value pair being added (or modified)
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

/* ============================ ACCESSORS ================================= */

   PtStatus get(const char* pKey, char*& rpValue);
     //:Sets <i>rpValue</i> to point to the value in the database
     //:associated with <i>pKey</i>.
     // If pKey is found in the database, returns PT_SUCCESS.  Otherwise,
     // returns PT_NOT_FOUND and sets rpValue to point to an empty string.
     //!param: pKey - The lookup key
     //!param: rpValue - Set to point to the value corresponding to <i>pKey</i>
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_NOT_FOUND - The key was not found
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   PtStatus get(const char* pKey, int& rValue);
     //:Sets <i>rValue</i> to the integer value in the database associated
     //:with <i>pKey</i>.
     // If pKey is found in the database, returns PT_SUCCESS.  Otherwise,
     // returns PT_NOT_FOUND and sets rValue to -1.
     //!param: pKey - The lookup key
     //!param: rValue - Set to the integer value corresponding to <i>pKey</i>
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_NOT_FOUND - The key was not found
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   PtStatus getNext(const char* pKey,
                    char*& rpNextKey, char*& rpNextValue);
     //:Relative to <i>pKey</i>, sets <i>rpNextKey</i> and
     //:<i>rpNextValue</i> to point to the key and value associated with
     //:next (lexicographically ordered) key/value pair stored in the
     //:database.
     // If pKey is the empty string, the key and value associated
     // with the first entry in the database will be returned.
     //!param: pKey - The lookup key
     //!param: rpNextKey - Set to point to the key for the next key/value pair stored in the database
     //!param: rpNextValue - Set to point to the value for the next key/value pair stored in the database
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_NO_MORE_DATA - There is no "next" entry
     //!retcode: PT_NOT_FOUND - The key was not found
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

/* ============================ INQUIRY =================================== */

   PtStatus isEmpty(PtBoolean& rIsEmpty);
     //:Sets <i>rIsEmpty</i> to TRUE if the database is empty, otherwise
     //:FALSE.
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   PtStatus numEntries(int& rNumEntries);
     //:Sets <i>rNumEntries</i> to the number of entries in the config database
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtConfigDb_h_

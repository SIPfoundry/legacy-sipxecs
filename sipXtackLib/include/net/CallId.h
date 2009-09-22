//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _CallId_h_
#define _CallId_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES

#include <utl/UtlString.h>
#include <os/OsMutex.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/** Completely static class that generates new Call-Id's, and to- and
 * from-tags.
 */

class CallId
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

   //! Generate a new Call-Id
   static void getNewCallId(UtlString& callId ///< output UtlString
                            );

   //! Generate a new tag.
   static void getNewTag(UtlString& tag ///< output UtlString
                         );

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   //! Mutex to protect sCallNum.
   static OsMutex sCallIdMutex;

   //! Call counter for generating Call-IDs.
   static size_t sCallNum;

   //! Chain pseudo-random value.
   static UtlString sChainValue;

   //! Flag to record if sChainValue has been initialized.
   static bool sChainValueInitialized;

   //! Initialize the chain value.
   static void initialize();

   //! Compute the next chain value.
   static void nextValue();

};

/* ============================ INLINE METHODS ============================ */

#endif  // _CallId_h_

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

   //! Generate a new Call-Id with the default prefix ("s").
   static void getNewCallId(/// output UtlString
                            UtlString& callId);

   //! Generate a new Call-Id with the specified prefix.
   static void getNewCallId(/// the prefix to use (10 chars max.)
                            const char* callIdPrefix,
                            /// output UtlString
                            UtlString& callId);

   //! Generate a new tag.
   static void getNewTag(/// seed string
                         const char* seed,
                         /**< Ideally, a pointer to an externally-generated
                          *   crypto-random string, especially the tag
                          *   from the other end of a dialog.
                          *   Don't bother using an internally-generated
                          *   string, which doesn't add any entroy,
                          *   use "" instead.
                          */
                         /// output UtlString
                         UtlString& tag);

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   //! Mutex to protect sCallNum.
   static OsMutex sCallIdMutex;

   //! Call counter for generating Call-IDs.
   static unsigned int sCallNum;

   //! Chain pseudo-random value.
   static UtlString sChainValue;

   //! Flag to record if sChainValue has been initialized.
   static UtlBoolean sChainValueInitialized;

   //! Secret key value.
   static UtlString sKey;

   //! Initialize the chain value.
   static void initialize();

   //! Compute the next chain value.
   static void nextValue(const char* seed);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _CallId_h_

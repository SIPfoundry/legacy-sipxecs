//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _UtlTokenizer_h_
#define _UtlTokenizer_h_

// SYSTEM INCLUDES
#include <ctype.h>

// APPLICATION INCLUDES
#include <utl/UtlDefs.h>
#include <os/OsDefs.h>
#include <utl/UtlString.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * C++ version of strtok, but obvisously reentrant.
 */
class UtlTokenizer
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/* ============================ CREATORS ================================== */

   /**
    * Default constructor
    */
   UtlTokenizer(const UtlString &tokens);

   /**
    * Destructor
    */
   virtual ~UtlTokenizer();

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */
   UtlBoolean next(UtlString &token, const char *delim);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   char *m_tokens;

   int m_tokenPosition;

   /**
    * Copy constructor
    */
   UtlTokenizer(const UtlTokenizer& rUtlTokenizer);

   /**
    * Assignment operator
    *
    * @param rhs right hand side of the equals operator
    */
   UtlTokenizer& operator=(const UtlTokenizer& rhs);

   int nextDelim(const char *tokens, const int start, const int len, const char *delim);

   UtlBoolean isDelim(const char c, const char *delim);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _UtlTokenizer_h_

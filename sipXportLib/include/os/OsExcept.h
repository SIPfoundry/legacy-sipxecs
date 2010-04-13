//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsExcept_h_
#define _OsExcept_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "OsDefs.h"
#include "utl/UtlString.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Base class for exceptions thrown from the OS abstraction layer
// The abstraction layer exception handling mechanism is based on the
// OsExcept class. This class stores information about the type, cause,
// and location of the exception.

class OsExcept
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum MajorCode
   {
      MAJOR_NONE,           // Unclassified exception
      MAJOR_RUNTIME,        // Runtime exception
      MAJOR_USER            // User-defined exception
   };
   //!enumcode: MAJOR_NONE - Unclassified exception
   //!enumcode: MAJOR_RUNTIME - Runtime exception
   //!enumcode: MAJOR_USER - User-defined exception

   enum MinorCode
   {
      MINOR_NONE,           // Unclassified exception
      MINOR_RUNTIME,        // Runtime exception
      MINOR_USER            // User-defined exception
   };
   //!enumcode: MINOR_NONE - Unclassified exception
   //!enumcode: MINOR_RUNTIME - Runtime exception
   //!enumcode: MINOR_USER - User-defined exception

/* ============================ CREATORS ================================== */

   OsExcept(const int majorCode=MAJOR_NONE, const int minorCode=MINOR_NONE,
                      const UtlString& rText="", const UtlString& rContext="");
     //:Constructor

   OsExcept(const OsExcept& rOsExcept);
     //:Copy constructor

   virtual
   ~OsExcept();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsExcept& operator=(const OsExcept& rhs);
     //:Assignment operator

   virtual void setMajorCode(const int majorCode);
     //:Set major exception code

   virtual void setMinorCode(const int minorCode);
     //:Set minor exception code

   virtual void setContext(const UtlString& rContext);
     //:Set exception context

   virtual void setText(const UtlString& rText);
     //:Set exception text

/* ============================ ACCESSORS ================================= */

   virtual int getMajorCode(void) const;
     //:Get major exception code

   virtual int getMinorCode(void) const;
     //:Get minor exception code

   virtual const UtlString& getContext(void) const;
     //:Get exception context

   virtual const UtlString& getText(void) const;
     //:Get exception text

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   int       mMajorCode;    // major exception code
   int       mMinorCode;    // minor exception code
   UtlString* mpText;        // exception text
   UtlString* mpContext;     // exception context

   void init(void);
     //:Initialize the member variables (called by the constructors)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsExcept_h_

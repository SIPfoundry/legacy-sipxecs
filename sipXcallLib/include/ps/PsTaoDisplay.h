//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PsTaoDisplay_h_
#define _PsTaoDisplay_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "ps/PsTaoComponent.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:The PsTaoDisplay class models the phone hook switch.
class PsTaoDisplay : public PsTaoComponent
{
   friend class PsPhoneTask;
     // The PsPhoneTask is responsible for creating and destroying
     // all objects derived from the PsTaoComponent class.  No other entity
     // should invoke the constructors or destructors for these classes.

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

        void setDisplay(const char*& string, int x, int y);
         //:Sets the display string at position (x, y).

        /* ============================ ACCESSORS ================================= */

        int getDisplayRows();
         //:Returns the display rows.

        int getDisplayColumns();
        //:Returns the display columns.

        char* getDisplay(int x, int y);
        // returns the string displayed at (x, y)

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   PsTaoDisplay(const UtlString& rName, int initialState);
     //:Constructor

   virtual
   ~PsTaoDisplay();
     //:Destructor

   PsTaoDisplay();
     //:Default constructor (not implemented for this class)

   PsTaoDisplay(const PsTaoDisplay& rPsTaoDisplay);
     //:Copy constructor (not implemented for this class)

   PsTaoDisplay& operator=(const PsTaoDisplay& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PsTaoDisplay_h_

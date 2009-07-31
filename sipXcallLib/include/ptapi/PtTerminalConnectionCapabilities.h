//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtTerminalConnectionCapabilities_h_
#define _PtTerminalConnectionCapabilities_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PtTerminalConnection;

//:The PtTerminalConnectionCapabilities interface represents the initial capabilities interface for the
// TerminalConnection. This interface supports basic queries for the core package.
// <p>
// Applications obtain the static TerminalConnection capabilities via the
// Provider.getTerminalConnectionCapabilities() method, and the dynamic capabilities via the
// TerminalConnection.getCapabilities() method. This interface is used to represent both static
// and dynamic capabilities.
// <p>
// Any package which extends the core TerminalConnection interface should also extend this
// interface to provide additional capability queries for that particular package. // <p>
// <p>

class PtTerminalConnectionCapabilities
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   PtTerminalConnectionCapabilities();
     //:Default constructor

   PtTerminalConnectionCapabilities(const PtTerminalConnectionCapabilities& rPtTerminalConnectionCapabilities);
     //:Copy constructor

   virtual
   ~PtTerminalConnectionCapabilities();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtTerminalConnectionCapabilities& operator=(const PtTerminalConnectionCapabilities& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

        UtlBoolean canAnswer();
     //:Returns true if the application can invoke TerminalConnection.answer(), false otherwise.
     //!retcode: True if the application can answer, false otherwise
         //!retcode: false otherwise.



        UtlBoolean canHold();
     //:Returns true if the application can invoke the hold feature, false otherwise.
     //!retcode: True if the application can invoke the hold feature,
         //!retcode: false otherwise.



        UtlBoolean canUnhold();
     //:Returns true if the application can invoke the unhold feature, false otherwise.
     //!retcode: True if the application can invoke the unhold feature,
         //!retcode: false otherwise.



        UtlBoolean canJoin();
     //:Returns true if the application can invoke the join feature, false otherwise.
     //!retcode: True if the application can invoke the join feature,
         //!retcode: false otherwise.



        UtlBoolean canLeave();
     //:Returns true if the application can invoke the leave feature, false otherwise.
     //!retcode: True if the application can invoke the leave feature,
         //!retcode: false otherwise.


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtTerminalConnectionCapabilities_h_

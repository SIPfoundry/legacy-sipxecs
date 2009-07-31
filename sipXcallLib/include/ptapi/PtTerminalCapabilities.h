//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PtTerminalCapabilities_h_
#define _PtTerminalCapabilities_h_

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
class PtAddress;
class PtTerminal;
class PtConnection;
class PtTerminalConnection;

//:The TerminalCapabilities interface represents the initial capabilities interface for the Terminal.
// This interface supports basic queries for the core package.
// <p>
// Applications obtain the static Terminal capabilities via the Provider.getTerminalCapabilities()
// method, and the dynamic capabilities via the Terminal.getCapabilities() method. This interface
// is used to represent both static and dynamic capabilities.
// <p>
// Any package which extends the core Terminal interface should also extend this interface to
// provide additional capability queries for that particular package.
// <p>

class PtTerminalCapabilities
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   PtTerminalCapabilities();
     //:Default constructor

   PtTerminalCapabilities(const PtTerminalCapabilities& rPtTerminalCapabilities);
     //:Copy constructor

   virtual
   ~PtTerminalCapabilities();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtTerminalCapabilities& operator=(const PtTerminalCapabilities& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

        UtlBoolean isObservable();
     //:Returns true if this terminal can be observed, false otherwise.
     //!retcode: True if this terminal can be observed,
         //!retcode: false otherwise.



        UtlBoolean canGetDoNotDisturb();
     //:Returns true if the application can obtain the do not disturb state, false otherwise.
     //!retcode: True if the application can obtain the do not disturb state,
         //!retcode: false otherwise.



        UtlBoolean canSetDoNotDisturb();
     //:Returns true if the application can set the do not disturb state, false otherwise.
     //!retcode: True if the application can set the do not disturb state,
         //!retcode: false otherwise.



        UtlBoolean canPickup(PtConnection connection, PtAddress address);
     //:Returns true if the application can invoke the overloaded pickup feature which takes a
     // Connection and an Address as arguments, false otherwise.
         // <p>
     // The arguments provided are for typing purposes only. The particular instances of the
     // objects given are ignored and not used to determine the capability outcome is any way.
     //!param: connection - This argument is used for typing information to determine the overloaded version of the pickup() method.
     //!param: address - This argument is used for typing information to determine the overloaded version of the pickup() method.
     //!retcode: True if the application can invoke the pickup feature which takes a PtConnection and an PtAddress as arguments,
         //!retcode: false otherwise.



        UtlBoolean canPickup(PtTerminalConnection tc, PtAddress address);
     //:Returns true if the application can invoke the overloaded pickup feature which takes a
     // PtTerminalConnection and an PtAddress as arguments, false otherwise.
         // <p>
     // The arguments provided are for typing purposes only. The particular instances of the
     // objects given are ignored and not used to determine the capability outcome is any way.
     //!param: tc - This argument is used for typing information to determine the overloaded version of the pickup() method.
     //!param: address - This argument is used for typing information to determine the overloaded version of the pickup() method.
     //!retcode: True if the application can invoke the pickup feature which takes a TerminalConnection and an Address as arguments,
         //!retcode: false otherwise.



        UtlBoolean canPickup(PtAddress address1, PtAddress address2);
     //:Returns true if the application can invoke the overloaded pickup feature which takes two
     // Addresses as arguments, false otherwise.
         // <p>
     // The arguments provided are for typing purposes only. The particular instances of the
     // objects given are ignored and not used to determine the capability outcome is any way.
     //!param: address1 - This argument is used for typing information to determine the overloaded version of the pickup() method.
     //!param: address2 - This argument is used for typing information to determine the overloaded version of the pickup() method.
     //!retcode: True if the application can invoke the pickup feature which takes two Addresses as arguments,
         //!retcode: false otherwise.



        UtlBoolean canPickupFromGroup(UtlString group, PtAddress address);
     //:Returns true if the application can invoke the pickup from group feature which takes a string
     // pickup group code and an Address as arguments, false otherwise.
         // <p>
     // The arguments provided are for typing purposes only. The particular instances of the
     // objects given are ignored and not used to determine the capability outcome is any way.
     //!param: group - This argument is used for typing information to determine the overloaded version of the pickupFromGroup() method.
     //!param: address - This argument is used for typing information to determine the overloaded version of the pickupFromGroup() method.
     //!retcode: True if the application can invoke the pickup from group feature which takes a string pickup group code and an Address as arguments,
         //!retcode: false otherwise.



        UtlBoolean canPickupFromGroup(PtAddress address);
     //:Returns true if the application can invoke the pickup from group feature which takes an
     // Address as an argument, false otherwise.
         // <p>
         // The arguments provided are for typing purposes only. The particular instances of the
     // objects given are ignored and not used to determine the capability outcome is any way.
     //!param: address - This argument is used for typing information to determine the overloaded version of the pickupFromGroup() method.
     //!retcode: True if the application can invoke the pickup from group feature which takes an Address as an argument,
         //!retcode: false otherwise.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtTerminalCapabilities_h_

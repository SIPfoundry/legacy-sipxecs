//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PtAddessCapabilities_h_
#define _PtAddessCapabilities_h_

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

//:The PtAddressCapabilities class provides methods to reflect the capabilities of the
// methods on the PtAddress class.
// <p>
// The PtProvider.getAddressCapabilities() method returns the static PtAddress capabilities, and the
// PtAddress.getCapabilities() method returns the dynamic Address capabilities. The object returned from each of
// these methods can be queried with the instanceof operator to check if it supports this interface. This same
// interface is used to reflect both static and dynamic PtAddress capabilities.
// <p>

class PtAddessCapabilities
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   PtAddessCapabilities();
     //:Default constructor

   PtAddessCapabilities(const PtAddessCapabilities& rPtAddessCapabilities);
     //:Copy constructor

   virtual
   ~PtAddessCapabilities();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtAddessCapabilities& operator=(const PtAddessCapabilities& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

        UtlBoolean isObservable();
         //:Returns true if this Address can be observed, false otherwise.
     //!retcode: TRUE if this Address can be observed
         //!retcode: FALSE otherwise.


        UtlBoolean canSetForwarding();
     //:Returns true if the application can set the forwarding on this Address, false otherwise.
     //!retcode: TRUE if the application can set the forwarding on this Address
         //!retcode: FALSE otherwise.



        UtlBoolean canGetForwarding();
     //:Returns true if the application can obtain the current forwarding status on this Address, false otherwise.
     //!retcode: True if the application can obtain the current forwarding status on this Address
         //!retcode: false otherwise.



        UtlBoolean canCancelForwarding();
         //:Returns true if the application can cancel the forwarding on this Address, false otherwise.
     //!retcode: True if the application can cancel the forwarding on this Address
         //!retcode: false otherwise.



        UtlBoolean canGetDoNotDisturb();
     //:Returns true if the application can obtain the do not disturb state, false otherwise.
     //!retcode: True if the application can obtain the do not disturb state
         //!retcode: false otherwise.



        UtlBoolean canSetDoNotDisturb();
     //:Returns true if the application can set the do not disturb state, false otherwise.
     //!retcode: True if the application can set the do not disturb state
         //!retcode: false otherwise.



        UtlBoolean canGetMessageWaiting();
     //:Returns true if the application can obtain the message waiting state, false otherwise.
     //!retcode: True if the application can obtain the message waiting state
         //!retcode: false otherwise.



        UtlBoolean canSetMessageWaiting();
     //:Returns true if the application can set the message waiting state, false otherwise.
     //!retcode: True if the application can set the message waiting state
         //!retcode: false otherwise.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtAddessCapabilities_h_

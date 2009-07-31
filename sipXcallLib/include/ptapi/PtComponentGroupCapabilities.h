//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtComponentGroupCapabilities_h_
#define _PtComponentGroupCapabilities_h_

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

//:The PtComponentGroupCapabilities interface represents the capabilities for the PtComponentGroup.

class PtComponentGroupCapabilities
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   PtComponentGroupCapabilities();
     //:Default constructor

   PtComponentGroupCapabilities(const PtComponentGroupCapabilities& rPtComponentGroupCapabilities);
     //:Copy constructor

   virtual
   ~PtComponentGroupCapabilities();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtComponentGroupCapabilities& operator=(const PtComponentGroupCapabilities& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

        UtlBoolean canActivate();
     //:Returns true if the ComponentGroup can be "activated" on the Terminal that the
     // ComponentGroup is associated with. For example, activation of a headset on a
     // certain Terminal allows media to flow between the headset and the telephone line
     // associated with the terminal for all calls on the line . This method allows the
     // application to determine if activation of the ComponentGroup on its Terminal is
     // supported.
     //!retcode: True if the component group can be activated on its Terminal,
        //!retcode: false otherwise.



        UtlBoolean canActivate(PtAddress address);
     //:Returns true if the ComponentGroup can be "activated" on the specified Address at
     // the Terminal that the ComponentGroup is associated with. For example, activation
     // of a headset on a certain Address at a Terminal allows media to flow between the
     // headset and the telephone line associated with the Terminal for all calls on the
     // specified Address. This method allows the application to determine if activation of
     // the ComponentGroup on a specific Address at a Terminal is supported.
     //!param: address - test if feature available for this address
     //!retcode: True if the component group can be activated on its Terminal at the specified Address,
         //!retcode: false otherwise.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtComponentGroupCapabilities_h_

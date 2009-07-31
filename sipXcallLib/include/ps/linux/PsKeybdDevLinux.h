//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#ifndef _PsKeybdDevLinux_h_
#define _PsKeybdDevLinux_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "ps/PsButtonTask.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS

//:Phone set keyboard device support on the Linux platform
class PsKeybdDevLinux : public PsKeybdDev
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   PsKeybdDevLinux(PsButtonTask* pButtonTask)
      : PsKeybdDev(pButtonTask)  { };
     //:Constructor
     // Initialize the keyboard device

   virtual
      ~PsKeybdDevLinux() { };
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual void disableIntr(void) { };
     //:Disable keyboard interrupts

   virtual void enableIntr(void) { };
     //:Enable keyboard interrupts

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   PsKeybdDevLinux(const PsKeybdDevLinux& rPsKeybdDevLinux);
     //:Copy constructor (not implemented for this class)

   PsKeybdDevLinux& operator=(const PsKeybdDevLinux& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PsKeybdDevLinux_h_

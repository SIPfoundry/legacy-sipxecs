//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PsKeybdDev_h_
#define _PsKeybdDev_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsBSem.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class PsButtonTask;

//:Base class for the phone set keyboard device
// Platform-specific keyboard classes are all derived from this class.
class PsKeybdDev
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   static PsKeybdDev* getKeybdDev(PsButtonTask* pButtonTask=NULL);
     //:Return a pointer to the keyboard device, creating it if necessary.

   virtual
   ~PsKeybdDev();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual void disableIntr(void) = 0;
     //:Disable keyboard interrupts

   virtual void enableIntr(void) = 0;
     //:Enable keyboard interrupts

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   static PsKeybdDev*  spInstance;  // pointer to the single instance of
                                    //  the PsKeybdDev class
   static OsBSem       sLock;       // semaphore used to ensure that there
                                    //  is only one instance of this class
   PsButtonTask*       mpButtonTask;

   PsKeybdDev(PsButtonTask* pButtonTask);
     //:Constructor

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   PsKeybdDev(const PsKeybdDev& rPsKeybdDev);
     //:Copy constructor (not implemented for this class)

   PsKeybdDev& operator=(const PsKeybdDev& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PsKeybdDev_h_

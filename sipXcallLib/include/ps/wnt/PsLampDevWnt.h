//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PsLampDevWnt_h_
#define _PsLampDevWnt_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "ps/PsLampDev.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class PsLampTask;

//:Phone set lamp device support on the WIN32 platform
class PsLampDevWnt : public PsLampDev
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   PsLampDevWnt(PsLampTask* pLampTask=NULL);
     //:Constructor
     // Initialize the lamp device

   virtual
   ~PsLampDevWnt();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   void lightLamps(unsigned long onLamps);
     //:Turn on the indicated lamps

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   PsLampDevWnt(const PsLampDevWnt& rPsLampDevWnt);
     //:Copy constructor (not implemented for this class)

   PsLampDevWnt& operator=(const PsLampDevWnt& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PsLampDevWnt_h_

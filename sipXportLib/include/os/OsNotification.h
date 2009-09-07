//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsNotification_h_
#define _OsNotification_h_

// SYSTEM INCLUDES
#include <stdint.h>

// APPLICATION INCLUDES
#include "os/OsStatus.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Abstract base class for event notifications

class OsNotification
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsNotification() { };
     //:Default constructor

   virtual
      ~OsNotification() { };
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus signal(intptr_t eventData) = 0;
     //:Signal the occurrence of the event

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsNotification(const OsNotification& rOsNotification);
     //:Copy constructor (not implemented for this class)

   OsNotification& operator=(const OsNotification& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsNotification_h_

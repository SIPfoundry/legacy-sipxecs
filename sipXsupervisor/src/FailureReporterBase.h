// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _FailureReporterBase_h_
#define _FailureReporterBase_h_

// SYSTEM INCLUDES
#include "os/OsStatus.h"

// APPLICATION INCLUDES

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlString;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class FailureReporterBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   FailureReporterBase();
     //:Default constructor

   FailureReporterBase(const FailureReporterBase& rFailureReporterBase);
     //:Copy constructor

   virtual
   ~FailureReporterBase();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   FailureReporterBase& operator=(const FailureReporterBase& rhs);
     //:Assignment operator
   
   virtual OsStatus report(UtlString &rProcessAlias,UtlString &rMessage) = 0;
   //: Queues the report information to the specified targte
   
   virtual OsStatus send() = 0;  
   //: Sends any queued reports to targte


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

/* ============================ INLINE METHODS ============================ */

#endif  // _FailureReporterBase_h_


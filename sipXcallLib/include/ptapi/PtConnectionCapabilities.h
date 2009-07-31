//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtConnectionCapabilities_h_
#define _PtConnectionCapabilities_h_

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
class PtConnection;

//:The PtConnectionCapabilities interface represents the initial capabilities interface for the
// PtConnection. This interface supports basic queries for the core package.
// <p>
// Applications obtain the static Connection capabilities via the
// PtProvider.getConnectionCapabilities() method, and the dynamic capabilities via the
// PtConnection.getCapabilities() method. This interface is used to represent both static and
// dynamic capabilities.
// <p>
// Any package which extends the core Connection interface should also extend this interface to
// provide additional capability queries for that particular package.
// <p>

class PtConnectionCapabilities
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   PtConnectionCapabilities();
     //:Default constructor

   PtConnectionCapabilities(const PtConnectionCapabilities& rPtConnectionCapabilities);
     //:Copy constructor

   virtual
   ~PtConnectionCapabilities();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtConnectionCapabilities& operator=(const PtConnectionCapabilities& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

        UtlBoolean canDisconnect();
     //:Returns true if the application can invoke Connection.disconnect()perform a disconnect(),
     // false otherwise.
         //!retcode: True if the application can disconnect,
         //!retcode: false otherwise.


        UtlBoolean canRedirect();
     //:Returns true if the application can invoke the redirect feature, false otherwise.
     //!retcode: True if the application can invoke the redirect feature,
         //!retcode: false otherwise.



        UtlBoolean canAddToAddress();
     //:Returns true if the application can invoke the add to address feature, false otherwise.
     //!retcode: True if the application can invoke the add to address feature,
         //!retcode: false otherwise.



        UtlBoolean canAccept();
     //:Returns true if the application can invoke the accept feature, false otherwise.
     //!retcode: True if the application can invoke the accept feature,
         //!retcode: false otherwise.



        UtlBoolean canReject();
     //:Returns true if the application can invoke the reject feature, false otherwise.
     //!retcode: True if the application can invoke the reject feature,
         //!retcode: false otherwise.



        UtlBoolean canPark();
     //:Returns true if the application can invoke the park feature, false otherwise.
     //!retcode: True if the application can invoke the park feature,
         //!retcode: false otherwise.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtConnectionCapabilities_h_

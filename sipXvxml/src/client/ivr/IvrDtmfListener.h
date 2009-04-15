// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _IvrDtmfListener_h_
#define _IvrDtmfListener_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <tao/TaoAdaptor.h>
#include "VXIplatform.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class CallManager;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class IvrDtmfListener : public TaoAdaptor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/* ============================ CREATORS ================================== */

   IvrDtmfListener(VXIplatform *platform = NULL,
                   const UtlString& name = "IvrDtmflListener-%d");
   //:Default constructor

   IvrDtmfListener(const IvrDtmfListener& rIvrDtmfListener);
   //:Copy constructor

   virtual
      ~IvrDtmfListener();
   //:Destructor

/* ============================ MANIPULATORS ============================== */

   IvrDtmfListener& operator=(const IvrDtmfListener& rhs);
   //:Assignment operator

   virtual UtlBoolean handleMessage(OsMsg& rMsg);

   void setPlatform(VXIplatform *platform);

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   VXIplatform*            mpPlatform;

#ifdef TEST
   static bool sIsTested;
   //:Set to true after the tests for this class have been executed once

   void test();
   //:Verify assertions for this class

   // Test helper functions
   void testCreators();
   void testManipulators();
   void testAccessors();
   void testInquiry();

#endif //TEST
};

/* ============================ INLINE METHODS ============================ */

#endif  // _IvrDtmfListener_h_

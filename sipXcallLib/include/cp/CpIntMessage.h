//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _CpIntMessage_h_
#define _CpIntMessage_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>
#include <cp/CallManager.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class CpIntMessage : public OsMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    CpIntMessage(unsigned char messageSubtype = CallManager::CP_UNSPECIFIED,
       const intptr_t intValue = 0);
     //:Default constructor


   virtual
   ~CpIntMessage();
     //:Destructor

   virtual OsMsg* createCopy(void) const;

/* ============================ MANIPULATORS ============================== */


/* ============================ ACCESSORS ================================= */
        void getIntData(intptr_t& intValue) const;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        intptr_t mIntData;

   CpIntMessage(const CpIntMessage& rCpIntMessage);
     //:disable Copy constructor

   CpIntMessage& operator=(const CpIntMessage& rhs);
     //:disable Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _CpIntMessage_h_

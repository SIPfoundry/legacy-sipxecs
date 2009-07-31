//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _CpStringMessage_h_
#define _CpStringMessage_h_

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
class CpStringMessage : public OsMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    CpStringMessage(unsigned char messageSubtype = CallManager::CP_UNSPECIFIED,
       const char* str = NULL);
     //:Default constructor


   virtual
   ~CpStringMessage();
     //:Destructor

   virtual OsMsg* createCopy(void) const;

/* ============================ MANIPULATORS ============================== */


/* ============================ ACCESSORS ================================= */
        void getStringData(UtlString& str) const;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        UtlString stringData;

   CpStringMessage(const CpStringMessage& rCpStringMessage);
     //:disable Copy constructor

   CpStringMessage& operator=(const CpStringMessage& rhs);
     //:disable Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _CpStringMessage_h_

//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _TaoReference_h_
#define _TaoReference_h_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsRWMutex.h"
#include "TaoDefs.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Used to build the call originating part, establishes connection with the server
// through the TaoTransport. Maintains a db of listeners the client has registered.
class TaoReference
{
/* //////////////////////////// PRIVATE /////////////////////////////////// */
public:
/* ============================ CREATORS =============================== */
        TaoReference();

        TaoReference(const TaoReference& rTaoReference);
     //:Copy constructor (not implemented for this class)

        virtual ~TaoReference();

/* ============================ MANIPULATORS =========================== */
        TaoStatus release();
        TaoStatus reset();
        unsigned int add();

/* ============================ ACCESSORS ============================== */
        unsigned int getRef() { return mRef; }

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        unsigned int    mRef;
        OsRWMutex               mLock;          // mutex lock used to protect mTransactionCnt


};

#endif // _TaoReference_h_

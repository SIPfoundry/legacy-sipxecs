//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES

#ifdef TEST
#include <assert.h>
#include "utl/UtlMemCheck.h"
#endif //TEST

// APPLICATION INCLUDES
#include <cp/CpStringMessage.h>
#include <cp/CallManager.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpStringMessage::CpStringMessage(unsigned char messageSubtype, const char* str) :
OsMsg(OsMsg::PHONE_APP, messageSubtype)
{
        stringData.append(str);
}

// Copy constructor
CpStringMessage::CpStringMessage(const CpStringMessage& rCpStringMessage):
OsMsg(OsMsg::PHONE_APP, rCpStringMessage.getMsgType())
{
        stringData = rCpStringMessage.stringData;
}

// Destructor
CpStringMessage::~CpStringMessage()
{

}

OsMsg* CpStringMessage::createCopy(void) const
{
        return(new CpStringMessage(getMsgSubType(), stringData.data()));
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
CpStringMessage&
CpStringMessage::operator=(const CpStringMessage& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   OsMsg::operator=(rhs);
        stringData = rhs.stringData;

   return *this;
}

/* ============================ ACCESSORS ================================= */

void CpStringMessage::getStringData(UtlString& str) const
{
        str = stringData;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

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

// APPLICATION INCLUDES
#include <cp/CpIntMessage.h>
#include <cp/CallManager.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpIntMessage::CpIntMessage(unsigned char messageSubtype, intptr_t intData) :
OsMsg(OsMsg::PHONE_APP, messageSubtype)
{
        mIntData = intData;
}

// Copy constructor
CpIntMessage::CpIntMessage(const CpIntMessage& rCpIntMessage):
OsMsg(OsMsg::PHONE_APP, rCpIntMessage.getMsgType())
{
        mIntData = rCpIntMessage.mIntData;
}

// Destructor
CpIntMessage::~CpIntMessage()
{

}

OsMsg* CpIntMessage::createCopy(void) const
{
        return(new CpIntMessage(getMsgSubType(), mIntData));
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
CpIntMessage&
CpIntMessage::operator=(const CpIntMessage& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   OsMsg::operator=(rhs);
        mIntData = rhs.mIntData;

   return *this;
}

/* ============================ ACCESSORS ================================= */

void CpIntMessage::getIntData(intptr_t& intData) const
{
        intData = mIntData;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

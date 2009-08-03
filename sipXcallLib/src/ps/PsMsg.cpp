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
#include <assert.h>

// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "ps/PsMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Message object used to communicate phone set information

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PsMsg::PsMsg(int msg, void* source, const int param1, const int param2)
:  OsMsg(OsMsg::PS_MSG, msg),
   mInUse(FALSE),
   mMsgSource(source),
   mParam1(param1),
   mParam2(param2)
{
   // all of the work is done by the initializers
   memset(mStringParam1, 0, PSMSG_MAX_STRINGPARAM_LENGTH + 1);
   memset(mStringParam2, 0, PSMSG_MAX_STRINGPARAM_LENGTH + 1);
}

// Copy constructor
PsMsg::PsMsg(const PsMsg& rPsMsg)
:  OsMsg(rPsMsg)
{
   mMsgSource = rPsMsg.mMsgSource;
   mParam1    = rPsMsg.mParam1;
   mParam2    = rPsMsg.mParam2;
   memset(mStringParam1, 0, PSMSG_MAX_STRINGPARAM_LENGTH + 1);
   memset(mStringParam2, 0, PSMSG_MAX_STRINGPARAM_LENGTH + 1);

   if (rPsMsg.mStringParam1[0])
   {
           int len = strlen(rPsMsg.mStringParam1);

           if (len > PSMSG_MAX_STRINGPARAM_LENGTH) len = PSMSG_MAX_STRINGPARAM_LENGTH;
           strncpy(mStringParam1, rPsMsg.mStringParam1, len);
   }
   if (rPsMsg.mStringParam2[0])
   {
           int len = strlen(rPsMsg.mStringParam2);

           if (len > PSMSG_MAX_STRINGPARAM_LENGTH) len = PSMSG_MAX_STRINGPARAM_LENGTH;
           strncpy(mStringParam2, rPsMsg.mStringParam2, len);
   }
}

// Create a copy of this msg object (which may be of a derived type)
OsMsg* PsMsg::createCopy(void) const
{
   return new PsMsg(*this);
}

// Destructor
PsMsg::~PsMsg()
{
   memset(mStringParam1, 0, PSMSG_MAX_STRINGPARAM_LENGTH + 1);
   memset(mStringParam2, 0, PSMSG_MAX_STRINGPARAM_LENGTH + 1);
   // no other work required
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PsMsg&
PsMsg::operator=(const PsMsg& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   OsMsg::operator=(rhs);       // assign fields for parent class

   mMsgSource = rhs.mMsgSource;
   mParam1    = rhs.mParam1;
   mParam2    = rhs.mParam2;

   memset(mStringParam1, 0, PSMSG_MAX_STRINGPARAM_LENGTH + 1);
   memset(mStringParam2, 0, PSMSG_MAX_STRINGPARAM_LENGTH + 1);
   if (rhs.mStringParam1[0])
   {
           int len = strlen(rhs.mStringParam1);

           if (len > PSMSG_MAX_STRINGPARAM_LENGTH) len = PSMSG_MAX_STRINGPARAM_LENGTH;
           strncpy(mStringParam1, rhs.mStringParam1, len);
   }
   if (rhs.mStringParam2[0])
   {
           int len = strlen(rhs.mStringParam2);

           if (len > PSMSG_MAX_STRINGPARAM_LENGTH) len = PSMSG_MAX_STRINGPARAM_LENGTH;
           strncpy(mStringParam2, rhs.mStringParam2, len);
   }
   return *this;
}

// Set the message source
void PsMsg::setMsgSource(void* source)
{
   mMsgSource = source;
}

// Set parameter1 of the phone set message
void PsMsg::setParam1(int param1)
{
   mParam1 = param1;
}

// Set parameter2 of the phone set message
void PsMsg::setParam2(int param2)
{
   mParam2 = param2;
}

// Set string parameter1 of the phone set message
void PsMsg::setStringParam1(const char* param)
{
   if (param)
   {
           memset(mStringParam1, 0, PSMSG_MAX_STRINGPARAM_LENGTH + 1);
           int len = strlen(param);
           if (len > PSMSG_MAX_STRINGPARAM_LENGTH) len = PSMSG_MAX_STRINGPARAM_LENGTH;
           strncpy(mStringParam1, param, len);
   }
}

// Set string parameter2 of the phone set message
void PsMsg::setStringParam2(const char* param)
{
   if (param)
   {
           memset(mStringParam2, 0, PSMSG_MAX_STRINGPARAM_LENGTH + 1);
           int len = strlen(param);

           if (len > PSMSG_MAX_STRINGPARAM_LENGTH) len = PSMSG_MAX_STRINGPARAM_LENGTH;
           strncpy(mStringParam2, param, len);
   }
}

// Set the InUse flag for the message.
// For messages sent from an ISR, TRUE indicates that the receiver is
// not done with the message yet.  The InUse flag is ignored for
// messages that were not sent from an ISR.
void PsMsg::setInUse(UtlBoolean isInUse)
{
   mInUse = isInUse;
}

/* ============================ ACCESSORS ================================= */

// Return the type of the phone set message
int PsMsg::getMsg(void) const
{
   return OsMsg::getMsgSubType();
}

// Return the message source
void* PsMsg::getMsgSource(void) const
{
   return mMsgSource;
}

// Return parameter1 of the message
int PsMsg::getParam1(void) const
{
   return mParam1;
}

// Return parameter2 of the message
int PsMsg::getParam2(void) const
{
   return mParam2;
}

// Return string parameter1 of the message
void PsMsg::getStringParam1(UtlString& stringData)
{
        if (mStringParam1[0])
        {
                stringData.append(mStringParam1);
        }
}

// Return string parameter2 of the message
void PsMsg::getStringParam2(UtlString& stringData)
{
        if (mStringParam2[0])
                stringData.append(mStringParam2);
}

/* ============================ INQUIRY =================================== */

// Returns the value of the InUse flag for the message.
// For messages sent from an ISR, TRUE indicates that the receiver is
// not done with the message yet.  The InUse flag is ignored for
// messages that were not sent from an ISR.
UtlBoolean PsMsg::isInUse(void)
{
   return mInUse;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

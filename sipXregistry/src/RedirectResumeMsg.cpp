//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "RedirectResumeMsg.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

RedirectResumeMsg::RedirectResumeMsg(RedirectPlugin::RequestSeqNo seqNo,
                                     int redirectorNo) :
   OsMsg(REDIRECT_RESTART, 0),
   mSeqNo(seqNo),
   mRedirectorNo(redirectorNo)
{
}

// Create a copy of this msg object
RedirectResumeMsg* RedirectResumeMsg::createCopy(void) const
{
   return new RedirectResumeMsg(mSeqNo, mRedirectorNo);
}

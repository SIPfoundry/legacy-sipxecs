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
#include <net/SipInputProcessor.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
UtlContainableType SipInputProcessor::TYPE = "SipInputProcessor";

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
SipInputProcessor::SipInputProcessor( uint priority ) :
   mPriority( priority )
{
}

SipInputProcessor::~SipInputProcessor()
{
}

/* ============================ ACCESSORS ================================= */
uint SipInputProcessor::getPriority( void ) const
{
   return mPriority;
}

unsigned SipInputProcessor::hash() const
{
   return directHash();
}

UtlContainableType SipInputProcessor::getContainableType() const
{
    return SipInputProcessor::TYPE ;
}

/* ============================ INQUIRY =================================== */

int SipInputProcessor::compareTo(UtlContainable const * inVal) const
{
   // To allow SipInputProcessor to be containable in sorted list
   // ordered by priority, compare prioritries first.  If not equal,
   // return comparison result.  If equal then return comparison of
   // pointers.  Perform comparison so that Lower priority values
   // appear first in the sorted list.
   int result;
   uint otherPriority = ((SipInputProcessor*)(inVal))->getPriority();

   result =
      mPriority > otherPriority ? 1 :
      mPriority < otherPriority ? -1 :
      this > inVal ? 1 :
      this < inVal ? -1 :
      0;

   return result;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

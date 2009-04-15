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
#include <net/SipOutputProcessor.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
UtlContainableType SipOutputProcessor::TYPE = "SipOutputProcessor";

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
SipOutputProcessor::SipOutputProcessor( uint priority ) :
   mPriority( priority )
{
}

SipOutputProcessor::~SipOutputProcessor()
{
}

/* ============================ ACCESSORS ================================= */
uint SipOutputProcessor::getPriority( void ) const
{
   return mPriority;
}

unsigned SipOutputProcessor::hash() const
{
   return directHash();
}

UtlContainableType SipOutputProcessor::getContainableType() const
{
    return SipOutputProcessor::TYPE ;
}

/* ============================ INQUIRY =================================== */

int SipOutputProcessor::compareTo(UtlContainable const * inVal) const
{
   // To allow SipOutputProcessor to be containable in sorted list
   // ordered by priority, compare prioritries first.  If not equal,
   // return comparison result.  If equal then return comparison of 
   // pointers.  Perform comparison so that Lower priority values 
   // appear first in the sorted list.
   int result;
   uint otherPriority = ((SipOutputProcessor*)(inVal))->getPriority();
   
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

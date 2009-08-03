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
#include "ptapi/PtAddressForwarding.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PtAddressForwarding::PtAddressForwarding(const char* destinationURL,
                                                                                 int type,
                                                                                 int noAnswerTimeout)
{
        mDestinationUrl = UtlString(destinationURL);
        mForwardingType = type;
        mFilterType = ALL_CALLS;
        mNoAnswerTimeout = noAnswerTimeout;
}

PtAddressForwarding::PtAddressForwarding(const char* destinationURL,
                                                                                 int type,
                                                                                 PtBoolean internalCalls,
                                                                                 int noAnswerTimeout)
{
   mDestinationUrl = UtlString(destinationURL);
   mForwardingType = type;
   mNoAnswerTimeout = noAnswerTimeout;
   if (internalCalls)
           mFilterType = INTERNAL_CALLS;
   else
           mFilterType = ALL_CALLS;
}

PtAddressForwarding::PtAddressForwarding(const char* destinationURL,
                                                                                 int type,
                                                                                 const char* callerURL,
                                                                                 int noAnswerTimeout)
{
   mDestinationUrl = UtlString(destinationURL);
   mForwardingType = type;
   mNoAnswerTimeout = noAnswerTimeout;
   if (callerURL)
   {
           mFilterType = SPECIFIC_ADDRESS;
           mCallerUrl = UtlString(callerURL);
   }
   else
           mFilterType = ALL_CALLS;
}

PtAddressForwarding::PtAddressForwarding(const char* destinationURL,
                                                                                 int type,
                                                                                 int filterType,
                                                                                 const char* callerURL,
                                                                                 int noAnswerTimeout)
{
   mDestinationUrl = UtlString(destinationURL);
   mForwardingType = type;
   mFilterType = filterType;
   mNoAnswerTimeout = noAnswerTimeout;
   if (callerURL)
   {
           mCallerUrl = UtlString(callerURL);
   }
}

// Copy constructor
PtAddressForwarding::PtAddressForwarding(const PtAddressForwarding& rPtAddressForwarding)
{
        mDestinationUrl = rPtAddressForwarding.mDestinationUrl;
        mForwardingType = rPtAddressForwarding.mForwardingType;
        mFilterType = rPtAddressForwarding.mFilterType;
        mCallerUrl = rPtAddressForwarding.mCallerUrl;
    mNoAnswerTimeout = rPtAddressForwarding.mNoAnswerTimeout;
}

// Destructor
PtAddressForwarding::~PtAddressForwarding()
{
}


/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtAddressForwarding&
PtAddressForwarding::operator=(const PtAddressForwarding& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

        mDestinationUrl = rhs.mDestinationUrl;
        mForwardingType = rhs.mForwardingType;
        mFilterType = rhs.mFilterType;
        mCallerUrl = rhs.mCallerUrl;
    mNoAnswerTimeout = rhs.mNoAnswerTimeout;

        return *this;
}

PtBoolean
PtAddressForwarding::operator==(const PtAddressForwarding& rhs) const
{
   return
      mDestinationUrl == rhs.mDestinationUrl &&
      mForwardingType == rhs.mForwardingType &&
      mFilterType == rhs.mFilterType &&
      mCallerUrl == rhs.mCallerUrl &&
      mNoAnswerTimeout == rhs.mNoAnswerTimeout;
}

/* ============================ ACCESSORS ================================= */

PtStatus PtAddressForwarding::getDestinationAddress(char * address, int len)
{
        if (address && !mDestinationUrl.isNull())
        {
                int bytes = mDestinationUrl.length();

                bytes = (bytes < len) ? bytes : len;
                strncpy(address, mDestinationUrl.data(), bytes);
                address[bytes] = 0;
                return PT_SUCCESS;
        }
        else
                return PT_RESOURCE_UNAVAILABLE;
}

PtStatus PtAddressForwarding::getFilter(int& filterType)
{
        filterType = mFilterType;
        return PT_SUCCESS;
}

PtStatus PtAddressForwarding::getSpecificCaller(char* address, int len)
{
        if (address &&
                mFilterType == SPECIFIC_ADDRESS &&
                !mCallerUrl.isNull())
        {
                int bytes = mCallerUrl.length();

                bytes = (bytes < len) ? bytes : len;
                strncpy(address, mCallerUrl.data(), bytes);
                address[bytes] = 0;
                return PT_SUCCESS;
        }
        else
                return PT_RESOURCE_UNAVAILABLE;
}

PtStatus PtAddressForwarding::getType(int& type)
{
        type = mForwardingType;
        return PT_SUCCESS;
}

PtStatus PtAddressForwarding::getNoAnswerTimeout(int& time)
{
        time = mNoAnswerTimeout;
        return PT_SUCCESS;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
// Protected constructor.
/* //////////////////////////// PRIVATE /////////////////////////////////// */
PtAddressForwarding::PtAddressForwarding()
{
        mForwardingType = FORWARD_ON_BUSY;
        mFilterType = ALL_CALLS;
}

/* ============================ FUNCTIONS ================================= */

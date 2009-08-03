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
#include "tao/TaoEventListener.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
TaoEventListener::TaoEventListener(const char* pTerminalName, int* pEventMask)
{
        mpEventMask = pEventMask;
        mpTerminalName = NULL;

        if (pTerminalName)
        {
                int len = strlen(pTerminalName);

                mpTerminalName = new char[len + 1];
                if (pTerminalName)
                        strcpy(mpTerminalName, pTerminalName);
        }

}

// Copy constructor
TaoEventListener::TaoEventListener(const TaoEventListener& rTaoEventListener)
{
        if (rTaoEventListener.mpTerminalName)
        {
                int len = strlen(rTaoEventListener.mpTerminalName);

                mpTerminalName = new char[len + 1];
                strcpy(mpTerminalName, rTaoEventListener.mpTerminalName);
        }
        else
                mpTerminalName = 0;
}

// Destructor
TaoEventListener::~TaoEventListener()
{
        if (mpTerminalName)
        {
                delete[] mpTerminalName;
                mpTerminalName = 0;
        }
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
TaoEventListener&
TaoEventListener::operator=(const TaoEventListener& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

        if (mpTerminalName)
        {
                delete[] mpTerminalName;
                mpTerminalName = 0;
        }

        if (rhs.mpTerminalName)
        {
                int len = strlen(rhs.mpTerminalName);

                mpTerminalName = new char[len + 1];
                strcpy(mpTerminalName, rhs.mpTerminalName);
        }

   return *this;
}

/* ============================ ACCESSORS ================================= */

TaoStatus TaoEventListener::getEventMask(const int*& rpMask)
{
        rpMask = mpEventMask;
        return TAO_SUCCESS;
}

TaoStatus TaoEventListener::getTerminalName(char* rpTerminalName, int maxLen)
{
        if (rpTerminalName && maxLen > 0)
        {
                if (mpTerminalName)
                {
                        int bytes = strlen(mpTerminalName);
                        bytes = (bytes > maxLen) ? maxLen : bytes;

                        memset(rpTerminalName, 0, maxLen);
                        strncpy ((char *)rpTerminalName, mpTerminalName, bytes);
                        return TAO_SUCCESS;
                }
        }

        return TAO_FAILURE;
}

/* ============================ INQUIRY =================================== */

UtlBoolean TaoEventListener::isEventEnabled(PtEvent::PtEventId& eventId)
{
        return true;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

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
#include "ptapi/PtEvent.h"
#include "ptapi/PtTerminalEvent.h"
#include "ptapi/PtTerminal.h"
#include "tao/TaoClientTask.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PtTerminalEvent::PtTerminalEvent(PtEvent::PtEventId eventId) :
PtEvent(eventId)
{
        mpClient = 0;
    mpTerminalName = 0;

}

// Copy constructor
PtTerminalEvent::PtTerminalEvent(const PtTerminalEvent& rPtTerminalEvent)
{
        mpClient = rPtTerminalEvent.mpClient;

        if (rPtTerminalEvent.mpTerminalName)
        {
                int len = strlen(rPtTerminalEvent.mpTerminalName);
                mpTerminalName = new char[len + 1];
                strcpy(mpTerminalName, rPtTerminalEvent.mpTerminalName);
        }
        else
        {
                mpTerminalName = 0;
        }
}

// Destructor
PtTerminalEvent::~PtTerminalEvent()
{
        if (mpTerminalName)
        {
                delete[] mpTerminalName;
                mpTerminalName = 0;
        }
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtTerminalEvent&
PtTerminalEvent::operator=(const PtTerminalEvent& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

        if (mpTerminalName)
        {
                delete[] mpTerminalName;
        }

        if (rhs.mpTerminalName)
        {
                int len = strlen(rhs.mpTerminalName);

                mpTerminalName = new char[len + 1];
                strcpy(mpTerminalName, rhs.mpTerminalName);
                mpTerminalName[len] = 0;
        }
        else
        {
                mpTerminalName = 0;
        }

   return *this;
}

/* ============================ ACCESSORS ================================= */

PtStatus PtTerminalEvent::getTerminal(PtTerminal& rTerminal)
{
        PtTerminal terminal(mpTerminalName, mpClient);

        rTerminal = terminal;

        return PT_SUCCESS;
}

/* ============================ INQUIRY =================================== */
// PT_IMPLEMENT_CLASS_INFO(PtTerminalEvent, PtEvent)

/* //////////////////////////// PROTECTED ///////////////////////////////// */
PtTerminalEvent::PtTerminalEvent(PtEventId eventId, const char* terminalName, TaoClientTask *pClient)
:
PtEvent(eventId)
{
        mpClient = pClient;

        if (terminalName)
        {
                int len = strlen(terminalName);
                mpTerminalName = new char[len + 1];
                strcpy(mpTerminalName, terminalName);
        }
        else
        {
                mpTerminalName = 0;
        }
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

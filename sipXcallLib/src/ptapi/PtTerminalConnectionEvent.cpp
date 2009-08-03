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
#include <string.h>

// APPLICATION INCLUDES
#include "ptapi/PtTerminalConnectionEvent.h"
#include "tao/TaoClientTask.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PtTerminalConnectionEvent::PtTerminalConnectionEvent(PtEvent::PtEventId eventId) :
PtCallEvent(eventId)
{
        memset(mAddress, 0, 128);
        memset(mTerminalName, 0, 128);
        mpClient = NULL;
        mIsTermConnLocal = 1;
        mbGotTerminalConnection = FALSE;

}

PtTerminalConnectionEvent::PtTerminalConnectionEvent(PtEvent::PtEventId eventId,
                                                        int metaCode,
                                                        const char* terminalName,
                                                        const char* callId,
                                                        int sipResponseCode,
                                                        const char* sipResponseText,
                                                        const char* newCallId,
                                                        const char** oldCallIds,
                                                        int numOldCalls,
                                                        const char* address,
                                                        PtEvent::PtEventCause cause,
                                                        int isLocal,
                                                        TaoClientTask *pClient)
 : PtCallEvent(eventId, metaCode, callId, pClient, sipResponseCode, sipResponseText, newCallId, oldCallIds, numOldCalls)
{
        mEventCause = cause;
        mIsTermConnLocal = isLocal;
        setEventAddress(address);
        setEventTerminal(terminalName);

        mTerminalConnection = PtTerminalConnection(pClient,
                                                                                        address,
                                                                                        terminalName,
                                                                                        callId,
                                                                                        isLocal);
        mbGotTerminalConnection = TRUE;
}

PtTerminalConnectionEvent::PtTerminalConnectionEvent(TaoClientTask *pClient)
 : PtCallEvent(pClient)
{
        memset(mAddress, 0, 128);
        memset(mTerminalName, 0, 128);
        mIsTermConnLocal = -1;
        mbGotTerminalConnection = FALSE;
}

// Copy constructor
PtTerminalConnectionEvent::PtTerminalConnectionEvent(const PtTerminalConnectionEvent& rPtTerminalConnectionEvent)
: PtCallEvent(rPtTerminalConnectionEvent)
{
        mEventCause = rPtTerminalConnectionEvent.mEventCause;
        mIsTermConnLocal = rPtTerminalConnectionEvent.mIsTermConnLocal;
        setEventAddress(rPtTerminalConnectionEvent.mAddress);
        setEventTerminal(rPtTerminalConnectionEvent.mTerminalName);
        mbGotTerminalConnection = rPtTerminalConnectionEvent.mbGotTerminalConnection;
        mTerminalConnection = rPtTerminalConnectionEvent.mTerminalConnection;
}

// Destructor
PtTerminalConnectionEvent::~PtTerminalConnectionEvent()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtTerminalConnectionEvent&
PtTerminalConnectionEvent::operator=(const PtTerminalConnectionEvent& rhs)
{
        if (this == &rhs)            // handle the assignment to self case
                return *this;

        PtCallEvent::operator=(rhs);

        setEventAddress(rhs.mAddress);
        setEventTerminal(rhs.mTerminalName);
    mEventCause = rhs.mEventCause;
        mIsTermConnLocal = rhs.mIsTermConnLocal;
        mbGotTerminalConnection = rhs.mbGotTerminalConnection;
        mTerminalConnection = rhs.mTerminalConnection;

        return *this;
}

void PtTerminalConnectionEvent::setEventAddress(const char* address)
{
        if (address)
        {
                int len = strlen(address);
                if (len > 127)
                        len = 127;
                strncpy(mAddress, address, len);
                mAddress[len] = 0;
        }
        else
                memset(mAddress, 0, 128);
}

void PtTerminalConnectionEvent::setEventTerminal(const char* terminal)
{
        if (terminal)
        {
                int len = strlen(terminal);
                if (len > 127)
                        len = 127;
                strncpy(mTerminalName, terminal, len);
                mTerminalName[len] = 0;
        }
        else
                memset(mTerminalName, 0, 128);
}

void PtTerminalConnectionEvent::setEventTcLocal(int islocal)
{
        mIsTermConnLocal = islocal;
        PtEvent::setEventLocal(islocal);
}

/* ============================ ACCESSORS ================================= */
// Return the terminal connection object associated with this event
PtStatus PtTerminalConnectionEvent
::getTerminalConnection(PtTerminalConnection& rTerminalConnection)
{
        // TODO: unit test it later.
        /* algorithm:
        call the protected constructor of PtTerminalConnection
        then assign it to rTerminalConnection
        */
        if (!mbGotTerminalConnection)
        {
                mTerminalConnection = PtTerminalConnection(mpClient,
                                                                                                mAddress,
                                                                                                mTerminalName,
                                                                                                mCallId,
                                                                                                mIsTermConnLocal);
                mbGotTerminalConnection = TRUE;
        }

        rTerminalConnection = mTerminalConnection;

        return PT_SUCCESS;
}
/* ============================ INQUIRY =================================== */
// PT_IMPLEMENT_CLASS_INFO(PtTerminalConnectionEvent, PtCallEvent)

/* //////////////////////////// PROTECTED ///////////////////////////////// */
// Protected constructor.
/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

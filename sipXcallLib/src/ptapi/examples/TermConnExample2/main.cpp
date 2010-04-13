//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

// System includes
#include "os/OsDefs.h"

// PTAPI includes
#include <ptapi/PtTerminalConnectionListener.h>
#include <ptapi/PtProvider.h>
#include <ptapi/PtTerminal.h>
#include <ptapi/PtTerminalConnection.h>
#include <ptapi/PtTerminalConnectionEvent.h>

// My listener class derived from PtTerminalConnectionListener
class MyTerminalConnectionListener : public PtTerminalConnectionListener
{
public:
    MyTerminalConnectionListener(PtEventMask* pMask = NULL);
    ~MyTerminalConnectionListener();

    virtual void terminalConnectionCreated(const PtTerminalConnectionEvent& rEvent);
    virtual void terminalConnectionIdle(const PtTerminalConnectionEvent& rEvent);
    virtual void terminalConnectionRinging(const PtTerminalConnectionEvent& rEvent);
    virtual void terminalConnectionDropped(const PtTerminalConnectionEvent& rEvent);
    virtual void terminalConnectionUnknown(const PtTerminalConnectionEvent& rEvent);
    virtual void terminalConnectionHeld(const PtTerminalConnectionEvent& rEvent);
    virtual void terminalConnectionTalking(const PtTerminalConnectionEvent& rEvent);
    virtual void terminalConnectionInUse(const PtTerminalConnectionEvent& rEvent);
};

// Hardcoded terminal limit for simpler example
#define MAX_TERMINALS 100

// My simple PTAPI application
int main(int argc, char* argv[])
{
    PtStatus status;

    // Get the provider and cause provider to startup
    PtProvider provider;
    status = PtProvider::getProvider("fred", "password", "localhost", "",
        provider);

    // Instantiate my listener
    MyTerminalConnectionListener myListener;

    // Array of terminals with listener registered
    PtTerminal listeningTerminals[MAX_TERMINALS];
    int listeningTerminalCount = 0;
    const char* listeningTerminalName;
    const char* terminalName;

    int numTerminals;
    PtBoolean alreadyListening;
    int terminalIndex;
    int listeningTerminalIndex;

    // Loop to poll for new terminals
    while(1)
    {
        PtTerminal terminals[MAX_TERMINALS];

        // Get the known terminals from the provider
        status = provider.getTerminals(terminals, MAX_TERMINALS, numTerminals);

        for(terminalIndex = 0; terminalIndex < numTerminals;
            terminalIndex++)
        {
            alreadyListening = FALSE;
            terminals[terminalIndex].getName(terminalName);
            if(terminalName == NULL) continue;

            // Loop through terminals with which we have registered
            // the listener
            for(listeningTerminalIndex = 0;
                listeningTerminalIndex < listeningTerminalCount;
                listeningTerminalIndex++)
            {
                // Check if we have already registered the listener on
                // this terminal.
                listeningTerminals[listeningTerminalIndex].getName(listeningTerminalName);
                if(strcmp(listeningTerminalName, terminalName) == 0)
                {
                    alreadyListening = TRUE;
                    break;
                }
            }

            if(!alreadyListening)
            {
                // Register the listener with the terminal to get events
                status = terminals[terminalIndex].addCallListener(myListener);

                // Add this terminal to the array of terminals with listeners
                // registered
                listeningTerminals[listeningTerminalCount] =
                    terminals[terminalIndex];
                listeningTerminalCount++;

                printf("REGISTERED MY LISTENER WITH TERMINAL: %s\n",
                    terminalName);

            }

        } // End loop through array of known terminals

        Sleep(1000);
    }
    return(1);
}



// Implementation of my listener
MyTerminalConnectionListener::MyTerminalConnectionListener(PtEventMask* pMask):
PtTerminalConnectionListener(pMask)
{
}

MyTerminalConnectionListener::~MyTerminalConnectionListener()
{
}

void MyTerminalConnectionListener::terminalConnectionCreated(const PtTerminalConnectionEvent& rEvent)
{
    // Instantiate a PtTerminalConnection
    PtTerminalConnection terminalConnection;

    // Associate the PtTerminalConnection with the terminal connection
    // on which this event occured.
    rEvent.getTerminalConnection(terminalConnection);

    // Instantiate a PtTerminal
    PtTerminal terminal;

    // Associate the PtTerminal with the terminal which this
    // terminal connection is related
    terminalConnection.getTerminal(terminal);

    // Get the terminal name
    const char* terminalName;
    terminal.getName(terminalName);

    printf("TERMINAL CONNECTION CREATED for terminal: %s\n",
        terminalName);
}

void MyTerminalConnectionListener::terminalConnectionIdle(const PtTerminalConnectionEvent& rEvent)
{
    // Instantiate a PtTerminalConnection
    PtTerminalConnection terminalConnection;

    // Associate the PtTerminalConnection with the terminal connection
    // on which this event occured.
    rEvent.getTerminalConnection(terminalConnection);

    // Instantiate a PtTerminal
    PtTerminal terminal;

    // Associate the PtTerminal with the terminal which this
    // terminal connection is related
    terminalConnection.getTerminal(terminal);

    // Get the terminal name
    const char* terminalName;
    terminal.getName(terminalName);

    printf("TERMINAL CONNECTION IDLE for terminal: %s\n",
        terminalName);
}

void MyTerminalConnectionListener::terminalConnectionRinging(const PtTerminalConnectionEvent& rEvent)
{
    // Instantiate a PtTerminalConnection
    PtTerminalConnection terminalConnection;

    // Associate the PtTerminalConnection with the terminal connection
    // on which this event occured.
    rEvent.getTerminalConnection(terminalConnection);

    // Instantiate a PtTerminal
    PtTerminal terminal;

    // Associate the PtTerminal with the terminal which this
    // terminal connection is related
    terminalConnection.getTerminal(terminal);

    // Get the terminal name
    const char* terminalName;
    terminal.getName(terminalName);

    printf("TERMINAL CONNECTION RINGING for terminal: %s\n",
        terminalName);
}

void MyTerminalConnectionListener::terminalConnectionDropped(const PtTerminalConnectionEvent& rEvent)
{
    // Instantiate a PtTerminalConnection
    PtTerminalConnection terminalConnection;

    // Associate the PtTerminalConnection with the terminal connection
    // on which this event occured.
    rEvent.getTerminalConnection(terminalConnection);

    // Instantiate a PtTerminal
    PtTerminal terminal;

    // Associate the PtTerminal with the terminal which this
    // terminal connection is related
    terminalConnection.getTerminal(terminal);

    // Get the terminal name
    const char* terminalName;
    terminal.getName(terminalName);

    printf("TERMINAL CONNECTION DROPPED for terminal: %s\n",
        terminalName);
}

void MyTerminalConnectionListener::terminalConnectionUnknown(const PtTerminalConnectionEvent& rEvent)
{
    // Instantiate a PtTerminalConnection
    PtTerminalConnection terminalConnection;

    // Associate the PtTerminalConnection with the terminal connection
    // on which this event occured.
    rEvent.getTerminalConnection(terminalConnection);

    // Instantiate a PtTerminal
    PtTerminal terminal;

    // Associate the PtTerminal with the terminal which this
    // terminal connection is related
    terminalConnection.getTerminal(terminal);

    // Get the terminal name
    const char* terminalName;
    terminal.getName(terminalName);

    printf("TERMINAL CONNECTION UNKNOWN for terminal: %s\n",
        terminalName);
}

void MyTerminalConnectionListener::terminalConnectionHeld(const PtTerminalConnectionEvent& rEvent)
{
    // Instantiate a PtTerminalConnection
    PtTerminalConnection terminalConnection;

    // Associate the PtTerminalConnection with the terminal connection
    // on which this event occured.
    rEvent.getTerminalConnection(terminalConnection);

    // Instantiate a PtTerminal
    PtTerminal terminal;

    // Associate the PtTerminal with the terminal which this
    // terminal connection is related
    terminalConnection.getTerminal(terminal);

    // Get the terminal name
    const char* terminalName;
    terminal.getName(terminalName);

    printf("TERMINAL CONNECTION HELD for terminal: %s\n",
        terminalName);
}

void MyTerminalConnectionListener::terminalConnectionTalking(const PtTerminalConnectionEvent& rEvent)
{
    // Instantiate a PtTerminalConnection
    PtTerminalConnection terminalConnection;

    // Associate the PtTerminalConnection with the terminal connection
    // on which this event occured.
    rEvent.getTerminalConnection(terminalConnection);

    // Instantiate a PtTerminal
    PtTerminal terminal;

    // Associate the PtTerminal with the terminal which this
    // terminal connection is related
    terminalConnection.getTerminal(terminal);

    // Get the terminal name
    const char* terminalName;
    terminal.getName(terminalName);

    printf("TERMINAL CONNECTION TALKING for terminal: %s\n",
        terminalName);
}

void MyTerminalConnectionListener::terminalConnectionInUse(const PtTerminalConnectionEvent& rEvent)
{
    // Instantiate a PtTerminalConnection
    PtTerminalConnection terminalConnection;

    // Associate the PtTerminalConnection with the terminal connection
    // on which this event occured.
    rEvent.getTerminalConnection(terminalConnection);

    // Instantiate a PtTerminal
    PtTerminal terminal;

    // Associate the PtTerminal with the terminal which this
    // terminal connection is related
    terminalConnection.getTerminal(terminal);

    // Get the terminal name
    const char* terminalName;
    terminal.getName(terminalName);

    printf("TERMINAL CONNECTION IN USE for terminal: %s\n",
        terminalName);
}

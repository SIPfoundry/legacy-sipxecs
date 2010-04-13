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


// My simple PTAPI application
int main(int argc, char* argv[])
{
    PtStatus status;

    // Get the provider and cause provider to startup
    PtProvider provider;
    status = PtProvider::getProvider("fred", "password", "localhost", "",
        provider);

    // Get the terminal for a named softphone
    PtTerminal softphone;
    // You must modify sofphoneTerminalName to the ip address of
    // a softphone in your installation.
    char* softphoneTerminalName = "10.1.1.100";
    status = provider.getTerminal(softphoneTerminalName, softphone);

    // Instantiate my listener
    MyTerminalConnectionListener myListener;

    // Register the listener with the terminal to get events
    status = softphone.addCallListener(myListener);

    // Do not exit, let the provider and listener do stuff
    while(1)
    {
        Sleep(60000);
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
    printf("TERMINAL CONNECTION CREATED\n");
}

void MyTerminalConnectionListener::terminalConnectionIdle(const PtTerminalConnectionEvent& rEvent)
{
    printf("TERMINAL CONNECTION IDLE\n");
}

void MyTerminalConnectionListener::terminalConnectionRinging(const PtTerminalConnectionEvent& rEvent)
{
    printf("TERMINAL CONNECTION RINGING\n");
}

void MyTerminalConnectionListener::terminalConnectionDropped(const PtTerminalConnectionEvent& rEvent)
{
    printf("TERMINAL CONNECTION DROPPED\n");
}

void MyTerminalConnectionListener::terminalConnectionUnknown(const PtTerminalConnectionEvent& rEvent)
{
    printf("TERMINAL CONNECTION UNKNOWN\n");
}

void MyTerminalConnectionListener::terminalConnectionHeld(const PtTerminalConnectionEvent& rEvent)
{
    printf("TERMINAL CONNECTION HELD\n");
}

void MyTerminalConnectionListener::terminalConnectionTalking(const PtTerminalConnectionEvent& rEvent)
{
    printf("TERMINAL CONNECTION TALKING\n");
}

void MyTerminalConnectionListener::terminalConnectionInUse(const PtTerminalConnectionEvent& rEvent)
{
    printf("TERMINAL CONNECTION IN USE\n");
}

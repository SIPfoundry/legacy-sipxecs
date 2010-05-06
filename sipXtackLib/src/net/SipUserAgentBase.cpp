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
#include <utl/UtlVoidPtr.h>
#include <net/SipUserAgentBase.h>
#include <os/OsWriteLock.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipUserAgentBase::SipUserAgentBase(int sipTcpPort,
                                   int sipUdpPort,
                                   int sipTlsPort,
                                   int queueSize) :
    OsServerTask("SipUserAgent-%d", NULL, queueSize),
    mObserverMutex(OsRWMutex::Q_FIFO)
{
    mTcpPort = sipTcpPort;
    mUdpPort = sipUdpPort;
    mTlsPort = sipTlsPort;
    mMessageLogEnabled = TRUE;
}



// Destructor
SipUserAgentBase::~SipUserAgentBase()
{
}

/* ============================ MANIPULATORS ============================== */

void SipUserAgentBase::addConfigChangeConsumer(OsMsgQ& messageQueue)
{
    UtlVoidPtr* observer = new UtlVoidPtr(&messageQueue);
    OsWriteLock lock(mObserverMutex);
    mConfigChangeObservers.insert(observer);
}

/* ============================ ACCESSORS ================================= */

void SipUserAgentBase::getContactUri(UtlString* contactUri)
{
    contactUri->remove(0);
    contactUri->append(mContactURI);
}

/* ============================ INQUIRY =================================== */

UtlBoolean SipUserAgentBase::isReady()
{
    return(isStarted());
}

UtlBoolean SipUserAgentBase::waitUntilReady()
{
    // Lazy hack, should be a semaphore or event
    while(!isReady())
    {
        delay(500);
    }
    return(TRUE);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Assignment operator not implemented

// Copy constructor not implemented

/* ============================ FUNCTIONS ================================= */

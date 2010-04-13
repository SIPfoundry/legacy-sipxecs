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

// APPLICATION INCLUDES
#include "mi/CpMediaInterfaceFactoryImpl.h"
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpMediaInterfaceFactoryImpl::CpMediaInterfaceFactoryImpl()
    : mlockList(OsMutex::Q_FIFO)
{
    miStartRtpPort = 0 ;
    miLastRtpPort = 0;
    miNextRtpPort = miStartRtpPort ;
}

// Destructor
CpMediaInterfaceFactoryImpl::~CpMediaInterfaceFactoryImpl()
{
    OsLock lock(mlockList) ;

    mlistFreePorts.destroyAll() ;
}

/* =========================== DESTRUCTORS ================================ */

/**
 * public interface for destroying this media interface
 */
void CpMediaInterfaceFactoryImpl::release()
{
   delete this;
}

/* ============================ MANIPULATORS ============================== */


void CpMediaInterfaceFactoryImpl::setRtpPortRange(int startRtpPort, int lastRtpPort)
{
    miStartRtpPort = startRtpPort ;
    if (miStartRtpPort < 0)
    {
        miStartRtpPort = 0 ;
    }
    miLastRtpPort = lastRtpPort ;
    miNextRtpPort = miStartRtpPort ;
}


OsStatus CpMediaInterfaceFactoryImpl::getNextRtpPort(int &rtpPort)
{
    OsLock lock(mlockList) ;

    // First attempt to get a free port for the free list, if that
    // fails, return a new one.
    if (mlistFreePorts.entries())
    {
        UtlInt* pInt = (UtlInt*) mlistFreePorts.first() ;
        mlistFreePorts.remove(pInt) ;
        rtpPort = pInt->getValue() ;
        delete pInt ;
    }
    else
    {
        rtpPort = miNextRtpPort ;

        // Only allocate if the nextRtpPort is greater then 0 -- otherwise we
        // are allowing the system to allocate ports.
        if (miNextRtpPort > 0)
        {
            miNextRtpPort += 2 ;
        }
    }
    return OS_SUCCESS ;
}


OsStatus CpMediaInterfaceFactoryImpl::releaseRtpPort(const int rtpPort)
{
    OsLock lock(mlockList) ;

    // Only bother noting the free port if the next port isn't 0 (OS selects
    // port)
    if (miNextRtpPort != 0)
    {
        mlistFreePorts.insert(new UtlInt(rtpPort)) ;
    }

    return OS_SUCCESS ;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

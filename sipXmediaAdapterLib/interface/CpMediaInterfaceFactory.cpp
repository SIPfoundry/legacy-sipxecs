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
#include <stdlib.h>

// APPLICATION INCLUDES
#include "mi/CpMediaInterfaceFactory.h"
#include "mi/CpMediaInterfaceFactoryImpl.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpMediaInterfaceFactory::CpMediaInterfaceFactory()
    : mpFactoryImpl(NULL)
{
}

// Destructor
CpMediaInterfaceFactory::~CpMediaInterfaceFactory()
{
    setFactoryImplementation(NULL) ;
}

/* ============================ MANIPULATORS ============================== */

// Set the actual factory implementation
void CpMediaInterfaceFactory::setFactoryImplementation(CpMediaInterfaceFactoryImpl* pFactoryImpl)
{
    // Only bother if the pointers are different
    if (pFactoryImpl != mpFactoryImpl)
    {
        // Delete old version
        if (mpFactoryImpl)
        {
            mpFactoryImpl->release() ;
            mpFactoryImpl = NULL ;
        }

        // Set new version
        mpFactoryImpl = pFactoryImpl;
    }
}


// Create a media interface via the specified factory
CpMediaInterface* CpMediaInterfaceFactory::createMediaInterface(const char* publicAddress,
                                                                const char* localAddress,
                                                                int numCodecs,
                                                                SdpCodec* sdpCodecArray[],
                                                                const char* locale,
                                                                int expeditedIpTos,
                                                                const char* szStunServer,
                                                                int stunOptions,
                                                                int iStunKeepAlivePeriodSecs)
{
    CpMediaInterface* pInterface = NULL ;

    if (mpFactoryImpl)
    {
        pInterface = mpFactoryImpl->createMediaInterface(publicAddress,
                localAddress, numCodecs, sdpCodecArray, locale,
                expeditedIpTos, szStunServer, stunOptions, iStunKeepAlivePeriodSecs) ;
    }

    return pInterface ;
}

/* ============================ ACCESSORS ================================= */

CpMediaInterfaceFactoryImpl*
CpMediaInterfaceFactory::getFactoryImplementation()
{
    return mpFactoryImpl ;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include <assert.h>
#include <stdio.h>

// APPLICATION INCLUDES
#include "include/ConferenceEngineDatagramSocket.h"
#include <os/OsLock.h>
#include <os/OsStunQueryAgent.h>
#include <os/OsSysLog.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS
// FORWARD DECLARATIONS



/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
ConferenceEngineDatagramSocket::ConferenceEngineDatagramSocket(ConferenceEngine* pConferenceEngine,
                                                               int channel,
                                                               int type,
                                                               int remoteHostPortNum,
                                                               const char* remoteHost,
                                                               int localHostPortNum,
                                                               const char* localHost,
                                                               bool bEnable,
                                                               const char* szStunServer,
                                                               int iRefreshPeriodInSec)
        : OsStunDatagramSocket(remoteHostPortNum, remoteHost, localHostPortNum, localHost, bEnable, szStunServer, iRefreshPeriodInSec)
{
    mpConferenceEngine = pConferenceEngine ;
    miConferenceEngineChannel = channel ;
    miType = type ;
}


// Destructor
ConferenceEngineDatagramSocket::~ConferenceEngineDatagramSocket()
{
    mpConferenceEngine = NULL;
}

/* ============================ MANIPULATORS ============================== */

#define MAX_RTP_BYTES 1500
void ConferenceEngineDatagramSocket::pushPacket()
{
    char cBuf[MAX_RTP_BYTES] ;

    int bytes = read(cBuf, sizeof(cBuf)) ;
    if (bytes > 0)
    {
        if (miType == TYPE_RTP)
        {
            mpConferenceEngine->GIPSConf_ReceivedRTPPacket(miConferenceEngineChannel, cBuf, bytes) ;
        }
        else
        {
            mpConferenceEngine->GIPSConf_ReceivedRTCPPacket(miConferenceEngineChannel, cBuf, bytes) ;
        }
    }
}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

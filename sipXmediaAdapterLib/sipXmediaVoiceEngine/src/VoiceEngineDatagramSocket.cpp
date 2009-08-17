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
#include "include/VoiceEngineDatagramSocket.h"

#ifdef VIDEO
#ifdef _WIN32
    #include <windows.h>
    #include "GipsVideoEngineWindows.h"
#endif
#endif
#include "os/OsLock.h"
#include "os/OsStunQueryAgent.h"
#include "os/OsSysLog.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS
// FORWARD DECLARATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
VoiceEngineDatagramSocket::VoiceEngineDatagramSocket(GipsVoiceEngineLib* pVoiceEngine,
                                                     GipsVideoEngineWindows* pVideoEngine,
                                                     int audioChannel,
                                                     int videoChannel,
                                                     int type,
                                                     int remoteHostPortNum,
                                                     const char* remoteHost,
                                                     int localHostPortNum,
                                                     const char* localHost,
                                                     bool bEnable,
                                                     const char* szStunServer,
                                                     int iRefreshPeriodInSec,
                                                     int stunOptions)
        : OsStunDatagramSocket(remoteHostPortNum, remoteHost, localHostPortNum, localHost, bEnable, szStunServer, iRefreshPeriodInSec, stunOptions)
{
    mpVoiceEngine = pVoiceEngine ;
    mpVideoEngine = pVideoEngine ;
    miVoiceEngineChannel = audioChannel ;
    miVideoEngineChannel = videoChannel ;
    miType = type ;
}


// Destructor
VoiceEngineDatagramSocket::~VoiceEngineDatagramSocket()
{
    mpVoiceEngine = NULL ;
    mpVideoEngine = NULL ;
}

/* ============================ MANIPULATORS ============================== */

#define MAX_RTP_BYTES 4096
void VoiceEngineDatagramSocket::pushPacket()
{
#ifdef TRANSPORT_DEBUG
    int static count = 0 ;
#endif
    char cBuf[MAX_RTP_BYTES] ;

    int bytes = read(cBuf, sizeof(cBuf)) ;
    if (bytes > 0)
    {
        switch (miType)
        {
            case TYPE_AUDIO_RTP:
                mpVoiceEngine->GIPSVE_ReceivedRTPPacket(miVoiceEngineChannel, cBuf, bytes) ;
                break ;
            case TYPE_AUDIO_RTCP:
                mpVoiceEngine->GIPSVE_ReceivedRTCPPacket(miVoiceEngineChannel, cBuf, bytes) ;
                break ;
#ifdef VIDEO
            case TYPE_VIDEO_RTP:
                if (miVideoEngineChannel > -1)
                {
                    mpVideoEngine->GIPSVideo_ReceivedRTPPacket(miVideoEngineChannel, cBuf, bytes) ;
                }
                break ;
            case TYPE_VIDEO_RTCP:
                if (miVideoEngineChannel > -1)
                {
                    mpVideoEngine->GIPSVideo_ReceivedRTCPPacket(miVideoEngineChannel, cBuf, bytes) ;
                }
                break ;
#endif /* VIDEO */
        }

#ifdef TRANSPORT_DEBUG
        if (count % 100 == 0)
        {
            printf("TRANSPORT_DEBUG: pushPacket type=%d\n", miType) ;
        }
        count++ ;
#endif
    }
}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

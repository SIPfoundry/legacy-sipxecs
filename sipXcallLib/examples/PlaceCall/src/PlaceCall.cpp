//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#include <assert.h>

#if defined(_WIN32)
#  include <windows.h>
#  define SLEEP(milliseconds) Sleep(milliseconds)
#else
#  include <unistd.h>
#  define SLEEP(milliseconds) usleep((milliseconds)*1000)
#endif

#include "os/OsDefs.h"
#include "utl/UtlString.h"
#include "os/OsDateTime.h"
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"

#define MAX_RECORD_EVENTS       16

SIPX_INST g_hInst = NULL ;      // Handle to the sipXtapi instanance
SIPX_LINE g_hLine = 0 ;         // Line Instance (id, auth, etc)
SIPX_CALL g_hCall = 0 ;         // Handle to a call

bool g_timestamp = 0;           // TRUE if events should be timestamped

SIPX_CALLSTATE_EVENT    g_eRecordEvents[MAX_RECORD_EVENTS] ;    // List of last N events
int                     g_iNextEvent ;      // Index for g_eRecordEvents ringer buffer


// Print usage message
void usage(const char* szExecutable)
{
    char szBuffer[64];

    sipxConfigGetVersion(szBuffer, 64);
    printf("\nUsage:\n") ;
    printf("   %s <options> [URL]\n", szExecutable) ;
    printf("      using %s\n", szBuffer);
    printf("\n") ;
    printf("Options:\n") ;
    printf("   -d durationInSeconds (default=30 seconds)\n") ;
    printf("   -t play tones (default = none)\n") ;
    printf("   -f play file (default = none)\n") ;
    printf("   -p SIP port (default = 5060)\n") ;
    printf("   -r RTP port start (default = 9000)\n") ;
    printf("   -R use rport as part of via (disabled by default)\n") ;
    printf("   -u username (for authentication)\n") ;
    printf("   -a password  (for authentication)\n") ;
    printf("   -m realm  (for authentication)\n") ;
    printf("   -i from identity\n") ;
    printf("   -S stun server\n") ;
    printf("   -x proxy (outbound proxy)\n");
    printf("   -v show sipXtapi version\n");
    printf("   -c repeat count/Prank mode (call end point N times)\n") ;
    printf("   -I call input device name\n");
    printf("   -O call output device name\n");
    printf("   -C codec name\n");
    printf("   -L list all supported codecs\n");
    printf("   -T timestamp events\n");
    printf("\n") ;
}


// Parse arguments
bool parseArgs(int argc,
               char*  argv[],
               int*   pDuration,
               int*   pSipPort,
               int*   pRtpPort,
               char** pszPlayTones,
               char** pszFile,
               char** pszUrl,
               bool*  bUseRport,
               char** pszUsername,
               char** pszPassword,
               char** pszRealm,
               char** pszFromIdentity,
               char** pszStunServer,
               char** pszProxy,
               int*   pRepeatCount,
               char** pszInputDevice,
               char** pszOutputDevice,
               char** pszCodecName,
               bool*  bCodecList,
               bool*  bTimestamp)
{
    bool bRC = false ;
    char szBuffer[64];

    assert(pDuration && pszPlayTones && pszUrl) ;
    *pDuration = 30 ;
    *pSipPort = 5060 ;
    *pRtpPort = 9000 ;
    *pRepeatCount = 1 ;
    *pszPlayTones = NULL ;
    *pszFile = NULL ;
    *pszUrl = NULL ;
    *bUseRport = false ;
    *pszUsername = NULL ;
    *pszPassword = NULL ;
    *pszRealm = NULL ;
    *pszFromIdentity = NULL ;
    *pszStunServer = NULL ;
    *pszProxy = NULL;
    *pszInputDevice = NULL;
    *pszOutputDevice = NULL;
    *pszCodecName = NULL;
    *bCodecList = false;
    *bTimestamp = false;

    for (int i=1; i<argc; i++)
    {
        if (strcmp(argv[i], "-d") == 0)
        {
            if ((i+1) < argc)
            {
                *pDuration = atoi(argv[++i]) ;
            }
            else
            {
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-t") == 0)
        {
            if ((i+1) < argc)
            {
                *pszPlayTones = strdup(argv[++i]) ;
            }
            else
            {
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-p") == 0)
        {
            if ((i+1) < argc)
            {
                *pSipPort = atoi(argv[++i]) ;
            }
            else
            {
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-r") == 0)
        {
            if ((i+1) < argc)
            {
                *pRtpPort = atoi(argv[++i]) ;
            }
            else
            {
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-f") == 0)
        {
            if ((i+1) < argc)
            {
                *pszFile = strdup(argv[++i]) ;
            }
            else
            {
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-u") == 0)
        {
            if ((i+1) < argc)
            {
                *pszUsername = strdup(argv[++i]) ;
            }
            else
            {
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-a") == 0)
        {
            if ((i+1) < argc)
            {
                *pszPassword = strdup(argv[++i]) ;
            }
            else
            {
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-m") == 0)
        {
            if ((i+1) < argc)
            {
                *pszRealm = strdup(argv[++i]) ;
            }
            else
            {
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-i") == 0)
        {
            if ((i+1) < argc)
            {
                *pszFromIdentity = strdup(argv[++i]) ;
            }
            else
            {
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-x") == 0)
        {
            if ((i+1) < argc)
            {
                *pszProxy = strdup(argv[++i]) ;
            }
            else
            {
                bRC = false ;
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-S") == 0)
        {
            if ((i+1) < argc)
            {
                *pszStunServer = strdup(argv[++i]) ;
            }
            else
            {
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-R") == 0)
        {
            *bUseRport = true ;
        }
        else if (strcmp(argv[i], "-L") == 0)
        {
            *bCodecList = true ;
            bRC = true ;
        }
        else if (strcmp(argv[i], "-v") == 0)
        {
            sipxConfigGetVersion(szBuffer, 64);
            printf("%s\n", szBuffer);
            exit(0);
        }
        else if (strcmp(argv[i], "-c") == 0)
        {
            if ((i+1) < argc)
            {
                *pRepeatCount = atoi(argv[++i]) ;
            }
            else
            {
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-I") == 0)
        {
            if ((i+1) < argc)
            {
                *pszInputDevice = strdup(argv[++i]) ;
            }
            else
            {
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-O") == 0)
        {
            if ((i+1) < argc)
            {
                *pszOutputDevice = strdup(argv[++i]) ;
            }
            else
            {
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-C") == 0)
        {
            if ((i+1) < argc)
            {
                *pszCodecName = strdup(argv[++i]) ;
            }
            else
            {
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-T") == 0)
        {
            *bTimestamp = true;
        }
        else
        {
            if ((i+1) == argc)
            {
                *pszUrl = strdup(argv[i]) ;
                bRC = true ;
            }
            else
            {
                fprintf(stderr, "Unknown argument '%s'\n", argv[i]);
                break ; // Error
            }
        }
    }
    return bRC ;
}

bool EventCallBack(SIPX_EVENT_CATEGORY category,
                   void* pInfo,
                   void* pUserData)
{
    char cBuf[1024] ;

    // Print the timestamp if requested.
    if (g_timestamp)
    {
       OsDateTime d;
       OsDateTime::getCurTime(d);
       UtlString s;
       d.getIsoTimeStringZ(s);
       printf("%s ", s.data());
    }

    printf("%s\n", sipxEventToString(category, pInfo, cBuf, sizeof(cBuf))) ;

    if (category == EVENT_CATEGORY_CALLSTATE)
    {
        SIPX_CALLSTATE_INFO* pCallInfo = static_cast<SIPX_CALLSTATE_INFO*>(pInfo);
        printf("    hCall=%d, hAssociatedCall=%d\n", pCallInfo->hCall, pCallInfo->hAssociatedCall) ;

        switch (pCallInfo->event)
        {
        case REMOTE_OFFERING:
           // Get and print the To: URI.
        {
            char remote[200];
            sipxCallGetRemoteID(pCallInfo->hCall, remote, sizeof (remote));
            printf("    To: %s\n", remote);
        }
            break ;
        case CALLSTATE_AUDIO_EVENT:
           if (pCallInfo->cause == CALLSTATE_AUDIO_START)
           {
              printf("* Negotiated codec: %s, payload type %d\n", pCallInfo->codecs.audioCodec.cName, pCallInfo->codecs.audioCodec.iPayloadType);
           }
           break;
        default:
           // There are many other events which we ignore.
           break;
        }
        g_eRecordEvents[g_iNextEvent] = pCallInfo->event;
        g_iNextEvent = (g_iNextEvent + 1) % MAX_RECORD_EVENTS ;
    }

    // Ensure the output is not delayed by buffering.
    fflush(stdout);

    return true;
}

// Wait for the designated event for at worst ~iTimeoutInSecs seconds
bool WaitForSipXEvent(SIPX_CALLSTATE_MAJOR eMajor, int iTimeoutInSecs)
{
    bool bFound = false ;
    int  tries = 0;

    // Kids, don't try this at home -- This method of waiting for events is
    // not recommended.  Generally, most UAs are asynchronous and event
    // driven -- if you need to want for an event, build something that is
    // thread safe and doesn't use sleeps.
    while (!bFound)
    {
        for (int i=0;i<MAX_RECORD_EVENTS; i++)
        {
            if (g_eRecordEvents[i] == eMajor)
            {
                bFound = true ;
                break ;
            }
        }

        if (!bFound)
        {
            SLEEP(1000) ;
            if (++tries > (iTimeoutInSecs))
            {
                break ;
            }
        }
    }

    return bFound ;
}

// Clear the event log
void ClearSipXEvents()
{
    for (int i=0;i<MAX_RECORD_EVENTS; i++)
    {
        g_eRecordEvents[i] = CALLSTATE_UNKNOWN ;
    }
    g_iNextEvent = 0 ;
}


void dumpLocalContacts(SIPX_CALL hCall)
{
    SIPX_CONTACT_ADDRESS contacts[10] ;
    size_t nContacts;

    SIPX_RESULT status = sipxConfigGetLocalContacts(g_hInst, contacts, 10, nContacts) ;
    if (status == SIPX_RESULT_SUCCESS)
    {
        for (size_t i = 0; i<nContacts; i++)
        {
            const char* szType = "UNKNOWN" ;
            switch (contacts[i].eContactType)
            {
                case CONTACT_LOCAL:
                    szType = "LOCAL" ;
                    break ;
                case CONTACT_NAT_MAPPED:
                    szType = "NAT_MAPPED" ;
                    break ;
                case CONTACT_RELAY:
                    szType = "RELAY" ;
                    break ;
                case CONTACT_CONFIG:
                    szType = "CONFIG" ;
                    break ;
                case CONTACT_AUTO:
                    szType = "AUTO" ;
                    break ;
            }
            printf("<-> Type %s, Interface: %s, Ip %s, Port %d\n",
                    szType, contacts[i].cInterface, contacts[i].cIpAddress,
                    contacts[i].iPort) ;
        }
    }
    else
    {
        printf("<-> Unable to query local contact addresses\n") ;
    }
}


// Place a call to szSipUrl as szFromIdentity
bool placeCall(char* szSipUrl, char* szFromIdentity, char* szUsername, char* szPassword, char *szRealm)
{
    bool bRC = false ;

    if ((szFromIdentity == NULL) || strlen(szFromIdentity) == 0)
    {
        szFromIdentity = "\"PlaceCall Demo\" <sip:placecalldemo@localhost>" ;
    }

    printf("<-> Placing call to \"%s\" as \"%s\"\n", szSipUrl, szFromIdentity) ;
    printf("<-> Username: %s, passwd: %s, realm: %s (all required for auth)\n", szUsername, szPassword, szRealm) ;

    sipxLineAdd(g_hInst, szFromIdentity, &g_hLine) ;
    if (szUsername && szPassword && szRealm)
    {
        sipxLineAddCredential(g_hLine, szUsername, szPassword, szRealm) ;
    }
    sipxCallCreate(g_hInst, g_hLine, &g_hCall) ;
    dumpLocalContacts(g_hCall) ;
    sipxCallConnect(g_hCall, szSipUrl) ;
    bRC = WaitForSipXEvent(CONNECTED, 30) ;

    return bRC ;
}


// Drop call, clean up resources
bool shutdownCall()
{
    printf("<-> Shutting down Call\n") ;

    ClearSipXEvents() ;
    sipxCallDestroy(g_hCall) ;
    sipxLineRemove(g_hLine) ;

    WaitForSipXEvent(DESTROYED, 5) ;

    return true ;
}


// Play a series of tones
bool playTones(char* szPlayTones)
{
    bool bRC = true ;

    while (*szPlayTones)
    {
        int toneId = *szPlayTones++ ;

        if (    (toneId >= '0' && toneId <= '9') ||
                (toneId == '#') || (toneId == '*') || toneId == ',' || toneId == '!')
        {
            if (toneId == ',')
            {
                printf("<-> Playtone: Sleeping for 2 seconds\n") ;
                SLEEP(2000) ;
            }
            else
            {
                printf("<-> Playtone: %c\n", toneId) ;
                SLEEP(250) ;
                if (sipxCallStartTone(g_hCall, (TONE_ID) toneId, true, true) != SIPX_RESULT_SUCCESS)
                {
                    printf("Playtone returned error\n");
                }
                SLEEP(500) ;
                sipxCallStopTone(g_hCall) ;
            }
        }
        else
        {
            bRC = false ;
            break ;
        }

    }

    return bRC ;
}


// Play a file (8000 samples/sec, 16 bit unsigned, mono PCM)
bool playFile(char* szFile)
{
    sipxCallPlayFile(g_hCall, szFile, true, true) ;

    return true ;
}

// Display the list of input & output devices
void dumpInputOutputDevices()
{
    size_t numDevices ;

    if (sipxAudioGetNumInputDevices(g_hInst, numDevices) == SIPX_RESULT_SUCCESS)
    {
        printf("Input Devices: %d\n", numDevices) ;
        for (size_t i=0; i<numDevices; i++)
        {
            const char* szDevice ;
            sipxAudioGetInputDevice(g_hInst, i, szDevice) ;
            printf("\t#%d: %s\n", i, szDevice) ;
        }
    }

    if (sipxAudioGetNumOutputDevices(g_hInst, numDevices) == SIPX_RESULT_SUCCESS)
    {
        printf("Output Devices: %d\n", numDevices) ;
        for (size_t i=0; i<numDevices; i++)
        {
            const char* szDevice ;
            sipxAudioGetOutputDevice(g_hInst, i, szDevice) ;
            printf("\t#%d: %s\n", i, szDevice) ;
        }
    }

    // sipxAudioSetCallOutputDevice(g_hInst, "NONE") ;
    // sipxAudioSetCallInputDevice(g_hInst, "SigmaTel Audio") ;
}


int main(int argc, char* argv[])
{
    bool bError = false ;
    int iDuration, iSipPort, iRtpPort, iRepeatCount ;
    char* szPlayTones;
    char* szSipUrl;
    char* szFile;
    char* szUsername;
    char* szPassword;
    char* szRealm;
    char* szFromIdentity;
    char* szStunServer;
    char* szProxy;
    char* szOutDevice;
    char* szInDevice;
    char* szCodec;
    bool bUseRport ;
    bool bCList;

    // Parse Arguments
    if (parseArgs(argc, argv, &iDuration, &iSipPort, &iRtpPort, &szPlayTones,
                  &szFile, &szSipUrl, &bUseRport, &szUsername, &szPassword,
                  &szRealm, &szFromIdentity, &szStunServer, &szProxy,
                  &iRepeatCount, &szInDevice, &szOutDevice, &szCodec, &bCList,
                  &g_timestamp)
            && (iDuration > 0) && (portIsValid(iSipPort)) && (portIsValid(iRtpPort)))
    {
        // initialize sipx TAPI-like API
        sipxConfigSetLogLevel(LOG_LEVEL_DEBUG) ;
        sipxConfigSetLogFile("PlaceCall.log");
        sipxInitialize(&g_hInst, iSipPort, iSipPort, PORT_NONE, iRtpPort);
        sipxConfigEnableRport(g_hInst, bUseRport) ;
        dumpInputOutputDevices() ;
        sipxEventListenerAdd(g_hInst, EventCallBack, NULL) ;

        if (bCList)
        {
            int numAudioCodecs;
            int numVideoCodecs;
            int index;
            SIPX_AUDIO_CODEC audioCodec;
            SIPX_VIDEO_CODEC videoCodec;

            printf("Audio codecs:\n");
            if (sipxConfigGetNumAudioCodecs(g_hInst, &numAudioCodecs) == SIPX_RESULT_SUCCESS)
            {
                for (index=0; index<numAudioCodecs; ++index)
                {
                    if (sipxConfigGetAudioCodec(g_hInst, index, &audioCodec) == SIPX_RESULT_SUCCESS)
                    {
                        printf("  audio %02d : %s\n", index, audioCodec.cName);
                    }
                    else
                    {
                        printf("Error in retrieving audio codec #%d\n", index);
                    }
                }
            }
            else
            {
                printf("Error in retrieving number of audio codecs\n");
            }
            printf("Video codecs:\n");
            if (sipxConfigGetNumVideoCodecs(g_hInst, &numVideoCodecs) == SIPX_RESULT_SUCCESS)
            {
                for (index=0; index<numVideoCodecs; ++index)
                {
                    if (sipxConfigGetVideoCodec(g_hInst, index, &videoCodec) == SIPX_RESULT_SUCCESS)
                    {
                        printf("  video %02d : %s\n", index, videoCodec.cName);
                    }
                    else
                    {
                        printf("Error in retrieving video codec #%d\n", index);
                    }
                }
            }
            else
            {
                printf("Error in retrieving number of video codecs\n");
            }
            exit(0);
        }
        if (szProxy)
        {
            sipxConfigSetOutboundProxy(g_hInst, szProxy);
        }

        if (szStunServer)
        {
            sipxConfigEnableStun(g_hInst, szStunServer, 28) ;
        }
        if (szOutDevice)
        {
            if (sipxAudioSetCallOutputDevice(g_hInst, szOutDevice) != SIPX_RESULT_SUCCESS)
            {
                printf("!! Setting output device %s failed !!\n", szOutDevice);
            }
        }
        if (szInDevice)
        {
            if (sipxAudioSetCallInputDevice(g_hInst, szInDevice) != SIPX_RESULT_SUCCESS)
            {
                printf("!! Setting input device %s failed !!\n", szOutDevice);
            }
        }
        if (szCodec)
        {
            if (sipxConfigSetAudioCodecByName(g_hInst, szCodec) == SIPX_RESULT_FAILURE)
            {
                printf("!! Setting audio codec to %s failed !!\n", szCodec);
            };
        }
        // Wait for a STUN response (should actually look for the STUN event status
        // (config event) ;
        SLEEP(1500) ;


        for (int i=0; i<iRepeatCount; i++)
        {
            ClearSipXEvents() ;

            printf("<-> Attempt %d of %d\n", i+1, iRepeatCount) ;

            // Place a call to designed URL
            if (placeCall(szSipUrl, szFromIdentity, szUsername, szPassword, szRealm))
            {
                bError = false ;

                // Play tones if provided
                if (szPlayTones)
                {
                    if (!playTones(szPlayTones))
                    {
                        printf("%s: Failed to play tones: %s\n", argv[0], szPlayTones) ;
                        bError = true ;
                    }
                }

                // Play file if provided
                if (szFile)
                {
                    if (!playFile(szFile))
                    {
                        printf("%s: Failed to play file: %s\n", argv[0], szFile) ;
                        bError = true ;
                    }
                }

                // Leave the call up for specified time period (or wait for hangup)
                WaitForSipXEvent(DISCONNECTED, iDuration) ;

                // Shutdown / cleanup
                if (!shutdownCall())
                {
                    printf("%s: Failed to shutdown call\n", argv[0]) ;
                    bError = true ;
                }
            }
            else
            {
                printf("%s: Unable to complete call\n", argv[0]) ;
                shutdownCall() ;
                bError = true ;
            }

            if (bError)
            {
                break ;
            }
        }
        sipxEventListenerRemove(g_hInst, EventCallBack, NULL) ;
    }
    else
    {
        usage(argv[0]) ;
    }

    sipxUnInitialize(g_hInst);
    return (int) bError ;
}

#if !defined(_WIN32)
// Dummy definition of JNI_LightButton() to prevent the reference in
// sipXcallLib from producing an error.
void JNI_LightButton(long)
{
}
#endif /* !defined(_WIN32) */

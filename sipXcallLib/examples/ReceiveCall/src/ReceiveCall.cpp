//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#include "os/OsDefs.h"
#include "os/OsTimer.h"
#include "os/OsNotification.h"
#include "os/OsStatus.h"
#include "net/SipSubscriptionMgr.h"
#include "net/SipSubscribeServerEventHandler.h"
#include "net/SipPublishContentMgr.h"
#include "cp/DialogEventPublisher.h"
#include "cp/CallManager.h"
#include "tapi/sipXtapi.h"
#include "net/SipSubscribeServer.h"
#include "tapi/sipXtapiEvents.h"
#include "tapi/sipXtapiInternal.h"

#include <assert.h>
#include <limits.h>
#include <time.h>

// SLEEP sleeps for a given number of milliseconds.
#if defined(_WIN32)
#  include <windows.h>
#  define SLEEP(milliseconds) Sleep(milliseconds)
#else
#  include <unistd.h>
#  define SLEEP(milliseconds) usleep((milliseconds)*1000)
#endif

#include "os/OsDefs.h"
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"

#define SAMPLES_PER_FRAME   80          // Number of samples per frame time
#define LOOPBACK_LENGTH     200         // Frames for loopback delay (10ms per frame)

static short* g_loopback_samples[LOOPBACK_LENGTH] ; // loopback buffer
static short g_loopback_head = 0 ;      // index into loopback
static char* g_szPlayTones = NULL ;     // tones to play on answer
static char* g_szFile = NULL ;          // file to play on answer

bool g_timestamp = 0;           // TRUE if events should be timestamped

// Print usage message
void usage(const char* szExecutable)
{
    char szBuffer[64] = "";

    sipxConfigGetVersion(szBuffer, 64);
    printf("\nUsage:\n");
    printf("   %s <options>\n", szExecutable) ;
    printf("      using %s\n", szBuffer) ;
    printf("\n") ;
    printf("Options:\n") ;
    printf("   -d durationInSeconds (default=30 seconds)\n") ;
    printf("   -t playT tones (default = none)\n") ;
    printf("   -f play file (default = none)\n") ;
    printf("   -p SIP port (default = 5060)\n") ;
    printf("   -r RTP port start (default = 9000)\n") ;
    printf("   -l loopback audio (2 second delay)\n") ;
    printf("   -i line identity (e.g. sip:122@pingtel.com)\n") ;
    printf("   -u username (for authentication)\n") ;
    printf("   -a password  (for authentication)\n") ;
    printf("   -m realm  (for authentication)\n") ;
    printf("   -x proxy (outbound proxy)\n");
    printf("   -S stun server\n") ;
    printf("   -v show sipXtapi version\n");
    printf("   -e generate dialog events\n");
    printf("   -D delay before answering call (or list of delays)\n");
    printf("   -T timestamp events\n");
    printf("   -R time after which to terminate (seconds) (default = forever)\n");
    printf("\n") ;
}


// Parse arguments
bool parseArgs(int argc,
               char *argv[],
               int* pDuration,
               int* pSipPort,
               int* pRtpPort,
               char** pszPlayTones,
               char** pszFile,
               bool* bLoopback,
               char** pszIdentity,
               char** pszUsername,
               char** pszPassword,
               char** pszRealm,
               char** pszStunServer,
               char** pszProxy,
               bool* bDialogEvents,
               char** pszDelay,
               bool* bTimestamp,
               long int* pRunTime)
{
    bool bRC = true ;
    char szBuffer[64];

    assert(pDuration && pszPlayTones) ;
    *pDuration = 30 ;
    *pSipPort = 5060 ;
    *pRtpPort = 9000 ;
    *pszPlayTones = NULL ;
    *pszFile = NULL ;
    *bLoopback = false ;
    *pszIdentity = NULL ;
    *pszUsername = NULL ;
    *pszPassword = NULL ;
    *pszRealm = NULL ;
    *pszStunServer = NULL ;
    *pszProxy = NULL;
    *bDialogEvents = false;
    *bTimestamp = false;
    *pszDelay = NULL;

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
                bRC = false ;
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
                bRC = false ;
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
        else if (strcmp(argv[i], "-p") == 0)
        {
            if ((i+1) < argc)
            {
                *pSipPort = atoi(argv[++i]) ;
            }
            else
            {
                bRC = false ;
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
                bRC = false ;
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-l") == 0)
        {
            *bLoopback = true ;
        }
        else if (strcmp(argv[i], "-i") == 0)
        {
            if ((i+1) < argc)
            {
                *pszIdentity = strdup(argv[++i]) ;
            }
            else
            {
                bRC = false ;
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
                bRC = false ;
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
                bRC = false ;
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
                bRC = false ;
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
        else if (strcmp(argv[i], "-v") == 0)
        {
            sipxConfigGetVersion(szBuffer, 64);
            printf("%s\n", szBuffer);
            exit(0);
        }
        else if (strcmp(argv[i], "-e") == 0)
        {
            *bDialogEvents = true;
        }
        else if (strcmp(argv[i], "-D") == 0)
        {
            if ((i+1) < argc)
            {
                *pszDelay = strdup(argv[++i]);
            }
            else
            {
                bRC = false ;
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-T") == 0)
        {
            *bTimestamp = true;
        }
        else if (strcmp(argv[i], "-R") == 0)
        {
            if ((i+1) < argc)
            {
                *pRunTime = atoi(argv[++i]) ;
            }
            else
            {
                bRC = false ;
                break ; // Error
            }
        }
        else
        {
            fprintf(stderr, "Unknown argument '%s'\n", argv[i]);
            bRC = false ;
            break ; // Error
        }
    }

    return bRC ;
}

// Play a file (8000 samples/sec, 16 bit unsigned, mono PCM)
bool playFile(char* szFile, SIPX_CALL hCall)
{
    bool bRC = false ;
    sipxCallPlayFile(hCall, g_szFile, true, true) ;

    return true ;
}

// Play a series of tones
bool playTones(char* szPlayTones, SIPX_CALL hCall)
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
                sipxCallStartTone(hCall, (TONE_ID) toneId, true, false) ;
                SLEEP(500) ;
                sipxCallStopTone(hCall) ;
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


void SpkrAudioHook(const int nSamples, short* pSamples)
{
    memcpy(g_loopback_samples[g_loopback_head], pSamples, sizeof(short) * SAMPLES_PER_FRAME) ;
    g_loopback_head = ((g_loopback_head + 1) % LOOPBACK_LENGTH) ;
    memset(pSamples, 0, sizeof(short) * SAMPLES_PER_FRAME) ;
}


void MicAudioHook(const int nSamples, short* pSamples)
{
    short index = ((g_loopback_head + 1) % LOOPBACK_LENGTH) ;
    memcpy(pSamples, g_loopback_samples[index], sizeof(short) * SAMPLES_PER_FRAME) ;
}


void clearLoopback()
{
    for (int i=0; i<LOOPBACK_LENGTH; i++)
    {
        if (g_loopback_samples[i])
        {
            memset(g_loopback_samples[i], 0, sizeof(short) * SAMPLES_PER_FRAME) ;
        }
    }
    g_loopback_head = 0 ;
}


void initLoopback()
{
    for (int i=0; i<LOOPBACK_LENGTH; i++)
    {
        g_loopback_samples[i] = new short[SAMPLES_PER_FRAME] ;
    }
    clearLoopback() ;

    sipxConfigSetSpkrAudioHook(SpkrAudioHook) ;
    sipxConfigSetMicAudioHook(MicAudioHook) ;
}

// Machinery to provide delay when answering calls.

// Answering delay in seconds.
// If NULL or empty, answer calls immediately.
// If a digit string, answer calls after that many seconds.
// If a digit string followed by a comma, answer the next call after that
// many seconds, then delete the characters up through the comma to set
// the next value.
char* g_callAnswerDelay = NULL;

class CallAnswerNotification : public OsNotification
{
public:

   virtual OsStatus signal(const int eventData);
   //:Signal the occurrence of the event

   void setHCall(SIPX_CALL hCall);
   //:Store the SIPX_CALL value of the call to be answered.
   SIPX_CALL getHCall();
   //:Get the SIPX_CALL value of the call to be answered.

private:

   SIPX_CALL mHCall;
   //:The SIPX_CALL value of the call to be answered.
};

// Called when the callAnswerTimer fires.
OsStatus CallAnswerNotification::signal(int eventData)
{
   return sipxCallAnswer(mHCall) == SIPX_RESULT_SUCCESS ?
      OS_SUCCESS :
      OS_FAILED;
}

// Store the SIPX_CALL value of the call to be answered.
void CallAnswerNotification::setHCall(SIPX_CALL hCall)
{
   mHCall = hCall;
}

// Get the SIPX_CALL value of the call to be answered.
SIPX_CALL CallAnswerNotification::getHCall()
{
   return mHCall;
}

// Set up timer and notifier for answering calls.
CallAnswerNotification callAnswerNotification;
OsTimer callAnswerTimer(callAnswerNotification);

// Machinery to terminate calls after iDuration seconds.

// Call length limit in seconds.
int iDuration = 30;

class CallHangupNotification : public OsNotification
{
public:

   virtual OsStatus signal(const int eventData);
   //:Signal the occurrence of the event

   void setHCall(SIPX_CALL hCall);
   //:Store the SIPX_CALL value of the call to be hung up..
   SIPX_CALL getHCall();
   //:Get the SIPX_CALL value of the call to be hung up..

private:

   SIPX_CALL mHCall;
   //:The SIPX_CALL value of the call to be hung up.
};

// Called when the callHangupTimer fires.
OsStatus CallHangupNotification::signal(int eventData)
{
   return sipxCallDestroy(mHCall) == SIPX_RESULT_SUCCESS ?
      OS_SUCCESS :
      OS_FAILED;
}

// Store the SIPX_CALL value of the call to be answered.
void CallHangupNotification::setHCall(SIPX_CALL hCall)
{
   mHCall = hCall;
}

// Get the SIPX_CALL value of the call to be answered.
SIPX_CALL CallHangupNotification::getHCall()
{
   return mHCall;
}

// Set up timer and notifier for hanging up calls.
CallHangupNotification callHangupNotification;
OsTimer callHangupTimer(callHangupNotification);

bool EventCallBack(SIPX_EVENT_CATEGORY category,
                   void* pInfo,
                   void* pUserData)
{
    assert (pInfo != NULL);

    // Dump event
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
        case CALLSTATE_OFFERING:
           // Get and print the From: URI.
        {
            char remote[200];
            sipxCallGetRemoteID(pCallInfo->hCall, remote, sizeof (remote));
            printf("    From: %s\n", remote);
        }
            sipxCallAccept(pCallInfo->hCall) ;
            break ;
        case CALLSTATE_ALERTING:
            clearLoopback() ;
            {
               // Determine the answering delay.
               int delay;
               if (g_callAnswerDelay == NULL || g_callAnswerDelay[0] == '\0')
               {
                  // No answer string is active, so delay is 0.
                  delay = 0;
               }
               else
               {
                  // Get the delay to be used from the delay string.
                  delay = atoi(g_callAnswerDelay);
                  // Remove the first number, if there is a comma.
                  char* p = strchr(g_callAnswerDelay, ',');
                  if (p != NULL)
                  {
                     g_callAnswerDelay = p + 1;
                  }
               }
               if (delay == 0)
               {
                  // If the delay is 0, answer immediately.
                  sipxCallAnswer(pCallInfo->hCall);
               }
               else
               {
                  // The delay is non-0, so set the timer to answer.
                  // Stop the timer in case it is running.
                  callAnswerTimer.stop();
                  // Record the call to be answered.
                  callAnswerNotification.setHCall(pCallInfo->hCall);
                  // Construct the delay to be used.
                  OsTime d(delay, 0);
                  // Start the timer.
                  callAnswerTimer.oneshotAfter(d);
               }
            }
            break ;
        case CALLSTATE_CONNECTED:
           // Per conversation with Bob A., commented this sleep out
           // to prevent trouble with processing re-INVITEs that come
           // just after INVITEs.  This should be replaced with a proper
           // timer-wait event.  But that leads to the open question
           // of whether to use the sipX OsTimer system, or the underlying
           // OS mechanisms (to avoid dependency on sipXportLib).  Ugh.
           //SLEEP(1000) ;   // BAD: Do not block the callback thread

           // Start the timer that limits the length of the call.
        {
           // Stop the timer in case it is running.
           callHangupTimer.stop();
           // Record the call to be answered.
           callHangupNotification.setHCall(pCallInfo->hCall);
           // Start the timer, scaled to seconds.
           OsTime delay(iDuration, 0);
           callHangupTimer.oneshotAfter(delay);
        }

            // Play file if provided
            if (g_szFile)
            {
                if (!playFile(g_szFile, pCallInfo->hCall))
                {
                    printf("Failed to play file: %s\n", g_szFile) ;
                }
            }

            // Play tones if provided
            if (g_szPlayTones)
            {
                if (!playTones(g_szPlayTones, pCallInfo->hCall))
                {
                    printf("Failed to play tones: %s\n", g_szPlayTones) ;
                }
            }
            break ;
        case CALLSTATE_DISCONNECTED:
           // Stop the timers in case they are running.
           callAnswerTimer.stop();
           callHangupTimer.stop();
           // Destroy the call.
           sipxCallDestroy(pCallInfo->hCall) ;
           break ;
        case CALLSTATE_AUDIO_EVENT:
            if (pCallInfo->cause == CALLSTATE_AUDIO_START)
            {
                printf("* Negotiated codec: %s, payload type %d\n", pCallInfo->codecs.audioCodec.cName, pCallInfo->codecs.audioCodec.iPayloadType);
            }
            break;
        case CALLSTATE_DESTROYED:
           // Stop the timer in case it is running.
           callAnswerTimer.stop();
           break ;
        default:
           // There are many other events which we ignore.
           break;
        }
    }

    // Ensure the output is not delayed by buffering.
    fflush(stdout);

    return true;
}


SIPX_LINE lineInit(SIPX_INST hInst, char* szIdentity, char* szUsername, char* szPassword, char* szRealm)
{
    SIPX_LINE hLine = SIPX_LINE_NULL;

    if (szIdentity && strlen(szIdentity))
    {
        sipxLineAdd(hInst, szIdentity, &hLine) ;

        if (    szUsername && strlen(szUsername) &&
            szPassword && strlen(szPassword) &&
            szRealm && strlen(szRealm))
        {
            sipxLineAddCredential(hLine, szUsername, szPassword, szRealm) ;
            sipxLineRegister(hLine, true);
        }
    }
    else
    {
        sipxLineAdd(hInst, "sip:receivecall@localhost", &hLine) ;
    }

    return hLine ;
}

void lineShutdown(SIPX_INST hInst, SIPX_LINE hLine)
{
   // Terminate our registration.
   sipxLineRegister(hLine, false);
}

void dumpLocalContacts(SIPX_INST hInst)
{
    SIPX_CONTACT_ADDRESS contacts[10] ;
    size_t nContacts;

    SIPX_RESULT status = sipxConfigGetLocalContacts(hInst, contacts, 10, nContacts) ;
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


int main(int argc, char* argv[])
{
    bool bError = true ;
    int iSipPort, iRtpPort ;
    bool bLoopback ;
    char* szIdentity ;
    char* szUsername ;
    char* szPassword ;
    char* szRealm ;
    char* szStunServer ;
    char* szProxy ;
    bool bDialogEvents ;
    // The default run time is 2^31-1 seconds, which is over 68 years.
    long int runTime = LONG_MAX;
    SIPX_INST hInst ;
    SIPX_LINE hLine ;
    // Support for the dialog event notifier.
    // Component for holding the subscription data
    SipSubscriptionMgr* pSubscriptionMgr;
    // Component for granting the subscription rights
    SipSubscribeServerEventHandler* pPolicyHolder;
    // Component for publishing the event contents
    SipPublishContentMgr* pPublisher;
    SipSubscribeServer* pSubscribeServer;
    // The dialog event publisher
    DialogEventPublisher* pDialogEvents;

    // Parse Arguments
    if (parseArgs(argc, argv, &iDuration, &iSipPort, &iRtpPort, &g_szPlayTones,
                  &g_szFile, &bLoopback, &szIdentity, &szUsername, &szPassword,
                  &szRealm, &szStunServer, &szProxy, &bDialogEvents,
                  &g_callAnswerDelay, &g_timestamp, &runTime) &&
        (iDuration > 0) && (portIsValid(iSipPort)) && (portIsValid(iRtpPort)))
    {
        if (bLoopback)
        {
            initLoopback() ;
        }

        // initialize sipx TAPI-like API
        sipxConfigSetLogLevel(LOG_LEVEL_DEBUG) ;
        sipxConfigSetLogFile("ReceiveCall.log");
        if (sipxInitialize(&hInst, iSipPort, iSipPort, 5061, iRtpPort, 16, szIdentity) == SIPX_RESULT_SUCCESS)
        {
           // Start dialog event notifier if requested.
           if (bDialogEvents)
           {
              // Get pointer to the call manager.
              CallManager* pCallManager =
                 ((SIPX_INSTANCE_DATA*) hInst)->pCallManager;
              SipUserAgent* pUserAgent =
                 ((SIPX_INSTANCE_DATA*) hInst)->pSipUserAgent;

              // Start the SIP Subscribe Server
              pSubscriptionMgr = new SipSubscriptionMgr();
              pPolicyHolder = new SipSubscribeServerEventHandler;
              pPublisher = new SipPublishContentMgr;
              pSubscribeServer =
                 new SipSubscribeServer(SipSubscribeServer::terminationReasonNoresource,
                                        *pUserAgent,
                                        *pPublisher,
                                        *pSubscriptionMgr,
                                        *pPolicyHolder);
              pSubscribeServer->enableEventType(DIALOG_EVENT_TYPE,
                                                NULL,
                                                NULL,
                                                NULL,
                                                NULL,
                                                SipSubscribeServer::standardVersionCallback);
              pSubscribeServer->start();

              // Create the dialog event publisher
              pDialogEvents = new DialogEventPublisher(pCallManager,
                                                       pPublisher);
              pCallManager->addTaoListener(pDialogEvents);
              pDialogEvents->start();
           }

            if (szProxy)
            {
                sipxConfigSetOutboundProxy(hInst, szProxy);
            }
            sipxConfigEnableRport(hInst, true) ;
            if (szStunServer)
            {
                sipxConfigEnableStun(hInst, szStunServer, 28) ;
            }
            sipxEventListenerAdd(hInst, EventCallBack, NULL) ;
            hLine = lineInit(hInst, szIdentity, szUsername, szPassword, szRealm) ;

            dumpLocalContacts(hInst) ;

            // Run as long as requested.
            for (long int i = 0; i < runTime; i++)
            {
                SLEEP(1000) ;
            }

            lineShutdown(hInst, hLine);
            // Wait a bit, so that we can re-send the REGISTER if it needs
            // authorization.
            SLEEP(5000);
        }
        else
        {
            printf("unable to initialize sipXtapi layer\n") ;
        }
    }
    else
    {
        usage(argv[0]) ;
    }

    return (int) bError ;
}

#if !defined(_WIN32)
// Dummy definition of JNI_LightButton() to prevent the reference in
// sipXcallLib from producing an error.
void JNI_LightButton(long)
{

}

#endif /* !defined(_WIN32) */

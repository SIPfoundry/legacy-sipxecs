/****************License************************************************
 *
 * Copyright 2001.  SpeechWorks International, Inc.
 *
 * Use of this software is subject to notices and obligations set forth
 * in the SpeechWorks Public License - Software Version 1.1 which is
 * included with this software.
 * 
 * SpeechWorks is a registered trademark, and SpeechWorks Here, 
 * DialogModules and the SpeechWorks logo are trademarks of SpeechWorks 
 * International, Inc. in the United States and other countries. 
 * 
 ***********************************************************************
 *
 * SBinet Implementation
 *
 * 
 *
 ***********************************************************************/
#ifndef _SB_USE_STD_NAMESPACE
#define _SB_USE_STD_NAMESPACE
#endif

#ifdef WIN32
    #ifndef UNICODE
        #define UNICODE
    #endif
    #ifndef _UNICODE
        #define _UNICODE
    #endif
#endif /* WIN32 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <wctype.h>

#include <WWWLib.h>
#include <WWWHTTP.h>
#include <WWWInit.h>
#include <HTUtils.h>
#include <WWWSSL.h>

#ifdef EINVAL
// Conflicts with OS definition
    #undef EINVAL
#endif

#define OSBINET_EXPORTS
#include "VXItypes.h"
#include "VXIvalue.h"
#include "VXIinet.h"
#ifdef OPENVXI
    #define SBINET_API static
#else
    #define SBINET_EXPORTS
    #include "SBinet.h"
#endif
#include "OSBinet.h"
#include "VXItrd.h"
#include "VXIlog.h"

#ifdef WIN32
    #undef HTTP_VERSION
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <wininet.h>
#endif /* WIN32 */

#include "SBHTEvtLst.h"
#include "SBinetLog.h"
#include "SBinetURL.h"
#include "SBinetStream.h"
#include "SBinetChannel.h"
#include "SBinetCookie.h"
#include "SBinetCacheLock.h"


class SBinet : public SBinetLogger
{
public:
    SBinet( VXIlogInterface *log,
            VXIunsigned      diagLogBase );
    virtual ~SBinet();

    VXIinetResult  Init(const VXIchar*   pszCacheFolder, 
                        const VXIint     nCacheTotalSizeMB,
                        const VXIint     nCacheEntryMaxSizeMB,
                        const VXIint     nCacheEntryExpTimeSec,
                        const VXIchar*   pszProxyServer,
                        const VXIulong   nProxyPort,
                        const VXIchar*   userAgentName,
                        const VXIMap*    extensionRules);
    VXIinetResult  Terminate(VXIbool     clearLogResource);

    VXItrdResult   LockLibwww()
    {
        return VXItrdMutexLock(m_pLibwwwMutex);
    }
    VXItrdResult   UnlockLibwww()
    {
        return VXItrdMutexUnlock(m_pLibwwwMutex);
    }
    const VXIString* mapExtension(const VXIchar* ext)
    {
        if ( m_extensionRules == NULL ) return(NULL);
        if ( ext == NULL ) return(NULL);
        return((const VXIString*) VXIMapGetProperty(m_extensionRules,ext));
    }

private:
    VXIinetResult  CheckEnvironment();
    VXIinetResult  InitializeCache();
    VXIinetResult  InitializeProxy();
    VXIinetResult  TerminateCache();

    static VXITRD_DEFINE_THREAD_FUNC( EventThreadMain, userData );

    static BOOL
    CacheLockRemovalConfirmCallback(HTRequest *request, HTAlertOpcode op,
                                    int msgnum, const char *deflt, 
                                    void *input, HTAlertPar *reply);

    static int PrintCallback(const char * fmt, va_list pArgs);
    static int TraceCallback(const char * fmt, va_list pArgs);
    static int TraceDataCallback(char * data, size_t len, char * fmt, 
                                 va_list pArgs);

    static BOOL AlertCallback( HTRequest*    pHtRequest, 
                               HTAlertOpcode eOpCode,
                               int           nMessageNum,
                               const char*   pszDefault, 
                               void*         pvInput,
                               HTAlertPar*   pReply     );
    static void OutOfMemoryExitCallback(char *name, char *file, 
                                        unsigned long line);

private:
    VXItrdThread* m_pEventThread;
    VXItrdMutex* m_pLibwwwMutex;
    VXItrdTimer* m_pEventTimer;
    VXIulong m_nSleepInterval;    // 5 second sleep interval
    VXIbool m_fDone;
    VXIbool m_fShutdown;
    VXItrdTimer * m_nInitializeStatus;

    SBinetString m_strCacheFolder;
    VXIint  m_nCacheTotalSizeMB;
    VXIint m_nCacheEntryMaxSizeMB;
    VXIint m_nCacheEntryExpTimeSec;
    VXIbool m_fInitializedCache;

    SBinetString m_strProxyServer;
    VXIulong m_nProxyPort;

    SBinetNString m_strUserAgentApp;
    SBinetNString m_strUserAgentVersion;

    VXIMap* m_extensionRules;

    int runEventLoop;
};

// Static class to ensure the global inet is destroyed even if
// SBinetShutDown() is not called, otherwise the libwww based cache
// is not properly shutdown
class SBinetTerminate
{
public:
    SBinetTerminate()
    {
    }
    virtual ~SBinetTerminate();
};


/*****************************************************************************
 *****************************************************************************
 * Globals
 *****************************************************************************
 *****************************************************************************
 */

// The Global inet object. Manages Event Thread, etc.
static SBinet *g_SBinet = NULL;

// Allows proper shutdown even when SBinetShutdown() is not called
static SBinetTerminate g_SBinetTerminate;

// Global diagnostic logging base
VXIunsigned g_SBinetDiagLogBase = 0;

// Global diagnostic logger
static VXIlogInterface *g_SBinetLogger = NULL;

/*****************************************************************************
 *****************************************************************************
 * The Event Thread entry. Inits libwww event stuff
 *****************************************************************************
 *****************************************************************************
 */
VXITRD_DEFINE_THREAD_FUNC( SBinet::EventThreadMain, userData )
{
    SBinet *pThis = (SBinet *) userData;

    HTList * converters = NULL;
    HTList * transfer_encodings = NULL;
    HTList * content_encodings = NULL;

    // HTProfile_newPreemptiveClient("TestApp", "1.0");
    //  HTProfile_newNoCacheClient("TestApp", "1.0");

    if ( !HTLib_isInitialized() )
        HTLibInit(pThis->m_strUserAgentApp.c_str(), 
                  pThis->m_strUserAgentVersion.c_str());

    /* Register the default set of messages and dialog functions,
       for now everything goes to the diagnostic log */
    HTAlertInit();
    HTAlert_setInteractive(YES);
    HTAlert_add( AlertCallback,      HT_A_PROGRESS );
    HTAlert_add( AlertCallback,      HT_A_MESSAGE  );
    HTAlert_add( AlertCallback,      HT_A_CONFIRM  );
    HTAlert_add( AlertCallback,      HT_A_PROMPT   );
    HTAlert_add( AlertCallback,      HT_A_SECRET   );
    HTAlert_add( AlertCallback,      HT_A_USER_PW  );

    /* Register the default out of memory handler */
    HTMemory_setExit(OutOfMemoryExitCallback);

    if ( !converters ) converters = HTList_new();
    if ( !transfer_encodings ) transfer_encodings = HTList_new();
    if ( !content_encodings ) content_encodings = HTList_new();

    /* Register the default set of transport protocols */
    HTTransportInit();

    /* Register the default set of application protocol modules */
    //HTProtocolPreemptiveInit();
    HTProtocolInit();

    /* Initialize suffix bindings for local files */
    HTBind_init();

    /* Set max number of sockets we want open simultanously */ 
    HTNet_setMaxSocket(32);

    /* The persistent cache does not work in preemptive mode */
    /* We init cach later */

    /* Register the default set of BEFORE and AFTER filters */
    HTNetInit();

    /* Set up the default set of Authentication schemes */
    HTAAInit();

    /* Get any proxy or gateway environment variables */
    HTProxy_getEnvVar();

    /* Register the default set of converters */
    HTConverterInit(converters);

    /* Set the convertes as global converters for all requests */
    HTFormat_setConversion(converters);

    /* Register the default set of transfer encoders and decoders */
    HTTransferEncoderInit(transfer_encodings);
    HTFormat_setTransferCoding(transfer_encodings);

    /* Register the default set of content encoders and decoders */
    HTContentEncoderInit(content_encodings);
    if ( HTList_count(content_encodings) > 0 )
        HTFormat_setContentCoding(content_encodings);
    else
    {
        HTList_delete(content_encodings);
        content_encodings = NULL;
    }

    /* Register the default set of MIME header parsers */
    HTMIMEInit();

    /* Register the default set of file suffix bindings */
    HTFileInit();

    /* Register the default set of Icons for directory listings */
    HTIconInit(NULL);

    /* Initialize event loop */
    SBinetHTEventInit ();

    /* Turn on TRACE so we can see what is going on by turning on
       diagnostic logging tags */
    HTPrint_setCallback(PrintCallback);
    HTTrace_setCallback(TraceCallback);
    HTSetTraceMessageMask("ospt");
    HTTraceData_setCallback(TraceDataCallback);

    /*
     * Need to fold startup errors back to main thread
     */
    pThis->InitializeCache();
    pThis->InitializeProxy();

    // Initiate cookies module
    HTCookie_init();
    HTCookie_setCookieMode((HTCookieMode)(HT_COOKIE_ACCEPT | HT_COOKIE_SEND));
    HTCookie_setCallbacks( SBinetInterface::setCookie, NULL, SBinetInterface::findCookie, NULL );

    VXItrdTimerWake(pThis->m_nInitializeStatus);
    SBinetHTEventList_newLoop(); // Start the event loop

    /* Terminate the Library */ 
    SBinetHTEventTerminate();
    HTProfile_delete();

    return(0);
}

// -----1=0-------2=0-------3=0-------4=0-------5=0-------6=0-------7=0-------8


/*****************************************************************************
 *****************************************************************************
 * SBinet C routines:  Init(), ShutDown(), CreateResource(), DestroyResource()
 *
 * NOTE: We provide both OSB and SB prefixed entry points, OSB for the
 * OpenVXI open source release, SB for the OpenSpeech Browser PIK product.
 *****************************************************************************
 *****************************************************************************
 */


SBINET_API
VXIinetResult SBinetInit( VXIlogInterface* piVXILog,
                          const VXIunsigned diagLogBase,
                          const VXIchar*   pszCacheFolder, 
                          const VXIint     nCacheTotalSizeMB,
                          const VXIint     nCacheEntryMaxSizeMB,
                          const VXIint     nCacheEntryExpTimeSec,
                          const VXIchar*   pszProxyServer,
                          const VXIulong   nProxyPort,
                          const VXIchar*   userAgentName,
                          const VXIMap*    extensionRules,
                          const VXIVector* reserved )
{
    VXIinetResult rc = VXIinet_RESULT_SUCCESS;

    g_SBinetDiagLogBase = diagLogBase;
    g_SBinetLogger = piVXILog;
    SBinetLogFunc apiTrace (piVXILog, diagLogBase+ MODULE_SBINET_TAGID,
                            L"SBinetInit", (int *) &rc, 
                            L"entering: 0x%p, %s, %d, %d, %d, %s, %lu, %s, "
                            L"0x%p, 0x%p", piVXILog, pszCacheFolder, 
                            nCacheTotalSizeMB, nCacheEntryMaxSizeMB,
                            nCacheEntryExpTimeSec, pszProxyServer, nProxyPort,
                            userAgentName, extensionRules, reserved);

    /* check the remaining arguments */
    if ( (nCacheTotalSizeMB < 0)     ||
         (nCacheEntryMaxSizeMB < 0)  ||
         (nCacheEntryExpTimeSec < 0) ||
         (nCacheEntryMaxSizeMB > nCacheTotalSizeMB) )
    {
        SBinetLogger::Error(piVXILog,MODULE_SBINET,200,NULL);
        SBinetLogger::Error(piVXILog,MODULE_SBINET,100,NULL);
        return VXIinet_RESULT_INVALID_ARGUMENT;
    }

    // Global SBinet initialization
    if ( ! g_SBinet )
    {
        g_SBinet = new SBinet(piVXILog, diagLogBase);
        if ( ! g_SBinet )
            return VXIinet_RESULT_OUT_OF_MEMORY;

        // @JC Set the SSL protocol method. By default, it is the highest
        // available protocol. Setting it up to SSL_V23 allows the client
        // to negotiate with the server and set up either TSLv1, SSLv3, 
        // or SSLv2
        HTSSL_protMethod_set ( HTSSL_V23 );

        // Set the certificate verification depth to 2 in order to be able to
        // validate self signed certificates
        HTSSL_verifyDepth_set (2);

        // Register SSL stuff for handling ssl access, this registers
        // the plugin and associates it with the 'https' scheme
        HTSSLhttps_init( YES );

        rc = g_SBinet->Init(pszCacheFolder, nCacheTotalSizeMB, 
                            nCacheEntryMaxSizeMB, nCacheEntryExpTimeSec, 
                            pszProxyServer, nProxyPort, userAgentName,
                            extensionRules);
        if ( rc != VXIinet_RESULT_SUCCESS )
        {
            // To avoid use of global during shutdown
            SBinet *temp = g_SBinet;
            g_SBinet = NULL;
            delete temp;
            temp = NULL;
        }
    }

    return( rc );
}


OSBINET_API
VXIinetResult OSBinetInit( VXIlogInterface* piVXILog,
                           const VXIunsigned diagLogBase,
                           const VXIchar*   pszCacheFolder, 
                           const VXIint     nCacheTotalSizeMB,
                           const VXIint     nCacheEntryMaxSizeMB,
                           const VXIint     nCacheEntryExpTimeSec,
                           const VXIchar*   pszProxyServer,
                           const VXIulong   nProxyPort,
                           const VXIchar*   userAgentName,
                           const VXIMap*    extensionRules,
                           const VXIVector* reserved )
{
    return SBinetInit(piVXILog, diagLogBase, pszCacheFolder, nCacheTotalSizeMB,
                      nCacheEntryMaxSizeMB, nCacheEntryExpTimeSec, 
                      pszProxyServer, nProxyPort, userAgentName, extensionRules,
                      reserved);
}


SBINET_API
VXIinetResult SBinetShutDown( VXIlogInterface* piVXILog )
{
    VXIinetResult rc = VXIinet_RESULT_SUCCESS;
    SBinetLogFunc apiTrace (piVXILog, g_SBinetDiagLogBase + MODULE_SBINET_TAGID,
                            L"SBinetShutdown", (int *) &rc, 
                            L"entering: 0x%p", piVXILog);

    if ( ! g_SBinet )
        return VXIinet_RESULT_NON_FATAL_ERROR;

    // Use a temporary to avoid references to the global during destruction
    SBinet *temp = g_SBinet;
    g_SBinet = NULL;
    temp->Terminate(FALSE);
    delete temp;
    temp = NULL;

    return(rc);
}


OSBINET_API
VXIinetResult OSBinetShutDown( VXIlogInterface* piVXILog )
{
    return SBinetShutDown(piVXILog);
}


SBINET_API
VXIinetResult SBinetCreateResource( VXIlogInterface* piVXILog,
                                    VXIinetInterface**     ppiVXIInet )
{
    VXIinetResult rc = VXIinet_RESULT_SUCCESS;

    SBinetLogFunc apiTrace (piVXILog, g_SBinetDiagLogBase + MODULE_SBINET_TAGID,
                            L"SBinetCreateResource", (int *) &rc, 
                            L"entering: 0x%p, 0x%p",
                            piVXILog, ppiVXIInet);

    if ( ! g_SBinet )
        return VXIinet_RESULT_FATAL_ERROR;

    if ( !piVXILog || !ppiVXIInet )
    {
        SBinetLogger::Error(piVXILog,MODULE_SBINET,102,NULL);
        return(rc = VXIinet_RESULT_INVALID_ARGUMENT);
    }

    // the channel number is now available
    g_SBinet->SetLog (piVXILog, g_SBinetDiagLogBase);

    SBinetInterface* pInter = new SBinetInterface(piVXILog,
                                                  g_SBinetDiagLogBase);
    if ( ! pInter )
    {
        SBinetLogger::Error(piVXILog,MODULE_SBINET,103,NULL);
        return(rc = VXIinet_RESULT_OUT_OF_MEMORY);
    }
    // Could add to Channel/Interface table for validation but...

    *ppiVXIInet = (VXIinetInterface*)pInter; // Downcast...

    return(rc);
}


OSBINET_API
VXIinetResult OSBinetCreateResource( VXIlogInterface* piVXILog,
                                     VXIinetInterface**     ppiVXIInet )
{
    return SBinetCreateResource(piVXILog, ppiVXIInet);
}


SBINET_API
VXIinetResult SBinetDestroyResource( VXIinetInterface** ppiVXIInet )
{
    if ( ! g_SBinet )
        return VXIinet_RESULT_FATAL_ERROR;

    VXIlogInterface* piVXILog = g_SBinet->GetLog(); 
    VXIinetResult rc = VXIinet_RESULT_SUCCESS;
    SBinetLogFunc apiTrace (piVXILog, g_SBinetDiagLogBase + MODULE_SBINET_TAGID,
                            L"SBinetDestroyResource", (int *) &rc, 
                            L"entering: 0x%p (0x%p)", ppiVXIInet,
                            (ppiVXIInet ? *ppiVXIInet : NULL));

    if ( ! ppiVXIInet )
    {
        g_SBinet->Error(102,NULL);
        return(rc = VXIinet_RESULT_INVALID_ARGUMENT);
    }
    if ( ! *ppiVXIInet )
    {
        g_SBinet->Error(102,NULL);
        return(rc = VXIinet_RESULT_INVALID_ARGUMENT);
    }
    SBinetInterface* pSBinet = (SBinetInterface*)(*ppiVXIInet);
    // Could delete to Channel/Interface table for validation but...

    // Since the log associated with this session is gone now, 
    // reset the log to the global one.  The channel number will be incorrect (-1).
    g_SBinet->SetLog(g_SBinetLogger, g_SBinetDiagLogBase);

    // Delete it, destructor cleans up
    delete pSBinet;
    pSBinet = NULL;
    *ppiVXIInet = static_cast<VXIinetInterface*>(NULL);

    return(rc);

}


OSBINET_API
VXIinetResult OSBinetDestroyResource( VXIinetInterface** ppiVXIInet )
{
    return SBinetDestroyResource(ppiVXIInet);
}


/****************************************************************************
 ****************************************************************************
 * SBinetTerminate methods
 *
 ****************************************************************************
 ****************************************************************************
 */

SBinetTerminate::~SBinetTerminate()
{
    if ( g_SBinet )
    {
        // To avoid having anything reference the global during termination
        SBinet *temp = g_SBinet;
        g_SBinet = NULL;

        // Disable logging during termination, the log resource is
        // probably gone by now so trying to use it as will be done at
        // shutdown causes a crash
        temp->Terminate(TRUE);
        delete temp;
        temp = NULL;
    }
}


/****************************************************************************
 ****************************************************************************
 * SBinet methods: Initializations
 *
 ****************************************************************************
 ****************************************************************************
 */


SBinet::SBinet(VXIlogInterface *log, VXIunsigned diagLogBase) :
SBinetLogger(MODULE_SBINET, log, diagLogBase),
m_pEventThread(0), m_pLibwwwMutex(0), m_pEventTimer(0),
m_nSleepInterval(0), m_fDone(false), m_fShutdown(false),
m_nInitializeStatus(NULL), m_strCacheFolder(),
m_nCacheEntryMaxSizeMB(0), m_nCacheEntryExpTimeSec(0),
m_fInitializedCache(false), m_strProxyServer(),
m_nProxyPort(0), m_extensionRules(0), runEventLoop(0)
{
}

SBinet::~SBinet()
{
}


//
// Initialize SBinet
//
VXIinetResult SBinet::Init(const VXIchar*   pszCacheFolder, 
                           const VXIint     nCacheTotalSizeMB,
                           const VXIint     nCacheEntryMaxSizeMB,
                           const VXIint     nCacheEntryExpTimeSec,
                           const VXIchar*   pszProxyServer,
                           const VXIulong   nProxyPort,
                           const VXIchar*   userAgentName,
                           const VXIMap*    extensionRules)
{
    // Check if the environment is correct for supporting SBinet
    VXIinetResult rc = CheckEnvironment();
    if ( rc != VXIinet_RESULT_SUCCESS )
        return rc;

    m_pEventThread = NULL;
    m_pEventTimer = NULL;
    m_nSleepInterval = 5 * 1000L;    // 5 second sleep interval
    m_fDone = false;
    m_fShutdown = false;
    m_strCacheFolder = 
    ( pszCacheFolder != NULL ) ? pszCacheFolder : L"";
    m_nCacheTotalSizeMB = nCacheTotalSizeMB;
    m_nCacheEntryMaxSizeMB = nCacheEntryMaxSizeMB;
    m_nCacheEntryExpTimeSec = nCacheEntryExpTimeSec;
    m_fInitializedCache = false;
    if ( pszProxyServer != NULL )
        m_strProxyServer = pszProxyServer;
    m_nProxyPort = nProxyPort;

    // Have to parse the user agent name
#ifdef OPENVXI
    const VXIchar *tempAgent = OSBINET_USER_AGENT_NAME_DEFAULT;
#else
    const VXIchar *tempAgent = SBINET_USER_AGENT_NAME_DEFAULT;
#endif
    if ( ( userAgentName ) && ( userAgentName[0] ) )
        tempAgent = userAgentName;
    const VXIchar *ptr = tempAgent;
    while ( iswspace(*ptr) )
        ptr++;
    while ( (*ptr) && (*ptr != L'/') )
    {
        m_strUserAgentApp += (char) *ptr;
        ptr++;
    }
    if ( *ptr )
        ptr++;
    while ( iswspace(*ptr) )
        ptr++;
    while ( (iswalnum(*ptr)) || (*ptr == L'.') )
    {
        m_strUserAgentVersion += (char) *ptr;
        ptr++;
    }
    if ( (m_strUserAgentApp.length() == 0) ||
         (m_strUserAgentVersion.length() == 0) )
    {
        Error(100,NULL);
        return VXIinet_RESULT_INVALID_ARGUMENT;
    }

    VXItrdResult eTrdResult = VXItrdTimerCreate( &m_pEventTimer );
    if ( eTrdResult != VXItrd_RESULT_SUCCESS )
    {
        Error(100,NULL);
        return VXIinet_RESULT_SYSTEM_ERROR;
    }

    eTrdResult = VXItrdMutexCreate( &m_pLibwwwMutex );
    if ( eTrdResult != VXItrd_RESULT_SUCCESS )
    {
        Error(100,NULL);
        return VXIinet_RESULT_SYSTEM_ERROR;
    }

    // Initialize the cache lock table
    SBinetCacheLockTable::Init(GetLog(), GetDiagBase());

    // Create timer.  The EventThreadMain wakes this timer when it is complete.
    eTrdResult = VXItrdTimerCreate(&m_nInitializeStatus);
    if ( eTrdResult != VXItrd_RESULT_SUCCESS )
    {
        Error(100,NULL);
        return VXIinet_RESULT_SYSTEM_ERROR;
    }

    runEventLoop = 1;
    eTrdResult = VXItrdThreadCreate(&m_pEventThread,
                                    SBinet::EventThreadMain, 
                                    this);
    if ( eTrdResult != VXItrd_RESULT_SUCCESS )
    {
        Error(100,NULL);
        return VXIinet_RESULT_SYSTEM_ERROR;
    }

    if ( extensionRules )
    {
        m_extensionRules = VXIMapClone(extensionRules);
    } else
        m_extensionRules = NULL;

    // Spin while event thread inits libwww
    VXIbool done = FALSE;
    while ( !done ) VXItrdTimerSleep(m_nInitializeStatus, 10000, &done);
    VXItrdTimerDestroy(&m_nInitializeStatus);

    return rc;
}


//
// Terminate SBinet
//
VXIinetResult SBinet::Terminate(VXIbool clearLogResource)
{
    VXIinetResult rc = VXIinet_RESULT_SUCCESS;

    if ( clearLogResource )
        SetLog(NULL, GetDiagBase());

    TerminateCache();

    // Shut down the cache lock table
    SBinetCacheLockTable::ShutDown(GetLog());

    // Shut down the cookies module
    HTCookie_deleteCallbacks();              
    // HTCookie_terminate(); // Terminate cookies module! HT BUG, HTLIST CRASHES (MEMORY ACCESS VIOLATION)

    if ( m_pEventThread )
    {
        m_fShutdown = true;  
        VXItrdThreadArg nExitResult;
        runEventLoop = 0;
        SBinetHTEventList_stopLoop ();

        // Wait for event thread to exit
        // VXItrdResult eTrdResult =
                   VXItrdThreadJoin(m_pEventThread, &nExitResult, 3 * 1000);
        /*
          if (eTrdResult == VXItrd_RESULT_SUCCESS)
            return (rc = VXIinet_RESULT_SUCCESS);
          else
            return (rc = VXIinet_RESULT_SYSTEM_ERROR);
        */
        VXItrdThreadDestroyHandle( &m_pEventThread );
    }

    if ( m_pEventTimer )
        VXItrdTimerDestroy( &m_pEventTimer );
    if ( m_pLibwwwMutex )
        VXItrdMutexDestroy( &m_pLibwwwMutex );

    if ( m_extensionRules )
    {
        VXIMapDestroy(&(m_extensionRules));
        m_extensionRules = NULL;
    }

    // TODO: Scan Instance and Stream maps and delete outstanding objects
    // Close down SSL
    HTSSLhttps_terminate();

    return rc;
}


//
// Check if the environment properly supports SBinet
//
VXIinetResult SBinet::CheckEnvironment()
{
    VXIinetResult rc = VXIinet_RESULT_SUCCESS;

#ifdef WIN32
    // Get the Microsoft Internet Explorer version, must have version
    // 5.0 or later otherwise InternetCrackUrl() rejects various
    // file:// URL formats that we need to support. Base this off the
    // version of the DLL that implements the browser as documented at
    // http://support.microsoft.com/support/kb/articles/q164/5/39.asp,
    // the IE 5 GA release = 5.00.2014.213
    static const wchar_t IE_DLL_NAME[] = L"shdocvw.dll";
    static const int IE_DLL_VER_1 = 5;
    static const int IE_DLL_VER_2 = 0;
    static const int IE_DLL_VER_3 = 2014;
    static const int IE_DLL_VER_4 = 213;

    VXIint infoSize = GetFileVersionInfoSize((wchar_t *) IE_DLL_NAME, 0);
    if ( infoSize <= 0 )
    {
        // Could not find the DLL or version info not available
        Error(105, NULL);
        rc = VXIinet_RESULT_PLATFORM_ERROR;
    } else
    {
        VXIbyte *infoData = new VXIbyte [infoSize];
        if ( infoData == NULL )
        {
            Error(103, NULL);
            rc = VXIinet_RESULT_OUT_OF_MEMORY;
        } else
        {
            LPDWORD dwResource;
            UINT cnSize;

            if ( (GetFileVersionInfo((wchar_t*)IE_DLL_NAME, 0,infoSize,infoData)==0)||
                 (VerQueryValue(infoData, L"\\", (LPVOID*)(&dwResource), &cnSize)==0) )
            {
                // Version info not available
                Error(105, NULL);
                rc = VXIinet_RESULT_PLATFORM_ERROR;
            } else
            {
                Diag(MODULE_SBINET_BASE_TAGID, NULL, 
                     L"Microsoft Internet Explorer %2d.%02d.%04d.%d is active",
                     HIWORD(dwResource[2]), LOWORD(dwResource[2]),
                     HIWORD(dwResource[3]), LOWORD(dwResource[3]));

                // Check if the DLL's version is OK for us
                bool badVersion = false;
                if ( HIWORD(dwResource[2]) < IE_DLL_VER_1 )
                    badVersion = true;
                else if ( HIWORD(dwResource[2]) == IE_DLL_VER_1 )
                {
                    if ( LOWORD(dwResource[2]) < IE_DLL_VER_2 )
                        badVersion = true;
                    else if ( LOWORD(dwResource[2]) == IE_DLL_VER_2 )
                    {
                        if ( HIWORD(dwResource[3]) < IE_DLL_VER_3 )
                            badVersion = true;
                        else if ( HIWORD(dwResource[3]) == IE_DLL_VER_3 )
                        {
                            if ( LOWORD(dwResource[3]) < IE_DLL_VER_4 )
                                badVersion = true;
                        }
                    }
                }

                if ( badVersion )
                {
                    Error(104, L"%s%d%s%d%s%d%s%d", 
                          L"IEMajor", HIWORD(dwResource[2]), 
                          L"IEMinor", LOWORD(dwResource[2]),
                          L"IEBuild", HIWORD(dwResource[3]), 
                          L"IEPatch", LOWORD(dwResource[3]));
                    rc = VXIinet_RESULT_PLATFORM_ERROR;
                }
            }

            delete [] infoData;
            infoData = NULL;
        }
    }
#endif

    return rc;
}


// Confirmation callback - used for the lock removal
BOOL 
SBinet::CacheLockRemovalConfirmCallback(HTRequest *request, HTAlertOpcode op,
                                        int msgnum, const char *deflt, 
                                        void *input, HTAlertPar *reply)
{
    // callback because there is a lock file - asking if it can be
    // removed. Answer is YES
    if ( msgnum == HT_MSG_CACHE_LOCK )
    {
        return TRUE;
    }
    return FALSE;
}


//
// InitializeCache:
// 
VXIinetResult 
SBinet::InitializeCache()
{
    VXIinetResult retCode;

    const VXIchar* pszCacheFolder = m_strCacheFolder.c_str();

    // If no cache folder was specified, or cache
    // parameters are invalid then don't use the cache
    if ( ( pszCacheFolder == NULL ) || ( pszCacheFolder[0] == L'\0' ) ||
         ( m_nCacheTotalSizeMB == 0 ) || ( m_nCacheEntryMaxSizeMB == 0 ) ||
         ( m_nCacheEntryExpTimeSec == 0 ) )
    {
        Error(303, NULL);
        return(VXIinet_RESULT_SUCCESS); // !!
    }

    VXIint len = ::wcslen( pszCacheFolder ) + 1;
    char *tempStr;

    // Simplify path, get rid of double or trailing path separators, etc.
#ifdef WIN32
    VXIchar *ignored;
    VXIchar *wTempStr;

    // Find out the required buffer size
    len = GetFullPathName( pszCacheFolder, 0, NULL, &ignored );
    if ( len == 0 )
    {
        Error(202,NULL);
        return VXIinet_RESULT_INVALID_ARGUMENT;
    }
    wTempStr = new VXIchar [len + 1];

    // Now process the path
    if ( GetFullPathName( pszCacheFolder, len, wTempStr, &ignored ) == 0 )
    {
        Error(202,NULL);
        return VXIinet_RESULT_INVALID_ARGUMENT;
    }

    tempStr = new char [len + 1];
    ::wcstombs( tempStr, wTempStr, len + 1 );
    delete [] wTempStr;
    wTempStr = NULL;
#else
    tempStr = new char [len];
    ::wcstombs( tempStr, pszCacheFolder, len );
    HTSimplify( &tempStr );
#endif

    char *pTempStr;
    if ( ::strstr( tempStr, "file:/" ) == tempStr )
        pTempStr = tempStr + ::strlen( "file:/" );
    else if ( ::strstr( tempStr, "file:" ) == tempStr )
        pTempStr = tempStr + ::strlen( "file:" );
    else
        pTempStr = tempStr;

    char *parsedTempStr = HTLocalToWWW( pTempStr, "" );

    // Register the confirmation callback
    if ( HTAlert_add(CacheLockRemovalConfirmCallback, HT_A_CONFIRM) == 0 )
    {
        Error(202,NULL);
        return VXIinet_RESULT_IO_ERROR;
    }

    if ( HTCacheInit( parsedTempStr, m_nCacheTotalSizeMB ) == NO )
    {
        Error(202,NULL);
        retCode = VXIinet_RESULT_IO_ERROR;
    } else
    {
        HTCacheMode_setMaxCacheEntrySize( m_nCacheEntryMaxSizeMB );
        HTCacheMode_setDefaultExpiration( m_nCacheEntryExpTimeSec );
        // At last, enable or disable the persistent cache 
        HTCacheMode_setEnabled(1);
        Diag (MODULE_SBINET_TAGID, L"SBinet::InitializeCache",
              L"Enabled Inet cache, folder is %S", parsedTempStr);
        // TODO
        // typedef enum _HTExpiresMode {HT_EXPIRES_IGNORE=0, HT_EXPIRES_NOTIFY, HT_EXPIRES_AUTO} HTExpiresMode;
        // extern void HTCacheMode_setExpires (HTExpiresMode mode);

        m_fInitializedCache = true;

        retCode = VXIinet_RESULT_SUCCESS;
    }

    // Cleanup
    if ( tempStr != parsedTempStr ) 
    {
	delete [] tempStr;
	tempStr = NULL;
    }
    HT_FREE( parsedTempStr );

    return( retCode );
}

VXIinetResult 
SBinet::TerminateCache( void )
{
    if ( m_fInitializedCache )
    {
        HTCacheTerminate();
        m_fInitializedCache = false;
    }
    return VXIinet_RESULT_SUCCESS;
}


VXIinetResult 
SBinet::InitializeProxy (void )
{

    const VXIulong nProxyPort = m_nProxyPort;
    VXIint len = m_strProxyServer.length();
    if ( len < 1 )
        return( VXIinet_RESULT_SUCCESS ); // No proxy specified

    len++;
    char *proxyID = new char [ len + 32 ];

    ::strcpy( proxyID, "http://" );
    VXIint httpLen = ::strlen( "http://" );

    ::wcstombs( proxyID + httpLen, m_strProxyServer.c_str(), len );

    sprintf( proxyID + httpLen + len - 1, ":%ld", nProxyPort );

    // Register the proxy for http, https and ftp requests
    VXIinetResult eResult = 
    (HTProxy_add( "http", proxyID ) ? VXIinet_RESULT_SUCCESS : 
     VXIinet_RESULT_FATAL_ERROR);
    if ( eResult == VXIinet_RESULT_SUCCESS ) eResult = 
        (HTProxy_add( "https", proxyID ) ? VXIinet_RESULT_SUCCESS : 
         VXIinet_RESULT_FATAL_ERROR);
    if ( eResult == VXIinet_RESULT_SUCCESS ) eResult = 
        (HTProxy_add( "ftp", proxyID ) ? VXIinet_RESULT_SUCCESS : 
         VXIinet_RESULT_FATAL_ERROR);

    delete [] proxyID;
    proxyID = NULL;

    return(eResult );
}


/****************************************************************************
 ****************************************************************************
 * SBinetInterface Implementation: Just call corresponding routine on channel
 *
 ****************************************************************************
 ****************************************************************************
 */

SBinetInterface::SBinetInterface( VXIlogInterface* pVXILog, 
                                  VXIunsigned diagLogBase ) : 
SBinetLogger (MODULE_SBINET_INTERFACE, pVXILog, diagLogBase) 
{
    m_ch = new SBinetChannel(pVXILog, diagLogBase);

    // Init interface
    VXIinetInterface::GetVersion = SBinetInterface::GetVersion;
    VXIinetInterface::GetImplementationName =
    SBinetInterface::GetImplementationName;

    VXIinetInterface::Prefetch = Prefetch;
    VXIinetInterface::Open = Open;
    VXIinetInterface::Read = Read;
    VXIinetInterface::Write = Write;
    VXIinetInterface::Close = Close;
    VXIinetInterface::SetCookieJar = SetCookieJar;
    VXIinetInterface::GetCookieJar = GetCookieJar;
}

SBinetInterface::~SBinetInterface()
{
    if ( m_ch )
    {
        m_ch->CloseAll();
        delete m_ch;
        m_ch = NULL;
    }
}


void
SBinetInterface::LockLibwww()
{
    g_SBinet->LockLibwww();
}

void
SBinetInterface::UnlockLibwww()
{
    g_SBinet->UnlockLibwww();
}


const VXIString*
SBinetInterface::mapExtension(const VXIchar* ext ) 
{
    return(g_SBinet->mapExtension(ext));
}


/*
 * Call Channel method
 */

VXIint32 SBinetInterface::GetVersion(void)
{
    return VXI_CURRENT_VERSION;
}


const VXIchar* SBinetInterface::GetImplementationName(void)
{
    return SBINET_IMPLEMENTATION_NAME;
}


VXIinetResult 
SBinetInterface::Prefetch(/* [IN]  */ VXIinetInterface*      pThis,
                          /* [IN]  */ const VXIchar*   pszModuleName,
                          /* [IN]  */ const VXIchar*   pszName,
                          /* [IN]  */ VXIinetOpenMode  eMode,
                          /* [IN]  */ VXIint32         nFlags,
                          /* [IN]  */ const VXIMap*    pProperties  )
{
    SBinetInterface* intr = (SBinetInterface*)pThis;
    if ( !intr )
    {
        intr->Error(200, L"%s%s", L"Operation", L"Prefetch");
        return(VXIinet_RESULT_INVALID_ARGUMENT);
    }

    VXIinetResult rc = VXIinet_RESULT_SUCCESS;
    SBinetLogFunc apiTrace (intr->GetLog(), g_SBinetDiagLogBase + 
                            MODULE_SBINET_TAGID,
                            L"SBinetInterface::Prefetch", (int *) &rc, 
                            L"entering: 0x%p, %s, %s, 0x%x, 0x%x, 0x%p",
                            pThis, pszModuleName, pszName, eMode, nFlags,
                            pProperties);

    // check validation
    SBinetChannel* ch = intr->m_ch;
    if ( !ch )
    {
        intr->Error(200, L"%s%s", L"Operation", L"Prefetch");
        return(rc = VXIinet_RESULT_INVALID_ARGUMENT);
    }
    rc = (ch->Prefetch)(pszModuleName, pszName, eMode, nFlags, pProperties);
    if ( rc != VXIinet_RESULT_SUCCESS )
    {
        intr->Error(203,L"%s%d",L"rc",rc);
    }
    return(rc);
}

/*
 * Call Channel method
 */
VXIinetResult 
SBinetInterface::Open(/* [IN]  */ VXIinetInterface*      pThis,
                      /* [IN]  */ const VXIchar*   pszModuleName,
                      /* [IN]  */ const  VXIchar*  pszName,
                      /* [IN]  */ VXIinetOpenMode  eMode,
                      /* [IN]  */ VXIint32         nFlags,
                      /* [IN]  */ const VXIMap*    pProperties,
                      /* [OUT] */ VXIMap*          pmapStreamInfo,
                      /* [OUT] */ VXIinetStream**  ppStream     )
{
    SBinetInterface* intr = (SBinetInterface*)pThis;
    if ( !intr )
    {
        intr->Error(200, L"%s%s", L"Operation", L"Open");
        return(VXIinet_RESULT_INVALID_ARGUMENT);
    }

    VXIinetResult rc = VXIinet_RESULT_SUCCESS;
    SBinetLogFunc apiTrace (intr->GetLog(), g_SBinetDiagLogBase + 
                            MODULE_SBINET_TAGID,
                            L"SBinetInterface::Open", (int *) &rc, 
                            L"entering: 0x%p, %s, %s, 0x%x, 0x%x, 0x%p, "
                            L"0x%p, 0x%p",
                            pThis, pszModuleName, pszName, eMode, nFlags,
                            pProperties, pmapStreamInfo, ppStream);

    // check validation
    SBinetChannel* ch = intr->m_ch;
    if ( !ch )
    {
        intr->Error(200, L"%s%s", L"Operation", L"Prefetch");
        return(rc = VXIinet_RESULT_INVALID_ARGUMENT);
    }
    rc = (ch->Open)(pszModuleName, pszName, eMode, nFlags, 
                    pProperties, pmapStreamInfo,ppStream);
    if ( (rc != VXIinet_RESULT_SUCCESS) && 
         (rc != VXIinet_RESULT_NOT_MODIFIED) &&
         (rc != VXIinet_RESULT_LOCAL_FILE) )
    {
        intr->Error(204,L"%s%d",L"rc",rc);
    }
    return(rc);
}

/*
 * We must call channel method so we can GC the stream
 */
VXIinetResult 
SBinetInterface::Close(/* [IN]  */ VXIinetInterface*      pThis,
                       /* [IN]  */ VXIinetStream**  ppStream     )
{
    SBinetInterface* intr = (SBinetInterface*)pThis;
    if ( !intr )
    {
        intr->Error(200, L"%s%s", L"Operation", L"Close");
        return(VXIinet_RESULT_INVALID_ARGUMENT);
    }

    VXIinetResult rc = VXIinet_RESULT_SUCCESS;
    SBinetLogFunc apiTrace (intr->GetLog(), g_SBinetDiagLogBase + 
                            MODULE_SBINET_TAGID,
                            L"SBinetInterface::Close", (int *) &rc, 
                            L"entering: 0x%p, 0x%p (0x%p)", pThis, ppStream,
                            (ppStream ? *ppStream : NULL));

    // check validation
    SBinetChannel* ch = intr->m_ch;
    if ( !ch || !ppStream )
    {
        intr->Error(200, L"%s%s", L"Operation", L"Close");
        return(rc = VXIinet_RESULT_INVALID_ARGUMENT);
    }
    rc = (ch->Close)(ppStream);
    if ( rc != VXIinet_RESULT_SUCCESS )
    {
        intr->Error(205,L"%s%d",L"rc",rc);
    }
    return(rc);
}

/*
 * Actually, we are simply going to go and call Stream routine directly here
 */
VXIinetResult 
SBinetInterface::Read(/* [IN]  */ VXIinetInterface*      pThis,
                      /* [OUT] */ VXIbyte*         pBuffer,
                      /* [IN]  */ VXIulong         nBuflen,
                      /* [OUT] */ VXIulong*        pnRead,
                      /* [IN]  */ VXIinetStream*   pStream      )
{
    SBinetInterface* intr = (SBinetInterface*)pThis;
    if ( !intr )
    {
        intr->Error(200, L"%s%s", L"Operation", L"Read");
        return(VXIinet_RESULT_INVALID_ARGUMENT);
    }

    VXIinetResult rc = VXIinet_RESULT_SUCCESS;
    SBinetLogFunc apiTrace (intr->GetLog(), g_SBinetDiagLogBase + 
                            MODULE_SBINET_TAGID,
                            L"SBinetInterface::Read", (int *) &rc, 
                            L"entering: 0x%p, 0x%p, %lu, %lu, 0x%p",
                            pThis, pBuffer, nBuflen, pnRead, pStream);

    // check validation
    SBinetChannel* ch = intr->m_ch;
    if ( !ch )
    {
        intr->Error(200, L"%s%s", L"Operation", L"Read");
        return(rc = VXIinet_RESULT_INVALID_ARGUMENT);
    }
    SBinetStream* st = ch->GetStream(pStream); // Really just validates and casts
    if ( !st )
    {
        intr->Error(200, L"%s%s", L"Operation", L"Read");
        return(rc = VXIinet_RESULT_INVALID_ARGUMENT);
    }
    //pBuffer[0] = 0; // may cause buffer overrun problems
    rc = (st->Read)(pBuffer,nBuflen, pnRead );
    if ( (rc != VXIinet_RESULT_SUCCESS) &&
         (rc != VXIinet_RESULT_WOULD_BLOCK) &&
         (rc != VXIinet_RESULT_END_OF_STREAM) )
    {
        intr->Error(206,L"%s%d",L"rc",rc);
    }
    return(rc);
}

/*
 * We don't implement this any where so just return
 */
VXIinetResult 
SBinetInterface::Write(/* [IN]  */ VXIinetInterface*      pThis,
                       /* [OUT] */ const VXIbyte*   pBuffer,
                       /* [IN]  */ VXIulong         nBuflen,
                       /* [OUT] */ VXIulong*        pnWritten,
                       /* [IN]  */ VXIinetStream*   pStream      )
{
    SBinetInterface* intr = (SBinetInterface*)pThis;
    if ( !intr )
    {
        intr->Error(200, L"%s%s", L"Operation", L"Write");
        return(VXIinet_RESULT_INVALID_ARGUMENT);
    }

    VXIinetResult rc = VXIinet_RESULT_SUCCESS;
    SBinetLogFunc apiTrace (intr->GetLog(), g_SBinetDiagLogBase + 
                            MODULE_SBINET_TAGID,
                            L"SBinetInterface::Write", (int *) &rc, 
                            L"entering: 0x%p, 0x%p, %lu, 0x%p, 0x%p",
                            pThis, pBuffer, nBuflen, pnWritten, pStream);

    // check validation
    SBinetChannel* ch = intr->m_ch;

    if ( rc != VXIinet_RESULT_SUCCESS )
    {
        intr->Error(207,L"%s%d",L"rc",rc);
    }
    if ( !ch )
    {
        intr->Error(200, L"%s%s", L"Operation", L"Write");
        return(rc = VXIinet_RESULT_INVALID_ARGUMENT);
    }

    // Not currently supported
    return(rc = VXIinet_RESULT_UNSUPPORTED);
}

/*
 * Call Channel method
 */
VXIinetResult 
SBinetInterface::SetCookieJar( /* [IN]  */ VXIinetInterface*      pThis,
                               /* [IN]  */ const VXIVector*       pJar )
{
    SBinetInterface* intr = (SBinetInterface*)pThis;
    if ( !intr )
    {
        intr->Error(200, L"%s%s", L"Operation", L"SetCookieJar");
        return(VXIinet_RESULT_INVALID_ARGUMENT);
    }

    VXIinetResult rc = VXIinet_RESULT_SUCCESS;
    SBinetLogFunc apiTrace (intr->GetLog(), g_SBinetDiagLogBase + 
                            MODULE_SBINET_TAGID,
                            L"SBinetInterface::SetCookieJar", (int *) &rc, 
                            L"entering: 0x%p, 0x%p", pThis, pJar);

    // check validation
    SBinetChannel* ch = intr->m_ch;
    if ( !ch )
    {
        intr->Error(200, L"%s%s", L"Operation", L"SetCookieJar");
        return(rc = VXIinet_RESULT_INVALID_ARGUMENT);
    }
    rc = (ch->SetCookieJar)(pJar);
    if ( rc != VXIinet_RESULT_SUCCESS )
    {
        intr->Error(208,L"%s%d",L"rc",rc);
    }
    return(rc);
}

/*
 * Call Channel method
 */
VXIinetResult 
SBinetInterface::GetCookieJar( /* [IN]  */ VXIinetInterface*      pThis,
                               /* [OUT] */ VXIVector**            ppJar,
                               /* [OUT] */ VXIbool*               pfChanged   )
{
    SBinetInterface* intr = (SBinetInterface*)pThis;
    if ( !intr )
    {
        intr->Error(200, L"%s%s", L"Operation", L"GetCookieJar");
        return(VXIinet_RESULT_INVALID_ARGUMENT);
    }

    VXIinetResult rc = VXIinet_RESULT_SUCCESS;
    SBinetLogFunc apiTrace (intr->GetLog(), g_SBinetDiagLogBase + 
                            MODULE_SBINET_TAGID,
                            L"SBinetInterface::GetCookieJar", (int *) &rc, 
                            L"entering: 0x%p, 0x%p, 0x%p", pThis, ppJar,
                            pfChanged);

    // check validation
    SBinetChannel* ch = intr->m_ch;
    if ( !ch )
    {
        intr->Error(200, L"%s%s", L"Operation", L"GetCookieJar");
        return(rc = VXIinet_RESULT_INVALID_ARGUMENT);
    }
    rc = (ch->GetCookieJar)(ppJar,pfChanged);
    if ( rc != VXIinet_RESULT_SUCCESS )
    {
        intr->Error(209, L"%s%d", L"rc", rc);
    }
    return(rc);
}

/*
 * Static callback for libwww to give us cookies.
 */
BOOL SBinetInterface::setCookie(HTRequest* pHtRequest, HTCookie* pHtCookie, 
                                void* pParam)
{
    VXIbool retVal = YES;

    if ( pHtCookie )
    {
        HTParentAnchor* pParentAnchor = HTRequest_anchor( pHtRequest );
        SBinetHttpStream* st = (SBinetHttpStream*)HTRequest_context(pHtRequest);
        if ( !st ) return(NO);
        SBinetChannel* chan = st->getChannel();
        if ( !chan ) return(NO);

        if ( !chan->cookiesEnabled() ) return(NO); // Don't accept cookies

        /* These are owned by the anchor, freed when the anchor is */
        const char *pszBase           = HTAnchor_base( pParentAnchor );
        const char *pszAnchorLocation = HTAnchor_address((HTAnchor*)pParentAnchor);
        if ( pszAnchorLocation == NULL )
            pszAnchorLocation = ""; // Added this to prevent HTParse from 
                                    // failing below

        // We use the actual originating domain rather than the sent
        // domain, for security. Really should compare the sent domain with
        // the one specified by the cookie, rejecting it if the domains are
        // different (can't be a straight strcmp( ), have to allow for 
        // subdomains, see the cookie RFC).
        //
        // PARSE_HOST: Host name, e.g. "www.w3.org" 
        // PARSE_ACCESS: URL scheme, e.g. "HTTP"
        char *pszDomain  = HTParse(pszAnchorLocation, pszBase, PARSE_HOST);
        char *pszAccess  = HTParse(pszAnchorLocation, pszBase, PARSE_ACCESS); 

        // const char*  pszDomain  = HTCookie_domain( pHtCookie );
        const char *pszPath    = HTCookie_path  ( pHtCookie );
        const char *pszName    = HTCookie_name  ( pHtCookie );
        const char *pszValue   = HTCookie_value ( pHtCookie );

        // Need to check for proper number of .'s in domain as specified
        // by the cookie RFC
        if ( (pszDomain != NULL) && (pszAccess != NULL) && (pszPath != NULL) &&
             (pszName != NULL) && (pszValue != NULL) )
        {
            time_t tExpiration = HTCookie_expiration( pHtCookie );
            VXIbool fSecure = HTCookie_isSecure( pHtCookie ) ? true : false;

            if ( chan->updateCookieIfExists(pszDomain, pszPath, pszName, 
                                            pszValue, tExpiration, fSecure) == 0 )
            {
                // Create new cookie
                SBinetCookie* pSBinetCookie = new SBinetCookie( pszDomain, pszPath, 
                                                                pszName, pszValue,
                                                                tExpiration, fSecure );
                if ( pSBinetCookie != NULL )
                {
                    if ( chan->addCookie(pSBinetCookie) == 0 )
                    {
		    	delete pSBinetCookie; // Could not add
		    	pSBinetCookie = NULL; 
		    }
                } else
                    retVal = NO;
            }
        }

        if ( pszDomain != NULL )
            HT_FREE( pszDomain );
        if ( pszAccess != NULL )
            HT_FREE( pszAccess );
    }
    return(retVal);
}


HTAssocList* SBinetInterface::findCookie( HTRequest* pHtRequest, void* pParam)
{
    if ( !pHtRequest ) return NULL; // Sanity

    HTParentAnchor* pParentAnchor = HTRequest_anchor( pHtRequest );
    SBinetHttpStream* st = (SBinetHttpStream*)HTRequest_context(pHtRequest);
    if ( !st ) return(NULL);
    SBinetChannel* chan = st->getChannel();
    if ( !chan ) return(NULL);

    if ( !chan->cookiesEnabled() ) return(NULL); // Don't send cookies

    const char* pszAnchorLocation = HTAnchor_location( pParentAnchor );
    if ( pszAnchorLocation == NULL )
        pszAnchorLocation = ""; // Added this to prevent HTParse from failing
                                // below

    char* pszBase   = HTAnchor_base( pParentAnchor );

    // Host name, e.g. "www.w3.org"
    char* pszDomain = HTParse(pszAnchorLocation,pszBase,PARSE_HOST); 

    // URL Path, e.g. "pub/WWW/TheProject.html"
    char* pszPath   = HTParse(pszAnchorLocation,pszBase,PARSE_PATH);

    // Remove the filename portion of the path, leaving the trailing
    // slash or backslash. (Note we don't have to worry about the
    // query args containing a slash because the standard for query
    // args makes it so slashes are hex encoded.)
    if ( pszPath )
    {
        char *ptr = strrchr (pszPath, '/');
        char *ptr2 = strrchr (pszPath, '\\');
        if ( (! ptr) || ((ptr2) && (ptr2 > ptr)) )
            ptr = ptr2;

        if ( ptr )
            *(ptr + 1) = '\0';
        else
            strcpy (pszPath, "/");
    }

    // Collect cookies
    HTAssocList* pAssocList = chan->collectCookies(pszDomain, pszPath);

    if ( pszDomain != NULL )
        HT_FREE(pszDomain);
    if ( pszPath != NULL )
        HT_FREE(pszPath);

    return(pAssocList);
}


//
// LibWWW global trace and alert handlers (static!)
//
PUBLIC int SBinet::PrintCallback(const char * fmt, va_list pArgs)
{
    int rc = 0;

    if ( (g_SBinet) && (g_SBinet->DiagIsEnabled(MODULE_SBINET_HTPRINT_TAGID)) )
    {
        char buffer[4096];
        rc = vsprintf(buffer, fmt, pArgs);
        if ( (buffer) && (buffer[0]) )
            g_SBinet->Diag (MODULE_SBINET_HTPRINT_TAGID, NULL, L"%S", buffer);
    }

    return rc;
}


PUBLIC int SBinet::TraceCallback(const char * fmt, va_list pArgs)
{
    int rc = 0;

    if ( (g_SBinet) && (g_SBinet->DiagIsEnabled(MODULE_SBINET_HTTRACE_TAGID)) )
    {
        char buffer[4096];
        rc = vsprintf(buffer, fmt, pArgs);
        if ( (buffer) && (buffer[0]) )
            g_SBinet->Diag (MODULE_SBINET_HTTRACE_TAGID, NULL, L"%S", buffer);
    }

    return rc;
}


PUBLIC int
SBinet::TraceDataCallback(char * data, size_t len, char * fmt, va_list pArgs)
{
    int rc = 0;

    if ( (g_SBinet) && 
         (g_SBinet->DiagIsEnabled(MODULE_SBINET_HTTRACEDATA_TAGID)) )
    {
        char buffer[4096];
        rc = vsprintf(buffer, fmt, pArgs);
        if ( (data) && (len) && (rc < 4096 - 3) )
        {
            if ( rc > 0 )
            {
                ::strcat(&buffer[rc], ": ");
                rc += 2;
            } else
            {
                rc = 0;
            }

            if ( rc + len < 4096 - 1 )
            {
                ::strncat(&buffer[rc], data, len);
                rc += len;
            } else
            {
                ::strncat(&buffer[rc], data, 4095 - rc);
                rc = 4095;
            }
            buffer[rc] = 0;
        }

        if ( (buffer) && (buffer[0]) )
            g_SBinet->Diag (MODULE_SBINET_HTTRACEDATA_TAGID, NULL, L"%S", buffer);
    }

    return rc;
}


PUBLIC BOOL
SBinet::AlertCallback(HTRequest*    pHtRequest, 
                      HTAlertOpcode eOpCode,
                      int           nMessageNum,
                      const char*   pszDefault, 
                      void*         pvInput,
                      HTAlertPar*   pReply)
{
    if ( (g_SBinet) && (g_SBinet->DiagIsEnabled(MODULE_SBINET_HTALERT_TAGID)) )
    {
        // Have to free the message when we're done with it
        char *pszMessageText = 
        HTDialog_progressMessage(pHtRequest, eOpCode, nMessageNum, pszDefault,
                                 pvInput);

        if ( (pszMessageText) && (pszMessageText[0]) )
            g_SBinet->Diag (MODULE_SBINET_HTALERT_TAGID, NULL, L"%S", 
                            pszMessageText);

        HT_FREE(pszMessageText);
    }

    return((BOOL)YES ); // Confirm...
}


PUBLIC void
SBinet::OutOfMemoryExitCallback(char *name, char *file, unsigned long line)
{
    if ( g_SBinet )
        g_SBinet->Error(106, L"%s%S%s%S%s%lu", L"name", name, L"file", file,
                        L"line", line);

    // Libwww doesn't know how to recover so we have to exit, see
    // HTMemory.c. For Win32, show an exception window, on UNIX dump
    // core.
#ifdef WIN32
    static char exception_msg[4096];
    static DWORD exception_args[EXCEPTION_MAXIMUM_PARAMETERS];
    sprintf (exception_msg, 
             "Libwww out of memory for %s, file %s, line %lu",
             name, file, line);
    exception_args[0] = (DWORD) exception_msg;
    RaiseException(0xE001AEAE, EXCEPTION_NONCONTINUABLE, 1, exception_args);
#else
    abort();
#endif
}

// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

// APPLICATION INCLUDES
#include <mp/MpMediaTask.h>
#include <mp/NetInTask.h>
#ifdef INCLUDE_RTCP
#include <rtcp/RTCManager.h>
#endif // INCLUDE_RTCP
#include <net/SipUserAgent.h>
#include <net/SdpCodecFactory.h>
#include <cp/CallManager.h>
#include <mi/CpMediaInterfaceFactoryFactory.h>
#include <cp/DialogEventPublisher.h>
#include <ptapi/PtProvider.h>
#include <net/NameValueTokenizer.h>
#include <net/SipPublishContentMgr.h>
#include <net/SipSubscribeServerEventHandler.h>
#include <net/SipSubscribeServer.h>
#include <net/SipSubscriptionMgr.h>
#include <os/OsFS.h>
#include <os/OsSysLog.h>
#include <os/OsConfigDb.h>
#include "OrbitListener.h"

// DEFINES
#ifndef SIPX_VERSION
#  include "sipxpbx-buildstamp.h"
#  define SIPXCHANGE_VERSION          SipXpbxVersion
#  define SIPXCHANGE_VERSION_COMMENT  SipXpbxBuildStamp
#else
#  define SIPXCHANGE_VERSION          SIPX_VERSION
#  define SIPXCHANGE_VERSION_COMMENT  ""
#endif

#define CONFIG_SETTINGS_FILE          "sipxpark-config"
#define CONFIG_ETC_DIR                SIPX_CONFDIR

#define CONFIG_LOG_FILE               "sipxpark.log"
#define CONFIG_LOG_DIR                SIPX_LOGDIR

#define CONFIG_SETTING_LOG_DIR        "SIP_PARK_LOG_DIR"
#define CONFIG_SETTING_LOG_LEVEL      "SIP_PARK_LOG_LEVEL"
#define CONFIG_SETTING_LOG_CONSOLE    "SIP_PARK_LOG_CONSOLE"
#define CONFIG_SETTING_UDP_PORT       "SIP_PARK_UDP_PORT"
#define CONFIG_SETTING_TCP_PORT       "SIP_PARK_TCP_PORT"
#define CONFIG_SETTING_RTP_PORT       "SIP_PARK_RTP_PORT"
#define CONFIG_SETTING_CODECS         "SIP_PARK_CODEC_LIST"
#define CONFIG_SETTING_MAX_SESSIONS   "SIP_PARK_MAX_SESSIONS"
#define CONFIG_SETTING_LIFETIME       "SIP_PARK_LIFETIME"
#define CONFIG_SETTING_BLIND_WAIT     "SIP_PARK_BLIND_XFER_WAIT"

#define LOG_FACILITY                  FAC_ACD

#define PARK_DEFAULT_UDP_PORT         5120       // Default UDP port
#define PARK_DEFAULT_TCP_PORT         5120       // Default TCP port
#define DEFAULT_RTP_PORT              8000       // Starting RTP port

#define DEFAULT_CODEC_LIST            "pcmu pcma telephone-event"

#define DEFAULT_MAX_SESSIONS          50         // Max number of sim. conns
#define MP_SAMPLE_RATE                8000       // Sample rate (don't change)
#define MP_SAMPLES_PER_FRAME          80         // Frames per second (don't change)

#define DEFAULT_LIFETIME              86400      // Default max. lifetime for parked,
                                                 // call, 24 hours.
#define DEFAULT_BLIND_WAIT            60         // Default time to wait for a
                                                 // blind transfer to succeed or
                                                 // fail, 60 seconds.
#define CONS_XFER_WAIT                1          // Time to wait for a cons. transfer
                                                 // to succeed or fail, 1 second.
                                                 // This is not configurable via
                                                 // sipxpark-config, as a cons. xfer.
                                                 // should succeed or fail immediately.

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef void (*sighandler_t)(int);

// FUNCTIONS
extern "C" {
    void  sigHandler( int sig_num );
    sighandler_t pt_signal( int sig_num, sighandler_t handler );
}

// FORWARD DECLARATIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// GLOBAL VARIABLE INITIALIZATIONS
UtlBoolean    gShutdownFlag = FALSE;



/* ============================ FUNCTIONS ================================= */

/**
 * Description:
 * This is a replacement for signal() which registers a signal handler but sets
 * a flag causing system calls ( namely read() or getchar() ) not to bail out
 * upon recepit of that signal. We need this behavior, so we must call
 * sigaction() manually.
 */
sighandler_t pt_signal( int sig_num, sighandler_t handler)
{
#if defined(__pingtel_on_posix__)
    struct sigaction action[2];
    action[0].sa_handler = handler;
    sigemptyset(&action[0].sa_mask);
    action[0].sa_flags = 0;
    sigaction ( sig_num, &action[0], &action[1] );
    return action[1].sa_handler;
#else
    return signal( sig_num, handler );
#endif
}


/**
 * Description:
 * This is the signal handler, When called this sets the
 * global gShutdownFlag allowing the main processing
 * loop to exit cleanly.
 */
void sigHandler( int sig_num )
{
    // set a global shutdown flag
    gShutdownFlag = TRUE;

    // Unregister interest in the signal to prevent recursive callbacks
    pt_signal( sig_num, SIG_DFL );

    // Minimize the chance that we loose log data
    OsSysLog::flush();
    if (SIGTERM == sig_num)
    {
       OsSysLog::add( LOG_FACILITY, PRI_INFO, "sigHandler: terminate signal received.");
    }
    else
    {
       OsSysLog::add( LOG_FACILITY, PRI_CRIT, "sigHandler: caught signal: %d", sig_num );
    }
    OsSysLog::flush();
}


// Initialize the OsSysLog
void initSysLog(OsConfigDb* pConfig)
{
   UtlString logLevel;               // Controls Log Verbosity
   UtlString consoleLogging;         // Enable console logging by default?
   UtlString fileTarget;             // Path to store log file.
   UtlBoolean bSpecifiedDirError ;   // Set if the specified log dir does not
                                    // exist
   struct tagPrioriotyLookupTable
   {
      const char*      pIdentity;
      OsSysLogPriority ePriority;
   };

   struct tagPrioriotyLookupTable lkupTable[] =
   {
      { "DEBUG",   PRI_DEBUG},
      { "INFO",    PRI_INFO},
      { "NOTICE",  PRI_NOTICE},
      { "WARNING", PRI_WARNING},
      { "ERR",     PRI_ERR},
      { "CRIT",    PRI_CRIT},
      { "ALERT",   PRI_ALERT},
      { "EMERG",   PRI_EMERG},
   };

   OsSysLog::initialize(0, "sipxpark");


   //
   // Get/Apply Log Filename
   //
   fileTarget.remove(0);
   if ((pConfig->get(CONFIG_SETTING_LOG_DIR, fileTarget) != OS_SUCCESS) ||
      fileTarget.isNull() || !OsFileSystem::exists(fileTarget))
   {
      bSpecifiedDirError = !fileTarget.isNull();

      // If the log file directory exists use that, otherwise place the log
      // in the current directory
      OsPath workingDirectory;
      if (OsFileSystem::exists(CONFIG_LOG_DIR))
      {
         fileTarget = CONFIG_LOG_DIR;
         OsPath path(fileTarget);
         path.getNativePath(workingDirectory);

         osPrintf("%s : %s\n", CONFIG_SETTING_LOG_DIR, workingDirectory.data());
         OsSysLog::add(LOG_FACILITY, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_DIR, workingDirectory.data());
      }
      else
      {
         OsPath path;
         OsFileSystem::getWorkingDirectory(path);
         path.getNativePath(workingDirectory);

         osPrintf("%s : %s\n", CONFIG_SETTING_LOG_DIR, workingDirectory.data());
         OsSysLog::add(LOG_FACILITY, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_DIR, workingDirectory.data());
      }

      fileTarget = workingDirectory +
         OsPathBase::separator +
         CONFIG_LOG_FILE;
   }
   else
   {
      bSpecifiedDirError = false;
      osPrintf("%s : %s\n", CONFIG_SETTING_LOG_DIR, fileTarget.data());
      OsSysLog::add(LOG_FACILITY, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_DIR, fileTarget.data());

      fileTarget = fileTarget +
         OsPathBase::separator +
         CONFIG_LOG_FILE;
   }
   OsSysLog::setOutputFile(0, fileTarget);


   //
   // Get/Apply Log Level
   //
   if ((pConfig->get(CONFIG_SETTING_LOG_LEVEL, logLevel) != OS_SUCCESS) ||
         logLevel.isNull())
   {
      logLevel = "ERR";
   }
   logLevel.toUpper();
   OsSysLogPriority priority = PRI_ERR;
   int iEntries = sizeof(lkupTable) / sizeof(struct tagPrioriotyLookupTable);
   for (int i = 0; i < iEntries; i++)
   {
      if (logLevel == lkupTable[i].pIdentity)
      {
         priority = lkupTable[i].ePriority;
         osPrintf("%s : %s\n", CONFIG_SETTING_LOG_LEVEL, lkupTable[i].pIdentity);
         OsSysLog::add(LOG_FACILITY, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_LEVEL, lkupTable[i].pIdentity);
         break;
      }
   }
   OsSysLog::setLoggingPriority(priority);

   //
   // Get/Apply console logging
   //
   UtlBoolean bConsoleLoggingEnabled = false;
   if ((pConfig->get(CONFIG_SETTING_LOG_CONSOLE, consoleLogging) == OS_SUCCESS))
   {
      consoleLogging.toUpper();
      if (consoleLogging == "ENABLE")
      {
         OsSysLog::enableConsoleOutput(true);
         bConsoleLoggingEnabled = true;
      }
   }

   osPrintf("%s : %s\n", CONFIG_SETTING_LOG_CONSOLE, bConsoleLoggingEnabled ? "ENABLE" : "DISABLE") ;
   OsSysLog::add(LOG_FACILITY, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_CONSOLE, bConsoleLoggingEnabled ? "ENABLE" : "DISABLE") ;

   if (bSpecifiedDirError)
   {
      OsSysLog::add(FAC_LOG, PRI_CRIT, "Cannot access %s directory; please check configuration.", CONFIG_SETTING_LOG_DIR);
   }
}


/**
 * Description:
 * Build an SdpCodecFactory based upon the codec list specified in the config file.
 */
void initCodecs(SdpCodecFactory* codecFactory, OsConfigDb* pConfig)
{
   UtlString codecList;
   UtlString oneCodec;
   int codecStringIndex = 0;
   SdpCodec::SdpCodecTypes internalCodecId;

   if ((pConfig->get(CONFIG_SETTING_CODECS, codecList) != OS_SUCCESS) ||
       codecList.isNull())
   {
      codecList = DEFAULT_CODEC_LIST;
   }

   NameValueTokenizer::getSubField(codecList, codecStringIndex, ", \n\r\t", &oneCodec);

   while(!oneCodec.isNull())
   {
      internalCodecId = SdpCodecFactory::getCodecType(oneCodec.data());
      if (internalCodecId == SdpCodec::SDP_CODEC_UNKNOWN)
      {
         OsSysLog::add(FAC_ACD, PRI_ERR, "initCodecs: Unknown codec ID: %s",
                       oneCodec.data());
      }
      else
      {
         codecFactory->buildSdpCodecFactory(1, &internalCodecId);
      }

      codecStringIndex++;
      NameValueTokenizer::getSubField(codecList, codecStringIndex, ", \n\r\t", &oneCodec);
   }
}


//
// The main entry point to the sipXpark
//
int main(int argc, char* argv[])
{
    // Configuration Database (used for OsSysLog)
    OsConfigDb configDb;

    // Register Signal handlers so we can perform graceful shutdown
    pt_signal(SIGINT,   sigHandler);    // Trap Ctrl-C on NT
    pt_signal(SIGILL,   sigHandler);
    pt_signal(SIGABRT,  sigHandler);    // Abort signal 6
    pt_signal(SIGFPE,   sigHandler);    // Floading Point Exception
    pt_signal(SIGSEGV,  sigHandler);    // Address access violations signal 11
    pt_signal(SIGTERM,  sigHandler);    // Trap kill -15 on UNIX
#if defined(__pingtel_on_posix__)
    pt_signal(SIGHUP,   sigHandler);    // Hangup
    pt_signal(SIGQUIT,  sigHandler);
    pt_signal(SIGPIPE,  SIG_IGN);    // Handle TCP Failure
    pt_signal(SIGBUS,   sigHandler);
    pt_signal(SIGSYS,   sigHandler);
    pt_signal(SIGXCPU,  sigHandler);
    pt_signal(SIGXFSZ,  sigHandler);
    pt_signal(SIGUSR1,  sigHandler);
    pt_signal(SIGUSR2,  sigHandler);
#endif

    UtlString argString;
    for(int argIndex = 1; argIndex < argc; argIndex++)
    {
        osPrintf("arg[%d]: %s\n", argIndex, argv[argIndex]);
        argString = argv[argIndex];
        NameValueTokenizer::frontBackTrim(&argString, "\t ");
        if(argString.compareTo("-v") == 0)
        {
            osPrintf("Version: %s (%s)\n", SIPXCHANGE_VERSION, SIPXCHANGE_VERSION_COMMENT);
            return(1);
        } else
        {
           osPrintf("usage: %s [-v]\nwhere:\n -v provides the software version\n",
            argv[0]);
            return(1);
        }
    }

    // Load configuration file file
    OsPath workingDirectory;
    if (OsFileSystem::exists(CONFIG_ETC_DIR))
    {
        workingDirectory = CONFIG_ETC_DIR;
        OsPath path(workingDirectory);
        path.getNativePath(workingDirectory);
    } else
    {
        OsPath path;
        OsFileSystem::getWorkingDirectory(path);
        path.getNativePath(workingDirectory);
    }

    UtlString fileName =  workingDirectory +
      OsPathBase::separator +
      CONFIG_SETTINGS_FILE;

    configDb.loadFromFile(fileName);

    // Initialize log file
    initSysLog(&configDb);

    // Read the user agent parameters from the config file.
    int UdpPort;
    if (configDb.get(CONFIG_SETTING_UDP_PORT, UdpPort) != OS_SUCCESS)
        UdpPort = PARK_DEFAULT_UDP_PORT;

    int TcpPort;
    if (configDb.get(CONFIG_SETTING_TCP_PORT, TcpPort) != OS_SUCCESS)
        TcpPort = PARK_DEFAULT_TCP_PORT;

    int RtpBase;
    if (configDb.get(CONFIG_SETTING_RTP_PORT, RtpBase) != OS_SUCCESS)
        RtpBase = DEFAULT_RTP_PORT;

    int MaxSessions;
    if (configDb.get(CONFIG_SETTING_MAX_SESSIONS, MaxSessions) != OS_SUCCESS)
        MaxSessions = DEFAULT_MAX_SESSIONS;

    // Read Park Server parameters from the config file.
    int Lifetime, BlindXferWait, ConsXferWait;
    if (configDb.get(CONFIG_SETTING_LIFETIME, Lifetime) != OS_SUCCESS)
    {
       Lifetime = DEFAULT_LIFETIME;
    }
    if (configDb.get(CONFIG_SETTING_BLIND_WAIT, BlindXferWait) != OS_SUCCESS)
    {
       BlindXferWait = DEFAULT_BLIND_WAIT;
    }
    // This is not configurable, as consultative transfers should
    // succeed or fail immediately.
    ConsXferWait = CONS_XFER_WAIT;

    // Bind the SIP user agent to a port and start it up
    SipUserAgent* userAgent = new SipUserAgent(TcpPort, UdpPort);
    userAgent->start();

    // Read the list of codecs from the configuration file.
    SdpCodecFactory codecFactory;
    initCodecs(&codecFactory, &configDb);

    // Initialize and start up the media subsystem
    mpStartUp(MP_SAMPLE_RATE, MP_SAMPLES_PER_FRAME, 6 * MaxSessions, &configDb);
    MpMediaTask::getMediaTask(MaxSessions);

#ifdef INCLUDE_RTCP
    CRTCManager::getRTCPControl();
#endif //INCLUDE_RTCP

    mpStartTasks();

    // Instantiate the call processing subsystem
    UtlString localAddress;
    OsSocket::getHostIp(&localAddress);
    // Construct an address to be used in outgoing requests (primarily
    // INVITEs stimulated by REFERs).
    UtlString outgoingAddress;
    {
       char buffer[100];
       sprintf(buffer, "sip:sipXpark@%s:%d", localAddress.data(),
               portIsValid(UdpPort) ? UdpPort : TcpPort);
       outgoingAddress = buffer;
    }
    CallManager callManager(FALSE,
                           NULL,
                           TRUE,                              // early media in 180 ringing
                           &codecFactory,
                           RtpBase,                           // rtp start
                           RtpBase + (2 * MaxSessions),       // rtp end
                           localAddress,
                           localAddress,
                           userAgent, 
                           0,                                 // sipSessionReinviteTimer
                           NULL,                              // mgcpStackTask
                           outgoingAddress,                   // defaultCallExtension
                           Connection::RING,                  // availableBehavior
                           NULL,                              // unconditionalForwardUrl
                           -1,                                // forwardOnNoAnswerSeconds
                           NULL,                              // forwardOnNoAnswerUrl
                           Connection::BUSY,                  // busyBehavior
                           NULL,                              // sipForwardOnBusyUrl
                           NULL,                              // speedNums
                           CallManager::SIP_CALL,             // phonesetOutgoingCallProtocol
                           4,                                 // numDialPlanDigits
                           CallManager::NEAR_END_HOLD,        // holdType
                           5000,                              // offeringDelay
                           "",                                // pLocal
                           CP_MAXIMUM_RINGING_EXPIRE_SECONDS, // inviteExpiresSeconds
                           QOS_LAYER3_LOW_DELAY_IP_TOS,       // expeditedIpTos
                           MaxSessions,                       // maxCalls
                           sipXmediaFactoryFactory(NULL));    // CpMediaInterfaceFactory



    // Create a listener (application) to deal with call
    // processing events (e.g. incoming call and hang ups)
    OrbitListener listener(&callManager, Lifetime, BlindXferWait, ConsXferWait);

    callManager.addTaoListener(&listener);
    listener.start();

    // Create the SIP Subscribe Server
    SipSubscriptionMgr subscriptionMgr; // Component for holding the subscription data
    SipSubscribeServerEventHandler policyHolder; // Component for granting the subscription rights
    SipPublishContentMgr publisher; // Component for publishing the event contents

    SipSubscribeServer subscribeServer(*userAgent, publisher,
                                       subscriptionMgr, policyHolder);
    subscribeServer.enableEventType(DIALOG_EVENT_TYPE);
    subscribeServer.start();

    // Create the dialog event publisher
    DialogEventPublisher dialogEvents(&callManager, &publisher);
    callManager.addTaoListener(&dialogEvents);
    dialogEvents.start();

    // Start up the call processing system
    callManager.start();
   
    // Loop forever until signaled to shut down
    while (!gShutdownFlag)
    {
       OsTask::delay(2000);
    }

    // Flush the log file
    OsSysLog::flush();

    // Say goodnight Gracie...
    return 0;
}


// Stub to avoid pulling in ps library
int JNI_LightButton(long)
{
   return 0;
}

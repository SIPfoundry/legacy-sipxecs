// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

static const char *rcsid = 0 ? (char *) &rcsid :
"";


// System includes
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#if defined(_WIN32)
#include <windows.h>
#define sleep Sleep
#elif defined(__pingtel_on_posix__)
#include <unistd.h>
#include "os/linux/OsUtilLinux.h"
#endif



#include "os/OsFS.h"

#include "net/NameValueTokenizer.h"
#include "net/SdpCodecFactory.h"
#include "net/Url.h"
#include "net/SipLine.h"
#include "net/SipLineMgr.h"
#include "net/SipUserAgent.h"

#include "sipXecsService/SipXecsService.h"
#include "sipdb/CredentialDB.h"

#include <mp/MpMediaTask.h>
#include <mp/NetInTask.h>
#include <rtcp/RtcpConfig.h>
#include <rtcp/RTCManager.h>
#include <cp/CallManager.h>
#include <mi/CpMediaInterfaceFactoryFactory.h>
#include <ptapi/PtProvider.h>

#include "VXIlog.h"
#include "VXItrd.h"
#include "SBclientUtils.h"
#include "ConfigFile.h"
#include "clientMain.h"
#include "OSBclient.h"
#include "IvrCallListener.h"
#include "IvrTelListener.h"
#include "IvrUtilTask.h"
#include "OSBprompt.h"

#ifndef SIPX_VERSION
#  include "sipxvxml-buildstamp.h"
#  define SIPX_VERSION SipXvxmlVersion
#  define SIPX_BUILD   SipXvxmlBuildStamp
#else
#  define SIPX_BUILD   ""
#endif

#ifdef DMALLOC
#include <dmalloc.h>
#endif
typedef std::basic_string<VXIchar> vxistring;
#define VXICHAR_SIZE (sizeof(VXIchar))
#define MAX_NB_CALLS  1000

#define CHANNEL_CHECK_RESULT(_func, _res) \
if ((VXIint)(_res) != 0) { \
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, "%s failed, error code %i, file %s, line %i\n",\
         (_func), (_res), __FILE__, __LINE__); \
   OsSysLog::flush(); \
   return ((VXItrdThreadArg)_res); \
}

#if defined(_WIN32)
#define SIGPIPE 13 
#endif

#ifdef WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

/* Defaults for command line arguments */
#ifdef OPENVXI
#define DEFAULT_CONFIG_FILENAME       "OSBclient.cfg"
#else
#define DEFAULT_CONFIG_FILENAME       "SBclient.cfg"
#endif
#define DEFAULT_NB_CHANNELS           1
#define DEFAULT_MAX_CALLS             -1 /* loop forever */

/* Configuration file parameters */
#define BASE_CLIENT_VXML_URL          L"client.vxmlURL"
#define MEDIASERVER_LOG   "mediaserver.log"
#define LOG_DIR                   L"client.log.dir"
#define MAX_ACTIVE_CALLS  L"mediaserver.max.active.calls"
#define SYS_LOG_LEVEL     L"mediaserver.log.level"
#define MEDIASERVER_RTP_CODECS L"mediaserver.rtp.codecs"

#define DEFAULT_CODEC_LIST_STRING "pcmu pcma telephone-event"

#define DEFAULT_MAX_ACTIVE_CALLS 100
#define DEFAULT_MAX_LISTENERS 200

const char* MEDIASERVER_ID_TOKEN = "~~id~media"; // see sipXregistry/doc/service-tokens.txt

//#define STACK_DEBUG 1

const char *EMPTY_STRING = "";
const char *ERR_STR = "ERR";

VXIchar *gblDefaultVxmlURL = NULL;
UtlSList glbChannelStack;
UtlSList glbCleanupThreadStack;

VXItrdMutex* glbpChannelMutex = 0;
VXItrdMutex* glbpThreadMutex = 0;

/* Structure for controlling channel threads */
typedef struct ChannelThreadArgs {
   char                          *callId;
   VXIunsigned    channelNum;
   VXIunsigned    maxCalls;
   const VXIMap  *configArgs;
   const VXIMap  *sessionArgs;
   const VXIchar *vxmlURL;
   VXItrdThread  *threadHandle;
   CallManager    *pCallMgr;
   VXIplatform   *platform;
} ChannelThreadArgs;


/* Structure for cleanup threads */
typedef struct CallCleanupThreadArgs {
   char  *callId;
   VXItrdThread  *threadHandle;
   ChannelThreadArgs* channel;
   int          VXISessionEnded;
} CallCleanupThreadArgs;


unsigned long glbNbCalls;
VXIMap *glbConfigArgs;
//VXIplatform *glbPlatform;
IvrTelListener *gpTelListener = 0;

typedef void (*sighandler_t)(int);

// Function that is the body of the thread that watches the length of the
// Media Task's message queue and reduces the number of allowed calls
// if the queue stays too large for too long.
void* autoThrottleFunction(void* arg);

void initCodecs(SdpCodecFactory* codecFactory);


/**
 * Description:
 * Establish a signal handler for a signal.
 * On non-POSIX systems, it just calls signal().
 * On POSIX systems, uses sigaction() so that it can set the flag causing
 * I/O system calls (specifically read() and getchar()) not to be interrupted
 * and return EINTR.
 * In either case, pt_signal() returns the previous handler for this signal.
 */
   sighandler_t
pt_signal(int sig_num, sighandler_t handler)
{
#if defined(__pingtel_on_posix__)
   // Define two sigaction structures, action[0] specifies the new action,
   // action[1] receives previous action.
   struct sigaction action[2];
   // Set up action[0] to specify the new handler.
   action[0].sa_handler = handler;
   // Don't mask any other signals during signal handling.
   sigemptyset(&action[0].sa_mask);
   // Set all special flags off, in particular SA_RESTART, so I/O calls
   // are not interrupted.
   action[0].sa_flags = 0;
   // Call sigaction().
   sigaction(sig_num, &action[0], &action[1]);
   return action[1].sa_handler;
#else
   return signal(sig_num, handler);
#endif
}

/* A thread can set gShutdownFlag to true to cause the main thread to clean
 * up execution and exit.  If it desires to indicate that the process should
 * exit with a failure status, it should set gExitStatus to EXIT_FAILURE
 * before setting gShutdownFlag to true.  It should not set gExitStatus to
 * EXIT_SUCCESS, because another thread might be simultaneously demanding
 * a failure exit.
 */
bool gShutdownFlag = FALSE;
int gExitStatus = EXIT_SUCCESS;

/** 
 * Description: 
 *
 * This is the signal handler.  Signals are processed as follows:
 *
 * SIGINT ("control-C") and SIGTERM are taken as legitimate signals to
 * shut down.  They set gShutdownFlag to cause the main thread to
 * clean up execution and exit.
 *
 * All other signals are assumed to be errors.  Log an appropriate
 * message, flush the log, then send ourselves a SIGSEGV to cause a
 * core dump (if "ulimit" allows it).
 * 
 * Which signals are seen by sigHandler() is determined by the main
 * line code which sets up the signal handling.
 */

// TRUE when executing from within a signal handler.
static int sInSigHandler = 0;

void 
sigHandler(int sig_num)
{
   // sInSigHandler is used to make sure we do not get into a loop
   // if OsSysLogTask throws a signal doing flush or add.
   if (sInSigHandler)
   {
      // Die by calling abort.
      abort() ;
      // In the unlikely case abort returns 
      exit(1);
   }
   sInSigHandler = TRUE;

   // Log receiving the signal.
   if (SIGTERM == sig_num)
   {
      OsSysLog::add( FAC_MEDIASERVER_VXI, PRI_INFO, "sigHandler: terminate signal received.");
   }
   else
   {
      OsSysLog::add( FAC_MEDIASERVER_VXI, PRI_CRIT, "sigHandler: caught signal: %d", sig_num );
   }

   // Flush the log to disk.
   OsSysLog::flush();

   // Otherwise, shut down gracefully.
   gShutdownFlag = TRUE;

   sInSigHandler = FALSE;
}

VXItrdResult   LockThreadStack()
{
   if (glbpThreadMutex)
      return VXItrdMutexLock(glbpThreadMutex);
   else
      return VXItrd_RESULT_FAILURE;
}

VXItrdResult   UnlockThreadStack()
{
   if (glbpThreadMutex)
      return VXItrdMutexUnlock(glbpThreadMutex);
   else
      return VXItrd_RESULT_FAILURE;
}

VXItrdResult   LockChannel()
{
   if (glbpChannelMutex)
      return VXItrdMutexLock(glbpChannelMutex);
   else
      return VXItrd_RESULT_FAILURE;
}

VXItrdResult   UnlockChannel()
{
   if (glbpChannelMutex)
      return VXItrdMutexUnlock(glbpChannelMutex);
   else
      return VXItrd_RESULT_FAILURE;
}

int getStackSize(UtlSList& channelStack)
{ 
   return (channelStack.entries());
}

ChannelThreadArgs* findHandlingChannel(const char* callId)
{
   LockChannel();
#ifdef TEST_PRINT
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "findHandlingChannel channelStack (0x%08x) for %s\n", (int)&glbChannelStack, callId);
#endif
   ChannelThreadArgs* handlingChannel = NULL;
   if (!glbChannelStack.isEmpty())
   {
#ifdef TEST_PRINT
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "findHandlingChannel entries=(%d)\n", getStackSize(glbChannelStack));
#endif
      UtlSListIterator iterator(glbChannelStack);
      UtlInt* channelCollectable;
      ChannelThreadArgs* channel;
      channelCollectable = (UtlInt*)iterator();
      while (channelCollectable && !handlingChannel)
      {
         channel = (ChannelThreadArgs*)channelCollectable->getValue();
#ifdef TEST_PRINT
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "channel=(0x%08x)\n", (int)channel);
#endif
         if(channel && channel->callId && (strcmp(channel->callId, callId) == 0))
         {
            handlingChannel = channel;
         }
         channelCollectable = (UtlInt*)iterator();
      }
   }
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "findHandlingChannel exiting channelStack (0x%08x) for %s found channel=(0x%08x)\n", (int)&glbChannelStack, callId, (int)handlingChannel);

   UnlockChannel();
   return(handlingChannel);
}

CallCleanupThreadArgs* findCleanupThread(const char* callId)
{
   LockThreadStack();

#ifdef TEST_PRINT
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "findCleanupThread channelStack (0x%08x) for %s\n", (int)&glbCleanupThreadStack, callId);
#endif
   CallCleanupThreadArgs* handlingChannel = NULL;
   if (!glbCleanupThreadStack.isEmpty())
   {
#ifdef TEST_PRINT
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "findCleanupThread entries=(%d)\n", getStackSize(glbCleanupThreadStack));
#endif
      UtlSListIterator iterator(glbCleanupThreadStack);
      UtlInt* channelCollectable;
      CallCleanupThreadArgs* channel;
      channelCollectable = (UtlInt*)iterator();
      while (channelCollectable && !handlingChannel)
      {
         channel = (CallCleanupThreadArgs*)channelCollectable->getValue();
#ifdef TEST_PRINT
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "channel=(0x%08x)\n", (int)channel);
#endif
         if(channel && channel->callId && (strcmp(channel->callId, callId) == 0))
         {
            handlingChannel = channel;
         }
         channelCollectable = (UtlInt*)iterator();
      }
   }
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "findCleanupThread exiting channelStack (0x%08x) for %s found thread=(0x%08x)\n", (int)&glbCleanupThreadStack, callId, (int)handlingChannel);

   UnlockThreadStack();
   return(handlingChannel);
}

void pushChannel(void* channel)
{
   LockChannel();
#ifdef TEST_PRINT
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "pushChannel: channel=0x%08x, callId=%s", (int)channel, ((ChannelThreadArgs*)channel)->callId);
#endif

   glbChannelStack.insertAt(0, new UtlInt((int) channel));
   UnlockChannel();
}

void pushThread(void* thread)
{
   LockThreadStack();
#ifdef TEST_PRINT
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "pushThread: thread=0x%08x, callId=%s", (int)thread, ((CallCleanupThreadArgs*)thread)->callId);
#endif

   glbCleanupThreadStack.insertAt(0, new UtlInt((int) thread));
        
   UnlockThreadStack();
}

CallCleanupThreadArgs* popThread()
{
   LockThreadStack();

   CallCleanupThreadArgs* thread = NULL;
   UtlInt* threadCollectable = (UtlInt*) glbCleanupThreadStack.get();
   if(threadCollectable)
   {
      thread = (CallCleanupThreadArgs*) threadCollectable->getValue();
      delete threadCollectable;
      threadCollectable = NULL;
   }

   UnlockThreadStack();
   return(thread);
}

ChannelThreadArgs* popChannel()
{
   LockChannel();
   ChannelThreadArgs* channel = NULL;
   UtlInt* channelCollectable = (UtlInt*) glbChannelStack.get();
   if(channelCollectable)
   {
      channel = (ChannelThreadArgs*) channelCollectable->getValue();
      delete channelCollectable;
      channelCollectable = NULL;
   }
   UnlockChannel();
   return(channel);
}

ChannelThreadArgs* removeChannel(const ChannelThreadArgs* channel)
{
   LockChannel();
   if (channel)
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "removeChannel: channel=0x%08x, callId=%s \n", (int)channel, channel->callId);

   UtlInt matchChannel((int)channel);
   UtlInt* channelCollectable = (UtlInt*) glbChannelStack.remove(&matchChannel);
   if(channelCollectable)
   {
      channel = (ChannelThreadArgs*) channelCollectable->getValue();
      delete channelCollectable;
      channelCollectable = NULL;
   }
   else
   {
      channel = NULL;
   }
   if (channel)
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "removeChannel exiting: channel=0x%08x, callId=%s \n", (int)channel, channel->callId);

   UnlockChannel();
   return((ChannelThreadArgs*)channel);
}

CallCleanupThreadArgs* removeThread(CallCleanupThreadArgs* thread)
{
   if (thread)
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "removeThread: channel=0x%08x, callId=%s \n", (int)thread, thread->callId);

   UtlInt matchThread((int)thread);
   UtlInt* channelCollectable = (UtlInt*) glbCleanupThreadStack.remove(&matchThread);
   if(channelCollectable)
   {
      thread = (CallCleanupThreadArgs*) channelCollectable->getValue();
      delete channelCollectable;
      channelCollectable = NULL;
   }
   else
   {
      thread = NULL;
   }
   if (thread)
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "removeThread exiting: channel=0x%08x, callId=%s \n", (int)thread, thread->callId);

   return(thread);
}

void freeChannel(ChannelThreadArgs* channel)
{
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "free channel (%d) thread for %s\n", channel->channelNum, channel->callId);
   delete[] channel->callId;
   channel->callId = NULL;

   VXIMapDestroy((struct VXIMap **)(&(channel->sessionArgs)));
   if ( channel->vxmlURL )
   {
      free((VXIchar *) channel->vxmlURL);
      channel->vxmlURL = NULL;
   }
   if (channel->sessionArgs)
   {
      VXIMap* sessionArgs = (VXIMap*)channel->sessionArgs;
      VXIMapDestroy(&sessionArgs);
      sessionArgs = NULL;
   }

   free(channel);
   channel = NULL;
}

void freeThread(CallCleanupThreadArgs* thread, bool cancel)
{
   VXItrdResult trdResult;
   VXItrdThreadArg status;
   UtlBoolean freed = FALSE;
   trdResult = VXItrdThreadJoin (thread->threadHandle, &status, 10000);
   if (trdResult == 0)
   {
      freed = TRUE;
   }
   else if (cancel)
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "main thread: Failed to destroy cleanup thread for %s - returned %d w status %d\n",  thread->callId, (int)trdResult, (int)status);
      VXItrdThreadCancel (thread->threadHandle);
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "main thread: Cancel cleanup thread for %s \n",  thread->callId);
      freed = TRUE;
   }

   if (freed)
   {
      VXItrdThreadDestroyHandle (&(thread->threadHandle));
      delete[] thread->callId;
      thread->callId = NULL;

      delete thread;
      thread = 0;
   }
   else
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, "main thread: Failed to free cleanup thread for %s - returned %d w status %d\n",  thread->callId, (int)trdResult, (int)status);
   }
}


///////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
   sInSigHandler = 0;
   // Register Signal handlers
   pt_signal(SIGINT,   sigHandler);    // Trap Ctrl-C
   // SIGTERM is the standard Unix way to tell a process to die.
   pt_signal(SIGTERM,  sigHandler);
#if defined(__pingtel_on_posix__)
   pt_signal(SIGHUP,   sigHandler);    // Hangup
   pt_signal(SIGQUIT,  sigHandler); 
   // SIGPIPE can result from TCP failure, so it has to be ignored.
   pt_signal(SIGPIPE,  SIG_IGN);
#endif

   char *configFile = NULL, *ptr = NULL;
   VXIplatformResult platformResult;
   VXIunsigned nbChannels = DEFAULT_NB_CHANNELS;
   VXIunsigned maxChannels;
   VXIint maxCalls = DEFAULT_MAX_CALLS;
   long tempLong;
   int i;

   printf("\nCommand-line arguments :\n"
          "  [-url vxmlDocURL] [-channels nbChannels] [-config configFile] "
          "[-calls maxCalls]\n"
          "To use all hardware channels, set nbChannels to 0\n"
          "To take unlimited calls, set maxCalls to -1\n\n");

   if( ( argc % 2 ) != 1 )
   {
      printf("ERROR: Invalid number of command-line arguments\n");
      return 1;
   }

   for( i = 1 ; i < argc ; i += 2 )
   {
      int j = i + 1;

      /* Determine the URL of the initial document to be fetched */
      /* Determine the number of channels */
      if (strcmp(argv[i], "-channels") == 0)
      {
         tempLong = strtol(argv[j], &ptr, 10);
         if ((tempLong < 0) || (*ptr != '\0'))
         {
            fprintf(stderr, "ERROR: Invalid number of channels '%s'\n",
                    argv[j]);
            return 1;
         }
         nbChannels = (VXIunsigned) tempLong;
      }
      /* Determine the maximum number of calls */
      else if( strcmp( argv[i], "-calls" ) == 0 )
      {
         tempLong = strtol(argv[j], &ptr, 10);
         if ((tempLong < -1) || (*ptr != '\0'))
         {
            fprintf(stderr, "ERROR: Invalid number of calls '%s'\n",
                    argv[j]);
            return 1;
         }
         maxCalls = (VXIunsigned) tempLong;
      }
      /* Determine the location of the config file */
      else if (strcmp(argv[i], "-config") == 0)
      {
         configFile = (char *) calloc(strlen(argv[j]) + 1, sizeof(char));
         CLIENT_CHECK_MEMALLOC(configFile, "config filename");
         strcpy(configFile, argv[j]);
      }
      else
      {
         fprintf(stderr, "ERROR: Invalid command-line option '%s'\n",
                 argv[i]);
         return 1;
      }
   }

   /* Use the default location for the config file */
   if (configFile == NULL)
   {
      const char *swiDir = getenv( "SWISBSDK" );
      if(( swiDir == NULL ) || ( swiDir[0] == '\0' )) swiDir = "..";

      configFile = (char *) calloc(strlen( swiDir ) + strlen( "config" ) +
                                   strlen( DEFAULT_CONFIG_FILENAME ) + 3,
                                   sizeof(char));
      CLIENT_CHECK_MEMALLOC( configFile, "config filename" );

      sprintf( configFile, "%s%c%s%c%s", swiDir, PATH_SEPARATOR,
               "config", PATH_SEPARATOR, DEFAULT_CONFIG_FILENAME );
   }

   osPrintf("Using config file '%s'\n\n", configFile);

   glbMaxNumListeners = DEFAULT_MAX_LISTENERS;
   glbPlayerListenerTable = (PlayerListenerDB**) malloc(sizeof(PlayerListenerDB *)*glbMaxNumListeners);
   for (i = 0; i < glbMaxNumListeners; i++)
   {
      glbPlayerListenerTable[i] = NULL;
   }
   glbPlayerListenerCnt = 0;

   /* Parse the configuration file */
   platformResult = ParseConfigFile(&glbConfigArgs, configFile);
   CLIENT_CHECK_RESULT("ParseConfigFile()", platformResult);

   /* Determine the URL if not specified on the command line */
   if (gblDefaultVxmlURL == NULL)
   {
      const VXIString *urlStr = 
         (const VXIString *)VXIMapGetProperty(glbConfigArgs,
                                              BASE_CLIENT_VXML_URL);

      if (urlStr == NULL)
      {
         fprintf(stderr, "ERROR: No VXML document specified\n");
         return 1;
      }
      else if (VXIValueGetType((const VXIValue *)urlStr) != VALUE_STRING)
      {
         fprintf(stderr, "ERROR: %S must be set to a VXIString value\n",
                 BASE_CLIENT_VXML_URL);
         return 1;
      }

      gblDefaultVxmlURL = (VXIchar *)VXIStringCStr(urlStr);
   }

   printf("Using VXML document base '%S'\n\n", gblDefaultVxmlURL);

   /* Initialize the platform */
   platformResult = VXIplatformInit(glbConfigArgs, &maxChannels);
   CLIENT_CHECK_RESULT("VXIplatformInit()", platformResult);

   if(maxChannels < nbChannels)
   {
      printf("WARNING: %d channels requested on the command line but only " 
             "%d available\n\n", nbChannels, maxChannels);
      nbChannels = maxChannels;
   }
   else if (nbChannels == 0)
   {
      nbChannels = maxChannels;
   }

   char *pathName = (char*) EMPTY_STRING;
   const VXIString *logStr =
      (const VXIString *)VXIMapGetProperty(glbConfigArgs, LOG_DIR);
   if(logStr != NULL) 
   {
      int len = VXIStringLength(logStr) + 1;
      pathName = new char [len];
      wcstombs(pathName, VXIStringCStr(logStr), len);
      pathName[len - 1] = 0;
   }

   OsPath workingDirectory;  
   if ( OsFileSystem::exists(pathName) )  
   {    
      workingDirectory = pathName;    
      OsPath path(workingDirectory);    
      path.getNativePath(workingDirectory);  
   }   
   else  
   {    
      OsPath path;    
      OsFileSystem::getWorkingDirectory(path);    
      path.getNativePath(workingDirectory);  
   }  
   UtlString fileName =  workingDirectory +     
      OsPathBase::separator +    
      MEDIASERVER_LOG;  
   osPrintf("Open SIP log file %s\n\n", fileName.data());  

   logStr = (const VXIString *)VXIMapGetProperty(glbConfigArgs, SYS_LOG_LEVEL);
   char *loglevel = (char*)ERR_STR;
   if(logStr != NULL) 
   {
      int len = VXIStringLength(logStr);
      loglevel = new char [len + 1];
      wcstombs(loglevel, VXIStringCStr(logStr), len);
      loglevel[len] = 0;
   }

   OsSysLog::initSysLog(FAC_MEDIASERVER_VXI, "mediaserver", fileName.data(), loglevel);
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, ">>>>>>>>>>>>>>>> Starting - version %s build %s",
                 SIPX_VERSION, SIPX_BUILD
                 );

   if(pathName != EMPTY_STRING) 
   {
      delete[] pathName;
      pathName = NULL;
   }
   if (loglevel != ERR_STR)  
   {
      delete[] loglevel;
      loglevel = NULL;
   }

   IvrUtilTask *pIvrUtilTask = 0;
   platformResult = VXIsetLogFlushPeriod(glbConfigArgs, (void**)&pIvrUtilTask);
   CLIENT_CHECK_RESULT("VXIsetLogFlushPeriod()", platformResult);

   VXItrdResult eTrdResult = VXItrdMutexCreate( &glbpChannelMutex );
   if ( eTrdResult != VXItrd_RESULT_SUCCESS )
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, "main: failed to create glbpChannelMutex  (%d) \n", (int)eTrdResult );
   }

   eTrdResult = VXItrdMutexCreate( &glbpThreadMutex );
   if ( eTrdResult != VXItrd_RESULT_SUCCESS )
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, "main: failed to create glbpThreadMutex  (%d) \n", (int)eTrdResult );
   }

   // Read the configuration of the identity to be used if/when challenged for authentication.
   SipLine*    line = NULL;
   SipLineMgr* lineMgr = NULL;

   UtlString   domain;
   UtlString   realm;
   UtlString   user;
   
   OsConfigDb  domainConfiguration;
   OsPath      domainConfigPath = SipXecsService::domainConfigPath();
   
   if (OS_SUCCESS == domainConfiguration.loadFromFile(domainConfigPath.data()))
   {
      domainConfiguration.get(SipXecsService::DomainDbKey::SIP_DOMAIN_NAME, domain);
      domainConfiguration.get(SipXecsService::DomainDbKey::SIP_REALM, realm);
      
      if (!domain.isNull() && !realm.isNull())
      {
         CredentialDB* credentialDb;
         if ((credentialDb = CredentialDB::getInstance()))
         {
            Url identity;

            identity.setUserId(MEDIASERVER_ID_TOKEN);
            identity.setHostAddress(domain);
            UtlString user;
            UtlString ha1_authenticator;
            UtlString authtype;
         
            if (credentialDb->getCredential(identity, realm, user, ha1_authenticator, authtype))
            {
               if ((line = new SipLine( identity // user entered url
                                       ,identity // identity url
                                       ,user     // user
                                       ,TRUE     // visible
                                       ,SipLine::LINE_STATE_PROVISIONED
                                       ,TRUE     // auto enable
                                       ,FALSE    // use call handling
                                       )))
               {
                  if ((lineMgr = new SipLineMgr()))
                  {
                     if (lineMgr->addLine(*line))
                     {
                        if (lineMgr->addCredentialForLine( identity, realm, user, ha1_authenticator
                                                          ,HTTP_DIGEST_AUTHENTICATION
                                                          )
                            )
                        {
                           OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO,
                                         "Added identity '%s': user='%s' realm='%s'"
                                         ,identity.toString().data(), user.data(), realm.data()
                                         );
                        }
                        else
                        {
                           OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                                         "Error adding identity '%s': user='%s' realm='%s'\n"
                                         "  transfer functions may not work.",
                                         identity.toString().data(), user.data(), realm.data()
                                         );
                        }

                        lineMgr->setDefaultOutboundLine(identity);
                     }
                     else
                     {
                        OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                                      "addLine failed: "
                                      "  transfer functions may not work."
                                      );
                     }
                  }
                  else
                  {
                     OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                                   "Constructing SipLineMgr failed:  "
                                   "  transfer functions may not work."
                                   );
                  }
               }
               else
               {
                  OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                                "Constructing SipLine failed:  "
                                "  transfer functions may not work."
                                );
               }
            }
            else
            {
               OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                             "No credential found for '%s@%s' in realm '%s'"
                             "; transfer functions will not work",
                             MEDIASERVER_ID_TOKEN, domain.data(), realm.data()
                             );
            }

            credentialDb->releaseInstance();
         }
         else
         {
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                          "Failed to open credentials database; transfer functions will not work"
                          );
         }
      }
      else
      {
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                       "Domain or Realm not configured:"
                       "\n  '%s' : '%s'\n  '%s' : '%s'"
                       "  transfer functions will not work.",
                       SipXecsService::DomainDbKey::SIP_DOMAIN_NAME, domain.data(),
                       SipXecsService::DomainDbKey::SIP_REALM, realm.data()
                       );
      }
   }
   else
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                    "main: failed to load domain configuration from '%s'",
                    domainConfigPath.data()
                    );
   }



   int port = 5100;
   SipUserAgent *userAgent =
      new SipUserAgent(port, 
                       port,
                       PORT_NONE,
                       NULL, // public IP address (nopt used in proxy)
                       user.isNull() ? NULL : user.data(), // default user
                       NULL, // default SIP address (not used in proxy)
                       domain.isNull() ? NULL : domain.data(), // outbound proxy
                       NULL, // directory server
                       NULL, // registry server
                       NULL, // auth scheme
                       NULL, //auth realm
                       NULL, // auth DB
                       NULL, // auth user IDs
                       NULL, // auth passwords
                       NULL, // nat ping URL
                       0, // nat ping frequency
                       "PING", // nat ping method
                       lineMgr, // line mgr
                       SIP_DEFAULT_RTT, // first resend timeout
                       TRUE, // default to UA transaction
                       SIPUA_DEFAULT_SERVER_UDP_BUFFER_SIZE, // socket layer read buffer size
                       SIPUA_DEFAULT_SERVER_OSMSG_QUEUE_SIZE // OsServerTask message queue size
         );

   userAgent->allowExtension(SIP_CALL_CONTROL_EXTENSION);
   userAgent->allowExtension("sip-cc-01");
   userAgent->allowExtension(SIP_REPLACES_EXTENSION);
   userAgent->setUserAgentHeaderProperty("sipX/vxml");

   userAgent->start();

   UtlString localAddress;
   OsSocket::getHostIp(&localAddress);

   // Read the list of codecs from the configuration file.
   SdpCodecFactory codecFactory;
   initCodecs(&codecFactory);

   int maxFlowGraphs = DEFAULT_MAX_ACTIVE_CALLS;
   const VXIInteger* value =
      (const VXIInteger*) VXIMapGetProperty(glbConfigArgs, MAX_ACTIVE_CALLS);
   if (value != NULL) 
   {
      maxFlowGraphs = VXIIntegerValue(value);
   }
   
   OsConfigDb configDb;
   configDb.set("PHONESET_MAX_ACTIVE_CALLS_ALLOWED", maxFlowGraphs);

   CallManager* pCallMgr;
   pCallMgr =
      new CallManager(FALSE,                             // isRequiredUserIdMatch
                      NULL,                              // lineMgrTask
                      TRUE,                              // early media in 180 ringing
                      &codecFactory,                     // SdpCodecFactory
                      9000,                              // rtp port start
                      9999,                              // rtp port end
                      localAddress.data(),               // localAddress
                      localAddress.data(),               // publicAddress
                      userAgent,                         // SipUserAgent
                      0,                                 // sipSessionReinviteTimer
                      NULL,                              // mgcpStackTask
                      "ivr@sip.pingtel.com",             // defaultCallExtension
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
                      maxFlowGraphs,                     // maxCalls
                      sipXmediaFactoryFactory(&configDb) // CpMediaInterfaceFactory
         );
         
   pCallMgr->setDelayInDeleteCall(CALL_DELETE_DELAY_SECS);

   // Create the Media Task monitoring thread if autothrottle.engage is set.

   pthread_t autoThrottleThread;
   UtlBoolean autoThrottleThreadStarted = FALSE;

   // This value has to stay allocated until autoThrottleThread is halted.
   MpMediaTask* pMedia = NULL;
   void *(args[2]) = { (void*) pMedia, (void*) pCallMgr };

   const VXIInteger* value2 =
      (const VXIInteger*) VXIMapGetProperty(glbConfigArgs,
                                            L"autothrottle.engage");
   if (value2 != NULL && VXIIntegerValue(value2) != 0)
   {
      pthread_create(&autoThrottleThread, NULL, &autoThrottleFunction,
                     (void*) args);
      autoThrottleThreadStarted = TRUE;
   }

   IvrCallListener *listener = new IvrCallListener(pCallMgr/*, platform*/);

   pCallMgr->addTaoListener(listener);

   glbNbCalls = 0;
   gpTelListener = IvrTelListener::getTelListener(pCallMgr);
   listener->start();
   listener->addListener(gpTelListener);

   pCallMgr->start();

#ifdef __STL_THREADS
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, " main __STL_THREADS is defined ");
#else
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, " main __STL_THREADS is NOT defined ");
#endif

#ifdef __STL_PTHREADS
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, " main __STL_PTHREADS is defined ");
#else
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, " main __STL_PTHREADS is NOT defined ");
#endif

#ifdef _REENTRANT
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, " main _REENTRANT is defined ");
#else
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, " main _REENTRANT is NOT defined ");
#endif

#ifdef _PTHREADS
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, " main _PTHREADS is defined ");
#else    
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, " main _PTHREADS is NOT defined ");
#endif

#ifdef __GNUC__
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, " main __GNUC__ is defined ");
#else    
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, " main __GNUC__ is NOT defined ");
#endif


#ifdef STACK_DEBUG
   // construct thread attribute
   pthread_attr_t thread_attr;
   int prc = pthread_attr_init(&thread_attr);
   if (prc != 0)
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, "main: pthread_attr_init failed (%d) ", prc );
   }

   size_t stacksize = 0;
   void* stackaddr = 0;
   if (0 == pthread_attr_getstack(&thread_attr, &stackaddr, &stacksize))
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, "main: thread stack size (%d) addr (0x%08x)", stacksize, stackaddr );
   }
#endif

   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "mediaserver main thread started.");

   while(listener->isStarted() && !gShutdownFlag)
   {
      OsTask::delay(120000);    // sleep for 2 minutes

      LockThreadStack();
      UtlSListIterator iterator(glbCleanupThreadStack);
      UtlInt* threadCollectable = (UtlInt*) iterator();
      int numThreads = getStackSize(glbCleanupThreadStack);
      int loops = 0;
      while(threadCollectable && (loops < numThreads))
      {
         CallCleanupThreadArgs* thread = (CallCleanupThreadArgs*) threadCollectable->getValue();
         if(thread)
         {
            const char* callId = thread->callId;
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "main loop: freeing cleanup threads for %s (%d remaining) \n", callId, numThreads-loops);
            if (NULL == findHandlingChannel(callId))
            {
               thread = removeThread(thread);
               iterator.reset();
               if (thread) 
               {
                  freeThread(thread, FALSE);
               }
            }
         }
         threadCollectable = (UtlInt*) iterator();
         loops++;
      } /* while (loops) ] */

      OsSysLog::flush();

      UnlockThreadStack();
   } /* while (!gShutdownFlag) ] */

   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "main loop: exiting clientMain gShutdownFlag=%d, listener started=%d \n", 
                 gShutdownFlag, listener->isStarted());
   OsSysLog::flush();

   while(getStackSize(glbChannelStack))
   {
      ChannelThreadArgs* channel = popChannel();
      if (channel) freeChannel(channel);
   }

   while(getStackSize(glbCleanupThreadStack))
   {
      CallCleanupThreadArgs* thread = popThread();
      if (thread) freeThread(thread, TRUE);
   }

   if (glbpChannelMutex)
   {
      VXItrdMutexUnlock(glbpChannelMutex);
      VXItrdMutexDestroy(&glbpChannelMutex);
      glbpChannelMutex = 0;
   }

   if (glbpThreadMutex)
   {
      VXItrdMutexUnlock(glbpThreadMutex);
      VXItrdMutexDestroy(&glbpThreadMutex);
      glbpThreadMutex = 0;
   }

   if ( glbConfigArgs )
      VXIMapDestroy(&glbConfigArgs);
   if ( configFile )
   {
      free(configFile);
      configFile = NULL;
   }

   if (listener)
   {
      delete listener;
      listener = NULL;
   }

   if (autoThrottleThreadStarted)
   {
      pthread_cancel(autoThrottleThread);
   }

   if (pMedia)
   {
      delete pMedia;
      pMedia = NULL;
   }
/*
Don't delete userAgent, it appears that the CallManger destructor does that.
If we delete it here, we get crashes when we CallManager tries to as well.

   if (userAgent)
   {
      userAgent->shutdown(FALSE);
      while (!userAgent->isShutdownDone());
      delete userAgent;
      userAgent = NULL;
   }
*/
   if (pCallMgr)
   {
      delete pCallMgr;
      pCallMgr = NULL;
   }
   if (pIvrUtilTask)
   {
      delete pIvrUtilTask;
      pIvrUtilTask = 0;
   }

   if (gpTelListener)
   {
      delete gpTelListener;
      gpTelListener = 0;
   }

   /* Shut down the platform */
   platformResult = VXIplatformShutdown();
   CLIENT_CHECK_RESULT("VXIplatformShutdown()", platformResult);


   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "mediaserver main thread exited.");

   OsSysLog::flush();

   exit(gExitStatus);
}

   static void 
ShowResult(const VXIValue *result_val, int ChanNum, VXIplatform *platform)
{
   switch (VXIValueGetType(result_val)) {
   case VALUE_INTEGER:
      SBclientDiag(platform, 60001, L"clientMain::ShowResult",
                   L"VXIInteger\tResult Value: %d\n",
                   VXIIntegerValue((const VXIInteger *) result_val));
      break;

   case VALUE_FLOAT:
      SBclientDiag(platform, 60001, L"clientMain::ShowResult",
                   L"VXIFloat\tResult Value: %f\n", 
                   VXIFloatValue((const VXIFloat *) result_val));
      break;

   case VALUE_STRING:
      SBclientDiag(platform, 60001, L"clientMain::ShowResult",
                   L"VXIString\tResult Value: %s\n", 
                   VXIStringCStr((const VXIString *) result_val));
      break;

   case VALUE_PTR:
      SBclientDiag(platform, 60001, L"clientMain::ShowResult",
                   L"Result Type: VXIPtr: 0x%p\n",
                   VXIPtrValue((const VXIPtr *) result_val));
      break;

   case VALUE_MAP:
      SBclientDiag(platform, 60001, L"clientMain::ShowResult",
                   L"Result Type: VXIMap");
      break;

   case VALUE_VECTOR:
      SBclientDiag(platform, 60001, L"clientMain::ShowResult",
                   L"Result Type: VXIVector");
      break;

   case VALUE_CONTENT:
      SBclientDiag(platform, 60001, L"clientMain::ShowResult",
                   L"Result Type: VXIContent");
      break;

   default:
      SBclientDiag(platform, 60001, L"clientMain::ShowResult",
                   L"Result Type: Unknown type");
      break;
   }
}


static VXITRD_DEFINE_THREAD_FUNC(ChannelThread, userData)
{
   const ChannelThreadArgs *channelArgs = (const ChannelThreadArgs *)userData;
   VXIplatformResult platformResult;
   VXIplatform *platform;
   VXIMap *channelConfig;
   VXIMap *sessionArgs;

   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                 "Begin ChannelThread to process channel %d\nCallID:%s\nplatform=%p",
                 channelArgs->channelNum, channelArgs->callId, channelArgs->platform);

   // Delay for one second to reduce the chances that the other end 
   // will not hear the first part of the prompts.
   OsTask::delay(1000);

#if defined(__pingtel_on_posix__)
   // If we ever receive a thread cancel request, it means that the OsTask
   // object is in the process of being destroyed.  To avoid the situation
   // where a thread attempts to run after its containing OsTask object has
   // been freed, we set the thread up so that the cancel takes effect
   // immediately (as opposed to waiting until the next thread cancellation
   // point).
   pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

#ifdef STACK_DEBUG /* [ */
   // construct thread attribute
   pthread_attr_t thread_attr;
   int prc = pthread_attr_init(&thread_attr);
   if (prc != 0) {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, "channel: pthread_attr_init failed (%d) ", prc );
   }

   size_t stacksize = 0;
   void* stackaddr = 0;
   if (0 == pthread_attr_getstack(&thread_attr, &stackaddr, &stacksize))
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, "channel: thread stack size (%d) addr (0x%08x)", stacksize, stackaddr );
#endif /* STACK_DEBUG ] */

#endif

   if ( channelArgs->configArgs )
      channelConfig = VXIMapClone (channelArgs->configArgs);
   else
      channelConfig = NULL;

   if ( channelArgs->sessionArgs )
      sessionArgs = VXIMapClone (channelArgs->sessionArgs);
   else
      sessionArgs = NULL;

   platform = channelArgs->platform;
   platformResult = VXIplatformCreateResources(channelArgs->channelNum, channelConfig,
                                               &platform, channelArgs->pCallMgr);
   CHANNEL_CHECK_RESULT("VXIplatformCreateResources()", platformResult);

   platformResult = VXIplatformAddResource(1, channelArgs->pCallMgr,
                                           gpTelListener, &platform);
   CHANNEL_CHECK_RESULT("VXIplatformAddResources()", platformResult);

   platformResult = VXIplatformEnableCall(platform);
   CHANNEL_CHECK_RESULT("VXIplatformEnableCall()", platformResult);

   SBclientDiag(platform, 60001, L"mainClient::ChannelThread",
                L"About to call VXIplatformWaitForCall");

   /* Clean up from prior calls if necessary */
   if (platform->telephonyProps) {
      VXIMapDestroy(&platform->telephonyProps);
      platform->telephonyProps = NULL;
   }
   platform->telephonyProps = VXIMapClone (channelArgs->sessionArgs);
   platformResult = VXIplatformWaitForCall(platform);
   CHANNEL_CHECK_RESULT("VXIplatformWaitForCall()", platformResult);

   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "Channel %d: In a Call\n", channelArgs->channelNum);
   SBclientDiag(platform, 60001, L"mainClient::ChannelThread",L"In a Call");

   // Add listener to IvrTelListener for this call
   gpTelListener->addListener(channelArgs->callId);

   VXIValue *result = NULL ;
   platformResult = VXIplatformProcessDocument(channelArgs->vxmlURL,
                                               sessionArgs, &result, platform);

   if((VXIint) platformResult != 0){
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, "Channel %d: ProcessDocument returned error code %d",
              channelArgs->channelNum, (int)platformResult);
   }
   else{
      if(result) {
         ShowResult(result, channelArgs->channelNum, platform);
         VXIValueDestroy(&result);
      }else{
         SBclientDiag(platform, 60001, L"mainClient::ChannelThread",
                      L"NULL result");
      }
   }

   // Remove listener from IvrTelListener for this call
   // We'll wait, do not timeout
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO,
                 "ChannelThread: calling removeListener for channel %d\nCallID:%s\nplatform=%p",
                 channelArgs->channelNum, channelArgs->callId, channelArgs->platform);
   while (OS_BUSY == gpTelListener->removeListener(channelArgs->callId))
   {
      OsTask::delay(10000);
   }


   // We'll wait for the cleanup to finish, do not timeout
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, 
                 "ChannelThread: check isCleanupInProgress for channel %d\nCallID:%s\nplatform=%p",
                 channelArgs->channelNum, channelArgs->callId, channelArgs->platform);
   while (gpTelListener->isCleanupInProgress(channelArgs->callId))
   {
      OsTask::delay(10000);
   }

   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, 
                 "ChannelThread: isCleanupInProgress is false, do DestroyResources now, for channel %d\nCallID:%s\nplatform=%p",
                 channelArgs->channelNum, channelArgs->callId, channelArgs->platform);
 
   OsSysLog::flush();

   platformResult = VXIplatformDestroyResources(&platform);
   CHANNEL_CHECK_RESULT("VXIplatformDestroyResources()", platformResult);

   /* Clean up from prior calls if necessary */
   if (platform->telephonyProps) {
      VXIMapDestroy(&platform->telephonyProps);
      platform->telephonyProps = NULL;
   }


   if (platform)
   {
      free(platform);
      platform = NULL;
   }

   if (channelConfig)
      VXIMapDestroy(&channelConfig);

   if (sessionArgs)
      VXIMapDestroy(&sessionArgs);

   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, 
                 "End ChannelThread for channel %d\nCallID:%s\nplatform=%p",
                 channelArgs->channelNum, channelArgs->callId, channelArgs->platform);

   VXItrdThreadExit(0);
   return 0;
}


int VXIProcessUrl (void *plistener,
      int channelNum,
      const char *callId,
      const char *from,
      const char *to,
      const char *url)
{
   VXIplatformResult platformResult = VXIplatform_RESULT_SUCCESS;
   VXIchar *vxmlURL = NULL;
   int maxCalls = 1;
   VXItrdResult trdResult;

   if (plistener == NULL)
      return 0;

   IvrCallListener *listener = (IvrCallListener *)plistener;
   CallManager *pCallMgr = listener->getCallManager();

   /* Determine the URL if not specified on the command line */
   VXIunsigned urlLen = strlen(url) + 1;
   if(url && ( strstr (url, "http") != 0) ) // @JC fixed by removing the :
   {
      vxmlURL = (VXIchar *) calloc(urlLen, VXICHAR_SIZE);
      CLIENT_CHECK_MEMALLOC( vxmlURL, "URL buffer" );
      mbstowcs(vxmlURL, url, urlLen);
   }
   else
   {
      int gurlLen = wcslen(gblDefaultVxmlURL);
      int llen = gurlLen + urlLen;

      char* tmp = new char[llen];
      vxmlURL = (VXIchar *) calloc(llen, VXICHAR_SIZE);
      CLIENT_CHECK_MEMALLOC( vxmlURL, "URL buffer" );
      sprintf(tmp, "%S%s", gblDefaultVxmlURL, url);
      mbstowcs(vxmlURL, (char*)tmp, llen);
      delete[] tmp;
      tmp = NULL;
   }

   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                 "VXIProcessUrl Using VXML document '%S'\n\n", vxmlURL);

   VXIMap *sessionArgs = VXIMapCreate();

   int len = (strlen(callId) + VXICHAR_SIZE - 1)/VXICHAR_SIZE; 
   VXIString *str = VXIStringCreateN((VXIchar*)callId, len);
   CLIENT_CHECK_MEMALLOC(str, "Call Id");
   VXIMapSetProperty(sessionArgs, L"callid", (VXIValue *)str);

   len = strlen(to) + 1;
   VXIchar *dnis = (VXIchar *) calloc(len, VXICHAR_SIZE);
   CLIENT_CHECK_MEMALLOC( dnis, "URL buffer" );
   mbstowcs(dnis, to, len);

   VXIString *str2 = VXIStringCreateN((VXIchar*)dnis, len);
   VXIMapSetProperty(sessionArgs, L"dnis", (VXIValue *)str2);

   len = strlen(from) + 1;
   VXIchar *ani = (VXIchar *) calloc(len, VXICHAR_SIZE);
   CLIENT_CHECK_MEMALLOC( dnis, "URL buffer" );
   mbstowcs(ani, from, len);
   VXIString *str3 = VXIStringCreateN((VXIchar*)ani, len);
   VXIMapSetProperty(sessionArgs, L"ani", (VXIValue *)str3);

   /* Start the call processing threads */
   ChannelThreadArgs *channel = (ChannelThreadArgs *) calloc(1, sizeof(ChannelThreadArgs));
   CLIENT_CHECK_MEMALLOC( channel, "channel thread args array" );

   char* id = 0;
   if (callId)
   {
      int len = strlen(callId);
      id = new char[len + 1];
      memcpy(id, callId, len);
      id[len] = 0;
   }

   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "Channel %ld: Creating VXI thread to handle call %s\n", glbNbCalls, callId);
   VXIplatform *platform = (VXIplatform *) malloc(sizeof(VXIplatform));
   CLIENT_CHECK_MEMALLOC(platform, "VXIplatform");
   memset(platform, 0, sizeof(VXIplatform));

   channel->platform = platform;
   channel->channelNum = glbNbCalls++;
   channel->maxCalls = maxCalls;
   channel->configArgs = glbConfigArgs;
   channel->sessionArgs = VXIMapClone(sessionArgs);
   channel->vxmlURL = vxmlURL;
   channel->pCallMgr = pCallMgr;
   channel->callId = id;

   SBclientDiag(platform, 60001, L"mainClient::ChannelThread",
                L"thread created to handle call");


   pushChannel(channel);


   trdResult = VXItrdThreadCreate(&channel->threadHandle,
                                  ChannelThread,
                                  (VXItrdThreadArg) channel);
   CLIENT_CHECK_RESULT("VXItrdThreadCreate()", trdResult);

   /* Clean up */

   VXIMapDeleteProperty(sessionArgs, L"callid");
   VXIMapDeleteProperty(sessionArgs, L"dnis");
   VXIMapDeleteProperty(sessionArgs, L"ani");

   VXIMapDestroy(&sessionArgs);

   if ( ani )
   {
      free((VXIchar *) ani);
      ani = NULL;
   }
   if ( dnis )
   {
      free((VXIchar *) dnis);
      dnis = NULL;
   }

   return (VXIint) platformResult;
}


static VXITRD_DEFINE_THREAD_FUNC(CallCleanupThread, userData)
{
   CallCleanupThreadArgs *threadArg = (CallCleanupThreadArgs *) userData;
   if (threadArg)
   {
      char *callId = threadArg->callId;
      ChannelThreadArgs* channel = threadArg->channel;
      int VXISessionEnded = threadArg->VXISessionEnded;

#if defined(__pingtel_on_posix__)
      // If we ever receive a thread cancel request, it means that the thread
      // is in the process of being destroyed.  To avoid the situation
      // where a thread attempts to run after its containing thread has
      // been freed, we set the thread up so that the cancel takes effect
      // immediately (as opposed to waiting until the next thread cancellation
      // point).
      pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
#endif

      VXIplatformResult platformResult = VXIplatform_RESULT_SUCCESS;
      VXItrdResult trdResult = (VXItrdResult) 0;
      VXItrdThreadArg status;

      // to see if this channel is still alive
      ChannelThreadArgs* currentChannel = findHandlingChannel(callId);

      /* Wait for threads to finish */
      if(channel && currentChannel)
      {

         if (VXISessionEnded == FALSE)
         {
            SBclientExitingCall(channel->platform);
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, 
                          "CallCleanupThread %s: Channel %d: after Exiting, before removeFromCleanUpInProgress\n", 
                          callId, channel->channelNum);
            gpTelListener->removeFromCleanupInProgress(callId);
         }

         int tries = 0;
         while ( tries++ < 3 && (0 != (trdResult = VXItrdThreadJoin (channel->threadHandle, &status, 15000))))
         {             
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, 
                          "CallCleanupThread-%s: Channel %d: VXItrdThreadJoin had error %d tries=%d VXISessionEnded=%d\n", 
                          callId, channel->channelNum, trdResult, tries, VXISessionEnded);
         }

         int okToFreeChannel = 1;
         if (trdResult != 0)
         {
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, "CallCleanupThread-%s: Channel %d: didn't exit, do not force thread to exit \n", callId, channel->channelNum);
            // VXItrdThreadCancel (channel->threadHandle);
            okToFreeChannel = 0;
         }
         else
         {
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, "CallCleanupThread-%s: Channel %d: VXItrdThreadJoin exited normally %d \n", callId, channel->channelNum, trdResult);
         }

         trdResult = VXItrdThreadDestroyHandle (&(channel->threadHandle));
         if (trdResult)
         {
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, "CallCleanupThread-%s: Channel %d: VXItrdThreadDestroyHandle had error %d \n", callId, channel->channelNum, trdResult);
         }

         channel = removeChannel(channel); // remove from stack
         if (channel && okToFreeChannel) freeChannel(channel);

         if ( status != 0 ) 
         {
            /* Need to cast this twice, some compilers won't allow going directly
               to VXIplatformResult */
            platformResult = (VXIplatformResult) ((VXIint) status);
         }

      }
   }

   OsSysLog::flush();
   VXItrdThreadExit(0);
   return 0;
}


int VXICleanUpCall (void *plistener,
      const char *callId,
      int VXISessionEnded)
{
   VXIplatformResult platformResult = VXIplatform_RESULT_SUCCESS;

   char* newCallId = 0;
   if (callId)
   {

      ChannelThreadArgs* channel = findHandlingChannel(callId);
      if (NULL == channel)
      {
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "VXICleanUpCall:  VXI session with %s does not exist.\n", callId);
         return platformResult;    // session/channel does not exist
      }

      CallCleanupThreadArgs* thread = findCleanupThread(callId);
      if (thread)
      {
         // This is generally bad.  The call should not exist.
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, "ERROR: VXICleanUpCall: cleanup thread for %s already exists.\n", callId);
         return VXIplatform_RESULT_FAILURE;
      }

      thread = new struct CallCleanupThreadArgs;

      int len = strlen(callId);
      newCallId = new char[len + 1];
      if (!thread || !newCallId)
         return VXIplatform_RESULT_OUT_OF_MEMORY;

      memcpy(newCallId, callId, len);
      newCallId[len] = 0;

      thread->callId = newCallId;
      thread->channel = channel;
      thread->VXISessionEnded = VXISessionEnded;

      VXItrdThread  *threadHandle;
      VXItrdResult trdResult = VXItrdThreadCreate(&threadHandle,
                                                  CallCleanupThread,
                                                  (VXItrdThreadArg) thread);
      CLIENT_CHECK_RESULT("VXItrdThreadCreate()", trdResult);

      if (trdResult)
         platformResult = VXIplatform_RESULT_FAILURE;

      thread->threadHandle = threadHandle;
      pushThread(thread);
   }

   return (VXIint) platformResult;
}


void initCodecs(SdpCodecFactory* codecFactory)
{
   UtlString codecList;
   char* codecListParameter = NULL;
   UtlString oneCodec;
   int codecStringIndex = 0;
   SdpCodec::SdpCodecTypes internalCodecId;

   const VXIString *cfgCodecStr = 
      (const VXIString *)VXIMapGetProperty(glbConfigArgs, MEDIASERVER_RTP_CODECS);

   if (cfgCodecStr == NULL)
   {
      codecList = DEFAULT_CODEC_LIST_STRING;
   }
   else
   {
      int len = VXIStringLength(cfgCodecStr);
      codecListParameter = new char [len + 1];
      wcstombs(codecListParameter, VXIStringCStr(cfgCodecStr), len);
      codecListParameter[len] = 0;
      codecList = codecListParameter;

      if (codecList.isNull())
         codecList = DEFAULT_CODEC_LIST_STRING;
   }

   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, "initCodecs: Using the following codecs: %s",
                 codecList.data());

   NameValueTokenizer::getSubField(codecList, codecStringIndex,
                                   ", \n\r\t", &oneCodec);

   while(!oneCodec.isNull())
   {
      internalCodecId = SdpCodecFactory::getCodecType(oneCodec.data());
      if (internalCodecId == SdpCodec::SDP_CODEC_UNKNOWN)
      {
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, "initCodecs: Unknown codec ID: %s",
                       oneCodec.data());
      }
      else
      {
         codecFactory->buildSdpCodecFactory(1, &internalCodecId);
      }

      codecStringIndex++;
      NameValueTokenizer::getSubField(codecList, codecStringIndex,
                                      ", \n\r\t", &oneCodec);
   }

   if (codecListParameter != NULL)
      delete[] codecListParameter;
}


#ifdef SOCKET_DEBUG /* [ */

#include <execinfo.h>

void getstackframe(UtlString& backTraceString)
{
   // For the stacks to contain function names you must
   // compile with -g and link with -rdynamic:w


   void *trace[16];
   char **messages = (char **)NULL;
   int i, trace_size = 0;

   trace_size = backtrace(trace, 16);
   messages = backtrace_symbols(trace, trace_size);

   backTraceString = "[bt] Execution path:\n";

   for (i=0; i<trace_size; ++i)
   {
      backTraceString.append("[bt] ");
      backTraceString.append(messages[i]);
      backTraceString.append("\n");
   }

   free(messages);
   messages = NULL;
}


extern "C" {
   int __real_socket(int domain, int type, int protocol);
   int __real_close(int fd);
   int __real_fclose(FILE* fp);
   int __real_open(const char *pathname, int flags);
   FILE* __real_fopen (const char *path, const char *mode);
   int __wrap_socket(int domain, int type, int protocol);
   int __wrap_close(int fd);
   int __wrap_fclose(FILE* fp);
   int __wrap_open(const char *pathname, int flags);
   FILE* __wrap_fopen (const char *path, const char *mode);
}


// Our wrapped version of socket
int __wrap_socket(int domain, int type, int protocol)
{
   // call the real system implementation.
   int sockDescriptor = __real_socket(domain, type, protocol);

   OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                 "socket created with fd: %d", sockDescriptor);

   return(sockDescriptor);
}

// Our wrapped version of open
int __wrap_open(const char *pathname, int flags)
{
   // call the real system implemention of close
   int fd = __real_open(pathname, flags);

   UtlString btString;
   getstackframe(btString);

   OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                 "::open pathname: %s flag: %d returning fd: %d", pathname, flags, fd);

   return(fd);
}

// Our wrapped version of close
int __wrap_close(int fd)
{
   // call the real system implemention of close
   int retCode = __real_close(fd);

   UtlString btString;
   getstackframe(btString);

   OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                 "::close  with fd: %d returned: %d from %s", fd, retCode, btString.data());

   return(retCode);
}

// Our wrapped version of fopen
FILE* __wrap_fopen (const char *path, const char *mode)
{
   // call the real system implemention of close
   FILE* fp = __real_fopen(path, mode);

   int fd = -1;
   if (fp) fd = fileno(fp);

   UtlString btString;
   getstackframe(btString);

   OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                 "::fopen path: %s mode: %s returning fp: %p, fd: %d", path, mode, fp, fd);

   return(fp);
}

// Our wrapped version of fclose
int __wrap_fclose(FILE* fp)
{
   int fd = -1;
   if (fp) fd = fileno(fp);

   // call the real system implemention of close
   int retCode = __real_fclose(fp);

   UtlString btString;
   getstackframe(btString);

   OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                 "::fclose  with fp: %p, fd: %d returned: %d from %s", fp, fd, retCode, btString.data());

   return(retCode);
}
#endif /* SOCKET_DEBUG ] */


#ifdef DEBUG /* [ */
#include <assert.h>

   extern "C" void
__cxa_pure_virtual (int pThis, int p2)
{
   int i[2];
   i[0] = (int)&i;
   i[1] = 0x12345678;
   printf("\n\n\n\nPINGTEL virtual method called pThis = 0x%08X; p2 = 0x%08X:\n", pThis, p2);
   printf("\n\n\n\nPINGTEL virtual method called caller = 0x%08X; memory info:\n", i[3]);
   for(int j = -4; j < 8; j++)
      printf("%08X  ", i[j]);
   printf("\n\n\n\n\n");
   assert(FALSE);
}
#endif /* DEBUG ] */

void JNI_LightButton(long)
{

}

// The auto-throttle thread.

void* autoThrottleFunction(void* arg)
{
   // Get the configuration parameters.
   const VXIInteger* value;

   // Milliseconds to sleep between auto-throttle executions.
   int autothrottle_tick = 1000;
   value = (const VXIInteger*) VXIMapGetProperty(glbConfigArgs,
                                                 L"autothrottle.tick");
   if (value != NULL && VXIIntegerValue(value) > 0)
   {
      autothrottle_tick = VXIIntegerValue(value);
   }

   // Message queue limit to activate auto-throttle.
   int autothrottle_limit = 10;
   value = (const VXIInteger*) VXIMapGetProperty(glbConfigArgs,
                                                 L"autothrottle.limit");
   if (value != NULL &&
       (VXIIntegerValue(value) > 0 || VXIIntegerValue(value) == -1))
   {
      autothrottle_limit = VXIIntegerValue(value);
   }

   // Number of consecutive times the message queue must be over-limit
   // to activate the auto-throttle.
   int autothrottle_trigger = 5;
   value = (const VXIInteger*) VXIMapGetProperty(glbConfigArgs,
                                                 L"autothrottle.trigger");
   if (value != NULL && VXIIntegerValue(value))
   {
      autothrottle_trigger = VXIIntegerValue(value);
   }

   // Turn on verbose logging.
   int autothrottle_verbose = 0;
   value = (const VXIInteger*) VXIMapGetProperty(glbConfigArgs,
                                                 L"autothrottle.trigger");
   if (value != NULL && VXIIntegerValue(value) != 0)
   {
      autothrottle_verbose = 1;
   }

   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_NOTICE,
                 "autoThrottleFunction "
                 "autothrottle_tick = %d, autothrottle_limit = %d, "
                 "autothrottle_trigger = %d, autothrottle_verbose = %d",
                 autothrottle_tick, autothrottle_limit,
                 autothrottle_trigger, autothrottle_verbose);

   static int no_ticks_over_limit = 0;
   MpMediaTask* pMedia = (MpMediaTask*) ((void**) arg)[0];
   CallManager* pCallMgr = (CallManager*) ((void**) arg)[1];
   int messages, soft_limit, hard_limit;
   int currentCalls, maxCalls;

   for (;;)
   {
      pMedia->getQueueUsage(messages, soft_limit, hard_limit);
      pCallMgr->getCalls(currentCalls, maxCalls);

      if (autothrottle_verbose)
      {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_NOTICE,
                    "autoThrottleFunction "
                    "currentCalls = %d, maxCalls = %d, "
                    "Autothrottle_limit = %d, messages = %d, "
                    "soft_limit = %d, hard_limit = %d",
                    currentCalls, maxCalls, autothrottle_limit,
                    messages, soft_limit, hard_limit);
      }

      // Check to see if the MT queue is over-limit.
      if (messages >=
          (autothrottle_limit == -1 ? soft_limit : autothrottle_limit))
      {
         no_ticks_over_limit++;
         // Check to see if the MT queue has been over-limit too long.
         if (no_ticks_over_limit >= autothrottle_trigger)
         {
            // Set maxCalls to 1 less than the current level of calls.
            int newMaxCalls = currentCalls - 1;
            // Make sure maxCalls does not increase.
            if (newMaxCalls > maxCalls)
            {
               newMaxCalls = maxCalls;
            }
            // Make sure maxCalls is never less than 1!
            if (newMaxCalls < 1)
            {
               newMaxCalls = 1;
            }
            pCallMgr->setMaxCalls(newMaxCalls);
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ALERT,
                          "autoThrottleFunction "
                          "Throttled max calls from %d to %d.",
                          maxCalls, newMaxCalls);

            // Reset the counter to zero for the next cycle.
            no_ticks_over_limit = 0;
         }
      }
      else
      {
         no_ticks_over_limit = 0;
      }

      OsTask::delay(autothrottle_tick);
   }
}

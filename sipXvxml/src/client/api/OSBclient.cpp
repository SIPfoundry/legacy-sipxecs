/****************License************************************************
 *
 * Copyright 2000-2001.  SpeechWorks International, Inc.  
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
 * 
 *
 * Initialization and creation of resources required by the VXI core.
 * Isolates the main program from the actual resource management
 * implementation details.
 *
 * Each component specifies the properties that they take for
 * initialization.  For example OSBprompt specifies the parameters the
 * parameters for prompting.
 *
 ************************************************************************
 */

static const char *rcsid = 0 ? (char *) &rcsid :
"";

// -----1=0-------2=0-------3=0-------4=0-------5=0-------6=0-------7=0-------8

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <wchar.h>
#include <wctype.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#define VXIPLATFORM_EXPORTS
#include <VXItypes.h>
#include <OSBclient.h>
#include <SBclientUtils.h>
#include <VXIplatform.h>

#include <OSBlog.h>
#include <OSBjsi.h>
#include <OSBinet.h>
#include <OSBtel.h>
#include <OSBrec.h>
#include <OSBprompt.h>
#include <OSBobject.h>
#include "ivr/IvrTelListener.h"
#include "ivr/IvrUtilTask.h"

#ifdef WIN32
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>          /* For GetFullPathName() */
#endif

#ifndef MODULE_PREFIX
#define MODULE_PREFIX  COMPANY_DOMAIN L"."
#endif
#define MODULE_OSBCLIENT MODULE_PREFIX L"OSBclient"
#define PROPERTY_MAX_LEN 0x100
#ifndef MAX_PATH
#define MAX_PATH  4096
#endif

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/* Globals */
static VXIbool gblPlatformInitialized = FALSE;
static VXIlogInterface *gblLog = NULL;
static VXIunsigned gblDiagLogBase = 0;
static const VXIchar *gblUserAgentName = NULL;


/**
 * Initialize the platform.  
 *
 * This function initializes all the components in the OSB PIK in the
 * required order.  The order can be changed quite a bit, but logging
 * must be initialized first. Also VXI requires that all the
 * components be initialized and set before it can be created.
 */
VXIPLATFORM_API VXIplatformResult 
VXIplatformInit(VXIMap *configArgs, VXIunsigned *nbChannels)
{
  VXIlogResult logResult;
  VXIpromptResult promptResult;
  VXIinetResult inetResult;
  VXItelResult telResult;
  VXIjsiResult jsiResult;
  VXIobjResult objResult;
  VXIrecResult recResult;
  VXIinterpreterResult interpreterResult;
  VXIint32 diagLogBase;
  
  if (gblPlatformInitialized) {
    return VXIplatform_RESULT_ALREADY_INITIALIZED;
  }

  /* Initialize OSBclient globals */
  {
    VXIint32 tempInt = 0;
    SBclientGetParamInt(configArgs, CLIENT_CLIENT_DIAG_BASE, &tempInt, TRUE);
    gblDiagLogBase = (VXIunsigned) tempInt;
  }

  /**
   * Initialize Logging, must be done first
   */
  {
    const VXIchar *lfname = NULL;
    char nfname[MAX_PATH];
    VXIbool logToStdout = FALSE;

    /* Load configuration parameters */
    SBclientGetParamStr(configArgs, CLIENT_LOG_FILE_NAME, &lfname, TRUE);
    if (lfname)
      wcstombs(nfname, lfname, wcslen(lfname) + 1);
    else
      nfname[0] = '\0';
    SBclientGetParamBool(configArgs, CLIENT_LOG_LOG_TO_STDOUT, &logToStdout,
			 FALSE);

    /* Initialize the logging interface */
    logResult = OSBlogInit(nfname, logToStdout); 
    CLIENT_CHECK_RESULT("OSBlogInit()", logResult);
    
    /* Create a global log stream with channel number -1 which is not
       used as a real channel number. This log is used for global
       operations. */
    logResult = OSBlogCreateResource(-1, &gblLog);
    CLIENT_CHECK_RESULT("OSBlogCreateResource()", logResult);

    /* Turn on/off diagnostic tags based on the configuration settings */
    SBclientConfigureDiagnosticTags (configArgs, NULL);

    /* Now log entry */
    SBclientDiag(NULL, CLIENT_API_TAG, L"VXIplatformInit", 
		 L"entering (log init complete): 0x%p, 0x%p", 
		 configArgs, nbChannels);
  }


  /**
   * Initialize the telephony interface
   */
  {
    diagLogBase = 0;
    SBclientGetParamInt(configArgs, CLIENT_TEL_DIAG_BASE, &diagLogBase, TRUE);
    telResult = OSBtelInit(gblLog, (VXIunsigned) diagLogBase);
    CLIENT_CHECK_RESULT("OSBtelInit()", telResult);
  }


  /**
   * Initialize the Internet fetch component
   */
  {
    /* Load configuration parameters */
    const VXIchar *cachePath = NULL;
    VXIint32       cacheTotalSizeMB = 0;
    VXIint32       cacheEntryMaxSizeMB = 0;
    VXIint32       cacheEntryExpTimeSec = 0;
    const VXIchar *proxyServer = NULL;
    VXIint32       proxyPort = 0;
    const VXIMap  *extensionRules = NULL;
    VXIMap *loadedExtensionRules = NULL;
    diagLogBase = 0;

    SBclientGetParamStr(configArgs, CLIENT_INET_CACHE_DIR, &cachePath, TRUE);
    SBclientGetParamInt(configArgs, CLIENT_INET_CACHE_TOTAL_SIZE_MB,
		      &cacheTotalSizeMB, TRUE);
    SBclientGetParamInt(configArgs, CLIENT_INET_CACHE_ENTRY_MAX_SIZE_MB,
		      &cacheEntryMaxSizeMB, TRUE);
    SBclientGetParamInt(configArgs, CLIENT_INET_CACHE_ENTRY_EXP_TIME_SEC,
		      &cacheEntryExpTimeSec, TRUE);
    SBclientGetParamMap(configArgs, CLIENT_INET_EXTENSION_RULES, 
			&extensionRules, FALSE);
    SBclientGetParamInt(configArgs, CLIENT_INET_DIAG_BASE, &diagLogBase, TRUE);
    SBclientGetParamStr(configArgs, CLIENT_INET_USER_AGENT, &gblUserAgentName,
			TRUE);

    /* If no extension rules loaded yet, they may be directly present in
       the top-level configuration map, scan for them */
    if (!extensionRules) {
      const VXIchar *key;
      const VXIValue *val;
      size_t prefixlen = wcslen(CLIENT_INET_EXTENSION_RULE_PREFIX);
      
      VXIMapIterator *iter = VXIMapGetFirstProperty(configArgs,&key,&val);
      if (iter) {
        do {
          if (wcsncmp(key, CLIENT_INET_EXTENSION_RULE_PREFIX, prefixlen)==0) {
            if (VXIValueGetType(val) == VALUE_STRING) {
              /* get extension from the key */
              const VXIchar *ext = key + prefixlen - 1;
              if (wcslen(ext) > 1) {   
                if (loadedExtensionRules == NULL) {
                  loadedExtensionRules = VXIMapCreate();
		  CLIENT_CHECK_MEMALLOC(loadedExtensionRules,
					"OSBinet extension rules");
                }

                VXIMapSetProperty(loadedExtensionRules, ext,
                                  VXIValueClone(val));
              } else {
                fprintf(stderr, "ERROR: Invalid extension suffix for "
                        "configuration parameter, %ls\n", key);
              }
            } else {
              fprintf(stderr, "ERROR: Invalid type for configuration "
                      "parameter, %ls, VXIInteger required\n", key);
            }
          }
        } while (VXIMapGetNextProperty(iter, &key, &val) == 0);
      }
      VXIMapIteratorDestroy(&iter);
     
      if (loadedExtensionRules)
        extensionRules = loadedExtensionRules;
    }

    /* The next three are optional */
    SBclientGetParamStr(configArgs, CLIENT_INET_PROXY_SERVER, &proxyServer,
			FALSE);
    SBclientGetParamInt(configArgs, CLIENT_INET_PROXY_PORT, &proxyPort, FALSE);

    /* Initialize the Internet component */
    inetResult = OSBinetInit(gblLog, (VXIunsigned) diagLogBase, cachePath, 
                            cacheTotalSizeMB, cacheEntryMaxSizeMB,
                            cacheEntryExpTimeSec, proxyServer, proxyPort,
			     gblUserAgentName, extensionRules, configArgs, NULL);
    CLIENT_CHECK_RESULT("OSBinetInit()", inetResult);

    if (loadedExtensionRules)
      VXIMapDestroy(&loadedExtensionRules);
  }


  /**
   * Initialize the Prompt component
   */
  {
    /* Load configuration parameters */
    diagLogBase = 0;
    SBclientGetParamInt(configArgs, CLIENT_PROMPT_DIAG_BASE, &diagLogBase,
			TRUE);
    
    const VXIchar *promptUrl = 0;
    SBclientGetParamStr(configArgs, CLIENT_PROMPT_BASE_URL, &promptUrl,
			TRUE);
    /* Initialize prompting */
    promptResult = OSBpromptInit(gblLog, (VXIunsigned) diagLogBase, promptUrl);
    CLIENT_CHECK_RESULT("OSBpromptInit()", promptResult);
  }


  /**
   * Initialize the Recognition component
   */
  {
    /* Retrieve the OSBrec diagnostic base TAG ID */
    diagLogBase = 0;
    SBclientGetParamInt(configArgs, CLIENT_REC_DIAG_BASE, &diagLogBase, TRUE);

    /* Initialize the recognition interface */
    recResult = OSBrecInit(gblLog, (VXIunsigned) diagLogBase);
    CLIENT_CHECK_RESULT("OSBrecInit()", recResult);
  }


  /**
   * Initialize the ECMAScript component
   */
  {
    /* Load configuration information */
    VXIint32  runtimeSize = 0;
    VXIint32  contextSize = 0;
    VXIint32  maxBranches = 0;
    diagLogBase = 0;

    SBclientGetParamInt(configArgs, CLIENT_JSI_RUNTIME_SIZE_BYTES, 
			&runtimeSize, TRUE);
    SBclientGetParamInt(configArgs, CLIENT_JSI_CONTEXT_SIZE_BYTES, 
			&contextSize, TRUE);
    SBclientGetParamInt(configArgs, CLIENT_JSI_MAX_BRANCHES, &maxBranches,
			TRUE);
    SBclientGetParamInt(configArgs, CLIENT_JSI_DIAG_BASE, &diagLogBase, TRUE);
    
    /* Initialize the ECMAScript engine */
    jsiResult = OSBjsiInit(gblLog, (VXIunsigned) diagLogBase, runtimeSize,
                          contextSize, maxBranches);
    CLIENT_CHECK_RESULT("OSBjsiInit()", jsiResult);
  }


  /**
   * Initialize the Object component
   */
  {
    /* Load configuration information */
    diagLogBase = 0;
    SBclientGetParamInt(configArgs, CLIENT_OBJ_DIAG_BASE, &diagLogBase, TRUE);
    
    /* Initialize the ECMAScript engine */
    objResult = OSBobjectInit(gblLog, (VXIunsigned) diagLogBase);
    CLIENT_CHECK_RESULT("OSBobjInit()", objResult);
  }


  /**
   * Initialize the VoiceXML interpreter
   */
  {
    /* Retrieve the OSBjsi diagnosis base TAG ID */
    diagLogBase = 0;
    SBclientGetParamInt(configArgs, CLIENT_VXI_DIAG_BASE, &diagLogBase, TRUE);

    /* Initialize the interpreter */
    interpreterResult = VXIinterpreterInit(gblLog, 
                                           (VXIunsigned) diagLogBase);
    CLIENT_CHECK_RESULT("VXIinterpreterInit()", interpreterResult);
  }
  
  
  /* store the number of channels */
  *nbChannels = UINT_MAX;
  gblPlatformInitialized = TRUE;
    
  SBclientDiag(NULL, CLIENT_API_TAG, L"VXIplatformInit", 
	       L"exiting: rc = %d, %u", VXIplatform_RESULT_SUCCESS,
	       (nbChannels ? *nbChannels : ~0));

  return VXIplatform_RESULT_SUCCESS;
}


/**
 * Shutdown the platform. 
 *
 * VXIplatformShutdown can only be called when all the resources have
 * been destroyed.  
 */
VXIPLATFORM_API VXIplatformResult VXIplatformShutdown(void)
{
  VXIlogResult logResult;
  VXItelResult telResult;
  VXIpromptResult promptResult;
  VXIinetResult inetResult;
  VXIjsiResult jsiResult;
  VXIobjResult objResult;
  VXIrecResult recResult;

  /* Error if not already initialized */
  if (!gblPlatformInitialized) {
    return VXIplatform_RESULT_NOT_INITIALIZED;
  }


  SBclientDiag(NULL, CLIENT_API_TAG, L"VXIplatformShutdown", L"entering");

  /* Do everything in reverse.  Shutdown the VXI first */
  VXIinterpreterShutDown(gblLog);
  
  /* Now shutdown Object */
  objResult = OSBobjectShutDown(gblLog);
  CLIENT_CHECK_RESULT("OSBobjectShutDown()", objResult);
  
  /* Now shutdown ECMAScript */
  jsiResult = OSBjsiShutDown(gblLog);
  CLIENT_CHECK_RESULT("OSBjsiShutDown()", jsiResult);
  
  /* Now shut down the recognizer */
  recResult = OSBrecShutDown(gblLog);
  CLIENT_CHECK_RESULT("OSBrecShutDown()", recResult);

  /* Shutdown the prompting engine. */
  promptResult = OSBpromptShutDown(gblLog);
  CLIENT_CHECK_RESULT("OSBpromptShutDown()", promptResult);
  
  /* Shutdown the internet fetching component */
  inetResult = OSBinetShutDown(gblLog);
  CLIENT_CHECK_RESULT("OSBinetShutDown()", inetResult);

  /* Shutdown the tel component */
  telResult = OSBtelShutDown(gblLog);
  CLIENT_CHECK_RESULT("OSBtelShutDown()", telResult);

  /* Destroy the global logging API.  The system is going
     out of service. Should be one of the last functions run. */
  SBclientDiag(NULL, CLIENT_API_TAG, L"VXIplatformShutdown", 
	       L"exiting: rc = %d (prior to log shutdown)",
	       VXIplatform_RESULT_SUCCESS);

  logResult = OSBlogDestroyResource(&gblLog);
  CLIENT_CHECK_RESULT("OSBlogDestroyResource()", logResult);

  /* Shutdown the logging API. At this point system should just
     be returning and no OSB PIK component functions should be run */
  logResult = OSBlogShutDown();
  CLIENT_CHECK_RESULT("OSBlogShutDown()", logResult);

  return VXIplatform_RESULT_SUCCESS;
}



/**
 * Create resources for the platform.
 *
 * This function creates all the resources needed by the VXI to
 * execute.  This includes both APIs that the VXI calls directly and
 * the APIS that those components call.  All components in the OSB PIK are
 * exposed here.<p>
 *
 * The resources will be written into the platform pointer which is
 * allocated in this function.  
 */
VXIPLATFORM_API VXIplatformResult
VXIplatformCreateResources(VXIunsigned channelNum,
                           VXIMap *configArgs, 
                           VXIplatform **platform,
						   CallManager *pCallMgr)
{
  VXIplatform *newPlatform;
  VXItelResult telResult;
  VXIlogResult logResult;
  VXIinetResult inetResult;
  VXIpromptResult promptResult;
  VXIjsiResult jsiResult;
  VXIobjResult objResult;
  VXIrecResult recResult;
  VXIinterpreterResult interpreterResult;
  
  if (!gblPlatformInitialized) {
    return VXIplatform_RESULT_NOT_INITIALIZED;
  }
  if (platform == NULL) {
    return VXIplatform_RESULT_INVALID_ARGUMENT;
  }

  SBclientDiag(NULL, CLIENT_API_TAG, L"VXIplatformCreateResources",
	       L"entering: %u, 0x%p, 0x%p", channelNum, configArgs, platform);

  /* The platform object has been allocated in main, so now just copy the
   * handle and update it. 
   */
  newPlatform = *platform;
  newPlatform->channelNum = channelNum;    

  /* Create a channel logging resource.  A logging interface, per
     channel, is required to do the logging.  The channel number is
     bound at the creation time and will appear in any log message
     generated through this interface instance. */
  logResult = OSBlogCreateResource(channelNum, &newPlatform->VXIlog);
  CLIENT_CHECK_RESULT("OSBlogCreateResource()", logResult);

  /* Turn on/off diagnostic tags based on the configuration settings,
     we have to log the implementation name after this otherwise
     we'll never see the message */
  SBclientConfigureDiagnosticTags (configArgs, newPlatform);
  CLIENT_LOG_IMPLEMENTATION(L"VXIlog", newPlatform->VXIlog);
  
  /* Create the VXItel implementation resource.  This is required by
     the VXI for handling telephony functions. The telephony resource
     uses the session control resource to perform its functions so it
     must be passed in. */
  telResult = OSBtelCreateResource(channelNum, newPlatform->VXIlog, &newPlatform->VXItel);
  CLIENT_CHECK_RESULT("OSBtelCreateResource()", telResult);
  CLIENT_LOG_IMPLEMENTATION(L"VXItel", newPlatform->VXItel);
  
  /* Lookup whether cookies should be accepted */
  SBclientGetParamBool(configArgs, CLIENT_INET_ACCEPT_COOKIES, 
		       &newPlatform->acceptCookies, TRUE);
  
  /* Create the internet component for fetching URLs */
  inetResult = OSBinetCreateResource(newPlatform->VXIlog, 
				     &newPlatform->VXIinet);
  CLIENT_CHECK_RESULT("OSBinetCreateResource()", inetResult);
  CLIENT_LOG_IMPLEMENTATION(L"VXIinet", newPlatform->VXIinet);

  /* Now create the prompt resource. The prompting resource takes a
     logging interface for logging and the inet interface for getting
     URIs */
  promptResult = OSBpromptCreateResource(channelNum,
					 newPlatform->VXIlog, 
					 newPlatform->VXIinet,
					 &newPlatform->VXIprompt,
					 pCallMgr);
  CLIENT_CHECK_RESULT("OSBpromptCreateResource()", promptResult);
  CLIENT_LOG_IMPLEMENTATION(L"VXIprompt", newPlatform->VXIprompt);
  
  /* Now create the recognizer resource */
  recResult = OSBrecCreateResource(channelNum, newPlatform->VXIlog, &newPlatform->VXIrec);
  CLIENT_CHECK_RESULT("OSBrecCreateResource()", recResult);
  CLIENT_LOG_IMPLEMENTATION(L"VXIrec", newPlatform->VXIrec);
  
  /* Now create the jsi resource.  This is used for storing and
     computing ECMAScript results by the VXI. Internal component
     required by VXI */
  jsiResult = OSBjsiCreateResource(newPlatform->VXIlog, &newPlatform->VXIjsi);
  CLIENT_CHECK_RESULT("OSBjsiCreateResource()", jsiResult);
  CLIENT_LOG_IMPLEMENTATION(L"VXIjsi", newPlatform->VXIjsi);

  /* Now create the object resource.  This is used for executing
     VoiceXML <object> elements */
  newPlatform->objectResources.log     = newPlatform->VXIlog;
  newPlatform->objectResources.inet    = newPlatform->VXIinet;
  newPlatform->objectResources.jsi     = newPlatform->VXIjsi;
  newPlatform->objectResources.rec     = newPlatform->VXIrec;
  newPlatform->objectResources.prompt  = newPlatform->VXIprompt;
  newPlatform->objectResources.tel     = newPlatform->VXItel;

  objResult = OSBobjectCreateResource(&newPlatform->objectResources,
				     &newPlatform->VXIobject);
  CLIENT_CHECK_RESULT("OSBobjectCreateResource()", objResult);
  CLIENT_LOG_IMPLEMENTATION(L"VXIobject", newPlatform->VXIobject);

  /* Now copy the components required by the VXI into its resource
     structure for the VXI initialization */
  newPlatform->resources.log     = newPlatform->VXIlog;
  newPlatform->resources.inet    = newPlatform->VXIinet;
  newPlatform->resources.jsi     = newPlatform->VXIjsi;
  newPlatform->resources.rec     = newPlatform->VXIrec;
  newPlatform->resources.prompt  = newPlatform->VXIprompt;
  newPlatform->resources.tel     = newPlatform->VXItel;
  newPlatform->resources.object  = newPlatform->VXIobject;

  interpreterResult = 
    VXIinterpreterCreateResource(&newPlatform->resources,
                                 &newPlatform->VXIinterpreter);
  CLIENT_CHECK_RESULT("VXIinterpreterCreateResource()", interpreterResult);
  CLIENT_LOG_IMPLEMENTATION(L"VXIinterpreter", newPlatform->VXIinterpreter);

  /* Set interpreter properties. */
  {
    const VXIchar *vxiBeepURI = NULL;
    const VXIchar *vxiDefaultsURI = NULL;

    VXIMap *vxiProperties = VXIMapCreate();
    CLIENT_CHECK_MEMALLOC(vxiProperties, "VXI properties");

    SBclientGetParamStr(configArgs, CLIENT_VXI_BEEP_URI, &vxiBeepURI, FALSE);
    if (vxiBeepURI != NULL) {
      VXIString * val = VXIStringCreate(vxiBeepURI);
      CLIENT_CHECK_MEMALLOC(val, "VXI properties beep URI");
      VXIMapSetProperty(vxiProperties, VXI_BEEP_AUDIO, (VXIValue *) val);
    }

    SBclientGetParamStr(configArgs, CLIENT_VXI_DEFAULTS_URI, &vxiDefaultsURI,
			FALSE);
    if (vxiDefaultsURI != NULL) {
      VXIString * val = VXIStringCreate(vxiDefaultsURI);
      CLIENT_CHECK_MEMALLOC(val, "VXI properties beep URI");
      VXIMapSetProperty(vxiProperties, VXI_PLATFORM_DEFAULTS, (VXIValue*) val);
    }

    if (VXIMapNumProperties(vxiProperties) != 0)
      newPlatform->VXIinterpreter->SetProperties(newPlatform->VXIinterpreter,
						 vxiProperties);
    VXIMapDestroy(&vxiProperties);
  }

  SBclientDiag(newPlatform, CLIENT_API_TAG, L"VXIplatformCreateResources",
	       L"exiting: rc = %d, 0x%p",
	       VXIplatform_RESULT_SUCCESS, newPlatform);

  /* Return the platform resources that have been created */
  *platform = newPlatform;
  return VXIplatform_RESULT_SUCCESS;
}


/**
 * Create resources for the platform.
 *
 * This function creates all the resources needed by the VXI to
 * execute.  This includes both APIs that the VXI calls directly and
 * the APIS that those components call.  All components in the OSB PIK are
 * exposed here.<p>
 *
 * The resources will be written into the platform pointer which is
 * allocated in this function.  
 */
VXIPLATFORM_API VXIplatformResult
VXIplatformAddResource(VXIunsigned channelNum,
			   CallManager *pCallMgr,
         IvrTelListener *pListener,
         VXIplatform **platform)
{
  VXIpromptResult promptResult;
  VXIrecResult recResult;
  VXItelResult telResult;
  VXIplatform *newPlatform;
  
  if (!gblPlatformInitialized) {
    return VXIplatform_RESULT_NOT_INITIALIZED;
  }
  if (platform == NULL) {
    return VXIplatform_RESULT_INVALID_ARGUMENT;
  }

  SBclientDiag(NULL, CLIENT_API_TAG, L"VXIplatformAddResource",
	       L"entering: %u, 0x%p", channelNum, platform);

  
  /* Now add call manager to the prompt resource. */
  promptResult = OSBpromptAddResource(&(*platform)->VXIprompt,
					 pCallMgr);

  /* Now add call manager to the rec resource. */
  recResult = OSBrecAddResource(&(*platform)->VXIrec,
					 pCallMgr,
           (*platform)->VXIprompt);

  /* Now add call manager to the tel resource. */
  telResult = OSBtelAddResource(&(*platform)->VXItel,
					 pCallMgr, pListener);

  newPlatform = *platform;
  CLIENT_CHECK_RESULT("OSBpromptCreateResource()", promptResult);
  CLIENT_LOG_IMPLEMENTATION(L"VXIprompt", (*platform)->VXIprompt);
  
  return VXIplatform_RESULT_SUCCESS;
}


/**
 * Destroy resources for the platform.
 *
 * This function destroys all the resources needed by the VXI to
 * execute.  This includes both APIs that the VXI calls directly and
 * the APIS that those components call.  All components in the OSB PIK are
 * exposed here.<p>
 *
 * The resources will be freed from the platform pointer which is
 * deleted and set to NULL in this function.  
 */
VXIPLATFORM_API VXIplatformResult
VXIplatformDestroyResources(VXIplatform **platform)
{
  VXItelResult telResult;
  VXIlogResult logResult;
  VXIpromptResult promptResult;
  VXIjsiResult jsiResult;
  VXIrecResult recResult;
  VXIinetResult inetResult;
  VXIobjResult objResult;
  VXIplatform *pPlatform = NULL;

  if ((platform == NULL) || (*platform == NULL)) {
    return VXIplatform_RESULT_INVALID_ARGUMENT;
  }
  if (!gblPlatformInitialized) {
    return VXIplatform_RESULT_NOT_INITIALIZED;
  }

  pPlatform = *platform;

  SBclientDiag(pPlatform, CLIENT_API_TAG, L"VXIplatformDestroyResources",
	       L"entering: 0x%p", platform);
  
  /* Destroy the VoiceXML interpreter */
  VXIinterpreterDestroyResource(&pPlatform->VXIinterpreter);

  /* Destroy the object handler */
  objResult = OSBobjectDestroyResource(&pPlatform->VXIobject);
  CLIENT_CHECK_RESULT("OSBobjectDestroyResource()", objResult);

  /* Destroy the ECMAScript interpreter */
  jsiResult = OSBjsiDestroyResource(&pPlatform->VXIjsi);
  CLIENT_CHECK_RESULT("OSBjsiDestroyResource()", jsiResult);

  /* Destroy the recognizer */
  recResult = OSBrecDestroyResource(&pPlatform->VXIrec);
  CLIENT_CHECK_RESULT("OSBrecDestroyResource()", recResult);

  /* Destroy the prompt engine */
  promptResult = OSBpromptDestroyResource(&pPlatform->VXIprompt);
  CLIENT_CHECK_RESULT("OSBpromptDestroyResource()", promptResult);
  
  /* Destroy the internet component */
  inetResult = OSBinetDestroyResource(&pPlatform->VXIinet);
  CLIENT_CHECK_RESULT("OSBinetDestroyResource()", inetResult);
  
  /* Destroy the telephony component */
  telResult = OSBtelDestroyResource(&pPlatform->VXItel);
  CLIENT_CHECK_RESULT("OSBtelDestroyResource()", telResult);
  
  /* Destroy the log component */
  logResult = OSBlogDestroyResource(&pPlatform->VXIlog);
  CLIENT_CHECK_RESULT("OSBlogDestroyResource()", logResult);
    
  /* Do not release the platform resource handle until the entire engine is shut down in main()*/

  SBclientDiag(NULL, CLIENT_API_TAG, L"VXIplatformDestroyResources",
	       L"exiting: rc = %d", VXIplatform_RESULT_SUCCESS);

  return VXIplatform_RESULT_SUCCESS;
}


/**
 * Enables the hardware to wait for a call
 *
 * This function enables the hardware to wait for a call using the
 * resources specified by the passed platform pointer. It blocks until
 * the hardware is enabled and then returns
 * VXIplatform_RESULT_SUCCESS.  This must be calld before
 * VXIplatformWaitForCall.
 */
VXIPLATFORM_API VXIplatformResult
VXIplatformEnableCall(VXIplatform *platform)
{
  VXItelResult telResult;
  OSBtelInterface *osbTel;

  SBclientDiag(platform, CLIENT_API_TAG, L"VXIplatformEnableCall",
	       L"entering: 0x%p", platform);

  if (!gblPlatformInitialized)
    return VXIplatform_RESULT_NOT_INITIALIZED;

  if (platform == NULL)
    return VXIplatform_RESULT_INVALID_ARGUMENT;

  SBclientDiag(platform, CLIENT_GEN_TAG, L"Entering EnableCall", NULL);

  /* Enable calls using the telephony interface */
  if (wcsstr(platform->VXItel->GetImplementationName( ), L".OSBtel")) {
    osbTel = (OSBtelInterface *) platform->VXItel;
    telResult = osbTel->EnableCall(osbTel);
  } else {
    telResult = VXItel_RESULT_UNSUPPORTED;
  }
  if (telResult != VXItel_RESULT_SUCCESS){
    SBclientError(platform, MODULE_OSBCLIENT, 100, L"%s%s%s%d",
		  L"MSG", L"OSBtel EnableCall failed", L"rc", telResult);
    return (telResult > 0 ? VXIplatform_RESULT_FAILURE : 
	    VXIplatform_RESULT_TEL_ERROR);
  }

  SBclientDiag(platform, CLIENT_API_TAG, L"VXIplatformEnableCall",
	       L"exiting: rc = %d", VXIplatform_RESULT_SUCCESS);
  SBclientDiag(platform, CLIENT_GEN_TAG, L"Leaving EnableCall", NULL);
  return VXIplatform_RESULT_SUCCESS;
}

/**
 * Wait for a call.
 *
 * This function waits for a call and then answers it using the
 * resources specified by the passed platform pointer. It blocks until
 * a call is recieved and then returns VXIplatform_RESULT_SUCCESS.
 */
VXIPLATFORM_API VXIplatformResult
VXIplatformWaitForCall(VXIplatform *platform)
{
  VXItelResult telResult;
  OSBtelInterface *osbTel;

  SBclientDiag(platform, CLIENT_API_TAG, L"VXIplatformWaitForCall",
	       L"entering: 0x%p", platform);

  if (platform == NULL)
    return VXIplatform_RESULT_INVALID_ARGUMENT;

  /* Wait for calls using the telephony interface */
  if (wcsstr(platform->VXItel->GetImplementationName( ), L".OSBtel")) {
    osbTel = (OSBtelInterface *) platform->VXItel;
    telResult = osbTel->WaitForCall(osbTel, &platform->telephonyProps);
  } else {
    telResult = VXItel_RESULT_UNSUPPORTED;
  }
  if (telResult != VXItel_RESULT_SUCCESS){
    SBclientError(platform, MODULE_OSBCLIENT, 100, L"%s%s%s%d",
		  L"MSG", L"OSBtel WaitForCall failed", L"rc", telResult);
    return (telResult > 0 ? VXIplatform_RESULT_FAILURE : 
	    VXIplatform_RESULT_TEL_ERROR);
  }

  SBclientDiag(platform, CLIENT_API_TAG, L"VXIplatformWaitForCall",
	       L"exiting: rc = %d", VXIplatform_RESULT_SUCCESS);
  SBclientDiag(platform, CLIENT_GEN_TAG, L"Leaving WaitForCall", NULL);

  return VXIplatform_RESULT_SUCCESS;
}


/**
 * Process a VoiceXML document.
 *
 * This function processes a VoiceXML document using the resources
 * specified by the passed platform pointer. It blocks until the
 * processing is complete and then returns VXIplatform_RESULT_SUCCESS.
 */
VXIPLATFORM_API VXIplatformResult
VXIplatformProcessDocument(const VXIchar *url,
                           VXIMap *sessionArgs,
                           VXIValue **documentResult,
                           VXIplatform *platform)
{
  VXIinterpreterResult interpreterResult;
  VXIinetResult inetResult;
  VXItelResult telResult;
  VXIpromptResult promptResult;
  VXIrecResult recResult;
  VXIchar *allocatedUrl = NULL;
  const VXIchar *finalUrl = NULL, *ani = NULL, *dnis = NULL;
  VXIVector *cookieJar = NULL;
  VXIMap *navigatorProps = NULL;

  if (!gblPlatformInitialized) {
    return VXIplatform_RESULT_NOT_INITIALIZED;
  }

  SBclientDiag(platform, CLIENT_API_TAG, L"VXIplatformProcessDocument",
	       L"entering: %s, 0x%p, 0x%p, 0x%p", 
	       url, sessionArgs, documentResult, platform);

  /* If the URL is really a local file path, change it to be a full
     path so relative URL references in it can be resolved */
  finalUrl = url;
  if (wcschr(url, L':') == NULL) {
    allocatedUrl = (VXIchar *) calloc(MAX_PATH + 1, sizeof(VXIchar));
    CLIENT_CHECK_MEMALLOC(allocatedUrl, "Allocated URL");

#ifdef WIN32
    /* Win32 version */
    {
      VXIchar *ignored;
      if (!allocatedUrl)
	CLIENT_CHECK_RESULT("full path allocation", -1);
      
      if (GetFullPathName((char*)url, MAX_PATH, (char*)allocatedUrl, (char**)&ignored) <= 0)
	CLIENT_CHECK_RESULT("Win32 GetFullPathName()", -1);
    }
#else
    {
      /* Unix version */
      char cwd[MAX_PATH + 1];
      cwd[0] = '\0';
      
      getcwd (cwd, MAX_PATH);
      if (!cwd[0])
	CLIENT_CHECK_RESULT("getcwd()", -1);
      
      if (strlen(cwd) + wcslen(url) + 1 > MAX_PATH)
	CLIENT_CHECK_RESULT("MAX_PATH exceeded for getting full path", -1);
      
      mbstowcs(allocatedUrl, cwd, MAX_PATH + 1);
      wcscat (allocatedUrl, L"/");
      wcscat (allocatedUrl, url);
    }
#endif

    finalUrl = allocatedUrl;
  }

  OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                "VXIplatformProcessDocument finalUrl = '%ls'", finalUrl);

  /* Set up OSBinet for a new call by enabling or disabling cookies as
     configured for the resource. When cookies are enabled, this
     implementation establishes a new cookie jar for each new call,
     and destroys it at the end of the call. This makes it so cookies
     do not persist across calls, meaning applications can use cookies
     to keep track of in-call state but not track state and callers
     across calls.

     Fully persistant cookies require a highly accurate mechanism for
     identifying individual callers (caller ID is not usually
     sufficient), a database for storing the cookie jar for each
     caller, and a way for individual callers to establish their
     privacy policy (web site or VoiceXML pages that allow them to
     configure whether cookies are enabled and cookie filters). Once
     in place, merely store the cookie jar at the end of the call,
     then once the caller is identified set the cookie jar to the
     previously stored one.  
  */
  if (platform->acceptCookies) {
    cookieJar = VXIVectorCreate();
    CLIENT_CHECK_MEMALLOC(cookieJar, "VXIinet cookie jar");
  }
  inetResult = platform->VXIinet->SetCookieJar (platform->VXIinet, cookieJar);
  CLIENT_CHECK_RESULT("VXIinet->SetCookieJar()", inetResult);
  VXIVectorDestroy (&cookieJar);

  /* Set up the prompt interface for a new call. Begin session has to 
     be called at the start of every call.  End session has to be
     called at the end of every call. */
  promptResult = platform->VXIprompt->BeginSession(platform->VXIprompt, sessionArgs);
  CLIENT_CHECK_RESULT("VXIprompt->BeginSession()", promptResult);

  /* Set up the telephony interface for a new call. Begin session has to 
     be called at the start of every call.  End session has to be
     called at the end of every call. */
  telResult = platform->VXItel->BeginSession(platform->VXItel, sessionArgs);
  CLIENT_CHECK_RESULT("VXItel->BeginSession()", telResult);

  /* Set up the recognizer for a new call. Begin session has to 
     be called at the start of every call.  End session has to be
     called at the end of every call. */
  recResult = platform->VXIrec->BeginSession(platform->VXIrec, sessionArgs);
  CLIENT_CHECK_RESULT("VXIrec->BeginSession()", recResult);

  /* Put the telephony map into the session map so they will be
     referenced as session.telephone.*. Note that ownership passes
     to the sessionArgs map. */
  if (platform->telephonyProps) {
    const VXIValue *val;
    val = VXIMapGetProperty(platform->telephonyProps, L"ani");
    if ((val) && (VXIValueGetType(val) == VALUE_STRING))
      ani = VXIStringCStr((const VXIString *) val);

    val = VXIMapGetProperty(platform->telephonyProps, L"dnis");
    if ((val) && (VXIValueGetType(val) == VALUE_STRING))
      dnis = VXIStringCStr((const VXIString *) val);
    
    VXIMapSetProperty(sessionArgs, L"telephone",
		      (VXIValue*)platform->telephonyProps);
    platform->telephonyProps = NULL;
  }
  
  /* Store properties for querying the browser implementation in the
     session map, they can be referenced as navigator.* (technically
     session.navigator.* but since the session scope is the global
     scope, navigator.* will be found, and the navigator.* form is
     what is universally used to query this information for HTML
     browsers). */
  navigatorProps = VXIMapCreate();
  CLIENT_CHECK_MEMALLOC(navigatorProps, "Navigator properties");

  {
    /* User agent name is configured */
    const VXIchar *start, *end;
    VXIString *str = VXIStringCreate(gblUserAgentName);
    CLIENT_CHECK_MEMALLOC(str, "Navigator User Agent Name");
    VXIMapSetProperty(navigatorProps, L"userAgent", (VXIValue *)str);

    /* Code name is the portion of the user agent up to the slash */
    end = wcschr(gblUserAgentName, L'/');
    if ((end) && (*end))
      str = VXIStringCreateN(gblUserAgentName, end - gblUserAgentName);
    else
      CLIENT_CHECK_RESULT("User Agent Name parse", 
		   VXIplatform_RESULT_INVALID_ARGUMENT);
    CLIENT_CHECK_MEMALLOC(str, "Navigator Browser Code Name");
    VXIMapSetProperty(navigatorProps, L"appCodeName", (VXIValue *)str);

    /* Version is the portion of the user agent after the slash and up
       through the alphanumeric sequence that follows it */
    start = end + 1;
    while (iswspace(*start))
      start++;
    end = start;
    while ((iswalnum(*end)) || (*end == L'.'))
      end++;
    
    str = VXIStringCreateN(start, end - start);
    CLIENT_CHECK_MEMALLOC(str, "Navigator Browser Version");
    VXIMapSetProperty(navigatorProps, L"appVersion", (VXIValue *)str);

    /* Application name is the full official product name */
    str = VXIStringCreate(L"SpeechWorks OpenSpeech Browser PIK");
    CLIENT_CHECK_MEMALLOC(str, "Navigator application name");
    VXIMapSetProperty(navigatorProps, L"appName", (VXIValue *)str);
  }

  VXIMapSetProperty(sessionArgs,L"navigator",(VXIValue*)navigatorProps);

  /* Log the start of call event */
  platform->VXIlog->Event(platform->VXIlog, VXIlog_EVENT_CALL_START,
			  L"%s%s%s%s%s%s", L"ANII", (ani ? ani : L""),
			  L"DNIS", (dnis ? dnis : L""), L"VURL", finalUrl);

  /* Ready to run the VXI.  This will return with a result of one of
     the following:

     a) a VoiceXML page hits an <exit> tag. In this case result will
        contain the value of the exit.
     b) A page simply has no where to go.  The application falls out
        of VoiceXML and ends. result will be NULL in this case.
     c) An error occurs.  result will be NULL in this case.

     Note that the session arguments contain all the information that
     the platform wants to put into the ECMAScript variable session at
     channel startup.
     
     The initial URL will be fetched with a POST sending the session
     arguments in the initial POST.  
  */
  interpreterResult = platform->VXIinterpreter->Run(platform->VXIinterpreter,
                                                    finalUrl, sessionArgs,
                                                    documentResult);
  CLIENT_CHECK_RESULT_NO_EXIT("VXIinterpreter->Run()", interpreterResult);
  if (allocatedUrl)
    free(allocatedUrl);

  /* Log the end of call event */
  platform->VXIlog->Event(platform->VXIlog, VXIlog_EVENT_CALL_END, NULL);

  /* Now end the recognizer, prompt, and tel session */
  telResult = platform->VXItel->EndSession(platform->VXItel, NULL);
  CLIENT_CHECK_RESULT("OSBtel->EndSession()", telResult);

  recResult = platform->VXIrec->EndSession(platform->VXIrec, NULL);
  CLIENT_CHECK_RESULT("OSBrec->EndSession()", recResult);

  promptResult = platform->VXIprompt->EndSession(platform->VXIprompt, NULL);
  CLIENT_CHECK_RESULT("OSBprompt->EndSession()", promptResult);

  SBclientDiag(platform, CLIENT_API_TAG, L"VXIplatformProcessDocument",
	       L"exiting: rc = %d, 0x%p", interpreterResult, 
	       (documentResult ? *documentResult : NULL));

  /* Do a map of the interpreter result to a platform result */
  return SBclientConvertInterpreterResult (interpreterResult);
}


/**
 * Log an error
 */
VXIPLATFORM_API VXIlogResult 
SBclientError(VXIplatform *platform, const VXIchar *moduleName,
	      VXIunsigned errorID, const VXIchar *format, ...)
{
  VXIlogResult rc;
  VXIlogInterface *log = NULL;
  va_list arguments;

  if ((platform) && (platform->VXIlog))
    log = platform->VXIlog;
  else if (gblLog)
    log = gblLog;
  else
    return VXIlog_RESULT_NON_FATAL_ERROR;
  
  if (format) {
    va_start(arguments, format);
    rc = (*log->VError)(log, moduleName, errorID, format, arguments);
    va_end(arguments);
  } else {
    rc = (*log->Error)(log, moduleName, errorID, NULL);
  }

  return rc;
}


/**
 * Log a diagnostic message
 */
VXIPLATFORM_API VXIlogResult 
SBclientDiag(VXIplatform *platform, VXIunsigned tag, 
	     const VXIchar *subtag, const VXIchar *format, ...)
{
  VXIlogResult rc;
  VXIlogInterface *log = NULL;
  va_list arguments;

  if ((platform) && (platform->VXIlog))
    log = platform->VXIlog;
  else if (gblLog)
    log = gblLog;
  else
    return VXIlog_RESULT_NON_FATAL_ERROR;

  if (format) {
    va_start(arguments, format);
    rc = (*log->VDiagnostic)(log, tag, subtag, format, arguments);
    va_end(arguments);
  } else {
    rc = (*log->Diagnostic)(log, tag, subtag, NULL);
  }

  return rc;
}


/**
 * Enable/disable diagnostic tags based on the passed configuration data
 */
VXIPLATFORM_API VXIplatformResult 
SBclientConfigureDiagnosticTags(const VXIMap     *configArgs,
				VXIplatform      *platform)
{
  VXIplatformResult rc = VXIplatform_RESULT_SUCCESS;
  const VXIchar *key;
  const VXIValue *val;
  size_t prefixlen = wcslen(CLIENT_LOG_DIAG_TAG_KEY_PREFIX);
  VXIMapIterator *iter;
  VXIlogInterface *log = NULL;
  OSBlogInterface *osbLog = NULL;

  if (!configArgs)
    rc = VXIplatform_RESULT_INVALID_ARGUMENT;

  if ((platform) && (platform->VXIlog))
    log = platform->VXIlog;
  else if (gblLog)
    log = gblLog;
  else
    return VXIplatform_RESULT_NON_FATAL_ERROR;

  /* Determine the implementation */
  if (wcsstr(log->GetImplementationName( ), L".OSBlog")) {
    osbLog = (OSBlogInterface *) log;
  } else {
    fprintf(stderr, "ERROR: Unknown log implementation for "
	    "SBclientConfigureDiagnosticTags( ), returning failure");
    return VXIplatform_RESULT_FAILURE;
  }

  /* Configure the diagnostic tags */
  iter = VXIMapGetFirstProperty(configArgs,&key,&val);
  if (iter) {
    do {
      if (wcsncmp(key, CLIENT_LOG_DIAG_TAG_KEY_PREFIX, prefixlen)==0) {
	if (VXIValueGetType(val) == VALUE_INTEGER) {
	  VXIunsigned tflag = VXIIntegerValue((const VXIInteger*)val);
	  /* get suffix TAG ID from the key */
	  VXIchar *ptr;
	  VXIunsigned tagID = (VXIunsigned) wcstol (key + prefixlen,&ptr,10);
	  if (tagID >= 0)
	    osbLog->ControlDiagnosticTag(osbLog, tagID, tflag ? TRUE :FALSE);
	  else
	    fprintf(stderr, "ERROR: Invalid tag ID suffix for "
		    "configuration parameter, %ls, integer greater then "
		    "zero required\n", key);
	} else {
	  fprintf(stderr, "ERROR: Invalid type for configuration "
		  "parameter, %ls, VXIInteger required\n", key);
	}
      }
    } while (VXIMapGetNextProperty(iter, &key, &val) == 0);
  }
  VXIMapIteratorDestroy(&iter);

  return rc;
}

/**
 * Seting the exit flag (live = 0) so the prompt and rec will exit after the 
 * last timeout event. recognize will throw TELEPHONY_HANGUP event.
 */
VXIPLATFORM_API VXIplatformResult 
SBclientExitingCall (VXIplatform      *platform)
{
  VXIplatformResult rc = VXIplatform_RESULT_SUCCESS;

  /* Error if not already initialized */
  if (!gblPlatformInitialized) {
    return VXIplatform_RESULT_NOT_INITIALIZED;
  }

  SBclientDiag(NULL, CLIENT_API_TAG, L"SBclientExitingCall", L"entering");

  OSBtelExiting (platform->VXIlog, &platform->VXItel);
  OSBrecExiting (platform->VXIlog, &platform->VXIrec);
  OSBpromptExiting (platform->VXIlog, &platform->VXIprompt);
  return rc;
}


/**
 * Set the flush period of the log file.  
 *
 * This function sets the file handle of the IvrUtilTask, so that the 
 * IvrUtilTask will periodically flush the log file.
 */
VXIPLATFORM_API VXIplatformResult 
VXIsetLogFlushPeriod(VXIMap *configArgs, void** pIvrUtilTask)
{
  VXIplatformResult rc = VXIplatform_RESULT_SUCCESS;
  const VXIchar *lfname = NULL;
  char nfname[MAX_PATH];
  VXIbool logToStdout = FALSE;

  /* Load configuration parameters */
  SBclientGetParamStr(configArgs, CLIENT_LOG_FILE_NAME, &lfname, TRUE);
  if (lfname)
    wcstombs(nfname, lfname, wcslen(lfname) + 1);
  else
    nfname[0] = '\0';
  SBclientGetParamBool(configArgs, CLIENT_LOG_LOG_TO_STDOUT, &logToStdout,
	   FALSE);

  if (gblLog && pIvrUtilTask)
  {
    IvrUtilTask* pTask = IvrUtilTask::getIvrUtilTask();
    if (!pTask || OS_SUCCESS != pTask->setLogHandle(gblLog, nfname, logToStdout))
      rc = VXIplatform_RESULT_FAILURE;
    *pIvrUtilTask = pTask;
  }

  return rc;
}


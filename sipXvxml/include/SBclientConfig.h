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
 * Configuration parameters used by SBclient.cpp and OSBclient.cpp
 *
 ************************************************************************
 */

#ifndef _SBCLIENT_CONFIG_H
#define _SBCLIENT_CONFIG_H

/** 
  @name SBclient properties
  @memo Definition of properties used by testClient
  @version 1.0 <br>
  @doc Collection of properties for each of the components used in the
  OpenSpeech Browser.  These properites are used by testClient and are
  read from the configuration file.
*/ 

/*@{*/

/**
  @name INET keys
  @memo Keys for the INET cache 
  @version 1.0
  @doc Properties read from the configuration file used to
  configure SBinet.
 */
/*@{*/
#define CLIENT_INET_CACHE_DIR                  L"client.inet.cacheDir"
#define CLIENT_INET_CACHE_TOTAL_SIZE_MB        L"client.inet.cacheTotalSizeMB"
#define CLIENT_INET_CACHE_ENTRY_MAX_SIZE_MB    L"client.inet.cacheEntryMaxSizeMB"
#define CLIENT_INET_CACHE_ENTRY_EXP_TIME_SEC   L"client.inet.cacheEntryExpTimeSec"
#define CLIENT_INET_PROXY_SERVER               L"client.inet.proxyServer"
#define CLIENT_INET_PROXY_PORT                 L"client.inet.proxyPort"
#define CLIENT_INET_EXTENSION_RULES            L"client.inet.extensionRules"
#define CLIENT_INET_EXTENSION_RULE_PREFIX      L"client.inet.extensionRule."
#define CLIENT_INET_USER_AGENT                 L"client.inet.userAgent"
#define CLIENT_INET_ACCEPT_COOKIES             L"client.inet.acceptCookies"
/*@}*/

/**
 @name Cache Keys
 @memo Keys for the write-back cache 
 */
/*@{*/
#define CLIENT_CACHE_CACHE_DIR                 L"client.cache.cacheDir"
#define CLIENT_CACHE_CACHE_TOTAL_SIZE_MB       L"client.cache.cacheTotalSizeMB"
#define CLIENT_CACHE_CACHE_ENTRY_MAX_SIZE_MB   L"client.cache.cacheEntryMaxSizeMB"
#define CLIENT_CACHE_CACHE_ENTRY_EXP_TIME_SEC  L"client.cache.cacheEntryExpTimeSec"
#define CLIENT_CACHE_UNLOCK_ENTRIES            L"client.cache.unlockEntries"
/*@}*/

/**
  @name JSI Keys
  @memo Keys for the ECMAScript interface
 */
/*@{*/
#define CLIENT_JSI_RUNTIME_SIZE_BYTES          L"client.jsi.runtimeSizeBytes"
#define CLIENT_JSI_CONTEXT_SIZE_BYTES          L"client.jsi.contextSizeBytes"
#define CLIENT_JSI_MAX_BRANCHES                L"client.jsi.maxBranches"
/*@}*/

/**
  @name Prompt Keys
  @memo Keys for the prompt interface
 */
/*@{*/
#define CLIENT_PROMPT_DEFAULT_REC_SRC          L"client.prompt.defaultRecSrc"
#define CLIENT_PROMPT_RESOURCES                L"client.prompt.resources"
#define CLIENT_PROMPT_RESOURCE_PREFIX          L"client.prompt.resource."
/*@}*/

/**
  @name Recognizer Keys
  @memo Keys for the recognition interface
 */
/*@{*/
#define CLIENT_REC_INIT_URI                    L"client.rec.initURI"
#define CLIENT_REC_WAVEFORM_CAPTURE_CHANNEL_LIMIT L"client.rec.waveformCaptureChannelLimit"
/*@}*/

/**
  @name Telephony Keys
  @memo Keys for the telephony interface
 */
/*@{*/
#define CLIENT_TEL_HW_IMPLEMENTATION           L"client.tel.hwImplementation"
#define CLIENT_TEL_SC_GC_PROTOCOL              L"client.tel.scGcProtocol"
#define CLIENT_TEL_DELAY_BEFORE_HANGUP_MSEC    L"client.tel.delayBeforeHangupMsec"
/*@}*/

/**
  @name VXI keys
  @memo Keys for the VXI interface
  */
/*@{*/
#define CLIENT_VXI_BEEP_URI                    L"client.vxi.beepURI"
#define CLIENT_VXI_DEFAULTS_URI                L"client.vxi.defaultsURI"
/*@}*/

/**
  @name Log keys
  @memo Keys for the Log interface
 */
/*@{*/
#define CLIENT_LOG_FILE_NAME                L"client.log.filename"
#define CLIENT_LOG_MAX_SIZE_MB              L"client.log.maxLogSizeMB"
#define CLIENT_LOG_DIAG_TAG_KEY_PREFIX      L"client.log.diagTag."
#define CLIENT_LOG_LOG_TO_STDOUT            L"client.log.logToStdout"
#define CLIENT_LOG_KEEP_LOG_FILE_OPEN       L"client.log.keepLogFileOpen"
/*@}*/

/**
  @name Diagnostic Tag keys
  @memo Base diagnostic tag offset for each interface.
  @doc The value for these keys controls the tags numbers that must be
  turned on to get diagnostic output.
 */
/*@{*/
#define CLIENT_HW_DIAG_BASE                 L"client.hw.diagLogBase"
#define CLIENT_AUDIO_DIAG_BASE              L"client.audio.diagLogBase"
#define CLIENT_CACHE_DIAG_BASE              L"client.cache.diagLogBase"
#define CLIENT_INET_DIAG_BASE               L"client.inet.diagLogBase"
#define CLIENT_JSI_DIAG_BASE                L"client.jsi.diagLogBase"
#define CLIENT_OBJ_DIAG_BASE                L"client.object.diagLogBase"
#define CLIENT_PROMPT_DIAG_BASE             L"client.prompt.diagLogBase"
#define CLIENT_PROMPT_BASE_URL              L"client.prompt.baseUrl"
#define CLIENT_REC_DIAG_BASE                L"client.rec.diagLogBase"
#define CLIENT_TEL_DIAG_BASE                L"client.tel.diagLogBase"
#define CLIENT_VXI_DIAG_BASE                L"client.vxi.diagLogBase"
#define CLIENT_CLIENT_DIAG_BASE             L"client.client.diagLogBase"
/*@}*/

#endif /* include guard */

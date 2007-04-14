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
 * SB Configuration File Parser - Header
 *
 **********************************************************************
 */

#ifndef _CONFIGFILE_H
#define _CONFIGFILE_H

#include <VXIplatform.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Constants for configuration file parsing
 */
#define CONFIG_FILE_SEPARATORS " \t\r\n"
#define CONFIG_FILE_ENV_VAR_BEGIN_ID "$("
#define CONFIG_FILE_ENV_VAR_END_ID ")"
#define CONFIG_FILE_COMMENT_ID '#'

/**
 * Parse a SBclient configuration file
 *
 * @param configArgs     Properties read from the configuration file are
 *                       stored in this array using the corresponding keys
 * @param fileName       Name and path of the configuration file to be
 *                       parsed. If NULL, the default configuration file
 *                       name defined above will be used.
 *
 * @result VXIplatformResult 0 on success
 */
VXIplatformResult ParseConfigFile (VXIMap **configArgs, const char *fileName);

/**
 * Parse a SBclient configuration line
 *
 * @param buffer         The configuration file line to be parsed, is modified
 *                       during the parse
 * @param configArgs     The property read from the configuration line is
 *                       stored in this array using the corresponding key
 *
 * @result VXIplatformResult 0 on success
 */
VXIplatformResult ParseConfigLine(char* buffer, VXIMap *configArgs);

#ifdef __cplusplus
}
#endif

#endif

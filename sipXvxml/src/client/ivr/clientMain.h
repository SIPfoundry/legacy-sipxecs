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

#ifndef _CLIENTMAIN_H
#define _CLIENTMAIN_H

//#ifdef __cplusplus
//extern "C" {
//#endif

#include "VXIplatform.h"

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
int main (int argc, char *argv[]);

VXIplatformResult VXICleanUpCall (void *plistener,
                    const char *callId,
                    int VXISessionEnded);

VXIplatformResult VXIProcessUrl (void *listener,
                   int channelNum,
                   const char *callId,
                   const char *from,
                   const char *to,
                   const char *url);

int VXIProcessRequest1 (CallManager* pCallMgr,
                        const char* callId,
                        const char *url, 
                        const char* cfg);

int VXIProcessRequest (VXIplatform *platform,
                       int channelNum,
                       const char* callId,
                       const char *from,
                       const char *to,
                       const char *url, 
                       const char* cfg);



//#ifdef __cplusplus
//}
//#endif

#endif //_CLIENTMAIN_H

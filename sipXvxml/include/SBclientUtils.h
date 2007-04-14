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
 * Utility functions used by SBclient.c and OSBclient.c
 *
 ************************************************************************
 */

#ifndef _SBCLIENT_UTILS_H
#define _SBCLIENT_UTILS_H

#include <VXIlog.h>          /* For the VXIlog interface */
#include <VXIplatform.h>     /* For VXIplatformResult */
#include <VXIinterpreter.h>  /* For VXIinterpreterResult */

#ifdef __cplusplus
extern "C" {
#endif

/* Error and diagnostic logging, must be defined by each VXIplatform
   implementation */
VXIPLATFORM_API VXIlogResult 
SBclientError(VXIplatform *platform, const VXIchar *moduleName,
	      VXIunsigned errorID, const VXIchar *format, ...);
VXIPLATFORM_API VXIlogResult 
SBclientDiag(VXIplatform *platform, VXIunsigned tag, 
	     const VXIchar *subtag, const VXIchar *format, ...);
VXIPLATFORM_API VXIplatformResult 
SBclientConfigureDiagnosticTags(const VXIMap     *configArgs,
				VXIplatform      *platform);

/* Configuration parameter retrieval */
VXIPLATFORM_API VXIplatformResult 
SBclientGetParamStr(const VXIMap     *configArgs,
		    const VXIchar    *paramName,
		    const VXIchar   **paramValue,
		    VXIbool           required);
VXIPLATFORM_API VXIplatformResult 
SBclientGetParamInt(const VXIMap     *configArgs,
		    const VXIchar    *paramName,
		    VXIint32         *paramValue,
		    VXIbool           required);
VXIPLATFORM_API VXIplatformResult 
SBclientGetParamBool(const VXIMap     *configArgs,
		     const VXIchar    *paramName,
		     VXIbool          *paramValue,
		     VXIbool           required);
VXIPLATFORM_API VXIplatformResult 
SBclientGetParamMap(const VXIMap     *configArgs,
		    const VXIchar    *paramName,
		    const VXIMap    **paramValue,
		    VXIbool           required);
VXIPLATFORM_API VXIplatformResult 
SBclientGetParamVector(const VXIMap     *configArgs,
		       const VXIchar    *paramName,
		       const VXIVector **paramValue,
		       VXIbool           required);

/* Convert VXIinterpreter result codes */
VXIPLATFORM_API VXIplatformResult 
SBclientConvertInterpreterResult(VXIinterpreterResult result);

/* Diagnostic tags */
#define CLIENT_API_TAG         (gblDiagLogBase + 0)
#define CLIENT_COMPONENT_TAG   (gblDiagLogBase + 1)
#define CLIENT_GEN_TAG         (gblDiagLogBase + 2)

/* Convenience macros */
#define CLIENT_CHECK_RESULT(_func, _res) \
    if ((VXIint)(_res) != 0) { \
        OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, "%s failed, error code %i, file %s, line %i\n",\
                (_func), (_res), __FILE__, __LINE__); \
        OsSysLog::flush(); \
        return VXIplatform_RESULT_FATAL_ERROR; \
    }

#define CLIENT_CHECK_RESULT_NO_EXIT(_func, _res) \
   if ((VXIint)(_res) != 0) { \
        OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, "%s failed, error code %i, file %s, line %i\n",\
                (_func), (_res), __FILE__, __LINE__); \
        OsSysLog::flush(); \
   }

#define CLIENT_CHECK_MEMALLOC( _buf, _msg ) \
    if( _buf == NULL ) { \
        OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, "Failed to allocate memory for %s\n", _msg); \
        OsSysLog::flush(); \
        return VXIplatform_RESULT_OUT_OF_MEMORY; \
    }

#define CLIENT_LOG_IMPLEMENTATION(_name, _intf)                           \
    SBclientDiag(newPlatform, CLIENT_COMPONENT_TAG, NULL,                 \
                 _name L" implementation: %s, interface version %d.%d",   \
                 _intf->GetImplementationName(),                          \
                 VXI_MAJOR_VERSION(_intf->GetVersion()),                  \
                 VXI_MINOR_VERSION(_intf->GetVersion()));

#ifdef __cplusplus
}
#endif

#endif /* include guard */

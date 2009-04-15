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
#include <wchar.h>

#define VXIPLATFORM_EXPORTS
#include <SBclientUtils.h>
#include <SBclientConfig.h>

// -----1=0-------2=0-------3=0-------4=0-------5=0-------6=0-------7=0-------8


/**
 * Get the value of a string typed configuration parameter from the
 * passed configuration structure.  
 */
VXIPLATFORM_API VXIplatformResult 
SBclientGetParamStr(const VXIMap    *configArgs,
		    const VXIchar   *paramName,
		    const VXIchar  **paramValue,
		    VXIbool          required)
{
  VXIplatformResult rc = VXIplatform_RESULT_SUCCESS;

  if ((!configArgs) || (!paramName) || (!paramName[0]) || (!paramValue)) {
    rc = VXIplatform_RESULT_INVALID_ARGUMENT;
  } else {
    const VXIValue *value = VXIMapGetProperty(configArgs, paramName);
    if (!value) {
      rc = VXIplatform_RESULT_FAILURE;
      if (required)
        fprintf(stderr, 
                "ERROR: Required configuration parameter not set, %ls\n",
                paramName);
    } else if (VXIValueGetType(value) != VALUE_STRING) {
      rc = VXIplatform_RESULT_NON_FATAL_ERROR;
      fprintf(stderr, "ERROR: Invalid type for configuration parameter, %ls, "
              "VXIString required\n", paramName);
    } else {
      *paramValue = VXIStringCStr((const VXIString *) value);
    }
  }

  return rc;
}


/**
 * Get the value of an integer typed configuration parameter from the
 * passed configuration structure.  
 */
VXIPLATFORM_API VXIplatformResult 
SBclientGetParamInt(const VXIMap    *configArgs,
		    const VXIchar   *paramName,
		    VXIint32        *paramValue,
		    VXIbool          required)
{
  VXIplatformResult rc = VXIplatform_RESULT_SUCCESS;

  if ((!configArgs) || (!paramName) || (!paramName[0]) || (!paramValue)) {
    rc = VXIplatform_RESULT_INVALID_ARGUMENT;
  } else {
    const VXIValue *value = VXIMapGetProperty(configArgs, paramName);
    if (!value) {
      rc = VXIplatform_RESULT_FAILURE;
      if (required)
        fprintf(stderr, 
                "ERROR: Required configuration parameter not set, %ls\n",
                paramName);
    } else if (VXIValueGetType(value) != VALUE_INTEGER) {
      rc = VXIplatform_RESULT_NON_FATAL_ERROR;
      fprintf(stderr, "ERROR: Invalid type for configuration parameter, %ls, "
              "VXIInteger required\n", paramName);
    } else {
      *paramValue = VXIIntegerValue((const VXIInteger *) value);
    }
  }

  return rc;
}


/**
 * Get the value of a boolean typed configuration parameter from the
 * passed configuration structure.  
 */
VXIPLATFORM_API VXIplatformResult 
SBclientGetParamBool(const VXIMap    *configArgs,
		     const VXIchar   *paramName,
		     VXIbool         *paramValue,
		     VXIbool          required)
{
  VXIplatformResult rc = VXIplatform_RESULT_SUCCESS;

  if ((!configArgs) || (!paramName) || (!paramName[0]) || (!paramValue)) {
    rc = VXIplatform_RESULT_INVALID_ARGUMENT;
  } else {
    const VXIValue *value = VXIMapGetProperty(configArgs, paramName);
    if (!value) {
      rc = VXIplatform_RESULT_FAILURE;
      if (required)
        fprintf(stderr, 
                "ERROR: Required configuration parameter not set, %ls\n",
                paramName);
    } else if (VXIValueGetType(value) != VALUE_INTEGER) {
      rc = VXIplatform_RESULT_NON_FATAL_ERROR;
      fprintf(stderr, "ERROR: Invalid type for configuration parameter, %ls, "
              "VXIInteger set to 0 or 1 required\n", paramName);
    } else if (VXIIntegerValue((const VXIInteger *) value) == 0) {
      *paramValue = FALSE;
    } else {
      *paramValue = TRUE;
    }
  }

  return rc;
}


/**
 * Get the value of a map typed configuration parameter from the
 * passed configuration structure.  
 */
VXIPLATFORM_API VXIplatformResult 
SBclientGetParamMap(const VXIMap     *configArgs,
		    const VXIchar    *paramName,
		    const VXIMap    **paramValue,
		    VXIbool           required)
{
  VXIplatformResult rc = VXIplatform_RESULT_SUCCESS;

  if ((!configArgs) || (!paramName) || (!paramName[0]) || (!paramValue)) {
    rc = VXIplatform_RESULT_INVALID_ARGUMENT;
  } else {
    const VXIValue *value = VXIMapGetProperty(configArgs, paramName);
    if (!value) {
      rc = VXIplatform_RESULT_FAILURE;
      if (required)
        fprintf(stderr, 
                "ERROR: Required configuration parameter not set, %ls\n",
                paramName);
    } else if (VXIValueGetType(value) != VALUE_MAP) {
      rc = VXIplatform_RESULT_NON_FATAL_ERROR;
      fprintf(stderr, "ERROR: Invalid type for configuration parameter, %ls, "
              "VXIMap required\n", paramName);
    } else {
      *paramValue = (const VXIMap *) value;
    }
  }

  return rc;
}


/**
 * Get the value of a vector of typed configuration parameter from the
 * passed configuration structure.  
 */
VXIPLATFORM_API VXIplatformResult 
SBclientGetParamVector(const VXIMap     *configArgs,
		       const VXIchar    *paramName,
		       const VXIVector **paramValue,
		       VXIbool           required)
{
  VXIplatformResult rc = VXIplatform_RESULT_SUCCESS;

  if ((!configArgs) || (!paramName) || (!paramName[0]) || (!paramValue)) {
    rc = VXIplatform_RESULT_INVALID_ARGUMENT;
  } else {
    const VXIValue *value = VXIMapGetProperty(configArgs, paramName);
    if (!value) {
      rc = VXIplatform_RESULT_FAILURE;
      if (required)
        fprintf(stderr, 
                "ERROR: Required configuration parameter not set, %ls\n",
                paramName);
    } else if (VXIValueGetType(value) != VALUE_VECTOR) {
      rc = VXIplatform_RESULT_NON_FATAL_ERROR;
      fprintf(stderr, "ERROR: Invalid type for configuration parameter, %ls, "
              "VXIVector required\n", paramName);
    } else {
      *paramValue = (const VXIVector *) value;
    }
  }

  return rc;
}


/**
 * Convert VXIinterpreterResult codes to VXIplatformResult codes
 */
VXIPLATFORM_API VXIplatformResult
SBclientConvertInterpreterResult(VXIinterpreterResult result)
{
  VXIplatformResult rc;

  switch (result) {
  case VXIinterp_RESULT_FATAL_ERROR:
    rc = VXIplatform_RESULT_INTERPRETER_ERROR;
    break;
  case VXIinterp_RESULT_IO_ERROR:
    rc = VXIplatform_RESULT_IO_ERROR;
    break;
  case VXIinterp_RESULT_OUT_OF_MEMORY:
    rc = VXIplatform_RESULT_OUT_OF_MEMORY;
    break;
  case VXIinterp_RESULT_SYSTEM_ERROR:
    rc = VXIplatform_RESULT_SYSTEM_ERROR;
    break;
  case VXIinterp_RESULT_PLATFORM_ERROR:
    rc = VXIplatform_RESULT_PLATFORM_ERROR;
    break;
  case VXIinterp_RESULT_BUFFER_TOO_SMALL:
    rc = VXIplatform_RESULT_BUFFER_TOO_SMALL;
    break;
  case VXIinterp_RESULT_INVALID_PROP_NAME:
    rc = VXIplatform_RESULT_INVALID_PROP_NAME;
    break;
  case VXIinterp_RESULT_INVALID_PROP_VALUE:
    rc = VXIplatform_RESULT_INVALID_PROP_VALUE;
    break;
  case VXIinterp_RESULT_INVALID_ARGUMENT:
    rc = VXIplatform_RESULT_INVALID_ARGUMENT;
    break;
  case VXIinterp_RESULT_SUCCESS:
    rc = VXIplatform_RESULT_SUCCESS;
    break;
  case VXIinterp_RESULT_FAILURE:
    rc = VXIplatform_RESULT_FAILURE;
    break;
  case VXIinterp_RESULT_NON_FATAL_ERROR:
    rc = VXIplatform_RESULT_NON_FATAL_ERROR;
    break;
  case VXIinterp_RESULT_NOT_FOUND:
  case VXIinterp_RESULT_FETCH_TIMEOUT:
  case VXIinterp_RESULT_FETCH_ERROR:
  case VXIinterp_RESULT_INVALID_DOCUMENT:
  case VXIinterp_RESULT_SYNTAX_ERROR:
  case VXIinterp_RESULT_UNCAUGHT_FATAL_EVENT:
  case VXIinterp_RESULT_SCRIPT_SYNTAX_ERROR:
  case VXIinterp_RESULT_SCRIPT_EXCEPTION:
    rc = VXIplatform_RESULT_NON_FATAL_ERROR;
    break;
  case VXIinterp_RESULT_UNSUPPORTED:
    rc = VXIplatform_RESULT_UNSUPPORTED;
    break;
  default:
    if (result > 0)
      rc = VXIplatform_RESULT_NON_FATAL_ERROR;
    else
      rc = VXIplatform_RESULT_INTERPRETER_ERROR;
  }
  
  return rc;
}

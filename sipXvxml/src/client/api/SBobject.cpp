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
 * Implementation of the SBobject functions defined in SBobject.h, see
 * that header for details
 *
 ************************************************************************
 */

static const char *rcsid = 0 ? (char *) &rcsid :
"";

// -----1=0-------2=0-------3=0-------4=0-------5=0-------6=0-------7=0-------8

#include <string.h>                     // For memset( )

#include "VXIlog.h"                     // For VXIlog interface
#include "OSBobject.h"                  // Header for these functions
#ifdef OPENVXI
#define SBOBJECT_API extern "C"
typedef OSBobjectResources SBobjectResources;
#else
#define SBOBJECT_EXPORTS
#include "SBobject.h"                   // Second header for these functions
#endif

#ifndef MODULE_PREFIX
#define MODULE_PREFIX  COMPANY_DOMAIN L"."
#endif
#ifdef OPENVXI
static const VXIchar MODULE_SBOBJECT[] = MODULE_PREFIX L"OSBobject";
static const VXIchar SBOBJECT_IMPLEMENTATION_NAME[] = 
                       COMPANY_DOMAIN L".OSBobject";
#else
static const VXIchar MODULE_SBOBJECT[] = MODULE_PREFIX L"SBobject";
static const VXIchar SBOBJECT_IMPLEMENTATION_NAME[] = 
                       COMPANY_DOMAIN L".SBobject";
#endif

// Global variable to track whether this is initialized
static bool gblInitialized = false;

// Global diagnostic logging base
static VXIunsigned gblDiagLogBase;

// Diagnostic logging tags
static const VXIunsigned LOG_API = 0;

// SBobject interface, "inherits" from VXIobjectInterface
typedef struct SBobjectInterface
{
  // Base interface, must be the first member
  VXIobjectInterface  object;

  // Resources for this channel
  SBobjectResources  *resources;

  // Diagnostic log base for this channel
  VXIunsigned         diagLogBase;

} SBobjectInterface;

// Convenience macro
#define GET_SBOBJECT(pThis, sbObject, log, rc) \
  VXIobjResult rc = VXIobj_RESULT_SUCCESS; \
  SBobjectInterface *sbObject = (SBobjectInterface *) pThis; \
  if ( ! sbObject ) { rc = VXIobj_RESULT_INVALID_ARGUMENT; return rc; } \
  VXIlogInterface *log = sbObject->resources->log;


// Forward declarations
extern "C" {
  VXIint32 SBobjectGetVersion(void);
  const VXIchar* SBobjectGetImplementationName(void);
  VXIobjResult SBobjectExecute(struct VXIobjectInterface *pThis,
			       const VXIMap              *properties,
			       const VXIMap              *parameters,
			       VXIValue                 **result);
  VXIobjResult SBobjectValidate(struct VXIobjectInterface *pThis,
				const VXIMap              *properties,
				const VXIMap              *parameters);
};


// -----1=0-------2=0-------3=0-------4=0-------5=0-------6=0-------7=0-------8

/**
 * Sample diagnostic logging object
 *
 * @param parameters    [IN]  See description in SBobjectExecute() 
 *                            or SBobjectValidate()
 * @param execute       [IN]  Specifies whether the object should be
 *                            executed (true) or simply validated (false)
 * @param result        [OUT] See description in SBobjectExecute() 
 *                            or SBobjectValidate()
 *
 * @result VXIobj_RESULT_SUCCESS on success
 */
static VXIobjResult 
ProcessComSpeechworksDiagObject (struct VXIobjectInterface *pThis,
                                 const VXIMap    *parameters,
                                 VXIbool          execute,
			                     VXIValue       **result)
{
  static const wchar_t func[] = L"ProcessComSpeechworksDiagObject";
  GET_SBOBJECT (pThis, sbObject, log, rc);
  VXIMap *resultObj = NULL;

  if(execute) {
    if(result == NULL) return VXIobj_RESULT_INVALID_ARGUMENT;

    // Create the result object
    resultObj = VXIMapCreate();
    if(resultObj == NULL) return VXIobj_RESULT_OUT_OF_MEMORY;
    *result = reinterpret_cast<VXIValue *>(resultObj);

    // Add a status field to the result object, initialized to 'failure'
    VXIMapSetProperty(resultObj, L"status", 
             reinterpret_cast<VXIValue *>(VXIStringCreate(L"failure")));
  }

  if(parameters == NULL) return VXIobj_RESULT_INVALID_ARGUMENT;

  // Get the tag ID
  const VXIValue *val = VXIMapGetProperty(parameters, L"tag");
  if(val == NULL) return VXIobj_RESULT_INVALID_PROP_VALUE;
  const VXIchar *tagStr = VXIStringCStr((VXIString *)val);
  if((tagStr == NULL) || (tagStr[0] == L'\0'))
    return VXIobj_RESULT_INVALID_PROP_VALUE;
  VXIint tag = ::wcstol(tagStr, NULL, 10);

  // Get the message string
  const VXIchar *messageStr;
  val = VXIMapGetProperty(parameters, L"message");
  if(val == NULL) return VXIobj_RESULT_INVALID_PROP_VALUE;

  // Check whether the message was sent in "data" or "ref" format.
  // If it is "ref", we need to retrieve it in the embedded map.
  if(VXIValueGetType(val) == VALUE_MAP) {
    const VXIValue *val2 = VXIMapGetProperty((const VXIMap *)val, 
                                             OBJECT_VALUE);
    messageStr = VXIStringCStr((VXIString *)val2);
  }
  else {
    messageStr = VXIStringCStr((VXIString *)val);
  }

  if(execute) {
    // Print a diagnostic message using the retrieved arguments.
    // To see this message, you must enable client.log.diagTag.xxx
    // in your SBclient configuration file, where 'xxx' is the 
    // value of client.object.diagLogBase defined in the same file.
    VXIlogResult rc = log->Diagnostic (log, sbObject->diagLogBase + tag,
                                       func, L"%s", messageStr);

    // Set the result object's status field to 'success' if needed
    if(rc == VXIlog_RESULT_SUCCESS) {
      VXIMapSetProperty(resultObj, L"status",
               reinterpret_cast<VXIValue *>(VXIStringCreate(L"success")));
    }
  }

  return VXIobj_RESULT_SUCCESS;
}

/**
 * Sample object echoing back all attributes
 *
 * @param properties    [IN]  See description in SBobjectExecute() 
 *                            or SBobjectValidate()
 * @param execute       [IN]  Specifies whether the object should be
 *                            executed (true) or simply validated (false)
 * @param result        [OUT] See description in SBobjectExecute() 
 *                            or SBobjectValidate()
 *
 * @result VXIobj_RESULT_SUCCESS on success
 */
static VXIobjResult 
ProcessComSpeechworksEchoObject (struct VXIobjectInterface *pThis,
                                 const VXIMap              *properties,
                                 VXIbool                    execute,
				 VXIValue                 **result)
{
  // UNUSED VARIABLE static const wchar_t func[] = L"ProcessComSpeechworksEchoObject";
  // UNUSED VARIABLE GET_SBOBJECT (pThis, sbObject, log, rc);
  VXIMap *resultObj = NULL;

  if(execute) {
    if(result == NULL) return VXIobj_RESULT_INVALID_ARGUMENT;

    // Create the result object
    resultObj = VXIMapCreate();
    if(resultObj == NULL) return VXIobj_RESULT_OUT_OF_MEMORY;
    *result = reinterpret_cast<VXIValue *>(resultObj);
  }
  
  if(properties == NULL) return VXIobj_RESULT_INVALID_ARGUMENT;

  if(execute) {
    // Simply add the input properties to the result object
    VXIMapSetProperty(resultObj, L"attributes", 
		      VXIValueClone((VXIValue *)properties));
  }

  return VXIobj_RESULT_SUCCESS;
}

// -----1=0-------2=0-------3=0-------4=0-------5=0-------6=0-------7=0-------8

/**
 * Global platform initialization of SBobject
 *
 * @param log            VXI Logging interface used for error/diagnostic 
 *                       logging, only used for the duration of this 
 *                       function call
 * @param  diagLogBase   Base tag number for diagnostic logging purposes.
 *                       All diagnostic tags for SBobject will start at this
 *                       ID and increase upwards.
 *
 * @result VXIobj_RESULT_SUCCESS on success
 */
SBOBJECT_API VXIobjResult SBobjectInit (VXIlogInterface  *log,
					VXIunsigned       diagLogBase)
{
  static const wchar_t func[] = L"SBobjectInit";
  if ( log )
    log->Diagnostic (log, diagLogBase + LOG_API, func, 
		     L"entering: 0x%p, %u", log, diagLogBase);

  gblDiagLogBase = diagLogBase;

  VXIobjResult rc = VXIobj_RESULT_SUCCESS;
  if (gblInitialized == true) {
    rc = VXIobj_RESULT_FATAL_ERROR;
    if ( log )
      log->Diagnostic (log, gblDiagLogBase + LOG_API, func, 
		       L"exiting: returned %d", rc);
    return rc;
  }

  if ( ! log ) {
    rc = VXIobj_RESULT_INVALID_ARGUMENT;
    if ( log )
      log->Diagnostic (log, gblDiagLogBase + LOG_API, func,
		       L"exiting: returned %d", rc);
    return rc;
  }

  // Return
  gblInitialized = true;

  log->Diagnostic (log, gblDiagLogBase + LOG_API, func,
		   L"exiting: returned %d", rc);
  return rc;
}


OSBOBJECT_API VXIobjResult OSBobjectInit (VXIlogInterface  *log,
					  VXIunsigned       diagLogBase)
{
  return SBobjectInit (log, diagLogBase);
}


/**
 * Global platform shutdown of SBobject
 *
 * @param log    VXI Logging interface used for error/diagnostic logging,
 *               only used for the duration of this function call
 *
 * @result VXIobj_RESULT_SUCCESS on success
 */
SBOBJECT_API VXIobjResult SBobjectShutDown (VXIlogInterface  *log)
{
  static const wchar_t func[] = L"SBobjectShutDown";
  if ( log ) log->Diagnostic (log, gblDiagLogBase + LOG_API, func,
			      L"entering: 0x%p", log);

  VXIobjResult rc = VXIobj_RESULT_SUCCESS;
  if (gblInitialized == false) {
    rc = VXIobj_RESULT_FATAL_ERROR;
    if ( log )
      log->Diagnostic (log, gblDiagLogBase + LOG_API, func,
		       L"exiting: returned %d", rc);
    return rc;
  }

  if ( ! log )
    return VXIobj_RESULT_INVALID_ARGUMENT;

  gblInitialized = false;

  log->Diagnostic (log, gblDiagLogBase + LOG_API, func, 
		   L"exiting: returned %d", rc);
  return rc;
}


OSBOBJECT_API VXIobjResult OSBobjectShutDown (VXIlogInterface  *log)
{
  return SBobjectShutDown (log);
}


/**
 * Create a new object service handle
 *
 * @param resources  A pointer to a structure containing all the interfaces
 *                   that may be required by the object resource
 *
 * @result VXIobj_RESULT_SUCCESS on success 
 */
SBOBJECT_API 
VXIobjResult SBobjectCreateResource(SBobjectResources   *resources,
				    VXIobjectInterface **object)
{
  static const wchar_t func[] = L"SBobjectCreateResource";
  VXIlogInterface *log = NULL;
  if (( resources ) && ( resources->log )) {
    log = resources->log;
    log->Diagnostic (log, gblDiagLogBase + LOG_API, func, 
		     L"entering: 0x%p, 0x%p", resources, object);
  }

  VXIobjResult rc = VXIobj_RESULT_SUCCESS;
  if (gblInitialized == false) {
    rc = VXIobj_RESULT_FATAL_ERROR;
    if ( log )
      log->Diagnostic (log, gblDiagLogBase + LOG_API, func, 
		       L"exiting: returned %d, 0x%p", 
		       rc, (object ? *object : NULL));
    return rc;
  }

  if (( ! log ) || ( ! resources ) || ( ! object )) {
    rc = VXIobj_RESULT_INVALID_ARGUMENT;
    if ( log )
      log->Diagnostic (log, gblDiagLogBase + LOG_API, func, 
		       L"exiting: returned %d, 0x%p", 
		       rc, (object ? *object : NULL));
    return rc;
  }

  *object = NULL;

  // Get a new interface instance
  SBobjectInterface *newObject = new SBobjectInterface;
  if (newObject == false) {
    rc = VXIobj_RESULT_OUT_OF_MEMORY;
    log->Diagnostic (log, gblDiagLogBase + LOG_API, func, 
		     L"exiting: returned %d, 0x%p", rc, *object);
    return rc;
  }
  memset (newObject, 0, sizeof (SBobjectInterface));

  // Initialize the function pointers
  newObject->object.GetVersion            = SBobjectGetVersion;
  newObject->object.GetImplementationName = SBobjectGetImplementationName;
  newObject->object.Execute               = SBobjectExecute;
  newObject->object.Validate              = SBobjectValidate;

  // Initialize data members
  newObject->resources = resources;
  newObject->diagLogBase = gblDiagLogBase;

  // Return the object
  if ( rc != VXIobj_RESULT_SUCCESS ) {
    if ( newObject )
      delete newObject;
    newObject = NULL;
  } else {
    *object = &(newObject->object);
  }

  log->Diagnostic (log, gblDiagLogBase + LOG_API, func,
		   L"exiting: returned %d, 0x%p", rc, *object);
  return rc;
}


OSBOBJECT_API 
VXIobjResult OSBobjectCreateResource(OSBobjectResources  *resources,
				     VXIobjectInterface **object)
{
  return SBobjectCreateResource ((SBobjectResources *) resources, object);
}


/**
 * Destroy the interface and free internal resources. Once this is
 *  called, the resource interfaces passed to SBobjectCreateResource( )
 *  may be released as well.
 *
 * @result VXIobj_RESULT_SUCCESS on success 
 */
SBOBJECT_API 
VXIobjResult SBobjectDestroyResource(VXIobjectInterface **object)
{
  VXIobjResult rc = VXIobj_RESULT_SUCCESS;
  static const wchar_t func[] = L"SBobjectDestroyResource";
  // Can't log yet, don't have a log handle

  if (gblInitialized == false)
    return VXIobj_RESULT_FATAL_ERROR;

  if ((object == NULL) || (*object == NULL))
    return VXIobj_RESULT_INVALID_ARGUMENT;

  // Get the real underlying interface
  SBobjectInterface *sbObject = (SBobjectInterface *) *object;

  VXIlogInterface *log = sbObject->resources->log;
  log->Diagnostic (log, gblDiagLogBase + LOG_API, func,
		   L"entering: 0x%p (0x%p)", object, *object);

  // Delete the object
  delete sbObject;
  sbObject = NULL;
  *object = NULL;

  log->Diagnostic (log, gblDiagLogBase + LOG_API, func,
		   L"exiting: returned %d", rc);
  return rc;
}


OSBOBJECT_API 
VXIobjResult OSBobjectDestroyResource(VXIobjectInterface **object)
{
  return SBobjectDestroyResource (object);
}


/**
 * @name GetVersion
 * @memo Get the VXI interface version implemented
 *
 * @return  VXIint32 for the version number. The high high word is 
 *          the major version number, the low word is the minor version 
 *          number, using the native CPU/OS byte order. The current
 *          version is VXI_CURRENT_VERSION as defined in VXItypes.h.
 */ 
extern "C"
VXIint32 SBobjectGetVersion(void)
{
  return VXI_CURRENT_VERSION;
}


/**
 * @name GetImplementationName
 * @memo Get the name of the implementation
 *
 * @return  Implementation defined string that must be different from
 *          all other implementations. The recommended name is one
 *          where the interface name is prefixed by the implementator's
 *          Internet address in reverse order, such as com.xyz.rec for
 *          VXIobject from xyz.com. This is similar to how VoiceXML 1.0
 *          recommends defining application specific error types.
 */
extern "C"
const VXIchar* SBobjectGetImplementationName(void)
{
  return SBOBJECT_IMPLEMENTATION_NAME;
}


/**
 * Execute an object
 *
 * @param properties  [IN] Map containing properties and attributes for
 *                      the <object> as specified above.
 * @param parameters  [IN] Map containing parameters for the <object> as
 *                      specified by the VoiceXML <param> tag. The keys
 *                      of the map correspond to the parameter name ("name"
 *                      attribute) while the value of each key corresponds
 *                      to a VXIValue based type.
 *
 *                      For each parameter, any ECMAScript expressions are
 *                      evaluated by the interpreter. Then if the "valuetype"
 *                      attribute is set to "ref" the parameter value is
 *                      packaged into a VXIMap with three properties:
 *
 *                      OBJECT_VALUE:       actual parameter value
 *                      OBJECT_VALUETYPE:   "valuetype" attribute value
 *                      OBJECT_TYPE:        "type" attribute value
 *
 *                      Otherwise a primitive VXIValue based type will
 *                      be used to specify the value.
 * @param result      [OUT] Return value for the <object> execution, this
 *                      is allocated on success, the caller is responsible
 *                      for destroying the returned value by calling 
 *                      VXIValueDestroy( ). The object's field variable
 *                      will be set to this value.
 *
 * @return        VXIobj_RESULT_SUCCESS on success,
 *                VXIobj_RESULT_NON_FATAL_ERROR on error, 
 *                VXIobj_RESULT_UNSUPPORTED for unsupported object types
 *                 (this will cause interpreter to throw the correct event)
 */
extern "C"
VXIobjResult SBobjectExecute(struct VXIobjectInterface *pThis,
			     const VXIMap              *properties,
			     const VXIMap              *parameters,
			     VXIValue                 **result)
{
  static const wchar_t func[] = L"SBobjectExecute";
  GET_SBOBJECT (pThis, sbObject, log, rc);
  log->Diagnostic (log, sbObject->diagLogBase + LOG_API,
		   func, L"entering: 0x%p, 0x%p, 0x%p, 0x%p", 
		   pThis, properties, parameters, result);

  if(properties == NULL) return VXIobj_RESULT_INVALID_ARGUMENT;

  // Get the name of the object to execute
  const VXIValue *val = VXIMapGetProperty(properties, OBJECT_CLASS_ID);
  if(val == NULL) return VXIobj_RESULT_INVALID_PROP_VALUE;
  const VXIchar *classID = VXIStringCStr((VXIString *)val);

  // Handle the object
  if (::wcscmp(classID, L"com.speechworks.diag") == 0) {
    //
    // Sample diagnostic logging object.
    // 
    rc = ProcessComSpeechworksDiagObject(pThis, parameters, true, result);
    if(rc != VXIobj_RESULT_SUCCESS) rc = VXIobj_RESULT_NON_FATAL_ERROR;
  } 
  else if (::wcscmp(classID, L"com.speechworks.echo") == 0) {
    //
    // Sample object echoing back all attributes
    // 
    rc = ProcessComSpeechworksEchoObject(pThis, properties, true, result);
    if(rc != VXIobj_RESULT_SUCCESS) rc = VXIobj_RESULT_NON_FATAL_ERROR;
  } 
  else {
    //
    // Unsupported object
    //
    rc = VXIobj_RESULT_UNSUPPORTED;
  }

  log->Diagnostic (log, sbObject->diagLogBase + LOG_API,
		          func, L"exiting: returned %d", rc);
  return rc;
}


/**
 * Validate an object, performing validity checks without execution
 *
 * @param properties  [IN] Map containing properties and attributes for
 *                      the <object> as specified in the VoiceXML
 *                      specification except that "expr" and "cond" are
 *                      always omitted (are handled by the interpreter).
 * @param parameters  [IN] Map containing parameters for the <object> as
 *                      specified by the VoiceXML <param> tag. The keys
 *                      of the map correspond to the parameter name ("name"
 *                      attribute) while the value of each key corresponds
 *                      to a VXIValue based type. See Execute( ) above 
 *                      for details.
 *
 * @return        VXIobj_RESULT_SUCCESS on success,
 *                VXIobj_RESULT_NON_FATAL_ERROR on error, 
 *                VXIobj_RESULT_UNSUPPORTED for unsupported object types
 *                 (this will cause interpreter to throw the correct event)
 */
extern "C"
VXIobjResult SBobjectValidate(struct VXIobjectInterface *pThis,
			      const VXIMap              *properties,
			      const VXIMap              *parameters)
{
  static const wchar_t func[] = L"SBobjectValidate";
  GET_SBOBJECT (pThis, sbObject, log, rc);
  log->Diagnostic (log, sbObject->diagLogBase + LOG_API,
		   func, L"entering: 0x%p, 0x%p, 0x%p", 
		   pThis, properties, parameters);

  if(properties == NULL) return VXIobj_RESULT_INVALID_ARGUMENT;

  // Get the name of the object to execute
  const VXIValue *val = VXIMapGetProperty(properties, OBJECT_CLASS_ID);
  if(val == NULL) return VXIobj_RESULT_INVALID_PROP_VALUE;
  const VXIchar *classID = VXIStringCStr((VXIString *)val);

  // Handle the object
  if (::wcscmp(classID, L"com.speechworks.diag") == 0) {
    //
    // Sample diagnostic logging object.
    // 
    rc = ProcessComSpeechworksDiagObject(pThis, parameters, false, NULL);
    if(rc != VXIobj_RESULT_SUCCESS) rc = VXIobj_RESULT_NON_FATAL_ERROR;
  }
  else if (::wcscmp(classID, L"com.speechworks.echo") == 0) {
    //
    // Sample object echoing back all attributes
    // 
    rc = ProcessComSpeechworksEchoObject(pThis, properties, false, NULL);
    if(rc != VXIobj_RESULT_SUCCESS) rc = VXIobj_RESULT_NON_FATAL_ERROR;
  } 
  else {
    //
    // Unsupported object
    //
    rc = VXIobj_RESULT_UNSUPPORTED;
  }


  log->Diagnostic (log, sbObject->diagLogBase + LOG_API,
		          func, L"exiting: returned %d", rc);
  return rc;
}

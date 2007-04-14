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
 * OSBobject, implementation of the VXIobject interface as defined
 * in VXIobject.h
 * 
 ************************************************************************
 */

#ifndef _OSBOBJECT_H
#define _OSBOBJECT_H

#include "VXIobject.h"                 /* For VXIobject base interface */
#include "VXIheaderPrefix.h"

#ifdef __cplusplus
#define OSBOBJECT_API extern "C"

struct VXIlogInterface;
struct VXIinetInterface;
struct VXIjsiInterface;
struct VXIrecInterface;
struct VXIpromptInterface;
struct VXItelInterface;

extern "C" {
#else
#define OSBOBJECT_API extern

#include "VXIlog.h"
#include "VXIinet.h"
#include "VXIjsi.h"
#include "VXIrec.h"
#include "VXIprompt.h"
#include "VXItel.h"
#endif

 /** 
  * @name OSBobject
  * @memo OSBobject implementation of VXIobject
  * @doc
  * Provides an implementation of the VXIobject abstract interface for
  * VoiceXML object functionality that allows integrators to define
  * VoiceXML language extensions that can be executed by applications
  * through the VoiceXML object element.  These objects can provide
  * almost any extended functionality that is desired.
  *
  * There is one object interface per thread/line.  
  */

/*@{*/

/**
 * @name OSBobjectResources
 * @memo Structure containing all the interfaces required for Objects
 * @doc This structure must be allocated and all the pointers filled
 * with created and initialized resources before creating the Object
 * interface.
 */
typedef struct OSBobjectResources {
  /** log interface */
  VXIlogInterface    * log;
  /** Internet interface */
  VXIinetInterface   * inet; 
  /** ECMAScript interface */
  VXIjsiInterface    * jsi;    
  /** Recognizer interface */
  VXIrecInterface    * rec;  
  /** Prompt interface */
  VXIpromptInterface * prompt;
  /** Telephony interface */
  VXItelInterface    * tel;
} OSBobjectResources;


/**
 * Global platform initialization of OSBobject
 *
 * @param log            VXI Logging interface used for error/diagnostic 
 *                       logging, only used for the duration of this 
 *                       function call
 * @param  diagLogBase   Base tag number for diagnostic logging purposes.
 *                       All diagnostic tags for OSBobject will start at this
 *                       ID and increase upwards.
 *
 * @result VXIobj_RESULT_SUCCESS on success
 */
OSBOBJECT_API VXIobjResult OSBobjectInit (VXIlogInterface  *log,
					  VXIunsigned       diagLogBase);

/**
 * Global platform shutdown of OSBobject
 *
 * @param log    VXI Logging interface used for error/diagnostic logging,
 *               only used for the duration of this function call
 *
 * @result VXIobj_RESULT_SUCCESS on success
 */
OSBOBJECT_API VXIobjResult OSBobjectShutDown (VXIlogInterface  *log);

/**
 * Create a new object service handle
 *
 * @param resources  A pointer to a structure containing all the interfaces
 *                   that may be required by the object resource
 *
 * @result VXIobj_RESULT_SUCCESS on success 
 */
OSBOBJECT_API 
VXIobjResult OSBobjectCreateResource (OSBobjectResources   *resources,
				      VXIobjectInterface **object);

/**
 * Destroy the interface and free internal resources. Once this is
 *  called, the resource interfaces passed to OSBobjectCreateResource( )
 *  may be released as well.
 *
 * @result VXIobj_RESULT_SUCCESS on success 
 */
OSBOBJECT_API 
VXIobjResult OSBobjectDestroyResource (VXIobjectInterface **object);

/*@}*/

#ifdef __cplusplus
}
#endif

#include "VXIheaderSuffix.h"

#endif  /* include guard */

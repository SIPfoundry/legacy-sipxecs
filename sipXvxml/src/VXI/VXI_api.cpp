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
 * Top level API implementation and VXI class methods
 *
 ***********************************************************************/

#define VXI_EXPORTS
#include "VXI.hpp"
#include "VXIinterpreter.h"
#include "SimpleLogger.hpp"
#include "DocumentParser.hpp"
#include "VXIjsi.h"

/******************************************************************
 * Init / ShutDown
 ******************************************************************/

// VXIinterp_RESULT_SUCCESS
// VXIinterp_RESULT_FAILURE
VXI_INTERPRETER 
VXIinterpreterResult VXIinterpreterInit(VXIlogInterface  *log,
					VXIunsigned       diagLogBase)
{
  SimpleLogger::SetMessageBase(diagLogBase);

  if (!DocumentParser::Initialize())
    return VXIinterp_RESULT_FAILURE;

  return VXIinterp_RESULT_SUCCESS;
}


VXI_INTERPRETER void VXIinterpreterShutDown(VXIlogInterface  *log)
{
  DocumentParser::Deinitialize();
}


/******************************************************************
 * Interface definition
 ******************************************************************/

// This is an interesting and legal hack.  The alignment of a structure in C
// must follow that of the first member.  The result is that the Interpreter
// structure has the same alignment as VXIinterpreterInterface.  The
// implementation may be safely appended onto the end.

extern "C" {
typedef struct Interpreter {
  VXIinterpreterInterface interface;
  VXIinetInterface * inet;
  VXIjsiInterface * jsi;
  SimpleLogger * logger;
  VXIpromptInterface * prompt;
  VXIrecInterface * rec;
  VXItelInterface * tel;
  VXIobjectInterface * object;
  VXI * implementation;
} Interpreter;
}


extern "C" VXIint32 VXIinterpreterGetVersion(void)
{
  return VXI_CURRENT_VERSION;
}


extern "C" const VXIchar* VXIinterpreterGetImplementationName(void)
{
#ifndef COMPANY_DOMAIN
#define COMPANY_DOMAIN L"com.pingtel"
#endif
  static const VXIchar IMPLEMENTATION_NAME[] = 
    COMPANY_DOMAIN L".VXI";
  return IMPLEMENTATION_NAME;
}


extern "C" VXIinterpreterResult VXIinterpreterRun(VXIinterpreterInterface *x,
						  const VXIchar *name,
						  const VXIMap *sessionArgs,
						  VXIValue **result)
{
  if (x == NULL) return VXIinterp_RESULT_INVALID_ARGUMENT;
  Interpreter * interpreter = reinterpret_cast<Interpreter*>(x);

  switch(interpreter->implementation->Run(name, sessionArgs,
                                          interpreter->logger,
                                          interpreter->inet,
                                          interpreter->jsi,
                                          interpreter->rec,
                                          interpreter->prompt,
                                          interpreter->tel,
                                          interpreter->object,
                                          result))
  {
  case -2: // Fatal error
    return VXIinterp_RESULT_FATAL_ERROR;
  case -1: // Out of memory
    return VXIinterp_RESULT_OUT_OF_MEMORY;
  case  0: // Success
    return VXIinterp_RESULT_SUCCESS;
  case  1: // Infinite loop suspected.
  case  2: // ECMAScript error
    return VXIinterp_RESULT_FAILURE;
  case  3: // Invalid startup documents
    return VXIinterp_RESULT_INVALID_DOCUMENT;
  default:
    return VXIinterp_RESULT_FATAL_ERROR;
  }
}


extern "C" VXIinterpreterResult VXIinterpreterSetProperties(
                                                 VXIinterpreterInterface * x,
                                                 const VXIMap * props)
{
  if (x == NULL) return VXIinterp_RESULT_INVALID_ARGUMENT;
  Interpreter * interpreter = reinterpret_cast<Interpreter*>(x);

  // Handle the degenerate case.
  if (VXIMapNumProperties(props) == 0) return VXIinterp_RESULT_SUCCESS;

  // Walk through each property in the map.
  const VXIchar  * key;
  const VXIValue * value;
  bool error = false;
  VXIMapIterator * i = VXIMapGetFirstProperty(props, &key, &value);
  do {
    // Only strings are supported.
    if (key == NULL || VXIValueGetType(value) != VALUE_STRING) {
      error = true;
      continue;
    }

    const VXIchar * valueString
      = VXIStringCStr(reinterpret_cast<const VXIString *>(value));

    VXI * vxi = interpreter->implementation;
    vxistring keyString(key);

    // Translate property names.
    if (keyString == VXI_BEEP_AUDIO)
      error |= !(vxi->SetRuntimeProperty(VXI::BeepURI, valueString));
    else if (keyString == VXI_PLATFORM_DEFAULTS)
      error |= !(vxi->SetRuntimeProperty(VXI::PlatDefaultsURI, valueString));
    else
      error = true;

  } while (VXIMapGetNextProperty(i, &key, &value) == VXIvalue_RESULT_SUCCESS);

  VXIMapIteratorDestroy(&i);

  if (error) return VXIinterp_RESULT_FAILURE;
  return VXIinterp_RESULT_SUCCESS;
}


extern "C" VXIinterpreterResult VXIinterpreterValidate(
                                                 VXIinterpreterInterface *x,
                                                 const VXIchar *name)
{
  if (x == NULL) return VXIinterp_RESULT_INVALID_ARGUMENT;
  Interpreter * interpreter = reinterpret_cast<Interpreter*>(x);

  DocumentParser parser;
  VXIMapHolder properties;
  VXMLDocument document;
  VXIMapHolder docProperties;

  switch (parser.FetchDocument(name, properties, interpreter->inet,
                               *interpreter->logger, document, docProperties)){
  case -1: // Out of memory?
    return VXIinterp_RESULT_OUT_OF_MEMORY;
  case  0: // Success
    return VXIinterp_RESULT_SUCCESS;
  case  2: // Unable to open URI
    return VXIinterp_RESULT_NOT_FOUND;
  case  3: // Unable to read from URI
    return VXIinterp_RESULT_FETCH_ERROR;
  case  4: // Unable to parse contents of URI
    return VXIinterp_RESULT_FAILURE;
  case  1: // Invalid parameter
  default:
    break;
  }

  return VXIinterp_RESULT_FATAL_ERROR;
}


/******************************************************************
 * Resource creation & destruction
 ******************************************************************/

// VXIinterp_RESULT_OUT_OF_MEMORY
// VXIinterp_RESULT_SUCCESS
VXI_INTERPRETER
VXIinterpreterResult VXIinterpreterCreateResource(VXIresources *resources,
                                                  VXIinterpreterInterface ** v)
{
  if (resources == NULL || resources->inet == NULL || 
      resources->log == NULL || v == NULL)
    return VXIinterp_RESULT_INVALID_ARGUMENT;

  Interpreter * interpreter = new Interpreter;
  if (interpreter == NULL)
    return VXIinterp_RESULT_OUT_OF_MEMORY;

  interpreter->implementation = new VXI();
  if (interpreter->implementation == NULL) {
    delete interpreter;
    interpreter = NULL;
    return VXIinterp_RESULT_OUT_OF_MEMORY;
  }

  interpreter->logger = SimpleLogger::CreateResource(resources->log);
  if (interpreter->logger == NULL) {
    delete interpreter->implementation;
    interpreter->implementation = NULL;
    delete interpreter;
    interpreter = NULL;
    return VXIinterp_RESULT_OUT_OF_MEMORY;
  }

  interpreter->interface.GetVersion = VXIinterpreterGetVersion;
  interpreter->interface.GetImplementationName = 
    VXIinterpreterGetImplementationName;
  interpreter->interface.Run = VXIinterpreterRun;
  interpreter->interface.Validate = VXIinterpreterValidate;
  interpreter->interface.SetProperties = VXIinterpreterSetProperties;

  interpreter->inet = resources->inet;
  interpreter->jsi = resources->jsi;
  interpreter->rec = resources->rec;
  interpreter->prompt = resources->prompt;
  interpreter->tel = resources->tel;
  interpreter->object = resources->object;

  *v = reinterpret_cast<VXIinterpreterInterface*>(interpreter);
  return VXIinterp_RESULT_SUCCESS;
}


VXI_INTERPRETER
void VXIinterpreterDestroyResource(VXIinterpreterInterface ** v)
{
  if (v == NULL || *v == NULL) return;
  Interpreter * interpreter = reinterpret_cast<Interpreter*>(*v);
  delete interpreter->implementation;
  interpreter->implementation = NULL;
  SimpleLogger::DestroyResource(interpreter->logger);
  delete interpreter;
  interpreter = NULL;
  *v = NULL;
}

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
 ************************************************************************
 *
 * 
 *
 ************************************************************************
 */

#ifndef _VXILOG_H                 /* Allows multiple inclusions */
#define _VXILOG_H

#include <stdarg.h>               /* For va_list */
#include "VXItypes.h"             /* For VXIchar */

#include "VXIheaderPrefix.h"
#ifdef VXILOG_EXPORTS
#define VXILOG_API SYMBOL_EXPORT_DECL
#else
#define VXILOG_API SYMBOL_IMPORT_DECL
#endif

#define LOG_MAX_TAG 25000  /* 24k of TAG ID, range from 0-200,000 */

// #define VXILOG_EVENTS_ENABLED

#ifdef __cplusplus
extern "C" {
#endif

/**
  @name VXIlog
 
  @memo Abstract interface for logging errors, diagnostic logging, and
  event logging.
  @version 1.0 <br>		       
  @doc
  <ul>
  The VXIlog Facility supports the following types of log streams:

  <li><b> Error stream:</b><br/>
   Used to report system faults (errors) and possible system faults
   (warnings) to the system operator for diagnosis and repair. Errors
   are reported by error numbers that are mapped to text with an
   associated severity level. This mapping may be maintained in a
   separate XML document.  This allows integrators to localize and
   customize the text without code changes, and also allows integrators
   and developers to review and re-define severity levels without code
   changes. Optional attribute/value pairs are used to convey details
   about the fault, such the file or URL that is associated with the
   fault.
  </li>
 
  <li><b>Diagnostic stream: </b><br/>
   Used by developers and support staff to trace and diagnose system
   behavior. Diagnostic messages are hard-coded text since they are
   private messages that are not intended for system operator
   use. Diagnostic messages are all associated with developer defined
   "tags" for grouping, where each tag may be independently
   enabled/disabled. It is guaranteed that diagnostic logging will have
   little or no performance impact if the tag is disabled, meaning that
   developers should be generous in their insertion of diagnostic log
   messages. These will be disabled by default, but specific tags can
   then be enabled for rapid diagnosis with minimal performance impact
   when issues arise.
  </li>

  <li><b>Event stream: </b><br/>
   Summarizes normal caller activity in order to support service
   bureau billing; reporting for capacity planning, application and
   recognition monitoring, and caller behavior trending; and traces of
   individual calls for application tuning.  Events are reported by
   event numbers that are mapped to event names. Optional
   attribute/value pairs are used to convey details about the event,
   such as the base application for a new call event.
  </li>
  </ul>
<p>
  Across all streams, the log implementation is responsible for
  automatically supplying the following information in some manner
  (possibly encoded into a file name, possibly in a data header,
  possibly as part of each log message) for end consumer use:
 <ul>
    <li> timestamp
    <li> channel name and/or number
    <li> host name
    <li> application name
    <li> (Error only) error text and severity based on the error number,
         and the supplied module name
    <li> (Diagnostic only) tag name based on the tag number, and the
         supplied subtag name
    <li> (Event only) event name based on the event number
 </ul>
 <p>
  In addition, for diagnostic logging the log implementation is
  responsible for defining a mechanism for enabling/disabling messages
  on an individual tag basis without requiring a recompile for use by
  consumers of the diagnostic log. It is critical that Diagnostic( )
  is highly efficient for cases when the tag is disabled: in other
  words, the lookup for seeing if a tag (an integer) is enabled should
  be done using a simple array or some other extremely low-overhead
  mechanism. It is highly recommended that log implementations provide
  a way to enable/disable tags on-the-fly (without needing to restart
  the software), and log implementations should consider providing a
  way to enable/disable tabs on a per-channel basis (for enabling tags
  in tight loops where the performance impact can be large).
 
  <p>
  Each of the streams has fields that need to be allocated by
  developers. The rules for each follows. As background, several of
  these fields require following the rules for XML names: it must
  begin with a letter, underscore, or colon, and is followed by one or
  more of those plus digits, hyphens, or periods. However, colons must
  only be used when indicating XML namespaces, and the "vxi" and "swi"
  namespaces (such as "swi:SBprompt") are reserved for use by
  SpeechWorks International, Inc.

  <p>
  <ul>
  <li><b>Error logging:</b><br/>

    Module names (moduleName) must follow XML name rules as described
    above, and must be unique for each implementation of each VXI
    interface. <p>
 
    Error IDs (errorID) are unsigned integers that for each module
    start at 0 and increase from there, allocated by each named module
    as it sees fit. Each VXI interface implementation must provide a
    document that provides the list of error IDs and the recommended
    text (for at least one language) and severity. Severities should
    be constrained to one of three levels: "Critical - out of service"
    (affects multiple callers), "Severe - service affecting" (affects
    a single caller), "Warning - not service affecting" (not readily
    apparent to callers, or it is at least fairly likely callers will
    not notice). <p>

    Attribute names must follow XML name rules as described
    above. However, these need not be unique for each implementation
    as they are interpreted relative to the module name (and
    frequently not interpreted at all, but merely output). For
    consumer convenience the data type for each attribute name should
    never vary, although most log implementations will not enforce
    this. <p>
 </li>

 <li><b>Diagnostic logging:</b><br/>

    Tags (tagID) are unsigned integers that must be globally unique
    across the entire run-time environment in order to allow VXIlog
    implementations to have very low overhead diagnostic log
    enablement lookups (see above). The recommended mechanism for
    avoiding conflicts with components produced by other developers is
    to make it so the initialization function for your component takes
    an unsigned integer argument that is a tag ID base. Then within
    your component code, allocate locally unique tag ID numbers
    starting at zero, but offset them by the base variable prior to
    calling the VXIlog Diagostic( ) method. This way integrators of
    your component can assign a non-conflicting base as they see
    fit. <p>
 
    There are no restrictions on subtag use, as they are relative to
    the tag ID and most log implementations will merely output them as
    a prefix to the diagnostic text. <p>
  </li>
 
  <li><b>Event logging:</b><br/>

    Events (eventID) are unsigned integers that are defined by each
    developer as they see fit in coordination with other component
    developers to avoid overlaps.  Globally unique events are required
    to make it easy for event log consumers to parse the log, all
    events should be well-known and well documented. <p>
 
    Attribute names must follow XML name rules as described
    above. However, these need not be unique for each implementation
    as they are interpreted relative to the module name (and
    frequently not interpreted at all, but merely output). For
    consumer convenience the data type for each attribute name should
    never vary, although most log implementations will not enforce
    this. <p> 
  </li> </ul> */

/*@{*/

/**
 * @memo Standard VXIlog events
 *
 * @doc Standardized events that may be reported to the VXIlog
 * interface. Platform dependant events start at
 * VXIlog_EVENT_PLATFORM_DEFINED and increase from there.
 */
typedef enum VXIlogEvent {
  VXIlog_EVENT_CALL_START         = 0,
  VXIlog_EVENT_CALL_END           = 1,
  VXIlog_EVENT_LOG_ELEMENT        = 2,
  VXIlog_EVENT_SPEECH_DETECTED    = 3,
  VXIlog_EVENT_PLATFORM_DEFINED   = 10000,
} VXIlogEvent;

/**
 * @name VXIlogResult
 * @memo Result codes for interface methods
 * 
 * @doc Result codes less then zero are severe errors (likely to be
 * platform faults), those greater then zero are warnings (likely to
 * be application issues) 
 */
typedef enum VXIlogResult {
  /** Fatal error, terminate call    */
  VXIlog_RESULT_FATAL_ERROR       =  -100, 
  /** I/O error                      */
  VXIlog_RESULT_IO_ERROR           =   -8,
  /** Out of memory                  */
  VXIlog_RESULT_OUT_OF_MEMORY      =   -7, 
  /** System error, out of service   */
  VXIlog_RESULT_SYSTEM_ERROR       =   -6, 
  /** Errors from platform services  */
  VXIlog_RESULT_PLATFORM_ERROR     =   -5, 
  /** Return buffer too small        */
  VXIlog_RESULT_BUFFER_TOO_SMALL   =   -4, 
  /** Property name is not valid    */
  VXIlog_RESULT_INVALID_PROP_NAME  =   -3, 
  /** Property value is not valid   */
  VXIlog_RESULT_INVALID_PROP_VALUE =   -2, 
  /** Invalid function argument      */
  VXIlog_RESULT_INVALID_ARGUMENT   =   -1, 
  /** Success.  Note that Success is defined as 0 and that all
    critical errors are less than 0 and all non critical errors are
    greater than 0.            */
  VXIlog_RESULT_SUCCESS            =    0,
  /** Normal failure, nothing logged */
  VXIlog_RESULT_FAILURE            =    1,
  /** Non-fatal non-specific error   */
  VXIlog_RESULT_NON_FATAL_ERROR    =    2, 
  /** Operation is not supported     */
  VXIlog_RESULT_UNSUPPORTED        =  100
} VXIlogResult;


/*
** ==================================================
** VXIlogInterface Interface definition
** ==================================================
*/
/** @name VXIlogInterface
 ** @memo VXIlog interface for logging
 ** @doc The VXIlogInterface provides a set of functions which are used for
 ** logging by all the OpenVXI browser components.
 **
 */

typedef struct VXIlogInterface {
  /**
   * @name GetVersion
   * @memo Get the VXI interface version implemented
   *
   * @return  VXIint32 for the version number. The high high word is 
   *          the major version number, the low word is the minor version 
   *          number, using the native CPU/OS byte order. The current
   *          version is VXI_CURRENT_VERSION as defined in VXItypes.h.
   */ 
  VXIint32 (*GetVersion)(void);
  
  /**
   * @name GetImplementationName
   * @memo Get the name of the implementation
   *
   * @return  Implementation defined string that must be different from
   *          all other implementations. The recommended name is one
   *          where the interface name is prefixed by the implementator's
   *          Internet address in reverse order, such as com.xyz.rec for
   *          VXIrec from xyz.com. This is similar to how VoiceXML 1.0
   *          recommends defining application specific error types.
   */
  const VXIchar* (*GetImplementationName)(void);
  
  /**
   * @name Error
   * @memo Log an error
   * @doc Basic error reporting mechanism.  Errors are reported by
   * moduleName, error number, a format, and a varargs argument list. <p>
   *
   * IMPORTANT: Error details are not free-form, they must be passed
   *    as a succession of key-value pairs, i.e. a string key followed by
   *    a value. For example, this format string and arguments is
   *    correct:<p>
   *
   *    L"%s%i%s%s", L"key1", 911, L"key2", L"value2" <p>
   *
   *    While this one is incorrect (second key missing):<br>
   *
   *    L"%s%i%f", L"key1", 911, (float)22 / 7<p>
   *
   *    Keys must always be specified by a %s, and the key names must
   *    follow the rules for XML names as summarized at the top of
   *    this header. Values may be specified by the ANSI C defined
   *    format parameters for printf( ), including the ability to
   *    control string widths, number of decimals to output, padding
   *    etc. There are no restrictions on the variable values, it is
   *    the responsibility of the logging system to escape the
   *    variable values if required by the final output stream (such
   *    as output via XML).<p>
   *
   *    NOTE: Do NOT use %C and %S in the format string for inserting
   *    narrow character buffers (char and char *) as supported by
   *    some compilers, as this is not portable and may result in
   *    system crashes on some UNIX variants if the VXIlog
   *    implementation uses the compiler supplied printf( ) family of
   *    functions for handling these variable argument lists.
   *
   * @param  moduleName   [IN] Name of the software module that is 
   *                      outputting the error. See the top of this file
   *                      for moduleName allocation rules.
   * @param  errorID      [IN] Error number to log, this is mapped to 
   *                      localized error text that is displayed to the 
   *                      system operator that has an associated severity
   *                      level. It is CRITICAL that this provides primary, 
   *                      specific, actionable information to the system 
   *                      operator, with attribute/value pairs only used to 
   *                      provide details. See the top of this file for 
   *                      errorID allocation rules.
   * @param  format       [IN] Format string as passed to wprintf( ) (the
   *                      wchar_t version of printf( ) as defined by the
   *                      ANSI C standard) for outputting optional error
   *                      details. This is followed by a variable list of
   *                      arguments supplying variables for insertion into the 
   *                      format string, also as passed to wprintf( ).
   *
   * @param  ...          [IN] Arguments matching the error details format
   *                      specified above.
   *
   * @return VXIlog_RESULT_SUCCESS on success 
   */
  VXIlogResult (*Error)(struct VXIlogInterface* pThis,
			const VXIchar*          moduleName,
			VXIunsigned             errorID,
			const VXIchar*          format,
			...);
  
  /**
   * @name VError
   * @memo Log an error (va_list variant)
   *
   * @doc
   * Same as <a href="Error.html"> Error</a>, but a va_list is supplied as
   * an argument instead of "..." in order to make it easy to layer
   * convenience functions/classes for logging on top of this
   * interface.
   *  
   * @return VXIlog_RESULT_SUCCESS on success
   */
  VXIlogResult (*VError)(struct VXIlogInterface* pThis,
			 const VXIchar*          moduleName,
			 VXIunsigned             errorID,
			 const VXIchar*          format,
			 va_list                 vargs);
  
  /**
   * @name Diagnostic
   * @memo Log a diagnostic message
   * @doc Basic diagnostic reporting mechanism.  Diagnostic messages are
   * reported by moduleName, tag id, subtag, a format, and a variable length
   * argument list.
   *
   * @param  tagID    [IN] Identifier that classifies a group of logically
   *                  associated diagnostic messages (usually from a single
   *                  software module) that are desirable to enable or
   *                  disable as a single unit. See the top of this file 
   *                  for tagID allocation rules.
   * @param  subtag   [IN] Arbitrary string that may be used to subdivide
   *                  the diagnostic messages of that tagID, or provide
   *                  additional standardized information such as the
   *                  source file, function, or method. There are no
   *                  rules for the content of this field.
   * @param  vargc    [IN] Intetger that if set to 0 indicates there is no 
   *                  optional arguments following the last parameter (format).
   *                  Any positive value indicates that there are additional 
   *                  optional arguments. Note this is added to fix a bug where 
   *                  escaped chars were falsely intepreted as format chars.
   * @param  format   [IN] Format string as passed to wprintf( ) (the
   *                  wchar_t version of printf( ) as defined by the ANSI C 
   *                  standard) for outputting free-form diagnostic text. 
   *                  This is followed by a variable list of arguments that 
   *                  supply values for insertion into the format string, 
   *                  also as passed to wprintf( ).<p>
   *
   *                  NOTE: Do NOT use %C and %S in the format string
   *                  for inserting narrow character strings (char and char *)
   *                  as supported by some compilers, as this is not portable
   *                  and may result in system crashes on some UNIX variants
   *                  if the VXIlog implementation uses the compiler supplied
   *                  ...printf( ) family of functions for handling these
   *                  variable argument lists.
   * @param  ...      [IN] Arguments matching the free-form diagnostic text
   *                  format specified above.
   *
   * @return VXIlog_RESULT_SUCCESS on success
   */
  VXIlogResult (*Diagnostic)(struct VXIlogInterface* pThis,
			     VXIunsigned             tagID,
			     const VXIchar*          subtag,
			     const VXIchar*          format
              ...);

  /**
   * @name VDiagnostic
   * @memo Log a diagnostic message (va_list variant)
   *
   * @doc
   * Same as <a href="Diagnostic.html"> Diagnostic</a>, but a va_list is
   * supplied as an argument instead of "..." in order to make it easy
   * to layer convenience functions/classes for logging on top of this
   * interface.
   *   
   * @return VXIlog_RESULT_SUCCESS on success
   */
  VXIlogResult (*VDiagnostic)(struct VXIlogInterface* pThis,
			      VXIunsigned             tagID,
			      const VXIchar*          subtag,
			      const VXIchar*          format,
			      va_list                 vargs);

  /**
   * @name DiagnosticIsEnabled
   * @memo Query whether diagnostic logging is enabled
   *
   * @doc
   * NOTE: Diagnostic log messages are automatically filtered in a
   *  high-performance way by the <a href="Diagnostic.html">
   Diagnostic </a>  method. This should 
   *  only be used in the rare conditions when there is significant 
   *  pre-processing work required to assemble the input parameters
   *  for Diagnostic( ), and thus it is best to suppress that
   *  performance impacting pre-processing as well.
   *
   * @param  tagID        [IN] Identifier for a class of 
   *
   * @return TRUE if that tag is enabled (information for that tag
   *         will be written to the diagnostic log), FALSE if that
   *         tag is disabled (information for that tag will be ignored)
   */
  VXIbool (*DiagnosticIsEnabled)(struct VXIlogInterface* pThis,
				 VXIunsigned             tagID);

  /**
   * @name Event
   * @memo Log an event
   * @doc Basic error reporting mechanism.  Errors are reported by
   * moduleName, error number, and with a varargs format. Event
   * details are not free-form, they  must be passed as a succession of 
   * key-value pairs, i.e. a string key followed by a value. See the
   * description of the format parameter for 
   * <a=href="Error.html"> Error </a> for a full explanation.
   *
   * @param  eventID      [IN] Event number to log, this is mapped to a
   *                      localized event name that is placed in the event
   *                      log. It is critical that this provide unambiguous
   *                      information about the nature of the event. See 
   *                      the top of this file for eventID allocation rules.
   * @param  eventHandle  [OUT] Returned handle to the event, used to add 
   *                      attribute/value pairs that provide details about
   *                      the event (such as the grammar name, etc.)
   * @param  format       [IN] Format string as passed to wprintf( ) (the
   *                      wchar_t version of printf( ) as defined by the
   *                      ANSI C standard) for outputting optional event
   *                      details. This is followed by a variable list of
   *                      arguments supplying values for insertion into the 
   *                      format string, also as passed to wprintf(). <p>
   *
   *                      IMPORTANT: Event details are not free-form, they
   *                      must be passed as a succession of key-value pairs,
   *                      i.e. a string key followed by a value. See the
   *                      description of the format parameter for 
   *                      <a href="Error.html"> Error </a> for a full 
   *                      explanation.
   *
   * @param  ...          [IN] Arguments matching the event details format
   *                      specified above.
   *
   * @return VXIlog_RESULT_SUCCESS on success
   */
  VXIlogResult (*Event)(struct VXIlogInterface* pThis,
			VXIunsigned             eventID,
			const VXIchar*          format,
			...);

  /**
   * @name VEvent
   * @memo Log an event (va_list variant)
   *
   * @doc
   * Same as <a href="Event.html"> Event </a>, but a va_list is supplied as
   * an argument instead of "..." in order to make it easy to layer
   * convenience functions/classes for logging on top of this
   * interface.
   *    
   * @return VXIlog_RESULT_SUCCESS on success
   */
  VXIlogResult (*VEvent)(struct VXIlogInterface* pThis,
			 VXIunsigned             eventID,
			 const VXIchar*          format,
			 va_list                 vargs);


  /**
   * @name Flush
   * @memo Flush the Log 
   *
   * @doc
   * Called to flush the log file periodically.
   *    
   * @return VXIlog_RESULT_SUCCESS on success
   */
  VXIlogResult (*Flush)(struct VXIlogInterface* pThis,
                        const char   *logFileName,
				                VXIbool       logToStdout);


} VXIlogInterface;


/*@}*/

#ifdef __cplusplus
}
#endif

#include "VXIheaderSuffix.h"

#endif  /* include guard */

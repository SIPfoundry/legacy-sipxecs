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

#ifndef _VXIJSI_H
#define _VXIJSI_H

#include "VXItypes.h"                  /* For VXIchar, VXIint, etc.  */
#include "VXIvalue.h"                  /* For VXIValue */

#include "VXIheaderPrefix.h"
#ifdef VXIJSI_EXPORTS
#define VXIJSI_API SYMBOL_EXPORT_DECL
#else
#define VXIJSI_API SYMBOL_IMPORT_DECL
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
struct VXIjsiContext;
#else
typedef struct VXIjsiContext { void * dummy; } VXIjsiContext;
#endif

/**
 * @name VXIjsi
 * @memo ECMAScript (JavaScript) Engine Interface
 *
 * @version 1.0
 * @doc
 * Abstract interface for interacting with a ECMAScript (JavaScript)
 * engine.  This provides functionality for creating ECMAScript
 * execution contexts, manipulating ECMAScript scopes, manipulating
 * variables within those scopes, and evaluating ECMAScript
 * expressions/scripts. <p>
 *
 * There is one ECMAScript interface per thread/line.
 */

/*@{*/

/**
 * Result codes for interface methods
 *
 * Result codes less then zero are severe errors (likely to be
 * platform faults), those greater then zero are warnings (likely to
 * be application issues)
 */
typedef enum VXIjsiResult {
  /* Fatal error, terminate call    */
  VXIjsi_RESULT_FATAL_ERROR       =  -100,
  /* I/O error                      */
  VXIjsi_RESULT_IO_ERROR           =   -8,
  /* Out of memory                  */
  VXIjsi_RESULT_OUT_OF_MEMORY      =   -7,
  /* System error, out of service   */
  VXIjsi_RESULT_SYSTEM_ERROR       =   -6,
  /* Errors from platform services  */
  VXIjsi_RESULT_PLATFORM_ERROR     =   -5,
  /* Return buffer too small        */
  VXIjsi_RESULT_BUFFER_TOO_SMALL   =   -4,
  /* Property name is not valid    */
  VXIjsi_RESULT_INVALID_PROP_NAME  =   -3,
  /* Property value is not valid   */
  VXIjsi_RESULT_INVALID_PROP_VALUE =   -2,
  /* Invalid function argument      */
  VXIjsi_RESULT_INVALID_ARGUMENT   =   -1,
  /* Success                        */
  VXIjsi_RESULT_SUCCESS            =    0,
  /* Normal failure, nothing logged */
  VXIjsi_RESULT_FAILURE            =    1,
  /* Non-fatal non-specific error   */
  VXIjsi_RESULT_NON_FATAL_ERROR    =    2,
  /* ECMAScript syntax error        */
  VXIjsi_RESULT_SYNTAX_ERROR       =   50,
  /* ECMAScript exception thrown    */
  VXIjsi_RESULT_SCRIPT_EXCEPTION   =   51,
  /* ECMAScript security violation  */
  VXIjsi_RESULT_SECURITY_VIOLATION =   52,
  /* Operation is not supported     */
  VXIjsi_RESULT_UNSUPPORTED        =  100
} VXIjsiResult;


/*
** ==================================================
** VXIjsiInterface Interface definition
** ==================================================
*/
/** @name VXIjsiInterface
 ** @memo VXIjsi interface for ECMAScript evaluation
 **
 */

typedef struct VXIjsiInterface {
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
   * Create and initialize a new script context
   *
   * This creates a new context. Currently one context is created per
   * thread, but the implementation must support the ability to have
   * multiple contexts per thread.
   *
   * @param context  [OUT] Newly created context
   *
   * @result VXIjsi_RESULT_SUCCESS on success
   */
  VXIjsiResult (*CreateContext)(struct VXIjsiInterface  *pThis,
                                VXIjsiContext          **context);

  /**
   * Destroy a script context, clean up storage if required
   *
   * @param context  [IN] Context to destroy
   *
   * @result VXIjsi_RESULT_SUCCESS on success
   */
  VXIjsiResult (*DestroyContext)(struct VXIjsiInterface  *pThis,
                                 VXIjsiContext          **context);

  /**
   * Create a script variable relative to the current scope, initialized
   *  to an expression
   *
   * NOTE: When there is an expression, the expression is evaluated,
   *  then the value of the evaluated expression (the final
   *  sub-expression) assigned. Thus an expression of "1; 2;" actually
   *  assigns 2 to the variable.
   *
   * @param context  [IN] ECMAScript context to create the variable within
   * @param name     [IN] Name of the variable to create
   * @param expr     [IN] Expression to set the initial value of the variable
   *                      (if NULL or empty the variable is set to ECMAScript
   *                      Undefined as required for VoiceXML 1.0 <var>)
   *
   * @result VXIjsi_RESULT_SUCCESS on success
   */
  VXIjsiResult (*CreateVarExpr)(struct VXIjsiInterface *pThis,
                                VXIjsiContext          *context,
                                const VXIchar          *name,
                                const VXIchar          *expr);

  /**
   * Create a script variable relative to the current scope, initialized
   *  to a VXIValue based value
   *
   * @param context  [IN] ECMAScript context to create the variable within
   * @param name     [IN] Name of the variable to create
   * @param value    [IN] VXIValue based value to set the initial value of
   *                      the variable (if NULL the variable is set to
   *                      ECMAScript Undefined as required for VoiceXML 1.0
   *                      <var>). VXIMap is used to pass ECMAScript objects.
   *
   * @result VXIjsi_RESULT_SUCCESS on success
   */
  VXIjsiResult (*CreateVarValue)(struct VXIjsiInterface *pThis,
                                 VXIjsiContext          *context,
                                 const VXIchar          *name,
                                 const VXIValue         *value);

  /**
   * Set a script variable to an expression relative to the current scope
   *
   * NOTE: The expression is evaluated, then the value of the
   *  evaluated expression (the final sub-expression) assigned. Thus
   *  an expression of "1; 2;" actually assigns 2 to the variable.
   *
   * @param context  [IN] ECMAScript context to set the variable within
   * @param name     [IN] Name of the variable to set
   * @param expr     [IN] Expression to be assigned
   *
   * @result VXIjsi_RESULT_SUCCESS on success
   */
  VXIjsiResult (*SetVarExpr)(struct VXIjsiInterface *pThis,
                             VXIjsiContext          *context,
                             const VXIchar          *name,
                             const VXIchar          *expr);

  /**
   * Set a script variable to a value relative to the current scope
   *
   * @param context  [IN] ECMAScript context to set the variable within
   * @param name     [IN] Name of the variable to set
   * @param value    [IN] VXIValue based value to be assigned. VXIMap is
   *                      used to pass ECMAScript objects.
   *
   * @result VXIjsi_RESULT_SUCCESS on success
   */
  VXIjsiResult (*SetVarValue)(struct VXIjsiInterface *pThis,
                              VXIjsiContext          *context,
                              const VXIchar          *name,
                              const VXIValue         *value);

  /**
   * Get the value of a variable
   *
   * @param context  [IN]  ECMAScript context to get the variable from
   * @param name     [IN]  Name of the variable to get
   * @param value    [OUT] Value of the variable, returned as the VXI
   *                       type that most closely matches the variable's
   *                       ECMAScript type. This function allocates this
   *                       for return on success (returns a NULL pointer
   *                       otherwise), the caller is responsible for
   *                       destroying it via VXIValueDestroy( ). VXIMap
   *                       is used to return ECMAScript objects.
   *
   * @result VXIjsi_RESULT_SUCCESS on success, VXIjsi_RESULT_FAILURE if the
   *         variable has a ECMAScript value of Null,
   *         VXIjsi_RESULT_NON_FATAL_ERROR if the variable is not
   *         defined (ECMAScript Undefined), or another error code for
   *         severe errors
   */
  VXIjsiResult (*GetVar)(struct VXIjsiInterface  *pThis,
                         const VXIjsiContext     *context,
                         const VXIchar           *name,
                         VXIValue               **value);

  /**
   * Check whether a variable is defined (not ECMAScript Undefined)
   *
   * NOTE: A variable with a ECMAScript Null value is considered defined
   *
   * @param context  [IN]  ECMAScript context to check the variable in
   * @param name     [IN]  Name of the variable to check
   *
   * @result VXIjsi_RESULT_SUCCESS on success (variable is defined),
   *         VXIjsi_RESULT_FAILURE if the variable is not defined,
   *         or another error code for severe errors
   */
  VXIjsiResult (*CheckVar)(struct VXIjsiInterface *pThis,
                           const VXIjsiContext    *context,
                           const VXIchar          *name);

  /**
   * Execute a script, optionally returning any execution result
   *
   * @param context  [IN]  ECMAScript context to execute within
   * @param expr     [IN]  Buffer containing the script text
   * @param value    [OUT] Result of the script execution, returned
   *                       as the VXI type that most closely matches
   *                       the variable's ECMAScript type. Pass NULL
   *                       if the result is not desired. Otherwise
   *                       this function allocates this for return on
   *                       success when there is a return value (returns
   *                       a NULL pointer otherwise), the caller is
   *                       responsible for destroying it via
   *                       VXIValueDestroy( ). VXIMap is used to return
   *                       ECMAScript objects.
   *
   * @result VXIjsi_RESULT_SUCCESS on success
   */
  VXIjsiResult (*Eval)(struct VXIjsiInterface  *pThis,
                       VXIjsiContext           *context,
                       const VXIchar           *expr,
                       VXIValue               **result);

  /**
   * Push a new context onto the scope chain (add a nested scope)
   *
   * @param context  [IN] ECMAScript context to push the scope onto
   * @param name     [IN] Name of the scope, used to permit referencing
   *                      variables from an explicit scope within the
   *                      scope chain, such as "myscope.myvar" to access
   *                      "myvar" within a scope named "myscope"
   *
   * @result VXIjsi_RESULT_SUCCESS on success
   */
  VXIjsiResult (*PushScope)(struct VXIjsiInterface *pThis,
                            VXIjsiContext          *context,
                            const VXIchar          *name);

  /**
   * Pop a context from the scope chain (remove a nested scope)
   *
   * @param context  [IN] ECMAScript context to pop the scope from
   *
   * @result VXIjsi_RESULT_SUCCESS on success
   */
  VXIjsiResult (*PopScope)(struct VXIjsiInterface *pThis,
                           VXIjsiContext          *context);

  /**
   * Reset the scope chain to the global scope (pop all nested scopes)
   *
   * @param context  [IN] ECMAScript context to pop the scopes from
   *
   * @result VXIjsi_RESULT_SUCCESS on success
   */
  VXIjsiResult (*ClearScopes)(struct VXIjsiInterface *pThis,
                              VXIjsiContext          *context);

} VXIjsiInterface;

/*@}*/

#ifdef __cplusplus
}
#endif

#include "VXIheaderSuffix.h"

#endif  /* include guard */

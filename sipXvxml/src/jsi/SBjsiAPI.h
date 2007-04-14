/*****************************************************************************
 *****************************************************************************
 *
 * 
 *
 * SBjsi JavaScript (ECMAScript) Engine API
 *
 * SBjsi API, a function library implementation of the VXIjsi abstract
 * interface for interacting with a JavaScript (ECMAScript) engine.
 * This provides functionality for creating JavaScript execution
 * contexts, manipulating JavaScript scopes, manipulating variables
 * within those scopes, and evaluating JavaScript expressions/scripts.
 *
 * There is one JavaScript interface per thread/line.
 *
 *****************************************************************************
 ****************************************************************************/


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
 */

#ifndef _SBJSIAPI_H
#define _SBJSIAPI_H

#include "VXIjsi.h"                    /* For VXIjsi base interface */

#include "VXIheaderPrefix.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Return the version
 */
VXIint32 SBjsiGetVersion(void);

VXIint32 SBjsiSeGetVersion(void);

/**
 * Return the implementation name
 */
const VXIchar* SBjsiGetImplementationName(void);

const VXIchar* SBjsiSeGetImplementationName(void);

/**
 * Create and initialize a new script context
 *
 * This creates a new context. Currently one context is created per
 * thread, but the implementation must support the ability to have
 * multiple contexts per thread.
 *
 * @param context  [OUT] Newly created context
 *
 * @result VXIjsiResult 0 on success 
 */
VXIjsiResult SBjsiCreateContext(VXIjsiInterface        *pThis,
				VXIjsiContext         **context);

VXIjsiResult SBjsiSeCreateContext(VXIjsiInterface        *pThis,
				  VXIjsiContext         **context);

/**
 * Destroy a script context, clean up storage if required
 *
 * @param context  [IN] Context to destroy
 *
 * @result VXIjsiResult 0 on success
 */
VXIjsiResult SBjsiDestroyContext(VXIjsiInterface        *pThis,
				 VXIjsiContext         **context);

VXIjsiResult SBjsiSeDestroyContext(VXIjsiInterface        *pThis,
				   VXIjsiContext         **context);

/**
 * Create a script variable relative to the current scope, initialized
 *  to an expression
 *
 * NOTE: When there is an expression, the expression is evaluated,
 *  then the value of the evaluated expression (the final
 *  sub-expression) assigned. Thus an expression of "1; 2;" actually
 *  assigns 2 to the variable.
 *
 * @param context  [IN] JavaScript context to create the variable within
 * @param name     [IN] Name of the variable to create
 * @param expr     [IN] Expression to set the initial value of the variable 
 *                      (if NULL or empty the variable is set to JavaScript 
 *                      Undefined as required for VoiceXML 1.0 <var>)
 *
 * @result VXIjsiResult 0 on success 
 */
VXIjsiResult SBjsiCreateVarExpr(VXIjsiInterface        *pThis,
				VXIjsiContext          *context, 
				const VXIchar          *name, 
				const VXIchar          *expr);
VXIjsiResult SBjsiSeCreateVarExpr(VXIjsiInterface        *pThis,
				  VXIjsiContext          *context, 
				  const VXIchar          *name, 
				  const VXIchar          *expr);
  
/**
 * Create a script variable relative to the current scope, initialized
 *  to a VXIValue based value
 *
 * @param context  [IN] JavaScript context to create the variable within
 * @param name     [IN] Name of the variable to create
 * @param value    [IN] VXIValue based value to set the initial value of 
 *                      the variable (if NULL the variable is set to 
 *                      JavaScript Undefined as required for VoiceXML 1.0
 *                      <var>). VXIMap is used to pass JavaScript objects.
 *
 * @result VXIjsiResult 0 on success 
 */
VXIjsiResult SBjsiCreateVarValue(VXIjsiInterface        *pThis,
				 VXIjsiContext          *context, 
				 const VXIchar          *name, 
				 const VXIValue         *value);
VXIjsiResult SBjsiSeCreateVarValue(VXIjsiInterface        *pThis,
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
 * @param context  [IN] JavaScript context to set the variable within
 * @param name     [IN] Name of the variable to set
 * @param expr     [IN] Expression to be assigned
 *
 * @result VXIjsiResult 0 on success 
 */
VXIjsiResult SBjsiSetVarExpr(VXIjsiInterface        *pThis,
			     VXIjsiContext          *context, 
			     const VXIchar          *name, 
			     const VXIchar          *expr);
VXIjsiResult SBjsiSeSetVarExpr(VXIjsiInterface        *pThis,
			       VXIjsiContext          *context, 
			       const VXIchar          *name, 
			       const VXIchar          *expr);
  
/**
 * Set a script variable to a value relative to the current scope
 *
 * @param context  [IN] JavaScript context to set the variable within
 * @param name     [IN] Name of the variable to set
 * @param value    [IN] VXIValue based value to be assigned. VXIMap is 
 *                      used to pass JavaScript objects.
 *
 * @result VXIjsiResult 0 on success 
 */
VXIjsiResult SBjsiSetVarValue(VXIjsiInterface        *pThis,
			      VXIjsiContext          *context, 
			      const VXIchar          *name, 
			      const VXIValue         *value);
VXIjsiResult SBjsiSeSetVarValue(VXIjsiInterface        *pThis,
				VXIjsiContext          *context, 
				const VXIchar          *name, 
				const VXIValue         *value);

/**
 * Get the value of a variable
 *
 * @param context  [IN]  JavaScript context to get the variable from
 * @param name     [IN]  Name of the variable to get
 * @param value    [OUT] Value of the variable, returned as the VXI
 *                       type that most closely matches the variable's
 *                       JavaScript type. This function allocates this
 *                       for return on success (returns a NULL pointer
 *                       otherwise), the caller is responsible for
 *                       destroying it via VXIValueDestroy( ). VXIMap
 *                       is used to return JavaScript objects.
 *
 * @result VXIjsiResult 0 on success, VXIjsi_RESULT_FAILURE if the
 *         variable has a JavaScript value of Null,
 *         VXIjsi_RESULT_NON_FATAL_ERROR if the variable is not
 *         defined (JavaScript Undefined), or another error code for
 *         severe errors 
 */
VXIjsiResult SBjsiGetVar(VXIjsiInterface         *pThis,
			 const VXIjsiContext     *context, 
			 const VXIchar           *name,
			 VXIValue               **value);
VXIjsiResult SBjsiSeGetVar(VXIjsiInterface         *pThis,
			   const VXIjsiContext     *context, 
			   const VXIchar           *name,
			   VXIValue               **value);

/**
 * Check whether a variable is defined (not JavaScript Undefined)
 *
 * NOTE: A variable with a JavaScript Null value is considered defined
 *
 * @param context  [IN]  JavaScript context to check the variable in
 * @param name     [IN]  Name of the variable to check
 *
 * @result VXIjsiResult 0 on success (variable is defined),
 *         VXIjsi_RESULT_FAILURE if the variable is not defined,
 *         or another error code for severe errors
 */
VXIjsiResult SBjsiCheckVar(VXIjsiInterface        *pThis,
			   const VXIjsiContext    *context, 
			   const VXIchar          *name);
VXIjsiResult SBjsiSeCheckVar(VXIjsiInterface        *pThis,
			     const VXIjsiContext    *context, 
			     const VXIchar          *name);

/**
 * Execute a script, optionally returning any execution result
 *
 * @param context  [IN]  JavaScript context to execute within
 * @param expr     [IN]  Buffer containing the script text
 * @param value    [OUT] Result of the script execution, returned 
 *                       as the VXI type that most closely matches 
 *                       the variable's JavaScript type. Pass NULL
 *                       if the result is not desired. Otherwise
 *                       this function allocates this for return on 
 *                       success when there is a return value (returns 
 *                       a NULL pointer otherwise), the caller is 
 *                       responsible for destroying it via 
 *                       VXIValueDestroy( ). VXIMap is used to return 
 *                       JavaScript objects.
 *
 * @result VXIjsiResult 0 on success
 */
VXIjsiResult SBjsiEval(VXIjsiInterface         *pThis,
		       VXIjsiContext           *context,
		       const VXIchar           *expr,
		       VXIValue               **result);
VXIjsiResult SBjsiSeEval(VXIjsiInterface         *pThis,
			 VXIjsiContext           *context,
			 const VXIchar           *expr,
			 VXIValue               **result);

/**
 * Push a new context onto the scope chain (add a nested scope)
 *
 * @param context  [IN] JavaScript context to push the scope onto
 * @param name     [IN] Name of the scope, used to permit referencing
 *                      variables from an explicit scope within the
 *                      scope chain, such as "myscope.myvar" to access
 *                      "myvar" within a scope named "myscope"
 *
 * @result VXIjsiResult 0 on success
 */
VXIjsiResult SBjsiPushScope(VXIjsiInterface        *pThis,
			    VXIjsiContext          *context,
			    const VXIchar          *name);
VXIjsiResult SBjsiSePushScope(VXIjsiInterface        *pThis,
			      VXIjsiContext          *context,
			      const VXIchar          *name);

/**
 * Pop a context from the scope chain (remove a nested scope)
 *
 * @param context  [IN] JavaScript context to pop the scope from
 *
 * @result VXIjsiResult 0 on success
 */
VXIjsiResult SBjsiPopScope(VXIjsiInterface        *pThis,
			   VXIjsiContext          *context);
VXIjsiResult SBjsiSePopScope(VXIjsiInterface        *pThis,
			     VXIjsiContext          *context);

/**
 * Reset the scope chain to the global scope (pop all nested scopes)
 *
 * @param context  [IN] JavaScript context to pop the scopes from
 *
 * @result VXIjsiResult 0 on success
 */
VXIjsiResult SBjsiClearScopes(VXIjsiInterface        *pThis,
			      VXIjsiContext          *context);
VXIjsiResult SBjsiSeClearScopes(VXIjsiInterface        *pThis,
				VXIjsiContext          *context);

#ifdef __cplusplus
}
#endif

#include "VXIheaderSuffix.h"

#endif  /* include guard */

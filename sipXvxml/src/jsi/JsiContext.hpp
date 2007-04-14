/*****************************************************************************
 *****************************************************************************
 *
 * $Id: JsiContext.hpp,v 1.20.6.1 2001/10/03 16:20:52 dmeyer Exp $
 *
 * JsiContext, class for managing JavaScript contexts
 *
 * The JsiContext class represents a JavaScript context, a script
 * execution state. All JavaScript variables are maintained in a
 * context, and all scripts are executed in reference to a context
 * (for accessing variables and maintaining script side-effects). Each
 * context may have one or more scopes that are used to layer the
 * state information so that it is possible for clients to control the
 * lifetime of state information within the context.
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

// -----1=0-------2=0-------3=0-------4=0-------5=0-------6=0-------7=0-------8

#ifndef _JSI_CONTEXT_H__
#define _JSI_CONTEXT_H__

#include "VXIjsi.h"              // For VXIjsiResult codes
#include "inet/SBinetString.hpp"      // For SBinetString class
#include "inet/SBinetLogger.hpp"      // For SBinetLogger base class

#include <jsapi.h>               // SpiderMonkey API, for typedefs

#ifndef JS_DLL_CALLBACK
#define JS_DLL_CALLBACK CRT_CALL // For SpiderMonkey 1.5 RC 3 and earlier
#endif

class JsiRuntime;
class JsiProtectedJsval;
class JsiScopeChainNode;
extern "C" struct VXIlogInterface;

class JsiContext : public SBinetLogger {
 public:
  // Constructor and destructor
  JsiContext( );
  virtual ~JsiContext( );

  // Creation method
  VXIjsiResult Create (JsiRuntime        *runtime, 
		       long               contextSize, 
		       long               maxBranches,
		       VXIlogInterface   *log,
		       VXIunsigned        diagTagBase);

  // Create a script variable relative to the current scope
  VXIjsiResult CreateVar (const VXIchar  *name, 
			  const VXIchar  *expr);
  VXIjsiResult CreateVar (const VXIchar  *name, 
			  const VXIValue *value);
  
  // Set a script variable relative to the current scope
  VXIjsiResult SetVar (const VXIchar     *name, 
		       const VXIchar     *expr);
  VXIjsiResult SetVar (const VXIchar     *name, 
		       const VXIValue    *value);
  
  // Get the value of a variable
  VXIjsiResult GetVar (const VXIchar     *name,
		       VXIValue         **value) const;
  
  // Check whether a variable is defined (not void, could be NULL)
  VXIjsiResult CheckVar (const VXIchar   *name) const;
  
  // Execute a script, optionally returning any execution result
  VXIjsiResult Eval (const VXIchar       *expr,
		     VXIValue           **result);
  
  // Push a new context onto the scope chain (add a nested scope);
  VXIjsiResult PushScope (const VXIchar  *name);
  
  // Pop a context from the scope chain (remove a nested scope);
  VXIjsiResult PopScope( );
  
  // Reset the scope chain to the global scope (pop all nested scopes);
  VXIjsiResult ClearScopes( );
  
 private:
  // NOTE: Internal methods, these do not do mutex locking

  // Flag that we are starting and stopping execution of a script,
  // used to ensure thread safety. For simplicity, return true on
  // success, false on failure (mutex error). For the threadsafe
  // build, this is really disabling/enabling garbage collection.  For
  // the non-threadsafe build this is doing mutex locks.
  bool AccessBegin( ) const { 
#ifdef JS_THREADSAFE
    JS_ResumeRequest (context, contextRefs);
    return true;
#else
    return runtime->AccessBegin( );
#endif
  }

  bool AccessEnd( ) const {
#ifdef JS_THREADSAFE
    JsiContext *pThis = (JsiContext *) this;
    pThis->contextRefs = JS_SuspendRequest (context);
#endif
#ifdef JSI_MEMORY_TEST
    // Do garbage collection to invalidate memory, use this during unit
    // test/Purify runs to make sure we're extremely clean
    if ( context ) JS_GC (context);
#endif
#ifdef JS_THREADSAFE
    return true;
#else
    return runtime->AccessEnd( );
#endif
  }
  
  // Script evaluation
  VXIjsiResult EvaluateScript (const VXIchar *script, 
			       JsiProtectedJsval *retval = NULL,
			       bool loggingEnabled = true) const;

  // Convert JS values to VXIValue types and vice versa
  static VXIjsiResult JsvalToVXIValue (JSContext *context,
				       const jsval val,
				       VXIValue **value);
  VXIjsiResult VXIValueToJsval (JSContext *context,
				const VXIValue *value,
				JsiProtectedJsval *val);

  // Reset for the next evaluation
  void EvaluatePrepare (bool enableLog = true) const { 
    JsiContext *pThis = (JsiContext *) this;
    pThis->logEnabled = enableLog;
    pThis->numBranches = 0L;
    if ( pThis->exception ) { 
      VXIValueDestroy (&pThis->exception); pThis->exception = NULL; }
  }

  // Disable the copy constructor and assignment operator
  JsiContext (const JsiContext &src);
  JsiContext & operator= (const JsiContext &src);

 public:
  // NOTE: Internal static methods, these do not do mutex locking

  // Static callback for finalizing scopes
  static void JS_DLL_CALLBACK ScopeFinalize (JSContext *cx, JSObject *obj);

  // Static callback for finalizing content objects
  static void JS_DLL_CALLBACK ContentFinalize (JSContext *cx, JSObject *obj);

  // Static callback for enforcing maxBranches
  static JSBool JS_DLL_CALLBACK BranchCallback (JSContext *context, 
						JSScript *script);

  // Static callback for reporting JavaScript errors
  static void JS_DLL_CALLBACK ErrorReporter (JSContext *context, 
					     const char *message,
					     JSErrorReport *report);

 private:
  JSVersion           version;           // JavaScript version
  JsiRuntime         *runtime;           // JavaScript runtime environment
  JSContext          *context;           // Underlying SpiderMonkey context
  jsrefcount          contextRefs;       // Reference count for the context
  JsiScopeChainNode  *scopeChain;        // Scope chain
  JsiScopeChainNode  *currentScope;      // Current (leaf) scope

  // Evaluation state information
  bool      logEnabled;      /* Whether to log JavaScript errors */
  long      maxBranches;     /* Maximum number of branches for each 
			        evaluation */
  long      numBranches;     /* Number of branches for the current evaluation,
		 	        used to enforce maxBranches */
  VXIValue *exception;      /* Exception data, NULL if no exception */


};

#endif  // _JSI_CONTEXT_H__

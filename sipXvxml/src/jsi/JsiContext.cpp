/*****************************************************************************
 *****************************************************************************
 *
 * 
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
 * SBjsiInterface, definition of the real SBjsi resource object
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

static const char *rcsid = 0 ? (char *) &rcsid :
"";

// -----1=0-------2=0-------3=0-------4=0-------5=0-------6=0-------7=0-------8


#include "SBjsiInternal.h"

#include <string.h>                  // For strlen( )
#include <jsapi.h>                   // SpiderMonkey API

#include "SBjsiLog.h"                // For logging
#include "inet/SBinetString.hpp"     // For SBinetString
#include "JsiRuntime.hpp"            // For the JsiRuntime class
#include "JsiContext.hpp"            // Defines this class

// SpiderMonkey class that describes our scope objects which we use
// as contexts
static JSClass SCOPE_CLASS = {
    "__SBjsiScope", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub,  
    JsiContext::ScopeFinalize, JSCLASS_NO_OPTIONAL_MEMBERS
};

// SpiderMonkey class that describes our content objects which we use
// to point at VXIContent objects
static JSClass CONTENT_CLASS = {
    "__SBjsiContent", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub,  
    JsiContext::ContentFinalize, JSCLASS_NO_OPTIONAL_MEMBERS
};

// SpiderMonkey class that describes our objects which we create from
// a VXIMap, just very simple objects
static JSClass MAP_CLASS = {
    "__SBjsiMap", 0,
    JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub,  
    JS_FinalizeStub, JSCLASS_NO_OPTIONAL_MEMBERS
};

// Names for JS objects we protect from garbage collection, only used
// for SpiderMonkey debugging purposes
static const char GLOBAL_SCOPE_NAME[]  = "__SBjsiGlobalScope";
static const wchar_t GLOBAL_SCOPE_NAME_W[]  = L"__SBjsiGlobalScope";
static const char SCRIPT_OBJECT_NAME[] = "__SBjsiScriptObject";
static const char PROTECTED_JSVAL_NAME[] = "__SBjsiProtectedJsval";

// Global variable we use for temporaries
static const char GLOBAL_TEMP_VAR[] = "__SBjsiTempVar";
static const wchar_t GLOBAL_TEMP_VAR_W[] = L"__SBjsiTempVar";


// -----1=0-------2=0-------3=0-------4=0-------5=0-------6=0-------7=0-------8


// Macros and objects used to efficiently convert between VXIchar (native
// OS wchar_t, usually 16 bits but under GNU gcc 32 bits) and jschar
// (SpiderMonkey 16 bit).
class JsiConvertJschar {
 public:
  JsiConvertJschar(const jschar *in) : len(0), str(NULL) {
    if (in) {
      while (in[len])
	len++;
    }
    str = new VXIchar [len + 1];
    str[len] = 0;
    for (size_t i = 0; i < len; i++)
      str[i] = in[i];
  }
  ~JsiConvertJschar( ) { if (str) delete [] str; str = NULL; }
  const VXIchar *Get( ) const { return str; }
  size_t GetLength( ) const { return len; }

 private:
  size_t   len;
  VXIchar *str;
};

class JsiConvertVXIchar {
 public:
  JsiConvertVXIchar(const VXIchar *in) : len(0), str(NULL) {
    if (in)
      len = wcslen (in);
    str = new jschar [len + 1];
    str[len] = 0;
    for (size_t i = 0; i < len; i++)
      str[i] = (jschar) in[i];
  }
  ~JsiConvertVXIchar( ) { if (str) delete [] str; str = NULL; }
  const jschar *Get( ) const { return str; }
  size_t GetLength( ) const { return len; }

 private:
  size_t   len;
  jschar  *str;
};

#ifdef VXICHAR_SIZE_16_BITS

#define GET_VXICHAR_FROM_JSCHAR(out, in) \
  const VXIchar *out = (const VXIchar *) in

#define GET_JSCHAR_FROM_VXICHAR(out, outlen, in) \
  const jschar *out = (const jschar *) in; \
  size_t outlen = wcslen(in)

#else /* not VXICHAR_SIZE_16_BITS */

#define GET_VXICHAR_FROM_JSCHAR(out, in) \
  JsiConvertJschar convert_##out(in); \
  const VXIchar *out = convert_##out.Get( )

#define GET_JSCHAR_FROM_VXICHAR(out, outlen, in) \
  JsiConvertVXIchar convert_##out(in); \
  const jschar *out = convert_##out.Get( ); \
  size_t outlen = convert_##out.GetLength( )

#endif /* VXICHAR_SIZE_16_BITS */


// -----1=0-------2=0-------3=0-------4=0-------5=0-------6=0-------7=0-------8


// Wrapper class around jsval that automatically protects the held
// jsval from garbage collection
class JsiProtectedJsval {
 public:
  // Constructor and destructor
  JsiProtectedJsval (JSContext *c) : context(c), val(JSVAL_VOID) { }
  ~JsiProtectedJsval( ) { Clear( ); }

  // Clear the value
  VXIjsiResult Clear( ) { 
    VXIjsiResult rc;
    rc = ((( JSVAL_IS_GCTHING(val) ) && ( ! JS_RemoveRoot (context, &val) )) ?
	  VXIjsi_RESULT_FATAL_ERROR : VXIjsi_RESULT_SUCCESS);
    val = JSVAL_VOID;
    return rc;
  }
  
  // Set the value
  VXIjsiResult Set (jsval v) {
    VXIjsiResult rc = Clear( );
    val = v;
    if (( JSVAL_IS_GCTHING(val) ) &&
	( ! JS_AddNamedRoot (context, &val, PROTECTED_JSVAL_NAME) )){
      rc = VXIjsi_RESULT_OUT_OF_MEMORY; // blown away by GC already!
    }
    return rc;
  }

  // Accessor
  const jsval Get( ) const { return val; }

 private:
  // Disabled copy constructor and assignment operator
  JsiProtectedJsval (const JsiProtectedJsval &v);
  JsiProtectedJsval &operator=(const JsiProtectedJsval &v);

 private:
  JSContext  *context;
  jsval      val;
};


// -----1=0-------2=0-------3=0-------4=0-------5=0-------6=0-------7=0-------8


// Scope chain class, used to keep the scope chain independantly of
// the actual user visible scope chain to avoid crashes from invalid
// or malicious scripts (push a scope called "session", then overwrite
// that variable with an integer named "session", then try to do more
// work in that scope)
class JsiScopeChainNode {
 public:
  // Constructor and destructor
  JsiScopeChainNode (JSContext *context, JsiScopeChainNode *p, 
		     const VXIchar *n) :
    name(n), parent(p), child(NULL), jsVal(context) { }
  ~JsiScopeChainNode( ) { }

  // Creation method
  VXIjsiResult Create (JSObject *scopeObj) { 
    return jsVal.Set (OBJECT_TO_JSVAL(scopeObj)); }

  // Accessors
  const SBinetString & GetName( )   const { return name; }
  jsval                GetJsval( )  { return jsVal.Get( ); }
  JSObject           * GetJsobj( )  { return JSVAL_TO_OBJECT(jsVal.Get( )); }
  JsiScopeChainNode  * GetParent( ) { return parent; }
  JsiScopeChainNode  * GetChild( )  { return child; }

  // Set the child scope
  void SetChild (JsiScopeChainNode *c) { child = c; }

  // Release this scope and all under it
  VXIjsiResult Release( );

 private:
  SBinetString        name;      // Name of this scope
  JsiScopeChainNode  *parent;    // Parent scope
  JsiScopeChainNode  *child;     // Child scope, may be NULL
  JsiProtectedJsval   jsVal;     // JS object for the scope
};


VXIjsiResult JsiScopeChainNode::Release( )
{
  JsiScopeChainNode *node = this, *next = NULL;
  while (node) {
    // Release the lock on the underlying object
    node->jsVal.Clear( );

    // Clear pointers for safety and advance to child scopes
    next = node->child;
    node->parent = NULL;
    node->child = NULL;
    node = next;

    // NOTE: the deletion of this node and the child nodes is handled
    // by the ScopeFinalize( ) method that is called when the JS
    // object is garbage collected
  }

  return VXIjsi_RESULT_SUCCESS;
}


// -----1=0-------2=0-------3=0-------4=0-------5=0-------6=0-------7=0-------8


// Constructor, only does initialization that cannot fail
JsiContext::JsiContext( ) : SBinetLogger (MODULE_SBJSI, NULL, 0),
  version(JSVERSION_DEFAULT), runtime(NULL), context(NULL),
  contextRefs(0), scopeChain(NULL), currentScope(NULL), logEnabled(true),
  maxBranches(0L), numBranches(0L), exception(NULL)
{
}


// Destructor
JsiContext::~JsiContext( )
{
  Diag (SBJSI_LOG_CONTEXT, L"JsiContext::~JsiContext", 
	L"start 0x%p, JS context 0x%p", this, context);
    
  if ( exception )
    VXIValueDestroy (&exception);

  if ( context ) {
    // Lock the context for access
#ifdef JS_THREADSAFE
    JS_ResumeRequest (context, contextRefs);
#else
    if ( ! AccessBegin( ) )
      return;
#endif

    // Destroy the scope chain, which automatically unroots the global
    // scope to allow garbage collection of everything
    if ( scopeChain )
      scopeChain->Release( );

    // Release the lock, must be done before destroying the context
#ifdef JS_THREADSAFE
    JS_EndRequest (context);
#else
    AccessEnd( );
#endif

    // Destroy the context, is set to NULL
    runtime->DestroyContext (&context);
  }

  Diag (SBJSI_LOG_CONTEXT, L"JsiContext::~JsiContext", L"end 0x%p", this);
}


// Creation method
VXIjsiResult JsiContext::Create (JsiRuntime        *rt, 
				 long               contextSize, 
				 long               mb,
				 VXIlogInterface   *l,
				 VXIunsigned        diagTagBase)
{
  if (( rt == NULL ) || ( contextSize < 1 ) || ( mb < 1 ) ||
      ( l == NULL ))
    return VXIjsi_RESULT_INVALID_ARGUMENT;

  // Copy simple variables
  runtime = rt;
  maxBranches = mb;

  // Base logging class initialization
  SetLog (l, diagTagBase);

  Diag (SBJSI_LOG_CONTEXT, L"JsiContext::Create", L"start 0x%p", this);

  // Create the context
  VXIjsiResult rc = runtime->NewContext (contextSize, &context);
  if ( rc != VXIjsi_RESULT_SUCCESS )
    return rc;

  // Lock the context for access
#ifdef JS_THREADSAFE
  JS_BeginRequest (context);
#else
  if ( AccessBegin( ) != true )
    rc = VXIjsi_RESULT_SYSTEM_ERROR;
#endif

  // Attach this as private data to the context for use in callbacks
  if ( rc == VXIjsi_RESULT_SUCCESS )
    JS_SetContextPrivate (context, this);

  // Set the callback to enforce maxBranches
  if ( rc == VXIjsi_RESULT_SUCCESS )
    JS_SetBranchCallback (context, JsiContext::BranchCallback);
  
  // Set the callback for reporting errors
  if ( rc == VXIjsi_RESULT_SUCCESS )
    JS_SetErrorReporter (context, JsiContext::ErrorReporter);

#if 0
  // Enable strictness, which provides additional warnings/errors.
  // NOTE: Disabled for now, may want this configurable in the future.
  if ( rc == VXIjsi_RESULT_SUCCESS )
    JS_SetOptions (context, JSOPTION_STRICT);
#endif

  // Create and intialize the global scope we require in the context,
  // the scope chain node object locks it to protect it from the
  // garbage collector
  if ( rc == VXIjsi_RESULT_SUCCESS ) {
    JSObject *globalScope = JS_NewObject (context, &SCOPE_CLASS, NULL, NULL);
    if ( ! globalScope )
      rc = VXIjsi_RESULT_OUT_OF_MEMORY;
    else {
      Diag (SBJSI_LOG_SCOPE, L"JsiContext::Create", 
	    L"global scope 0x%p, context 0x%p", globalScope, context);

      scopeChain = new JsiScopeChainNode (context, NULL, GLOBAL_SCOPE_NAME_W);
      if ( scopeChain ) {
	rc = scopeChain->Create (globalScope);
	if ( rc == VXIjsi_RESULT_SUCCESS ) {
	  if ( JS_SetPrivate (context, globalScope, scopeChain) )
	    currentScope = scopeChain;
	  else
	    rc = VXIjsi_RESULT_FATAL_ERROR;
	}
      } else {
	rc = VXIjsi_RESULT_OUT_OF_MEMORY;
      }
    }
  }

  // Initialize the standard JavaScript classes, like Array, Math, String, etc.
  if (( rc == VXIjsi_RESULT_SUCCESS ) &&
      ( ! JS_InitStandardClasses (context, currentScope->GetJsobj( )) ))
    rc = VXIjsi_RESULT_OUT_OF_MEMORY;

  // Set version only after there is a valid global scope object
  if (( rc == VXIjsi_RESULT_SUCCESS ) &&
      ( version != JSVERSION_DEFAULT ))
    JS_SetVersion (context, version);

  // On failure, destroy the context here to avoid use of it
  if ( rc != VXIjsi_RESULT_SUCCESS ) {
    if ( context ) {
#ifdef JS_THREADSAFE
      JS_EndRequest (context);
#endif
      runtime->DestroyContext (&context);
    }
  }
#ifdef JS_THREADSAFE
  else {
    contextRefs = JS_SuspendRequest (context);
  }
#else

  if ( AccessEnd( ) != true )
    rc = VXIjsi_RESULT_SYSTEM_ERROR;
#endif

  Diag (SBJSI_LOG_CONTEXT, L"JsiContext::Create", 
	L"end 0x%p, JS context 0x%p", this, context);

  return rc;
}


// Create a script variable relative to the current scope
VXIjsiResult JsiContext::CreateVar (const VXIchar  *name, 
				    const VXIchar  *expr)
{
  if (( name == NULL ) || ( name[0] == 0 ))
    return VXIjsi_RESULT_INVALID_ARGUMENT;

  // If this is a fully qualified name, just do a SetVar( ) as that
  // handles explicitly scoped variables and this doesn't. No
  // functional effect to doing so, the only difference is that
  // CreateVar( ) masks vars of the same name from prior scopes when
  // SetVar( ) just sets the old one, having an explicit scope makes
  // that irrelevant since then JS knows exactly what to do.
  if ( wcschr (name, L'.') )
    return SetVar (name, expr);

  if ( AccessBegin( ) == false )
    return VXIjsi_RESULT_SYSTEM_ERROR;

  // Evaluate the expression, need to eval before setting so we can
  // deal with things like "1; 2;" which can't be handled if we just
  // make a temporary script
  VXIjsiResult rc = VXIjsi_RESULT_SUCCESS;
  JsiProtectedJsval val (context);
  if (( expr != NULL ) && ( expr[0] != 0 ))
    rc = EvaluateScript (expr, &val);

  // Set the variable in the current scope directly, ensures we mask
  // any var of the same name from earlier scopes
  if ( rc == VXIjsi_RESULT_SUCCESS ) {
    GET_JSCHAR_FROM_VXICHAR(tmpname, tmpnamelen, name);
    if ( ! JS_DefineUCProperty (context, currentScope->GetJsobj( ), 
				tmpname, tmpnamelen, val.Get( ), NULL, NULL,
				JSPROP_ENUMERATE) )
      rc = VXIjsi_RESULT_OUT_OF_MEMORY;
  }

  // Must unroot before unlocking
  val.Clear( );

  if ( AccessEnd( ) == false )
    rc = VXIjsi_RESULT_SYSTEM_ERROR;

  return rc;
}


// Create a script variable relative to the current scope
VXIjsiResult JsiContext::CreateVar (const VXIchar  *name, 
				    const VXIValue *value)
{
  if (( name == NULL ) || ( name[0] == 0 ))
    return VXIjsi_RESULT_INVALID_ARGUMENT;

  // If this is a fully qualified name, just do a SetVar( ) as that
  // handles explicitly scoped variables and this doesn't. No
  // functional effect to doing so, the only difference is that
  // CreateVar( ) masks vars of the same name from prior scopes when
  // SetVar( ) just sets the old one, having an explicit scope makes
  // that irrelevant since then JS knows exactly what to do.
  if ( wcschr (name, L'.') )
    return SetVar (name, value);

  if ( AccessBegin( ) == false )
    return VXIjsi_RESULT_SYSTEM_ERROR;

  // Convert the value to a JavaScript variable
  VXIjsiResult rc = VXIjsi_RESULT_SUCCESS;
  JsiProtectedJsval val (context);
  if ( value )
    rc = VXIValueToJsval (context, value, &val);

  // Set the variable in the current scope directly, ensures we mask
  // any var of the same name from earlier scopes
  if ( rc == VXIjsi_RESULT_SUCCESS ) {
    GET_JSCHAR_FROM_VXICHAR(tmpname, tmpnamelen, name);
    if ( ! JS_DefineUCProperty (context, currentScope->GetJsobj( ), 
				tmpname, tmpnamelen, val.Get( ), NULL, NULL,
				JSPROP_ENUMERATE) )
      rc = VXIjsi_RESULT_OUT_OF_MEMORY;
  }

  // Must unroot before unlocking
  val.Clear( );

  if ( AccessEnd( ) == false )
    rc = VXIjsi_RESULT_SYSTEM_ERROR;

  return rc;
}


// Set a script variable relative to the current scope
VXIjsiResult JsiContext::SetVar (const VXIchar     *name, 
				 const VXIchar     *expr)
{
  if (( name == NULL ) || ( name[0] == 0 ) ||
      ( expr == NULL ) || ( expr[0] == 0 ))
    return VXIjsi_RESULT_INVALID_ARGUMENT;

  if ( AccessBegin( ) == false )
    return VXIjsi_RESULT_SYSTEM_ERROR;

  // Evaluate the expression, need to eval before setting so we can
  // deal with things like "1; 2;" which can't be handled if we just
  // make a temporary script
  JsiProtectedJsval val (context);
  VXIjsiResult rc = EvaluateScript (expr, &val);

  // What we'd like to do is lookup the variable, then set it to the
  // jsval we got above. (The lookup is complex because it could be
  // a partially or fully qualified name, with potential scope
  // chain/prototype chain lookups.) However, that isn't possible in
  // SpiderMonkey. So instead we set a temporary variable in the
  // global scope, then assign the specified object to that, then
  // destroy the temporary.
  if ( rc == VXIjsi_RESULT_SUCCESS ) {
    if ( JS_DefineProperty (context, currentScope->GetJsobj( ),
			    GLOBAL_TEMP_VAR, val.Get( ), NULL, NULL, 0) ) {
      // Do the copy using a temporary script
      SBinetString script (name);
      script += L"=";
      script += GLOBAL_TEMP_VAR_W;
      rc = EvaluateScript (script.c_str( ), NULL);

      // Destroy the temporary variable
      jsval ignored = JSVAL_VOID;
      if ( ! JS_DeleteProperty2 (context, currentScope->GetJsobj( ), 
				 GLOBAL_TEMP_VAR, &ignored) )
	rc = VXIjsi_RESULT_FATAL_ERROR;
    } else {
      rc = VXIjsi_RESULT_OUT_OF_MEMORY;
    }
  }

  // Must unroot before unlocking
  val.Clear( );

  if ( AccessEnd( ) == false )
    rc = VXIjsi_RESULT_SYSTEM_ERROR;

  return rc;
}


// Set a script variable relative to the current scope
VXIjsiResult JsiContext::SetVar (const VXIchar   *name, 
				 const VXIValue  *value)
{
  if (( name == NULL ) || ( name[0] == 0 ) || ( value == NULL ))
    return VXIjsi_RESULT_INVALID_ARGUMENT;

  if ( AccessBegin( ) == false )
    return VXIjsi_RESULT_SYSTEM_ERROR;

  // Convert the value to a JavaScript variable
  JsiProtectedJsval val (context);
  VXIjsiResult rc = VXIValueToJsval (context, value, &val);

  // What we'd like to do is lookup the variable, then set it to the
  // jsval we got above. (The lookup is complex because it could be
  // a partially or fully qualified name, with potential scope
  // chain/prototype chain lookups.) However, that isn't possible in
  // SpiderMonkey. So instead we set a temporary variable in the
  // global scope, then assign the specified object to that, then
  // destroy the temporary.
  if ( rc == VXIjsi_RESULT_SUCCESS ) {
    if ( JS_DefineProperty (context, currentScope->GetJsobj( ),
			    GLOBAL_TEMP_VAR, val.Get( ), NULL, NULL, 0) ) {
      // Do the copy using a temporary script
      SBinetString script (name);
      script += L"=";
      script += GLOBAL_TEMP_VAR_W;
      rc = EvaluateScript (script.c_str( ), NULL);

      // Destroy the temporary variable
      jsval ignored = JSVAL_VOID;
      if ( ! JS_DeleteProperty2 (context, currentScope->GetJsobj( ), 
				 GLOBAL_TEMP_VAR, &ignored) )
	rc = VXIjsi_RESULT_FATAL_ERROR;
    } else {
      rc = VXIjsi_RESULT_OUT_OF_MEMORY;
    }
  }

  // Must unroot before unlocking
  val.Clear( );

  if ( AccessEnd( ) == false )
    rc = VXIjsi_RESULT_SYSTEM_ERROR;

  return rc;
}


// Get the value of a variable
VXIjsiResult JsiContext::GetVar (const VXIchar     *name,
				 VXIValue         **value) const
{
  if (( name == NULL ) || ( name[0] == 0 ) || ( value == NULL ))
    return VXIjsi_RESULT_INVALID_ARGUMENT;

  *value = NULL;

  if ( AccessBegin( ) == false )
    return VXIjsi_RESULT_SYSTEM_ERROR;

  // Get the variable, we need to evaluate instead of just calling
  // JS_GetUCProperty( ) in order to have it automatically look back
  // through the scope chain, as well as to permit explicit scope
  // references like "myscope.myvar"
  JsiProtectedJsval val (context);
  VXIjsiResult rc = EvaluateScript (name, &val);

  if ( rc == VXIjsi_RESULT_SUCCESS )
    rc = JsvalToVXIValue (context, val.Get( ), value);
  else if (( rc == VXIjsi_RESULT_SYNTAX_ERROR ) ||
	   ( rc == VXIjsi_RESULT_SCRIPT_EXCEPTION ))
    rc = VXIjsi_RESULT_NON_FATAL_ERROR;

  // Must unroot before unlocking
  val.Clear( );

  if ( AccessEnd( ) == false )
    rc = VXIjsi_RESULT_SYSTEM_ERROR;

  return rc;
}


// Check whether a variable is defined (not void, could be NULL)
VXIjsiResult JsiContext::CheckVar (const VXIchar   *name) const
{
  if (( name == NULL ) || ( name[0] == 0 ))
    return VXIjsi_RESULT_INVALID_ARGUMENT;

  if ( AccessBegin( ) == false )
    return VXIjsi_RESULT_SYSTEM_ERROR;

  // Get the variable, we need to evaluate instead of just calling
  // JS_GetUCProperty( ) in order to have it automatically look back
  // through the scope chain, as well as to permit explicit scope
  // references like "myscope.myvar". We disable logging because
  // we don't want JavaScript error reports about this var being
  // undefined.
  JsiProtectedJsval val (context);
  VXIjsiResult rc = EvaluateScript (name, &val, false);

  if (( rc == VXIjsi_RESULT_SYNTAX_ERROR ) ||
      ( rc == VXIjsi_RESULT_SCRIPT_EXCEPTION ) ||
      ( rc == VXIjsi_RESULT_NON_FATAL_ERROR ) ||
      (( rc == VXIjsi_RESULT_SUCCESS ) &&
       ( JSVAL_IS_VOID (val.Get( )) )))   // JavaScript undefined
    rc = VXIjsi_RESULT_FAILURE;

  // Must unroot before unlocking
  val.Clear( );

  if ( AccessEnd( ) == false )
    rc = VXIjsi_RESULT_SYSTEM_ERROR;

  return rc;
}


// Execute a script, optionally returning any execution result
VXIjsiResult JsiContext::Eval (const VXIchar       *expr,
			       VXIValue           **result)
{
  if (( expr == NULL ) || ( expr[0] == 0 ))
    return VXIjsi_RESULT_INVALID_ARGUMENT;

  if ( result )
    *result = NULL;

  // Execute the script
  if ( AccessBegin( ) == false )
    return VXIjsi_RESULT_SYSTEM_ERROR;

  JsiProtectedJsval val (context);
  VXIjsiResult rc = EvaluateScript (expr, &val);

  if (( result ) && ( rc == VXIjsi_RESULT_SUCCESS ) &&
      ( val.Get( ) != JSVAL_VOID ))
    rc = JsvalToVXIValue (context, val.Get( ), result);

  // Must unroot before unlocking
  val.Clear( );

  if ( AccessEnd( ) == false )
    rc = VXIjsi_RESULT_SYSTEM_ERROR;

  return rc;
}


// Push a new context onto the scope chain (add a nested scope)
VXIjsiResult JsiContext::PushScope (const VXIchar *name)
{
  if (( ! name ) || ( ! name[0] ))
    return VXIjsi_RESULT_INVALID_ARGUMENT;

  if ( AccessBegin( ) == false )
    return VXIjsi_RESULT_SYSTEM_ERROR;

  // Create an object for the scope, the current scope is its parent
  VXIjsiResult rc = VXIjsi_RESULT_SUCCESS;
  JSObject *scope = JS_NewObject (context, &SCOPE_CLASS, NULL, 
				  currentScope->GetJsobj( ));
  if ( ! scope ) {
    rc = VXIjsi_RESULT_OUT_OF_MEMORY;
  } else {
    Diag (SBJSI_LOG_SCOPE, L"JsiContext::PushScope", 
	  L"scope %s (0x%p), context 0x%p", name, scope, context);

    // The scope chain node object locks it to protect this from
    // garbage collection in case someone (possibly a malicious
    // script) destroys the data member pointing at this in the global
    // scope
    JsiScopeChainNode *newScope = 
      new JsiScopeChainNode (context, currentScope, name);
    if ( newScope ) {
      rc = newScope->Create (scope);
      if (( rc == VXIjsi_RESULT_SUCCESS ) &&
	  ( ! JS_SetPrivate (context, scope, newScope) ))
	rc = VXIjsi_RESULT_FATAL_ERROR;
    } else {
      rc = VXIjsi_RESULT_OUT_OF_MEMORY;
    }

    // Set this scope as a property of the parent scope
    if ( rc == VXIjsi_RESULT_SUCCESS ) {
      GET_JSCHAR_FROM_VXICHAR(tmpname, tmpnamelen, name);
      if ( JS_DefineUCProperty (context, currentScope->GetJsobj( ), tmpname,
				tmpnamelen, newScope->GetJsval( ), NULL, NULL, 
				JSPROP_ENUMERATE) ) {
	// Add it to the scope chain and set it as the current scope
	currentScope->SetChild (newScope);
	currentScope = newScope;
      } else {
	rc = VXIjsi_RESULT_FATAL_ERROR;
      }
    }
  }

  if ( AccessEnd( ) == false )
    rc = VXIjsi_RESULT_SYSTEM_ERROR;

  return rc;
}


// Pop a context from the scope chain (remove a nested scope)
VXIjsiResult JsiContext::PopScope( )
{
  // Don't pop up past the global scope
  if ( currentScope == scopeChain )
    return VXIjsi_RESULT_NON_FATAL_ERROR;

  if ( AccessBegin( ) == false )
    return VXIjsi_RESULT_SYSTEM_ERROR;

  Diag (SBJSI_LOG_SCOPE, L"JsiContext::PopScope", 
	L"scope %s (0x%p), context 0x%p", 
	currentScope->GetName( ).c_str( ), currentScope->GetJsobj( ), context);
  
  // Revert to the parent scope
  JsiScopeChainNode *oldScope = currentScope;
  currentScope = currentScope->GetParent( );
  currentScope->SetChild (NULL);

  // Release the old scope variable within the parent scope to permit
  // it to be garbage collected
  VXIjsiResult rc = VXIjsi_RESULT_SUCCESS;
  jsval rval;
  GET_JSCHAR_FROM_VXICHAR(tmpname, tmpnamelen, oldScope->GetName( ).c_str( ));
  if ( ! JS_DeleteUCProperty2 (context, currentScope->GetJsobj( ), tmpname,
			       tmpnamelen, &rval) )
    rc = VXIjsi_RESULT_FATAL_ERROR;

  // Now finish releasing the old scope, the actual wrapper object is
  // freed by the scope finalize method when garbage collection occurs
  rc = oldScope->Release( );

  if ( AccessEnd( ) == false )
    rc = VXIjsi_RESULT_SYSTEM_ERROR;

  return rc;
}


// Reset the scope chain to the global scope (pop all nested scopes)
VXIjsiResult JsiContext::ClearScopes( )
{
  // See if there is anything to do
  if ( currentScope == scopeChain )
    return VXIjsi_RESULT_SUCCESS;

  if ( AccessBegin( ) == false )
    return VXIjsi_RESULT_SYSTEM_ERROR;

  Diag (SBJSI_LOG_SCOPE, L"JsiContext::ClearScopes", L"context 0x%p", 
	context);

  // Revert to the global scope
  JsiScopeChainNode *oldScope = scopeChain->GetChild( );
  currentScope = scopeChain;
  currentScope->SetChild (NULL);

  // Release the old child scope variable within the global scope to
  // permit it and all its descendants to be garbage collected
  VXIjsiResult rc = VXIjsi_RESULT_SUCCESS;
  jsval rval;
  GET_JSCHAR_FROM_VXICHAR(tmpname, tmpnamelen, oldScope->GetName( ).c_str( ));
  if ( ! JS_DeleteUCProperty2 (context, currentScope->GetJsobj( ), tmpname,
			       tmpnamelen, &rval) )
    rc = VXIjsi_RESULT_FATAL_ERROR;
  
  // Now finish releasing the old scope, the actual wrapper object is
  // freed by the scope finalize method when garbage collection occurs
  rc = oldScope->Release( );

  if ( AccessEnd( ) == false )
    rc = VXIjsi_RESULT_SYSTEM_ERROR;

  return rc;
}


// Script evaluation
VXIjsiResult JsiContext::EvaluateScript (const VXIchar *script, 
					 JsiProtectedJsval *retval,
					 bool loggingEnabled) const
{
  if (( script == NULL ) || ( script[0] == 0 ))
    return VXIjsi_RESULT_INVALID_ARGUMENT;

  // Reset our private data for evaluation callbacks
  EvaluatePrepare (loggingEnabled);
  if ( retval )
    retval->Clear( );

  // Compile the script
  VXIjsiResult rc = VXIjsi_RESULT_SUCCESS;
  GET_JSCHAR_FROM_VXICHAR(tmpscript, tmpscriptlen, script);
  JSScript *jsScript = JS_CompileUCScript (context, currentScope->GetJsobj( ), 
                                           tmpscript, tmpscriptlen, NULL, 1);
  if ( ! jsScript )
    rc = VXIjsi_RESULT_SYNTAX_ERROR;
  else {
    // Create a script object and root it to protect the script from
    // garbage collection, note that once this object exists it owns
    // the jsScript and thus we must not free it ourselves
    JSObject *jsScriptObj = JS_NewScriptObject (context, jsScript);
    if (( ! jsScriptObj ) ||
	( ! JS_AddNamedRoot (context, &jsScriptObj, SCRIPT_OBJECT_NAME) )) {
      JS_DestroyScript (context, jsScript);
      rc = VXIjsi_RESULT_OUT_OF_MEMORY;
    } else {
      // Evaluate the script
      jsval val = JSVAL_VOID;
      if ( JS_ExecuteScript (context, currentScope->GetJsobj( ), jsScript, 
			     &val) ) {
	if ( retval )
	  rc = retval->Set (val);
      } else if ( exception ) {
	rc = VXIjsi_RESULT_SCRIPT_EXCEPTION;
      } else if ( numBranches > maxBranches ) {
	rc = VXIjsi_RESULT_SECURITY_VIOLATION;
      } else {
	rc = VXIjsi_RESULT_NON_FATAL_ERROR;
      }
      
      // Release the script object
      if ( ! JS_RemoveRoot (context, &jsScriptObj) )
	rc = VXIjsi_RESULT_FATAL_ERROR;
    }
  }
  
  return rc;
}


// Convert JS values to VXIValue types
VXIjsiResult JsiContext::JsvalToVXIValue (JSContext *context,
					  const jsval val,
					  VXIValue **value)
{
  if ( value == NULL )
    return VXIjsi_RESULT_INVALID_ARGUMENT;
  
  *value = NULL;
  VXIjsiResult rc = VXIjsi_RESULT_SUCCESS;

  if ( JSVAL_IS_STRING (val) ) {
    GET_VXICHAR_FROM_JSCHAR(tmpval, JS_GetStringChars (JSVAL_TO_STRING (val)));
    *value = (VXIValue *) VXIStringCreate(tmpval);
  
  } else if ( JSVAL_IS_INT (val) )
    *value = (VXIValue *) VXIIntegerCreate (JSVAL_TO_INT (val));
  
  else if ( JSVAL_IS_DOUBLE (val) )
    *value = (VXIValue *) VXIFloatCreate ((VXIflt32) *JSVAL_TO_DOUBLE (val));
  
  else if ( JSVAL_IS_BOOLEAN (val) )
    *value = (VXIValue *) VXIIntegerCreate (JSVAL_TO_BOOLEAN (val));
  
  else if ( JSVAL_IS_NULL (val) )  // JavaScript null
    rc = VXIjsi_RESULT_FAILURE;

  else if ( JSVAL_IS_OBJECT (val) ) {
    JSObject *obj = JSVAL_TO_OBJECT(val);
    if ( JS_IsArrayObject (context, obj) ) {
      // Array, create a VXIVector
      jsuint elements = 0;
      VXIVector *vec = NULL;
      if ( ! JS_GetArrayLength (context, obj, &elements) ) {
	rc = VXIjsi_RESULT_FATAL_ERROR;
      } else if ( (vec = VXIVectorCreate( )) == NULL ) {
	rc = VXIjsi_RESULT_OUT_OF_MEMORY;
      } else {
	// Traverse through the elements in the vector (could be empty,
	// in which case we want to return an empty VXIVector)
	for (jsuint i = 0; 
	     (( rc == VXIjsi_RESULT_SUCCESS ) && ( i < elements )); i++) {
	  // Convert the element to a VXIValue, then insert
	  jsval elVal = JSVAL_VOID;
	  if ( JS_GetElement (context, obj, i, &elVal) ) {
	    VXIValue *elValue;
	    rc = JsvalToVXIValue (context, elVal, &elValue);
	    if (( rc == VXIjsi_RESULT_SUCCESS ) &&
		( VXIVectorAddElement (vec, elValue) != 
		  VXIvalue_RESULT_SUCCESS )) {
	      rc = VXIjsi_RESULT_OUT_OF_MEMORY;
	      VXIValueDestroy (&elValue);
	    }
	  } else {
	    rc = VXIjsi_RESULT_FATAL_ERROR;
	  }
	}

	if ( rc == VXIjsi_RESULT_SUCCESS )
	  *value = (VXIValue *) vec;
	else
	  VXIVectorDestroy (&vec);
      }

    } else if ( JS_InstanceOf (context, obj, &CONTENT_CLASS, NULL) ) {
      // Content object, retrieve the VXIContent
      *value = (VXIValue *) JS_GetPrivate (context, obj);
      if ( *value == NULL )
	rc = VXIjsi_RESULT_FATAL_ERROR;
      else 
	*value = VXIValueClone (*value);

    } else {
      // Regular object, create a VXIMap
      JSIdArray *props = JS_Enumerate (context, obj);
      VXIMap *vmap = NULL;
      if ( ! props ) {
	rc = VXIjsi_RESULT_FATAL_ERROR;
      } else if ( (vmap = VXIMapCreate( )) == NULL ) {
	rc = VXIjsi_RESULT_OUT_OF_MEMORY;
      } else {
	// Traverse through the properties in the object (could be empty,
	// in which case we want to return an empty VXIMap)
	for (jsint i = 0;
	     (( rc == VXIjsi_RESULT_SUCCESS ) && ( i < props->length )); i++) {
	  // Get the property name, looks funky but this was
	  // recommended by one of the SpiderMonkey contributors in a
	  // newsgroup thread
	  jsval prName = JSVAL_VOID, prVal = JSVAL_VOID;
	  if ( JS_IdToValue (context, props->vector[i], &prName) ) {
	    const jschar *name = JS_GetStringChars (JSVAL_TO_STRING (prName));
	    size_t namelen = JS_GetStringLength(JSVAL_TO_STRING (prName));
	    // Lookup the property, convert to a value, then add to the map
	    if (( name ) &&
		( JS_GetUCProperty (context, obj, name, namelen, &prVal) )) {
	      VXIValue *prValue;
	      GET_VXICHAR_FROM_JSCHAR(tmpname, name);
	      rc = JsvalToVXIValue (context, prVal, &prValue);
	      if (( rc == VXIjsi_RESULT_SUCCESS ) &&
		  ( VXIMapSetProperty (vmap, tmpname, prValue) 
		    != VXIvalue_RESULT_SUCCESS )) {
		rc = VXIjsi_RESULT_OUT_OF_MEMORY;
		VXIValueDestroy (&prValue);
	      }
	    } else {
	      rc = VXIjsi_RESULT_FATAL_ERROR;
	    }
	  } else {
	    rc = VXIjsi_RESULT_FATAL_ERROR;
	  }
	}

	if ( rc == VXIjsi_RESULT_SUCCESS )
	  *value = (VXIValue *) vmap;
	else
	  VXIMapDestroy (&vmap);
      }

      // Free the ID array
      if ( props )
	JS_DestroyIdArray (context, props);
    }
  }

  else                             // JavaScript undefined (VOID)
    rc = VXIjsi_RESULT_NON_FATAL_ERROR;

  // Check for out of memory during VXIValue type create
  if (( rc == VXIjsi_RESULT_SUCCESS ) && ( *value == NULL ))
    rc = VXIjsi_RESULT_OUT_OF_MEMORY;

  return rc;
}


// Convert VXIValue types to JS values
VXIjsiResult JsiContext::VXIValueToJsval (JSContext *context,
					  const VXIValue *value,
					  JsiProtectedJsval *val)
{
  VXIjsiResult rc = VXIjsi_RESULT_SUCCESS;

  if (( value == NULL ) || ( val == NULL ))
    return VXIjsi_RESULT_INVALID_ARGUMENT;

  switch ( VXIValueGetType (value) ) {
  case VALUE_INTEGER: {
    VXIint32 i = VXIIntegerValue ((VXIInteger *) value);
    if ( INT_FITS_IN_JSVAL(i) )
      rc = val->Set (INT_TO_JSVAL(i));
    else
      rc = VXIjsi_RESULT_NON_FATAL_ERROR;
    } break;

  case VALUE_FLOAT: {
    jsdouble *d = JS_NewDouble (context, 
				(double) VXIFloatValue ((VXIFloat *) value));
    if ( d )
      rc = val->Set (DOUBLE_TO_JSVAL(d));
    else
      rc = VXIjsi_RESULT_OUT_OF_MEMORY;
    } break;

  case VALUE_STRING: {
    GET_JSCHAR_FROM_VXICHAR(tmpvalue, tmpvaluelen, 
			    VXIStringCStr ((const VXIString *) value));
              tmpvaluelen = 0; // prevent compiler warning...
    JSString *s = JS_NewUCStringCopyZ (context, tmpvalue);
    if ( s )
      rc = val->Set (STRING_TO_JSVAL(s));
    else
      rc = VXIjsi_RESULT_OUT_OF_MEMORY;
    } break;

  case VALUE_PTR:
    // Not supported in JavaScript
    rc = VXIjsi_RESULT_NON_FATAL_ERROR;
    break;

  case VALUE_CONTENT: {
    // Create an object for the content, attach the passed content as
    // private data. We set the return value prior to setting the
    // private data to avoid getting garbage collected in the
    // meantime.
    VXIValue *c = NULL;
    JSObject *content = JS_NewObject (context, &CONTENT_CLASS, NULL, NULL);
    if (( ! content ) || 
	( val->Set (OBJECT_TO_JSVAL(content)) != VXIjsi_RESULT_SUCCESS ) ||
	( (c = VXIValueClone (value)) == NULL )) {
      val->Clear( );
      rc = VXIjsi_RESULT_OUT_OF_MEMORY; // JS object gets garbage collected
    } else if ( ! JS_SetPrivate (context, content, c) ) {
      val->Clear( );
      rc = VXIjsi_RESULT_FATAL_ERROR;
    }
    } break;

  case VALUE_MAP: {
    // Create an object for the map, temporarily root it so the data
    // added underneath is not garbage collected on us
    JSObject *mapObj = JS_NewObject (context, &MAP_CLASS, NULL, NULL);
    if ( ! mapObj ) {
      rc = VXIjsi_RESULT_OUT_OF_MEMORY;
    } else if ( (rc = val->Set (OBJECT_TO_JSVAL (mapObj))) ==
		VXIjsi_RESULT_SUCCESS ) {
      // Walk the map and add object properties, the map may be empty
      const VXIchar *prop;
      const VXIValue *propValue;
      VXIMapIterator *it = VXIMapGetFirstProperty ((const VXIMap *) value, 
						   &prop, &propValue);
      while (( it ) && ( rc == VXIjsi_RESULT_SUCCESS )) {
	JsiProtectedJsval propVal (context);
	rc = VXIValueToJsval (context, propValue, &propVal);
	if ( rc == VXIjsi_RESULT_SUCCESS ) {
	  GET_JSCHAR_FROM_VXICHAR(tmpprop, tmpproplen, prop);
	  if ( ! JS_DefineUCProperty (context, mapObj, tmpprop, tmpproplen,
				      propVal.Get( ), NULL, NULL, 
				      JSPROP_ENUMERATE) )
	    rc = VXIjsi_RESULT_OUT_OF_MEMORY;
	}
	
	if ( VXIMapGetNextProperty (it, &prop, &propValue) != 
	     VXIvalue_RESULT_SUCCESS ) {
	  VXIMapIteratorDestroy (&it);
	  it = NULL;
	}
      }

      if ( it )
	VXIMapIteratorDestroy (&it);
    }
    } break;

  case VALUE_VECTOR: {
    // Create a vector for the map, temporarily root it so the data
    // added underneath is not garbage collected on us
    JSObject *vecObj = JS_NewArrayObject (context, 0, NULL);
    if ( ! vecObj ) {
      rc = VXIjsi_RESULT_OUT_OF_MEMORY;
    } else if ( (rc = val->Set (OBJECT_TO_JSVAL (vecObj))) ==
		VXIjsi_RESULT_SUCCESS ) {
      // Walk the map and add object properties, the map may be empty
      const VXIVector *vec = (const VXIVector *) value;
      VXIunsigned i = 0;
      const VXIValue *elValue;
      while (( rc == VXIjsi_RESULT_SUCCESS ) &&
	     ( (elValue = VXIVectorGetElement (vec, i)) != NULL )) {
	JsiProtectedJsval elVal (context);
	rc = VXIValueToJsval (context, elValue, &elVal);
	if (( rc == VXIjsi_RESULT_SUCCESS ) &&
	    ( ! JS_DefineElement (context, vecObj, i, elVal.Get( ), NULL,
				  NULL, JSPROP_ENUMERATE) ))
	  rc = VXIjsi_RESULT_OUT_OF_MEMORY;

	i++;
      }
    }
    } break;

  default:
    rc = VXIjsi_RESULT_UNSUPPORTED;
  }

  if ( rc != VXIjsi_RESULT_SUCCESS )
    val->Clear( );

  return rc;
}


// Static callback for finalizing scopes
void JS_DLL_CALLBACK 
JsiContext::ScopeFinalize (JSContext *context, JSObject *obj)
{
  // Get this for logging the destruction
  JsiContext *pThis = (JsiContext *) JS_GetContextPrivate (context);

  // Get the scope node from the private data and delete it
  JsiScopeChainNode *scopeNode = 
    (JsiScopeChainNode *) JS_GetPrivate (context, obj);
  if ( scopeNode ) {
    if ( pThis )
      pThis->Diag (SBJSI_LOG_SCOPE, L"JsiContext::ScopeFinalize",
		   L"scope garbage collected %s (0x%p), context 0x%p", 
		   scopeNode->GetName( ).c_str( ), obj, context);

    delete scopeNode;
    scopeNode = NULL;
  }
}


// Static callback for finalizing content objects
void JS_DLL_CALLBACK JsiContext::ContentFinalize (JSContext *cx, JSObject *obj)
{
  // Get the scope name from the private data and delete it
  VXIContent *content = (VXIContent *) JS_GetPrivate (cx, obj);
  if ( content ) {
    VXIValue *tmp = (VXIValue *) content;
    VXIValueDestroy (&tmp);
  }
}


// Static callback for enforcing maxBranches
JSBool JS_DLL_CALLBACK 
JsiContext::BranchCallback (JSContext *context, JSScript *script)
{
  if ( ! context )
    return JS_FALSE;

  // Get ourself from our private data
  JsiContext *pThis = (JsiContext *) JS_GetContextPrivate (context);
  if ( ! pThis ) {
    // Severe error
    return JS_FALSE;
  }

  pThis->numBranches++;
  JSBool rc = JS_TRUE;
  if ( pThis->numBranches > pThis->maxBranches ) {
    rc = JS_FALSE;
    pThis->Error (JSI_WARN_MAX_BRANCHES_EXCEEDED, L"%s%ld", 
		  L"maxBranches", pThis->maxBranches);
  }

  return rc;
}


// Static callback for reporting JavaScript errors
void JS_DLL_CALLBACK JsiContext::ErrorReporter (JSContext *context, 
						const char *message,
						JSErrorReport *report)
{
  if ( ! context )
    return;

  // Get ourself from our private data
  JsiContext *pThis = (JsiContext *) JS_GetContextPrivate (context);
  if ( ! pThis )
    return;  // Severe error

  // Save exception information, ownership of the value is passed
  jsval ex = JSVAL_VOID;
  VXIValue *exception = NULL;
  if (( JS_IsExceptionPending (context) ) &&
      ( JS_GetPendingException (context, &ex) ) &&
      ( JsvalToVXIValue (context, ex, &exception) == VXIjsi_RESULT_SUCCESS )) {
    if ( pThis->exception ) 
      VXIValueDestroy (&pThis->exception);
    pThis->exception = exception;
  }

  if ( pThis->logEnabled ) {
    // Convert the ASCII string to wide characters
    wchar_t wmessage[1024];
    size_t len = strlen (message);
    if ( len > 1023 )
      len = 1023;
    for (size_t i = 0; i < len; i++)
      wmessage[i] = (wchar_t) message[i];
    wmessage[len] = 0;
    
    // Determine the error number to log
    unsigned int errNum;
    if ( JSREPORT_IS_WARNING (report->flags) )
      errNum = JSI_WARN_SM_SCRIPT_WARNING;
    else if ( JSREPORT_IS_EXCEPTION (report->flags) )
      errNum = JSI_ERROR_SM_SCRIPT_EXCEPTION;
    else if ( JSREPORT_IS_STRICT (report->flags) )
      errNum = JSI_WARN_SM_SCRIPT_STRICT;
    else
      errNum = JSI_ERROR_SM_SCRIPT_ERROR;
    
    // Log the error
    GET_VXICHAR_FROM_JSCHAR(tmpuclinebuf, report->uclinebuf);
    GET_VXICHAR_FROM_JSCHAR(tmpuctokenptr, report->uctokenptr);
    pThis->Error (errNum, L"%s%s%s%ld%s%s%s%s", L"errmsg", wmessage,
		  L"line", report->lineno, L"linetxt", tmpuclinebuf,
		  L"tokentxt", tmpuctokenptr);
  }
}

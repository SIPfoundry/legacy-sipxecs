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
 * ECMA Script evaluation engine
 *
 * Using the VXIjsi interface, this layer is employed by the VoiceXML 
 * Interpreter to set and retrieve script variables and evaluate expressions.
 *
 ***********************************************************************/

#ifndef _EXE_CONT
#define _EXE_CONT

#include "VXIvalue.h"
#include <vector>
#include <string>

extern "C" struct VXIjsiInterface;
extern "C" struct VXIjsiContext;

typedef std::basic_string<VXIchar> vxistring;

class Scripter {
  // All functions, except the destructor, may throw exceptions.  These fall
  // into two classes.   The interpreter events represent semantic errors
  // which could in principal be handled by the application.  And the severe
  // JavaScriptErrors indicate serious errors with the VXIjsi layer.
  //
  //    VXIException::InterpreterEvent(EV_ERROR_ECMASCRIPT)
  //    VXIException::JavaScriptError();

public:
  Scripter(VXIjsiInterface* jsiapi);
  ~Scripter();

  bool IsVarDefined(const vxistring & name);
  // Determines whether or not the named variable is defined.
  //
  // Returns: true if variable has been defined
  //          false otherwise

  void MakeVar(const vxistring & name, const vxistring & expr);
  void SetVar(const vxistring & name, const vxistring & expr);
  // Creates a new variable or sets an existing variable to the indicated
  // expression.

  void ClearVar(const vxistring & name);
  // Sets a variable to 'undefined'.

  void SetString(const vxistring & var, const vxistring & val);
  // Creates a new variable or sets an existing variable to the indicated
  // string.

  void SetValue(const vxistring & var, const VXIValue * value);
  // Creates a new variable or sets an existing variable to the indicated
  // value type.  This may be used for complex types such as Object.

  VXIValue* GetValue(const vxistring & name) const;
  // Retrieves a variable from ECMAScript.  The conversion is as follows:
  //
  //   Undefined, Null       --> NULL pointer
  //   String                --> VXIString
  //   Boolean               --> VXIFloat or VXIInteger {true, false} -> {1, 0}
  //   Number                --> VXIFloat or VXIInteger
  //   Object                --> VXIMap
  //
  // NOTE: The caller must free the resulting VXIValue.

  void PushScope(const vxistring & name);
  // Pushes a new variable scope onto the stack with the given name.  New
  // variables will be defined in this scope, unless they are explicitly
  // referenced using <scope>.<variable_name> as in application.lastresult$.

  void PopScope();
  // Pops the current scope, destroying it and any associated variables.

  bool CurrentScope(const vxistring & name) const;
  // Checks the name of the current scope against the argument.
  //
  // Returns: true if the current scope matches the requested name.
  //          false otherwise

  void EvalScript(const vxistring & script);
  // Simply evaluates an ECMAScript script and returns nothing.

  VXIint EvalScriptToInt(const vxistring & script);
  // Evaluates an ECMAScript script, and converts the result to an integer.

  void EvalScriptToString(const vxistring & script, vxistring & result);
  // Evaluates an ECMAScript script, and converts the result to a string.

  VXIValue * EvalScriptToValue(const vxistring & script);
  // Evaluates an ECMAScript script, and converts the result to a string.
  // NOTE: The caller must free the resulting VXIValue.

  bool TestCondition(const vxistring & script);
  // Evaluates an expression as a boolean.

private:
  void maybe_throw_js_error(int err) const;

  Scripter(const Scripter &);                 // no implementation
  Scripter & operator=(const Scripter &);     // no implementation

private:
  typedef std::vector<vxistring> SCOPESTACK;
  SCOPESTACK scopeStack;

  VXIjsiInterface * jsi_api;
  VXIjsiContext * jsi_context;
};

#endif

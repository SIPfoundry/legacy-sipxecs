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
 * This is largely a wrapper around the VXIjsi interface.  The limited
 * responsibilities of this layer include:
 *
 *   (1) storing the VXIjsiContext and current scope name;
 *   (2) generating error.semantic exceptions for ECMAScript failures;
 *   (3) converting vxistring's to const wchar_t *;
 *   (4) converting VXIValue types.
 *
 ***********************************************************************/

#include "Scripter.hpp"
#include "CommonExceptions.hpp"
#include "VXML.h"
#include "VXIjsi.h"
#include <sstream>

// This is a simple conversion tool from VXIString -> vxistring.
/// static vxistring toString(const VXIString * s)
/// {
///   if (s == NULL) return L"";
///   const VXIchar * temp = VXIStringCStr(s);
///   if (temp == NULL) return L"";
///   return temp;
/// }

/*****************************************************************************
 * Constructor/Destructor - here is where the VXIjsiContext is managed.
 *****************************************************************************/

Scripter::Scripter(VXIjsiInterface *jsiapi) : jsi_api(jsiapi)
{
  if (jsi_api == NULL)
    maybe_throw_js_error(VXIjsi_RESULT_INVALID_ARGUMENT);

  maybe_throw_js_error(jsi_api->CreateContext(jsi_api, &jsi_context));
}


Scripter::~Scripter()
{
  // UNUSED VARIABLE VXIjsiResult err =
                 jsi_api->DestroyContext(jsi_api, &jsi_context);
}

/*****************************************************************************
 * Raise javascript exeception on error
 ****************************************************************************/
void Scripter::maybe_throw_js_error(int err) const
{
  switch (err) {
  case VXIjsi_RESULT_SUCCESS:
    return;
  case VXIjsi_RESULT_FAILURE:           // Normal failure
  case VXIjsi_RESULT_NON_FATAL_ERROR:   // Non-fatal non-specific error
  case VXIjsi_RESULT_SYNTAX_ERROR:      // JavaScript syntax error
  case VXIjsi_RESULT_SCRIPT_EXCEPTION:  // JavaScript exception thrown
  case VXIjsi_RESULT_SECURITY_VIOLATION:// JavaScript security violation
    throw VXIException::InterpreterEvent(EV_ERROR_ECMASCRIPT);
  default:
    throw VXIException::JavaScriptError();
  }
}

/*****************************************************************************
 * Wrappers around basic JSI interface. VXI only call JavaScript through here.
 *****************************************************************************/
void Scripter::MakeVar(const vxistring & name, const vxistring & expr)
{
  VXIjsiResult err = jsi_api->CreateVarExpr(jsi_api, jsi_context,
                                            name.c_str(), expr.c_str());

  // This handles variables such as x.y which are invalid arguments.
  if (err == VXIjsi_RESULT_INVALID_ARGUMENT && !name.empty())
    err = VXIjsi_RESULT_FAILURE;

  maybe_throw_js_error(err);
}


void Scripter::SetVar(const vxistring & name, const vxistring & expr)
{
  VXIjsiResult err = jsi_api->SetVarExpr(jsi_api, jsi_context,
                                         name.c_str(), expr.c_str());
  maybe_throw_js_error(err);
}


void Scripter::ClearVar(const vxistring & name)
{
  VXIjsiResult err = jsi_api->SetVarExpr(jsi_api, jsi_context,
                                         name.c_str(), L"undefined");
  maybe_throw_js_error(err);
}


void Scripter::SetValue(const vxistring & var, const VXIValue * value)
{
  VXIjsiResult err = jsi_api->SetVarValue(jsi_api, jsi_context,
                                          var.c_str(), value);
  maybe_throw_js_error(err);
}


VXIValue * Scripter::GetValue(const vxistring & name) const
{
  VXIValue* val = NULL;
  VXIjsiResult err = jsi_api->GetVar(jsi_api, jsi_context, name.c_str(), &val);

  switch (err) {
  case VXIjsi_RESULT_FAILURE:         // ECMAScript type Null
  case VXIjsi_RESULT_NON_FATAL_ERROR: // ECMAScript type Undefined
  case VXIjsi_RESULT_SUCCESS:
    break;
  default:
    maybe_throw_js_error(err);
  }

  return val;
}


void Scripter::SetString(const vxistring & name, const vxistring & val)
{
  vxistring line = val;

  // Escape the ' characters.
  const VXIchar APOSTROPHE = '\'';
  const VXIchar * const SLASH = L"\\";

  vxistring::size_type pos = line.find(APOSTROPHE);
  if (pos != vxistring::npos) {
    unsigned int size = line.size();
    do {
      line.insert(pos, SLASH);    // Convert ' -> \'
      if (++pos == size) break;   // Stop if last character was replaced.
      ++size;                     // Update the size.
      pos = line.find(APOSTROPHE, pos + 1);
    } while (pos != vxistring::npos);
  }

  // Pass the expression to ECMAScript.
  line = name + L" = '" + line + L"';";
  VXIjsiResult err = jsi_api->Eval(jsi_api, jsi_context, line.c_str(), NULL);
  maybe_throw_js_error(err);
}


void Scripter::PushScope(const vxistring & name)
{
  VXIjsiResult err = jsi_api->PushScope(jsi_api, jsi_context, name.c_str());
  if (err == VXIjsi_RESULT_SUCCESS)
    scopeStack.push_back(name);

  maybe_throw_js_error(err);
}


void Scripter::PopScope()
{
  VXIjsiResult err = jsi_api->PopScope(jsi_api, jsi_context);
  if (err == VXIjsi_RESULT_SUCCESS)
    scopeStack.pop_back();

  maybe_throw_js_error(err);
}


bool Scripter::CurrentScope(const vxistring & name) const
{
  return name == scopeStack.back();
}


bool Scripter::IsVarDefined(const vxistring & name)
{
  switch (jsi_api->CheckVar(jsi_api, jsi_context, name.c_str())) {
  case VXIjsi_RESULT_SUCCESS:
    return true;
  case VXIjsi_RESULT_FAILURE:
    return false;
  default:
    throw VXIException::JavaScriptError();
  }

  // We should never get here.
  return false;
}


bool Scripter::TestCondition(const vxistring & script)
{
  return (EvalScriptToInt(script) != 0);
}


VXIint Scripter::EvalScriptToInt(const vxistring & expr)
{
  VXIValue* val = NULL;
  VXIjsiResult err = jsi_api->Eval(jsi_api, jsi_context, expr.c_str(), &val);

  VXIint result = 0;

  if (err == VXIjsi_RESULT_SUCCESS) {
    switch (VXIValueGetType(val)) {
    case VALUE_INTEGER:
      result = VXIIntegerValue(reinterpret_cast<const VXIInteger*>(val));
      break;
    case VALUE_FLOAT:
      // Convert float down to VXIint.
      result = VXIint(VXIFloatValue(reinterpret_cast<const VXIFloat*>(val)));
      break;
    default:
      err = VXIjsi_RESULT_FAILURE;
      break;
    }
  }

  VXIValueDestroy(&val);
  maybe_throw_js_error(err);

  return result;
}


void Scripter::EvalScriptToString(const vxistring & expr, vxistring & result)
{
  VXIValue * val = NULL;
  VXIjsiResult err = jsi_api->Eval(jsi_api, jsi_context, expr.c_str(), &val);

  std::basic_ostringstream<wchar_t> os;

  if (err == VXIjsi_RESULT_SUCCESS) {
    switch (VXIValueGetType(val)) {
    case VALUE_INTEGER:
      os << VXIIntegerValue(reinterpret_cast<const VXIInteger*>(val));
      break;
    case VALUE_FLOAT:
      os << VXIFloatValue(reinterpret_cast<const VXIFloat*>(val));
      break;
    case VALUE_STRING:
      std::fixed(os);
      os << VXIStringCStr(reinterpret_cast<const VXIString *>(val));
      break;
    default:
      err = VXIjsi_RESULT_FAILURE;
      break;
    }
  }

  VXIValueDestroy(&val);
  result = os.str();

  maybe_throw_js_error(err);
}


void Scripter::EvalScript(const vxistring & expr)
{
  VXIjsiResult err = jsi_api->Eval(jsi_api, jsi_context, expr.c_str(), NULL);
  maybe_throw_js_error(err);
}


VXIValue * Scripter::EvalScriptToValue(const vxistring & expr)
{
  VXIValue* val = NULL;
  VXIjsiResult err = jsi_api->Eval(jsi_api, jsi_context, expr.c_str(), &val);
  maybe_throw_js_error(err);
  return val;
}


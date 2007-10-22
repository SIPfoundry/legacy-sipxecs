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
 ***********************************************************************/

#include "VXI.hpp"
#include "DocumentParser.hpp"
#include "SimpleLogger.hpp"
#include "Scripter.hpp"                // for use in ExecutionContext
#include "GrammarManager.hpp"          // for use in ExecutionContext
#include "Counters.hpp"                // for EventCounter
#include "PropertyList.hpp"            // for PropertyList
#include "PromptManager.hpp"           // for PromptManager
#include "VXIinet.h"                   // for VXIinetInterface
#include "VXML.h"                      // for node & attribute names
#include "VXItel.h"                    // for VXItelInterface
#include "VXIrec.h"                    // for parameters from rec answer
#include "VXIlog.h"                    // for event IDs
#include "VXIobject.h"                 // for Object type
#include <deque>                       // for deque (used by Prompts)
#include <sstream>                     // by ProcessNameList()
#include <algorithm>                   // for sort, set_intersection, etc.

// TBD #pragma message ("Cleanup comments - Did FIA level functions")

static const wchar_t * const SCOPE_Session       = L"session";
static const wchar_t * const SCOPE_Application   = L"application";
static const wchar_t * const SCOPE_Defaults      = L"$_platDefaults_";
static const wchar_t * const SCOPE_Document      = L"document";
static const wchar_t * const SCOPE_Dialog        = L"dialog";
static const wchar_t * const SCOPE_Local         = L"local";
static const wchar_t * const SCOPE_Anonymous     = L"$_execAnonymous_";

static const wchar_t * const GENERIC_DEFAULTS    = L"*";



// TBD #pragma message ("VXI:: Select better values for these.")
const int MAX_LOOP_ITERATIONS = 20;
const int MAX_DOCUMENTS = 500;
const int MAX_EXE_STACK_DEPTH = 20;

// ------*---------*---------*---------*---------*---------*---------*---------

static vxistring toString(const VXIString * s)
{
  if (s == NULL) return L"";
  const VXIchar * temp = VXIStringCStr(s);
  if (temp == NULL) return L"";
  return temp;
}

static vxistring toString(const VXIchar * s)
{
  if (s == NULL) return L"";
  return s;
}

// ------*---------*---------*---------*---------*---------*---------*---------

class AnswerInformation {
public:
  RecognitionAnswer recAnswer;
  bool usedDTMF;
  VXMLElement element;
  VXMLElement dialog;

  AnswerInformation(RecognitionAnswer & a, bool u,
                    VXMLElement & e, VXMLElement & d)
    : recAnswer(a), usedDTMF(u), element(e), dialog(d)  { }
};

class JumpReturn {
public:
  JumpReturn() { }
};

class JumpItem {
public:
  VXMLElement item;
  JumpItem(const VXMLElement & i) : item(i) { }
};

class JumpDialog {
public:
  VXMLElement dialog;
  JumpDialog(const VXMLElement d) : dialog(d)  { }
};

class JumpDoc {
public:
  VXMLElement  defaults;
  VXMLDocument application;
  vxistring    applicationURI;
  VXMLDocument document;
  VXMLElement  documentDialog;
  bool         isSubdialog;
  PropertyList properties;
  
  JumpDoc(const VXMLElement & def, const VXMLDocument & app,
          const vxistring & uri, const VXMLDocument & doc,
          const VXMLElement & docdial, bool issub, const PropertyList & p)
    : defaults(def), application(app), applicationURI(uri),
      document(doc), documentDialog(docdial), isSubdialog(issub), properties(p)
  { }

  ~JumpDoc() { }
};

//#############################################################################
// ExecutionContext & Utilities
//#############################################################################

static std::basic_ostream<VXIchar>& operator<<(std::basic_ostream<VXIchar>& os,
                                               const VXIValue * val)
{
  if (val == NULL)
    return os << L"NULL";

  switch (VXIValueGetType(val)) {
  case VALUE_INTEGER:
    return os << VXIIntegerValue(reinterpret_cast<const VXIInteger*>(val));
  case VALUE_FLOAT:
    return os << VXIFloatValue(reinterpret_cast<const VXIFloat*>(val));
  case VALUE_STRING:
    return os << VXIStringCStr(reinterpret_cast<const VXIString *>(val));
  case VALUE_VECTOR:
    {
      const VXIVector * v = reinterpret_cast<const VXIVector *>(val);
      const VXIunsigned len = VXIVectorLength(v);

      os << L"{ ";
      for (VXIunsigned i = 0; i < len; ++i) {
        if (i != 0) os << L", ";
        os << VXIVectorGetElement(v, i);
      }
      return os << L" }";
    }
  case VALUE_MAP:
    {
      const VXIMap * m = reinterpret_cast<const VXIMap *>(val);
      const VXIchar * key;
      const VXIValue * value;

      os << L"{ ";
      if (VXIMapNumProperties(m) != 0) {
        VXIMapIterator * i = VXIMapGetFirstProperty(m, &key, &value);
        os << L"(" << key << L", " << value << L")";
        
        while (VXIMapGetNextProperty(i, &key, &value)
               == VXIvalue_RESULT_SUCCESS)
          os << L", (" << key << L", " << value << L")";

        VXIMapIteratorDestroy(&i);
      }
      return os << L" }";
    }
  default:
    break;
  }

  return os << L"(unprintable result)";
}


class ExecutionContext {
public:
  // These are used by the main run loops, event handling, and subdialog.
  // Each of these gets initialized by InstallDocument.

  VXMLElement    platDefaults;

  VXMLDocument   application;
  vxistring      applicationURI;

  VXMLDocument   document;
  vxistring      documentName;

  VXMLElement    currentDialog;

  // Limited purpose members.
  VXMLElement    eventSource;        // For prompting (enumerate) during events
  VXMLElement    lastItem;           // Set by <subdialog>

public: // These are used more generally.
  Scripter       script;
  GrammarManager gm;
  PromptTracker  promptcounts;
  EventCounter   eventcounts;
  PropertyList   properties;

  typedef std::deque<vxistring> STRINGDEQUE;
  STRINGDEQUE formitems;

  bool playingPrompts;

  ExecutionContext * next;

  ExecutionContext(VXIrecInterface * r, VXIjsiInterface * j,
                   const SimpleLogger & l, ExecutionContext * n)
    : script(j), gm(r, l), properties(l), playingPrompts(true), next(n) { }
    // may throw VXIException::OutOfMemory()

  ~ExecutionContext() { }
};


typedef std::deque<vxistring> STRINGDEQUE;

static void ProcessNameList(const vxistring & namelist, STRINGDEQUE & names)
{
  names.clear();
  if (namelist.empty()) return;

  // The namelist is a series of whitespace delimited strings.  Read each in
  // and insert it into the deque.

  std::basic_stringstream<VXIchar> namestream(namelist);

#if defined(__GNUC__) && ((__GNUC__ == 2 && __GNUC_MINOR__ <= 95) || __GNUC__ >= 3)
  // G++ 2.95 does not support istream_iterator.
  while (namestream.good()) {
    vxistring temp;
    namestream >> temp;
    if (!temp.empty()) names.push_back(temp);
  }
#else
  std::copy(std::istream_iterator<vxistring, VXIchar>(namestream),
            std::istream_iterator<vxistring, VXIchar>(),
            std::back_inserter(names));
#endif

  // Now sort the deque and return the unique set of names.

  if (names.empty()) return;
  std::sort(names.begin(), names.end());
  STRINGDEQUE::iterator i = std::unique(names.begin(), names.end());
  if (i != names.end()) names.erase(i, names.end());
}

//#############################################################################
// Creation and Run
//#############################################################################

VXI::VXI()
  : parser(NULL),
    log(NULL), inet(NULL), rec(NULL), jsi(NULL), tel(NULL),
    sdParams(NULL), sdResult(NULL), stackDepth(0), exe(NULL)
{
  try {
    parser = new DocumentParser();
  }
  catch (const VXIException::OutOfMemory &) {
    parser = NULL;
    throw;
  }

  try {
    pm = new PromptManager();
  }
  catch (...) {
    delete parser;
    parser = NULL;
    pm = NULL;
    throw;
  }

  if (pm == NULL) {
    delete parser;
    parser = NULL;
    throw VXIException::OutOfMemory();
  }
}


VXI::~VXI()
{
  while (exe != NULL) {
    ExecutionContext * temp = exe->next;
    delete exe;
    exe = temp;
  }

  delete pm;
  pm = NULL;
  delete parser;
  parser = NULL;
}


bool VXI::SetRuntimeProperty(PropertyID id, const VXIchar * value)
{
  mutex.Lock();

  switch (id) {
  case VXI::BeepURI:
    if (value == NULL) uriBeep.erase();
    else uriBeep = value;
    break;
  case VXI::PlatDefaultsURI:
    if (value == NULL) uriPlatDefaults.erase();
    else uriPlatDefaults = value;
    break;
  default:
    mutex.Unlock();
    return false;
  }

  mutex.Unlock();
  return true;
}


void VXI::GetRuntimeProperty(PropertyID id, vxistring & value) const
{
  mutex.Lock();

  switch (id) {
  case VXI::BeepURI:           value = uriBeep;          break;
  case VXI::PlatDefaultsURI:   value = uriPlatDefaults;  break;
  default:                     value.erase();
  }

  mutex.Unlock();
}



int VXI::Run(const VXIchar * initialDocument,
             const VXIMap * args,
             SimpleLogger * resourceLog,
             VXIinetInterface * resourceInet,
             VXIjsiInterface * resourceJsi,
             VXIrecInterface * resourceRec,
             VXIpromptInterface * resourcePrompt,
             VXItelInterface * resourceTel,
             VXIobjectInterface * resourceObject,
             VXIValue ** resultValue)
{
  // (1) Check arguments.

  // (1.1) Check external resources
  if (resourceLog == NULL || resourceInet   == NULL || resourceJsi == NULL ||
      resourceRec == NULL || resourcePrompt == NULL || resourceTel == NULL)
    return 1;

  log = resourceLog;
  inet = resourceInet;
  jsi = resourceJsi;
  rec = resourceRec;
  tel = resourceTel;
  object = resourceObject;

  if (!pm->ConnectResources(log, resourcePrompt)) return 1;

  if (log->IsLogging(2)) {
    log->StartDiagnostic(2) << L"VXI::Run(" << initialDocument << L")";
    log->EndDiagnostic();
  }

  // (1.2) Check document
  if (initialDocument == NULL || wcslen(initialDocument) == 0) {
    log->LogError(201);
    return 1;
  }

  // (2) Delegate real work to RunOuterLoop & handle the serious errors.
  int exitCode;
  try {
    exitCode = RunOuterLoop(initialDocument, args, resultValue);
    pm->PlayAll();
  }
  catch (VXIException::InterpreterEvent & e) {
    log->LogError(207, SimpleLogger::EXCEPTION, e.GetValue().c_str());
    exitCode = 0;
  }
  catch (const VXIException::Exit &) {
    exitCode = 0;
  }
  catch (const VXIException::Fatal &) {
    log->LogError(209);
    exitCode = -2;
  }
  catch (const VXIException::OutOfMemory &) {
    PopExecutionContext();
    log->LogError(202);
    exitCode = 1;
  }
  catch (const VXIException::JavaScriptError &) {
    log->LogError(212);
    exitCode = -2;
  }
  catch (const JumpDialog &) {
    log->LogError(999, SimpleLogger::MESSAGE, L"unexpected jump to a dialog");
    exitCode = -2;
  }
  catch (const JumpDoc &) {
    log->LogError(999, SimpleLogger::MESSAGE,L"unexpected jump to a document");
    exitCode = -2;
  }
  catch (const JumpItem &) {
    log->LogError(999, SimpleLogger::MESSAGE, L"unexpected jump to an item");
    exitCode = -2;
  }
  catch (const JumpReturn &) {
    log->LogError(999, SimpleLogger::MESSAGE,
                  L"unexpected jump from a return element");
    exitCode = -2;
  }

  try {
    while (exe != NULL)
      PopExecutionContext();
  }
  catch (const VXIException::JavaScriptError &) {
    log->LogError(212);
    return -2;
  }
    

  return exitCode;

}


int VXI::RunOuterLoop(const vxistring & initialDocument,
                      const VXIMap * args,
                      VXIValue ** resultValue)
{
  // (1) Load the document containing the default handlers.

  log->LogDiagnostic(2, L"VXI::RunOuterLoop - loading defaultDoc.");

  // (1.1) Get URI if possible.

  vxistring defaultsUri;
  GetRuntimeProperty(VXI::PlatDefaultsURI, defaultsUri);
  
  // (1.2) Load platform defaults.

  try {
    VXIMapHolder tmpVXIMapHolder(NULL);
    VXIMapHolder domDefaultProp;
    AttemptDocumentLoad(defaultsUri, tmpVXIMapHolder, domDefaultDoc,
                        domDefaultProp, true);
  }
  catch (const VXIException::InterpreterEvent & e) {
    log->LogError(221, SimpleLogger::EXCEPTION, e.GetValue().c_str());
    return 3;
  }

  // (2) Create a new execution context and initialize the defaults.

  // (2.1) Create new execution context.
  if (!PushExecutionContext(args)) return -1;

  // (2.1) Find generic language properties.
  VXMLElement defaultsRoot = domDefaultDoc.GetRoot();
  for (VXMLNodeIterator it(defaultsRoot); it; ++it) {
    VXMLNode child = *it;
    if (child.GetType() != VXMLNode::Type_VXMLElement) continue;
    const VXMLElement & elem = reinterpret_cast<const VXMLElement &>(child);
    if (elem.GetName() != DEFAULTS_LANGUAGE) continue;

    vxistring id;
    elem.GetAttribute(ATTRIBUTE_ID, id);
    if (id != GENERIC_DEFAULTS) continue;
    exe->platDefaults = elem;
    break;
  }

  if (exe->platDefaults == 0) {
    log->LogError(221, SimpleLogger::EXCEPTION,
                  L"error.semantic.missing_generic_defaults");
    PopExecutionContext();
    return 3;
  }

  // (2.2) Install defaults.  We only need to worry about the properties (for
  // document load) and the ECMA variables and scripts (for catch handlers on
  // load failure).  The grammars & prompts may be safely ignored.

  VXIMapHolder tmpVXIMapHolder;
  exe->properties.SetProperties(exe->platDefaults, DEFAULTS_PROP, tmpVXIMapHolder);
  exe->script.PushScope(SCOPE_Application);
  exe->script.PushScope(SCOPE_Defaults);
  ProcessRootScripts(exe->platDefaults);

  bool firstTime = true;
  int loopCount = 0;

  // (3) Start the loops through contexts and documents.

  int result;
  while (1) {
    // (3.1) Check loop count. Exit if exceeded
    if (++loopCount > MAX_DOCUMENTS) {
      log->LogError(210);
      result = 1;
      break;
    }

    try {
      if (firstTime) {
        firstTime = false;
        // (3) Jump to the initial document.  Any events which occur while
        // loading the initial document are handled by the defaults.
        log->LogDiagnostic(2,L"VXI::RunOuterLoop - loading initial document.");

        try {
          DoOuterJump(exe->platDefaults, initialDocument);
        }
        catch (const VXIException::InterpreterEvent & e) {
          DoEvent(exe->platDefaults, e);
          PopExecutionContext();
          throw VXIException::Exit(NULL);
        }
      }

      log->LogDiagnostic(2, L"VXI::RunOuterLoop - new document");

      RunInnerLoop();
      break;
    }
    catch (JumpDoc & e) {
      if (e.isSubdialog)
        if (!PushExecutionContext(args)) return -1;

      InstallDocument(e);
    }
    catch (JumpReturn &) {
      PopExecutionContext();
    }
    catch (const VXIException::Exit & e) { // This handles <exit>.
      if (resultValue != NULL) *resultValue = e.exprResult;
      break;
    }
  }

  PopExecutionContext();
  return 0;
}


void VXI::InstallDocument(JumpDoc & e)
{
  // (1) Check to see what needs to be initialized.
  bool reinitApplication
    = e.isSubdialog || (e.applicationURI != exe->applicationURI);
  bool reinitDefaults
    = reinitApplication || (e.defaults != exe->platDefaults);

  // (2) Set the easy stuff.
  exe->platDefaults   = e.defaults;
  exe->application    = e.application;
  exe->applicationURI = e.applicationURI;
  exe->document       = e.document;
  exe->currentDialog  = e.documentDialog;
  exe->eventSource    = VXMLElement();
  exe->properties     = e.properties;

  exe->gm.ReleaseGrammars();

  VXMLElement documentRoot = exe->document.GetRoot();
  VXMLElement applicationRoot = exe->application.GetRoot();

  // (3) Load grammars.  The grammars are reloaded using the current
  // understanding of the properties.  During activation, the actual
  // proerties may differ slightly.

  try {
    exe->documentName = L"";
    exe->gm.LoadGrammars(documentRoot, exe->documentName, exe->properties);
  }
  catch (const VXIException::InterpreterEvent & e) {
    DoEvent(documentRoot, e);
    throw VXIException::Exit(NULL);
  }

  try {
    vxistring temp;
    exe->gm.LoadGrammars(exe->platDefaults, temp, exe->properties);
  }
  catch (const VXIException::InterpreterEvent & e) {
    DoEvent(exe->platDefaults, e);
    throw VXIException::Exit(NULL);
  }

  try {
    vxistring temp;
    exe->gm.LoadGrammars(applicationRoot, temp, exe->properties);
  }
  catch (const VXIException::InterpreterEvent & e) {
    DoEvent(applicationRoot, e);
    throw VXIException::Exit(NULL);
  }

    // (4) Clear existing ECMA script scopes.

  Scripter & script = exe->script;

  if (script.CurrentScope(SCOPE_Anonymous)) script.PopScope();
  if (script.CurrentScope(SCOPE_Local))     script.PopScope();
  if (script.CurrentScope(SCOPE_Dialog))    script.PopScope();
  if (script.CurrentScope(SCOPE_Document))  script.PopScope();

  if (script.CurrentScope(SCOPE_Defaults) && reinitDefaults)
    script.PopScope();
  if (script.CurrentScope(SCOPE_Application) && reinitApplication)
    script.PopScope();

  if (reinitApplication && !script.CurrentScope(SCOPE_Session)) {
    log->LogError(999,SimpleLogger::MESSAGE,L"ECMA Script scope inconsistent");
    throw VXIException::Fatal();
  }

  // (5) And set the new ones.
  try {
    if (script.CurrentScope(SCOPE_Session)) {
      script.PushScope(SCOPE_Application);
      ProcessRootScripts(applicationRoot);
    }
  }
  catch (const VXIException::InterpreterEvent & e) {
    DoEvent(applicationRoot, e);
    throw VXIException::Exit(NULL);
  }

  try {
    if (script.CurrentScope(SCOPE_Application)) {
      script.PushScope(SCOPE_Defaults);
      ProcessRootScripts(exe->platDefaults);
    }
  }
  catch (const VXIException::InterpreterEvent & e) {
    DoEvent(exe->platDefaults, e);
    throw VXIException::Exit(NULL);
  }

  try {
    script.PushScope(SCOPE_Document);
    ProcessRootScripts(documentRoot);
  }
  catch (const VXIException::InterpreterEvent & e) {
    DoEvent(documentRoot, e);
    throw VXIException::Exit(NULL);
  }
}


void VXI::DoOuterJump(const VXMLElement & elem,
                      const vxistring & rawURI,
                      VXIMap * rawSubmitData,
                      bool isSubdialog)
{
  if (log->IsLogging(2)) {
    log->StartDiagnostic(2) << L"VXI::DoOuterJump(" << rawURI << L")";
    log->EndDiagnostic();
  }

  VXIMapHolder submitData(rawSubmitData);

  // (1) Determine fetch properties for document load.

  // (1.1) Create empty fetch object.  Now we need to fill this in.
  VXIMapHolder fetchobj;
  if (fetchobj.GetValue() == NULL) throw VXIException::OutOfMemory();

  // (1.2) Set URIs for the Jump.
  vxistring uri(rawURI);
  vxistring fragment;

  // (1.2.1) Divide raw URI into uri + fragment.
  exe->properties.GetFetchobjURIs(elem, fetchobj, uri, fragment);

  // (1.2.2) Handle the (rare) degerate case.
  if (uri.empty() && fragment.empty()) {
    log->StartDiagnostic(0) << L"VXI::DoOuterJump - invalid URI, \""
                            << rawURI << L"\"";
    log->EndDiagnostic();
    throw VXIException::InterpreterEvent(EV_ERROR_BADURI);
  }

  // (1.2.3) In the fragment only case, just go to the indicated item.
  if (uri.empty()) {
    VXMLElement targetElement = FindDialog(elem, fragment);
    if (targetElement == 0) {
      log->StartDiagnostic(0) << L"VXI::DoOuterJump - non-existent dialog, \""
                              << rawURI << L"\"";
      log->EndDiagnostic();
      throw VXIException::InterpreterEvent(EV_ERROR_BADDIALOG);
    }

    if (!isSubdialog)
      throw JumpDialog(targetElement);
    else
      throw JumpDoc(exe->platDefaults, exe->application, exe->applicationURI,
                    exe->document, targetElement, isSubdialog,
                    exe->properties);
  }

  // TBD #pragma message ("VXI::DoOuterJump - ignoring fetchhint")
  // (1.3) Get remaining fetch properties.
  exe->properties.GetFetchobjCacheAttrs(elem, PropertyList::Document,fetchobj);

  if (submitData.GetValue() != NULL) {
    if (!exe->properties.GetFetchobjSubmitAttributes(elem, submitData,
                                                     fetchobj))
    {
      // This should never occur.
      log->StartDiagnostic(0) << L"VXI::DoOuterJump - couldn't set the submit "
                                 L"attributes.";
      log->EndDiagnostic();
      throw VXIException::InterpreterEvent(EV_ERROR_BADURI);
    }
    submitData.Release(); // This map is now owned by fetchobj.
  }

  // (2) Load Document.

  // (2.1) Start fetch audio.

  vxistring fetchaudio;
  if (!elem.GetAttribute(ATTRIBUTE_FETCHAUDIO, fetchaudio))
    fetchaudio = toString(exe->properties.GetProperty(L"fetchaudio"));

  if (!fetchaudio.empty())
    pm->PlayFiller(exe->properties, fetchaudio);

  // (2.2) Do real work

  VXMLDocument document;
  VXMLElement documentDialog;
  VXIMapHolder documentFetchProps;

  AttemptDocumentLoad(uri, fetchobj, document, documentFetchProps);

  VXMLElement documentRoot = document.GetRoot();

  documentDialog = FindDialog(documentRoot, fragment);
  if (documentDialog == 0) {
    if (fragment.empty())
      log->StartDiagnostic(0) << L"VXI::DoOuterJump - no dialog element found "
                                 L"in \"" << uri << L"\"";
    else 
      log->StartDiagnostic(0) << L"VXI::DoOuterJump - named dialog, "
                              << fragment << L" not found in \"" << uri<<L"\"";
    log->EndDiagnostic();
    throw VXIException::InterpreterEvent(EV_ERROR_BADDIALOG);
  }

  // (3) Get Document language & find associated defaults.

  // (3.1) Create a new property list containing the document properties.
  PropertyList newProperties(*log);

  // (3.2) Extract the language setting.
  const VXIchar * language = newProperties.GetProperty(PropertyList::Language);
  if (language == NULL) language = GENERIC_DEFAULTS;

  // (3.3) Find the language defaults.
  VXMLElement defaults;

  VXMLElement defaultsRoot = domDefaultDoc.GetRoot();
  for (VXMLNodeIterator it(defaultsRoot); it; ++it) {
    VXMLNode child = *it;

    // Look for a language node.
    if (child.GetType() != VXMLNode::Type_VXMLElement) continue;
    const VXMLElement & elem = reinterpret_cast<const VXMLElement &>(child);
    if (elem.GetName() != DEFAULTS_LANGUAGE) continue;

    vxistring id;
    elem.GetAttribute(ATTRIBUTE_ID, id);
    if (id == language || (id == GENERIC_DEFAULTS && defaults == 0))
      defaults = elem;
  }
  if (defaults != 0) {
    VXIMapHolder tmpVXIMapHolder(NULL);
    newProperties.SetProperties(defaults, DEFAULTS_PROP, tmpVXIMapHolder);
  }

  newProperties.SetProperties(documentRoot, DOC_PROP, documentFetchProps);

  // (4) Load the application.
  VXMLDocument application;
  VXIMapHolder appProperties;

  // (4.1) Get the application URI.
  vxistring appuri;
  documentRoot.GetAttribute(ATTRIBUTE_APPLICATION, appuri);
  if (!appuri.empty()) {
    // (4.2) Initialize application fetch parameters.
    VXIMapHolder appFetchobj;
    if (appFetchobj.GetValue() == NULL) throw VXIException::OutOfMemory();

    vxistring appFragment;
    newProperties.GetFetchobjURIs(documentRoot, appFetchobj, appuri,
                                  appFragment);
    if (appuri.empty() || !appFragment.empty()) {
      log->LogError(214);
      throw VXIException::InterpreterEvent(EV_ERROR_APP_BADURI);
    }
    newProperties.GetFetchobjCacheAttrs(documentRoot, PropertyList::Document,
                                        appFetchobj);

    // (4.3) Load the application and its properties (we must then restore the
    // document properties.
    AttemptDocumentLoad(appuri, appFetchobj, application, appProperties);
    newProperties.SetProperties(application.GetRoot(), APP_PROP,appProperties);
    newProperties.SetProperties(documentRoot, DOC_PROP, documentFetchProps);

    // (4.4) Update appuri to absolute URI.
    const VXIValue* absuri = VXIMapGetProperty(appProperties.GetValue(),
                                               PropertyList::AbsoluteURI);
    if (VXIValueGetType(absuri) == VALUE_STRING)
      appuri = toString(reinterpret_cast<const VXIString *>(absuri));
  }

  // (5) Generate the final event.
  throw JumpDoc(defaults, application, appuri,
                document, documentDialog, isSubdialog, newProperties);
}


// Finds the named dialog in the document.  If the name is empty, the first
// item is returned.
//
VXMLElement VXI::FindDialog(const VXMLElement & start, const vxistring & name)
{
  if (log->IsLogging(2)) {
    log->StartDiagnostic(2) << L"VXI::FindDialog(" << name << L")";
    log->EndDiagnostic();
  }

  // (0) find the root node.
  VXMLElement doc;
  for (doc = start; doc != 0 && doc.GetName() != NODE_VXML;
       doc = doc.GetParent());
  if (doc == 0) return VXMLElement();

  // (1) Walk through all elements at this level and find match.
  for (VXMLNodeIterator it(doc); it; ++it) {
    VXMLNode child = *it;

    // (1.1) Only <form> & <menu> elements are considered.
    if (child.GetType() != VXMLNode::Type_VXMLElement) continue;
    const VXMLElement & elem = reinterpret_cast<const VXMLElement &>(child);
    VXMLElementType nodeName = elem.GetName();
    if (nodeName != NODE_FORM && nodeName != NODE_MENU) continue;

    // (1.2) If no dialog was specified, return the first one.
    if (name.empty()) return elem;

    // (1.3) Otherwise, look for an exact match.
    vxistring id;
    if (!elem.GetAttribute(ATTRIBUTE__ITEMNAME, id)) continue;

    if (name == id)
      return elem;
  }

  // (2) User attempted to GOTO to non-existant dialog or have an empty doc!
  log->LogDiagnostic(2, L"VXI::FindDialog - no match found.");

  return VXMLElement();
}


//#############################################################################
// Document Loop
//#############################################################################

// return success/failure (can't throw error here as caller needs
//  a chance to clean up
// Also initialize new context (session scope)
bool VXI::PushExecutionContext(const VXIMap * sessionArgs)
{
  log->LogDiagnostic(2, L"VXI::PushExecutionContext()");

  if (stackDepth >= MAX_EXE_STACK_DEPTH) {
    log->LogError(211);
    return false;
  }

  ExecutionContext * ep = new ExecutionContext(rec, jsi, *log, exe);
  if (ep == NULL) throw VXIException::OutOfMemory();

  exe = ep;
  ++stackDepth;

  // Init new context from channel
  exe->script.PushScope(SCOPE_Session);

  if (sessionArgs != NULL && VXIMapNumProperties(sessionArgs) != 0) {
    const vxistring SESSIONDOT = vxistring(SCOPE_Session) + L".";
    const VXIchar * key;
    const VXIValue * value;
    VXIMapIterator * i = VXIMapGetFirstProperty(sessionArgs, &key, &value);
    exe->script.SetValue(SESSIONDOT + key, value);

    while (VXIMapGetNextProperty(i, &key, &value) == VXIvalue_RESULT_SUCCESS)
      exe->script.SetValue(SESSIONDOT + key, value);

    VXIMapIteratorDestroy(&i);
  }

    log->LogDiagnostic(2, L"VXI::PushExecutionContext - session variables "
                       L"initialized");

  return true;
}


void VXI::PopExecutionContext()
{
  if (exe == NULL) return;
  ExecutionContext * current = exe;

  exe = current->next;
  --stackDepth;

  delete current;
  current = NULL;
}


void VXI::AttemptDocumentLoad(const vxistring & uri,
                              const VXIMapHolder & uriProperties,
                              VXMLDocument & doc,
                              VXIMapHolder & docProperties,
                              bool isDefaults)
{
  // (1) Create map to store document properties.
  if (docProperties.GetValue() == NULL) 
    throw VXIException::OutOfMemory();


  // (2) Fetch the document hinting current language as accepted language
  if (exe) {
    const VXIchar *lang = exe->properties.GetProperty(PropertyList::Language);
    if (lang != NULL) {
      VXIString *accept_lang = VXIStringCreate(lang);
      if (accept_lang == NULL) throw VXIException::OutOfMemory();
      VXIMapSetProperty(uriProperties.GetValue(), PropertyList::AcceptedLang, reinterpret_cast<VXIValue *>(accept_lang));
      log->StartDiagnostic(0) << L"VXI::AttemptDocumentLoad - (" << uri << L") accept-language = '" << VXIStringCStr(accept_lang) << L"'";
      log->EndDiagnostic();
    }
  }
  int result = parser->FetchDocument(uri.c_str(), uriProperties, inet,
                                     *log, doc, docProperties, isDefaults);

  // (3) Handle error conditions.
  switch (result) {
  case -1: // Out of memory
    throw VXIException::OutOfMemory();
  case 0:  // Success
    break;
  case 1: // Invalid parameter
  case 2: // Unable to open URI
    log->LogError(203, SimpleLogger::URI, uri.c_str());
    throw VXIException::InterpreterEvent(EV_ERROR_BADFETCH);
  case 3: // Unable to read from URI
    log->LogError(204, SimpleLogger::URI, uri.c_str());
    throw VXIException::InterpreterEvent(EV_ERROR_BADFETCH);
  case 4: // Unable to parse contents of URI
    log->LogError(205, SimpleLogger::URI, uri.c_str());
    throw VXIException::InterpreterEvent(EV_ERROR_BADFETCH);
  default:
    log->LogError(206, SimpleLogger::URI, uri.c_str());
    throw VXIException::Fatal();
  }

  if (doc.GetRoot() == 0) {
    log->LogError(999, SimpleLogger::MESSAGE,
                  L"able to fetch initial document but node empty");
    throw VXIException::Fatal();
  }
}


void VXI::ProcessRootScripts(VXMLElement& doc)
{
  if (doc == 0) return;

  log->LogDiagnostic(2, L"VXI::ProcessRootScripts()");

  // Do <var> <script> and <meta> <property> elements
  for (VXMLNodeIterator it(doc); it; ++it) {
    VXMLNode child = *it;

    if (child.GetType() != VXMLNode::Type_VXMLElement) continue;
    const VXMLElement & elem = reinterpret_cast<const VXMLElement &>(child);
    VXMLElementType nodeName = elem.GetName();
    if (nodeName == NODE_VAR)
      var_element(elem);
    else if (nodeName == NODE_META)
      meta_element(elem);
    else if (nodeName == NODE_SCRIPT)
      script_element(elem);
  }

  log->LogDiagnostic(2, L"VXI::ProcessRootScripts - done");
}


//#############################################################################
// Dialog Loop
//#############################################################################

// There are two cases in which this routine may be entered.
//   A. After a new document is loaded  (lastItem == 0)
//   B. After a return from <subdialog>
// and three ways in which the loop may be re-entered.
//   1. Jump to new dialog.
//   2. Jump to new form item in the existing dialog.
//   3. Jump to new dialog after recognizing a document scope grammar.
//
// The ECMA script scopes are reset accordingly:
//   anonymous: A B 1 2 3
//   local:     A   1 2 3
//   dialog:    A   1   3

void VXI::RunInnerLoop()
{
  log->LogDiagnostic(2, L"VXI::RunInnerLoop()");

  if (exe->currentDialog == 0) {
    log->LogError(999, SimpleLogger::MESSAGE, L"no current active document");
    return;
  }

  int loop_count = 0;
  VXMLElement item = exe->lastItem;
  exe->lastItem = VXMLElement();
  bool newDialog = (item == 0);
  exe->playingPrompts = true;

  // For passing answers.
  RecognitionAnswer unprocessedAnswer;
  bool usedDTMF = false;
  bool formLevelGrammar = false;

  while (1) {
    try {
      // (1) Check loop count. Throw event if exceeded, caught in DocumentLoop
      if (++loop_count > 4 * MAX_LOOP_ITERATIONS) {
        log->LogError(210);
        throw VXIException::InterpreterEvent(EV_ERROR_LOOP_COUNT);
      }

      try {
        if (exe->script.CurrentScope(SCOPE_Anonymous)) exe->script.PopScope();

        // (2) Initialize dialog (if necessary)
        if (newDialog) {
          newDialog = false;
          exe->playingPrompts = true;

          // (2.1) Reset ECMA script scope.
          if (exe->script.CurrentScope(SCOPE_Local))  exe->script.PopScope();
          if (exe->script.CurrentScope(SCOPE_Dialog)) exe->script.PopScope();
          exe->script.PushScope(SCOPE_Dialog);

          // (2.2) Do 'initialization phase' from FIA.
          VXIMapHolder params(sdParams);
          sdParams = NULL;
          FormInit(exe->currentDialog, params);

          // (2.3) Do 'select phase' from FIA if the item is not already known
          if (item == 0) {
            DoInnerJump(exe->currentDialog, L"");
            break;
          }
        }

        // (3) The loop cases.

        if (!unprocessedAnswer.keys.empty()) {
          // (3.1) Re-entering loop with an unprocessed recognition result.
          RecognitionAnswer temp = unprocessedAnswer;
          unprocessedAnswer.Clear();
          ProcessRecognitionResult(exe->currentDialog, 
                                   formLevelGrammar ? VXMLElement() : item,
                                   usedDTMF, temp);
        }

        else if (sdResult != NULL) {
          // (3.2) Re-entering loop after returning from a <subdialog>.
          VXIValue * temp = sdResult;
          sdResult = NULL;
          ProcessReturn(exe->currentDialog, item, temp);
        }

        else {
          // (3.3) Each time we enter collect phase, we get fresh local scope.
          // All filled and catched triggered form here will execute in this
          // scope.  The final local scope is popped with we leave.

          if (exe->script.CurrentScope(SCOPE_Local)) exe->script.PopScope();
          exe->script.PushScope(SCOPE_Local);
          
          // Do the 'collect phase & process phase' from the FIA.
          CollectPhase(exe->currentDialog, item);
        }
      }
      catch (const VXIException::InterpreterEvent & e) {
        // Handles document events.
        if (log->IsLogging(2)) {
          log->StartDiagnostic(2) << L"VXI::RunInnerLoop - got exception: "
                                  << e.GetValue();
          log->EndDiagnostic();
        }

        if (item != 0)
          DoEvent(item, e);
        else
          DoEvent(exe->currentDialog, e);
      }

      DoInnerJump(exe->currentDialog, L"");
      break;
    }
    catch (JumpDialog & e) { // Handle <goto> events
      exe->currentDialog = e.dialog;
      item = VXMLElement();
      newDialog = true;
    }
    catch (const JumpItem & e) { // This handles local <goto>s.
      item = e.item;
    }
    catch (AnswerInformation & e) {
      if (exe->currentDialog != e.dialog) {
        exe->currentDialog = e.dialog;
        item = e.element;
        newDialog = true;
      }
      unprocessedAnswer = e.recAnswer;
      usedDTMF = e.usedDTMF;
      formLevelGrammar = (e.dialog == e.element);
    }
  } // while (1)

  log->LogDiagnostic(2, L"VXI::RunInnerLoop - done");
}


void VXI::ProcessReturn(const VXMLElement& form, const VXMLElement & item,
                        VXIValue * & result)
{
  log->LogDiagnostic(2, L"VXI::ProcessReturn()");

  if (VXIValueGetType(result) == VALUE_STRING) {
    const VXIString * temp = reinterpret_cast<const VXIString *>(result);
    throw VXIException::InterpreterEvent(toString(temp));
  }
    
  vxistring filled;
  item.GetAttribute(ATTRIBUTE__ITEMNAME, filled);

  exe->script.SetValue(filled, result);
  VXIValueDestroy(&result);
  EasyFilled(filled, form);
}


// Perform initialization associated with property tags and form level
// variables.  Reset the event and prompts counts.
//
void VXI::FormInit(const VXMLElement & form, VXIMapHolder & params)
{
  log->LogDiagnostic(2, L"VXI::FormInit()");

  // (1) Set the form properties.
  VXIMapHolder tmpVXIMapHolder;
  exe->properties.SetProperties(form, DIALOG_PROP, tmpVXIMapHolder);

  // (2) Clear the prompt & event counts when the form is entered.
  exe->promptcounts.Clear();
  exe->eventcounts.Clear();

  exe->formitems.clear();

  // (3) Walk through the form nodes.  Set up variables as necessary.
  for (VXMLNodeIterator it(form); it; ++it) {
    VXMLNode child = *it;

    if (child.GetType() != VXMLNode::Type_VXMLElement) continue;
    const VXMLElement & elem = reinterpret_cast<const VXMLElement &>(child);
 
    // (3.1) Handle <var> elements.
    VXMLElementType nodeName = elem.GetName();
    if (nodeName == NODE_VAR) {
      // (3.1.1) We first follow the normal proceedure.
      var_element(elem);

      if (params.GetValue() != NULL) {
        // (3.1.2) Undefined variables get set to the value in the param list.
        // Each located parameter gets removed from the map.
        vxistring name;
        elem.GetAttribute(ATTRIBUTE_NAME, name);
        if (!name.empty() && !exe->script.IsVarDefined(name)) {
          const VXIValue * value = VXIMapGetProperty(params.GetValue(),
                                                     name.c_str());
          if (value != NULL) {
            exe->script.SetValue(name, value);
            VXIMapDeleteProperty(params.GetValue(), name.c_str());
          }
        }
      }
      continue;
    }

    // (3.2) Ignore anything which is not a form item.
    if (!IsFormItemNode(elem)) continue;

    // (3.3) Initialize variables for each form item.
    vxistring name;
    vxistring expr;
    elem.GetAttribute(ATTRIBUTE__ITEMNAME, name);
    elem.GetAttribute(ATTRIBUTE_EXPR, expr);
    exe->script.MakeVar(name, expr);
    exe->formitems.push_back(name);
  }

  // (4) Did all incoming parameters get used?
  if (params.GetValue() != NULL &&
      VXIMapNumProperties(params.GetValue()) != 0)
    throw VXIException::InterpreterEvent(EV_ERROR_SEMANTIC);

  log->LogDiagnostic(2, L"VXI::FormInit - Done");
}


// Returns true iff the element is a form item.
//
// This is the list from section 6.2 of the VXML 1.0 specification with one
// addition - <menu>.
//
bool VXI::IsFormItemNode(const VXMLElement & doc)
{
  VXMLElementType nodeName = doc.GetName();
  if (nodeName == NODE_FIELD  || nodeName == NODE_INITIAL   ||
      nodeName == NODE_RECORD || nodeName == NODE_TRANSFER  ||
      nodeName == NODE_OBJECT || nodeName == NODE_SUBDIALOG ||
      nodeName == NODE_MENU   || nodeName == NODE_BLOCK)
    return true;

  return false;
}


// Finds the named form item within the dialog.  If the name is empty, the
// first non-filled item is returned.
//
void VXI::DoInnerJump(const VXMLElement & elem, const vxistring & item)
{
  if (elem == 0) return;

  if (log->IsLogging(2)) {
    log->StartDiagnostic(2) << L"VXI::DoInnerJump(" << item << L")";
    log->EndDiagnostic();
  }

  // find form.
  VXMLElement current = elem;

  while (1) {
    VXMLElementType nodeName = current.GetName();
    if (nodeName == NODE_MENU)
      throw JumpItem(current); // Menu is a special case.
    if (nodeName == NODE_FORM)
      break;

    const VXMLElement & parent = current.GetParent();
    if (parent == 0) {
      log->LogError(999, SimpleLogger::MESSAGE,
                    L"could not locate form on local jump");
      throw VXIException::Fatal();
    }

    current = parent;
  };

  // (2) If the next item is specified (such as from a previous <goto>, look
  //     for an exact match.
  if (!item.empty()) {
    for (VXMLNodeIterator it(current); it; ++it) {
      VXMLNode child = *it;

      if (child.GetType() != VXMLNode::Type_VXMLElement) continue;
      const VXMLElement & elem = reinterpret_cast<const VXMLElement &>(child);

      if (!IsFormItemNode(elem)) continue;
      vxistring name;
      if (!elem.GetAttribute(ATTRIBUTE__ITEMNAME, name)) continue;
      if (item == name) throw JumpItem(elem);
    }
  }

  // (3) Otherwise, find the first non-filled item with a valid condition.
  else {
    for (VXMLNodeIterator it(current); it; ++it) {
      VXMLNode child = *it;

      if (child.GetType() != VXMLNode::Type_VXMLElement) continue;
      const VXMLElement & elem = reinterpret_cast<const VXMLElement &>(child);

      if (!IsFormItemNode(elem)) continue;

      // Must use itemname here, as could be implicit name
      vxistring itemname;
      if (!elem.GetAttribute(ATTRIBUTE__ITEMNAME, itemname)) {
        log->LogError(999, SimpleLogger::MESSAGE,
                      L"unnamed item found on local jump");
        throw VXIException::Fatal();
      }

      if (exe->script.IsVarDefined(itemname)) continue;

      // OK if var is undefined, check condition
      vxistring cond;
      elem.GetAttribute(ATTRIBUTE_COND, cond);
      if (cond.empty() || exe->script.TestCondition(cond))
        throw JumpItem(elem);
    }
  }

  log->LogDiagnostic(2, L"VXI::DoInnerJump - no match found.");
}

//#############################################################################
// Utility functions
//#############################################################################

// do_event() - top level call into event handler; deals with 
// event counts and defaults.
//
void VXI::DoEvent(const VXMLElement & item,
                  const VXIException::InterpreterEvent & e)
{
  // (0) Initial logging
  if (item == 0) {
    log->LogDiagnostic(0, L"VXI::DoEvent - invalid argument, ignoring event");
    return;
  }
  else if (log->IsLogging(2)) {
    log->StartDiagnostic(2) << L"VXI::DoEvent(" << e.GetValue() << L")";
    log->EndDiagnostic();
  }

  // (1) Disable queuing of prompts outside of event handler.
  exe->playingPrompts = false;

  int count = 0;
  VXIException::InterpreterEvent event = e;

  do {
    try {
      // (2) Increments counts associated with this event.
      exe->eventcounts.Increment(event.GetValue());

      // (3) Process the current event.
      exe->eventSource = item;
      bool handled = do_event(item, event);
      exe->eventSource = VXMLElement();

      if (handled) {
        if (log->IsLogging(2)) {
          log->StartDiagnostic(2) << L"VXI::DoEvent - event processed.";
          log->EndDiagnostic();
        }
        return;
      }

      // (4) No one willing to handle this event.  Exit.
      vxistring exitmessage(L"Unhandled exception: ");
      exitmessage += event.GetValue();
      VXIString * val = VXIStringCreate(exitmessage.c_str());
      if (val == NULL) throw VXIException::OutOfMemory();
      throw VXIException::Exit(reinterpret_cast<VXIValue*>(val));
    }

    // (5) The catch had a <throw> element inside.  Must process the new event.

    catch (const VXIException::InterpreterEvent & e) {
      event = e;
    }
  } while (++count < MAX_LOOP_ITERATIONS);
  
  // (6) Probable loop - catch X throws X?  Quit handling after a while. 

  vxistring exitmessage(L"Unhandled exception (suspected infinite loop)");
  VXIString * val = VXIStringCreate(exitmessage.c_str());
  if (val == NULL) throw VXIException::OutOfMemory();
  throw VXIException::Exit(reinterpret_cast<VXIValue*>(val));
}


bool VXI::do_event(const VXMLElement & item,
                   const VXIException::InterpreterEvent & e)
{
  const vxistring & event = e.GetValue();

  // (1) Define the variables for the best match.
  int bestCount = 0;
  vxistring::size_type bestMatchLength = 0;
  VXMLElement bestMatch;

  enum {
    DOCUMENT,
    APPLICATION,
    DEFAULTS
  } stage = DOCUMENT;

  bool done = false;

  // Start from current item in document.
  VXMLElement currentNode = item;

  do {
    // (2) Walk through all nodes at this level looking for catch elements.
    for (VXMLNodeIterator it(currentNode); it; ++it) {
      VXMLNode child = *it;

      if (child.GetType() != VXMLNode::Type_VXMLElement) continue;
      const VXMLElement & elem = reinterpret_cast<const VXMLElement &>(child);

      // (2.1) Can this node catch events?
      VXMLElementType nodeName = elem.GetName();
      if (nodeName != NODE_CATCH   && nodeName != NODE_ERROR   &&
          nodeName != NODE_HELP    && nodeName != NODE_NOINPUT &&
          nodeName != NODE_NOMATCH) continue;

      // (2.2) If it is not a catch, is this an active match?
      int eventCount = 0;

      vxistring catchEvent;

      if (nodeName != NODE_CATCH) {
        // Map back to strings.
        if      (nodeName == NODE_ERROR)   catchEvent = L"error";
        else if (nodeName == NODE_HELP)    catchEvent = L"help";
        else if (nodeName == NODE_NOINPUT) catchEvent = L"noinput";
        else if (nodeName == NODE_NOMATCH) catchEvent = L"nomatch";        

        eventCount = exe->eventcounts.GetCount(event, catchEvent);
        if (eventCount == 0) continue;
      }
      // (2.3) If it is a catch, we need to examine the list.
      else {
        elem.GetAttribute(ATTRIBUTE_EVENT, catchEvent);

        // Find the best match in the event list.
        STRINGDEQUE catchList;
        ProcessNameList(catchEvent, catchList);
        catchEvent.erase();

        if (catchList.empty())
          eventCount = exe->eventcounts.GetCount(event, catchEvent);
        else {
          STRINGDEQUE::iterator i;
          for (i = catchList.begin(); i != catchList.end(); ++i) {
            int temp = exe->eventcounts.GetCount(event, *i);
            if (temp != 0 && (*i).length() > catchEvent.length()) {
              catchEvent = (*i);
              eventCount = temp;
            }
          }
          if (catchEvent.empty()) continue;
        }
      }

      // (2.4) Matching catch element found.  Evaluate its 'cond' attribute.
      vxistring attr;
      elem.GetAttribute(ATTRIBUTE_COND, attr);
      if (!attr.empty() && !exe->script.TestCondition(attr))
        continue;

      // (2.5) Condition okay.  Check the count against the eventCount.
      int catchCount = 1;
      if (elem.GetAttribute(ATTRIBUTE_COUNT, attr)) {
#if defined(__GNUC__) && (__GNUC__ <= 2 || (__GNUC__ == 3 && __GNUC_MINOR__ == 0))
        // The G++ implementation of basic_stringstream is faulty.
        VXIchar * temp;
        catchCount = int(wcstol(attr.c_str(), &temp, 10));
#else
        std::basic_stringstream<VXIchar> attrStream(attr);
        attrStream >> catchCount;
#endif
      }
      if (catchCount < 1 || catchCount > eventCount) continue;

      // (2.6) We now have a candidate.
      vxistring::size_type catchEventLength = catchEvent.length();
      if (catchEventLength < bestMatchLength) continue; // Name isn't as good.
      if (catchEventLength == bestMatchLength && catchCount <= bestCount)
        continue;  // Names match, but count isn't better.

      // (2.7) Update best match.
      bestCount = catchCount;
      bestMatchLength = catchEventLength;
      bestMatch = elem;
    }

    // (3) Decide where to search next.
    const VXMLElement & parent = currentNode.GetParent();
    if (parent != 0)
      currentNode = parent;
    else {
      if (stage == DOCUMENT) {
        stage = APPLICATION;
        currentNode = exe->application.GetRoot();
        if (currentNode != 0) continue;
        // Otherwise, fall through to application level.
      }
      if (stage == APPLICATION) {
        stage = DEFAULTS;

        // We resort to this level _only_ if no match has been found.
        if (bestCount < 1) {
          vxistring language =
            toString(exe->properties.GetProperty(PropertyList::Language));

          // We clear the current node.  It will be set either to the global
          // language (*) or to an exact match.
          currentNode = VXMLElement();

          VXMLElement defaultsRoot = domDefaultDoc.GetRoot();
          for (VXMLNodeIterator it(defaultsRoot); it; ++it) {
            VXMLNode child = *it;

            // Look for a language node.
            if (child.GetType() != VXMLNode::Type_VXMLElement) continue;
            
            const VXMLElement & elem
              = reinterpret_cast<const VXMLElement &>(child);
            if (elem.GetName() != DEFAULTS_LANGUAGE) continue;

            vxistring id;
            elem.GetAttribute(ATTRIBUTE_ID, id);
            if (id == language || (id == GENERIC_DEFAULTS && currentNode == 0))
              currentNode = elem;
          }

          if (currentNode != 0) continue;
        }
      }
      done = true;
    }
  } while (!done);

  if (bestCount == 0)
    return false;

  // Compliance Note:
  //
  // Because of the 'as-if-by-copy' semantic, we now execute the catch content,
  // including relative URIs, in the local scope.  So nothing special needs to
  // be done.

  VXIMapHolder vars;
  VXIValue * temp = NULL;

  temp = reinterpret_cast<VXIValue *>(VXIStringCreate(e.GetValue().c_str()));
  if (temp == NULL) throw VXIException::OutOfMemory();
  VXIMapSetProperty(vars.GetValue(), L"_event", temp);

  temp = reinterpret_cast<VXIValue *>(VXIStringCreate(e.GetMessage().c_str()));
  if (temp == NULL) throw VXIException::OutOfMemory();
  VXIMapSetProperty(vars.GetValue(), L"_message", temp);

  execute_content(bestMatch, vars);
  return true;
}


// Top level call into executable content section.
// Called from <block>,<catch>, and <filled>
//
void VXI::execute_content(const VXMLElement& doc, const VXIMapHolder & vars)
{
  log->LogDiagnostic(2, L"VXI::execute_content()");

  // (1) Add a new scope.  The allows anonymous variables to be defined. 
  if (exe->script.CurrentScope(SCOPE_Anonymous)) exe->script.PopScope();
  exe->script.PushScope(SCOPE_Anonymous);

  // (2) Set externally specified variables (if necessary).
  if (vars.GetValue() != NULL) {
    const VXIchar * key;
    const VXIValue * value;
    VXIMapIterator * i = VXIMapGetFirstProperty(vars.GetValue(), &key, &value);
    exe->script.SetValue(key, value);

    while (VXIMapGetNextProperty(i, &key, &value) == VXIvalue_RESULT_SUCCESS)
      exe->script.SetValue(key, value);

    VXIMapIteratorDestroy(&i);
  }

  // (3) Walk through the children and execute each node.
  for (VXMLNodeIterator it(doc); it; ++it) {
    VXMLNode child = *it;

    switch (child.GetType()) {
    case VXMLNode::Type_VXMLContent:
      executable_prompt(child);
      break;
    case VXMLNode::Type_VXMLElement:
    {
      const VXMLElement & elem = reinterpret_cast<VXMLElement &>(child);
      executable_element(elem);
      break;
    }
    default: // do nothing
      break;
    }
  }
}


// Executable element dispatch
//
void VXI::executable_element(const VXMLElement & elem)
{
  if (elem == 0) {
    log->LogError(999, SimpleLogger::MESSAGE, L"empty executable element");
    return;
  }

  VXMLElementType nodeName = elem.GetName();

  if (log->IsLogging(2)) {
    log->StartDiagnostic(2) << L"VXI::executable_element - " << nodeName;
    log->EndDiagnostic();
  }

  if (nodeName == NODE_VAR)
    var_element(elem);
  else if (nodeName == NODE_ASSIGN)
    assign_element(elem);
  else if (nodeName == NODE_CLEAR)
    clear_element(elem);
  else if (nodeName == NODE_DISCONNECT)
    disconnect_element(elem);
  else if (nodeName == NODE_EXIT)
    exit_element(elem);
  else if (nodeName == NODE_GOTO)
    goto_element(elem);
  else if (nodeName == NODE_IF)
    if_element(elem);
  else if (nodeName == NODE_LOG)
    log_element(elem);
  else if (nodeName == NODE_PROMPT || nodeName == NODE_AUDIO ||
           nodeName == NODE_VALUE  || nodeName == NODE_ENUMERATE)
    executable_prompt(elem);
  else if (nodeName == NODE_REPROMPT)
    reprompt_element(elem);
  else if (nodeName == NODE_RETURN)
    return_element(elem);
  else if (nodeName == NODE_SCRIPT)
    script_element(elem);
  else if (nodeName == NODE_SUBMIT)
    submit_element(elem);
  else if (nodeName == NODE_THROW)
    throw_element(elem);
  else
    log->LogError(999, SimpleLogger::MESSAGE,L"unexpected executable element");
}


/* 
 * Process <var> elements in current interp context.
 *
 * This differs from assign in that it makes new var in current scope
 * assign follows scope chain lookup for var name (and throws 
 *   error if fails)
 *
 * This is also used for initialiation of guard vars in field items
 *
 * <var> processing is compliated by the need to check for <param> values
 *  from subdialog calls.
 */
void VXI::var_element(const VXMLElement & doc)
{
  vxistring name;
  vxistring expr;
  doc.GetAttribute(ATTRIBUTE_NAME, name);
  doc.GetAttribute(ATTRIBUTE_EXPR, expr);

  if (log->IsLogging(2)) {
    log->StartDiagnostic(2) << L"VXI::var_element(name=\"" << name
                            << L"\" expr = \"" << expr << L"\")";
    log->EndDiagnostic();
  }

  if (name.empty()) return;
  exe->script.MakeVar(name, expr);
}

 
// Process <assign> elements in current interpreter context
//
void VXI::assign_element(const VXMLElement & doc)
{
  vxistring name;
  doc.GetAttribute(ATTRIBUTE_NAME, name);
  if (name.empty()) return;

  vxistring expr;
  doc.GetAttribute(ATTRIBUTE_EXPR, expr);

  if (log->IsLogging(2)) {
    log->StartDiagnostic(2) << L"VXI::assign_element(name=\"" << name
                            << L"\" expr = \"" << expr << L"\"";
    log->EndDiagnostic();
  }

  exe->script.SetVar(name, expr);
}



// Handler for meta elements. Do nothing for now.
//
void VXI::meta_element(const VXMLElement & doc)
{
  // vxistring & name = doc.GetAttribute("name");
  // vxistring & name = doc.GetAttribute("content");
  // vxistring & name = doc.GetAttribute("http-equiv");
}


// Handler for clear elements.  This may resets all form items or a user
// specified subset.
//
void VXI::clear_element(const VXMLElement & doc)
{
  log->LogDiagnostic(2, L"VXI::clear_element()");

  // (1) Get the namelist.
  vxistring namelist;
  doc.GetAttribute(ATTRIBUTE_NAMELIST, namelist);

  // (2) Handle the easy case: empty namelist --> clear all
  if (namelist.empty()) {
    // (2.1) Clear prompt and event counts.
    exe->promptcounts.Clear();
    exe->eventcounts.Clear();

    // (2.2) The list of form items resides in the slot map.
    ExecutionContext::STRINGDEQUE::iterator i;
    ExecutionContext::STRINGDEQUE & formitems = exe->formitems;
    for (i = formitems.begin(); i != formitems.end(); ++i)
      exe->script.ClearVar(*i);

    return;
  }

  // (3) Handle case where user specifies form items.

  STRINGDEQUE names;
  ProcessNameList(namelist, names);

  for (STRINGDEQUE::const_iterator i = names.begin(); i != names.end(); ++i) {
    // (3.1) Check that the name is a real form item.  A linear search should
    //       be sufficently fast.
    ExecutionContext::STRINGDEQUE::iterator j;
    ExecutionContext::STRINGDEQUE & formitems = exe->formitems;
    for (j = formitems.begin(); j != formitems.end(); ++j)
      if (*i == *j) break;

    if (j == formitems.end()) {
      log->StartDiagnostic(0) << L"VXI::clear_element - user attempted to "
        L"clear '" << (*i) << L"', but this is not a form item.";
      log->EndDiagnostic();
      continue;
    }

    // (3.2) Clear associated counters.
    exe->promptcounts.Clear(*i);
    exe->eventcounts.ClearIf(*i, true);
    exe->script.ClearVar(*i);
  }
}

// This implementation returns the values as VXIObjects.
//
void VXI::exit_element(const VXMLElement & doc)
{
  log->LogDiagnostic(2, L"VXI::exit_element()");

  VXIMapHolder exprMap(NULL); 

  vxistring namelist;
  doc.GetAttribute(ATTRIBUTE_NAMELIST, namelist);
  if (!namelist.empty()) {
    exprMap.Acquire(VXIMapCreate());
    if (exprMap.GetValue() == NULL) throw VXIException::OutOfMemory();

    STRINGDEQUE names;
    ProcessNameList(namelist, names);
    STRINGDEQUE::const_iterator i;
    for (i = names.begin(); i != names.end(); ++i) {
      VXIValue * val = exe->script.GetValue(*i);
      if (val != NULL)
        VXIMapSetProperty(exprMap.GetValue(), (*i).c_str(), val);
    }
  }

  VXIValue * exprResult = NULL;

  vxistring expr;
  doc.GetAttribute(ATTRIBUTE_EXPR, expr);
  if (!expr.empty()) {
    // To evaluate expressions is a bit more ugly.  Because any object may be
    // returned by the expression, we create a new variable, evaluate it, and
    // return the result.
    vxistring variable;
    VXMLDocumentModel::CreateHiddenVariable(variable);
    exe->script.MakeVar(variable, expr);
    exprResult = exe->script.GetValue(variable);
  }

  // Now combine the two into one result (if necessary)
  if (exprMap.GetValue() != NULL && exprResult != NULL) {
    // We insert the exprResult into the exprMap.
    VXIMapSetProperty(exprMap.GetValue(), L"Attribute_expr_value", exprResult);
    exprResult = NULL;
  }
  if (exprResult == NULL)
    exprResult = reinterpret_cast<VXIValue *>(exprMap.Release());

  if (log->IsLogging(2)) {
    log->StartDiagnostic(2) << L"exit_element got \"" << exprResult << L"\"";
    log->EndDiagnostic();
  }

  pm->PlayAll();

  throw VXIException::Exit(exprResult); // Stuff exprResult in here.
}


void VXI::disconnect_element(const VXMLElement & doc)
{
  log->LogDiagnostic(2, L"VXI::disconnect_element()");

  pm->PlayAll();

  if (tel->Disconnect(tel) != VXItel_RESULT_SUCCESS)
    log->LogError(213);

  throw VXIException::InterpreterEvent(EV_TELEPHONE_HANGUP);
}


void VXI::reprompt_element(const VXMLElement & doc)
{
  log->LogDiagnostic(2, L"VXI::reprompt_element()");

  // set a flag enabling prompting
  exe->playingPrompts = true;
}


void VXI::throw_element(const VXMLElement & elem)
{
  vxistring event;
  elem.GetAttribute(ATTRIBUTE_EVENT, event);
  if (event.empty()) {
    elem.GetAttribute(ATTRIBUTE_EVENTEXPR, event);
    if (!event.empty())
      exe->script.EvalScriptToString(event, event);
  }
  if (event.empty()) {
    // This should be caught by the DTD, but just in case...
    log->LogDiagnostic(0, L"VXI::throw_element - failure, no event to throw");
    throw VXIException::InterpreterEvent(EV_ERROR_BAD_THROW);
  }

  vxistring message;
  elem.GetAttribute(ATTRIBUTE_MESSAGE, message);
  if (message.empty()) {
    elem.GetAttribute(ATTRIBUTE_MESSAGEEXPR, message);
    if (!message.empty())
      exe->script.EvalScriptToString(message, message);
  }

  if (log->IsLogging(2)) {
    log->StartDiagnostic(2) << L"VXI::throw_element - throwing (" << event
                            << L", " << message << L")";
    log->EndDiagnostic();
  }
  throw VXIException::InterpreterEvent(event, message);
}


// The log element is handled in a slihtly odd manner.  The expressions in
// attributes are evaluated first because "expressions must be evaluated in
// document order".  And even if no output is being generated, the expressions
// must still be generated.
//
void VXI::log_element(const VXMLElement & doc)
{
  bool logging = log->IsLogging(1);

  // (1) Evaluate expressions in attributes first.

  vxistring attr;
  VXIValue * expr = NULL;
  if (doc.GetAttribute(ATTRIBUTE_EXPR, attr))
    expr = exe->script.EvalScriptToValue(attr);

  // (2) Handle main text.

  bool first = true;
  std::basic_ostringstream<wchar_t> out;

  const VXIchar * const SPACE = L" ";

  for (VXMLNodeIterator it(doc); it; ++it) {
    VXMLNode child = *it;

    switch (child.GetType()) {
    case VXMLNode::Type_VXMLContent:
      if (first)
	first = false;
      else
	out << SPACE;
      out << reinterpret_cast<const VXMLContent&>(child).GetValue();
      break;
    case VXMLNode::Type_VXMLElement:
    {
      const VXMLElement & elem = reinterpret_cast<const VXMLElement &>(child);
      if (elem.GetName() == NODE_VALUE) {
        vxistring value;
        elem.GetAttribute(ATTRIBUTE_EXPR, value);
        if (value.empty()) break;

        try {
          VXIValue * val = exe->script.EvalScriptToValue(value);

          if (val != NULL) {
            if (first)
              first = false;
            else
              out << SPACE;
            out << val;
            VXIValueDestroy(&val);
          }
        }
        catch (...) {
	  log->LogEvent(VXIlog_EVENT_LOG_ELEMENT, SimpleLogger::CONTENT,
			out.str().c_str());
	  if (logging)
	    log->LogDiagnostic(1, out.str().c_str());
          throw;
        }
      }
      break;
    }
    default:
      break;
    }
  }

  if (!out.str().empty()) {
    log->LogEvent(VXIlog_EVENT_LOG_ELEMENT, SimpleLogger::CONTENT,
                  out.str().c_str());
    if (logging)
      log->LogDiagnostic(1, out.str().c_str());
  }

  // (3) Handle label attribute.

  if (doc.GetAttribute(ATTRIBUTE_LABEL, attr)) {
    log->LogEvent(VXIlog_EVENT_LOG_ELEMENT, SimpleLogger::LABEL, attr.c_str());
    if (logging)
      log->LogDiagnostic(1, attr.c_str());
  }

  // (4) Handle expr attribute (if defined)

  if (expr != NULL) {
    out.str(L"");
    out << expr;
    log->LogEvent(VXIlog_EVENT_LOG_ELEMENT, SimpleLogger::EXPR, 
                  out.str().c_str());
    if (logging)
      log->LogDiagnostic(1, out.str().c_str());
    VXIValueDestroy(&expr);
  }
}


void VXI::if_element(const VXMLElement & doc)
{
  log->LogDiagnostic(2, L"VXI::if_element()");

  vxistring cond;
  doc.GetAttribute(ATTRIBUTE_COND, cond);
  bool executing = !cond.empty() && exe->script.TestCondition(cond);

  // Loop through children.
  // If cond == true, execute until <else> or <elseif>
  // If cond == false, continue until <else> or <elseif> 
  //   and test condition where <else> === <elseif cond="true">
  // Control is a little strange since intuitive body of <if> 
  //  and <else> are actually siblings of <else> and <elseif>, 
  //  not descendants

  for (VXMLNodeIterator it(doc); it; ++it) {
    VXMLNode child = *it;

    // Handle PCDATA as TTS prompts
    VXMLNode::VXMLNodeType type = child.GetType();
    if (type == VXMLNode::Type_VXMLContent) {
      if (executing) executable_prompt(child);
      continue;
    }
    if (type != VXMLNode::Type_VXMLElement) continue;

    const VXMLElement & elem = reinterpret_cast<VXMLElement &>(child);

    VXMLElementType nodeName = elem.GetName();
    if (nodeName == NODE_ELSE) { 
      if (executing) return;
      else executing = true;
    }
    else if (nodeName == NODE_ELSEIF) { 
      if (executing) return;
      vxistring cond;
      elem.GetAttribute(ATTRIBUTE_COND, cond);
      executing = !cond.empty() && exe->script.TestCondition(cond);
    }
    else if (executing)
      executable_element(elem);
  }
}



// <goto> is identical to submit except
//  - it can transition to internal items
//  - doesn't have a namelist
//
void VXI::goto_element(const VXMLElement & elem)
{
  log->LogDiagnostic(2, L"VXI::goto_element()");

  // Clear any information from unprocessed jumps.
  if (sdParams != NULL) {
    VXIMapDestroy(&sdParams);
    sdParams = NULL;
  }
  if (sdResult != NULL) {
    VXIValueDestroy(&sdResult);
    sdResult = NULL;
  }

  // According to the spec, the attributes follow the priority sequence:
  //   next, expr, nextitem, expritem

  vxistring uri;
  elem.GetAttribute(ATTRIBUTE_NEXT, uri);
  if (uri.empty()) {
    elem.GetAttribute(ATTRIBUTE_EXPR, uri);
    if (!uri.empty())
      exe->script.EvalScriptToString(uri, uri);
  }

  if (uri.empty()) {
    vxistring target;
    elem.GetAttribute(ATTRIBUTE_NEXTITEM, target);
    if (target.empty()) {
      elem.GetAttribute(ATTRIBUTE_EXPRITEM, target);
      if (!target.empty())
        exe->script.EvalScriptToString(target, target);
    }
    if (!target.empty()) {
      DoInnerJump(elem, target);
      throw VXIException::InterpreterEvent(EV_ERROR_BADFETCH);
    }
  }
  if (uri.empty())
    throw VXIException::InterpreterEvent(EV_ERROR_SEMANTIC);

  DoOuterJump(elem, uri);
}


void VXI::submit_element(const VXMLElement & elem)
{
  log->LogDiagnostic(2, L"VXI::submit_element()");

  // Clear any information from unprocessed jumps.
  if (sdParams != NULL) {
    VXIMapDestroy(&sdParams);
    sdParams = NULL;
  }
  if (sdResult != NULL) {
    VXIValueDestroy(&sdResult);
    sdResult = NULL;
  }

  // Get Submit-specific properties.
  VXIMapHolder submitData;
  if (submitData.GetValue() == NULL) throw VXIException::OutOfMemory();

  vxistring att;
  elem.GetAttribute(ATTRIBUTE_NAMELIST, att);
  if (att.empty()) {
    ExecutionContext::STRINGDEQUE::const_iterator i;
    for (i = exe->formitems.begin(); i != exe->formitems.end(); ++i) {
      // Check for and ignore hidden variables.
      if (VXMLDocumentModel::IsHiddenVariable(*i)) continue;
      // Otherwise, add it.
      VXIValue * val = exe->script.GetValue(*i);
      if (val != NULL)
        VXIMapSetProperty(submitData.GetValue(), (*i).c_str(), val);
    }
  }
  else {
    STRINGDEQUE names;
    ProcessNameList(att, names);
    STRINGDEQUE::const_iterator i;
    for (i = names.begin(); i != names.end(); ++i) {
      VXIValue * val = exe->script.GetValue(*i);
      if (val != NULL)
        VXIMapSetProperty(submitData.GetValue(), (*i).c_str(), val);
    }
  }

  vxistring uri;
  elem.GetAttribute(ATTRIBUTE_NEXT, uri);
  if (uri.empty()) {
    elem.GetAttribute(ATTRIBUTE_EXPR, uri);
    if (!uri.empty())
      exe->script.EvalScriptToString(uri, uri);
  }

  // DoOuterJump now owns the submitData.
  DoOuterJump(elem, uri, submitData.Release(), false);
}


void VXI::script_element(const VXMLElement & elem)
{
  log->LogDiagnostic(2, L"VXI::script_element()");

  // (1) Is the source specified?
  vxistring uri;
  elem.GetAttribute(ATTRIBUTE_SRC, uri);

  // (2) If not, use the content of the script.
  if (uri.empty()) {
    for (VXMLNodeIterator it(elem); it; ++it) {
      VXMLNode child = *it;
      
      if (child.GetType() != VXMLNode::Type_VXMLContent) {
        log->LogDiagnostic(0, L"VXI::script_element - unexpected element");
        continue;
      }

      VXMLContent & content = reinterpret_cast<VXMLContent &>(child);
      vxistring str = content.GetValue();
      if (!str.empty()) exe->script.EvalScript(str);
      return;
    }
  }

  // (3) Otherwise, we must retrieve the document.

  // (3.1) Get the cache attributes and document base.
  VXIMapHolder fetchobj;
  exe->properties.GetFetchobjBase(fetchobj);
  exe->properties.GetFetchobjCacheAttrs(elem, PropertyList::Script, fetchobj);

  // (3.2) Get the character encoding.
  vxistring encoding;
  elem.GetAttribute(ATTRIBUTE_CHARSET, encoding);

  // (3.3) Get the content.
  vxistring content;
  switch (parser->FetchContent(uri.c_str(), fetchobj, inet, *log,
                               encoding, content))
  {
  case -1: // Out of memory
    throw VXIException::OutOfMemory();
  case 0:  // Success
    break;
  case 1: // Invalid parameter
  case 2: // Unable to open URI
    log->LogError(203, SimpleLogger::URI, uri.c_str());
    throw VXIException::InterpreterEvent(EV_ERROR_BADFETCH);
  case 3: // Unable to read from URI
    log->LogError(204, SimpleLogger::URI, uri.c_str());
    throw VXIException::InterpreterEvent(EV_ERROR_BADFETCH);
  case 4: // Unsupported encoding
    log->LogError(208, SimpleLogger::URI, uri.c_str(),
                  SimpleLogger::ENCODING, encoding.c_str());
    throw VXIException::InterpreterEvent(EV_ERROR_BADFETCH);
  default:
    log->LogError(206, SimpleLogger::URI, uri.c_str());
    throw VXIException::Fatal();
  }

  // (3.4) Execute the script.
  exe->script.EvalScript(content);
}

/***************************************************************
 * Collect Phase
 ***************************************************************/

void VXI::CollectPhase(const VXMLElement & form, const VXMLElement & item)
{
  if (item == 0) {
    log->LogError(999, SimpleLogger::MESSAGE, L"empty executable element");
    return;
  }

  VXMLElementType nodeName = item.GetName();

  if (log->IsLogging(2)) {
    log->StartDiagnostic(2) << L"VXI::CollectPhase - " << nodeName;
    log->EndDiagnostic();
  }

  // Clear the event counts if we're collecting from a different form item
  vxistring itemname;
  item.GetAttribute(ATTRIBUTE__ITEMNAME, itemname);
  
  exe->eventcounts.ClearIf(itemname, false);

  if (nodeName == NODE_BLOCK)
    block_element(item);
  else if (nodeName == NODE_OBJECT)
    object_element(item);
  else if (nodeName == NODE_TRANSFER)
    transfer_element(form, item);
  else if (nodeName == NODE_FIELD || nodeName == NODE_INITIAL)
    field_element(form, item);
  else if (nodeName == NODE_SUBDIALOG)
    subdialog_element(item);
  else if (nodeName == NODE_RECORD)
    record_element(form, item);
  else if (nodeName == NODE_MENU)
    menu_element(item);
  else
    log->LogError(999, SimpleLogger::MESSAGE,L"unexpected executable element");
}


// process <block> elements in current interp context
// 
void VXI::block_element(const VXMLElement & elem)
{
  log->LogDiagnostic(2, L"VXI::block_element()");

  // The conditions (attributes 'cond' and 'expr' are set and checked
  // elsewhere (FormInit & DoInnerJump).

  // (1) Set this variable to prevent re-executing this block in the FIA.
  vxistring itemname;
  elem.GetAttribute(ATTRIBUTE__ITEMNAME, itemname);

  exe->script.SetVar(itemname, L"true");

  // (2) Execute the content of the <block>
  VXIMapHolder vars(NULL);
  execute_content(elem, vars);
}


void VXI::subdialog_element(const VXMLElement & elem)
{
  log->LogDiagnostic(2, L"VXI::subdialog_element()");
  
  // (1) Set properties.
  VXIMapHolder tmpVXIMapHolder;
  exe->properties.SetProperties(elem, FIELD_PROP, tmpVXIMapHolder);

  // (2) Disables any existing grammars and play entry prompts.
  exe->gm.DisableAllGrammars();
  queue_prompts(elem);

  // (3) Search for parameters defined within the subdialog tag.
  VXIMap * params = CollectParams(elem, false);

  // (4) Retrieve namelist properties.  This is similar to <submit> but differs
  //     in its default behavior.
  VXIMapHolder submitData;
  if (submitData.GetValue() == NULL) throw VXIException::OutOfMemory();

  vxistring att;
  elem.GetAttribute(ATTRIBUTE_NAMELIST, att);
  if (!att.empty()) {
    STRINGDEQUE names;
    ProcessNameList(att, names);
    STRINGDEQUE::const_iterator i;
    for (i = names.begin(); i != names.end(); ++i) {
      VXIValue * val = exe->script.GetValue(*i);
      if (val != NULL)
        VXIMapSetProperty(submitData.GetValue(), (*i).c_str(), val);
    }
  }

  vxistring uri;
  elem.GetAttribute(ATTRIBUTE_SRC, uri);
  if (uri.empty()) {
    elem.GetAttribute(ATTRIBUTE_SRCEXPR, uri);
    if (!uri.empty())
      exe->script.EvalScriptToString(uri, uri);
  }

  if (uri.empty())
    throw VXIException::InterpreterEvent(EV_ERROR_SEMANTIC,
                                         L"src or srcexpr must be specified");

  try {
    DoOuterJump(elem, uri, submitData.Release(), true);
  }
  catch (JumpDoc & e) {  // If the jump works
    exe->lastItem = elem;
    sdParams = params;
    throw e;
  }
}


void VXI::return_element(const VXMLElement & elem)
{
  log->LogDiagnostic(2, L"VXI::return_element()");

  // Returning when subdialog was not invoked is a semantic error.
  if (stackDepth == 1)
    throw VXIException::InterpreterEvent(EV_ERROR_SEMANTIC);

  VXIValue * result = NULL;

  vxistring event;
  elem.GetAttribute(ATTRIBUTE_EVENT, event);
  if (!event.empty()) {
    result = reinterpret_cast<VXIValue *>(VXIStringCreate(event.c_str()));
    if (result == NULL) throw VXIException::OutOfMemory();
  }
  else {
    VXIMapHolder resultVars;
    vxistring namelist;
    elem.GetAttribute(ATTRIBUTE_NAMELIST, namelist);
    if (!namelist.empty()) {
      STRINGDEQUE names;
      ProcessNameList(namelist, names);
      STRINGDEQUE::const_iterator i;
      for (i = names.begin(); i != names.end(); ++i) {
        VXIValue * val = exe->script.GetValue(*i);
        if (val != NULL)
          VXIMapSetProperty(resultVars.GetValue(), (*i).c_str(), val);
      }
    }

    result = reinterpret_cast<VXIValue *>(resultVars.Release());
  }

  sdResult = result;
  throw JumpReturn();
}


/*
 * Record element: The spec for <record> is a little unclear. The difficulties
 *  arise because the spec allows simultaneous recognition and recording 
 *  If a recording is  made the result is put into a the field var 
 *  and can apparently be used in-line in prompts. However, it is unclear
 *  what should happen if both recording and recognition occur.
 * In the current implementation, we are making <record> a pure recording
 *  tag with no recognition or dtmf enabled. 
 */
void VXI::record_element(const VXMLElement & form, const VXMLElement & elem)
{
  log->LogDiagnostic(2, L"VXI::record_element()");
  
  // (0) Before doing anything real, we only are supporting modal = true.
  vxistring modal;
  elem.GetAttribute(ATTRIBUTE_MODAL, modal);
  if (!modal.empty() && modal != L"true")
    throw VXIException::InterpreterEvent(EV_UNSUPPORT_RECORD_M);

  // (1) Set properties.
  VXIMapHolder tmpVXIMapHolder;
  exe->properties.SetProperties(elem, FIELD_PROP, tmpVXIMapHolder);

  // (2) Disables any existing grammars and play entry prompts.
  exe->gm.DisableAllGrammars();

  // (2.1) Play main prompts.
  queue_prompts(elem);

  // (2.2) Possibly append a beep.
  vxistring beep;
  if (elem.GetAttribute(ATTRIBUTE_BEEP, beep) && beep == L"true") {
    beep.erase();
    GetRuntimeProperty(VXI::BeepURI, beep);
    pm->Queue(beep);
  }

  // (3) Create a map containing the properties for the record.

  // (3.1) Get the other attributes.
  vxistring maxtime;  elem.GetAttribute(ATTRIBUTE_MAXTIME,      maxtime);
  vxistring silence;  elem.GetAttribute(ATTRIBUTE_FINALSILENCE, silence);
  vxistring dtmfterm; elem.GetAttribute(ATTRIBUTE_DTMFTERM,     dtmfterm);
  vxistring type;     elem.GetAttribute(ATTRIBUTE_TYPE,         type);

  // (3.2) Add them to the map.
  if (!maxtime.empty())
    exe->properties.SetProperty(GrammarManager::MaxTime,
                                maxtime.c_str(), FIELD_PROP);
  if (!silence.empty())
    exe->properties.SetProperty(GrammarManager::FinalSilence,
                                silence.c_str(), FIELD_PROP);
  if (!dtmfterm.empty())
    exe->properties.SetProperty(GrammarManager::DTMFTerm,
                                dtmfterm.c_str(), FIELD_PROP);
  if (!type.empty())
    exe->properties.SetProperty(GrammarManager::RecordingType,
                                type.c_str(), FIELD_PROP);

  // (3.3) Flatten the properties.
  VXIMapHolder recProps(exe->gm.GetRecordProperties(exe->properties,
                                                 pm->GetMillisecondTimeout()));

  if (!beep.empty()) AddParamValue(recProps, REC_RECORD_BEEP_URL, beep);

  if (recProps.GetValue() == NULL) throw VXIException::OutOfMemory();

  // (4) Issue record call and handle the return values.
  VXIrecRecordResult * answer = NULL;

  try {
    pm->Play();
    int recResult = exe->gm.Record(recProps, answer);
    pm->Stop(); // Just to clean up play structures

    switch (recResult) {
    case GrammarManager::Success:     // Record produced a recording
      break;
    case GrammarManager::Disconnect:  // Caller has disconnected w/o recording
      if (answer == NULL || answer->waveform == NULL)
        throw VXIException::InterpreterEvent(EV_TELEPHONE_HANGUP);
      break;
    case GrammarManager::Timeout:     // No signal was detected
      throw VXIException::InterpreterEvent(EV_NOINPUT);
    case GrammarManager::Error:       // An error aborted record
      throw VXIException::InterpreterEvent(EV_ERROR_RECOGNITION);
    case GrammarManager::OutOfMemory: // Insufficient memory available
      throw VXIException::OutOfMemory();
    case GrammarManager::InternalError: // Invalid result from Record
      throw VXIException::Fatal();
    case GrammarManager::BadMimeType:
      throw VXIException::InterpreterEvent(EV_UNSUPPORT_FORMAT);
    default:
      log->LogError(999, SimpleLogger::MESSAGE,
                    L"unexpected value in recognition result");
      throw VXIException::Fatal();
    }

    // (5) Store the waveform and create shadow variables.

    vxistring itemname;
    elem.GetAttribute(ATTRIBUTE__ITEMNAME, itemname);

    // (5.1) put waveform into 'itemname'
    exe->script.SetValue(itemname,
                         reinterpret_cast<const VXIValue*>(answer->waveform));

    // (5.2) Set up shadow variables
    vxistring shadow;
    exe->script.EvalScript(itemname + L"$ = new Object();");

    // (5.2.1) duration
    VXIInteger * duration = VXIIntegerCreate(answer->duration);
    if (duration == NULL) VXIException::OutOfMemory();
    shadow = itemname + L"$.duration";
    exe->script.SetValue(shadow, reinterpret_cast<const VXIValue*>(duration));
    VXIIntegerDestroy( &duration ); 

    // (5.2.2) termchar
    if (answer->termchar != 0) {
      const VXIchar tmp[2] = {answer->termchar, 0};
      VXIString * termchar = VXIStringCreate((VXIchar*)tmp);
      if (termchar == NULL) VXIException::OutOfMemory();
      shadow = itemname + L"$.termchar";
      exe->script.SetValue(shadow,reinterpret_cast<const VXIValue*>(termchar));
      VXIStringDestroy(&termchar);
    }
    else {
      shadow = itemname + L"$.termchar = null";
      exe->script.EvalScript(shadow);
    }

    // (5.2.3) size
    const VXIchar * contentType;
    const VXIbyte * content;
    VXIulong contentSize = 0; 
    if (VXIContentValue(answer->waveform, &contentType, &content, &contentSize)
        != VXIvalue_RESULT_SUCCESS)
      throw VXIException::Fatal();

    VXIInteger * size = VXIIntegerCreate(contentSize);
    if (size == NULL) VXIException::OutOfMemory();
    shadow = itemname + L"$.size";
    exe->script.SetValue(shadow, reinterpret_cast<const VXIValue*>(size));
    VXIIntegerDestroy( &size ); 

    // (5.2.4) time stamp
    VXIString * timestamp = VXIStringCreate(answer->timestamp);
    if (timestamp == NULL) VXIException::OutOfMemory();
    shadow = itemname + L"$.timestamp";
    exe->script.SetValue(shadow, reinterpret_cast<const VXIValue*>(timestamp));
    VXIStringDestroy(&timestamp);

    // (6) Trigger filled or hangup event as appropriate.

    if (recResult == GrammarManager::Disconnect)
      throw VXIException::InterpreterEvent(EV_TELEPHONE_HANGUP);

    EasyFilled(itemname, form);

    // (7) Call destroy on the structure.
    if (answer != NULL) answer->Destroy(&answer);
  }
  catch (...) {
    if (answer != NULL) answer->Destroy(&answer);
    throw;
  }
}


/*
 * Object element: this is essentially a platform dependent field 
 *  with odd return values. In order to preserve generality, 
 *  we will setup field item processing here (prompts and properties).
 *  We will collect the rest of the attributes and params into separate
 *  objects (to avoid name collisions) and pass them down to the 
 *  object handler. The object handler must copy out any data it 
 *  wants as we gc the args and params.
 */
void VXI::object_element(const VXMLElement & elem)
{
  log->LogDiagnostic(2, L"VXI::object_element()");

  // (1) Set properties.
  VXIMapHolder tmpVXIMapHolder;
  exe->properties.SetProperties(elem, FIELD_PROP, tmpVXIMapHolder);

  // (2) Disable any existing grammars and play entry prompts.
  exe->gm.DisableAllGrammars();
  queue_prompts(elem);

  if (object == NULL)
    throw VXIException::InterpreterEvent(EV_UNSUPPORT_OBJECT);

  // (3) Get the basic parameters.
  vxistring itemname;
  elem.GetAttribute(ATTRIBUTE__ITEMNAME, itemname);

  // (3.1) Create a new property map.
  VXIMapHolder properties;
  if (properties.GetValue() == NULL) throw VXIException::OutOfMemory();

  // (3.2) Add the attributes as properties.
  vxistring arg;
  elem.GetAttribute(ATTRIBUTE_CLASSID, arg);
  if (!arg.empty()) AddParamValue(properties, OBJECT_CLASS_ID, arg);
  elem.GetAttribute(ATTRIBUTE_CODEBASE, arg);
  if (!arg.empty()) AddParamValue(properties, OBJECT_CODE_BASE, arg);
  elem.GetAttribute(ATTRIBUTE_CODETYPE, arg);
  if (!arg.empty()) AddParamValue(properties, OBJECT_CODE_TYPE, arg);
  elem.GetAttribute(ATTRIBUTE_DATA, arg);
  if (!arg.empty()) AddParamValue(properties, OBJECT_DATA, arg);
  elem.GetAttribute(ATTRIBUTE_TYPE, arg);
  if (!arg.empty()) AddParamValue(properties, OBJECT_TYPE, arg);
  elem.GetAttribute(ATTRIBUTE_ARCHIVE, arg);
  if (!arg.empty()) AddParamValue(properties, OBJECT_ARCHIVE, arg);

  // (4) Search for parameters defined within the object tag.
  VXIMap * parameters = CollectParams(elem, true);

  // (5) Call the platform dependent implementation.
  VXIValue * resultObj;
  VXIobjResult rc = object->Execute(object, properties.GetValue(),
                                    parameters, & resultObj);

  // (6) Process result code.

  // (6.1) Handle error results.
  switch (rc) {
  case VXIobj_RESULT_SUCCESS:
    break;
  case VXIobj_RESULT_OUT_OF_MEMORY:
    throw VXIException::OutOfMemory();
  case VXItel_RESULT_UNSUPPORTED:
    throw VXIException::InterpreterEvent(EV_UNSUPPORT_OBJECT);
  default:
    log->LogError(460, SimpleLogger::MESSAGE, L"unexpected return value");
    throw VXIException::InterpreterEvent(EV_ERROR_OBJECT);
  }

  VXIMapDestroy(& parameters);

  // (7) Process returned object.
  exe->script.SetValue(itemname, resultObj);
  VXIValueDestroy(& resultObj);

  // (8) Done.

  log->LogDiagnostic(2, L"VXI::object_element - done");

  EasyFilled(itemname, elem);
}


// Field or Initial item
//
void VXI::field_element(const VXMLElement & form, const VXMLElement & itemNode)
{
  log->LogDiagnostic(2, L"VXI::field_element()");

  VXIMapHolder tmpVXIMapHolder;
  exe->properties.SetProperties(itemNode, FIELD_PROP, tmpVXIMapHolder);

  // (1) Queue the prompts.
  queue_prompts(itemNode);      // calls platform

  // (2) Activate grammars as necessary

  exe->gm.DisableAllGrammars();

  // (2.1) Get form name.
  vxistring formName;
  form.GetAttribute(ATTRIBUTE__ITEMNAME, formName);

  // (2.2) Get dialog name
  vxistring itemName;
  itemNode.GetAttribute(ATTRIBUTE__ITEMNAME, itemName);

  // (2.3) Is this dialog modal?
  vxistring modal;
  itemNode.GetAttribute(ATTRIBUTE_MODAL, modal);
  bool isModal = false;
  if (modal.length() != 0) isModal = (modal == L"true");

  // (2.4) Finally, do the recognition.

  if (log->IsLogging(2)) {
    log->StartDiagnostic(2) << L"VXI::field_element - activating grammars for "
      L"form = '" << formName << L"' formitem = '" << itemName << L"'";
    log->EndDiagnostic();
  }

  VXIMapHolder recProps(exe->gm.GetRecProperties(exe->properties,
                                                 pm->GetMillisecondTimeout()));
  if (recProps.GetValue() == NULL) throw VXIException::OutOfMemory();
  if (!exe->gm.EnableGrammars(exe->documentName, formName, itemName,
                              recProps, isModal))
    throw VXIException::InterpreterEvent(EV_ERROR_NO_GRAMMARS);

  do_recognition(recProps);

  log->LogError(999, SimpleLogger::MESSAGE, L"unexpected exit from field");
  throw VXIException::Fatal();
}


/* 
 * The description of <transfer> a little unclear on result handling.
 *  The are 2 simultaeous mechanisms for handling error conditions
 *  return values in item var, and event throwing (with potential races between
 *  them) but apparently no mechanism to describe success condition.
 * We will do the following:
 *  If attempt to transfer was unsuccessful, set field var to error code
 *  If transfer was successful and non-bridging, set field var to 'transferred'
 *   and THEN throw disconnent.transfer event
 *  If transfer was successful and bridging:
 *    If far_end_disconnect set field var and continue in FIA
 *    If our user hung up or was otherwise disconnected, throw hangup event
 *  We also need return policy for maxtime. Here we will throw a 
 *    telephone.bridge.timeout event. Which will probably log the event and
 *    exit, cleaning up the session. (Ideally the user could be tranferred
 *    without tied up local resources and then transfered back into old session
 *    with return values. However, this will probably require a server-side
 *    implementation).
 */
void VXI::transfer_element(const VXMLElement & form, const VXMLElement & elem)
{
  log->LogDiagnostic(2, L"VXI::transfer_element()");

  // (1) Set properties.
  VXIMapHolder tmpVXIMapHolder;
  exe->properties.SetProperties(elem, FIELD_PROP, tmpVXIMapHolder);

  // (2) Disables any existing grammars and play entry prompts.
  exe->gm.DisableAllGrammars();
  queue_prompts(elem);

  // (3) Get the basic parameters.

  vxistring dest;
  elem.GetAttribute(ATTRIBUTE_DEST, dest);
  if (dest.empty()) {
    elem.GetAttribute(ATTRIBUTE_DESTEXPR, dest);
    if (!dest.empty())
      exe->script.EvalScriptToString(dest, dest);
  }

  bool bridged = false;
  vxistring temp;
  elem.GetAttribute(ATTRIBUTE_BRIDGE, temp);
  if (temp == L"true") bridged = true;

  // (4) Create a map containing the properties for the transfer.

  // (4.1) Get the other attributes.
  vxistring maxtime;
  vxistring connecttime;
  elem.GetAttribute(ATTRIBUTE_MAXTIME, maxtime);
  elem.GetAttribute(ATTRIBUTE_CONNECTTIME, connecttime);

  // (4.2) Create a new property map.
  VXIMapHolder m;
  if (m.GetValue() == NULL) throw VXIException::OutOfMemory();
  exe->properties.GetProperties(m);

  // (4.3) Add the attributes as properties.
  int time;

  // (4.3.1) Maximum call duration
  if (!maxtime.empty() &&
      exe->properties.ConvertTimeToMilliseconds(*log, maxtime, time))
    AddParamValue(m, TEL_MAX_CALL_TIME, time);

  // (4.3.2) Timeout on connection attempts
  if (!connecttime.empty() &&
      exe->properties.ConvertTimeToMilliseconds(*log, connecttime, time))
    AddParamValue(m, TEL_CONNECTTIMEOUT, time);

// get and add the transferaudio attribute to the properties map.
  vxistring transferaudio;
  elem.GetAttribute(ATTRIBUTE_TRANSFERAUDIO, transferaudio);
  if (transferaudio.empty()) {
    elem.GetAttribute(ATTRIBUTE_TRANSFERAUDIOEXPR, transferaudio);
    if (!transferaudio.empty())
      exe->script.EvalScriptToString(transferaudio, transferaudio);
  }
  if ( !transferaudio.empty() ) {
    AddParamValue(m, TEL_TRANSFER_AUDIO, transferaudio);

    const VXIchar * base = exe->properties.GetProperty(PropertyList::BaseURI);
    if (base != NULL)
      AddParamValue(m, INET_URL_BASE, base );
  }

  vxistring formName;
  form.GetAttribute(ATTRIBUTE__ITEMNAME, formName);
  vxistring itemName;
  elem.GetAttribute(ATTRIBUTE__ITEMNAME, itemName);

  VXIMapHolder recProps(exe->gm.GetRecProperties(exe->properties,
                                                 pm->GetMillisecondTimeout()));
  if (recProps.GetValue() == NULL) throw VXIException::OutOfMemory();
  exe->gm.EnableGrammars(exe->documentName, formName, itemName, recProps, true );


  // (5) Play the queued prompts and do the transfer.
  pm->PlayAll();

  VXIMap * rawXferInfo = NULL;
  VXItelResult result;

  if (!bridged) {
    result = tel->TransferBlind(tel, m.GetValue(),
                                dest.c_str(), NULL, &rawXferInfo);
  }
  else {
    result = tel->TransferBridge(tel, m.GetValue(),
                                 dest.c_str(), NULL, &rawXferInfo);
  }
  VXIMapHolder xferInfo(rawXferInfo);

  // (6) Process result code.

  // (6.1) Handle error results.
  switch (result) {
  case VXItel_RESULT_OUT_OF_MEMORY:
    throw VXIException::OutOfMemory();
  case VXItel_RESULT_SUCCESS:
    break;
  case VXItel_RESULT_UNSUPPORTED:
    throw VXIException::InterpreterEvent(EV_UNSUPPORT_TRANSFER);
  case VXItel_RESULT_INVALID_ARGUMENT:
    log->LogError(bridged ? 441 : 440, SimpleLogger::MESSAGE,
                  L"invalid argument");
    throw VXIException::InterpreterEvent(EV_ERROR_TRANSFER);
  case VXItel_RESULT_FAILURE:
  default:
    log->LogError(bridged ? 441 : 440, SimpleLogger::MESSAGE,
                  L"unexpected return value");
    throw VXIException::InterpreterEvent(EV_ERROR_TRANSFER);
  }

  // (6.2) Blind transfers result in an immediate event.
  if (!bridged) {
    log->LogDiagnostic(2, L"VXI::transfer_element - blind done");
    throw VXIException::InterpreterEvent(EV_TELEPHONE_TRANSFER);
  }

  // (7) Process returned information for bridged transfers.

  vxistring itemname;
  elem.GetAttribute(ATTRIBUTE__ITEMNAME, itemname);

  // (7.1) Transfer status.
  const VXIValue * resultCode = VXIMapGetProperty(xferInfo.GetValue(),
                                                  TEL_TRANSFER_STATUS);
  if (resultCode == NULL || VXIValueGetType(resultCode) != VALUE_INTEGER) {
    vxistring message(TEL_TRANSFER_STATUS);
    message += L" must be defined with type integer";
    log->LogError(bridged ? 441 : 440, SimpleLogger::MESSAGE, message.c_str());
    throw VXIException::InterpreterEvent(EV_ERROR_TRANSFER);
  }
  switch (VXIIntegerValue(reinterpret_cast<const VXIInteger *>(resultCode))) {
  case VXItel_TRANSFER_BUSY:
    exe->script.SetString(itemname, L"busy");
    break;
  case VXItel_TRANSFER_NOANSWER:
    exe->script.SetString(itemname, L"noanswer");
    break;
  case VXItel_TRANSFER_NETWORK_BUSY:
    exe->script.SetString(itemname, L"network_busy");
    break;
  case VXItel_TRANSFER_NEAR_END_DISCONNECT:
    exe->script.SetString(itemname, L"near_end_disconnect");
    break;
  case VXItel_TRANSFER_FAR_END_DISCONNECT:
    exe->script.SetString(itemname, L"far_end_disconnect");
    break;
  case VXItel_TRANSFER_NETWORK_DISCONNECT: 
    exe->script.SetString(itemname, L"network_disconnect");
    break;
  case VXItel_TRANSFER_MAXTIME_DISCONNECT:
    exe->script.SetString(itemname, L"near_end_disconnect");
    break;
  case VXItel_TRANSFER_UNKNOWN:
  default:
    log->LogError(bridged ? 441 : 440, SimpleLogger::MESSAGE,
                  L"unexpected result code");
    throw VXIException::InterpreterEvent(EV_ERROR_TRANSFER);
  }

  // (7.2) Duration.
  const VXIValue * duration = VXIMapGetProperty(xferInfo.GetValue(),
                                                TEL_TRANSFER_DURATION);
  if (duration == NULL || VXIValueGetType(duration) != VALUE_INTEGER) {
    vxistring message(TEL_TRANSFER_DURATION);
    message += L" must be defined with type integer";
    log->LogError(bridged ? 441 : 440, SimpleLogger::MESSAGE, message.c_str());
    throw VXIException::InterpreterEvent(EV_ERROR_TRANSFER);
  }

  exe->script.EvalScript(itemname + L"$ = new Object();");
  vxistring shadow = itemname + L"$.duration";

  // Convert the duration from milliseconds to seconds and add it.

  int durVal = VXIIntegerValue(reinterpret_cast<const VXIInteger *>(duration));
  VXIFloat * seconds = VXIFloatCreate(VXIflt32(durVal / 1000.0));
  if (seconds == NULL) throw VXIException::OutOfMemory();

  try {
    exe->script.SetValue(shadow, reinterpret_cast<const VXIValue *>(seconds));
  }
  catch (...) {
    VXIFloatDestroy(&seconds);
    throw;
  }
  VXIFloatDestroy(&seconds);

  // (8) Done.

  log->LogDiagnostic(2, L"VXI::transfer_element - bridged done");

  EasyFilled(itemname, form);
}


// Menu element as field
//
void VXI::menu_element(const VXMLElement & menuNode)
{
  log->LogDiagnostic(2, L"VXI::menu_element()");

  VXIMapHolder tmpVXIMapHolder;
  exe->properties.SetProperties(menuNode, FIELD_PROP, tmpVXIMapHolder);

  // (1) Queue the prompts.
  queue_prompts(menuNode);      // calls platform

  // (2) Activate grammars as necessary

  exe->gm.DisableAllGrammars();

  // (2.1) Get form name.
  vxistring menuName;
  menuNode.GetAttribute(ATTRIBUTE__ITEMNAME, menuName);

  // (2.2) Do the recognition.

  if (log->IsLogging(2)) {
    log->StartDiagnostic(2) << L"VXI::menu_element - activating grammars for "
      L"menu = '" << menuName << L"'";
    log->EndDiagnostic();
  }

  VXIMapHolder recProps(exe->gm.GetRecProperties(exe->properties,
                                                 pm->GetMillisecondTimeout()));
  if (recProps.GetValue() == NULL) throw VXIException::OutOfMemory();
  if (!exe->gm.EnableGrammars(exe->documentName, menuName, menuName,
                              recProps, false))
    throw VXIException::InterpreterEvent(EV_ERROR_NO_GRAMMARS);

  do_recognition(recProps);

  log->LogError(999, SimpleLogger::MESSAGE, L"unexpected exit from menu");
  throw VXIException::Fatal();
}


// Collect <param> elements into a VXIMap.  This is used by both the
// <subdialog> and <object> elements.
//
VXIMap * VXI::CollectParams(const VXMLElement & doc, bool isObject)
{
  VXIMapHolder obj;
  if (obj.GetValue() == NULL) throw VXIException::OutOfMemory();

  // (1) Walk through the children on this node.
  for (VXMLNodeIterator it(doc); it; ++it) {
    VXMLNode child = *it;

    // Ignore anything which isn't a <param>
    if (child.GetType() != VXMLNode::Type_VXMLElement) continue;
    const VXMLElement & elem = reinterpret_cast<const VXMLElement &>(child);
    if (elem.GetName() != NODE_PARAM) continue;

    // <param> elements must have names.  This is enforced by the DTD.
    vxistring name;
    if (!elem.GetAttribute(ATTRIBUTE_NAME, name) || name.empty()) continue;

    // For <object>, check if the value type is "ref" or "data".
    // If "ref", the param properties must be embedded in another map.
    VXIbool embedMap = false;
    VXIMap * properties = NULL;
    if (isObject) {
      vxistring vtype;
      elem.GetAttribute(ATTRIBUTE_VALUETYPE, vtype);
      if (vtype == L"ref") {
        embedMap = true;

        // Create the new properties map and set the VALUETYPE 
        // property (for now, always "ref").
        properties = VXIMapCreate();
        if (properties == NULL) continue;
        VXIString * val = VXIStringCreate(vtype.c_str());
        if (val == NULL) throw VXIException::OutOfMemory();
        VXIMapSetProperty(properties, OBJECT_VALUETYPE, 
                          reinterpret_cast<VXIValue *>(val));
      }
    }

    vxistring value;
    elem.GetAttribute(ATTRIBUTE_VALUE, value);
    if (!value.empty()) {
      VXIString * val = VXIStringCreate(value.c_str());
      if (val == NULL) throw VXIException::OutOfMemory();
      if (embedMap)
        VXIMapSetProperty(properties, OBJECT_VALUE,
                          reinterpret_cast<VXIValue *>(val));
      else
        VXIMapSetProperty(obj.GetValue(), name.c_str(),
                          reinterpret_cast<VXIValue *>(val));
    }
    else {
      elem.GetAttribute(ATTRIBUTE_EXPR, value);
      if (value.empty()) continue;
      VXIValue * val = exe->script.GetValue(value);
      if (val == NULL) continue;
      if (embedMap)
        VXIMapSetProperty(properties, OBJECT_VALUE, val);
      else
        VXIMapSetProperty(obj.GetValue(), name.c_str(), val);
    }

    if (embedMap) {
      vxistring mime;
      elem.GetAttribute(ATTRIBUTE_TYPE, mime);
      if (!mime.empty()) {
        VXIString * val = VXIStringCreate(mime.c_str());
        if (val == NULL) throw VXIException::OutOfMemory();
        VXIMapSetProperty(properties, OBJECT_TYPE, 
                          reinterpret_cast<VXIValue *>(val));
      }

      // Finally, embed the properties map into the original map
      VXIMapSetProperty(obj.GetValue(), name.c_str(), 
                        reinterpret_cast<VXIValue *>(properties));
    }
  }

  return obj.Release();
}


/*
// This is the where recognition occurs and results are processed.  The outline
// looks something like this:
//
// 1) Call VXIrecInterface::Recognize and handle errors.
// 2) Handle <link> & <choice> (where the actual result doesn't matter).
// 3) Extract the recognition results and set shadow variables.
// 4) Handle <filled>.
//
// The grammars must be activated before calling this function.
*/
void VXI::do_recognition(const VXIMapHolder & properties)
{
  log->LogDiagnostic(2, L"VXI::do_recognition()");

  RecognitionAnswer answer;
  VXMLElement answerElement;
  bool usedDTMF = false;

  // (1) Process the recognizer result status.
  pm->Play();
  int recResult = exe->gm.Recognize(properties, answer, answerElement);
  pm->Stop(); // Just to clean up play structures

  // (1.1) Copy waveform.
  if (answer.waveform != NULL)
    exe->script.SetValue(L"application.waveform$",
                         reinterpret_cast<const VXIValue*>(answer.waveform));
  else
    exe->script.EvalScript(L"application.waveform$ = null;");

  // (1.2) Handle result.
  switch (recResult) {
  case GrammarManager::Success:     // Recognition returned a hypothesis
    usedDTMF = false;
    break;
  case GrammarManager::SuccessDTMF: // Recognition returned DTMF hypothesis
    usedDTMF = true;
    break;
  case GrammarManager::Failure:     // Speech detected, no likely hypothesis
    throw VXIException::InterpreterEvent(EV_NOMATCH);
  case GrammarManager::Timeout:     // No speech was detected
    throw VXIException::InterpreterEvent(EV_NOINPUT);
  case GrammarManager::Disconnect:  // Caller has disconnected;no hypothesis
    throw VXIException::InterpreterEvent(EV_TELEPHONE_HANGUP);
  case GrammarManager::Error:       // An error aborted recognition
    throw VXIException::InterpreterEvent(EV_ERROR_RECOGNITION);
  case GrammarManager::OutOfMemory: // Insufficient memory available
    throw VXIException::OutOfMemory();
  case GrammarManager::InternalError: // Invalid result from Recognize
    throw VXIException::Fatal();
  default:
    log->LogError(999, SimpleLogger::MESSAGE,
                  L"unexpected value in recognition result");
    throw VXIException::Fatal();
  }

  // (2) At this point we have success.  Find the action associated with this
  // element.

  log->LogDiagnostic(2, L"VXI::do_recognition - have an answer");

  // (2.1) Find the element in which the grammar lives.

  VXMLElementType name = answerElement.GetName();
  if (name == NODE_GRAMMAR || name == NODE_DTMF) {
    answerElement = answerElement.GetParent();
    if (answerElement == 0) {
      log->LogError(999, SimpleLogger::MESSAGE,
                    L"could not locate parent of grammar");
      throw VXIException::Fatal();
    }
    name = answerElement.GetName();
  }

  // (2.2) Handle the two goto possibilities (choide & link).

  if (name == NODE_CHOICE || name == NODE_LINK) {
    // Decide whether to treat this as an event or a goto.
    vxistring next;
    answerElement.GetAttribute(ATTRIBUTE_NEXT, next);
    if (next.empty())
      answerElement.GetAttribute(ATTRIBUTE_EXPR, next);
    if (next.empty()) {
      answerElement.GetAttribute(ATTRIBUTE_EVENT, next);
      if (!next.empty()) {
        // At this point, the semantics are identical to throw. 
        throw_element(answerElement);
      }
    }

    // Otherwise, the semantics are identical to goto.
    goto_element(answerElement);
  }

  // (3) Find the form and item associated with this response.

  // (3.1) Find the enclosing form.
  VXMLElement answerDialog = answerElement;
  name = answerDialog.GetName();
  if (name != NODE_FORM) {
    answerDialog = answerDialog.GetParent();
    if (answerDialog == 0) {
      log->LogError(999, SimpleLogger::MESSAGE,
                    L"could not locate form associated with answer");
      throw VXIException::Fatal();
    }
    name = answerDialog.GetName();
  };

  // (3.2) Perform a final sanity check to verify that the results match our
  // expection.
  if (name != NODE_FORM ||
      (!IsFormItemNode(answerElement) && answerElement != answerDialog))
  {
    log->LogError(999, SimpleLogger::MESSAGE,
                  L"bad grammar location from recognition");
    throw VXIException::Fatal();
  }

  AnswerInformation temp(answer, usedDTMF, answerElement, answerDialog);
  throw temp;
}


void VXI::ProcessRecognitionResult(const VXMLElement & answerForm,
                                   const VXMLElement & answerElement,
                                   bool usedDTMF,
                                   const RecognitionAnswer & answer)
{
  log->LogDiagnostic(2, L"VXI::ProcessRecognitionResult()");

  // If only one key is returned in the answer, this string is set.  Otherwise,
  // it remains empty.
  vxistring onlyKey;

  // (1) Copy the answer to application scope.

  exe->script.EvalScript(L"application.lastresult$ = null;");
  exe->script.EvalScript(L"application.lastresult$ = new Array();");

  for (unsigned int n = 0; n < answer.numAnswers; ++n) {
    // (1.1) Create an object in the array for each answer.
    std::basic_ostringstream<wchar_t> out;
    out << L"application.lastresult$[" << n << L"]";
    const vxistring name(out.str());

    exe->script.EvalScript(name + L" = new Object();");

    // (1.2) Get the utterance & set it locally.
    const VXIValue * temp = VXIVectorGetElement(answer.utts[n], 0);
    if (temp == NULL || VXIValueGetType(temp) != VALUE_STRING) {
      log->LogError(420, SimpleLogger::MESSAGE,
                    L"REC_RAW contents must be all strings");
      throw VXIException::Fatal();
    }
    const VXIString * str = reinterpret_cast<const VXIString*>(temp);
    exe->script.SetString(name + L".utterance", toString(str));

    // (1.3) Get the confidence & set it locally.
    temp = VXIVectorGetElement(answer.scores[n], 0);
    if (temp == NULL || VXIValueGetType(temp) != VALUE_FLOAT) {
      log->LogError(420, SimpleLogger::MESSAGE,
                    L"REC_CONF contents must be all floats");
      throw VXIException::Fatal();
    }
    exe->script.SetValue(name + L".confidence", temp);

    // (1.4) Set the input mode.
    if (usedDTMF == true)
      exe->script.SetString(name + L".inputmode", L"dtmf");
    else
      exe->script.SetString(name + L".inputmode", L"voice");

    // (1.5) Build up the interpretation.

    const unsigned int numKeys = VXIVectorLength(answer.keys[n]);
    VXIMapHolder interpretations;

    for (VXIunsigned i = 0; i < numKeys; ++i) {
      // (1.5.1) Get each key.
      temp = VXIVectorGetElement(answer.keys[n], i);
      if (temp == NULL || VXIValueGetType(temp) != VALUE_STRING) {
        log->LogError(420, SimpleLogger::MESSAGE,
                      L"REC_KEYS contents must be all strings");
        throw VXIException::Fatal();
      }
      const VXIString * key = reinterpret_cast<const VXIString*>(temp);
      if (numKeys == 1) onlyKey = VXIStringCStr(key);

      // (1.5.2) Get each corresponding value.
      temp = VXIVectorGetElement(answer.values[n], i);
      if (temp == NULL) {
        log->LogError(420, SimpleLogger::MESSAGE,
                      L"REC_VALUES must be non-null");
        throw VXIException::Fatal();
      }

      // (1.5.3) Add the value (as a string) into the interpretations map.
      VXIvalueType type = VXIValueGetType(temp);
      VXIValue * entry = NULL;

      if (type == VALUE_STRING) {
        entry = VXIValueClone(temp);
      }
      else if (type == VALUE_INTEGER) {
        const VXIInteger * val = reinterpret_cast<const VXIInteger*>(temp);
        const VXIchar * value = (VXIIntegerValue(val)!=0) ? L"true" : L"false";
        entry = reinterpret_cast<VXIValue *>(VXIStringCreate(value));
      }
      else {
        log->LogError(420, SimpleLogger::MESSAGE,
                      L"REC_VALUES contents have illegal type");
        throw VXIException::Fatal();
      }

      if (entry == NULL) throw VXIException::OutOfMemory();
      VXIMapSetProperty(interpretations.GetValue(), VXIStringCStr(key), entry);
    }

    // (1.6) Set the interpretation locally.  This creates a string if a single
    // key was returned and an object otherwise.

    const VXIValue * interp = NULL;
    if (onlyKey.empty())
      interp = reinterpret_cast<const VXIValue *>(interpretations.GetValue());
    else {
      interp = VXIMapGetProperty(interpretations.GetValue(), onlyKey.c_str());
      if (interp == NULL || VXIValueGetType(interp) != VALUE_STRING) {
        log->LogError(420, SimpleLogger::MESSAGE,
                      L"unable to extract single key from interpretations");
        throw VXIException::Fatal();
      }
    }

    exe->script.SetValue(L"application.lastresult$[0].interpretation", interp);
  }

  // (2) Copy the 0th element to application.lastresult$
  exe->script.EvalScript(L"application.lastresult$.utterance = "
                         L"application.lastresult$[0].utterance;");
  exe->script.EvalScript(L"application.lastresult$.confidence = "
                         L"application.lastresult$[0].confidence;");
  exe->script.EvalScript(L"application.lastresult$.inputmode = "
                         L"application.lastresult$[0].inputmode;");
  exe->script.EvalScript(L"application.lastresult$.interpretation = "
                         L"application.lastresult$[0].interpretation;");

  // (3) Now walk through vectors.  Set ECMA script variables as necessary.

  // Determine the field name.  This may be empty.
  vxistring fieldName;
  if (answerElement != 0 && answerElement.GetName() != NODE_INITIAL)
    answerElement.GetAttribute(ATTRIBUTE__ITEMNAME, fieldName);

  if (answerElement != 0 && fieldName.empty()) {
    log->LogError(999, SimpleLogger::MESSAGE,
                  L"unable to obtain field name in rec result processing");
    throw VXIException::Fatal();
  }

  STRINGDEQUE filledSlots;

  // Handle field level grammars.
  if (!fieldName.empty()) {
    exe->script.EvalScript(fieldName +
                           L" = application.lastresult$[0].interpretation;");

    exe->script.EvalScript(fieldName + L"$ = new Object();");
    exe->script.EvalScript(fieldName + L"$.confidence = "
                           L"application.lastresult$[0].confidence;");
    exe->script.EvalScript(fieldName + L"$.utterance = "
                           L"application.lastresult$[0].utterance;");
    exe->script.EvalScript(fieldName + L"$.inputmode = "
                           L"application.lastresult$[0].inputmode;");
    exe->script.EvalScript(fieldName + L"$.interpretation = "
                           L"application.lastresult$[0].interpretation;");

    filledSlots.push_back(fieldName);
  }

  // And form level grammars.
  else {
    // Find all <field> elements for this form.
    for (VXMLNodeIterator it(answerForm); it; ++it) {
      VXMLNode child = *it;
      if (child.GetType() != VXMLNode::Type_VXMLElement) continue;
      const VXMLElement & elem = reinterpret_cast<const VXMLElement &>(child);
      if (elem.GetName() != NODE_FIELD) continue;
      
      // Find the corresponding field variable and slot names
      vxistring slot;
      vxistring fieldvar;
      elem.GetAttribute(ATTRIBUTE__ITEMNAME, fieldvar);
      elem.GetAttribute(ATTRIBUTE_SLOT, slot);
      if (slot.empty()) slot = fieldvar;

      // Does the answer contain data for this slot?
      if (onlyKey.empty()) {
        vxistring test(L"application.lastresult$[0].interpretation.");
        test += slot;
        test += L" == undefined";
        if (exe->script.TestCondition(test)) continue;
      } else
        if (slot != onlyKey) continue;

      slot += L";";

      if (onlyKey.empty())
        exe->script.EvalScript(fieldvar +
                               L" = application.lastresult$[0].interpretation."
                               + slot);
      else
        exe->script.EvalScript(fieldvar + L" = application."
                               L"lastresult$[0].interpretation;");

      exe->script.EvalScript(fieldvar + L"$ = new Object();");
      exe->script.EvalScript(fieldvar + L"$.confidence = "
                             L"application.lastresult$[0].confidence." + slot);
      exe->script.EvalScript(fieldvar + L"$.utterance = "
                             L"application.lastresult$[0].utterance;");
      exe->script.EvalScript(fieldvar + L"$.inputmode = "
                             L"application.lastresult$[0].inputmode;");
      exe->script.EvalScript(fieldvar + L"$.interpretation = " + fieldvar);

      filledSlots.push_back(fieldvar);
    }
  }

  // (4) In nothing was 'filled', we treat this as a nomatch event.
  if (filledSlots.empty()) 
    throw VXIException::InterpreterEvent(EV_NOMATCH);

  // (5) Sort the filledSlots.  We'll need this later.
  std::sort(filledSlots.begin(), filledSlots.end());
  if (std::unique(filledSlots.begin(), filledSlots.end()) != filledSlots.end())
  {
    log->LogError(420, SimpleLogger::MESSAGE,
                  L"REC_KEY contents not all unique");
    throw VXIException::Fatal();
  }

  // (6) After a successful recognition, the <initial> elements are all
  //     disabled.  This must happen BEFORE executing <filled> elements.
  for (VXMLNodeIterator it2(answerForm); it2; ++it2) {
    VXMLNode child2 = *it2;

    if (child2.GetType() != VXMLNode::Type_VXMLElement) continue;
    const VXMLElement & elem = reinterpret_cast<const VXMLElement &>(child2);
    if (elem.GetName() == NODE_INITIAL) {
      vxistring itemName;
      elem.GetAttribute(ATTRIBUTE__ITEMNAME, itemName);
      exe->script.SetVar(itemName, L"true");
    }
  }

  // (7) Process <filled> elements

  vxistring ALLNAMES;

  // Get all names
  for (VXMLNodeIterator iter(answerForm); iter; ++iter) {
    VXMLNode child = *iter;
    if (child.GetType() != VXMLNode::Type_VXMLElement) continue;
    const VXMLElement & elem = reinterpret_cast<const VXMLElement &>(child);
    vxistring id;
    if (!elem.GetAttribute(ATTRIBUTE__ITEMNAME, id) || id.empty()) continue;
    if (!ALLNAMES.empty()) ALLNAMES += L' ';
    ALLNAMES += id;
  }

  for (VXMLNodeIterator it(answerForm); it; ++it) {
    VXMLNode child = *it;

    if (child.GetType() != VXMLNode::Type_VXMLElement) continue;
    const VXMLElement & elem = reinterpret_cast<const VXMLElement &>(child);

    // (7.1) Handle filled elements at the form level.

    if (elem.GetName() == NODE_FILLED) {
      vxistring mode;
      vxistring namelist;
      elem.GetAttribute(ATTRIBUTE_MODE, mode);
      elem.GetAttribute(ATTRIBUTE_NAMELIST, namelist);

      if (namelist.empty())
        namelist = ALLNAMES;

      // Find the intersection between the slots & the namelist.
      STRINGDEQUE names, intersection;
      ProcessNameList(namelist, names);
      std::set_intersection(filledSlots.begin(), filledSlots.end(),
                            names.begin(), names.end(),
                            std::back_inserter(intersection));
      if (intersection.empty()) continue;

      // Test the mode requirements.
      if (mode == L"any") {
        VXIMapHolder vars(NULL);
        execute_content(elem, vars);
        continue;
      }

      bool doit = true;
      for (STRINGDEQUE::const_iterator i = names.begin(); i != names.end();++i)
      {
        doit &= exe->script.IsVarDefined(*i);
        if (!doit) break;
      }

      if (doit) {
        VXIMapHolder vars(NULL);
        execute_content(elem, vars);
        continue;
      }
    }

    // (7.2) Locate form items corresponding to filled slots.

    if (!IsFormItemNode(elem)) continue;

    STRINGDEQUE::const_iterator i;
    vxistring itemName;
    elem.GetAttribute(ATTRIBUTE__ITEMNAME, itemName);
    for (i = filledSlots.begin(); i != filledSlots.end(); ++i)
      if ((*i) == itemName) break;
    if (i == filledSlots.end()) continue;

    // (7.3) Look inside for filled elements.

    for (VXMLNodeIterator it(elem); it; ++it) {
      VXMLNode itemChild = *it;

      if (itemChild.GetType() != VXMLNode::Type_VXMLElement) continue;
      const VXMLElement & itemElem
        = reinterpret_cast<const VXMLElement &>(itemChild);
      if (itemElem.GetName() == NODE_FILLED) {
        VXIMapHolder vars(NULL);
        execute_content(itemElem, vars);
      }
    }
  }

  log->LogDiagnostic(2, L"VXI::ProcessRecognitionResult - done");
}


void VXI::EasyFilled(const vxistring & filled, const VXMLElement & answerForm)
{
  // (1) Process <filled> elements

  for (VXMLNodeIterator it(answerForm); it; ++it) {
    VXMLNode child = *it;

    if (child.GetType() != VXMLNode::Type_VXMLElement) continue;
    const VXMLElement & elem = reinterpret_cast<const VXMLElement &>(child);

    // (1.1) Handle filled elements at the form level.

    if (elem.GetName() == NODE_FILLED) {
      vxistring mode;
      vxistring namelist;
      elem.GetAttribute(ATTRIBUTE_MODE, mode);
      elem.GetAttribute(ATTRIBUTE_NAMELIST, namelist);

      if (namelist.empty()) { // Handle the simple case first.
        VXIMapHolder vars(NULL);
        execute_content(elem, vars);
        continue;
      }

      // Find the intersection between the slots & the namelist.
      STRINGDEQUE names;
      ProcessNameList(namelist, names);
      if (std::find(names.begin(), names.end(), filled) == names.end())
        continue;

      // Test the mode requirements.
      if (mode == L"any") {
        VXIMapHolder vars(NULL);
        execute_content(elem, vars);
        continue;
      }

      bool doit = true;
      for (STRINGDEQUE::const_iterator i = names.begin(); i != names.end();++i)
      {
        doit &= exe->script.IsVarDefined(*i);
        if (!doit) break;
      }

      if (doit) {
        VXIMapHolder vars(NULL);
        execute_content(elem, vars);
        continue;
      }
    }

    // (1.2) Locate form items corresponding to filled slots.

    if (!IsFormItemNode(elem)) continue;

    // UNUSED VARIABLE STRINGDEQUE::const_iterator i;
    vxistring itemName;
    elem.GetAttribute(ATTRIBUTE__ITEMNAME, itemName);
    if (itemName != filled) continue;

    // (1.3) Look inside for filled elements.

    for (VXMLNodeIterator it(elem); it; ++it) {
      VXMLNode itemChild = *it;

      if (itemChild.GetType() != VXMLNode::Type_VXMLElement) continue;
      const VXMLElement & itemElem
        = reinterpret_cast<const VXMLElement &>(itemChild);
      if (itemElem.GetName() == NODE_FILLED) {
        VXIMapHolder vars(NULL);
        execute_content(itemElem, vars);
      }
    }
  }

  // (6) After a successful recognition, the <initial> elements are all
  //     disabled.
  for (VXMLNodeIterator it2(answerForm); it2; ++it2) {
    VXMLNode child2 = *it2;

    if (child2.GetType() != VXMLNode::Type_VXMLElement) continue;
    const VXMLElement & elem = reinterpret_cast<const VXMLElement &>(child2);
    if (elem.GetName() == NODE_INITIAL) {
      vxistring itemName;
      elem.GetAttribute(ATTRIBUTE__ITEMNAME, itemName);
      exe->script.SetVar(itemName, L"true");
    }
  }
}


//#############################################################################
// Prompt related
//#############################################################################

class VXIPromptTranslator : public PromptTranslator {
public:
  virtual VXIValue * EvaluateExpression(const vxistring & expression)
  { if (expression.empty()) return NULL;
    return jsi.EvalScriptToValue(expression); }

  virtual void SetVariable(const vxistring & name, const vxistring & value)
  { if (name.empty()) return;
    jsi.SetString(name, value); }

  VXIPromptTranslator(Scripter & j) : jsi(j) { }
  virtual ~VXIPromptTranslator() { }

private:
  Scripter & jsi;
};


void VXI::queue_prompts(const VXMLElement & doc)
{
  // If an event has occurred, prompt playing may be disabled.  If so, we
  // must re-enable it for the next recognition.
  if (!exe->playingPrompts) {
    log->LogDiagnostic(2, L"VXI::queue_prompts - disabled");
    exe->playingPrompts = true;
    return;
  }

  log->LogDiagnostic(2, L"VXI::queue_prompts()");

  int count = 1;
  int correctcount = 1;

  vxistring itemname;
  doc.GetAttribute(ATTRIBUTE__ITEMNAME, itemname);
  if (itemname.empty())
    log->LogError(999, SimpleLogger::MESSAGE, L"unnamed internal node found");
  else
    count = exe->promptcounts.Increment(itemname);

  typedef std::deque<bool> PROMPTFLAGS;
  PROMPTFLAGS flags;

  // Walk through nodes.  Determine closes match to the count - curiously, only
  // <prompt> elements have an associated count.  By using the PROMPTFLAGS
  // container, we guarentee that we evaluate the conditions exactly once as
  // required by the spec.

  for (VXMLNodeIterator it(doc); it; ++it) {
    VXMLNode child = *it;

    if (child.GetType() != VXMLNode::Type_VXMLElement) continue;
    const VXMLElement & elem = reinterpret_cast<const VXMLElement &>(child);
    if (elem.GetName() != NODE_PROMPT) continue;

    // Evaluate the condition for each prompt that is encountered.
    vxistring cond;
    elem.GetAttribute(ATTRIBUTE_COND, cond);
    if (cond.empty())
      flags.push_back(true);
    else
      flags.push_back(exe->script.TestCondition(cond));
    if (!flags.back()) continue;
    
    // Retrieve the associated count.
    vxistring countvar;
    elem.GetAttribute(ATTRIBUTE_COUNT, countvar);
    if (countvar.empty()) continue;

    // Update the best match (if necessary)
    int icount = exe->script.EvalScriptToInt(countvar);
    if (icount <= count && icount > correctcount) correctcount = icount;
  }

  // And walk through once again, playing all matching prompts.  Here we do
  // evaluate the count a second time, but this should be harmless.

  for (VXMLNodeIterator it2(doc); it2; ++it2) {
    VXMLNode child2 = *it2;

    switch (child2.GetType()) {
    case VXMLNode::Type_VXMLContent:
      if (count == 1) PlayPrompt(child2, doc);
      break;
    case VXMLNode::Type_VXMLElement:
    {
      const VXMLElement & elem = reinterpret_cast<const VXMLElement &>(child2);
      VXMLElementType type = elem.GetName();

      if ((type == NODE_AUDIO || type == NODE_ENUMERATE ||
           type == NODE_VALUE) && count == 1)
      {
        PlayPrompt(child2, doc);
      }
      else if (type == NODE_PROMPT) {
        bool cond = flags.front();
        flags.pop_front();
        if (!cond) continue;

        vxistring countvar;
        elem.GetAttribute(ATTRIBUTE_COUNT, countvar);
        int icount;
        if (countvar.length() == 0) icount = 1;
        else icount = exe->script.EvalScriptToInt(countvar);

        if (icount == correctcount) PlayPrompt(child2, doc);
      }
      break;
    }
    default: // skip it
      break;
    }
  }
}


void VXI::executable_prompt(const VXMLNode& child)
{
  // The 'cond' attribute is still active on prompts in executable content.
  // Handle this special case.

  if (child.GetType() == VXMLNode::Type_VXMLElement) {
    const VXMLElement & elem = reinterpret_cast<const VXMLElement &>(child);
    if (elem.GetName() == NODE_PROMPT) {
      vxistring cond;
      elem.GetAttribute(ATTRIBUTE_COND, cond);
      if (!cond.empty() && !exe->script.TestCondition(cond))
        return;
    }
  }

  // Otherwise, play the prompt.
  PlayPrompt(child,
             (exe->eventSource==0) ? child.GetParent() : exe->eventSource);
}


// The second element, item, represents a 'best guess' as to where execution is
// when this prompt is being queued.
//
void VXI::PlayPrompt(const VXMLNode & prompt, const VXMLElement & item)
{
  log->LogDiagnostic(2, L"VXI::PlayPrompt()");

  // Create new translator for ECMA script.
  VXIPromptTranslator translator(exe->script);

  pm->Queue(prompt, item, exe->properties, translator);
}

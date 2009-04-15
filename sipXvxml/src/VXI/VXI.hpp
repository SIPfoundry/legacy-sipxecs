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
 * VXI class decl. Implementation in VXI_*.cpp
 *
 ***********************************************************************/

#ifndef _VXI_H
#define _VXI_H

#include "VXIvalue.h"                     // for VXIchar, VXImap
#include "CommonExceptions.hpp"           // for VXIException class
#include "DocumentModel.hpp"
#include "InternalMutex.hpp"
#include <string>

typedef std::basic_string<VXIchar> vxistring;

class AnswerInformation;
class DocumentParser;
class ExecutionContext;
class JumpDoc;
class PromptManager;
class PropertyList;
class RecognitionAnswer;
class SimpleLogger;
extern "C" struct VXIinetInterface;
extern "C" struct VXIjsiInterface;
extern "C" struct VXIpromptInterface;
extern "C" struct VXIrecInterface;
extern "C" struct VXIrecRecognitionResult;
extern "C" struct VXItelInterface;
extern "C" struct VXIobjectInterface;

class VXI {
public:
  VXI();  // May throw VXIException::OutOfMemory
  ~VXI();

  int Run(const VXIchar * initialDocument, const VXIMap * args,
          SimpleLogger * log, VXIinetInterface * inet,
          VXIjsiInterface * jsi, VXIrecInterface * rec,
          VXIpromptInterface * prompt, VXItelInterface * tel,
          VXIobjectInterface * object, VXIValue ** result);
  // Returns: -2 Fatal error
  //          -1 Out of memory
  //           0 Success
  //           1 Infinite loop suspected.
  //           2 Javascript error
  //           3 Invalid startup documents

  enum PropertyID {
    BeepURI,
    PlatDefaultsURI
  };

  bool SetRuntimeProperty(PropertyID, const VXIchar * value);
  // Returns: true  - Property set
  //          false - Invalid parameter value

private:
  void GetRuntimeProperty(PropertyID, vxistring &) const;

  ////////////////////////////////////////////////////////////////////////////
  // Document level functions
  ////////////////////////////////////////////////////////////////////////////

  int RunOuterLoop(const vxistring & initialDocument, const VXIMap * args,
                   VXIValue ** result);

  void DoOuterJump(const VXMLElement & doc, const vxistring & url,
                   VXIMap * submitData = NULL, bool isSubdialog = false);

  VXMLElement FindDialog(const VXMLElement & doc, const vxistring & name);
  // Finds the named dialog in the document.  If the name is empty, the first
  // item is returned.

  ////////////////////////////////////////////////////////////////////////////
  // Document level support functions
  ////////////////////////////////////////////////////////////////////////////

  bool PushExecutionContext(const VXIMap * sessionArgs);
  // Returns: true - success
  //          false - failure (stack depth exceeded?)

  void PopExecutionContext();

  void InstallDocument(JumpDoc &);

  void ProcessRootScripts(VXMLElement & doc);

  void AttemptDocumentLoad(const vxistring & rawURL,
                                const VXIMapHolder & urlProperties,
                                VXMLDocument & doc,
                                VXIMapHolder & docProperties,
                                bool isDefaults = false);

  void PrepareDocumentTree(VXMLElement & doc);
  // Recursively walks the document tree and assigns internal names as needed.

private:
  ////////////////////////////////////////////////////////////////////////////
  // Dialog level functions
  ////////////////////////////////////////////////////////////////////////////
  void RunInnerLoop();

  void DoInnerJump(const VXMLElement & elem, const vxistring & item);
  // Either throws an event containing the next form item to execute or
  // simply returns if none is found.

  ////////////////////////////////////////////////////////////////////////////
  // Dialog level support functions
  ////////////////////////////////////////////////////////////////////////////

  void FormInit(const VXMLElement & form, VXIMapHolder & params);
  // Perform initialization associated with property tags and form level
  // variables.  Reset the event and prompts counts.

  bool IsFormItemNode(const VXMLElement& doc);
  // Returns true iff this element is a 'form item'.

private:
  ////////////////////////////////////////////////////////////////////////////
  // Collect Phase and element related.
  ////////////////////////////////////////////////////////////////////////////

  void CollectPhase(const VXMLElement& form, const VXMLElement& item);

  void ProcessReturn(const VXMLElement& form, const VXMLElement& item,
                     VXIValue * & result);

  void DoEvent(const VXMLElement & item,
               const VXIException::InterpreterEvent & event);

  bool do_event(const VXMLElement & item,
                const VXIException::InterpreterEvent & event);
  // Returns: true - event handled successfully.
  //          false - no handler available.

  void do_recognition(const VXIMapHolder & properties);

  void EasyFilled(const vxistring & filled, const VXMLElement & form);
  // A simplified version of the algorithm from do_recognition.

  void ProcessRecognitionResult(const VXMLElement & dialog,
                                const VXMLElement & element,
                                bool usedDTMF,
                                const RecognitionAnswer & answer);

  void execute_content(const VXMLElement & doc, const VXIMapHolder & vars);

  void executable_element(const VXMLElement& child);
  void executable_prompt(const VXMLNode& child);

  void assign_element(const VXMLElement& doc);
  void clear_element(const VXMLElement& doc);
  void disconnect_element(const VXMLElement& doc);
  void goto_element(const VXMLElement& doc);
  void exit_element(const VXMLElement& doc);
  void if_element(const VXMLElement& doc);
  void log_element(const VXMLElement& doc);
  void meta_element(const VXMLElement & doc);
  void reprompt_element(const VXMLElement& doc);
  void return_element(const VXMLElement& doc);
  void script_element(const VXMLElement& doc);
  void submit_element(const VXMLElement& doc);
  void throw_element(const VXMLElement& doc);
  void var_element(const VXMLElement & doc);

  void block_element(const VXMLElement& doc);
  void field_element(const VXMLElement& form, const VXMLElement& field);
  void menu_element(const VXMLElement& doc);
  void object_element(const VXMLElement& doc);
  void record_element(const VXMLElement& form, const VXMLElement& doc);
  void subdialog_element(const VXMLElement& doc);
  void transfer_element(const VXMLElement & form, const VXMLElement& doc);

  VXIMap * CollectParams(const VXMLElement & doc, bool isObject);

private: // All prompt related.
  void PlayPrompt(const VXMLNode& child, const VXMLElement & activeItem);

  void queue_prompts(const VXMLElement& doc);

private:
  DocumentParser     * parser;  // owned
  SimpleLogger       * log;
  VXIinetInterface   * inet;
  VXIrecInterface    * rec;
  VXIjsiInterface    * jsi;
  VXItelInterface    * tel;
  VXIobjectInterface * object;
  PromptManager      * pm;      // owned

  VXIMap             * sdParams;
  VXIValue           * sdResult;

  VXMLDocument domDefaultDoc;

  // Used by Get/Set Property
  InternalMutex mutex;
  vxistring uriPlatDefaults;
  vxistring uriBeep;

  int stackDepth;
  ExecutionContext * exe;
};

#endif

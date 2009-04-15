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

#include "VXIvalue.h"
#include <string>
#include <vector>

typedef std::basic_string<VXIchar> vxistring;

extern "C" struct VXIrecGrammar;
extern "C" struct VXIrecInterface;
extern "C" struct VXIrecRecognitionResult;
extern "C" struct VXIrecRecordResult;
class GrammarInfo;
class PropertyList;
class SimpleLogger;
class VXMLElement;


class RecognitionAnswer {
public:
  RecognitionAnswer();
  ~RecognitionAnswer();
  RecognitionAnswer(RecognitionAnswer &);
  RecognitionAnswer & operator=(RecognitionAnswer &);

  void Bind(VXIrecRecognitionResult *);
  void Clear();

  typedef std::vector<const VXIVector *> VXIVECTORS;
  unsigned int numAnswers;
  VXIVECTORS keys;
  VXIVECTORS values;
  VXIVECTORS scores;
  VXIVECTORS utts;

  const VXIContent * waveform;

private:
  VXIrecRecognitionResult * result;
};


class GrammarManager {
public:
  static const VXIchar * const DTMFTerm;
  static const VXIchar * const FinalSilence;
  static const VXIchar * const MaxTime;
  static const VXIchar * const RecordingType;

  void LoadGrammars(const VXMLElement& doc, vxistring & id, PropertyList &);
  // This function recursively walks through the document, creating grammars as
  // necessary.  The id of the document (passed to Enable) is returned.
  //
  // may throw: VXIException::InterpreterError & VXIException::OutOfMemory

  void DisableAllGrammars();
  // Deactivates all current grammars.

  bool EnableGrammars(const vxistring & documentID,
                      const vxistring & dialogName,
                      const vxistring & fieldName,
                      const VXIMapHolder & properties,
                      bool isModal);
  // Activates (for recognition) grammars matching the given dialog & field
  // name.  The documentID is returned by LoadGrammars.
  //
  // Returns: false - no grammars were enabled
  //          true  - at least one grammar is ready
  //
  // may throw VXIException::InterpreterEvent

  enum {
    Success,       // Recognition returned a hypothesis
    SuccessDTMF,   // Recognition returned a hypothesis entered using DTMF
    Failure,       // Speech detected, no likely hypothesis
    Timeout,       // No speech was detected
    Disconnect,    // Caller has disconnected; no hypothesis
    Error,         // An error aborted recognition
    OutOfMemory,   // Insufficient memory available
    BadMimeType,   // The requested type is not supported
    InternalError  // VXIrecInterface::Recognize failed gave invalid results
  };

  int Recognize(const VXIMapHolder & properties,
                RecognitionAnswer & rsltStruct,
                VXMLElement & recNode);

  int Record(const VXIMapHolder & properties, VXIrecRecordResult * & answer);

  void ReleaseGrammars();
  // Deletes all current grammars.

  VXIMap * GetRecProperties(const PropertyList &, int timeout = -1) const;
  VXIMap * GetRecordProperties(const PropertyList &, int timeout = -1) const;

  GrammarManager(VXIrecInterface * r, const SimpleLogger & l);
  // may throw: VXIException::OutOfMemory()

  ~GrammarManager();

private:
  void AddGrammar(VXIrecGrammar * gr, const vxistring & docID,
                  const VXMLElement & elem);
  // Invoked by LoadGrammars to commit each grammar to the collection.
  //
  // may throw: VXIException::OutOfMemory()
  
  static bool GetEnclosedText(const SimpleLogger & log,
                              const VXMLElement & doc, vxistring & str);
  static void BuildOptionGrammar(const SimpleLogger & log,
                                 const VXMLElement& doc, bool isDTMF,
                                 vxistring & gram);
  static VXIrecGrammar * CreateGrammarFromString(VXIrecInterface * vxirec,
                                                 const SimpleLogger & log,
                                                 const vxistring & source,
                                                 const VXIchar * type,
                                                 const VXIMapHolder & props);

private:
  typedef std::vector<GrammarInfo *> GRAMMARS;
  GRAMMARS grammars;
  const SimpleLogger & log;
  VXIrecInterface * vxirec;
};


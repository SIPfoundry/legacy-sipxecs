/****************License************************************************
 *
 * Copyright 2000.  SpeechWorks International, Inc.  
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
 * Rec integration utils
 *
 ****************************************************************************/

#ifndef _OSBREC_UTILS
#define _OSBREC_UTILS

#include "VXItypes.h"
#include "VXIvalue.h"
#include "VXIlog.h"
#include <list>
#include <stack>
#include <string>
typedef std::basic_string<VXIchar> vxistring;


// Constants for diagnostic logging tags
//
static const VXIunsigned DIAG_TAG_RECOGNITION = 0;
static const VXIunsigned DIAG_TAG_GRAMMARS    = 1;
static const VXIunsigned DIAG_TAG_RECORDING   = 2;
static const VXIunsigned DIAG_TAG_RECOGNITION_RESULT = 5;

/******************************************
 * OSBrecWordList : A list of words
 ******************************************/
struct OSBrecWordListEntry {
  const vxistring word;
  const vxistring return_val;
  const int min_repeat;    // used to support grammars for <0-1> or +
  const int max_repeat;    // max = -1 means no maximum limit

  OSBrecWordListEntry(const vxistring & w, 
                      const vxistring & val,
                      const int n,
                      const int x)
    : word(w), return_val(val), min_repeat(n), max_repeat(x)  { }
};

typedef std::list<OSBrecWordListEntry *> WORDS;

class OSBrecGrammar {
 public:
  virtual void SetEnabled(bool) = 0;
  // Sets the flag indicating whether or not the grammar is enabled for the
  // next recognition.

  virtual bool IsEnabled() const = 0;
  // Returns current enabled state of this grammar.

  virtual bool CheckPhrase(const VXIchar* best, const VXIchar** val) const = 0;
  // Is this phrase supported by this grammar?  If so, find the matching
  // value string.

  virtual int CheckPhraseRep(const VXIchar* best, const VXIchar** val, int* maxlen, bool poundPressed) const = 0;
  // Is this phrase supported by this grammar?  If so, find the matching
  // value string. This function supports the repeatition attributes.

  OSBrecGrammar() { }
  virtual ~OSBrecGrammar() { }
};

class OSBrecWordList : public OSBrecGrammar {
  enum GTYPE {
    GTYPE_NONE,
    GTYPE_DTMF,
    GTYPE_SPEECH
  };

  WORDS words;

  bool enabled;
  GTYPE gtype;

public:
  enum AddWordResult {
    ADDWORD_RESULT_SUCCESS = 0,
    ADDWORD_RESULT_SPEECH_AND_DTMF = 1,
    ADDWORD_RESULT_GENERIC_ERROR = 2
  };

public: // OSBrecGrammar fuctions...
  virtual int CheckPhraseRep(const VXIchar* best, const VXIchar** val, int* maxlen, bool poundPressed) const;
  virtual bool CheckPhrase(const VXIchar* best, const VXIchar** val) const;
  virtual void SetEnabled(bool e)    { enabled = e;    }
  virtual bool IsEnabled() const     { return enabled; }
  virtual bool is_dtmf (const VXIchar* word) const;

public:
  AddWordResult AddWord(const vxistring & word, const vxistring & value, const int min, const int max);
  // Adds a new word/value pair to the list.
  //
  // Returns: AddWordResult as defined above

  OSBrecWordList();
  virtual ~OSBrecWordList();
};


struct OSBrecData {

public:
  void Clear();
  // Release all existing grammars.

  void AddGrammar(OSBrecGrammar *);
  // Add a new grammar.  The OSBrecData assumes ownership of the grammar.

  void FreeGrammar(OSBrecGrammar *);
  // Removed an exising grammar.  The corresponding grammar is destroyed.

  OSBrecGrammar * FindGrammarForPhraseAfterPound(const VXIchar* best,
                                                 const VXIchar** val,
                                                 int* maxlen);
  // Returns a grammar pointer and return value for an enabled grammar
  // containing this phrase.  Otherwise, no grammar matches and NULL is
  // returned. This function is called when the '#' key is pressed to end
  // the input.

  OSBrecGrammar * FindGrammarForPhrase(const VXIchar* best,
				       const VXIchar** val,
               int* maxlen);
  // Returns a grammar pointer and return value for an enabled grammar
  // containing this phrase.  Otherwise, no grammar matches and NULL is
  // returned.

  OSBrecGrammar * CreateWordListFromString(const VXIchar* str) const;
  // Returns a grammar pointer from a string, which may be JSGF. On failure
  // NULL is returned.

  VXIlogResult Error(VXIunsigned errorID, const VXIchar *format, ...) const;
  VXIlogResult Diag(VXIunsigned tag, const VXIchar *subtag,
		    const VXIchar *format, ...) const;

  OSBrecData(VXIunsigned diagLogBase, VXIlogInterface *log);
  virtual ~OSBrecData();

private:
  void GrammarGetNextToken(const vxistring & grammar,
			   vxistring::size_type pos,
			   vxistring::size_type end,
			   vxistring & token) const;
  bool GrammarConvertJSGFString(const vxistring & language,
				const vxistring & incoming,
				OSBrecWordList * gp) const;
    
  bool GrammarParseString(const vxistring & language,
				const vxistring & incoming,
				OSBrecWordList * gp) const;
    
private:
  VXIunsigned diagLogBase;
  VXIlogInterface *log;
  typedef std::list<OSBrecGrammar *> GRAMMARS;
  GRAMMARS grammars;
};


#endif /* include guard */

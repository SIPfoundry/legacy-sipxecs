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
 * OSBrec integration utils
 * 
 ****************************************************************************/

#include "osbrec_utils.h"

#include <cstdio>
#include <string>
#include <cstring>
#include <ctype.h>
#include <stdlib.h>
#include <wctype.h>

#include "VXIvalue.h"

#ifdef _wtoi
#define WTOI _wtoi
#else
int WTOI(vxistring tmpstr)
{
  int val;
  int len = tmpstr.length();
  char* astring = new char[len+1];
  memset(astring, 0, len+1);
  for (int i = 0; i < len; i++)
    astring[i] = (char)tmpstr[i];
  val = atoi(astring);
  delete[] astring;
  astring = NULL;
  return val;
}
#endif



OSBrecWordList::OSBrecWordList()
  : enabled(false), gtype(OSBrecWordList::GTYPE_NONE)
{ }


OSBrecWordList::~OSBrecWordList()
{
  for (WORDS::iterator i = words.begin(); i != words.end(); ++i)
  {
    if (*i != 0)
    {
       delete *i;
       *i = 0;
    }
  }
  words.clear();
}


bool OSBrecWordList::CheckPhrase(const VXIchar* best,
                                 const VXIchar** val) const
{
  vxistring phrase(best);

  for (WORDS::const_iterator i = words.begin(); i != words.end(); ++i) {
    if ((*i)->word == phrase) {
      *val = (*i)->return_val.c_str();
      return true;
    }
  }

  return false;
}

// This function checks the phrase and repeataion of the phrase
int OSBrecWordList::CheckPhraseRep(const VXIchar* best,
                                 const VXIchar** val,
                                 int* maxlen,
                                 bool poundPressed) const
{
  vxistring phrase(best);
  vxistring result(L"");

  *maxlen = 0;
  int grlen = 1;
  for (WORDS::const_iterator i = words.begin(); i != words.end(); ++i) 
  {
    int min = (*i)->min_repeat;
    int max = (*i)->max_repeat;

    if (max == -1)
      *maxlen = -1;

    if (*maxlen != -1)
    {
      if (max > *maxlen)
        *maxlen = max;

      grlen = (*i)->word.length();
      if (grlen > *maxlen)
        *maxlen = grlen;
    }

    if (min == max && max == 1)
    {
      if ((*i)->word == phrase) 
      {
        *val = (*i)->return_val.c_str();
        return 1;
      }
    }
  }

  int blen = phrase.length();
  int min = 1;
  int max = 1;
  int matchedDigits = 0;
  for (int in = 0; in < blen;)
  {
    for (WORDS::const_iterator i = words.begin(); i != words.end(); ++i) 
    {
      min = (*i)->min_repeat;
      max = (*i)->max_repeat;
      grlen = (*i)->word.length();

      if ((*i)->word == phrase.substr(in, grlen)) 
      {
        result.append((*i)->return_val);
        matchedDigits++;
        if ((poundPressed && min <= matchedDigits) ||
            max == matchedDigits)
        {
          memcpy((char*)*val, result.c_str(), sizeof(VXIchar) * result.length());
          return 1;
        }
        break;
      }
    }
    in += grlen;
    grlen = 1;
  }

  return 0;
}


// We allow 1 char DTMF grammars
//
inline bool OSBrecWordList::is_dtmf(const VXIchar* word) const
{
  if (word[1] != 0) return false;
  if (word[0] >= '0' && word[0] <= '9') return true;
  if (word[0] == '*') return true;
  if (word[0] == '#') return true;
  return false;
}


OSBrecWordList::AddWordResult 
OSBrecWordList::AddWord(const vxistring & word, const vxistring & value, const int min, const int max)
{
  if (word.empty() || value.empty()) return ADDWORD_RESULT_GENERIC_ERROR;

/*  if (is_dtmf(word.c_str())) {
    if (gtype == OSBrecWordList::GTYPE_SPEECH)
      return ADDWORD_RESULT_SPEECH_AND_DTMF;
    gtype = OSBrecWordList::GTYPE_DTMF;
  }
  else {
    if (gtype == OSBrecWordList::GTYPE_DTMF)
      return ADDWORD_RESULT_SPEECH_AND_DTMF;
    gtype = OSBrecWordList::GTYPE_SPEECH;
  }
*/

  OSBrecWordListEntry * entry = new OSBrecWordListEntry(word, value, min, max);
  words.push_front(entry);

  return ADDWORD_RESULT_SUCCESS;
}

//////////////////////////////////////////////////////
// These routines process JSGF-like grammars.       //
//////////////////////////////////////////////////////

void OSBrecData::GrammarGetNextToken(const vxistring & grammar,
				     vxistring::size_type pos,
				     vxistring::size_type end,
				     vxistring & token) const
{
  // Find first real character
  while (pos != grammar.length() && isspace(grammar[pos])) ++pos;
  if (pos == grammar.length()) {
    token.erase();
    return;
  }

  // Extract wordRaw; we know there is at least one character
  while (iswspace(grammar[end - 1])) --end;
  token = grammar.substr(pos, end - pos);
}


bool OSBrecData::GrammarConvertJSGFString(const vxistring & language,
					  const vxistring & incoming,
					  OSBrecWordList * gp) const
{
  int count = 0;

  // These define the symbols used in the JSGF-lite that defines these grammars
  const char NEXT      = '|';
  const char BEGIN_VAL = '{';
  const char END_VAL   = '}';

  // If the language is not en-US, return an error.
  if (language != L"en-US") return false;

  // These are positions within the string that we're parsing.
  vxistring::size_type pos = 0;
  vxistring::size_type next;
  vxistring::size_type valmark;

  vxistring wordRaw;
  vxistring wordValue;

  while (pos < incoming.length()) {
    // Find the next item
    next = incoming.find(NEXT, pos);
    if (next == vxistring::npos) next = incoming.length();

    // Find the start of the value
    valmark = incoming.find(BEGIN_VAL, pos);
    if (valmark == vxistring::npos || valmark > next) valmark = next;

    // Extract left hand side (raw text)
    GrammarGetNextToken(incoming, pos, valmark, wordRaw);
    if (wordRaw.empty()) {
      pos = next + 1;
      continue;
    }
    pos = valmark + 1;

    // Extract right hand side (value)
    if (valmark != next && pos < incoming.length()) {
      valmark = incoming.find(END_VAL, pos);
      if (valmark == vxistring::npos || valmark > next) {
	Error(201, L"%s%s", L"MSG",
	      L"OSBrec::ConvertGrammar - Mismatched { } pair");
        return false;
      }
      GrammarGetNextToken(incoming, pos, valmark, wordValue);
      if (wordValue.empty()) {
	Error(202, L"%s%s", L"MSG",
	      L"OSBrec::ConvertGrammar - Empty { } pair");
        return false;
      }
      pos = next + 1;
    }
    else
      wordValue.erase();

    // Add word to grammar

    OSBrecWordList::AddWordResult addRc;
    if (!wordValue.empty()) {
      // Got tag value
      Diag(DIAG_TAG_GRAMMARS, L"OSBrec::ConvertGrammar", 
	   L"Adding #%d (%s, %s)", count, wordRaw.c_str(), wordValue.c_str());
      addRc = gp->AddWord(wordRaw, wordValue, 1, 1);
    } else {
      // Didn't get tag value
      Diag(DIAG_TAG_GRAMMARS, L"OSBrec::ConvertGrammar", 
	   L"Adding #%d (%s, %s)", count, wordRaw.c_str(), wordRaw.c_str());
      addRc = gp->AddWord(wordRaw, wordRaw, 1, 1);
    }

    switch (addRc) {
    case OSBrecWordList::ADDWORD_RESULT_SUCCESS:
      break;

    case OSBrecWordList::ADDWORD_RESULT_SPEECH_AND_DTMF:
      Error(200, L"%s%s", L"MSG",
	    L"OSBrec::ConvertGrammar - No support for mixed DTMF and "
	    L"speech grammars");
      return false;
      break;

    case OSBrecWordList::ADDWORD_RESULT_GENERIC_ERROR:
    default:
      Error(203, L"%s%s", L"MSG", L"OSBrec::ConvertGrammar - Add word failed");
      return false;
    }

    ++count;
  }

  Diag(DIAG_TAG_GRAMMARS, L"OSBrec::ConvertGrammar", L"Found %d item(s).",
       count);

  return count != 0;
}


bool OSBrecData::GrammarParseString(const vxistring & language,
					  const vxistring & incoming,
					  OSBrecWordList * gp) const
{
  int count = 0;

  // These define the symbols used in the JSGF that defines these grammars
  const char NEXT       = '|';
  // UNUSED VARIABLE const char BEGIN_OR   = '[';
  // UNUSED VARIABLE const char END_OR     = ']';
  const char BEGIN_AND  = '(';
  const char END_AND    = ')';
  const char BEGIN_REP  = '{';
  const char END_REP    = '}';
  const char REPEAT1    = '+';
  const char TO         = '-';
  // UNUSED VARIABLE const char OPTIONAL   = '?';

  // If the language is not en-US, return an error.
  if (language != L"en-US") return false;

  // These are positions within the string that we're parsing.
  vxistring::size_type pos = 0;
  vxistring::size_type next;
  vxistring::size_type rep_start;
  vxistring::size_type rep_end;
  vxistring::size_type and_start;
  vxistring::size_type and_end;
  vxistring::size_type valmark;

  vxistring wordRaw;
  vxistring wordValue;

  int min = 1;
  int max = 1;

  unsigned int stop = incoming.length();
  while (pos < stop) {
    // Find the next item

    and_start = incoming.find(BEGIN_AND, pos);
    if (and_start != vxistring::npos) // finding '('
    {
      pos = and_start + 1;  // start seraching after '('
      and_end = incoming.find(END_AND, pos);   // finding ')'
      if (and_end != vxistring::npos) 
      {
        stop = and_end;   // only one () is parsed!!!
        valmark = incoming.find(REPEAT1, and_end);  // finding '+'
        if (valmark != vxistring::npos) 
        {
          min = 1;      // 1 or more times
          max = -1;
        }
        else if ((rep_start = incoming.find(BEGIN_REP, and_end)) != vxistring::npos) // finding '<'
        {
          rep_end = incoming.find(END_REP, rep_start);  // finding '>'
          if (rep_end != vxistring::npos) 
          {
            if ((valmark = incoming.find(TO, rep_start)) != vxistring::npos) // finding '-'
            {
              GrammarGetNextToken(incoming, (rep_start + 1), valmark, wordRaw);
              if (wordRaw.empty())
                min = -1;
              else
                min = WTOI(wordRaw.c_str());
              GrammarGetNextToken(incoming, (valmark + 1), rep_end, wordRaw);
              if (wordRaw.empty())
                max = -1;
              else
                max = WTOI(wordRaw.c_str());
            }
            else
            {
              GrammarGetNextToken(incoming, (rep_start + 1), rep_end, wordRaw);
              min = max = WTOI(wordRaw.c_str());
            }
            if (min > max && max != -1)
            {
              int tmp = max;
              max = min;
              min = tmp;
            }
          }
        }
      }
    }

    next = incoming.find(NEXT, pos);
    if (next == vxistring::npos) next = stop;

    // Extract left hand side (raw text)
    GrammarGetNextToken(incoming, pos, next, wordRaw);
    if (wordRaw.empty()) {
      pos = next + 1;
      continue;
    }
    pos = next + 1;

    // Add word to grammar
    OSBrecWordList::AddWordResult addRc;
    wordValue = wordRaw;
    if (!wordValue.empty()) {
      // Got tag value
      Diag(DIAG_TAG_GRAMMARS, L"OSBrec::ConvertGrammar", 
	   L"Adding #%d (%s, %s)", count, wordRaw.c_str(), wordValue.c_str());
      addRc = gp->AddWord(wordRaw, wordValue, min, max);
    } else {
      // Didn't get tag value
      Diag(DIAG_TAG_GRAMMARS, L"OSBrec::ConvertGrammar", 
	   L"Adding #%d (%s, %s)", count, wordRaw.c_str(), wordRaw.c_str());
      addRc = gp->AddWord(wordRaw, wordRaw, min, max);
    }

    switch (addRc) {
    case OSBrecWordList::ADDWORD_RESULT_SUCCESS:
      break;

    case OSBrecWordList::ADDWORD_RESULT_SPEECH_AND_DTMF:
      Error(200, L"%s%s", L"MSG",
	    L"OSBrec::ConvertGrammar - No support for mixed DTMF and "
	    L"speech grammars");
//      return false;
      break;

    case OSBrecWordList::ADDWORD_RESULT_GENERIC_ERROR:
    default:
      Error(203, L"%s%s", L"MSG", L"OSBrec::ConvertGrammar - Add word failed");
      return false;
    }

    ++count;
  }

  Diag(DIAG_TAG_GRAMMARS, L"OSBrec::ConvertGrammar", L"Found %d item(s).",
       count);

  return count != 0;
}



OSBrecGrammar * OSBrecData::CreateWordListFromString(const VXIchar* text) const
{
  OSBrecWordList * gp = new OSBrecWordList();
  if (gp == NULL) return NULL;

  if (!GrammarParseString(L"en-US", text, gp)) {  //GrammarConvertJSGFString
    delete gp;
    gp = NULL;
    return NULL;
  }

  return gp;
}


/******************************************
 * OSBrecData : The grammar container
 ******************************************/

OSBrecData::OSBrecData(VXIunsigned b, VXIlogInterface *l) 
  : diagLogBase(b), log(l), grammars()
{ }


OSBrecData::~OSBrecData()
{
  for (GRAMMARS::iterator i = grammars.begin(); i != grammars.end(); ++i)
  {
    if (*i != 0)  
    {
       delete *i;
       *i = 0;
    }
  }
}


void OSBrecData::Clear()
{
  for (GRAMMARS::iterator i = grammars.begin(); i != grammars.end(); ++i)
  {
    if (*i != 0)  
    {
       delete *i;
       *i = 0;
    }
  }
  grammars.clear();

}


void OSBrecData::AddGrammar(OSBrecGrammar * l)
{
  if (l == NULL) return;
  grammars.push_front(l);
}


void OSBrecData::FreeGrammar(OSBrecGrammar * l)
{
  if (l == NULL) return;
  grammars.remove(l);
  delete l;
  l = NULL;
}


OSBrecGrammar * OSBrecData::FindGrammarForPhrase(const VXIchar* best,
                                                 const VXIchar** val,
                                                 int* maxlen)
{
  OSBrecGrammar * match = NULL;

  for (GRAMMARS::iterator i = grammars.begin(); i != grammars.end(); ++i)
    if ((*i)->IsEnabled() && (*i)->CheckPhraseRep(best, val, maxlen, 0)) {
      if (match != NULL)
	Diag(DIAG_TAG_GRAMMARS, L"OSBrec::FindGrammarForPhrase", 
	     L"Multiple grammars matched %s", best);
      match = *i;
  }

  return match;
}


OSBrecGrammar * OSBrecData::FindGrammarForPhraseAfterPound(const VXIchar* best,
                                                 const VXIchar** val,
                                                 int* maxlen)
{
  OSBrecGrammar * match = NULL;

  for (GRAMMARS::iterator i = grammars.begin(); i != grammars.end(); ++i)
    if ((*i)->IsEnabled() && (*i)->CheckPhraseRep(best, val, maxlen, 1)) {
      if (match != NULL)
	Diag(DIAG_TAG_GRAMMARS, L"OSBrec::FindGrammarForPhraseAfterPound", 
	     L"Multiple grammars matched %s", best);
      match = *i;
  }

  if (!match && best[0] == '#')
  {
    match = CreateWordListFromString(best);
    if (match) AddGrammar(match);
  }

  return match;
}


VXIlogResult OSBrecData::Error(VXIunsigned errorID,
                               const VXIchar *format, ...) const
{
  VXIlogResult rc;
  va_list args;

  if (log)
    return VXIlog_RESULT_NON_FATAL_ERROR;
  
  if (format) {
    va_start(args, format);
    rc = (*log->VError)(log, COMPANY_DOMAIN L".OSBrec", errorID, format, args);
    va_end(args);
  } else {
    rc = (*log->Error)(log, COMPANY_DOMAIN L".OSBrec", errorID, NULL);
  }

  return rc;
}


VXIlogResult OSBrecData::Diag(VXIunsigned tag, const VXIchar *subtag,
                              const VXIchar *format, ...) const
{
  VXIlogResult rc;
  va_list args;

  if (!log)
    return VXIlog_RESULT_NON_FATAL_ERROR;

  if (format) {
    va_start(args, format);
    rc = (*log->VDiagnostic)(log, tag + diagLogBase, subtag, format, args);
    va_end(args);
  } else {
    rc = (*log->Diagnostic)(log, tag + diagLogBase, subtag, NULL);
  }

  return rc;
}

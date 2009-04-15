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
 * Error message logging to files.
 *
 ***********************************************************************/

#include "SimpleLogger.hpp"
#include "VXIlog.h"
#include <sstream>

static unsigned int MESSAGE_BASE;
static const VXIchar * const MODULE_NAME  = COMPANY_DOMAIN L".vxi";

// ------*---------*---------*---------*---------*---------*---------*---------

static const VXIchar * const TAG_ENCODING  = L"encoding";
static const VXIchar * const TAG_EXCEPTION = L"exception";
static const VXIchar * const TAG_MESSAGE   = L"message";
static const VXIchar * const TAG_URI       = L"uri";

static const VXIchar * const TAG_LABEL     = L"label";
static const VXIchar * const TAG_EXPR      = L"expr";
static const VXIchar * const TAG_CONTENT   = L"content";

inline const VXIchar * const GetInfoTagText(SimpleLogger::InfoTag t)
{
  switch (t) {
  case SimpleLogger::ENCODING:    return TAG_ENCODING;
  case SimpleLogger::EXCEPTION:   return TAG_EXCEPTION;
  case SimpleLogger::MESSAGE:     return TAG_MESSAGE;
  case SimpleLogger::URI:         return TAG_URI;
  default:                        return TAG_MESSAGE;
  }
}

inline const VXIchar * const GetEventTagText(SimpleLogger::EventTag t)
{
  switch (t) {
  case SimpleLogger::LABEL:       return TAG_LABEL;
  case SimpleLogger::EXPR:        return TAG_EXPR;
  case SimpleLogger::CONTENT:     return TAG_CONTENT;
  default:                        return TAG_MESSAGE;
  }
}

// ------*---------*---------*---------*---------*---------*---------*---------

class SimpleLoggerImpl : public SimpleLogger {
public:
  SimpleLoggerImpl(VXIlogInterface * l) : log(l) { }
  ~SimpleLoggerImpl() { }

  // Diagnostic...
  typedef unsigned int TAGID;

  virtual bool IsLogging(TAGID tagID) const
  { return (log->DiagnosticIsEnabled(log, MESSAGE_BASE + tagID) == TRUE); }

  virtual std::basic_ostream<wchar_t> & StartDiagnostic(TAGID tagID) const
  { id = tagID;
    buffer.str(L"");
    return buffer; }

  virtual void EndDiagnostic() const
  { if (buffer.str().empty()) return;
    log->Diagnostic(log, MESSAGE_BASE + id, NULL, L"%ls", buffer.str().c_str()); }

  virtual void LogDiagnostic(TAGID tagID, const wchar_t * text) const
  { log->Diagnostic(log, MESSAGE_BASE + tagID, NULL, L"%ls", text); }

  // Error

  virtual void LogError(int errorNum,
                        SimpleLogger::InfoTag i1, const wchar_t * txt1,
                        SimpleLogger::InfoTag i2, const wchar_t * txt2) const
  { log->Error(log, MODULE_NAME, errorNum, L"%ls%ls%ls%ls",
               GetInfoTagText(i1), txt1, GetInfoTagText(i2), txt2); }

  virtual void LogError(int errorNum,
                        SimpleLogger::InfoTag i, const wchar_t * txt) const
  { log->Error(log, MODULE_NAME, errorNum, L"%ls%ls", GetInfoTagText(i), txt); }

  virtual void LogError(int errorNum) const
  { log->Error(log, MODULE_NAME, errorNum, L""); }

  // Event

  virtual void LogEvent(int eventNum,
                        SimpleLogger::EventTag i1, const wchar_t * txt1,
                        SimpleLogger::EventTag i2, const wchar_t * txt2) const
  { log->Event(log, eventNum, L"%ls%ls%ls%ls",
               GetEventTagText(i1), txt1, GetEventTagText(i2), txt2); }

  virtual void LogEvent(int eventNum,
                        SimpleLogger::EventTag i, const wchar_t * txt) const
  { log->Event(log, eventNum, L"%ls%ls", GetEventTagText(i), txt); }

  virtual void LogEvent(int eventNum) const
  { log->Event(log, eventNum, L""); }

private:
  VXIlogInterface * log;
  mutable TAGID id;
  mutable std::basic_ostringstream<wchar_t> buffer;
};



SimpleLogger * SimpleLogger::CreateResource(VXIlogInterface * l)
{
  return new SimpleLoggerImpl(l);
}


void SimpleLogger::DestroyResource(SimpleLogger * & log)
{
  if (log == NULL) return;
  delete log;
  log = NULL;
}


void SimpleLogger::SetMessageBase(unsigned int base)
{
  MESSAGE_BASE = base;
}

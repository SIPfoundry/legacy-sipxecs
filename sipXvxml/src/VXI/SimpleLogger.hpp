#ifndef _VXI_SimpleLogger_HPP
#define _VXI_SimpleLogger_HPP
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

#include <cwchar>
#include <ostream>

extern "C" struct VXIlogInterface;
class SimpleLoggerImpl;

class SimpleLogger {
  friend class SimpleLoggerImpl;
public:
  enum InfoTag {
    ENCODING,
    EXCEPTION,
    MESSAGE,
    URI,
  };

  enum EventTag {
    LABEL,
    EXPR,
    CONTENT,
  };

  static void SetMessageBase(unsigned int base);
  static SimpleLogger * CreateResource(VXIlogInterface *);
  static void DestroyResource(SimpleLogger * &);

public:
  typedef unsigned int TAGID;

  // Error logging
  virtual void LogError(int errorNum) const = 0;
  virtual void LogError(int errorNum,
                        InfoTag, const wchar_t * text) const = 0;
  virtual void LogError(int errorNum,
                        InfoTag, const wchar_t * text1,
                        InfoTag, const wchar_t * text2) const = 0;

  // Event logging
  virtual void LogEvent(int eventNum) const = 0;
  virtual void LogEvent(int eventNum,
                        EventTag, const wchar_t * text) const = 0;
  virtual void LogEvent(int eventNum,
                        EventTag, const wchar_t * text1,
                        EventTag, const wchar_t * text2) const = 0;

  // Diagnostic logging.  There are two basic modes.
  virtual bool IsLogging(TAGID tagID) const = 0;
  virtual std::basic_ostream<wchar_t> & StartDiagnostic(TAGID tagID) const = 0;
  virtual void EndDiagnostic() const = 0;
  virtual void LogDiagnostic(TAGID tagID, const wchar_t * text) const = 0;

private:
  SimpleLogger() { }
  virtual ~SimpleLogger() { }
};

#endif  /* include guard */

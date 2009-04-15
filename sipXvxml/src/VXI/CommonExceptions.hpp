#ifndef _CommonExceptions_H
#define _CommonExceptions_H
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
 * Common exceptions used by components in the VXI.
 *
 ***********************************************************************/

#include "VXIvalue.h"
#include <string>

typedef std::basic_string<VXIchar> vxistring;

class VXIException {
public:
  class OutOfMemory { };

  class JavaScriptError { };

  class InterpreterEvent {
  public:
    InterpreterEvent(const vxistring & v, const vxistring & m = vxistring())
      : val(v), message(m)  { }
    ~InterpreterEvent()                             { }

    const vxistring & GetValue() const              { return val; }
    const vxistring & GetMessage() const            { return message; }

  private:
    vxistring val;
    vxistring message;
  };

  class Fatal { };

  class Exit {
  public:
    Exit(VXIValue * v) : exprResult(v) { }

    VXIValue * exprResult;
  };

private: // unimplemented
  VXIException();
  ~VXIException();
};

#endif

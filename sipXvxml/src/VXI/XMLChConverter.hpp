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

#include "VXItypes.h"                 // for VXIchar
#include <util/XercesDefs.hpp>        // for XMLCh
#include <util/XMLString.hpp>         // for XMLString

// Xerces specifies that XMLCh must be able to store UTF-16 characters.
// VXIchar should be the general wide character representation (wchar_t) of the
// platform.  As wchar_t may be other types, conversion functions are
// necessary.

// ------*---------*---------*---------*---------*---------*---------*---------

// The native Solaris and Linux wide character encoding is UTF-32.  This
// provides an imperfect conversion from UTF-16 to UTF-32, ignoring all
// surrogate pairs.

#if defined(__linux__) || \
    defined(SOLARIS) || defined(__SVR4) || defined(UNIXWARE) || defined(__hpux)
#define UTF16TO32

// ------*---------*---------*---------*---------*---------*---------*---------

// Windows uses UTF-16 (or UCS-2 which is nearly equivalent), so no conversion
// is necessary.
#elif defined(XML_WIN32)
#define NOCONVERSION

// ------*---------*---------*---------*---------*---------*---------*---------

#else
#error Platform not supported.
#endif

// ------*---------*---------*---------*---------*---------*---------*---------

#if defined(NOCONVERSION)
#include <cstring>

inline bool Compare(const XMLCh * x, const VXIchar * y)
{ return wcscmp(x, y) == 0; }

struct VXIcharToXMLCh {
  const XMLCh * c_str() const                 { return cstr; }
  VXIcharToXMLCh(const VXIchar * x) : cstr(x) { }
  ~VXIcharToXMLCh()                           { }

private:
  const XMLCh * cstr;
  VXIcharToXMLCh(const VXIcharToXMLCh &);
  VXIcharToXMLCh& operator=(const VXIcharToXMLCh &);
};

struct XMLChToVXIchar {
  const VXIchar * c_str() const             { return cstr; }
  XMLChToVXIchar(const XMLCh * x) : cstr(x) { }
  ~XMLChToVXIchar() { }

private:
  const VXIchar * cstr;
  XMLChToVXIchar(const XMLChToVXIchar &);
  XMLChToVXIchar& operator=(const XMLChToVXIchar &);
};

#endif /* NOCONVERSION */

// ------*---------*---------*---------*---------*---------*---------*---------

#if defined(UTF16TO32)
#include <ostream>

inline bool Compare(const XMLCh * x, const VXIchar * y)
{
  if (x == NULL && y == NULL) return true;
  if (x == NULL && *y == '\0') return true;
  if (y == NULL && *x == '\0') return true;
  if (y == NULL || x == NULL) return false;

  while (*x && *y && VXIchar(*x) == *y) ++x, ++y;
  if (*x || *y) return false;
  return true;
}


struct VXIcharToXMLCh {
  const XMLCh * c_str() const { return cstr; }

  VXIcharToXMLCh(const VXIchar * x) : cstr(NULL)
  {
    if (x == NULL) return;
    unsigned int len = wcslen(x);
    cstr = new XMLCh[len + 1];
    if (cstr == NULL) return;
    for (unsigned int i = 0; i < len + 1; ++i)
      // We throw out any surrogate characters (0xD800 - 0xDFFF)
      cstr[i] = ((x[i] & 0xD800) == 0xD800) ? XMLCh(0xBF) : XMLCh(x[i]);
  }
  ~VXIcharToXMLCh() { delete [] cstr; cstr = NULL;}

private:
  XMLCh * cstr;
  VXIcharToXMLCh(const VXIcharToXMLCh &);
  VXIcharToXMLCh& operator=(const VXIcharToXMLCh &);
};


struct XMLChToVXIchar {
  const VXIchar * c_str() const { return cstr; }

  XMLChToVXIchar(const XMLCh * x) : cstr(NULL)
  {
    if (x == NULL) return;
    unsigned int len = XMLString::stringLen(x);
    cstr = new VXIchar[len + 1];
    if (cstr == NULL) return;
    for (unsigned int i = 0; i < len + 1; ++i)
      // We throw out anything above 0xFFFF
      cstr[i] = (x[i] != 0 && (x[i] & ~XMLCh(0xFFFF))) ? VXIchar(0xBE)
                                                       : VXIchar(x[i]);
  }
  ~XMLChToVXIchar() { delete [] cstr; cstr = NULL;}

private:
  VXIchar * cstr;
  XMLChToVXIchar(const XMLChToVXIchar &);
  XMLChToVXIchar& operator=(const XMLChToVXIchar &);
};

#endif /* UTF16TO32 */

// ------*---------*---------*---------*---------*---------*---------*---------

inline std::basic_ostream<VXIchar>& operator<<(std::basic_ostream<VXIchar>& os,
					       const XMLChToVXIchar & val)
{ return os << val.c_str(); }

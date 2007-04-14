/*****************************************************************************
 *****************************************************************************
 *
 * $Id: SBinetString.hpp,v 1.4.6.5 2002/02/04 18:17:57 dmeyer Exp $
 *
 * SBinetString, string class that is a subset of STL wstring
 *
 * The SBinetString class stores a string in a grow-only buffer, a
 * functional subset of the STL wstring class. This header merely
 * exists to make it easy to eliminate the use of STL wstring which
 * does not exist on some Linux versions.
 *
 *****************************************************************************
 ****************************************************************************/


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
 ************************************************************************
 */

// -----1=0-------2=0-------3=0-------4=0-------5=0-------6=0-------7=0-------8

#ifndef _SBINET_STRING_H__
#define _SBINET_STRING_H__

#include "utl/UtlString.h"
#include "SBinetInternal.h"

// Utility method for end users to convert wide to narrow characters
// (characters that cannot be converted become a Latin-1 upside down ?
// character), not used here
inline char SBinetW2C(wchar_t w) {
  return ((w & 0xff00)?'\277':((unsigned char) (w & 0x00ff))); }

#ifdef NO_STL

// Non-STL implementation
#include <stdlib.h>
#include <wchar.h>

class SBinetString {
 public:
  // Constructors and destructor
  SBinetString( ) : data(NULL), allocated(0), dataLen(0) { }
  SBinetString (const VXIchar *str) : data(NULL), allocated(0), dataLen(0) {
    *this += str; }
  SBinetString (const SBinetString &str) : 
    data(NULL), allocated(0), dataLen(0) { *this += str.data; }
  virtual ~SBinetString( ) { if ( data ) free (data); }

  // Assignment operators
  SBinetString & operator= (const SBinetString &str) {
    if ( &str != this ) {
      dataLen = 0;
      *this += str.data;
    }
    return *this;
  }
  SBinetString & operator= (const VXIchar *str) {
    dataLen = 0;
    *this += str;
    return *this;
  }

  // Clear operator
  void clear( ) { dataLen = 0; }

  // Operators for appending data to the string
  SBinetString & operator+= (const SBinetString & str) { 
    return (*this += str.data); }
  SBinetString & operator+= (const VXIchar *str) { 
    unsigned int len = wcslen (str);
    if (( allocated - dataLen > len + 1 ) || ( Grow (len + 1) == true )) {
      wcscpy (&data[dataLen], str);
      dataLen += len;
    }
    return *this; 
  }
  SBinetString & operator+= (VXIchar c) { 
    if (( allocated - dataLen > 2 ) || ( Grow (2) == true )) {
      data[dataLen] = c;
      dataLen += 1;
    }
    return *this; 
  }

  // Operators to access the string data
  unsigned int length( ) const { return dataLen; } 
  const VXIchar *c_str( ) const { return data; }

 private:
  // Grow the buffer
  bool Grow (unsigned int size) {
    if ( size < 128 ) size = 128;
    VXIchar *newData = 
      (VXIchar *) realloc (data, (dataLen + size) * sizeof (VXIchar));
    if ( ! newData )
      return false;
    data = newData;
    allocated = dataLen + size;
    return true;
  }

 private:
  unsigned int  allocated;
  unsigned int  dataLen;
  VXIchar      *data;
};

#else  // NO_STL

// Highly efficient STL wstring implementation, use a wrapper to
// ensure we don't go beyond a specific subset of functionality that
// will break the non-STL implementation
#include <string>

class SBinetString {
 public:
  // Constructors and destructor
  SBinetString( ) : details( ) { }
  SBinetString (const VXIchar *str) : details(str) { }
  SBinetString (const SBinetString &str) : details(str.details) { }
  virtual ~SBinetString( ) { }

  // Assignment operators
  SBinetString & operator= (const SBinetString &str) {
    if ( &str != this )
      details = str.details;
    return *this;
  }
  SBinetString & operator= (const VXIchar *str) {
    details = str;
    return *this;
  }
  SBinetString & operator= (const char *str) {
    details = L"";
    size_t len = ::strlen(str);
    for (size_t i = 0; i < len; i++)
      details += (const VXIchar) str[i];
    return *this;
  }

  // Clear operator
  void clear( ) { details = L""; }

  // Operators for appending data to the string
  SBinetString & operator+= (const SBinetString & str) { 
    details += str.details; return *this; }
  SBinetString & operator+= (const VXIchar *str) { 
    details += str; return *this; }
  SBinetString & operator+= (VXIchar c) { 
    details += c; return *this; }
  SBinetString & operator+= (const char *str) {
    size_t len = ::strlen(str);
    for (size_t i = 0; i < len; i++)
      details += (const VXIchar) str[i];
    return *this;
  }

  // Operators to access the string data
  unsigned int length( ) const { return details.length( ); } 
  const VXIchar *c_str( ) const { return details.c_str( ); }
  VXIchar operator[] (unsigned int i) const { return details[i]; }

  // Operator to search the string for a character
  unsigned int find(VXIchar c) { return details.find (c); }

 private:
  std::basic_string<VXIchar> details;
};

class SBinetNString {
 public:
  // Constructors and destructor
  SBinetNString( ) : details( ) { }
  SBinetNString (const char *str) : details(str) { }
  SBinetNString (const SBinetNString &str) : details(str.details) { }
  virtual ~SBinetNString( ) { }

  // Assignment operators
  SBinetNString & operator= (const SBinetNString &str) {
    if ( &str != this )
      details = str.details;
    return *this;
  }
  SBinetNString & operator= (const char *str) {
    details = str;
    return *this;
  }

  // Clear operator
  void clear( ) { details = ""; }

  // Operators for appending data to the string
  SBinetNString & operator+= (const SBinetNString & str) { 
    details += str.details; return *this; }
  SBinetNString & operator+= (const char *str) { 
    details += str; return *this; }
  SBinetNString & operator+= (char c) { 
    details += c; return *this; }

  // Operators to access the string data
  unsigned int length( ) const { return details.length( ); } 
  const char *c_str( ) const { return details.data( ); }
  char operator[] (unsigned int i) const { return details[i]; }

  // Operator to search the string for a character
  unsigned int find(char c) { return details.first (c); }

  void append(const char* src, int size ) 
  { details.append(src, size ); } 


 private:
  UtlString  details;
};

#endif  // NO_STL

#endif  // _SBINET_STRING_H__

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

#include "Counters.hpp"

// ------*---------*---------*---------*---------*---------*---------*---------

void EventCounter::Increment(const vxistring & eventName)
{ 
  vxistring::size_type start = 0;
  do {
    vxistring::size_type pos = eventName.find('.', start);
    if (pos == vxistring::npos) pos = eventName.length();
    if (pos != start) { // ignore isolated '.' or '..'
      COUNTS::iterator i = counts.find(eventName.substr(0, pos));
      if (i != counts.end()) ++(*i).second;
      else counts[eventName.substr(0, pos)] = 1;
    }
    start = pos + 1;
  } while (start < eventName.length());
}


int EventCounter::GetCount(const vxistring & eventName,
                           const vxistring & rawCatchName) const
{
  if (eventName.empty()) return 0;
  if (rawCatchName.length() > eventName.length()) return 0;

  // This is necessary to support the 'catch all' semantic.  We make the catch
  // as unspecific as possible.

  vxistring catchName(rawCatchName);

  if (catchName.empty()) {
    vxistring::size_type pos = eventName.find('.');
    if (pos == vxistring::npos) catchName = eventName;
    else catchName = eventName.substr(0, pos);
  }

  // Now look up the count for the event.

  if (eventName.find(catchName) != 0) return 0;
  if (eventName.length() != catchName.length() &&
      eventName[catchName.length()] != '.') return 0;

  COUNTS::const_iterator i = counts.find(catchName);
  if (i != counts.end()) return (*i).second;

  return 0; // This shouldn't happen if Increment was called first.
}

// ------*---------*---------*---------*---------*---------*---------*---------

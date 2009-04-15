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

#include "VXItypes.h"
#include <map>
#include <string>

typedef std::basic_string<VXIchar> vxistring;

class EventCounter {
public:
  void Clear()
  { counts.clear(); phaseName.erase(); }

  void ClearIf(const vxistring & phase, bool condition)
  { if ((phase == phaseName) == condition) { counts.clear();
                                             phaseName = phase; } }

  void Increment(const vxistring & eventName);
  // Increments the count associated with all events.  For instance, given
  // 'error.semantic.bad_name', this will increment 'error', 'error.semantic',
  // and 'error.semantic.bad_name'.

  int GetCount(const vxistring & eventName, const vxistring & catchName) const;
  // Returns the count associated with this catch _if_ it matches the event,
  // otherwise zero.

private:
  vxistring phaseName;
  typedef std::map<vxistring, int> COUNTS;
  COUNTS counts;
};


class PromptTracker {
public:
  void Clear()
  { counts.clear(); }

  void Clear(const vxistring & name)
  { COUNTS::iterator i = counts.find(name);
    if (i != counts.end()) counts.erase(i); }

  int Increment(const vxistring & name)
  { COUNTS::iterator i = counts.find(name);
    if (i != counts.end()) return ++(*i).second;
    counts[name] = 1;
    return 1; }

private:
  typedef std::map<vxistring, int> COUNTS;
  COUNTS counts;
};

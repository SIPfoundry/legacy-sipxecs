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

#include "VXItrd.h"

class InternalMutex {
public:
  void Lock() const
  { if (error) return;
    error = (VXItrdMutexLock(mutex) != VXItrd_RESULT_SUCCESS); }

  void Unlock() const
  { if (error) return;
    error = (VXItrdMutexUnlock(mutex) != VXItrd_RESULT_SUCCESS); }

  bool IsBad() const
  { return error; }

  InternalMutex() : mutex(NULL)
  { error = (VXItrdMutexCreate(&mutex) != VXItrd_RESULT_SUCCESS); }

  ~InternalMutex()
  { if (mutex != NULL) VXItrdMutexDestroy(&mutex);  mutex = NULL;  }

private:
  mutable bool error;
  mutable VXItrdMutex * mutex;
};

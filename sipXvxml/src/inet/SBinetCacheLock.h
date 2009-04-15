/****************License************************************************
 *
 * Copyright 2001.  SpeechWorks International, Inc.
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
 * SBinetCacheLock -- try to lock libwww cache 
 *
 * 
 *
 ***********************************************************************/
#ifndef __SBINETCACHELOCK_H_                   /* Allows multiple inclusions */
#define __SBINETCACHELOCK_H_

#include <WWWLib.h>
#include <WWWHTTP.h>
#include <WWWInit.h>

#include "HTCache.h"					 /* Implemented here */
#include "HTAncMan.h"
#include "HTAnchor.h"

#include "VXItrd.h"
#include "VXIinet.h"
#include "SBinetLogger.hpp"

class SBinetCacheLock {
  HTAnchor* m_anchor;
  HTRequest* m_writer;
  int m_readers;
  VXItrdMutex* m_testmutex;
  VXItrdMutex* m_writemutex;

public:
  SBinetCacheLock(HTAnchor* anchor);
  virtual ~SBinetCacheLock();
  VXIbool hasAnchor(HTAnchor* anchor);
  VXItrdResult GetWriteLock(HTRequest* request);
  VXItrdResult GetReadLock(HTRequest* request);
  VXItrdResult FreeWriteLock(HTRequest* request);
  VXItrdResult FreeReadLock(HTRequest* request);
};

class SBinetCacheLockTable : public SBinetLogger {
  SBinetCacheLock** m_table;
  unsigned int m_size;
  SBinetCacheLock* FindCacheLock1(HTAnchor* anchor);

public:
  SBinetCacheLockTable(VXIlogInterface *log, VXIunsigned diagLogBase);
  virtual ~SBinetCacheLockTable();
  static VXIinetResult Init(VXIlogInterface *log, VXIunsigned diagLogBase);
  static VXIinetResult ShutDown(VXIlogInterface *log);
  static SBinetCacheLock* FindCacheLock(HTAnchor* anchor);
};

#endif


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
#include <WWWLib.h>
#include <WWWHTTP.h>
#include <WWWInit.h>

#include "HTCache.h"					 /* Implemented here */
#include "HTAncMan.h"
#include "HTAnchor.h"

#ifdef EINVAL
// Conflicts with OS definition
#undef EINVAL
#endif

#include "SBinetLog.h"
#include "SBinetCacheLock.h"

static SBinetCacheLockTable* g_locktable = NULL;   // the table

VXIinetResult SBinetCacheLockTable::Init(VXIlogInterface *log,
					 VXIunsigned diagLogBase)
{
  if (! log) return VXIinet_RESULT_INVALID_ARGUMENT;
  if (g_locktable) return VXIinet_RESULT_FATAL_ERROR;
  g_locktable = new SBinetCacheLockTable(log, diagLogBase);
  return (g_locktable ? VXIinet_RESULT_SUCCESS : VXIinet_RESULT_OUT_OF_MEMORY);
}

VXIinetResult SBinetCacheLockTable::ShutDown(VXIlogInterface *log)
{
  if (! log) return VXIinet_RESULT_INVALID_ARGUMENT;
  if (! g_locktable) return VXIinet_RESULT_FATAL_ERROR;
  delete g_locktable;
  g_locktable = NULL;
  return VXIinet_RESULT_SUCCESS;
}

SBinetCacheLockTable::SBinetCacheLockTable(VXIlogInterface *log,
					   VXIunsigned diagLogBase)
  : SBinetLogger(MODULE_SBINET_CACHE_LOCK, log, diagLogBase)
{
  m_size = 1024;
  m_table = new SBinetCacheLock*[m_size];
  for(unsigned int i=0;i<m_size;i++) m_table[i] = NULL;
}

SBinetCacheLockTable::~SBinetCacheLockTable()
{
  if(m_table) 
  {
     delete[] m_table;
     m_table = 0;
  }

}

SBinetCacheLock*
SBinetCacheLockTable::FindCacheLock(HTAnchor* anchor){
  //  return(g_locktable->FindCacheLock1((HTAnchor*)anchor));
  /*
   * A gross hack to prevent sharing of data structures in libwww
   *   This serializes requests, but this seems to be necessary to
   *   avoid the POST corruption bug we were seeing (Entity_callback
   *   not being called properly on 100 responses).
   */
  return(g_locktable->FindCacheLock1((HTAnchor*)37));
}

/*
 * Anchors last for the duration of the libwww instance so no need to
 * worry about invalid data.  Should really rewrite using STL
 * hashtables or something that grows
 */
SBinetCacheLock*
SBinetCacheLockTable::FindCacheLock1(HTAnchor* anchor){
  if(!m_table) return(NULL);

  size_t hash = (size_t)anchor % m_size;
  size_t i;
  for(i=hash;i<m_size;i++){
    SBinetCacheLock* lock = m_table[i];
    if(lock && lock->hasAnchor(anchor)) return(lock);
    if(lock == NULL){
      lock = new SBinetCacheLock(anchor);
      m_table[i] = lock;
      return(lock);
    }
  }
  for(i=0;i<hash;i++){
    SBinetCacheLock* lock = m_table[i];
    if(lock && lock->hasAnchor(anchor)) return(lock);
    if(lock == NULL){
      lock = new SBinetCacheLock(anchor);
      m_table[i] = lock;
      return(lock);
    }
  }

  Error(227, NULL);
  return(NULL);
}

/*
 * OK, the current implementation is a total punt. We are going to
 * lock each anchor for the entire transaction. Although this sounds
 * severe, it reality it does not seem to affect performance and is
 * the only way I have been able to get stability.  The ReadLock and
 * WriteLock functions now do the same thing (wait/free anchor
 * mutex). There is vestigial support for multiple readers it anyone
 * want to give it another try. Note that problems are not in the
 * locking, but in libwww
 *   
 */

VXIbool
SBinetCacheLock::hasAnchor(HTAnchor* anchor)
{
  return (anchor == m_anchor ? TRUE : FALSE);
}

/*
 * Note: we only call this when don't have the libwww mutex, so we can block
 */
VXItrdResult
SBinetCacheLock::GetWriteLock(HTRequest* request){
  VXItrdResult err;
  err = VXItrdMutexLock(m_writemutex);
  if (err != VXItrd_RESULT_SUCCESS) 
    g_locktable->Error(108, NULL);

  g_locktable->Diag(MODULE_SBINET_CACHE_LOCK_TAGID, NULL, 
		    L"Thread %x has WriteLock", VXItrdThreadGetID());
  g_locktable->Diag(MODULE_SBINET_CACHE_LOCK_TAGID, NULL, 
		    L"Anchor %x has %d readers", m_anchor, m_readers);
  return(err);
}

/*
 * Note: we only call this when we have the libwww mutex, so we are safe
 */
VXItrdResult
SBinetCacheLock::FreeWriteLock(HTRequest* request){
  VXItrdResult err;
  g_locktable->Diag(MODULE_SBINET_CACHE_LOCK_TAGID, NULL, 
		    L"Thread %x freeing WriteLock", VXItrdThreadGetID());
  err = VXItrdMutexUnlock(m_writemutex);
  if (err != VXItrd_RESULT_SUCCESS) 
    g_locktable->Error(108, NULL);
  m_writer = NULL;
  return err;
}

/*
 * Note:  call this when we don't have the libwww mutex, so we are safe
 */
VXItrdResult
SBinetCacheLock::GetReadLock(HTRequest* request){
  VXItrdResult err;
  g_locktable->Diag(MODULE_SBINET_CACHE_LOCK_TAGID, NULL, 
		    L"Thread %x in ReadLock m_writemutex 0x%08x", VXItrdThreadGetID(), m_writemutex);
  err = VXItrdMutexLock(m_writemutex);
  g_locktable->Diag(MODULE_SBINET_CACHE_LOCK_TAGID, NULL, 
		    L"Thread %x exiting ReadLock m_writemutex 0x%08xi with err = %d", VXItrdThreadGetID(), m_writemutex, err);
  return(err);
}


/*
 * Note: we only call this when we have the libwww mutex, so we are safe
 */
VXItrdResult
SBinetCacheLock::FreeReadLock(HTRequest* request){
  VXItrdResult err;
  g_locktable->Diag(MODULE_SBINET_CACHE_LOCK_TAGID, NULL,
		    L"Thread %x freeing ReadLock m_writemutex 0x%08x", VXItrdThreadGetID(), m_writemutex);
  err = VXItrdMutexUnlock(m_writemutex);
  g_locktable->Diag(MODULE_SBINET_CACHE_LOCK_TAGID, NULL,
		    L"Thread %x exiting freeing ReadLock m_writemutex 0x%08x with err = %d", VXItrdThreadGetID(), m_writemutex, err);
  return err;
}

SBinetCacheLock::SBinetCacheLock(HTAnchor* anchor){ 
  m_anchor = anchor; 
  m_writer = NULL; 
  m_readers= 0; 
  VXItrdMutexCreate(&m_writemutex);
}

SBinetCacheLock::~SBinetCacheLock(){
  VXItrdMutexDestroy(&m_writemutex);
}




/*
 * Copyright (c) eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
	#include <stdio.h>
	#include <stdarg.h>
	#include <unistd.h>
	#include <pthread.h>
	#include <sys/times.h>
	#include <limits.h>
	#include <sys/time.h>
	#include <time.h>
	#include <stdlib.h>

#include <string>
#include <boost/lexical_cast.hpp>

#include "sipdb/UnqliteDB.h"
#include "os/OsLogger.h"
#include "os/qsTypes.h"

extern "C"
{
  #include "sipdb/unqlite.h"
}

 
struct KeyConsumer
{
  std::string filter;
  UnqliteDB::Keys* keys;
};

struct RecordConsumer
{
  std::string filter;
  std::string key;
  UnqliteDB::Records* records;
};

static bool string_ends_with(const std::string& str, const char* key)
{
  size_t i = str.rfind(key);
  return (i != std::string::npos) && (i == (str.length() - ::strlen(key)));
}

static bool string_wildcard_compare(const char* wild, const std::string& str)
{
  const char* string = str.c_str();

  const char *cp = NULL, *mp = NULL;

  while ((*string) && (*wild != '*')) {
    if ((*wild != *string) && (*wild != '?')) {
      return 0;
    }
    wild++;
    string++;
  }

  while (*string) {
    if (*wild == '*') {
      if (!*++wild) {
        return 1;
      }
      mp = wild;
      cp = string+1;
    } else if ((*wild == *string) || (*wild == '?')) {
      wild++;
      string++;
    } else {
      wild = mp;
      string = cp++;
    }
  }

  while (*wild == '*') {
    wild++;
  }
  return !*wild;
}

static UInt64 getTime()
{
  struct timeval sTimeVal;
	gettimeofday( &sTimeVal, NULL );
	return (UInt64)( sTimeVal.tv_sec * 1000 + ( sTimeVal.tv_usec / 1000 ) );
}

template <typename T>
std::string string_from_number(T var)
  /// This is a template function that converts basic types to std::string
{
  return boost::lexical_cast<std::string>(var);
}

template <typename T>
T string_to_number(const std::string& str)
  /// Convert a string to a numeric value
{
  try { return boost::lexical_cast<T>(str);} catch(...){return 0;};
}

static int RecordConsumerCallback(const void *pData,unsigned int nDatalen,void *pUserData)
{
  RecordConsumer* pConsumer = static_cast<RecordConsumer*>(pUserData);
  
  if (pConsumer && nDatalen && pData)
  {
    std::string data((const char*)pData, nDatalen);
    
    if (pConsumer->key.empty())
    {
      //
      // There is no key yet so we assume this is a key assignment
      //
      if (!pConsumer->filter.empty() && pConsumer->filter != "*")
      {
        if (!string_wildcard_compare(pConsumer->filter.c_str(), data))
        {
          return UNQLITE_OK;
        }
      }
      
      //
      // Do not process expiration keys
      //
      if (!string_ends_with(pConsumer->key, PERSISTENT_STORE_EXPIRES_SUFFIX))
      {
        pConsumer->key = data;
      }
      else
      {
        return UNQLITE_OK;
      }
    }
    else
    {
      //
      // There is a key.  This is a data assignment
      //
      UnqliteDB::Record record;
      record.key = pConsumer->key;
      record.value = data;
      pConsumer->records->push_back(record);
      pConsumer->key.clear();
    }
  }
  
  return UNQLITE_OK;
}

static int KeyConsumerCallback(const void *pData,unsigned int nDatalen,void *pUserData)
{
  KeyConsumer* pConsumer = static_cast<KeyConsumer*>(pUserData);
  
  if (pConsumer && nDatalen && pData)
  {
    std::string data((const char*)pData, nDatalen);
    
    if (!pConsumer->filter.empty() && pConsumer->filter != "*")
    {
      if (!string_wildcard_compare(pConsumer->filter.c_str(), data))
        return UNQLITE_OK;
    }
    
    //
    // Do not process expiration keys
    //
    if (!string_ends_with(data, PERSISTENT_STORE_EXPIRES_SUFFIX))
    {
      pConsumer->keys->push_back(data);
    }
    else
    {
      return UNQLITE_OK;
    }
  }
  
  return UNQLITE_OK;
}

UnqliteDB::UnqliteDB() :
  _pDbHandle(0)
{
}

UnqliteDB::UnqliteDB(const std::string& path) :
  _pDbHandle(0)
{
  open(path);
}

UnqliteDB::~UnqliteDB()
{
  close();
}

void UnqliteDB::log_error()
{
  unqlite* pDbHandle = static_cast<unqlite*>(_pDbHandle);
  if (!pDbHandle)
    return;

  const char *zBuf = 0;
  int iLen;
  unqlite_config(pDbHandle,UNQLITE_CONFIG_ERR_LOG,&zBuf,&iLen);
  if( iLen > 0 )
  {
    OS_LOG_ERROR(FAC_DB, "UnqliteDB Exception:  " << zBuf);
  }
}

bool UnqliteDB::close()
{
  unqlite* pDbHandle = static_cast<unqlite*>(_pDbHandle);
  if (pDbHandle)
  {
    if (unqlite_close(pDbHandle) == UNQLITE_OK)
    {
      _pDbHandle = 0;
      return true;
    }
    else
    {
      log_error();
      return false;
    }
  }
  return true;
}

bool UnqliteDB::open(const std::string& path)
{
  if (isOpen())
    return false;

  _path = path;

  unqlite* pDbHandle = 0;

  if (unqlite_open(&pDbHandle, path.c_str(), UNQLITE_OPEN_CREATE) != UNQLITE_OK || !pDbHandle)
  {
    log_error();
    return false;
  }

  _pDbHandle = pDbHandle;

  return true;
}

bool UnqliteDB::put(const std::string& key, const std::string& value)
{
  if (key.size() > PERSISTENT_STORE_MAX_KEY_SIZE || value.size() > PERSISTENT_STORE_MAX_VALUE_SIZE)
  {
    OS_LOG_ERROR(FAC_DB, "UnqliteDB::put:  Maximum data/ size exceeded");
    return false;
  }
  unqlite* pDbHandle = static_cast<unqlite*>(_pDbHandle);
  if (!pDbHandle)
    return false;

  mutex_write_lock lock(_mutex);

  if (unqlite_begin(pDbHandle) != UNQLITE_OK)
  {
    log_error();
    return false;
  }

  if (unqlite_kv_store(pDbHandle, key.c_str(), key.size(), value.c_str(), value.size()) != UNQLITE_OK)
  {
    log_error();
    unqlite_rollback(pDbHandle);
    return false;
  }
  return true;
}

bool UnqliteDB::put(const std::string& key, const std::string& value, unsigned int expireInSeconds)
{
  if (key.size() > PERSISTENT_STORE_MAX_KEY_SIZE || value.size() > PERSISTENT_STORE_MAX_VALUE_SIZE)
  {
    OS_LOG_ERROR(FAC_DB, "UnqliteDB::put:  Maximum data/ size exceeded");
    return false;
  }

  unqlite* pDbHandle = static_cast<unqlite*>(_pDbHandle);
  if (!pDbHandle)
    return false;

  mutex_write_lock lock(_mutex);

  if (unqlite_begin(pDbHandle) != UNQLITE_OK)
  {
    log_error();
    return false;
  }

  if (unqlite_kv_store(pDbHandle, key.c_str(), key.size(), value.c_str(), value.size()) != UNQLITE_OK)
  {
    log_error();
    unqlite_rollback(pDbHandle);
    return false;
  }

  UInt64 expires = getTime() + (expireInSeconds*1000);
  std::string expireString = string_from_number(expires);
  std::string expireKey = key + std::string(PERSISTENT_STORE_EXPIRES_SUFFIX);

  if (unqlite_kv_store(pDbHandle, expireKey.c_str(), expireKey.size(), expireString.c_str(), expireString.size()) != UNQLITE_OK)
  {
    log_error();
    unqlite_rollback(pDbHandle);
    return false;
  }

  unqlite_commit(pDbHandle);

  return true;
}

bool UnqliteDB::get(const std::string& key, std::string& value)
{
  if (is_expired(key))
  {
    purge_expired(key);
    return false;
  }
    
  return _get(key, value);
}

bool UnqliteDB::_get(const std::string& key, std::string& value)
{
  unqlite* pDbHandle = static_cast<unqlite*>(_pDbHandle);
  if (!pDbHandle)
  {
    return false;
  }

  mutex_read_lock lock(_mutex);

  char buff[PERSISTENT_STORE_MAX_VALUE_SIZE];

  unqlite_int64 nBytes = PERSISTENT_STORE_MAX_VALUE_SIZE;
  if (unqlite_kv_fetch(pDbHandle, key.c_str(),key.size(), buff, &nBytes) != UNQLITE_OK)
  {
    log_error();
    return false;
  }

  value = std::string(buff, nBytes);

  return true;
}

bool UnqliteDB::is_expired(const std::string& key)
{
  std::string expireKey = key + std::string(PERSISTENT_STORE_EXPIRES_SUFFIX);

  std::string expires;
  if (!_get(expireKey, expires))
    return false;

  UInt64 expireTime = string_to_number<UInt64>(expires);
  
  return expireTime <= getTime();
}

bool UnqliteDB::purge_expired(const std::string& key)
{
  std::string expireKey = key + std::string(".expires");
  if (!del(key))
    return false;
  return del(expireKey);
}


bool UnqliteDB::del(const std::string& key)
{
  unqlite* pDbHandle = static_cast<unqlite*>(_pDbHandle);
  if (!pDbHandle)
    return false;

  mutex_write_lock lock(_mutex);

  if (unqlite_kv_delete(pDbHandle, key.c_str(), key.size()) != UNQLITE_OK)
  {
    log_error();
    return false;
  }

  return true;
}

bool UnqliteDB::getKeys(const std::string& filter, Keys& keys)
{
  unqlite* pDbHandle = static_cast<unqlite*>(_pDbHandle);
  if (!pDbHandle)
  {
    return false;
  }

  mutex_read_lock lock(_mutex);
  
  unqlite_kv_cursor* pCursor = 0;
  
  if (unqlite_kv_cursor_init(pDbHandle, &pCursor) != UNQLITE_OK || !pCursor)
  {
    log_error();
    return false;
  }
  
  KeyConsumer consumer;
  consumer.filter = filter;
  consumer.keys = &keys;
  
  /* Point to the first record */
	for (
    unqlite_kv_cursor_first_entry(pCursor); 
    unqlite_kv_cursor_valid_entry(pCursor);
    unqlite_kv_cursor_next_entry(pCursor))
  {
    unqlite_kv_cursor_key_callback(pCursor, KeyConsumerCallback,(void*)&consumer);
  }
  
  unqlite_kv_cursor_release(pDbHandle, pCursor);
  
  return true;
}

bool UnqliteDB::getRecords(const std::string& filter, Records& records)
{
  unqlite* pDbHandle = static_cast<unqlite*>(_pDbHandle);
  if (!pDbHandle)
  {
    return false;
  }

  mutex_read_lock lock(_mutex);
  
  unqlite_kv_cursor* pCursor = 0;
  
  if (unqlite_kv_cursor_init(pDbHandle, &pCursor) != UNQLITE_OK || !pCursor)
  {
    log_error();
    return false;
  }
  
  RecordConsumer consumer;
  consumer.filter = filter;
  consumer.records = &records;
  
  /* Point to the first record */
	for (
    unqlite_kv_cursor_first_entry(pCursor); 
    unqlite_kv_cursor_valid_entry(pCursor);
    unqlite_kv_cursor_next_entry(pCursor))
  {
    unqlite_kv_cursor_key_callback(pCursor, RecordConsumerCallback,(void*)&consumer);
    if (!consumer.key.empty())
    {
      //
      // We got a key so consume the data
      //
      unqlite_kv_cursor_data_callback(pCursor, RecordConsumerCallback,(void*)&consumer);
    }
  }
  
  unqlite_kv_cursor_release(pDbHandle, pCursor);
  
  return true;
}

bool UnqliteDB::delKeys(const std::string& filter)
{
  unqlite* pDbHandle = static_cast<unqlite*>(_pDbHandle);
  if (!pDbHandle)
    return false;
  
  Keys keys;
  if (!getKeys(filter, keys))
    return false;
  
  if (!keys.empty())
  {
    mutex_write_lock lock(_mutex);
    for (Keys::const_iterator iter = keys.begin(); iter != keys.end(); iter++)
    {
      if (unqlite_kv_delete(pDbHandle, iter->c_str(), iter->size()) != UNQLITE_OK)
      {
        log_error();
        return false;
      }
    }
  }
   
  return true;
}








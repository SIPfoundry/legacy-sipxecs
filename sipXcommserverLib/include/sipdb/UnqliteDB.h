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


#ifndef UNQLITEDB_H_INCLUDED
#define UNQLITEDB_H_INCLUDED

#include <boost/thread.hpp>


#define PERSISTENT_STORE_MAX_VALUE_SIZE 65536
#define PERSISTENT_STORE_MAX_KEY_SIZE 1024
#define PERSISTENT_STORE_EXPIRES_SUFFIX ".KV_EXPIRES"


class UnqliteDB : boost::noncopyable
{
public:
  struct Record
  {
    std::string key;
    std::string value;
  };
  
  typedef std::vector<std::string> Keys;
  typedef std::vector<Record> Records;
  typedef boost::shared_mutex mutex_read_write;
  typedef boost::shared_lock<boost::shared_mutex> mutex_read_lock;
  typedef boost::lock_guard<boost::shared_mutex> mutex_write_lock;
  typedef void* HANDLE;
  
  UnqliteDB();
  
  UnqliteDB(const std::string& path);

  ~UnqliteDB();

  bool open(const std::string& path);

  bool isOpen();

  bool close();

  bool put(const std::string& key, const std::string& value);

  bool put(const std::string& key, const std::string& value, unsigned int expireInSeconds);

  bool get(const std::string& key, std::string& value);

  bool del(const std::string& key);
  
  bool getKeys(Keys& keys);
  
  bool getKeys(const std::string& filter, Keys& keys);
  
  bool getRecords(Records& records);
  
  bool getRecords(const std::string& filter, Records& records);
  
  bool delKeys(const std::string& filter);
  
  const std::string getPath() const;
private:

  void log_error();

  bool is_expired(const std::string& key);

  bool purge_expired(const std::string& key);

  bool _get(const std::string& key, std::string& value);

  HANDLE _pDbHandle;

  std::string _path;

  mutex_read_write _mutex;
};


//
// Inlines
//

inline bool UnqliteDB::isOpen()
{
  return _pDbHandle != 0;
}

inline const std::string UnqliteDB::getPath() const
{
  return _path;
}

inline bool UnqliteDB::getKeys(Keys& keys)
{
  return getKeys("", keys);
}

inline bool UnqliteDB::getRecords(Records& records)
{
  return getRecords("", records);
}


#endif
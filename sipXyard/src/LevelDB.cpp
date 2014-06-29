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


#include "sipxyard/LevelDB.h"


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

static std::string string_left(const std::string& str, size_t size)
{
  if (str.size() <= size)
    return str;
  return str.substr(0, size);
}

LevelDB::LevelDB() :
  _pDb(0)
{
  
}
  
LevelDB::~LevelDB()
{
  close();
}

bool LevelDB::open(const std::string& path)
{
  _path = path;
  leveldb::Options options;
  options.create_if_missing = true;
  leveldb::Status status = leveldb::DB::Open(options, path, &_pDb);
  return status.ok();
}

bool LevelDB::isOpen()
{
  return _pDb != 0;
}

bool LevelDB::close()
{
  delete _pDb;
  _pDb = 0;
  return true;
}

bool LevelDB::put(const std::string& key, const std::string& value)
{
  return _pDb->Put(leveldb::WriteOptions(), key, value).ok();
}

bool LevelDB::get(const std::string& key, std::string& value)
{
  return _pDb->Get(leveldb::ReadOptions(), key, &value).ok();
}

bool LevelDB::del(const std::string& key)
{
  return _pDb->Delete(leveldb::WriteOptions(), key).ok();
}

bool LevelDB::getKeys(Keys& keys)
{
  leveldb::Iterator* it = _pDb->NewIterator(leveldb::ReadOptions());
  for (it->SeekToFirst(); it->Valid(); it->Next()) 
  {
    std::string key = it->key().ToString();
    keys.push_back(key);
  }
  
  bool status = it->status().ok();
  delete it;
  return status;
}

bool LevelDB::getKeys(const std::string& filter, Keys& keys)
{
  if (filter == "*" || filter.empty())
    return getKeys(keys);
  
  std::string startKey = string_left(filter, filter.size() - 1);
  
  leveldb::Iterator* it = _pDb->NewIterator(leveldb::ReadOptions());
  for (it->Seek(startKey); it->Valid(); it->Next()) 
  {
    std::string key = it->key().ToString();
    bool validKey = true;
    validKey = string_wildcard_compare(filter.c_str(), key);

    if (validKey)
      keys.push_back(key);
    else
      break;
  }
  
  bool status = it->status().ok();
  delete it;
  return status;
}

bool LevelDB::getRecords(Records& records)
{
  leveldb::Iterator* it = _pDb->NewIterator(leveldb::ReadOptions());
  for (it->SeekToFirst(); it->Valid(); it->Next()) 
  {
    Record record;
    record.key = it->key().ToString();
    record.value = it->value().ToString();
    records.push_back(record);
  }
  
  bool status = it->status().ok();
  delete it;
  return status;
}

bool LevelDB::getRecords(const std::string& filter, Records& records)
{
  if (filter == "*" || filter.empty())
    return getRecords(records);
  
  std::string key = string_left(filter, filter.size() - 1);
  
  leveldb::Iterator* it = _pDb->NewIterator(leveldb::ReadOptions());
  for (it->Seek(key); it->Valid(); it->Next()) 
  {
    Record record;
    record.key = it->key().ToString();
    bool validKey = true;
    if (!filter.empty() && filter != "*")
      validKey = string_wildcard_compare(filter.c_str(), record.key);

    if (validKey)
    {
      record.value = it->value().ToString();
      records.push_back(record);
    }
    else
    {
      break;
    }
  }
  
  bool status = it->status().ok();
  delete it;
  return status;
}

bool LevelDB::delKeys(const std::string& filter)
{
  Keys keys;
  if (!getKeys(filter, keys))
  return false;
  
  for (Keys::const_iterator iter= keys.begin(); iter != keys.end(); iter++)
    del(*iter);
  
  return true;
}







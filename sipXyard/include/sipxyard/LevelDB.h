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

#ifndef LEVELDB_H_INCLUDED
#define	LEVELDB_H_INCLUDED

#include <string>
#include <vector>
#include <leveldb/db.h>
#include <boost/noncopyable.hpp>


class LevelDB : boost::noncopyable
{
public: 
  struct Record
  {
    std::string key;
    std::string value;
  };

  typedef std::vector<std::string> Keys;
  typedef std::vector<Record> Records;

  LevelDB();
  
  ~LevelDB();

  bool open(const std::string& path);

  bool isOpen();

  bool close();

  bool put(const std::string& key, const std::string& value);

  bool get(const std::string& key, std::string& value);
  
  bool del(const std::string& key);
  
  bool getKeys(Keys& keys);
  
  bool getKeys(const std::string& filter, Keys& keys);
  
  bool getRecords(Records& records);
  
  bool getRecords(const std::string& filter, Records& records);
  
  bool delKeys(const std::string& filter);
  
  const std::string getPath() const;
  
private:
  leveldb::DB* _pDb;
  std::string _path;
};


//
// Inlines
//

inline const std::string LevelDB::getPath() const
{
  return _path;
}


#endif	// LEVELDB_H_INCLUDED


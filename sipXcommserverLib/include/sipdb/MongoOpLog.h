/*
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
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

#ifndef MONGOOPLOG_H
#define	MONGOOPLOG_H

#include "sipdb/MongoDB.h"
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <vector>
#include <string>

class MongoOpLog : public MongoDB::BaseDB
{
public:
  enum OpLog
  {
    Insert,
    Delete,
    Update,
    All
  };
  typedef boost::function<void(const std::string&)> OpLogCallBack;
  MongoOpLog(const MongoDB::ConnectionInfo& info, const std::string ns);
  ~MongoOpLog();
  void registerCallback(OpLog type, OpLogCallBack cb);
  void run();
  void stop();
protected:
  void internal_run();
  void notifyCallBacks(const std::string& type, const std::string& opLog);
  bool _isRunning;
  boost::thread* _pTailThread;
  mongo::ScopedDbConnection* _pTailConnection;
  std::vector<OpLogCallBack> _allCb;
  std::vector<OpLogCallBack> _insertCb;
  std::vector<OpLogCallBack> _deleteCb;
  std::vector<OpLogCallBack> _updateCb;
  std::string _ns;

};


#endif	/* MONGOOPLOG_H */


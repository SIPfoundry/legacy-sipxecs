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
  enum OpLogType
  {
    FirstOpLog = 0,
    Insert = FirstOpLog,
    Delete,
    Update,
    All,
    OpLogDataNumber = All,
    LastOpLog,// The last callback
    OpLogsNumber = LastOpLog// Total number of callbacks
  };

  static const char* ts_fld();
  static const char* op_fld();

  typedef boost::function<void(const mongo::BSONObj&, const std::string&)> OpLogCallBack;
  typedef std::vector<OpLogCallBack> OpLogCbVector;

  /**
   * Constructor:
   *
   * @param info - contain connection data and the table used for query (Ex: local.oplog.rs)
   * @param customQuery - A specific query given by the user in order to be informed of changes
   *  only to specific data. For example: BSON("ns" << "test.dbName") - will be notified only
   *  for changes to test.dbName; if not set
   * @param querySleepTime - the timeout in seconds before searching if new entries were added to database;
   *  If not set the processor thread will not sleep between queries.
   * @param timeStamp - the time as number of seconds since 00:00:00, 1 Jan 1970, GMT used
   *  for selecting only the entries that have the timestamp greater than this one;
   *  If not set the client will be notified for all entries
   */
  MongoOpLog(const MongoDB::ConnectionInfo& info,
             const mongo::BSONObj& customQuery = mongo::BSONObj(),
             const int querySleepTime = 0,
             const unsigned long timeStamp = 0);
  ~MongoOpLog();

  /**
   * Registers a callback to be called when new entries are added to "local.oplog.rs" tables.
   * The client can use this to monitor changes to a specific database in mongo
   * @param type - Monitored operation type. It can be:  Insert, Delete, Update or All
   * @param cb - The callback to be called
   */
  void registerCallback(OpLogType type, OpLogCallBack cb);

  // Starts monitor thead
  void run();

  // Stops monitor thread
  void stop();

protected:
  // The main function ran by the monitor thread
  void internal_run();

  //
  bool processQuery(mongo::DBClientCursor* cursor,
                    mongo::BSONElement& lastTailId);

  void createQuery(mongo::BSONObj& query, const mongo::BSONElement& lastTailId);

  bool createFirstQuery(mongo::BSONObj& timeStampObj,
                        mongo::BSONObj& query,
                        mongo::BSONElement& lastTailId);

  // Run callbacks
  void notifyCallBacks(const mongo::BSONObj& bSONObj);

  bool _isRunning; // thread state
  boost::thread* _pThread;
  OpLogCbVector _opLogCbVectors[OpLogsNumber];// Vector of std::vector<OpLogCallBack>

  int _querySleepTime; // the time to sleep in seconds before every new query

  //The time as number of seconds since 00:00:00, 1 Jan 1970, GMT used
  //for selecting only the entries that have the timestamp greater than this one
  unsigned long _timeStamp;

  mongo::BSONObj _customQuery;// Custom query given by user

  typedef struct
  {
    OpLogType opLogType;
    const char* opName;
  }OpLogData;
  typedef OpLogData OpLogDataVec[OpLogDataNumber];

  // An internally used vector that contain a mapping between opLogType and operation name
  //from "local.oplog.rs"
  static const OpLogDataVec& opLogDataVec();
};


#endif	/* MONGOOPLOG_H */


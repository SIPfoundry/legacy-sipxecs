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
#include <map>

/**
 * This class creates a thread that monitors "local.oplog.rs" mongo collection.
 * "local.oplog.rs" is the capped collection that stores an ordered history of
 * logical writes to a MongoDB database. The oplog is the basic mechanism enabling
 * replication in MongoDB. The client of this class can register a callback
 * (OpLogCallBac) in order to be notified for every new entry added to this database.
 * This way, a client can monitor any change to any database from mongo.
 */
class MongoOpLog : public MongoDB::BaseDB
{
public:
  static const std::string NS;
  static const int MULTIPLIER;

  enum OpLogType
  {
    Insert = 0, // Warning Insert must be always the first element
    Delete,
    Update,
    All,
    OpLogTypeNumber
  };

  /**
   * Callback invoked when a new entry is added to "local.oplog.rs" collection
   *
   * @param mongo::BSONObj - describes the current operation
   */
  typedef boost::function<void(const mongo::BSONObj&)> OpLogCallBack;
  typedef std::vector<OpLogCallBack> OpLogCbVector;
  typedef std::map<std::string, OpLogType> OpLogDataMap;

  static const char* ts_fld();// "ts" field (timestamp)
  static const char* op_fld();// "op" field (operation)

  /**
   * Constructor:
   *
   * @param info - contains connection data
   * @param customQuery - A specific query given by the user of this class in order to be informed
   * for changes only to specific data. For example: BSON("ns" << "test.dbName") - will be notified only
   * for changes to test.dbName; if not set if will be informed for all changes
   * @param querySleepTime - the timeout in seconds before searching if new entries were added to database;
   *  If not set the processor thread will not sleep between queries.
   * @param startFromTimestamp - the time as number of seconds elapsed since 00:00:00, 1 Jan 1970, GMT
   * since the client will be notified for new entries. If not set, the client will be notified for all entries
   */
  MongoOpLog(const MongoDB::ConnectionInfo& info,
             const mongo::BSONObj& customQuery = mongo::BSONObj(),
             const int querySleepTime = 0,
             const unsigned long startFromTimestamp = 0);
  ~MongoOpLog();

  /**
   * Registers a callback to be called when new entries are added to "local.oplog.rs" table.
   * The client can use this to monitor changes to a specific database in mongo
   * @param type - Monitored operation type. It can be:  Insert, Delete, Update or All
   * @param cb - The callback to be called
   */
  void registerCallback(OpLogType type, OpLogCallBack cb);

  // Starts monitor thread and sets _isRunning variable true
  bool run();

  // Sets _isRunning variable false and then waits for the monitor thread to finish
  void stop();

protected:
  // The main function ran by the monitor thread
  void internal_run();
  void internal_run_esafe();

  // As long as the cursor is valid (not dead) this function process every new
  // entry added to "local.oplog.rs" collection beginning with the lastEntry
  bool processQuery(mongo::DBClientCursor* cursor,
                    mongo::BSONObj& lastEntry);

  // creates query for filtering entries based on _customQuery and _startFromTimestamp
  void createQuery(const mongo::BSONObj& lastEntry, mongo::BSONObj& query);

  // If startFromTimestamp is set it creates lastEntry BSONElement from it and
  // is calls createQuery
  bool prepareFirstEntry(mongo::BSONObj& lastEntry);

  // Run all registered callbacks
  void runCallBacks(const mongo::BSONObj& bSONObj);

  // initializes opLogDataMap with the default values
  static void createOpLogDataMap(OpLogDataMap& opLogDataMap);

  // returns the corresponding opLogType for a given operationType
  bool getOpLogType(const std::string& operationType,
                    OpLogType& opLogType);

  bool _isRunning; // thread state
  boost::thread* _pThread;
  OpLogCbVector _opLogCbVectors[OpLogTypeNumber];// Vector of std::vector<OpLogCallBack>

  int _querySleepTime; // the time to sleep in seconds before every new query

  // The start time as number of seconds since 00:00:00, 1 Jan 1970, GMT
  //  since the client will be notified for new entries
  unsigned long _startFromTimestamp;

  mongo::BSONObj _customQuery;// Custom query given by user



  // An internally used map that contain a mapping between operation name and opLogType
  OpLogDataMap _opLogDataMap;

  mongo::BSONObj _lastEntry;

private:
  std::string _ns;
  static const unsigned int EXCEPTION_RECOVER_TIME_SEC = 1;
};


#endif	/* MONGOOPLOG_H */


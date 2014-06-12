
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

#include "sipdb/MongoOpLog.h"
#include <mongo/client/connpool.h>

#include <os/OsLogger.h>

const string MongoOpLog::NS("local.oplog.rs");

const int MongoOpLog::MULTIPLIER = 1000;

const char* MongoOpLog::ts_fld(){static std::string name = "ts"; return name.c_str();}
const char* MongoOpLog::op_fld(){static std::string name = "op"; return name.c_str();}
void MongoOpLog::createOpLogDataMap(OpLogDataMap& opLogDataMap)
{
  opLogDataMap.insert(std::pair<std::string, OpLogType>("u", Update));
  opLogDataMap.insert(std::pair<std::string, OpLogType>("i", Insert));
  opLogDataMap.insert(std::pair<std::string, OpLogType>("d", Delete));
}

MongoOpLog::MongoOpLog(const MongoDB::ConnectionInfo& info,
                       const mongo::BSONObj& customQuery,
                       const int querySleepTime,
                       const unsigned long startFromTimestamp) :
	MongoDB::BaseDB(info),
  _isRunning(false),
  _pThread(0),
  _querySleepTime(querySleepTime),
  _startFromTimestamp(startFromTimestamp),
  _customQuery(customQuery),
  _ns(NS)
{
  OS_LOG_INFO(FAC_SIP, "MongoOpLog::MongoOpLog:" <<
                        " entering" <<
                        " querySleepTime=" << querySleepTime <<
                        " startFromTimestamp=" << startFromTimestamp);

  _customQuery.getOwned();
  createOpLogDataMap(_opLogDataMap);
}

MongoOpLog::~MongoOpLog()
{
  stop();
}

void MongoOpLog::registerCallback(OpLogType type, OpLogCallBack cb)
{
  if (type < Insert ||
      type > OpLogTypeNumber)
  {
    OS_LOG_ERROR(FAC_SIP, "MongoOpLog::registerCallback:" <<
                          " invalid callback type");

    return;
  }

  OS_LOG_INFO(FAC_SIP, "MongoOpLog::registerCallback:" <<
                        " entering" <<
                        " type=" << type);

  _opLogCbVectors[type].push_back(cb);
}

bool MongoOpLog::run()
{
  OS_LOG_INFO(FAC_SIP, "MongoOpLog::run:" <<
                        " starting MongoOpLog thread");

  bool rc = prepareFirstEntry(_lastEntry);
  if (false == rc)
  {
    OS_LOG_ERROR(FAC_SIP, "MongoOpLog::run" <<
                 " exited with error");
    return rc;
  }

  _isRunning = true;
  _pThread = new boost::thread(boost::bind(&MongoOpLog::internal_run_esafe, this));

  return true;
}

void MongoOpLog::stop()
{
  if (false == _isRunning)
  {
    return;
  }

  OS_LOG_INFO(FAC_SIP, "MongoOpLog::stop:" <<
                       " stopping MongoOpLog thread");

  _isRunning = false;

  if (_pThread)
  {
    _pThread->join();

    delete _pThread;
    _pThread = 0;
  }

  OS_LOG_INFO(FAC_SIP, "MongoOpLog::stop:" <<
                       " exiting");
}

// The monitor thread stays in this function as long as the cursor is not dead
// If the cursor already processed lastEntryObj, it gets blocked in a while in
// cursor->more() for 1 or 2 seconds until new entries are added to this collection.
// After that the entries are processed and the thread gets blocked again in
// cursor->more function
bool MongoOpLog::processQuery(mongo::DBClientCursor* cursor,
                              mongo::BSONObj& lastEntry)
{
  OS_LOG_INFO(FAC_SIP, "MongoOpLog::processQuery:" <<
                        " entering");

  if (!cursor)
  {
    OS_LOG_ERROR(FAC_SIP, "MongoOpLog:processQuery - Cursor is NULL");
    return false;
  }

  while (_isRunning)
  {
    if(!cursor->more())
    {
      if (_querySleepTime)
      {
        sleep(_querySleepTime);
      }

      if(cursor->isDead())
      {
        break;
      }

      continue; // we will try more() again
    }

    // WARNING: BSONObj must stay in scope for the life of the BSONElement
    lastEntry = cursor->next().getOwned();
    runCallBacks(lastEntry);
  }

  return true;
}

void MongoOpLog::createQuery(const mongo::BSONObj& lastEntry, mongo::BSONObj& query)
{
  OS_LOG_INFO(FAC_SIP, "MongoOpLog::createQuery:" <<
                        " entering");

  mongo::BSONObjBuilder queryBSONObjBuilder;

  queryBSONObjBuilder.appendElements(BSON(ts_fld() << mongo::GT << lastEntry[ts_fld()]));
  if (!_customQuery.isEmpty())
  {
    queryBSONObjBuilder.appendElements(_customQuery);
  }
  query = queryBSONObjBuilder.obj();
}

bool MongoOpLog::prepareFirstEntry(mongo::BSONObj& lastEntry)
{
  OS_LOG_INFO(FAC_SIP, "MongoOpLog::prepareFirstEntry:" <<
                        " entering");

  if (_startFromTimestamp == 0)
  {
    lastEntry = mongo::minKey;
  }
  else
  {
    mongo::BSONObjBuilder builder;
    unsigned long long timeStamp = (unsigned long long)_startFromTimestamp << 32;
    builder.appendTimestamp(ts_fld(), timeStamp);

    lastEntry = builder.obj();

    // Check that the created BSONElement has the correct timeStamp
    unsigned long long lastEntryTimeStamp = lastEntry[ts_fld()].timestampTime();
    if (_startFromTimestamp * MULTIPLIER != lastEntryTimeStamp)
    {
      OS_LOG_ERROR(FAC_SIP, "MongoOpLog::prepareFirstEntry" <<
                            " time stamps are different " <<
                            _startFromTimestamp * MULTIPLIER << lastEntryTimeStamp);
      return false;
    }
  }

  return true;
}

void MongoOpLog::internal_run_esafe()
{
  OS_LOG_INFO(FAC_SIP, "MongoOpLog::internal_run_esafe:"
              << " entering");

  while (_isRunning)
  {
    try
    {
      internal_run();
    }
    #ifdef MONGO_assert
    catch (mongo::DBException& e)
    {
      OS_LOG_ERROR( FAC_SIP, "MongoOpLog::internal_run_esafe Mongo DB Exception: "
          << e.what());
    }
    #endif //MONGO_assert
    catch (boost::exception& e)
    {
      OS_LOG_ERROR( FAC_SIP, "MongoOpLog::internal_run_esafe Boost Library Exception: "
          << boost::diagnostic_information(e));
    }
    catch (std::exception& e)
    {
      OS_LOG_ERROR( FAC_SIP, "MongoOpLog::internal_run_esafe Standard Library Exception: "
          << e.what());
    }
    catch (...)
    {
      OS_LOG_ERROR( FAC_SIP, "MongoOpLog::internal_run_esafe Exception: Unknown exception");
    }

    if (_isRunning)
    {
      // reset timestamp to check for entries right after the exception was triggered
      _startFromTimestamp = OsDateTime::getSecsSinceEpoch();
      // sleep for some time to give mongo time to recover
      sleep(EXCEPTION_RECOVER_TIME_SEC);
    }
  }

  OS_LOG_INFO(FAC_SIP, "MongoOpLog::internal_run_esafe:"
              << " exiting");
}

void MongoOpLog::internal_run()
{
  OS_LOG_INFO(FAC_SIP, "MongoOpLog::internal_run:"
              << " entering");

  mongo::BSONObj query;
  // WARNING: BSONObj must stay in scope for the life of the BSONElement
  createQuery(_lastEntry, query);

  MongoDB::ScopedDbConnectionPtr pConn(mongoMod::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));

  while (_isRunning)
  {
    // If we are at the end of the data, block for a while rather
    // than returning no data. After a timeout period, we do return as normal
    std::auto_ptr<mongo::DBClientCursor> cursor = pConn->get()->query(_ns, query, 0, 0, 0,
                 mongo::QueryOption_CursorTailable | mongo::QueryOption_AwaitData );

    bool rc = processQuery(cursor.get(), _lastEntry);
    if (false == rc)
    {
      break;
    }

    createQuery(_lastEntry, query);
  }

  pConn->done();

  OS_LOG_INFO(FAC_SIP, "MongoOpLog::internal_run:"
              << " exiting");
}

bool MongoOpLog::getOpLogType(const std::string& operationType,
                              OpLogType& opLogType)
{
  OpLogDataMap::iterator it;

  it = _opLogDataMap.find(operationType);
  if (_opLogDataMap.end() != it)
  {
    opLogType = it->second;

    return true;
  }

  return false;
}

void MongoOpLog::runCallBacks(const mongo::BSONObj& bSONObj)
{
  std::string type = bSONObj.getStringField(op_fld());
  std::string opLog = bSONObj.toString();

  OS_LOG_DEBUG(FAC_SIP, "MongoOpLog::notifyCallBacks:" <<
                " type=" << type <<
                " opLog=" << opLog);

  if (type.empty())
  {
    OS_LOG_WARNING(FAC_SIP, "MongoOpLog::notifyCallBacks:"
                << " type is empty");
    return;
  }

  // Notify all subscribers
  for(OpLogCbVector::iterator iter = _opLogCbVectors[All].begin(); iter != _opLogCbVectors[All].end(); iter++)
  {
    (*iter)(bSONObj);
  }

  OpLogType opLogType;
  bool found = false;
  found = getOpLogType(type, opLogType);
  if (false == found)
  {
    return;
  }
  
  // Notify specific subscribers
  for(OpLogCbVector::iterator iter = _opLogCbVectors[opLogType].begin();
      iter != _opLogCbVectors[opLogType].end(); iter++)
  {
    (*iter)(bSONObj);
  }
}


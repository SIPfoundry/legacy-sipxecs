
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

const char* MongoOpLog::ts_fld(){static std::string name = "ts"; return name.c_str();}
const char* MongoOpLog::op_fld(){static std::string name = "op"; return name.c_str();}
const MongoOpLog::OpLogDataVec& MongoOpLog::opLogDataVec()
{
  static OpLogDataVec opLogDataVec = {{Insert, "i"},
                                      {Delete, "d"},
                                      {Update, "u"}};

  return opLogDataVec;
}

MongoOpLog::MongoOpLog(const MongoDB::ConnectionInfo& info,
                       const mongo::BSONObj& customQuery,
                       const int querySleepTime,
                       const unsigned long timeStamp) :
	MongoDB::BaseDB(info),
  _isRunning(false),
  _pThread(0),
  _querySleepTime(querySleepTime),
  _timeStamp(timeStamp),
  _customQuery(customQuery)
{
  _customQuery.getOwned();
}

MongoOpLog::~MongoOpLog()
{
  stop();
}

void MongoOpLog::registerCallback(OpLogType type, OpLogCallBack cb)
{
  if (type < FirstOpLog ||
      type > LastOpLog)
  {
    return;
  }

  _opLogCbVectors[type].push_back(cb);
}

void MongoOpLog::run()
{
  _isRunning = true;
  _pThread = new boost::thread(boost::bind(&MongoOpLog::internal_run, this));
}

void MongoOpLog::stop()
{
  _isRunning = false;

  if (_pThread)
  {
    _pThread->join();

    delete _pThread;
    _pThread = 0;
  }
}

bool MongoOpLog::processQuery(mongo::DBClientCursor* cursor,
                              mongo::BSONElement& lastTailId)
{
  if (!cursor)
  {
    OS_LOG_ERROR(FAC_SIP, "MongoOpLog:processQuery - Cursor is NULL");
    return false;
  }

  while(_isRunning)
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

    const mongo::BSONObj& o = cursor->next();
    lastTailId = o[ts_fld()];
    notifyCallBacks(o);
  }

  return true;
}

void MongoOpLog::createQuery(mongo::BSONObj& query,
                             const mongo::BSONElement& lastTailId)
{
  mongo::BSONObjBuilder queryBSONObjBuilder;

  queryBSONObjBuilder.appendElements(BSON(ts_fld() << mongo::GT << lastTailId));
  if (!_customQuery.isEmpty())
  {
    queryBSONObjBuilder.appendElements(_customQuery);
  }
  query = queryBSONObjBuilder.obj();
}

bool MongoOpLog::createFirstQuery(mongo::BSONObj& query,
                                  mongo::BSONElement& lastTailId)
{
  if (_timeStamp == 0)
  {
    lastTailId = mongo::minKey.firstElement();

    createQuery(query, lastTailId);
  }
  else
  {
    mongo::BSONObjBuilder builder;
    unsigned long long timeStamp = (unsigned long long)_timeStamp << 32;
    builder.appendTimestamp(ts_fld(), timeStamp);

    lastTailId = builder.obj()[ts_fld()];

    // Check that the created BSONElement has the correct timeStamp
    unsigned long long lastTailIdTimeStamp = lastTailId.timestampTime();
    if (_timeStamp * 1000 != lastTailIdTimeStamp)
    {
      OS_LOG_ERROR(FAC_SIP, "MongoOpLog time stamps are different " <<
          _timeStamp * 1000 << lastTailIdTimeStamp);
      return false;
    }

    // WARNING: BSONObj must stay in scope for the life of the BSONElement
    createQuery(query, lastTailId);
  }

  return true;
}

void MongoOpLog::internal_run()
{
  mongo::BSONElement lastTailId;
  mongo::BSONObj query;

  bool rc = createFirstQuery(query, lastTailId);
  if (false == rc)
  {
    return;
  }

  MongoDB::ScopedDbConnectionPtr pConn(mongo::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));

  while (_isRunning)
  {
    // If we are at the end of the data, block for a while rather
    // than returning no data. After a timeout period, we do return as normal
    std::auto_ptr<mongo::DBClientCursor> cursor = pConn->get()->query(_info.getNS(), query, 0, 0, 0,
                 mongo::QueryOption_CursorTailable | mongo::QueryOption_AwaitData );

    bool rc = processQuery(cursor.get(), lastTailId);
    if (false == rc)
    {
      return;
    }

    createQuery(query, lastTailId);
  }

  pConn->done();
}

void MongoOpLog::notifyCallBacks(const mongo::BSONObj& bSONObj)
{
  std::string type = bSONObj.getStringField(op_fld());
  std::string opLog = bSONObj.toString();

  if (type.empty() || opLog.empty())
  {
    return;
  }

  // Notify all subscribers
  for(OpLogCbVector::iterator iter = _opLogCbVectors[All].begin(); iter != _opLogCbVectors[All].end(); iter++)
  {
    (*iter)(bSONObj, opLog);
  }

  OpLogType opLogType;
  bool found = false;

  for (unsigned int i = 0; i <  sizeof(OpLogDataVec)/sizeof(OpLogData); i++)
  {
    if (type == opLogDataVec()[i].opName) // insert
    {
      opLogType = opLogDataVec()[i].opLogType;
      found = true;
      break;
    }
  }

  if (false == found)
  {
    return;
  }
  
  // Notify specific subscribers
  for(OpLogCbVector::iterator iter = _opLogCbVectors[opLogType].begin();
      iter != _opLogCbVectors[opLogType].end(); iter++)
  {
    (*iter)(bSONObj, opLog);
  }
}


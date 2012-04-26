
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

MongoOpLog::MongoOpLog(const MongoDB::ConnectionInfo& info) :
	MongoDB::BaseDB(info),
  _isRunning(false),
  _pTailThread(0),
  _pTailConnection(0)
{
}

MongoOpLog::~MongoOpLog()
{
  stop();
}

void MongoOpLog::registerCallback(OpLog type, OpLogCallBack cb)
{
  switch(type)
  {
    case Insert:
      _insertCb.push_back(cb);
      break;
    case Delete:
      _deleteCb.push_back(cb);
      break;
    case Update:
      _updateCb.push_back(cb);
      break;
    case All:
      _allCb.push_back(cb);
      break;
  }
}

void MongoOpLog::run()
{
  if (!_pTailThread)
  {
    _pTailThread = new boost::thread(boost::bind(&MongoOpLog::internal_run, this));
  }
  else if (!_pTailConnection)
  {
    //
    // There is a thread but no connecton
    //
    stop();
    _pTailThread = new boost::thread(boost::bind(&MongoOpLog::internal_run, this));
  }
}

void MongoOpLog::stop()
{
  _isRunning = false;
  if (_pTailThread)
  {
    if (_pTailConnection)
    {
      (*_pTailConnection).done();
      (*_pTailConnection).kill();
    }
    _pTailThread->join();
  }
  
  delete _pTailThread;
  delete _pTailConnection;
  _pTailThread = 0;
  _pTailConnection = 0;
}

void MongoOpLog::internal_run()
{
  mongo::BSONElement lastTailId = mongo::minKey.firstElement();

  mongo::Query query = QUERY( "_id" << mongo::GT << lastTailId
          << "ns" << _info.getNS()).sort("$natural");

  _pTailConnection = new mongo::ScopedDbConnection(_info.getConnectionString());
  mongo::ScopedDbConnection& conn = *_pTailConnection;

  std::auto_ptr<mongo::DBClientCursor> c =
    conn->query("local.oplog", query, 0, 0, 0,
               mongo::QueryOption_CursorTailable | mongo::QueryOption_AwaitData );
  while(true)
  {
    if(c->isDead())
    {
      delete _pTailConnection;
      _pTailConnection = 0;
      return;
    }

    if(!c->more())
      break;

    mongo::BSONObj o = c->next();
    lastTailId = o["_id"];
  }

  _isRunning = true;

  while (_isRunning)
  {
    std::auto_ptr<mongo::DBClientCursor> c =
      conn->query("local.oplog", query, 0, 0, 0,
                 mongo::QueryOption_CursorTailable | mongo::QueryOption_AwaitData );
    while(true)
    {
      if(c->isDead())
      {
        delete _pTailConnection;
        _pTailConnection = 0;
        _isRunning = false;
        return;
      }

      if(!c->more())
        break;

      mongo::BSONObj o = c->next();
      lastTailId = o["_id"];
      notifyCallBacks(o.getStringField("op"), o.toString());
    }
  }

}

void MongoOpLog::notifyCallBacks(const std::string& type, const std::string& opLog)
{
  if (type.empty() || opLog.empty())
    return;
  std::vector<OpLogCallBack>* _pCb = 0;
  if (type == "i") // insert
     _pCb = &_insertCb;
  else if (type == "u") // update
    _pCb = &_updateCb;
  else if (type == "d") // delete
    _pCb = &_deleteCb;
  
  if (_pCb)
  {
    //
    // Notify specific subscribers
    //
    for(std::vector<OpLogCallBack>::iterator iter = (*_pCb).begin(); iter != (*_pCb).end(); iter++)
      (*iter)(opLog);
    //
    // Notify all subscribers
    //
    for(std::vector<OpLogCallBack>::iterator iter = _allCb.begin(); iter != _allCb.end(); iter++)
      (*iter)(opLog);
  }
}




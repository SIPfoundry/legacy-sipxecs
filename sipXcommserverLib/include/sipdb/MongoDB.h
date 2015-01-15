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

#ifndef MONGODB_H
#define	MONGODB_H

#include <queue>
#include <vector>
#include <os/OsTime.h>
#include <boost/circular_buffer.hpp>
#include <boost/thread.hpp>

// Avoids this error
//   /usr/include/mongo/client/../pch.h:116:15: error: expected unqualified-id before string constant
#undef VERSION

// Avoids this error
//.../usr/include/mongo/util/net/sock.h:62:15: error: expected unqualified-id before '-' token
#ifdef INVALID_SOCKET
#undef INVALID_SOCKET
#endif

#include <mongo/client/dbclient.h>


// unfortunately mongo undefines assert and for some c++ files, that's bad
// so we universally include it here whether c++ uses it or not.
#include <assert.h>

#include <boost/exception/all.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_array.hpp>
#include <exception>
#include <boost/function.hpp>
#include "sipdb/CustomConnPool.h"


// is assert is undefined, just include it again
//  include <assert.h>

// cannot seem to redfine it so safer to undefine it
#undef VERSION

#define BSON_NOT_EQUAL(val) BSON("$ne"<< val)
#define BSON_LESS_THAN(val) BSON("$lt"<< val)
#define BSON_LESS_THAN_EQUAL(val) BSON("$lte"<< val)
#define BSON_GREATER_THAN(val) BSON("$gt" << val)
#define BSON_GREATER_THAN_EQUAL(val) BSON("$gte" << val)
#define BSON_ELEM_MATCH(val) BSON("$elemMatch" << val)
#define BSON_OR(val) BSON("$or" << val)

typedef boost::error_info<struct tag_errmsg, std::string> errmsg_info;

namespace MongoDB
{
   //typedef boost::scoped_ptr<mongo::ScopedDbConnection> ScopedDbConnectionPtr;
   typedef boost::scoped_ptr<mongoMod::ScopedDbConnection> ScopedDbConnectionPtr;

class ConfigError: public boost::exception, public std::exception {
public:
    ConfigError() {
    }
};

class ConnectionInfo
{
public:
  ConnectionInfo();
  
	ConnectionInfo(const ConnectionInfo& rhs);
  
  ConnectionInfo(const mongo::ConnectionString& connectionString);

	ConnectionInfo(std::ifstream& configFile);

	~ConnectionInfo()
	{
	}
	
  ConnectionInfo& operator=(const ConnectionInfo& conn);
  


	/**
	 * Read just the connection string from a file.
	 *
	 * Example:
	 *    MyDB db(ConnectionInfo(ConnectionInfo::connectionStringFromFile(), "mydb.mycollection"));
	 *
	 * Example:
	 *    ConnectionString connString = ConnectionInfo::connectionStringFromFile();
	 *    MyDB db1(ConnectionInfo(connString, "mydb.mycollection1"));
	 *    MyDB db2(ConnectionInfo(connString, "mydb.mycollection2"));
	 *
	 * Example file contents:
	 * ======================
	 * sipxecs/localhost:27017,localhost:27018
	 * ======================
	 */
	static ConnectionInfo globalInfo();
	static ConnectionInfo localInfo();

	static bool	testConnection(const mongo::ConnectionString &connectionString, std::string& errmsg);

    const mongo::ConnectionString& getConnectionString() const
	{
              return _connectionString;
	}
	;

	const int getShardId() const
	{
		return _shard;
	}
	;

	const bool useReadTags() const
	{
		return _useReadTags;
	}
	;

	void enableReadTags(bool enable)
	{
		_useReadTags = enable;
	};

	const bool isEmpty() const
	{
		return !_connectionString.isValid();
	}
	;
  
  const std::string& getClusterId() const
  {
    return _clusterId;
  }

  const unsigned int getReadQueryTimeoutMs() const
  {
    return _readQueryTimeoutMs;
  }

  const unsigned int getWriteQueryTimeoutMs() const
  {
    return _writeQueryTimeoutMs;
  }

  const double getReadQueryTimeout() const
  {
    double readQueryTimeout = _readQueryTimeoutMs;
    return readQueryTimeout/1000;
  }

  const double getWriteQueryTimeout() const
  {
    double writeQueryTimeout = _writeQueryTimeoutMs;
    return writeQueryTimeout/1000;
  }


private:

  mongo::ConnectionString _connectionString;
  int _shard;
  bool _useReadTags; 
  std::string _clusterId;
  unsigned int _readQueryTimeoutMs;
  unsigned int _writeQueryTimeoutMs;
  std::string _rawConnectionString;
};

class UpdateTimer;
class ReadTimer;

class BaseDB
{
public:
	BaseDB(const ConnectionInfo& info, const std::string& ns);

	virtual ~BaseDB()
	{
	}
	;

	// This does something for each record. Efficient because it doesn't store each record into a collection
	// then pass back the collection for you to iterate over.
	//
	// NOTE: You can easily load _all_ objects which would prohibit your function from working on a large
	// production system depending on the circumstance.
	//
	// Example:
	//   include <boost/bind.hpp>
	//
	//   class X {
	//      void y(mongo::BSONObj o) {
	//         println("%s\n", o.getStringField("a"));
	//      }
	//   };
	//
	//   BaseDB d(info);
	//   X z;
	//   mongo::BSONObj all;
	//   d.forEach(all, bind(&X::y, &z, _1));   // _1 is required means a single argument
	//
	void forEach(mongo::BSONObj& query, const std::string& ns, boost::function<void(mongo::BSONObj)> doSomething);

	void  nearest(mongo::BSONObjBuilder& builder, mongo::BSONObj query) const;
	
	void  primaryPreferred(mongo::BSONObjBuilder& builder, mongo::BSONObj query) const;
	
	void  setReadPreference(mongo::BSONObjBuilder& builder, mongo::BSONObj query, const char* readPreferrence) const;

	const int getShardId() const { return _info.getShardId(); };

	const bool useReadTags() const { return _info.useReadTags(); };
  
  const std::string& getClusterId() const { return _info.getClusterId(); }

  void registerTimer(const UpdateTimer* pTimer);
  
  void registerTimer(const ReadTimer* pTimer);
  
  Int64 getUpdateAverageSpeed() const;
  
  Int64 getLastUpdateSpeed() const;
  
  Int64 getReadAverageSpeed() const;
  
  Int64 getLastReadSpeed() const;
  
  const double getReadQueryTimeout() const { double readQueryTimeout = _info.getReadQueryTimeoutMs(); return readQueryTimeout/1000; }

  const double getWriteQueryTimeout() const { double writeQueryTimeout = _info.getWriteQueryTimeoutMs(); return writeQueryTimeout/1000; }

protected:
  std::string _ns;
	mutable ConnectionInfo _info;
  boost::circular_buffer<Int64> _updateTimerSamples;
  boost::circular_buffer<Int64> _readTimerSamples;
  mutable boost::mutex _updateTimerSamplesMutex; 
  mutable boost::mutex _readTimerSamplesMutex; 
  Int64 _lastReadSpeed;
  Int64 _lastUpdateSpeed;
  long _lastAlarmLog;
};

class UpdateTimer
{
public:
  UpdateTimer(BaseDB& db);
  ~UpdateTimer();
  
protected:
  Int64 _start;
  Int64 _end;
  BaseDB& _db;
  friend class BaseDB;
};

class ReadTimer
{
public:
  ReadTimer(BaseDB& db);
  ~ReadTimer();
  
protected:
  Int64 _start;
  Int64 _end;
  BaseDB& _db;
  friend class BaseDB;  
};


} // namespace MongoDB

#endif	/* MONGODB_H */



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

// Avoids this error
//   /usr/include/mongo/client/../pch.h:116:15: error: expected unqualified-id before string constant
#undef VERSION

#include <mongo/client/dbclient.h>

// unfortunately mongo undefines assert and for some c++ files, that's bad
// so we universally include it here whether c++ uses it or not.
#include <assert.h>

#include <boost/exception/all.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_array.hpp>
#include <exception>
#include <boost/function.hpp>


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
   typedef boost::scoped_ptr<mongo::ScopedDbConnection> ScopedDbConnectionPtr;

class ConfigError: public boost::exception, public std::exception {
public:
    ConfigError() {
    }
};

class ConnectionInfo
{
public:
	ConnectionInfo(const ConnectionInfo& rhs) :
		_connectionString(rhs._connectionString), _ns(rhs._ns)
	{
	}
	;

	ConnectionInfo(const mongo::ConnectionString& connectionString, const std::string& ns) :
		_connectionString(connectionString), _ns(ns)
	{
	}
	;

	virtual ~ConnectionInfo()
	{
	}
	;

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
	static const mongo::ConnectionString connectionStringFromFile(
			const std::string& configFile = "");

	static const mongo::ConnectionString connectionString(const std::string& str);

	static bool	testConnection(const mongo::ConnectionString &connectionString, const std::string& errmsg);

        const mongo::ConnectionString& getConnectionString() const
	{
              return _connectionString;
	}
	;

	const std::string& getNS() const
	{
		return _ns;
	}
	;

private:
	mongo::ConnectionString _connectionString;
	std::string _ns;
};

class BaseDB
{
public:
	BaseDB(const ConnectionInfo& info) :
		_info(info)
	{
	}
	;

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
	void forEach(mongo::BSONObj& query, boost::function<void(mongo::BSONObj)> doSomething);

protected:
	mutable ConnectionInfo _info;
};

}
;

#endif	/* MONGODB_H */


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

#include "AppPerform.h"

#include "AppConfig.h"

#include "sipdb/EntityRecord.h"
#include "sipdb/EntityDB.h"

#include "os/OsDateTime.h"

#include <string>

#include <mongo/util/net/hostandport.h>
#include <mongo/client/connpool.h>

#include <boost/format.hpp>

#include <boost/lexical_cast.hpp>


using namespace MongoDBTool;


AppPerform::AppPerform() : _mongoConnectionString(mongo::HostAndPort("localhost"))
{
}

AppPerform::~AppPerform()
{
}

void AppPerform::deleteDbEntries(std::vector<std::string>& whereOptVector,
                                 const std::string& databaseName)
{
  MongoDB::ConnectionInfo connectionInfo(_mongoConnectionString);

  DbHelper::deleteDbEntries(&connectionInfo, databaseName, whereOptVector);
}

void AppPerform::printDbEntries(std::vector<std::string>& whereOptVector,
                                const std::string& databaseName,
                                bool multipleLines)
{
  DbHelper::DbType dbType;
  if (databaseName == pNodeRegistrarDbName)
    dbType = DbTypeRegBinding;
  else if (databaseName == pImdbEntityDbName)
    dbType = DbTypeEntityRecord;
  else
  {
    BOOST_THROW_EXCEPTION(DbHelperException() <<
          DbHelperTagInfo(std::string("Unknown database name. Please select one of the following databases: imdb.entity, node.registrar")));
  }

  MongoDB::ConnectionInfo connectionInfo(_mongoConnectionString);

  DbHelper::printDbEntries(std::cout, &connectionInfo, databaseName, whereOptVector, dbType, multipleLines);
}

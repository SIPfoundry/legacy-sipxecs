/*
 * Copyright (c) 2015 eZuce, Inc. All rights reserved.
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

#include <mongo/client/dbclient.h>

#include "sipdb/MongoMod.h"

//
// Rewrites the mongo::minKey external variable defined in Mongo c++ driver
// implementation - mongo/db/jsonobj.cpp file
//
static struct MinKeyData
{
  MinKeyData()
  {
    totsize=7;
    minkey=mongo::MinKey;
    name=0;
    eoo=mongo::EOO;
  }
  int totsize;
  char minkey;
  char name;
  char eoo;
} minkeydata;
mongo::BSONObj mongoMod::minKey((const char *) &minkeydata);

mongo::ScopedDbConnection* mongoMod::ScopedDbConnection::getScopedDbConnection(const std::string& host, double socketTimeout)
{
  return new mongo::ScopedDbConnection(host, socketTimeout);
}

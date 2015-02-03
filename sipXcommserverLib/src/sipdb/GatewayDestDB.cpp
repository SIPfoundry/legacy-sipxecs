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

#include <fstream>
#include <mongo/client/connpool.h>
#include <os/OsDateTime.h>
#include <os/OsLogger.h>
#include "sipdb/GatewayDestDB.h"
#include "sipdb/RegExpireThread.h"

using namespace std;

const string GatewayDestDB::NS("node.gatewaydest");

void GatewayDestDB::updateRecord(const GatewayDestRecord& record, bool upsert)
{
  if (!upsert)
  {
    OS_LOG_DEBUG(FAC_ODBC, "GatewayDestDB::updateRecord - adding record for "
        " callid " << record.getCallId() <<
        " toTag " << record.getToTag() <<
        " fromTag " << record.getFromTag());
  }
  else
  {
    OS_LOG_DEBUG(FAC_ODBC, "GatewayDestDB::updateRecord - updating record for "
        " callid " << record.getCallId() <<
        " toTag " << record.getToTag() <<
        " fromTag " << record.getFromTag() <<
        " identity " << record.getIdentity() <<
        " lineId " << record.getLineId() <<
        " expirationTime " << record.getExpirationTime());
  }

	mongo::BSONObj query = BSON(
			GatewayDestRecord::callIdField() << record.getCallId() <<
			GatewayDestRecord::toTagField() << record.getToTag() <<
			GatewayDestRecord::fromTagField() << record.getFromTag());

	mongo::BSONObj update = BSON("$set" << BSON(
          GatewayDestRecord::callIdField() << record.getCallId() <<
          GatewayDestRecord::toTagField() << record.getToTag() <<
          GatewayDestRecord::fromTagField() << record.getFromTag() <<
          GatewayDestRecord::identityField() << record.getIdentity() <<
          GatewayDestRecord::lineIdField() << record.getLineId() <<
          GatewayDestRecord::expirationTimeField() << record.getExpirationTime()));

  MongoDB::ScopedDbConnectionPtr conn(mongoMod::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString(), getWriteQueryTimeout()));
  mongo::DBClientBase* client = conn->get();

  client->update(_ns, query, update, upsert, false);
	client->ensureIndex("node.gatewaydest", BSON( GatewayDestRecord::callIdField() << 1 ));
	client->ensureIndex("node.gatewaydest", BSON( GatewayDestRecord::expirationTimeField() << 1 ));

	conn->done();

	removeAllExpired();
}

void GatewayDestDB::removeRecord(const GatewayDestRecord& record)
{
  OS_LOG_DEBUG(FAC_ODBC, "GatewayDestDB::removeRecord - "
      " callid " << record.getCallId() <<
      " toTag " << record.getToTag() <<
      " fromTag " << record.getFromTag());

  mongo::BSONObj query = BSON(
      GatewayDestRecord::callIdField() << record.getCallId() <<
      GatewayDestRecord::toTagField() << record.getToTag() <<
      GatewayDestRecord::fromTagField() << record.getFromTag());

  MongoDB::ScopedDbConnectionPtr conn(mongoMod::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString(), getWriteQueryTimeout()));
  mongo::DBClientBase* client = conn->get();

  client->remove(_ns, query);
  client->ensureIndex("node.gatewaydest",  BSON( GatewayDestRecord::callIdField() << 1 ));
  client->ensureIndex("node.gatewaydest", BSON( GatewayDestRecord::expirationTimeField() << 1 ));

  conn->done();
}

void GatewayDestDB::removeAllRecords()
{
  OS_LOG_DEBUG(FAC_ODBC, "GatewayDestDB::removeAllRecords ");

  mongo::BSONObj all;
  MongoDB::ScopedDbConnectionPtr conn(mongoMod::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString(), getWriteQueryTimeout()));
  conn->get()->remove(_ns, all);
  conn->done();
}

void GatewayDestDB::removeAllExpired()
{
  int timeNow = (int) OsDateTime::getSecsSinceEpoch();
	mongo::BSONObj query = BSON(GatewayDestRecord::expirationTimeField() << BSON_LESS_THAN_EQUAL(timeNow));

  OS_LOG_DEBUG(FAC_ODBC, "GatewayDestDB::removeAllExpired - "
      " timeNow " << timeNow);

	MongoDB::ScopedDbConnectionPtr conn(mongoMod::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString(), getWriteQueryTimeout()));
	mongo::DBClientBase* client = conn->get();

  client->remove(_ns, query);
  client->ensureIndex("node.gatewaydest",  BSON( GatewayDestRecord::callIdField() << 1 ));
	client->ensureIndex("node.gatewaydest", BSON( GatewayDestRecord::expirationTimeField() << 1 ));

	conn->done();
}

bool GatewayDestDB::getUnexpiredRecord(GatewayDestRecord& record) const
{
  int timeNow = OsDateTime::getSecsSinceEpoch();

  OS_LOG_DEBUG(FAC_ODBC, "GatewayDestDB::getUnexpiredRecord for "
      " callid " << record.getCallId() <<
      " toTag " << record.getToTag() <<
      " fromTag " << record.getFromTag() <<
      " timeNow" << timeNow);

  mongo::BSONObj query = BSON(
      GatewayDestRecord::callIdField() << record.getCallId().c_str() <<
      GatewayDestRecord::toTagField() << record.getToTag() <<
      GatewayDestRecord::fromTagField() << record.getFromTag() <<
      GatewayDestRecord::expirationTimeField() << BSON_GREATER_THAN(timeNow));

  MongoDB::ScopedDbConnectionPtr conn(mongoMod::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString(), getReadQueryTimeout()));

  mongo::BSONObjBuilder builder;
  BaseDB::nearest(builder, query);

  auto_ptr<mongo::DBClientCursor> pCursor = conn->get()->query(_ns, builder.obj(), 0, 0, 0, mongo::QueryOption_SlaveOk);
  if (!pCursor.get())
  {
   throw mongo::DBException("mongo query returned null cursor", 0);
  }
  else if (pCursor->more())
  {
    OS_LOG_DEBUG(FAC_ODBC, "GatewayDestDB::getUnexpiredRecord - found record "
        " callid " << record.getCallId() <<
        " toTag " << record.getToTag() <<
        " fromTag " << record.getFromTag() <<
        " identity " << record.getIdentity() <<
        " lineId " << record.getLineId() <<
        " expirationTime " << record.getExpirationTime());

    record = pCursor->next();
    conn->done();
    return true;
  }

  OS_LOG_DEBUG(FAC_ODBC, "GatewayDestDB::getUnexpiredRecord - NOT found record "
      " callid " << record.getCallId() <<
      " toTag " << record.getToTag() <<
      " fromTag " << record.getFromTag() <<
      " expirationTime " << timeNow);

  conn->done();
  return false;
}

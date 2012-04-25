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

#include <mongo/client/connpool.h>
#include "os/OsLogger.h"
#include "sipdb/EntityDB.h"

using namespace std;

const string EntityDB::NS("imdb.entity");

bool EntityDB::findByIdentity(const string& identity, EntityRecord& entity) const
{
	mongo::BSONObj query = BSON(EntityRecord::identity_fld() << identity);
	OS_LOG_INFO(FAC_ODBC, "EntityDB::findByIdentity - Finding entity record for " << identity << " from namespace " << _info.getNS());

	mongo::ScopedDbConnection conn(_info.getConnectionString());
	auto_ptr<mongo::DBClientCursor> pCursor = conn->query(_info.getNS(), query);
	if (pCursor.get() && pCursor->more())
	{
		OS_LOG_DEBUG(FAC_ODBC, identity << " is present in namespace " << _info.getNS());
		entity = pCursor->next();
                conn.done();
		return true;
	}
	OS_LOG_DEBUG(FAC_ODBC, identity << " is NOT present in namespace " << _info.getNS());
	OS_LOG_INFO(FAC_ODBC, "EntityDB::findByIdentity - Unable to find entity record for " << identity << " from namespace " << _info.getNS());
        conn.done();
	return false;
}

bool EntityDB::findByUserId(const string& userId, EntityRecord& entity) const
{
	mongo::BSONObj query = BSON(EntityRecord::userId_fld() << userId);
	mongo::ScopedDbConnection conn(_info.getConnectionString());
	auto_ptr<mongo::DBClientCursor> pCursor = conn->query(_info.getNS(), query);
	OS_LOG_INFO(FAC_ODBC, "EntityDB::findByUserId - Finding entity record for " << userId << " from namespace " << _info.getNS());
	if (pCursor.get() && pCursor->more())
	{
		entity = pCursor->next();
                conn.done();
		return true;
	}
	OS_LOG_INFO(FAC_ODBC, "EntityDB::findByUserId - Unable to find entity record for " << userId << " from namespace " << _info.getNS());
        conn.done();
	return false;
}

bool EntityDB::findByIdentityOrAlias(const Url& uri, EntityRecord& entity) const
{
	UtlString identity;
	UtlString userId;
	uri.getIdentity(identity);
	uri.getUserId(userId);
	return findByIdentityOrAlias(identity.str(), userId.str(), entity);
}

bool EntityDB::findByIdentityOrAlias(const string& identity, const string& alias,
		EntityRecord& entity) const
{
	bool found = false;
	if (!identity.empty())
		found = findByIdentity(identity, entity);

	if (!found && !alias.empty())
		found = findByAliasUserId(alias, entity);

	return found;
}

bool EntityDB::findByAliasUserId(const string& alias, EntityRecord& entity) const
{
	mongo::BSONObj query = BSON( EntityRecord::aliases_fld() <<
			BSON_ELEM_MATCH( BSON(EntityRecord::aliasesId_fld() << alias) ) );

	mongo::ScopedDbConnection conn(_info.getConnectionString());
	auto_ptr<mongo::DBClientCursor> pCursor = conn->query(_info.getNS(), query);
	OS_LOG_INFO(FAC_ODBC, "EntityDB::findByAliasUserId - Finding entity record for alias " << alias << " from namespace " << _info.getNS());
	if (pCursor.get() && pCursor->more())
	{
		entity = pCursor->next();
                conn.done();
		return true;
	}
	OS_LOG_INFO(FAC_ODBC, "EntityDB::findByAliasUserId - Unable to find entity record for alias " << alias << " from namespace " << _info.getNS());
        conn.done();
	return false;
}

/// Retrieve the SIP credential check values for a given identity and realm
bool EntityDB::getCredential(const Url& uri, const UtlString& realm, UtlString& userid, UtlString& passtoken,
		UtlString& authType) const
{
	UtlString identity;
	uri.getIdentity(identity);

	EntityRecord entity;
	if (!findByIdentity(identity.str(), entity))
		return false;

	if (entity.realm() != realm.str())
		return false;

	userid = entity.userId();
	passtoken = entity.password();
	authType = entity.authType();

	return true;
}

/// Retrieve the SIP credential check values for a given userid and realm
bool EntityDB::getCredential(const UtlString& userid, const UtlString& realm, Url& uri, UtlString& passtoken,
		UtlString& authType) const
{
	EntityRecord entity;
	if (!findByUserId(userid.str(), entity))
		return false;

	if (entity.realm() != realm.str())
		return false;

	uri = entity.identity().c_str();
	passtoken = entity.password();
	authType = entity.authType();

	return true;
}

void EntityDB::getAliasContacts(const Url& aliasIdentity, Aliases& aliases, bool& isUserIdentity) const
{
	UtlString alias;
	aliasIdentity.getUserId(alias);
	if (alias.isNull())
		return;

	EntityRecord entity;
	if (findByAliasUserId(alias.str(), entity))
	{
		Aliases result = entity.aliases();
		for (Aliases::iterator iter = result.begin(); iter != result.end(); iter++)
		{
			if (iter->id == alias.data())
				aliases.push_back(*iter);
		}
		isUserIdentity = !entity.realm().empty() && !entity.password().empty();
	}
}

bool EntityDB::findByIdentity(const Url& uri, EntityRecord& entity) const
{
	UtlString identity;
	uri.getIdentity(identity);
	return findByIdentity(identity.str(), entity);
}


bool  EntityDB::tail(std::vector<std::string>& opLogs) {
  // minKey is smaller than any other possible value

  static bool hasLastTailId = false;
  mongo::ScopedDbConnection conn(_info.getConnectionString());
  if (!hasLastTailId)
  {
    mongo::Query query = QUERY( "_id" << mongo::GT << _lastTailId
          << "ns" << NS).sort("$natural");

    std::auto_ptr<mongo::DBClientCursor> c =
      conn->query("local.oplog", query, 0, 0, 0,
                 mongo::QueryOption_CursorTailable | mongo::QueryOption_AwaitData );
    while(true)
    {
      if( !c->more() )
      {
        if( c->isDead() )
        {
          // we need to requery
          return false;
        }
        // No need to wait here, cursor will block for several sec with _AwaitData
        break;
      }
      mongo::BSONObj o = c->next();
      _lastTailId = o["_id"];
      hasLastTailId = true;
    }
  }

  mongo::Query query = QUERY( "_id" << mongo::GT << _lastTailId
          << "ns" << NS).sort("$natural");

  // capped collection insertion order

  std::auto_ptr<mongo::DBClientCursor> c =
    conn->query("local.oplog", query, 0, 0, 0,
               mongo::QueryOption_CursorTailable | mongo::QueryOption_AwaitData );
  while(true)
  {
    if( !c->more() )
    {
      if( c->isDead() )
      {
        // we need to requery
        return false;
      }
      // No need to wait here, cursor will block for several sec with _AwaitData
      return !opLogs.empty();
    }
    mongo::BSONObj o = c->next();
    _lastTailId = o["_id"];
    opLogs.push_back(o.toString());
  }
  return true;
}



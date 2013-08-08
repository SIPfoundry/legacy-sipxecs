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
#include <mongo/client/dbclient.h>
#include <mongo/client/connpool.h>
#include <os/OsDateTime.h>
#include <os/OsLogger.h>
#include "sipdb/RegDB.h"
#include "sipdb/RegExpireThread.h"

using namespace std;

const string RegDB::NS("node.registrar");

extern mongo::DBConnectionPool pool;

RegDB* RegDB::CreateInstance() {
   RegDB* lRegDb = NULL;

   MongoDB::ConnectionInfo local = MongoDB::ConnectionInfo::localInfo();
   if (!local.isEmpty()) {
     Os::Logger::instance().log(FAC_SIP, PRI_INFO, "Regional database defined");
     Os::Logger::instance().log(FAC_SIP, PRI_INFO, local.getConnectionString().toString().c_str());
     lRegDb = new RegDB(local);
   } else {
     Os::Logger::instance().log(FAC_SIP, PRI_INFO, "No regional database found");
   }

   MongoDB::ConnectionInfo global = MongoDB::ConnectionInfo::globalInfo();
   RegDB* regDb = new RegDB(global, lRegDb);

   return regDb;
}

void RegDB::updateBinding(const RegBinding::Ptr& pBinding)
{
	updateBinding(*(pBinding.get()));
}

void RegDB::updateBinding(RegBinding& binding)
{
	if (_local != NULL) {
		_local->updateBinding(binding);
		return;
	}
	if (binding.getTimestamp() == 0)
		binding.setTimestamp((int) OsDateTime::getSecsSinceEpoch());

	if (binding.getLocalAddress().empty())
	{
		string serverId = _localAddress;
		binding.setLocalAddress(serverId);
	}

	mongo::BSONObj query = BSON(
			"identity" << binding.getIdentity() <<
			"contact" << binding.getContact() <<
                        "shardId" << getShardId());

  bool isExpired = binding.getExpirationTime() <= 0;
	mongo::BSONObj update;
    update = BSON(
          "timestamp" << binding.getTimestamp() <<
          "localAddress" << binding.getLocalAddress() <<
          "identity" << binding.getIdentity() <<
          "uri" << binding.getUri() <<
          "callId" << binding.getCallId() <<
          "contact" << binding.getContact() <<
          "qvalue" << binding.getQvalue() <<
          "instanceId" << binding.getInstanceId() <<
          "gruu" << binding.getGruu() <<
          "shardId" << getShardId() <<
          "path" << binding.getPath() <<
          "cseq" << binding.getCseq() <<
          "expirationTime" << binding.getExpirationTime() <<
          "instrument" << binding.getInstrument() <<
          "expired" << isExpired );

    MongoDB::ScopedDbConnectionPtr connPtr(mongo::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));
    mongo::ScopedDbConnection& conn = *connPtr;

    conn->remove(_ns, query);
    conn->insert(_ns, update);
	conn->ensureIndex("node.registrar", BSON( "identity" << 1 ));
	conn->ensureIndex("node.registrar", BSON( "expirationTime" << 1 ));

        string e = conn->getLastError();
        if( !e.empty() ) {
          Os::Logger::instance().log(FAC_SIP, PRI_ERR, e.c_str());
        } else {
          Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "Save reg ok");
        }

	conn.done();
}

void RegDB::expireOldBindings(const string& identity, const string& callId, unsigned int cseq,
		unsigned int timeNow)
{
	if (_local != NULL) {
		_local->expireOldBindings(identity, callId, cseq, timeNow);
		return;
	}
	mongo::BSONObj query = BSON(
			"identity" << identity <<
			"callId"<< callId <<
			"cseq" << BSON_LESS_THAN(cseq) <<
                        "shardId" << getShardId());

    MongoDB::ScopedDbConnectionPtr conn(mongo::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));
    mongo::DBClientBase* client = conn->get();

	client->remove(_ns, query);
	client->ensureIndex("node.registrar",  BSON( "identity" << 1 ));
	client->ensureIndex("node.registrar", BSON( "expirationTime" << 1 ));

	conn->done();
}

void RegDB::expireAllBindings(const string& identity, const string& callId, unsigned int cseq,
		unsigned int timeNow)
{
	if (_local != NULL) {
		_local->expireAllBindings(identity, callId, cseq, timeNow);
		return;
	}
	mongo::BSONObj query = BSON(
                        "shardId" << getShardId() <<
			"identity" << identity);

    MongoDB::ScopedDbConnectionPtr conn(mongo::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));
    mongo::DBClientBase* client = conn->get();

    client->remove(_ns, query);
	client->ensureIndex("node.registrar",  BSON( "identity" << 1 ));
	client->ensureIndex("node.registrar", BSON( "expirationTime" << 1 ));

	conn->done();
}

void RegDB::removeAllExpired()
{
	if (_local != NULL) {
		_local->removeAllExpired();
		return;
	}
    int timeNow = (int) OsDateTime::getSecsSinceEpoch();
	mongo::BSONObj query = BSON(
            "shardId" << getShardId() <<
            "expirationTime" << BSON_LESS_THAN_EQUAL(timeNow));

	MongoDB::ScopedDbConnectionPtr conn(mongo::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));
    mongo::DBClientBase* client = conn->get();

    client->remove(_ns, query);
    client->ensureIndex("node.registrar",  BSON( "identity" << 1 ));
	client->ensureIndex("node.registrar", BSON( "expirationTime" << 1 ));

	conn->done();
}

bool RegDB::isOutOfSequence(const string& identity, const string& callId, unsigned int cseq) const
{
    // Remove this method altogether?!?!? -- Conversation between douglas and joegen on 6/18/13
	return false;
}

bool RegDB::getUnexpiredContactsUser(const string& identity, int timeNow, Bindings& bindings) const
{
	static string gruuPrefix = GRUU_PREFIX;

	bool isGruu = identity.substr(0, gruuPrefix.size()) == gruuPrefix;

	mongo::BSONObjBuilder query;
	if (_local) {
		_local->getUnexpiredContactsUser(identity, timeNow, bindings);
		query.append("$not", BSON("shardId" << getShardId()));
	} else {
		query.append("expirationTime", BSON_GREATER_THAN(timeNow));
	}

	if (isGruu) {
		string searchString(identity);
		searchString += ";";
		searchString += SIP_GRUU_URI_PARAM;
		query.append("gruu", searchString);
	}
	else {
		query.append("identity", identity);
	}

	mongo::BSONObjBuilder builder;
	BaseDB::nearest(builder, query.obj());

	MongoDB::ScopedDbConnectionPtr conn(mongo::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));
	auto_ptr<mongo::DBClientCursor> pCursor = conn->get()->query(_ns, builder.obj(), 0, 0, 0, mongo::QueryOption_SlaveOk);
	if (pCursor.get() && pCursor->more())
	{
		while (pCursor->more())
		{
			bindings.push_back(RegBinding(pCursor->next()));
		}
	}
	conn->done();


	return bindings.size() > 0;
}

bool RegDB::getUnexpiredContactsUserContaining(const string& matchIdentity, int timeNow, Bindings& bindings) const
{
	mongo::BSONObjBuilder query;

	if (_local) {
		_local->getUnexpiredContactsUserContaining(matchIdentity, timeNow, bindings);
		query.append("$not", BSON("shardId" << getShardId()));
	} else {
		// we only include expired in local because global can be stale.
		query.append("expirationTime", BSON_GREATER_THAN(timeNow));
	}

	mongo::BSONObjBuilder builder;
	BaseDB::nearest(builder, query.obj());

	MongoDB::ScopedDbConnectionPtr conn(mongo::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));
	auto_ptr<mongo::DBClientCursor> pCursor = conn->get()->query(_ns, builder.obj(), 0, 0, 0, mongo::QueryOption_SlaveOk);
	if (pCursor.get() && pCursor->more())
	{
		while (pCursor->more())
		{
			RegBinding binding(pCursor->next());
			if (binding.getContact().find(matchIdentity) != string::npos)
				bindings.push_back(binding);
		}
		conn->done();
		return bindings.size() > 0;
	}

	conn->done();
	return false;
}


bool RegDB::getUnexpiredContactsUserInstrument(const string& identity, const string& instrument, int timeNow,
		Bindings& bindings) const
{
	mongo::BSONObjBuilder query;
	query.append("identity", identity);
	query.append("instrument", instrument);
	if (_local) {
		_local->getUnexpiredContactsUserInstrument(identity, instrument, timeNow, bindings);
		query.append("$not", BSON("shardId" << getShardId()));
	} else {
		// we only include expired in local because global can be stale.
		query.append("expirationTime", BSON_GREATER_THAN(timeNow));
	}

	mongo::BSONObjBuilder builder;
	BaseDB::nearest(builder, query.obj());

	MongoDB::ScopedDbConnectionPtr conn(mongo::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));
	auto_ptr<mongo::DBClientCursor> pCursor = conn->get()->query(_ns, builder.obj(), 0, 0, 0, mongo::QueryOption_SlaveOk);
	if (pCursor.get() && pCursor->more())
	{
		while (pCursor->more())
		{
			bindings.push_back(RegBinding(pCursor->next()));
		}
		conn->done();
		return true;
	}

	conn->done();
	return false;
}

// TODO : Unclear how big this dataset would be, decide if this should be removed
bool RegDB::getUnexpiredContactsInstrument(const string& instrument, int timeNow, Bindings& bindings) const
{
	mongo::BSONObjBuilder query;
	query.append("instrument", instrument);
	if (_local) {
		_local->getUnexpiredContactsInstrument(instrument, timeNow, bindings);
		query.append("$not", BSON("shardId" << getShardId()));
	} else {
		// we only include expired in local because global can be stale.
		query.append("expirationTime", BSON_GREATER_THAN(timeNow));
	}

	mongo::BSONObjBuilder builder;
	BaseDB::nearest(builder, query.obj());

    MongoDB::ScopedDbConnectionPtr conn(mongo::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));
	auto_ptr<mongo::DBClientCursor> pCursor = conn->get()->query(_ns, builder.obj(), 0, 0, 0, mongo::QueryOption_SlaveOk);
	if (pCursor.get() && pCursor->more())
	{
		while (pCursor->more()) {
			bindings.push_back(RegBinding(pCursor->next()));
		}
		conn->done();
		return true;
	}

	conn->done();
	return false;
}

void RegDB::cleanAndPersist(int currentExpireTime)
{
	if (_local) {
		_local->cleanAndPersist(currentExpireTime);
		return;
	}
    mongo::BSONObj query = BSON(
        "expirationTime" << BSON_LESS_THAN(currentExpireTime));
    MongoDB::ScopedDbConnectionPtr conn(mongo::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));
    conn->get()->remove(_ns, query);
	conn->done();
}

void RegDB::clearAllBindings()
{
	if (_local) {
		_local->clearAllBindings();
		return;
	}
	mongo::BSONObj all;
    MongoDB::ScopedDbConnectionPtr conn(mongo::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));
    conn->get()->remove(_ns, all);
	conn->done();
}

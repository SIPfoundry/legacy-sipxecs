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
#include "sipdb/RegDB.h"

using namespace std;

const string RegDB::NS("node.registrar");

void RegDB::updateBinding(const RegBinding::Ptr& pBinding)
{
	updateBinding(*(pBinding.get()));
}

void RegDB::updateBinding(RegBinding& binding)
{
	if (binding.getTimestamp() == 0)
		binding.setTimestamp((int) OsDateTime::getSecsSinceEpoch());

	if (binding.getLocalAddress().empty())
	{
		string serverId = _localAddress;
		binding.setLocalAddress(serverId);
	}

	mongo::BSONObj query = BSON(
			"identity" << binding.getIdentity() <<
			"contact" << binding.getContact());

	mongo::BSONObj update;
	if (binding.getExpirationTime() > 0)
	{
		update = BSON("$set" << BSON(
						"timestamp" << binding.getTimestamp() <<
						"localAddress" << binding.getLocalAddress() <<
						"identity" << binding.getIdentity() <<
						"uri" << binding.getUri() <<
						"callId" << binding.getCallId() <<
						"contact" << binding.getContact() <<
						"qvalue" << binding.getQvalue() <<
						"instanceId" << binding.getInstanceId() <<
						"gruu" << binding.getGruu() <<
						"path" << binding.getPath() <<
						"cseq" << binding.getCseq() <<
						"expirationTime" << binding.getExpirationTime() <<
						"instrument" << binding.getInstrument() <<
						"expired" << binding.getExpired() ));
	}
	else
	{
		// This is an unregister.  mark it as inactive
		update = BSON("$set" << BSON(
						"cseq" << binding.getCseq() <<
						"expired" << true));
	}

	mongo::ScopedDbConnection conn(_info.getConnectionString());
	conn->update(_info.getNS(), query, update, true, false);
	conn->ensureIndex("node.registrar", BSON( "identity" << 1 ));
	conn.done();
}

void RegDB::expireOldBindings(const string& identity, const string& callId, unsigned int cseq,
		unsigned int timeNow)
{
	unsigned int expirationTime = timeNow - 1;
	mongo::BSONObj query = BSON(
			"identity" << identity <<
			"callId"<< callId <<
			"cseq" << BSON_LESS_THAN(cseq) <<
			"expirationTime" << BSON_GREATER_THAN_EQUAL(expirationTime));

	mongo::BSONObj update = BSON("$set" << BSON(
					"timestamp" << timeNow <<
					"expired" << true <<
					"cseq" << cseq));

	mongo::ScopedDbConnection conn(_info.getConnectionString());
	conn->update(_info.getNS(), query, update);
	conn->ensureIndex("node.registrar",  BSON( "identity" << 1 ));
	conn.done();
}

void RegDB::expireAllBindings(const string& identity, const string& callId, unsigned int cseq,
		unsigned int timeNow)
{
	unsigned int expirationTime = timeNow - 1;
	mongo::BSONObj query = BSON(
			"identity" << identity <<
			"expirationTime" << BSON_GREATER_THAN_EQUAL(expirationTime));

	mongo::BSONObj update = BSON("$set" << BSON(
					"timestamp" << timeNow <<
					"expired" << true <<
					"cseq" << cseq));

	mongo::ScopedDbConnection conn(_info.getConnectionString());
	conn->update(_info.getNS(), query, update, false, true);
	conn->ensureIndex("node.registrar",  BSON( "identity" << 1 ));
	conn.done();
}

void RegDB::expireAllBindings(unsigned int timeNow)
{
	mongo::BSONObj query = BSON("expirationTime" << BSON_GREATER_THAN_EQUAL(0) <<
			"localAddress" << _localAddress);

	mongo::BSONObj update = BSON("$set" << BSON(
					"timestamp" << timeNow <<
					"expired" << true));

	mongo::ScopedDbConnection conn(_info.getConnectionString());
	conn->update(_info.getNS(), query, update, false, true);
	conn->ensureIndex("node.registrar",  BSON( "identity" << 1 ));
	conn.done();
}

bool RegDB::isOutOfSequence(const string& identity, const string& callId, unsigned int cseq) const
{
	mongo::BSONObj query = BSON(
			"identity" << identity <<
			"callId" << callId);

	mongo::ScopedDbConnection conn(_info.getConnectionString());
	auto_ptr<mongo::DBClientCursor> pCursor = conn->query(_info.getNS(), query);
	if (pCursor.get() && pCursor->more())
	{
		while (pCursor->more())
		{
			RegBinding binding = pCursor->next();
			unsigned int a = binding.getCseq();
			if (a >= cseq) {
				conn.done();
				return true;
			}
		}
	}

	conn.done();
	return false;
}

bool RegDB::getUnexpiredContactsUser(const string& identity, int timeNow, Bindings& bindings) const
{
	static string gruuPrefix = GRUU_PREFIX;

	bool isGruu = identity.substr(0, gruuPrefix.size()) == gruuPrefix;
	mongo::BSONObj query;

	if (isGruu)
	{
		string searchString(identity);
		searchString += SIP_GRUU_URI_PARAM;
		query = BSON(
				"gruu" << searchString <<
				"expirationTime" << BSON_GREATER_THAN(timeNow) <<
				"expired" << false);
	}
	else
	{
		query = BSON(
				"identity" << identity <<
				"expirationTime" << BSON_GREATER_THAN(timeNow) <<
				"expired" << false);
	}

	mongo::ScopedDbConnection conn(_info.getConnectionString());
	auto_ptr<mongo::DBClientCursor> pCursor = conn->query(_info.getNS(), query);
	if (pCursor.get() && pCursor->more())
	{
		while (pCursor->more())
		{
			bindings.push_back(RegBinding(pCursor->next()));
		}
		conn.done();
		return true;
	}

	conn.done();
	return false;
}

bool RegDB::getUnexpiredContactsUserContaining(const string& matchIdentity, int timeNow, Bindings& bindings) const
{
	mongo::BSONObj query = BSON("expirationTime" << BSON_GREATER_THAN(timeNow) <<
			"expired" << false);

	mongo::ScopedDbConnection conn(_info.getConnectionString());
	auto_ptr<mongo::DBClientCursor> pCursor = conn->query(_info.getNS(), query);
	if (pCursor.get() && pCursor->more())
	{
		while (pCursor->more())
		{
			RegBinding binding(pCursor->next());
			if (binding.getIdentity().find(matchIdentity) != string::npos)
				bindings.push_back(binding);
		}
		conn.done();
		return bindings.size() > 0;
	}

	conn.done();
	return false;
}

bool RegDB::getUnexpiredContactsUserInstrument(const string& identity, const string& instrument, int timeNow,
		Bindings& bindings) const
{
	mongo::BSONObj query = BSON(
			"identity" << identity <<
			"instrument" << instrument <<
			"expirationTime" << BSON_GREATER_THAN(timeNow) <<
			"expired" << false);

	mongo::ScopedDbConnection conn(_info.getConnectionString());
	auto_ptr<mongo::DBClientCursor> pCursor = conn->query(_info.getNS(), query);
	if (pCursor.get() && pCursor->more())
	{
		while (pCursor->more())
		{
			bindings.push_back(RegBinding(pCursor->next()));
		}
		conn.done();
		return true;
	}

	conn.done();
	return false;
}

// TODO : Unclear how big this dataset would be, decide if this should be removed
bool RegDB::getUnexpiredContactsInstrument(const string& instrument, int timeNow, Bindings& bindings) const
{
	mongo::BSONObj query = BSON(
			"instrument" << instrument <<
			"expirationTime" << BSON_GREATER_THAN(timeNow) <<
			"expired" << false);

	mongo::ScopedDbConnection conn(_info.getConnectionString());
	auto_ptr<mongo::DBClientCursor> pCursor = conn->query(_info.getNS(), query);
	if (pCursor.get() && pCursor->more())
	{
		while (pCursor->more()) {
			bindings.push_back(RegBinding(pCursor->next()));
		}
		conn.done();
		return true;
	}

	conn.done();
	return false;
}

void RegDB::cleanAndPersist(int currentExpireTime)
{
	mongo::BSONObj query = BSON("expirationTime" << BSON_LESS_THAN(currentExpireTime));
	mongo::ScopedDbConnection conn(_info.getConnectionString());
	conn->remove(_info.getNS(), query);
	conn.done();
}

void RegDB::clearAllBindings()
{
	mongo::BSONObj all;
	mongo::ScopedDbConnection conn(_info.getConnectionString());
	conn->remove(_info.getNS(), all);
	conn.done();
}

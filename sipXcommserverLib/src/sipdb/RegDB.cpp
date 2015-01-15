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
  
  MongoDB::UpdateTimer updateTimer(const_cast<RegDB&>(*this));
  
	if (binding.getTimestamp() == 0)
		binding.setTimestamp(OsDateTime::getSecsSinceEpoch());

	if (binding.getLocalAddress().empty())
	{
		string serverId = _localAddress;
		binding.setLocalAddress(serverId);
	}
  
  if (binding.getBinding().empty())
  {
    Url curl(binding.getContact().c_str());
    UtlString hostPort;
    UtlString user;
    curl.getHostWithPort(hostPort);
    curl.getUserId(user);
    
    std::ostringstream strm;
    strm << "sip:";
    if (!user.isNull())
      strm << user.data() << "@";
    strm << hostPort.data();
    
    binding.setBinding(strm.str());
  }

	mongo::BSONObj query = BSON(
			"identity" << binding.getIdentity() <<
			"contact" << binding.getContact() <<
                        "shardId" << getShardId());

  bool isExpired = binding.getExpirationTime() <= 0;
	mongo::BSONObj update;
    update = BSON(
          "timestamp" << static_cast<long long>(binding.getTimestamp()) <<
          "localAddress" << binding.getLocalAddress() <<
          "identity" << binding.getIdentity() <<
          "uri" << binding.getUri() <<
          "callId" << binding.getCallId() <<
          "contact" << binding.getContact() <<
          "binding" << binding.getBinding() <<
          "qvalue" << binding.getQvalue() <<
          "instanceId" << binding.getInstanceId() <<
          "gruu" << binding.getGruu() <<
          "shardId" << getShardId() <<
          "path" << binding.getPath() <<
          "cseq" << binding.getCseq() <<
          "expirationTime" << static_cast<long long>(binding.getExpirationTime()) <<
          "instrument" << binding.getInstrument() <<
          "expired" << isExpired );

    MongoDB::ScopedDbConnectionPtr conn(mongoMod::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString(), getWriteQueryTimeout()));
    mongo::DBClientBase* client = conn->get();

    client->remove(_ns, query);
    client->insert(_ns, update);
    client->ensureIndex("node.registrar", BSON( "identity" << 1 ));
    client->ensureIndex("node.registrar", BSON( "expirationTime" << 1 ));

        string e = client->getLastError();
        if( !e.empty() ) {
          Os::Logger::instance().log(FAC_SIP, PRI_ERR, e.c_str());
        } else {
          Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "Save reg ok");
        }

	conn->done();
}

void RegDB::expireOldBindings(const string& identity, const string& callId, unsigned int cseq,
		unsigned long timeNow)
{
	if (_local != NULL) {
		_local->expireOldBindings(identity, callId, cseq, timeNow);
		return;
	}
  
  MongoDB::UpdateTimer updateTimer(const_cast<RegDB&>(*this));
	mongo::BSONObj query = BSON(
			"identity" << identity <<
			"callId"<< callId <<
			"cseq" << BSON_LESS_THAN(cseq) <<
                        "shardId" << getShardId());

    MongoDB::ScopedDbConnectionPtr conn(mongoMod::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString(), getWriteQueryTimeout()));
    mongo::DBClientBase* client = conn->get();

	client->remove(_ns, query);
	client->ensureIndex("node.registrar",  BSON( "identity" << 1 ));
	client->ensureIndex("node.registrar", BSON( "expirationTime" << 1 ));

	conn->done();
}

void RegDB::expireAllBindings(const string& identity, const string& callId, unsigned int cseq,
		unsigned long timeNow)
{
	if (_local != NULL) {
		_local->expireAllBindings(identity, callId, cseq, timeNow);
		return;
	}
  
  MongoDB::UpdateTimer updateTimer(const_cast<RegDB&>(*this));
	mongo::BSONObj query = BSON(
                        "shardId" << getShardId() <<
			"identity" << identity);

    MongoDB::ScopedDbConnectionPtr conn(mongoMod::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString(), getWriteQueryTimeout()));
    mongo::DBClientBase* client = conn->get();

    client->remove(_ns, query);
	client->ensureIndex("node.registrar",  BSON( "identity" << 1 ));
	client->ensureIndex("node.registrar", BSON( "expirationTime" << 1 ));

	conn->done();
}

void RegDB::removeAllExpired()
{
  if (_local != NULL)
  {

    _local->removeAllExpired();
    return;
  }

  unsigned long timeNow = OsDateTime::getSecsSinceEpoch();
  OS_LOG_INFO(FAC_SIP, "RegDB::removeAllExpired INVOKED for shard == " << getShardId() << " and expireTime <= " << timeNow);

  MongoDB::UpdateTimer updateTimer(const_cast<RegDB&>(*this));
  mongo::BSONObj query = BSON(
            "shardId" << getShardId() <<
            "expirationTime" << BSON_LESS_THAN_EQUAL((long long)timeNow));

  MongoDB::ScopedDbConnectionPtr conn(mongoMod::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString(), getWriteQueryTimeout()));
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

bool RegDB::isRegisteredBinding(const Url& curl, bool preferPrimary)
{
  bool isRegistered = false;
  UtlString hostPort;
  UtlString user;
  curl.getHostWithPort(hostPort);
  curl.getUserId(user);

  std::ostringstream binding;
  binding << "sip:";
  if (!user.isNull())
    binding << user.data() << "@";
  binding << hostPort.data();
  
  mongo::BSONObjBuilder query;
	query.append("binding", binding.str());

	if (_local)
  {
		preferPrimary = false;
		_local->isRegisteredBinding(curl, preferPrimary);
		query.append("shardId", BSON("$ne" << _local->getShardId()));
	} 

  MongoDB::ReadTimer readTimer(const_cast<RegDB&>(*this));
  
	mongo::BSONObjBuilder builder;
	if (!preferPrimary)
	  BaseDB::nearest(builder, query.obj());
	else
	  BaseDB::primaryPreferred(builder, query.obj());

	MongoDB::ScopedDbConnectionPtr conn(mongoMod::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString(), getReadQueryTimeout()));
	auto_ptr<mongo::DBClientCursor> pCursor = conn->get()->query(_ns, builder.obj(), 0, 0, 0, mongo::QueryOption_SlaveOk);
	isRegistered = pCursor.get() && pCursor->more();
	conn->done();
  
  OS_LOG_INFO(FAC_SIP, "RegDB::isRegisteredBinding returning " << (isRegistered ? "TRUE" : "FALSE") << " for binding " <<  binding.str());
   
	return isRegistered;
}

bool RegDB::getUnexpiredContactsUser(const string& identity, unsigned long timeNow, Bindings& bindings, bool preferPrimary) const
{
	static string gruuPrefix = GRUU_PREFIX;

	bool isGruu = identity.substr(0, gruuPrefix.size()) == gruuPrefix;

	mongo::BSONObjBuilder query;
  query.append("expirationTime", BSON_GREATER_THAN((long long)timeNow));

	if (_local)
  {
		preferPrimary = false;
		_local->getUnexpiredContactsUser(identity, timeNow, bindings, preferPrimary);
		query.append("shardId", BSON("$ne" << _local->getShardId()));
	}
  
   MongoDB::ReadTimer readTimer(const_cast<RegDB&>(*this));

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
	if (!preferPrimary)
	  BaseDB::nearest(builder, query.obj());
	else
	  BaseDB::primaryPreferred(builder, query.obj());

	MongoDB::ScopedDbConnectionPtr conn(mongoMod::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString(), getReadQueryTimeout()));
	auto_ptr<mongo::DBClientCursor> pCursor = conn->get()->query(_ns, builder.obj(), 0, 0, 0, mongo::QueryOption_SlaveOk);
	if (pCursor.get() && pCursor->more())
	{
		while (pCursor->more())
		{
      RegBinding binding(pCursor->next());
      
      if (binding.getExpirationTime() > timeNow)
      {
        OS_LOG_INFO(FAC_SIP, "RegDB::getUnexpiredContactsUser "
        << " Identity: " << identity
        << " Contact: " << binding.getContact()
        << " Expires: " << binding.getExpirationTime() - timeNow << " sec");
        bindings.push_back(binding);
      }
      else
      {
        OS_LOG_WARNING(FAC_SIP, "RegDB::getUnexpiredContactsUser returned an expired record?!?!"
          << " Identity: " << identity
          << " Contact: " << binding.getContact()
          << " Expires: " <<  binding.getExpirationTime() << " epoch"
          << " TimeNow: " << timeNow << " epoch");
      }

		}
	}
  else
  {
    OS_LOG_INFO(FAC_SIP, "RegDB::getUnexpiredContactsUser returned empty recordset for identity " << identity);
  }
	conn->done();


	return bindings.size() > 0;
}

bool RegDB::getUnexpiredContactsUserContaining(const string& matchIdentity, unsigned long timeNow, Bindings& bindings, bool preferPrimary) const
{
	mongo::BSONObjBuilder query;
  query.append("expirationTime", BSON_GREATER_THAN((long long)timeNow));

	if (_local)
  {
		preferPrimary = false;
		_local->getUnexpiredContactsUserContaining(matchIdentity, timeNow, bindings, preferPrimary);
		query.append("shardId", BSON("$ne" << _local->getShardId()));
	} 

  MongoDB::ReadTimer readTimer(const_cast<RegDB&>(*this));
   
	mongo::BSONObjBuilder builder;
	if (!preferPrimary)
	  BaseDB::nearest(builder, query.obj());
	else
	  BaseDB::primaryPreferred(builder, query.obj());

	MongoDB::ScopedDbConnectionPtr conn(mongoMod::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString(), getReadQueryTimeout()));
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


bool RegDB::getUnexpiredContactsUserInstrument(const string& identity, const string& instrument, unsigned long timeNow,
		Bindings& bindings, bool preferPrimary) const
{
	mongo::BSONObjBuilder query;
	query.append("identity", identity);
	query.append("instrument", instrument);
  query.append("expirationTime", BSON_GREATER_THAN((long long)timeNow));

	if (_local)
  {
		preferPrimary = false;
		_local->getUnexpiredContactsUserInstrument(identity, instrument, timeNow, bindings, preferPrimary);
		query.append("shardId", BSON("$ne" << _local->getShardId()));
	} 

  MongoDB::ReadTimer readTimer(const_cast<RegDB&>(*this));
   
	mongo::BSONObjBuilder builder;
	if (!preferPrimary)
	  BaseDB::nearest(builder, query.obj());
	else
	  BaseDB::primaryPreferred(builder, query.obj());

	MongoDB::ScopedDbConnectionPtr conn(mongoMod::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString(), getReadQueryTimeout()));
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
bool RegDB::getUnexpiredContactsInstrument(const string& instrument, unsigned long timeNow, Bindings& bindings, bool preferPrimary) const
{
	mongo::BSONObjBuilder query;
	query.append("instrument", instrument);
  query.append("expirationTime", BSON_GREATER_THAN((long long)timeNow));

	if (_local)
  {
  		preferPrimary = false;
		_local->getUnexpiredContactsInstrument(instrument, timeNow, bindings, preferPrimary);
		query.append("shardId", BSON("$ne" << _local->getShardId()));
	} 

  MongoDB::ReadTimer readTimer(const_cast<RegDB&>(*this));
	
  mongo::BSONObjBuilder builder;
	if (!preferPrimary)
	  BaseDB::nearest(builder, query.obj());
	else
	  BaseDB::primaryPreferred(builder, query.obj());

    MongoDB::ScopedDbConnectionPtr conn(mongoMod::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString(), getReadQueryTimeout()));
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
  
   MongoDB::UpdateTimer updateTimer(const_cast<RegDB&>(*this));
    mongo::BSONObj query = BSON(
        "expirationTime" << BSON_LESS_THAN(currentExpireTime));
    MongoDB::ScopedDbConnectionPtr conn(mongoMod::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString(), getWriteQueryTimeout()));
    conn->get()->remove(_ns, query);
	conn->done();
}

void RegDB::clearAllBindings()
{
	if (_local) {
		_local->clearAllBindings();
		return;
	}
  
  MongoDB::UpdateTimer updateTimer(const_cast<RegDB&>(*this));
  
	mongo::BSONObj all;
    MongoDB::ScopedDbConnectionPtr conn(mongoMod::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString(), getWriteQueryTimeout()));
    conn->get()->remove(_ns, all);
	conn->done();
}

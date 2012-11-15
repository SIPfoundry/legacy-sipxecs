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

#ifndef ENTITYDB_H
#define	ENTITYDB_H

#include <set>
#include "sipdb/EntityRecord.h"
#include "utl/UtlString.h"
#include "net/Url.h"

class EntityDB: public MongoDB::BaseDB
{
public:
	static const std::string NS;
	typedef std::vector<EntityRecord> Entities;
	typedef std::map<std::string, EntityRecord> EntitiesByIdentity;
	typedef std::vector<EntityRecord::Alias> Aliases;
	typedef std::set<std::string> Permissions;

	EntityDB(const MongoDB::ConnectionInfo& info) :
		BaseDB(info)
	{
    _lastTailId = mongo::minKey.firstElement();
	}
	;

	~EntityDB()
	{
	}
	;

	bool findByIdentity(const std::string& identity, EntityRecord& entity) const;
	bool findByIdentity(const Url& uri, EntityRecord& entity) const;
	bool findByIdentityOrAlias(const std::string& identity, const std::string& alias, EntityRecord& entity) const;
	bool findByIdentityOrAlias(const Url& uri, EntityRecord& entity) const;
	bool findByUserId(const std::string& userId, EntityRecord& entity) const;
	bool findByAliasUserId(const std::string& alias, EntityRecord& entity) const;
        bool findByGatewayIdentity(const std::string& identity, EntityRecord& entity) const;
  bool findByAliasIdentity(const std::string& identity, EntityRecord& entity) const;

	/// Retrieve the SIP credential check values for a given identity and realm
	bool getCredential(const Url& uri, const UtlString& realm, UtlString& userid, UtlString& passtoken,
			UtlString& authType) const;

	/// Retrieve the SIP credential check values for a given userid and realm
	bool getCredential(const UtlString& userid, const UtlString& realm, Url& uri, UtlString& passtoken,
			UtlString& authType) const;

	// Query interface to return a set of mapped full URI
	// contacts associated with the alias
	void getAliasContacts(const Url& aliasIdentity, Aliases& aliases, bool& isUserIdentity) const;
	
	bool tail(std::vector<std::string>& opLogs);

private:
  mongo::BSONElement _lastTailId;
};

#endif	/* ENTITYDB_H */


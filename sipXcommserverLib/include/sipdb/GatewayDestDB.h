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

#ifndef GatewayDestDB_H
#define	GatewayDestDB_H

#include "sipdb/GatewayDestRecord.h"
#include "sipdb/MongoDB.h"

/**
 * GatewayDestDB class is the interface to mongo db collection "node.gatewaydest"
 * used to manipulate gateway records. The following operations are available:
 * - add new records to mongo;
 * - update existing records in mongo;
 * - remove all existing records;
 * - remove all expired records;
 * - retrieve unexpired records;
 *
 * @nosubgrouping
 */

class GatewayDestDB : public MongoDB::BaseDB
{
public:
	static const std::string NS;

	/// Default Constructor
  GatewayDestDB(const MongoDB::ConnectionInfo& info) :
    BaseDB(info)
	{
	}
	;

	~GatewayDestDB()
	{
	}
	;

	/// Adds or updates an existing record in mongo
	void updateRecord(const GatewayDestRecord& record, bool upsert);
  /**<
   * Adds or updates an existing record in mongo.
   * when the SipXSignedHeader information is generated.
   * @param upsert Allow mongo upsert? If true and record does not exist in mongo
   * it will be added. If upsert is false, and record does not exist it will not
   * be added
   */

	void removeRecord(const GatewayDestRecord& record);

	void removeAllRecords();

	void removeAllExpired();

	bool getUnexpiredRecord(GatewayDestRecord& record) const;

protected:

private:

};

#endif	/* GatewayDestDB_H */


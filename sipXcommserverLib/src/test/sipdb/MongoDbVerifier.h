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

#ifndef _MONGO_VERIFIER_H__
#define _MONGO_VERIFIER_H__

#include "sipdb/MongoDB.h"
#include <string>

class MongoDbVerifier
{
public:
  MongoDbVerifier(MongoDB::ScopedDbConnectionPtr& conn,
                const std::string& dbName,
                int maxTimeToWaitMs,
                int timeToWaitBetweenRetriesMs = 1000);
  ~MongoDbVerifier();

  void waitUtilEmpty(mongo::BSONObj bSONObj = mongo::BSONObj());
  void waitUtilHaveOneEntry(mongo::BSONObj bSONObj = mongo::BSONObj());
  void waitUtilReachNumberOfEntries(mongo::BSONObj bSONObj, unsigned long long numberOfEntries);
private:
  //! Disabled copy constructor
  MongoDbVerifier(const MongoDbVerifier& rhs);

  //! Disabled assignment operator
  MongoDbVerifier& operator=(const MongoDbVerifier& rhs);

  void wait(mongo::BSONObj bSONObj, bool empty);

  MongoDB::ScopedDbConnectionPtr& _conn;
  const std::string _dbName;
  int _maxTimeToWaitMs;
  int _timeToWaitBetweenRetriesMs;
};


#endif  /* _MONGO_VERIFIER_H__ */

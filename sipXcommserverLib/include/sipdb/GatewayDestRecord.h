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

#ifndef GatewayDestRecord_H
#define	GatewayDestRecord_H


#include <string>
#include <boost/shared_ptr.hpp>
#include "sipdb/MongoDB.h"

/**
 * GatewayDestRecord class encapsulates information about gateways addresses used in call
 * transfer scenarios. This information will be stored in mongo db.
 *  _callId - callid of the call which used the gateway as destination;
    _toTag - to-tag identifier of the callid;
    _fromTag - from-tag identifier of the callid;
    _lineId - line-id of the gateway, used to identify it;
    _expirationTime - time in sec, specifying for how long the record is valid
 *
 * @nosubgrouping
 */

class GatewayDestRecord
{
public:
  GatewayDestRecord();
  GatewayDestRecord(const std::string& callId, const std::string& toTag, const std::string& fromTag);
  GatewayDestRecord(const GatewayDestRecord& record);
  GatewayDestRecord(const mongo::BSONObj& bson);

  ~GatewayDestRecord();

  void swap(GatewayDestRecord& record);
  GatewayDestRecord& operator=(const GatewayDestRecord& record);
  GatewayDestRecord& operator=(const mongo::BSONObj& bson);

  const std::string& getCallId() const;
  static const char* callIdField();

  const std::string& getToTag() const;
  static const char* toTagField();

  const std::string& getFromTag() const;
  static const char* fromTagField();

  const std::string& getIdentity() const;
  static const char* identityField();

  const std::string& getLineId() const;
  static const char* lineIdField();

  unsigned int getExpirationTime() const;
  static const char* expirationTimeField();


  void setCallId(const std::string& callId);
  void setToTag(const std::string& toTag);
  void setFromTag(const std::string& fromTag);
  void setIdentity(const std::string& identity);
  void setLineId(const std::string& lineId);
  void setExpirationTime(unsigned int expirationTime);

private:
  std::string _callId;
  std::string _toTag;
  std::string _fromTag;
  std::string _identity;
  std::string _lineId;

  unsigned int _expirationTime;
};

//
// Inlines
//

inline const std::string& GatewayDestRecord::getCallId() const
{
  return _callId;
}

inline const std::string& GatewayDestRecord::getToTag() const
{
  return _toTag;
}

inline const std::string& GatewayDestRecord::getFromTag() const
{
  return _fromTag;
}

inline const std::string& GatewayDestRecord::getIdentity() const
{
  return _identity;
}

inline const std::string& GatewayDestRecord::getLineId() const
{
  return _lineId;
}

inline unsigned int GatewayDestRecord::getExpirationTime() const
{
    return _expirationTime;
}

inline void GatewayDestRecord::setCallId(const std::string& callId)
{
  _callId = callId;
}

inline void GatewayDestRecord::setToTag(const std::string& toTag)
{
  _toTag = toTag;
}

inline void GatewayDestRecord::setFromTag(const std::string& fromTag)
{
  _fromTag = fromTag;
}

inline void GatewayDestRecord::setIdentity(const std::string& identity)
{
  _identity = identity;
}

inline void GatewayDestRecord::setLineId(const std::string& lineId)
{
  _lineId = lineId;
}

inline void GatewayDestRecord::setExpirationTime(unsigned int expirationTime)
{
  _expirationTime = expirationTime;
}

#endif	/* GatewayDestRecord_H */

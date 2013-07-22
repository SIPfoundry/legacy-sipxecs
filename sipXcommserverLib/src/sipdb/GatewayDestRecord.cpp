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

#include "sipdb/GatewayDestRecord.h"

const char* GatewayDestRecord::callIdField() { static std::string field = "callid"; return field.c_str(); }
const char* GatewayDestRecord::toTagField() { static std::string field = "toTag"; return field.c_str(); }
const char* GatewayDestRecord::fromTagField() { static std::string field = "fromTag"; return field.c_str(); }
const char* GatewayDestRecord::identityField() { static std::string field = "identity"; return field.c_str(); }
const char* GatewayDestRecord::lineIdField() { static std::string field = "lineId"; return field.c_str(); }
const char* GatewayDestRecord::expirationTimeField() { static std::string field = "expirationTime"; return field.c_str(); }

GatewayDestRecord::GatewayDestRecord() :
    _expirationTime(0)
{
}

GatewayDestRecord::GatewayDestRecord(const std::string& callId, const std::string& toTag, const std::string& fromTag) :
    _callId(callId),
    _toTag(toTag),
    _fromTag(fromTag),
    _expirationTime(0)
{
}

GatewayDestRecord::GatewayDestRecord(const GatewayDestRecord& record)
{
  _callId = record._callId;
  _toTag = record._toTag;
  _fromTag = record._fromTag;
  _identity = record._identity;
  _lineId = record._lineId;
  _expirationTime = record._expirationTime;
}

GatewayDestRecord::~GatewayDestRecord()
{
}

void GatewayDestRecord::swap(GatewayDestRecord& record)
{
  std::swap(_callId, record._callId);
  std::swap(_toTag, record._toTag);
  std::swap(_fromTag, record._fromTag);
  std::swap(_identity, record._identity);
  std::swap(_lineId, record._lineId);
  std::swap(_expirationTime, record._expirationTime);
}

GatewayDestRecord::GatewayDestRecord(const mongo::BSONObj& bson)
{
  if (bson.hasField(callIdField()))
    _callId = bson.getStringField(callIdField());

  if (bson.hasField(toTagField()))
    _toTag = bson.getStringField(toTagField());

  if (bson.hasField(fromTagField()))
    _fromTag = bson.getStringField(fromTagField());

  if (bson.hasField(identityField()))
    _identity = bson.getStringField(identityField());

  if (bson.hasField(lineIdField()))
    _lineId = bson.getStringField(lineIdField());

  if (bson.hasField(expirationTimeField()))
    _expirationTime = bson.getIntField(expirationTimeField());
}


GatewayDestRecord& GatewayDestRecord::operator=(const mongo::BSONObj& bson)
{
  if (bson.hasField(callIdField()))
    _callId = bson.getStringField(callIdField());

  if (bson.hasField(toTagField()))
    _toTag = bson.getStringField(toTagField());

  if (bson.hasField(fromTagField()))
    _fromTag = bson.getStringField(fromTagField());

  if (bson.hasField(identityField()))
    _identity = bson.getStringField(identityField());

  if (bson.hasField(lineIdField()))
    _lineId = bson.getStringField(lineIdField());

  if (bson.hasField(expirationTimeField()))
    _expirationTime = bson.getIntField(expirationTimeField());

    return *this;
}

GatewayDestRecord& GatewayDestRecord::operator=(const GatewayDestRecord& record)
{
  GatewayDestRecord clonable(record);

  swap(clonable);

  return *this;
}

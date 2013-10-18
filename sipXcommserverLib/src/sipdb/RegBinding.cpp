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

#include "sipdb/RegBinding.h"

RegBinding::RegBinding() :
    _cseq(0),
    _expirationTime(0),
    _timestamp(0),
    _expired(false)
{
}

RegBinding::RegBinding(const RegBinding& binding)
{
    _identity = binding._identity;
    _uri = binding._uri;
    _callId = binding._callId;
    _contact = binding._contact;
    _qvalue = binding._qvalue;
    _instanceId = binding._instanceId;
    _gruu = binding._gruu;
    _shardId = binding._shardId;
    _path = binding._path;
    _cseq = binding._cseq;
    _expirationTime = binding._expirationTime;
    _instrument = binding._instrument;
    _localAddress = binding._localAddress;
    _timestamp = binding._timestamp;
    _expired = binding._expired;
}

RegBinding::~RegBinding()
{
}

void RegBinding::swap(RegBinding& binding)
{
    std::swap(_identity, binding._identity);
    std::swap(_uri, binding._uri);
    std::swap(_callId, binding._callId);
    std::swap(_contact, binding._contact);
    std::swap(_qvalue, binding._qvalue);
    std::swap(_instanceId, binding._instanceId);
    std::swap(_gruu, binding._gruu);
    std::swap(_shardId, binding._shardId);
    std::swap(_path, binding._path);
    std::swap(_cseq, binding._cseq);
    std::swap(_expirationTime, binding._expirationTime);
    std::swap(_instrument, binding._instrument);
    std::swap(_localAddress, binding._localAddress);
    std::swap(_timestamp, binding._timestamp);
    std::swap(_expired, binding._expired);
}

RegBinding::RegBinding(const mongo::BSONObj& bson)
{
    if (bson.hasField("identity"))
      _identity = bson.getStringField("identity");

    if (bson.hasField("uri"))
      _uri = bson.getStringField("uri");

    if (bson.hasField("callId"))
      _callId = bson.getStringField("callId");

    if (bson.hasField("contact"))
      _contact = bson.getStringField("contact");

    if (bson.hasField("qvalue"))
      _qvalue = bson.getStringField("qvalue");

    if (bson.hasField("instanceId"))
      _instanceId = bson.getStringField("instanceId");

    if (bson.hasField("gruu"))
      _gruu = bson.getStringField("gruu");

    if (bson.hasField("shardId"))
      _shardId = bson.getIntField("shardId");

    if (bson.hasField("path"))
      _path = bson.getStringField("path");

    if (bson.hasField("cseq"))
      _cseq = bson.getIntField("cseq");

    if (bson.hasField("expirationTime"))
      _expirationTime = bson.getIntField("expirationTime");

    if (bson.hasField("instrument"))
      _instrument = bson.getStringField("instrument");

    if (bson.hasField("localAddress"))
      _localAddress = bson.getStringField("localAddress");

    if (bson.hasField("timestamp"))
      _timestamp = bson.getIntField("timestamp");

    if (bson.hasField("expired"))
      _expired = bson.getBoolField("expired");
}

RegBinding& RegBinding::operator=(const mongo::BSONObj& bson)
{
    if (bson.hasField("identity"))
      _identity = bson.getStringField("identity");

    if (bson.hasField("uri"))
      _uri = bson.getStringField("uri");

    if (bson.hasField("callId"))
      _callId = bson.getStringField("callId");

    if (bson.hasField("contact"))
      _contact = bson.getStringField("contact");

    if (bson.hasField("qvalue"))
      _qvalue = bson.getStringField("qvalue");

    if (bson.hasField("instanceId"))
      _instanceId = bson.getStringField("instanceId");

    if (bson.hasField("gruu"))
      _gruu = bson.getStringField("gruu");

    if (bson.hasField("shardId"))
      _shardId = bson.getIntField("shardId");

    if (bson.hasField("path"))
      _path = bson.getStringField("path");

    if (bson.hasField("cseq"))
      _cseq = bson.getIntField("cseq");

    if (bson.hasField("expirationTime"))
      _expirationTime = bson.getIntField("expirationTime");

    if (bson.hasField("instrument"))
      _instrument = bson.getStringField("instrument");

    if (bson.hasField("localAddress"))
      _localAddress = bson.getStringField("localAddress");

    if (bson.hasField("timestamp"))
      _timestamp = bson.getIntField("timestamp");

    if (bson.hasField("expired"))
      _expired = bson.getBoolField("expired");
    
    return *this;
}

RegBinding& RegBinding::operator=(const RegBinding& binding)
{
  RegBinding clonable(binding);
  swap(clonable);
  return *this;
}



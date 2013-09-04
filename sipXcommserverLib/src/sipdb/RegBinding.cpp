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

const char* RegBinding::identity_fld(){ static std::string fld = "identity"; return fld.c_str(); }
const char* RegBinding::uri_fld(){ static std::string fld = "uri"; return fld.c_str(); }
const char* RegBinding::callId_fld(){ static std::string fld = "callId"; return fld.c_str(); }
const char* RegBinding::contact_fld(){ static std::string fld = "contact"; return fld.c_str(); }
const char* RegBinding::qvalue_fld(){ static std::string fld = "qvalue"; return fld.c_str(); }
const char* RegBinding::instanceId_fld(){ static std::string fld = "instanceId"; return fld.c_str(); }
const char* RegBinding::gruu_fld(){ static std::string fld = "gruu"; return fld.c_str(); }
const char* RegBinding::path_fld(){ static std::string fld = "path"; return fld.c_str(); }
const char* RegBinding::shardId_fld(){ static std::string fld = "shardId"; return fld.c_str(); }
const char* RegBinding::cseq_fld(){ static std::string fld = "cseq"; return fld.c_str(); }
const char* RegBinding::expirationTime_fld(){ static std::string fld = "expirationTime"; return fld.c_str(); }
const char* RegBinding::instrument_fld(){ static std::string fld = "instrument"; return fld.c_str(); }
const char* RegBinding::localAddress_fld(){ static std::string fld = "localAddress"; return fld.c_str(); }
const char* RegBinding::timestamp_fld(){ static std::string fld = "timestamp"; return fld.c_str(); }
const char* RegBinding::expired_fld(){ static std::string fld = "expired"; return fld.c_str(); }


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
    if (bson.hasField(RegBinding::identity_fld()))
      _identity = bson.getStringField(RegBinding::identity_fld());

    if (bson.hasField(RegBinding::uri_fld()))
      _uri = bson.getStringField(RegBinding::uri_fld());

    if (bson.hasField(RegBinding::callId_fld()))
      _callId = bson.getStringField(RegBinding::callId_fld());

    if (bson.hasField(RegBinding::contact_fld()))
      _contact = bson.getStringField(RegBinding::contact_fld());

    if (bson.hasField(RegBinding::qvalue_fld()))
      _qvalue = bson.getStringField(RegBinding::qvalue_fld());

    if (bson.hasField(RegBinding::instanceId_fld()))
      _instanceId = bson.getStringField(RegBinding::instanceId_fld());

    if (bson.hasField(RegBinding::gruu_fld()))
      _gruu = bson.getStringField(RegBinding::gruu_fld());

    if (bson.hasField(RegBinding::shardId_fld()))
      _shardId = bson.getIntField(RegBinding::shardId_fld());

    if (bson.hasField(RegBinding::path_fld()))
      _path = bson.getStringField(RegBinding::path_fld());

    if (bson.hasField(RegBinding::cseq_fld()))
      _cseq = bson.getIntField(RegBinding::cseq_fld());

    if (bson.hasField(RegBinding::expirationTime_fld()))
      _expirationTime = bson.getIntField(RegBinding::expirationTime_fld());

    if (bson.hasField(RegBinding::instrument_fld()))
      _instrument = bson.getStringField(RegBinding::instrument_fld());

    if (bson.hasField(RegBinding::localAddress_fld()))
      _localAddress = bson.getStringField(RegBinding::localAddress_fld());

    if (bson.hasField(RegBinding::timestamp_fld()))
      _timestamp = bson.getIntField(RegBinding::timestamp_fld());

    if (bson.hasField(RegBinding::expired_fld()))
      _expired = bson.getBoolField(RegBinding::expired_fld());
}

RegBinding& RegBinding::operator=(const mongo::BSONObj& bson)
{
    if (bson.hasField(RegBinding::identity_fld()))
      _identity = bson.getStringField(RegBinding::identity_fld());

    if (bson.hasField(RegBinding::uri_fld()))
      _uri = bson.getStringField(RegBinding::uri_fld());

    if (bson.hasField(RegBinding::callId_fld()))
      _callId = bson.getStringField(RegBinding::callId_fld());

    if (bson.hasField(RegBinding::contact_fld()))
      _contact = bson.getStringField(RegBinding::contact_fld());

    if (bson.hasField(RegBinding::qvalue_fld()))
      _qvalue = bson.getStringField(RegBinding::qvalue_fld());

    if (bson.hasField(RegBinding::instanceId_fld()))
      _instanceId = bson.getStringField(RegBinding::instanceId_fld());

    if (bson.hasField(RegBinding::gruu_fld()))
      _gruu = bson.getStringField(RegBinding::gruu_fld());

    if (bson.hasField(RegBinding::shardId_fld()))
      _shardId = bson.getIntField(RegBinding::shardId_fld());

    if (bson.hasField(RegBinding::shardId_fld()))
      _shardId = bson.getIntField(RegBinding::shardId_fld());

    if (bson.hasField(RegBinding::path_fld()))
      _path = bson.getStringField(RegBinding::path_fld());

    if (bson.hasField(RegBinding::cseq_fld()))
      _cseq = bson.getIntField(RegBinding::cseq_fld());

    if (bson.hasField(RegBinding::expirationTime_fld()))
      _expirationTime = bson.getIntField(RegBinding::expirationTime_fld());

    if (bson.hasField(RegBinding::instrument_fld()))
      _instrument = bson.getStringField(RegBinding::instrument_fld());

    if (bson.hasField(RegBinding::localAddress_fld()))
      _localAddress = bson.getStringField(RegBinding::localAddress_fld());

    if (bson.hasField(RegBinding::timestamp_fld()))
      _timestamp = bson.getIntField(RegBinding::timestamp_fld());

    if (bson.hasField(RegBinding::expired_fld()))
      _expired = bson.getBoolField(RegBinding::expired_fld());
    
    return *this;
}

RegBinding& RegBinding::operator=(const RegBinding& binding)
{
  RegBinding clonable(binding);
  swap(clonable);
  return *this;
}



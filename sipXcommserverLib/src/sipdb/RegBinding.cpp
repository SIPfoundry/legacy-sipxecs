#include "sipdb/RegBinding.h"

RegBinding::RegBinding() :
    _cseq(0),
    _expirationTime(0)
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
    _path = binding._path;
    _cseq = binding._cseq;
    _expirationTime = binding._expirationTime;
    _instrument = binding._instrument;
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
    std::swap(_path, binding._path);
    std::swap(_cseq, binding._cseq);
    std::swap(_expirationTime, binding._expirationTime);
    std::swap(_instrument, binding._instrument);
}

RegBinding::RegBinding(const MongoDB::BSONObj& bson)
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

    if (bson.hasField("path"))
      _path = bson.getStringField("path");

    if (bson.hasField("cseq"))
      _cseq = bson.getIntField("cseq");

    if (bson.hasField("expirationTime"))
      _expirationTime = bson.getIntField("expirationTime");

    if (bson.hasField("instrument"))
      _instrument = bson.getStringField("instrument");
}

RegBinding& RegBinding::operator=(const MongoDB::BSONObj& bson)
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

    if (bson.hasField("path"))
      _path = bson.getStringField("path");

    if (bson.hasField("cseq"))
      _cseq = bson.getIntField("cseq");

    if (bson.hasField("expirationTime"))
      _expirationTime = bson.getIntField("expirationTime");

    if (bson.hasField("instrument"))
      _instrument = bson.getStringField("instrument");

    return *this;
}

RegBinding& RegBinding::operator=(const RegBinding& binding)
{
  RegBinding clonable(binding);
  swap(clonable);
  return *this;
}



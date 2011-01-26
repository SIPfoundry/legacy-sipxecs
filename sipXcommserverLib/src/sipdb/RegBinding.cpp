#include "sipdb/RegBinding.h"

RegBinding::RegBinding()
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
    _expires = binding._expires;
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
    std::swap(_expires, binding._expires);
    std::swap(_instrument, binding._instrument);
}

RegBinding::RegBinding(const MongoDB::BSONObj& bson)
{
    _identity = bson.getStringField("identity");
    _uri = bson.getStringField("uri");
    _callId = bson.getStringField("callId");
    _contact = bson.getStringField("contact");
    _qvalue = bson.getStringField("qvalue");
    _instanceId = bson.getStringField("instanceId");
    _gruu = bson.getStringField("gruu");
    _path = bson.getStringField("path");
    _cseq = bson.getIntField("cseq");
    _expires = bson.getIntField("expires");
    _instrument = bson.getStringField("instrument");
}

RegBinding& RegBinding::operator=(const MongoDB::BSONObj& bson)
{
    _identity = bson.getStringField("identity");
    _uri = bson.getStringField("uri");
    _callId = bson.getStringField("callId");
    _contact = bson.getStringField("contact");
    _qvalue = bson.getStringField("qvalue");
    _instanceId = bson.getStringField("instanceId");
    _gruu = bson.getStringField("gruu");
    _path = bson.getStringField("path");
    _cseq = bson.getIntField("cseq");
    _expires = bson.getIntField("expires");
    _instrument = bson.getStringField("instrument");
    return *this;
}

RegBinding& RegBinding::operator=(const RegBinding& binding)
{
  RegBinding clonable(binding);
  swap(clonable);
  return *this;
}



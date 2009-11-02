// $Id$
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "net/Url.h"
#include "utl/UtlHashMap.h"
#include "utl/UtlInt.h"
#include "utl/UtlLongLongInt.h"
#include "utl/UtlString.h"

#include "sipdb/RegistrationBinding.h"
#include "sipdb/RegistrationDB.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType RegistrationBinding::TYPE = "RegistrationBinding";


// Default constructor
RegistrationBinding::RegistrationBinding() :
   mIdentity(NULL),
   mUri(NULL),
   mCallId(NULL),
   mContact(NULL),
   mQvalue(NULL),
   mInstanceId(NULL),
   mGruu(NULL),
   mPath(NULL),
   mCseq(0),
   mExpires(0),
   mPrimary(NULL),
   mUpdateNumber(0),
   mInstrument(NULL)
{
}


RegistrationBinding::RegistrationBinding(const UtlHashMap& regData) :
   mIdentity(NULL),
   mUri(NULL),
   mCallId(NULL),
   mContact(NULL),
   mQvalue(NULL),
   mInstanceId(NULL),
   mGruu(NULL),
   mPath(NULL),
   mCseq(0),
   mExpires(0),
   mPrimary(NULL),
   mUpdateNumber(0),
   mInstrument(NULL)
{
   UtlString* identityStr = dynamic_cast<UtlString*>(regData.findValue(&RegistrationDB::gIdentityKey));
   if (identityStr)
   {
      setIdentity(*identityStr);
   }
   UtlString* uriStr = dynamic_cast<UtlString*>(regData.findValue(&RegistrationDB::gUriKey));
   if (uriStr)
   {
      setUri(*uriStr);
   }
   UtlString* callidStr = dynamic_cast<UtlString*>(regData.findValue(&RegistrationDB::gCallidKey));
   if (callidStr)
   {
      setCallId(*callidStr);
   }
   UtlString* contactStr = dynamic_cast<UtlString*>(regData.findValue(&RegistrationDB::gContactKey));
   if (contactStr)
   {
      setContact(*contactStr);
   }
   UtlString* qvalueStr = dynamic_cast<UtlString*>(regData.findValue(&RegistrationDB::gQvalueKey));
   if (qvalueStr)
   {
      setQvalue(*qvalueStr);
   }
   UtlString* instanceIdStr = dynamic_cast<UtlString*>(regData.findValue(&RegistrationDB::gInstanceIdKey));
   if (instanceIdStr)
   {
      setInstanceId(*instanceIdStr);
   }
   UtlString* gruuStr = dynamic_cast<UtlString*>(regData.findValue(&RegistrationDB::gGruuKey));
   if (gruuStr)
   {
      setGruu(*gruuStr);
   }
   UtlString* pathStr = dynamic_cast<UtlString*>(regData.findValue(&RegistrationDB::gPathKey));
   if (pathStr)
   {
      setPath(*pathStr);
   }
   UtlInt* cseq = dynamic_cast<UtlInt*>(regData.findValue(&RegistrationDB::gCseqKey));
   if (cseq)
   {
      setCseq(cseq->getValue());
   }
   UtlInt* expires = dynamic_cast<UtlInt*>(regData.findValue(&RegistrationDB::gExpiresKey));
   if (expires)
   {
      setExpires(expires->getValue());
   }
   UtlString* primaryStr = dynamic_cast<UtlString*>(regData.findValue(&RegistrationDB::gPrimaryKey));
   if (primaryStr)
   {
      setPrimary(*primaryStr);
   }
   UtlLongLongInt* updateNumber = dynamic_cast<UtlLongLongInt*>(regData.findValue(&RegistrationDB::gUpdateNumberKey));
   if (updateNumber)
   {
      setUpdateNumber(updateNumber->getValue());
   }
   UtlString* instrumentStr = dynamic_cast<UtlString*>(regData.findValue(&RegistrationDB::gInstrumentKey));
   if (instrumentStr)
   {
      setInstrument(*instrumentStr);
   }
}

RegistrationBinding::~RegistrationBinding()
{
   delete mIdentity;
   delete mUri;
   delete mCallId;
   delete mContact;
   delete mQvalue;
   delete mInstanceId;
   delete mGruu;
   delete mPath;
   delete mPrimary;
   delete mInstrument;
}

void RegistrationBinding::copy(UtlHashMap& map) const
{
   if (mIdentity)
   {
      UtlString* identityKey = new UtlString(RegistrationDB::gIdentityKey);
      UtlString* identityValue = new UtlString(*mIdentity);
      map.insertKeyAndValue(identityKey, identityValue);
   }

   if (mUri)
   {
      UtlString* uriKey = new UtlString(RegistrationDB::gUriKey);
      UtlString* uriValue = new UtlString();
      mUri->toString(*uriValue);
      map.insertKeyAndValue(uriKey, uriValue);
   }

   if (mCallId)
   {
      UtlString* callIdKey = new UtlString(RegistrationDB::gCallidKey);
      UtlString* callIdValue = new UtlString(*mCallId);
      map.insertKeyAndValue(callIdKey, callIdValue);
   }

   if (mContact)
   {
      UtlString* contactKey = new UtlString(RegistrationDB::gContactKey);
      UtlString* contactValue = new UtlString(*mContact);
      map.insertKeyAndValue(contactKey, contactValue);
   }

   if (mQvalue)
   {
      UtlString* qvalueKey = new UtlString(RegistrationDB::gQvalueKey);
      UtlString* qvalueValue = new UtlString(*mQvalue);
      map.insertKeyAndValue(qvalueKey, qvalueValue);
   }

   if (mInstanceId)
   {
      UtlString* instanceIdKey = new UtlString(RegistrationDB::gInstanceIdKey);
      UtlString* instanceIdValue = new UtlString(*mInstanceId);
      map.insertKeyAndValue(instanceIdKey, instanceIdValue);
   }

   if (mGruu)
   {
      UtlString* gruuKey = new UtlString(RegistrationDB::gGruuKey);
      UtlString* gruuValue = new UtlString(*mGruu);
      map.insertKeyAndValue(gruuKey, gruuValue);
   }

   if (mPath)
   {
      UtlString* pathKey = new UtlString(RegistrationDB::gPathKey);
      UtlString* pathValue = new UtlString(*mPath);
      map.insertKeyAndValue(pathKey, pathValue);
   }

   UtlString* cseqKey = new UtlString(RegistrationDB::gCseqKey);
   UtlInt* cseqValue = new UtlInt(mCseq);
   map.insertKeyAndValue(cseqKey, cseqValue);

   UtlString* expiresKey = new UtlString(RegistrationDB::gExpiresKey);
   UtlInt* expiresValue = new UtlInt(mExpires);
   map.insertKeyAndValue(expiresKey, expiresValue);

   if (mPrimary)
   {
      UtlString* primaryKey = new UtlString(RegistrationDB::gPrimaryKey);
      UtlString* primaryValue = new UtlString(*mPrimary);
      map.insertKeyAndValue(primaryKey, primaryValue);
   }

   if (mGruu)
   {
      UtlString* gruuKey = new UtlString(RegistrationDB::gGruuKey);
      UtlString* gruuValue = new UtlString(*mGruu);
      map.insertKeyAndValue(gruuKey, gruuValue);
   }

   if (mInstrument)
   {
      UtlString* instrumentKey = new UtlString(RegistrationDB::gInstrumentKey);
      UtlString* instrumentValue = new UtlString(*mInstrument);
      map.insertKeyAndValue(instrumentKey, instrumentValue);
   }

   UtlString* updateNumberKey = new UtlString(RegistrationDB::gUpdateNumberKey);
   UtlLongLongInt* updateNumberValue = new UtlLongLongInt(mUpdateNumber);
   map.insertKeyAndValue(updateNumberKey, updateNumberValue);
}

const UtlString* RegistrationBinding::getIdentity() const
{
   return mIdentity;
}

void RegistrationBinding::setIdentity(const UtlString& identity)
{
   if (mIdentity)
   {
      *mIdentity = identity;
   }
   else
   {
      mIdentity = new UtlString(identity);
   }
}

const Url* RegistrationBinding::getUri() const
{
   return mUri;
}

void RegistrationBinding::setUri(const Url& uri)
{
   if (mUri)
   {
      *mUri = uri;
   }
   else
   {
      mUri = new Url(uri);
   }
}
void RegistrationBinding::setUri(const char* uri)
{
   if (mUri)
   {
      *mUri = uri;              // name-addr format
   }
   else
   {
      mUri = new Url(uri);      // name-addr format
   }
}

const UtlString* RegistrationBinding::getCallId() const
{
   return mCallId;
}
void RegistrationBinding::setCallId(const UtlString& callId)
{
   if (mCallId)
   {
      *mCallId = callId;
   }
   else
   {
      mCallId = new UtlString(callId);
   }
}

const UtlString* RegistrationBinding::getContact() const
{
   return mContact;
}
void RegistrationBinding::setContact(const UtlString& contact)
{
   if (mContact)
   {
      *mContact = contact;
   }
   else
   {
      mContact = new UtlString(contact);
   }
}

const UtlString* RegistrationBinding::getQvalue() const
{
   return mQvalue;
}
void RegistrationBinding::setQvalue(const UtlString& qvalue)
{
   if (mQvalue)
   {
      *mQvalue = qvalue;
   }
   else
   {
      mQvalue = new UtlString(qvalue);
   }
}

const UtlString* RegistrationBinding::getInstanceId() const
{
   return mInstanceId;
}
void RegistrationBinding::setInstanceId(const UtlString& instanceId)
{
   if (mInstanceId)
   {
      *mInstanceId = instanceId;
   }
   else
   {
      mInstanceId = new UtlString(instanceId);
   }
}

const UtlString* RegistrationBinding::getGruu() const
{
   return mGruu;
}
void RegistrationBinding::setGruu(const UtlString& gruu)
{
   if (mGruu)
   {
      *mGruu = gruu;
   }
   else
   {
      mGruu = new UtlString(gruu);
   }
}

const UtlString* RegistrationBinding::getPath() const
{
   return mPath;
}

void RegistrationBinding::setPath(const UtlString& path)
{
   if (mPath)
   {
      *mPath = path;
   }
   else
   {
      mPath = new UtlString(path);
   }
}

int RegistrationBinding::getCseq() const
{
   return mCseq;
}
void RegistrationBinding::setCseq(int cseq)
{
   mCseq = cseq;
}
void RegistrationBinding::setCseq(const UtlString& cseq)
{
   mCseq = atoi(cseq);
}

int RegistrationBinding::getExpires() const
{
   return mExpires;
}
void RegistrationBinding::setExpires(int expires)
{
   mExpires = expires;
}
void RegistrationBinding::setExpires(const UtlString& expires)
{
   mExpires = atoi(expires);
}

const UtlString* RegistrationBinding::getPrimary() const
{
   return mPrimary;
}
void RegistrationBinding::setPrimary(const UtlString& primary)
{
   if (mPrimary)
   {
      *mPrimary = primary;
   }
   else
   {
      mPrimary = new UtlString(primary);
   }
}

Int64 RegistrationBinding::getUpdateNumber() const
{
   return mUpdateNumber;
}
void RegistrationBinding::setUpdateNumber(Int64 updateNumber)
{
   mUpdateNumber = updateNumber;
}
void RegistrationBinding::setUpdateNumber(const UtlString& updateNumber)
{
   mUpdateNumber = strtoll(updateNumber, 0, 0);
}

const UtlString* RegistrationBinding::getInstrument() const
{
   return mInstrument;
}
void RegistrationBinding::setInstrument(const UtlString& instrument)
{
   if (mInstrument)
   {
      *mInstrument = instrument;
   }
   else
   {
      mInstrument = new UtlString(instrument);
   }
}

UtlContainableType RegistrationBinding::getContainableType() const
{
   return RegistrationBinding::TYPE;
}

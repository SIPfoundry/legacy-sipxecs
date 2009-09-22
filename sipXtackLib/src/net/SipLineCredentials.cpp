//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SipLineCredentials.cpp: implementation of the SipLineCredentials class.
//
//////////////////////////////////////////////////////////////////////

#include <net/SipLineCredentials.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SipLineCredentials::SipLineCredentials( const UtlString realm,
                                        const UtlString userId,
                                        const UtlString passwordToken,
                                        const UtlString type)
:UtlString(realm)
{

        mType = type;
        mPasswordToken = passwordToken;
        mUserId = userId;
        mRealm = realm;
}

SipLineCredentials::~SipLineCredentials()
{

}

void SipLineCredentials::getRealm(UtlString* realm)
{
        realm->remove(0);
        realm->append(mRealm);

}

void SipLineCredentials::getUserId(UtlString* UserId)
{
        UserId->remove(0);
        UserId->append(mUserId);
}

void SipLineCredentials::getPasswordToken(UtlString* passToken)
{
        passToken->remove(0);
        passToken->append(mPasswordToken);
}
void SipLineCredentials::getType(UtlString* type)
{
        type->remove(0);
        type->append(mType);
}

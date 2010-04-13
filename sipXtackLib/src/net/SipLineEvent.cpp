//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SipLineEvent.cpp: implementation of the SipLineEvent class.
//
//////////////////////////////////////////////////////////////////////

#include <net/SipLineEvent.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SipLineEvent::SipLineEvent(SipLine* Line,
                           int msgType,
                           UtlString realm,
                           UtlString scheme,
                           int SipReturnCode,
                           UtlString SipReturnText,
                           void* observerData)
:OsMsg(OsMsg::LINE_MGR_MSG, SipLineEvent::SIP_LINE_EVENT_FAILED)
{
        if (Line)
                mLine = *Line;
   mRealm.remove(0);
   mScheme.remove(0);
   mRealm = realm;
   mScheme = scheme;
   mSipReturnText = SipReturnText;
   mSipReturnCode = SipReturnCode;
   mMessageType = msgType;
   mObserverData = observerData;
}

SipLineEvent::~SipLineEvent()
{

}
SipLine* SipLineEvent::getLine()
{
        return &mLine;
}

UtlString& SipLineEvent::getSipReturnText()
{
   return mSipReturnText;
}

int SipLineEvent::getSipReturnCode()
{
        return mSipReturnCode;
}

UtlString& SipLineEvent::getRealm()
{
        return mRealm;
}
UtlString& SipLineEvent::getScheme()
{
        return mScheme;
}
void SipLineEvent::setMessageType(int status)
{
        mMessageType= status;
}

int SipLineEvent::getMessageType() const
{
        return(mMessageType);
}

void SipLineEvent::setObserverData(void* pData)
{
   mObserverData = pData;
}

void* SipLineEvent::getObserverData()
{
   return mObserverData;
}

SipLineEvent& SipLineEvent::operator=(const SipLineEvent& rhs)
{
    if (this == &rhs)            // handle the assignment to self case
        return *this;

   mLine = rhs.mLine ;
   mRealm = rhs.mRealm ;
   mScheme = rhs.mScheme ;
   mSipReturnText = rhs.mSipReturnText;
   mSipReturnCode = rhs.mSipReturnCode ;
   mMessageType = rhs.mMessageType ;
   mObserverData = rhs.mObserverData ;
   return *this;
}

OsMsg* SipLineEvent::createCopy(void) const
{
    SipLine line = mLine ;
    return new SipLineEvent(&line, mMessageType, mRealm, mScheme,
            mSipReturnCode, mSipReturnText, mObserverData) ;
}

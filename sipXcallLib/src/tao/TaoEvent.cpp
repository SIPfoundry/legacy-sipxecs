//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifdef TEST
#include "utl/UtlMemCheck.h"
#endif

#include "os/OsEvent.h"
#include "tao/TaoEvent.h"

#ifdef TAO_DEBUG
#include <assert.h>
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TaoEvent::TaoEvent(const void* userData)
        :OsEvent((void*)userData)
{
        mIntData = 0;
        mIntData2 = 0;
        mpMutex = NULL;
#ifdef TAO_DEBUG
        mWaits = 0;
#endif
}

TaoEvent::~TaoEvent()
{
}

void TaoEvent::setStringData(UtlString& rStringData)
{
        if (!rStringData.isNull())
        {
                mStringData.remove(0);
                mStringData.append(rStringData.data());
        }
}

void TaoEvent::setIntData(int data)
{
        mIntData = data;
}

void TaoEvent::setIntData2(int data)
{
        mIntData2 = data;
}

void TaoEvent::setMutex(OsMutex* pMutex)
{
        mpMutex = pMutex;
}

OsStatus TaoEvent::reset(void)
{
   OsStatus res;

   res = OsEvent::reset();
   mStringData.remove(0);

   return res;
}

// Wait for the event to be signaled.
// Return OS_BUSY if the timeout expired, otherwise return OS_SUCCESS.
OsStatus TaoEvent::wait(int msgId, const OsTime& rTimeout)
{
        OsStatus res;

        mIntData2 = msgId;
        res = OsEvent::wait(rTimeout);
        if (res != OS_SUCCESS)
                signal(mIntData);
        return res;
}

TaoStatus TaoEvent::getStringData(UtlString& data)
{
        if (!mStringData.isNull())
        {
                data.remove(0);
                data.append(mStringData.data());
        }

        return TAO_SUCCESS;
}

TaoStatus TaoEvent::getIntData(int& data)
{
        data = mIntData;
        return TAO_SUCCESS;
}

TaoStatus TaoEvent::getIntData2(int& data)
{
        data = mIntData2;
        return TAO_SUCCESS;
}

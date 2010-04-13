//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#ifdef _WIN32
#include <windows.h>
#endif

// APPLICATION INCLUDES
#include "utl/UtlVoidPtr.h"
#include "utl/UtlInt.h"
#include "tapi/SipXHandleMap.h"
#include "utl/UtlHashMapIterator.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const SIPXHANDLE SIPXHANDLE_NULL = 0;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipXHandleMap::SipXHandleMap()
    : mLock(OsMutex::Q_FIFO)
    , mNextHandle(1)
{
}

// Destructor
SipXHandleMap::~SipXHandleMap()
{
}

/* ============================ MANIPULATORS ============================== */

bool SipXHandleMap::addHandleRef(SIPXHANDLE hHandle)
{
    bool bRC = false ;

    if (lock())
    {
        mLockCountHash.findValue(&UtlInt(hHandle));
        UtlInt* count = static_cast<UtlInt*>(mLockCountHash.findValue(&UtlInt(hHandle))) ;
        if (count == NULL)
        {
            mLockCountHash.insertKeyAndValue(new UtlInt(hHandle), new UtlInt(1));
        }
        else
        {
            count->setValue(count->getValue() + 1);
        }

        bRC = unlock() ;
    }

    return bRC ;
}

bool SipXHandleMap::releaseHandleRef(SIPXHANDLE hHandle)
{
    bool bRC = false ;

    if (lock())
    {
        UtlInt* pCount = static_cast<UtlInt*>(mLockCountHash.findValue(&UtlInt(hHandle))) ;
        if (pCount == NULL)
        {
            mLockCountHash.insertKeyAndValue(new UtlInt(hHandle), new UtlInt(0));
        }
        else
        {
            pCount->setValue(pCount->getValue() - 1);
        }
        bRC = unlock();
    }
    return bRC ;
}

bool SipXHandleMap::lock()
{
    return (mLock.acquire() == OS_SUCCESS);
}


bool SipXHandleMap::unlock()
{
    return (mLock.release() == OS_SUCCESS);
}


SIPXHANDLE SipXHandleMap::allocHandle(const void* pData)
{
    SIPXHANDLE rc = SIPXHANDLE_NULL;

    if (lock())
    {
        rc = mNextHandle++ ;
        insertKeyAndValue(new UtlInt(rc), new UtlVoidPtr((void*) pData)) ;
        addHandleRef(rc);
        unlock() ;
    }

    return rc;
}

const void* SipXHandleMap::findHandle(SIPXHANDLE handle)
{
    const void* pRC = NULL ;
    if (lock())
    {
        UtlInt key(handle) ;
        UtlVoidPtr* pValue ;

        pValue = (UtlVoidPtr*) findValue(&key) ;
        if (pValue != NULL)
        {
            pRC = pValue->getValue() ;
        }

        unlock() ;
    }

    return pRC ;
}


const void* SipXHandleMap::removeHandle(SIPXHANDLE handle)
{
    const void* pRC = NULL ;

    if (lock())
    {
        releaseHandleRef(handle);

        UtlInt* pCount = static_cast<UtlInt*>(mLockCountHash.findValue(&UtlInt(handle))) ;

        if (pCount == NULL || pCount->getValue() < 1)
        {
            UtlInt key(handle) ;
            UtlVoidPtr* pValue ;

            pValue = (UtlVoidPtr*) findValue(&key) ;
            if (pValue != NULL)
            {
                pRC = pValue->getValue() ;
                destroy(&key) ;
            }

            if (pCount)
            {
                mLockCountHash.destroy(&UtlInt(handle));
            }
        }

        unlock() ;
    }

    return pRC ;
}


/* ============================ ACCESSORS ================================= */

void SipXHandleMap::dump()
{
    UtlHashMapIterator itor(*this) ;
    UtlInt* pKey ;
    UtlVoidPtr* pValue ;

    while ((pKey = (UtlInt*) itor()))
    {
        pValue = (UtlVoidPtr*) findValue(pKey) ;
        printf("\tkey=%" PRIdPTR ", value=%p\n", pKey->getValue(),
                pValue ? pValue->getValue() : 0) ;
    }
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

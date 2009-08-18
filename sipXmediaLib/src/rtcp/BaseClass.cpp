//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


//  Includes
#include "rtcp/BaseClass.h"
// #include <assert.h>
#include "os/OsBSem.h"
#include "os/OsLock.h"

//  Use a lock to protect access to the reference count
OsBSem sMultiThreadLock(OsBSem::Q_PRIORITY, OsBSem::FULL);

/**
 *
 * Method Name:  AddRef
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long
 *
 * Description: Increments the number of references to this object.
 *
 * Usage Notes:
 *
 *
 */
unsigned long CBaseClass::AddRef(void)
{

    OsLock lock(sMultiThreadLock);

    //  Increment Reference Count
    m_ulReferences++;
    sulTotalReferenceCount++;
    return(m_ulReferences);

}


/**
 *
 * Method Name:  Release
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long
 *
 * Description: Decrements the number of references to this object.
 *
 * Usage Notes:
 *
 *
 */
unsigned long CBaseClass::Release(void)
{

    OsLock lock(sMultiThreadLock);

    //  Decrement Reference Count
    sulTotalReferenceCount--;
    m_ulReferences--;

    return(m_ulReferences);

}

#ifdef __pingtel_on_posix__

/* See below for an explanation of these - they're implemented in terms of
* the OS abstraction layer under POSIX (as should the vxWorks ones, but they
* don't yet). Really, the WIN32 version ought to as well, but it seems that
* we've just defined this particular bit of code in terms of that API so
* there's no need to reinvent the wheel there. */

void InitializeCriticalSection(CRITICAL_SECTION *csSynchronized)
{
//  Create an OS abstraction layer binary semaphore
    *csSynchronized = new OsBSem(OsBSem::Q_PRIORITY, OsBSem::FULL);
}

void EnterCriticalSection(CRITICAL_SECTION *csSynchronized)
{
//  Attempt to gain exclusive access to a protected resource by taking
//  its semaphore
    if(*csSynchronized)
        (*csSynchronized)->acquire();
}

void LeaveCriticalSection(CRITICAL_SECTION *csSynchronized)
{
//  Attempt to relinquish exclusive access to a protected resource by giving
//  back its semaphore
    if(*csSynchronized)
        (*csSynchronized)->release();
}

void DeleteCriticalSection(CRITICAL_SECTION *csSynchronized)
{
//  Free all blocking
    if(*csSynchronized)
        delete *csSynchronized;
    *csSynchronized = NULL;
}
#endif /* __pingtel_on_posix__ ] */

#ifdef _VXWORKS /* [ */

/*|><|************************************************************************
*
* Function Name: InitializeCriticalSection
*
*
* Inputs:       CRITICAL_SECTION *csSynchronized
*
* Outputs:      None
*
* Returns:      None
*
* Description:  This is a compatability function that maps the creation of a
*               vxWorks semaphore to a complementary call under Win32.
*
* Usage Notes:
*
*
*************************************************************************|<>|*/
void InitializeCriticalSection(CRITICAL_SECTION *csSynchronized)
{

//  Create a binary vxWorks Semaphore
    *csSynchronized = semBCreate(SEM_Q_FIFO, SEM_FULL);

}

/*|><|*************************************************************************
*
* Function Name: EnterCriticalSection
*
*
* Inputs:       CRITICAL_SECTION *csSynchronized
*
* Outputs:      None
*
* Returns:      None
*
* Description:  This is a compatability function that maps the creation of a
*               vxWorks semaphore to a complementary call under Win32.
*
* Usage Notes:
*
*
*************************************************************************|<>|*/
void EnterCriticalSection(CRITICAL_SECTION *csSynchronized)
{

//  Attempt to gain exclusive access to a protected resource by taking
//  its semaphore
    if(*csSynchronized)
        semTake(*csSynchronized, WAIT_FOREVER);

}

/*|><|*************************************************************************
*
* Function Name: LeaveCriticalSection
*
*
* Inputs:       CRITICAL_SECTION *csSynchronized
*
* Outputs:      None
*
* Returns:      None
*
* Description:  This is a compatability function that maps the creation of a
*               vxWorks semaphore to a complementary call under Win32.
*
* Usage Notes:
*
*
*************************************************************************|<>|*/
void LeaveCriticalSection(CRITICAL_SECTION *csSynchronized)
{

//  Attempt to relinquish exclusive access to a protected resource by giving
//  back its semaphore
    if(*csSynchronized)
        semGive(*csSynchronized);

}
/*|><|*************************************************************************
*
* Function Name: DeleteCriticalSection
*
*
* Inputs:       CRITICAL_SECTION *csSynchronized
*
* Outputs:      None
*
* Returns:      None
*
* Description:  This is a compatability function that maps the creation of a
*               vxWorks semaphore to a complementary call under Win32.
*
* Usage Notes:
*
*
*************************************************************************|<>|*/
void DeleteCriticalSection(CRITICAL_SECTION *csSynchronized)
{

//  Free all blocking
    if(*csSynchronized)
    {
        semFlush(*csSynchronized);
        semDelete(*csSynchronized);
    }

}
#endif /* _VXWORKS ] */

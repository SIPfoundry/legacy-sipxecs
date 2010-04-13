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

// APPLICATION INCLUDES
#include "net/SipContactDb.h"
#include "utl/UtlInt.h"
#include "utl/UtlVoidPtr.h"
#include "os/OsLock.h"
#include "utl/UtlHashMapIterator.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipContactDb::SipContactDb() :
    mNextContactId(1),
    mLock(OsMutex::Q_FIFO)
{
}

// Copy constructor
SipContactDb::SipContactDb(const SipContactDb& SipContactDb) :
    mLock(OsMutex::Q_FIFO)
{
}

// Destructor
SipContactDb::~SipContactDb()
{
    UtlHashMapIterator iterator(mContacts);

    UtlInt* pKey = NULL;
    while ((pKey = dynamic_cast<UtlInt*>(iterator())))
    {
        UtlVoidPtr* pValue = dynamic_cast<UtlVoidPtr*>(iterator.value());
        if (pValue)
        {
           delete (ContactAddress*) pValue->getValue();
        }
    }
    mContacts.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipContactDb&
SipContactDb::operator=(const SipContactDb& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

const bool SipContactDb::addContact(ContactAddress& contact)
{
    OsLock lock(mLock);
    bool bRet = false;

    assert (contact.id < 1);

    if (!isDuplicate(contact.cIpAddress, contact.iPort))
    {
        assignContactId(contact);

        ContactAddress* pContactCopy = new ContactAddress(contact);
        mContacts.insertKeyAndValue(new UtlInt(pContactCopy->id),
                                    new UtlVoidPtr(pContactCopy));
        bRet = true;
    }
    else
    {
        // fill out the information in the contact,
        // to match what is already in the database
        contact = *(find(contact.cIpAddress, contact.iPort));
    }
    return bRet;
}

const bool SipContactDb::deleteContact(const ContactId id)
{
    OsLock lock(mLock);
    UtlInt idKey(id);
    return mContacts.destroy(&idKey);
}

ContactAddress* SipContactDb::find(ContactId id)
{
    OsLock lock(mLock);
    ContactAddress* pContact = NULL;
    UtlInt idKey(id);

    UtlVoidPtr* pValue = (UtlVoidPtr*)mContacts.findValue(&idKey);
    if (pValue)
    {
        pContact = (ContactAddress*)pValue->getValue();
    }

    return pContact;
}


// Finds the first contact by a given contact type
ContactAddress* SipContactDb::findByType(ContactType type)
{
    OsLock lock(mLock);
    UtlHashMapIterator iterator(mContacts);
    ContactAddress* pRC = NULL ;

    UtlVoidPtr* pValue = NULL;
    ContactAddress* pContact = NULL;
    UtlInt* pKey;
    while ((pKey = dynamic_cast<UtlInt*>(iterator())))
    {
        pValue = dynamic_cast<UtlVoidPtr*>(mContacts.findValue(pKey));
        assert(pValue);

        pContact = (ContactAddress*)pValue->getValue();
        assert(pContact) ;
        if (pContact->eContactType == type)
        {
            pRC = pContact ;
            break ;
        }
    }

    return pRC ;
}

// Find the local contact from a contact id
ContactAddress* SipContactDb::getLocalContact(ContactId id)
{
    OsLock lock(mLock);

    ContactAddress* pRC = NULL;
    ContactAddress* pOriginal = find(id);
    if (pOriginal)
    {
        if (pOriginal->eContactType == ContactAddress::LOCAL)
        {
            pRC = pOriginal;
        }
        else
        {
            UtlHashMapIterator iterator(mContacts);
            UtlVoidPtr* pValue = NULL;
            ContactAddress* pContact = NULL;
            UtlInt* pKey;
            while ((pKey = dynamic_cast<UtlInt*>(iterator())))
            {
	        pValue = dynamic_cast<UtlVoidPtr*>(mContacts.findValue(pKey));
                assert(pValue);

                pContact = (ContactAddress*)pValue->getValue();
                assert(pContact) ;
                if ((strcmp(pContact->cInterface, pOriginal->cInterface) == 0) &&
                    (pContact->eContactType == ContactAddress::LOCAL))
                {
                    pRC = pContact ;
                    break ;
                }
            }
        }
    }

    return pRC ;
}


ContactAddress* SipContactDb::find(const UtlString ipAddress, const int port)
{
    OsLock lock(mLock);
    bool bFound = false;
    UtlHashMapIterator iterator(mContacts);

    UtlVoidPtr* pValue = NULL;
    ContactAddress* pContact = NULL;
    UtlInt* pKey;
    while ((pKey = dynamic_cast<UtlInt*>(iterator())))
    {
        pValue = dynamic_cast<UtlVoidPtr*>(mContacts.findValue(pKey));
        assert(pValue);

        pContact = (ContactAddress*)pValue->getValue();
        if (strcmp(pContact->cIpAddress, ipAddress.data()) == 0)
        {
            if (port < 0 || port == pContact->iPort)
            {
                bFound = true;
                break;
            }
        }
    }

    if (!bFound)
    {
        pContact = NULL;
    }

    return pContact;
}

void SipContactDb::getAll(ContactAddress* contacts[], int& actualNum) const
{

    OsLock lock(mLock);
    UtlHashMapIterator iterator(mContacts);

    UtlVoidPtr* pValue = NULL;
    ContactAddress* pContact = NULL;
    UtlInt* pKey;
    actualNum = 0; // array index
    while ((pKey = dynamic_cast<UtlInt*>(iterator())))
    {
        pValue = dynamic_cast<UtlVoidPtr*>(mContacts.findValue(pKey));
        assert(pValue);

        pContact = (ContactAddress*)pValue->getValue();
        contacts[actualNum] = pContact;
        actualNum++;
    }

    return;
}
const bool SipContactDb::getRecordForAdapter(ContactAddress& contact,
                                             const char* szAdapter,
                                             const ContactType typeFilter) const
{
    bool bRet = false;

    OsLock lock(mLock);
    UtlHashMapIterator iterator(mContacts);

    UtlVoidPtr* pValue = NULL;
    ContactAddress* pContact = NULL;
    UtlInt* pKey;
    while ((pKey = dynamic_cast<UtlInt*>(iterator())))
    {
        pValue = dynamic_cast<UtlVoidPtr*>(mContacts.findValue(pKey));
        assert(pValue);

        pContact = (ContactAddress*)pValue->getValue();

        if (0 != strcmp(pContact->cInterface, szAdapter))
        {
            continue;
        }
        if (pContact->eContactType != typeFilter)
        {
            continue;
        }

        contact = *pContact;
        bRet = true;
        break;
    }
    return bRet;
}

void SipContactDb::getAllForAdapter(const ContactAddress* contacts[],
                                    const char* szAdapter,
                                    int& actualNum,
                                    const ContactType typeFilter) const
{

    OsLock lock(mLock);
    UtlHashMapIterator iterator(mContacts);

    UtlVoidPtr* pValue = NULL;
    ContactAddress* pContact = NULL;
    UtlInt* pKey;
    actualNum = 0; // array index
    while ((pKey = dynamic_cast<UtlInt*>(iterator())))
    {
        pValue = dynamic_cast<UtlVoidPtr*>(mContacts.findValue(pKey));
        assert(pValue);

        pContact = (ContactAddress*)pValue->getValue();

        if (0 != strcmp(pContact->cInterface, szAdapter))
        {
            continue;
        }
        if (typeFilter != ContactAddress::ALL &&
            pContact->eContactType != typeFilter)
        {
            continue;
        }

        contacts[actualNum] = pContact;
        actualNum++;
    }

    return;
}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */



/* //////////////////////////// PROTECTED ///////////////////////////////// */


/* //////////////////////////// PRIVATE /////////////////////////////////// */

const bool SipContactDb::isDuplicate(const ContactId id)
{
    OsLock lock(mLock);
    bool bRet = false;
    UtlInt idKey(id);

    UtlVoidPtr* pValue = (UtlVoidPtr*)mContacts.findValue(&idKey);
    if (pValue)
    {
        bRet = true;
    }
    return bRet;
}

const bool SipContactDb::isDuplicate(const UtlString& ipAddress, const int port)
{
    OsLock lock(mLock);
    bool bRet = false;
    UtlHashMapIterator iterator(mContacts);

    UtlVoidPtr* pValue = NULL;
    ContactAddress* pContact = NULL;
    UtlInt* pKey;
    while ((pKey = dynamic_cast<UtlInt*>(iterator())))
    {
        pValue = dynamic_cast<UtlVoidPtr*>(mContacts.findValue(pKey));
        assert(pValue);

        pContact = (ContactAddress*)pValue->getValue();
        if (strcmp(pContact->cIpAddress, ipAddress.data()) == 0)
        {
            if (port < 0 || port == pContact->iPort)
            {
                bRet = true;
                break;
            }
        }
    }
    return bRet;
}

const bool SipContactDb::assignContactId(ContactAddress& contact)
{
    OsLock lock(mLock);

    contact.id = mNextContactId;
    mNextContactId++;

    return true;
}

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

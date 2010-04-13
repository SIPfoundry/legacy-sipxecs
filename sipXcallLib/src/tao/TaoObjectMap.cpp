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

#include <utl/UtlInt.h>
#include <utl/UtlHashMap.h>
#include <utl/UtlHashMapIterator.h>

#include "tao/TaoObjectMap.h"


////////////////////////////
// for TaoListenerDb class

TaoListenerDb::TaoListenerDb()
{
    mName.remove(0);
    mRef = 0;
    mpListenerPtr = 0;
    mId = -10;
    mIntData = 0;
}

TaoListenerDb::~TaoListenerDb()
{
    mRef = 0;
    mId = -10;
    mIntData = 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TaoObjectMap::TaoObjectMap(int initialDbSize)
:  mDict(), mNumInserts(0), mNumRemoves(0)
{
   initialDbSize = initialDbSize;
}

//:Copy constructor
TaoObjectMap::TaoObjectMap(const TaoObjectMap& rTaoObjectMap)
{

    mNumInserts = rTaoObjectMap.mNumInserts;
    mNumRemoves = rTaoObjectMap.mNumRemoves;
    rTaoObjectMap.mDict.copyInto(mDict);
    UtlHashMapIterator iter(mDict);
    UtlContainable*    next;
    UtlInt* key;
    UtlInt* value;

    iter.reset();
    while ((next = iter()))
    {
        key   = (UtlInt*) iter.key();
        value = (UtlInt*) iter.value();
        (UtlInt*) mDict.insertKeyAndValue(key, value);
    }

}

//:Assignment operator
TaoObjectMap& TaoObjectMap::operator=(const TaoObjectMap& rhs)
{
    if (this == &rhs)            // handle the assignment to self case
      return *this;

    mNumInserts = rhs.mNumInserts;
    mNumRemoves = rhs.mNumRemoves;
    rhs.mDict.copyInto(mDict);
    UtlHashMapIterator iter(mDict);
    UtlContainable*    next;
    UtlInt* key;
    UtlInt* value;

    iter.reset();
    while ((next = iter()))
    {
        key   = (UtlInt*) iter.key();
        value = (UtlInt*) iter.value();
        (UtlInt*) mDict.insertKeyAndValue(key, value);
    }
    return *this;
}


TaoObjectMap::~TaoObjectMap()
{
    mDict.destroyAll();
/*   UtlHashMapIterator iter(mDict);
   UtlContainable*    next;
   UtlInt* key;
   UtlInt* value;

   iter.reset();
   while (next = iter())
   {
      key   = (UtlInt*) iter.key();
      value = (UtlInt*) iter.value();
      iter.remove();
      delete key;
      delete value;
      iter.reset();
   }
*/
}

//////////////////////////////////////////////////////////////////////
// MANIPULATORS
//////////////////////////////////////////////////////////////////////

TaoStatus TaoObjectMap::insert(TaoObjHandle objId, TaoMessage::TaoMsgTypes objValue)
{
   UtlInt* pDictKey;
   UtlInt* pDictValue;
   UtlInt* pInsertedKey;

   pDictKey   = new UtlInt(objId);
   pDictValue = new UtlInt(objValue);

   pInsertedKey = (UtlInt*)
                  mDict.insertKeyAndValue(pDictKey, pDictValue);

   if (pInsertedKey == NULL)
   {                             // insert failed
      delete pDictKey;           // clean up the key and value objects
      delete pDictValue;

      return TAO_IN_USE;
   }
   else
   {
      mNumInserts++;
      return TAO_SUCCESS;
   }
}

TaoStatus TaoObjectMap::insert(TaoObjHandle objId, TaoObjHandle objValue)
{
   UtlInt* pDictKey;
   UtlInt* pDictValue;
   UtlInt* pInsertedKey;

   pDictKey   = new UtlInt(objId);
   pDictValue = new UtlInt(objValue);

   pInsertedKey = (UtlInt*)
                  mDict.insertKeyAndValue(pDictKey, pDictValue);

   if (pInsertedKey == NULL)
   {                             // insert failed
      delete pDictKey;           // clean up the key and value objects
      delete pDictValue;

      return TAO_IN_USE;
   }
   else
   {
      mNumInserts++;
      return TAO_SUCCESS;
   }
    return TAO_SUCCESS;
}

TaoStatus TaoObjectMap::insert(TaoObjHandle objId, UtlString objValue)
{
   UtlInt* pDictKey;
   UtlString* pDictValue;
   UtlInt* pInsertedKey;

   pDictKey   = new UtlInt(objId);
   pDictValue = new UtlString(objValue);

   pInsertedKey = (UtlInt*)
                  mDict.insertKeyAndValue(pDictKey, pDictValue);

   if (pInsertedKey == NULL)
   {                             // insert failed
      delete pDictKey;           // clean up the key and value objects
      delete pDictValue;

      return TAO_IN_USE;
   }
   else
   {
      mNumInserts++;
      return TAO_SUCCESS;
   }
    return TAO_SUCCESS;
}

TaoStatus TaoObjectMap::insert(const char* key, TaoObjHandle objValue)
{
   UtlString* pDictKey;
   UtlInt* pDictValue;
   UtlInt* pInsertedKey;

   pDictKey   = new UtlString(key);
   pDictValue = new UtlInt(objValue);

   pInsertedKey = (UtlInt*)
                  mDict.insertKeyAndValue(pDictKey, pDictValue);

   if (pInsertedKey == NULL)
   {                             // insert failed
      delete pDictKey;           // clean up the key and value objects
      delete pDictValue;

      return TAO_IN_USE;
   }
   else
   {
      mNumInserts++;
      return TAO_SUCCESS;
   }
    return TAO_SUCCESS;
}

TaoStatus TaoObjectMap::remove(const char* key)
{
   UtlString* pLookupKey;
   UtlInt* pDictKey;
   UtlInt* pDictValue;

   pLookupKey = new UtlString(key);
   pDictKey   = (UtlInt*)
                mDict.removeKeyAndValue(pLookupKey,
                                        (UtlContainable*&) pDictValue);
   delete pLookupKey;

   if (pDictKey == NULL)
      return TAO_NOT_FOUND;   // did not find the specified key
   else
      mNumRemoves++;

   delete pDictKey;          // before returning we need to destroy the
   delete pDictValue;        //  objects that were used to maintain the
                             //  dictionary entry
   return TAO_SUCCESS;
}

TaoStatus TaoObjectMap::remove(TaoObjHandle objId)
{
   UtlInt* pLookupKey;
   UtlInt* pDictKey;
   UtlInt* pDictValue;

   pLookupKey = new UtlInt(objId);
   pDictKey   = (UtlInt*)
                mDict.removeKeyAndValue(pLookupKey,
                                        (UtlContainable*&) pDictValue);
   delete pLookupKey;

   if (pDictKey == NULL)
      return TAO_NOT_FOUND;   // did not find the specified key
   else
      mNumRemoves++;

   delete pDictKey;          // before returning we need to destroy the
   delete pDictValue;        //  objects that were used to maintain the
                             //  dictionary entry
   return TAO_SUCCESS;
}


//////////////////////////////////////////////////////////////////////
// ACCESSORS
//////////////////////////////////////////////////////////////////////

TaoStatus TaoObjectMap::findValue(TaoObjHandle objId, TaoObjHandle& objValue)
{
    UtlInt* pLookupKey;
    UtlInt*    pDictValue;

    pLookupKey = new UtlInt(objId);
    pDictValue = (UtlInt*)
                mDict.findValue(pLookupKey); // perform the lookup
    delete pLookupKey;

    if (pDictValue == NULL)
      return TAO_NOT_FOUND;   // did not find the specified key

    objValue = pDictValue->getValue();

    return TAO_SUCCESS;
}

TaoStatus TaoObjectMap::findValue(const char* key, TaoObjHandle& objValue)
{
    UtlString* pLookupKey;
    UtlInt*    pDictValue;

    pLookupKey = new UtlString(key);
    pDictValue = (UtlInt*)
                mDict.findValue(pLookupKey); // perform the lookup
    delete pLookupKey;

    if (pDictValue == NULL)
      return TAO_NOT_FOUND;   // did not find the specified key

    objValue = pDictValue->getValue();

    return TAO_SUCCESS;
}

TaoStatus TaoObjectMap::findValue(TaoObjHandle objId, UtlString& objValue)
{
   UtlInt* pDictKey;
   UtlString* pDictValue;

   pDictKey   = new UtlInt(objId);

    pDictValue = (UtlString*)
                mDict.findValue(pDictKey); // perform the lookup
    delete pDictKey;

    if (pDictValue == NULL)
      return TAO_NOT_FOUND;   // did not find the specified key

    objValue = *pDictValue;

    return TAO_SUCCESS;
}

UtlBoolean TaoObjectMap::findValue(TaoObjHandle value)
{
    UtlHashMapIterator iter(mDict);
    UtlContainable*    next;
    UtlInt* cvalue;

    iter.reset();
    while ((next = iter()))
    {
        cvalue = (UtlInt*) iter.value();
        if (value == (TaoObjHandle) cvalue->getValue())
        {
            return TRUE;
        }
    }

    return FALSE;
}

// Get an array of pointers to the TaoObjHandles that are currently active.
// The caller provides an array that can hold up to "size" TaoObjHandle
// pointers. This method will fill in the "active Objects" array with
// up to "size" TaoObjHandle. The method returns the number of TaoObjHandle
// in the array that were actually filled in.
int TaoObjectMap::getActiveObjects(TaoObjHandle activeObjects[], int size)
{
   UtlHashMapIterator iter(mDict);
   UtlContainable*    next;
   UtlInt* value;
   int      i;

   iter.reset();
   i = 0;
   while ((next = iter()))
   {
      if (i >= size) break;

      value = (UtlInt*) iter.value();
      activeObjects[i] = (TaoObjHandle) value->getValue();
      i++;
   }

   return i;
}

TaoStatus TaoObjectMap::removeByValue(TaoObjHandle value)
{
    UtlHashMapIterator iter(mDict);
    UtlContainable*    next;
    UtlInt* cvalue;
    TaoStatus status = TAO_NOT_FOUND;

    while ((next = iter()))
    {
        cvalue = (UtlInt*) iter.value();
        if (value == (TaoObjHandle) cvalue->getValue())
        {
            mDict.destroy(next);
            status = TAO_SUCCESS;
            mNumRemoves++;
//          osPrintf("<** %d removeByValue: cvalue 0x%p, next 0x%p **>\n", value, cvalue, next);
            break;
        }
    }

    return status;
}

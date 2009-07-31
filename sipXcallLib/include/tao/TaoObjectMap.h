//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _TaoObjectMap_h_
#define _TaoObjectMap_h_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// SYSTEM INCLUDES

// APPLICATION INCLUDES

#include <utl/UtlHashMap.h>

#include "ptapi/PtDefs.h"
#include "tao/TaoDefs.h"
#include "tao/TaoObject.h"
#include "tao/TaoMessage.h"

// DEFINES
//#define MAX_NUM_LISTENERS 20
#define MAX_NUM_TONE_LISTENERS 50
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class TaoListenerDb
{
public:
        TaoListenerDb();

        ~TaoListenerDb();

        UtlString       mName;
        void*           mpListenerPtr;
        int             mRef;
        int             mId;
        intptr_t        mIntData;
};

//:Maintains a db of TaoObjHandle to TaoObject or PTAPI object mappings.
class TaoObjectMap
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum TaoObjectDbInitSize { TAOOBJ_DB_INIT_SIZE = 256 };

/* ============================ CREATORS ================================== */

   TaoObjectMap(int initialDbSize = TAOOBJ_DB_INIT_SIZE);
     //:Default constructor

        TaoObjectMap(const TaoObjectMap& rTaoObjectMap);
         //:Copy constructor

        TaoObjectMap& operator=(const TaoObjectMap& rhs);
         //:Assignment operator

        virtual
   ~TaoObjectMap();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   TaoStatus insert(TaoObjHandle objId, TaoMessage::TaoMsgTypes objValue);
     //:Insert the indicated TaoObjHandle into the database of active TaoObjHandles.
     // Return TAO_SUCCESS if successful, TAO_EXISTS if the key is
     // already in the database.

   TaoStatus insert(TaoObjHandle objId, TaoObjTypes objValue);
     //:Insert the indicated TaoObjHandle into the database of active TaoObjHandles.
     // Return TAO_SUCCESS if successful, TAO_EXISTS if the key is
     // already in the database.

   TaoStatus insert(TaoObjHandle objId, TaoObjHandle objValue);
     //:Insert the indicated TaoObjHandle into the database of active TaoObjHandles.
     // Return TAO_SUCCESS if successful, TAO_EXISTS if the key is
     // already in the database.

   TaoStatus insert(const char* key, TaoObjHandle objValue);
     //:Insert the indicated TaoObjHandle into the database of active TaoObjHandles.
     // Return TAO_SUCCESS if successful, TAO_EXISTS if the key is
     // already in the database.

   TaoStatus insert(TaoObjHandle objId, UtlString objValue);
     //:Insert the indicated TaoObjHandle into the database of active TaoObjHandles.
     // Return TAO_SUCCESS if successful, TAO_EXISTS if the key is
     // already in the database.

   TaoStatus remove(TaoObjHandle objId);
     //:Remove the indicated TaoObjHandle from the database of active TaoObjHandles.
     // Return TAO_SUCCESS if the indicated TaoObjHandleId is found, return
     // TAO_NOT_FOUND if there is no match for the specified key.

   TaoStatus remove(const char* key);
     //:Remove the indicated TaoObjHandle from the database of active TaoObjHandles.
     // Return TAO_SUCCESS if the indicated TaoObjHandleId is found, return
     // TAO_NOT_FOUND if there is no match for the specified key.

        TaoStatus removeByValue(TaoObjHandle value);
     //:Find an active TaoObjHandle that has the value of the input,
         // then remove the indicated TaoObjHandle from the database.
     // Return TAO_SUCCESS if the indicated value is found, return
     // TAO_NOT_FOUND if there is no match for the specified value.

/* ============================ ACCESSORS ================================= */

   TaoStatus findValue(TaoObjHandle objId, TaoObjTypes& objValue);
     //:Finds the value associated with the objId key.
     // Return TAO_SUCCESS if successful, TAO_NOT_FOUND if the key is
     // not found in the database.

   TaoStatus findValue(TaoObjHandle objId, TaoObjHandle& objValue);
     //:Finds the value associated with the objId key.
     // Return TAO_SUCCESS if successful, TAO_NOT_FOUND if the key is
     // not found in the database.

   TaoStatus findValue(const char* key, TaoObjHandle& objValue);
     //:Finds the value associated with the objId key.
     // Return TAO_SUCCESS if successful, TAO_NOT_FOUND if the key is
     // not found in the database.

   TaoStatus findValue(TaoObjHandle key, UtlString& objValue);
     //:Finds the value associated with the objId key.
     // Return TAO_SUCCESS if successful, TAO_NOT_FOUND if the key is
     // not found in the database.

   UtlBoolean findValue(TaoObjHandle objValue);
     //:Check if the value exists.
     // Return TRUE if successful, FALSE if the value is
     // not found in the database.

   int getActiveObjects(TaoObjHandle activeObjects[], int size);
     //:Get an array of pointers to the TaoObjHandles that are currently active.
     // The caller provides an array that can hold up to <i>size</i> TaoObjHandles.
     // This method will fill in the <i>activeObjects</i> array with
     // up to <i>size</i> TaoObjHandles. The method returns the number of TaoObjHandles
     // in the array that were actually filled in.

   void getDbStats(unsigned& nInserts, unsigned& nRemoves) const;
     //:Get the number of insertions and removals for the database.

   int numEntries(void) const { return (mNumInserts - mNumRemoves); };
     //:Return the number of key-value pairs in the database.

/* ============================ INQUIRY =================================== */

   UtlBoolean isEmpty(void) const { return ((mNumInserts - mNumRemoves) ? TRUE : FALSE); };
     //:Return TRUE if the mObjDict database is empty.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        UtlHashMap mDict;       // hash table used to store the key/value

        unsigned mNumInserts;            // number of insertions into the database
        unsigned mNumRemoves;            // number of removals from the database



};

#endif // _TaoObjectMap_h_

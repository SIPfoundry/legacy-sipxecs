//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES


// APPLICATION INCLUDES
#include "utl/UtlHashMapIterator.h"
#include "sipdb/ResultSet.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

ResultSet::ResultSet()
{}

ResultSet::~ResultSet()
{
    // make sure we destroy the records
    // we allocated in the addValue method
    destroyAll();
}

int
ResultSet::getSize() const
{
    return entries();
}

void
ResultSet::destroyAll()
{
    // the pRecord is actually a UtlHashMap
    UtlHashMap* pRecord;
    while ((pRecord = dynamic_cast<UtlHashMap*>(get())))
    {
        pRecord->destroyAll();
        delete pRecord;
    }
}

void
ResultSet::clear()
{
   // Clearing the ResultSet is simple, as the superclass UtlSList is the
   // entirety of its state.
   destroyAll();
}

OsStatus
ResultSet::getIndex(
    const int& index,
    UtlHashMap& rRecord) const
{
    // The record must be empty.  We can't clear the content because we don't own it.
    assert(rRecord.isEmpty());

    OsStatus result = OS_FAILED;
    UtlHashMap *m;
    if ((m = dynamic_cast<UtlHashMap*>(at(index))))
    {
        m->copyInto(rRecord);
        result = OS_SUCCESS;
    }
    return result;
}

void
ResultSet::addValue( const UtlHashMap& record )
{
    UtlHashMap*     pNewRecord = new UtlHashMap() ;
    UtlContainable* pObj ;

    // Proceed with shallow copy
    UtlHashMapIterator itor(const_cast<UtlHashMap&>(record)) ;
    while ((pObj = (UtlContainable*) itor()) != NULL)
    {
        pNewRecord->insertKeyAndValue(itor.key(), itor.value()) ;
    }
    append(pNewRecord) ;
}

//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef DIALBYNAMEDB_H
#define DIALBYNAMEDB_H

// SYSTEM INCLUDES


// APPLICATION INCLUDES
#include "os/OsMutex.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class Url;
class DialByNameCursor;
class dbDatabase;
class dbFieldDescriptor;
class UtlHashMap;
class ResultSet;
class UtlSList;

/**
 * This class implements the DialByName database abstract class
 */
class DialByNameDB
{
public:
    /**
     * Singleton Accessor
     *
     * @return
     */
    static DialByNameDB* getInstance( const UtlString& name = "dialbyname" );

    /// releaseInstance - cleans up the singleton (for use at exit)
    static void releaseInstance();

    // Domain Serialization/Deserialization
    OsStatus store();

    /// Count rows in table
    int getRowCount () const;

    // utility method for dumping all rows
    void getAllRows ( ResultSet& rResultSet ) const;

    // Queries the IMDB for all contacts
    // associated with a set of DTMF digits
    // this method is non-const as it may reload the IMDB
    // from the credentials/aliases databases
    void getContacts (
        const UtlString& digitstring,
        ResultSet& rResultset ) const;

    // Determine if the table is loaded from xml file
    bool isLoaded();

protected:
    // Insert database rows (potentially multiple if > 2 words in display name)
    UtlBoolean insertRow ( const Url& contact ) const;

    // Delete methods (protcted for this DB).
    UtlBoolean removeRow (const Url& contact );

    // Flushes the entire DB (protcted for this DB).
    void removeAllRows () const;

    UtlBoolean getDigitStrings (
        const UtlString& displayName,
        UtlSList& dtmfStrings ) const;

    // implicit loader
    OsStatus load() const;

    // Singleton Constructor is private
    DialByNameDB ( const UtlString& name );

    // One step closer to common load/store code
    UtlBoolean insertRow ( const UtlHashMap& nvPairs ) const;

    // There is only one singleton in this design
    static DialByNameDB* spInstance;

    // Singleton and Serialization mutex
    static OsMutex sLockMutex;

    // ResultSet column Keys
    static UtlString gNp_identityKey;
    static UtlString gNp_contactKey;
    static UtlString gNp_digitsKey;

    // Fast DB instance
    dbDatabase* m_pFastDB;

    // the persistent filename for loading/saving
    UtlString mDatabaseName;

    // boolean indicating table is loaded
    bool mTableLoaded;

private:
    /**
     * Virtual Destructor
     */
    virtual ~DialByNameDB();

};

#endif //DIALBYNAMEDB_H

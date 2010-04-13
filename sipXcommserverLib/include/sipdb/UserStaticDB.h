// 
// 
// Copyright (C) 2009 Nortel Networks, certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef USERSTATICDB_H
#define USERSTATICDB_H

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
class dbDatabase;
class dbFieldDescriptor;
class UtlHashMap;
class TiXmlNode;
class ResultSet;

/**
 * This class implements the UserStatic database abstract class
 */
class UserStaticDB
{
public:
    /**
     * Singleton Accessor
     *
     * @return
     */
    static UserStaticDB* getInstance( const UtlString& name = "userstatic" );

    /// releaseInstance - cleans up the singleton (for use at exit)
    static void releaseInstance();

    // Domain Serialization/Deserialization
    OsStatus store();

    /// Count rows in table
    int getRowCount () const;

    // One step closer to common load/store code
    UtlBoolean insertRow ( const UtlHashMap& nvPairs );

    // Insert or update a row in the UserStatices database.
    UtlBoolean insertRow (
        const Url& identity,
        const UtlString& event,
        const UtlString& contact,
        const UtlString& from_uri,
        const UtlString& to_uri,
        const UtlString& callid );

    // Delete methods
    UtlBoolean removeRow (const Url& identity );

    // Flushes the entire DB
    void removeAllRows ();

    // utility method for dumping all rows
    void getAllRows(ResultSet& rResultSet) const;

    // Get the static contact information for the user uri
    UtlBoolean getStaticContact (
        const UtlString& identityStr,
        const UtlString& eventStr,
        UtlString& contact,
        UtlString& from_uri,
        UtlString& to_uri,
        UtlString& callid) const;

    // The 'type' attribute of the top-level 'items' element.
    static const UtlString sType;

    // The XML namespace of the top-level 'items' element.
    static const UtlString sXmlNamespace;

    // Determine if the table is loaded from xml file.
    bool isLoaded();

protected:
    // implicit loader
    OsStatus load();

    // Singleton Constructor is private
    UserStaticDB ( const UtlString& name );

    // There is only one singleton in this design
    static UserStaticDB* spInstance;

    // Singleton and Serialization mutex
    static OsMutex sLockMutex;

    // ResultSet column Keys
    static UtlString gIdentityKey;
    static UtlString gEventKey;
    static UtlString gContactKey;
    static UtlString gFromUriKey;
    static UtlString gToUriKey;
    static UtlString gCallidKey;

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
    virtual ~UserStaticDB();

};

#endif //USERSTATICDB_H


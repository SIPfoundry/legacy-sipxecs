//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef ALIASDB_H
#define ALIASDB_H

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
 * This class implements the Alias database abstract class
 */
class AliasDB
{
public:
    /**
     * Singleton Accessor
     *
     * @return
     */
    static AliasDB* getInstance( const UtlString& name = "alias" );

    /// releaseInstance - cleans up the singleton (for use at exit)
    static void releaseInstance();

    // Domain Serialization/Deserialization
    OsStatus store();

    /// Count rows in table
    int getRowCount () const;

    // One step closer to common load/store code
    UtlBoolean insertRow ( const UtlHashMap& nvPairs );

    // Insert or update a row in the Aliases database.
    // Note: since because aliasIdentities can have multiple
    // contacts, we need to pass an update hint default false
    // parameter to the insert
    UtlBoolean insertRow (
        const Url& aliasIdentity,
        const Url& contact,
        bool updateContact = FALSE);

    // Delete methods
    UtlBoolean removeRow (const Url& aliasIdentity );

    // Flushes the entire DB
    void removeAllRows ();

    // utility method for dumping all rows
    void getAllRows(ResultSet& rResultSet) const;

    // Query interface to return a set of mapped full URI
    // contacts associated with the alias
    void getContacts (
        const Url& aliasIdentity,
        ResultSet& rResultSet) const;

    // Query interface to return aliases associated with a sipIdentity
    void getAliases (
        const Url& contactIdentity,
        ResultSet& rResultSet ) const;

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
    AliasDB ( const UtlString& name );

    // There is only one singleton in this design
    static AliasDB* spInstance;

    // Singleton and Serialization mutex
    static OsMutex sLockMutex;

    // ResultSet column Keys
    static UtlString gIdentityKey;
    static UtlString gContactKey;

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
    virtual ~AliasDB();

};

#endif //ALIASDB_H

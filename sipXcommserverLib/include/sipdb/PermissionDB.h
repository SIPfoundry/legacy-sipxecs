//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef PERMISSIONDB_H
#define PERMISSIONDB_H

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
class ResultSet;
class TiXmlNode;

/**
 * This class implements the Alias database abstract class
 */
class PermissionDB
{
public:
    /**
     * Singleton Accessor
     *
     * @return
     */
    static PermissionDB* getInstance( const UtlString& name = "permission" );

    /// releaseInstance - cleans up the singleton (for use at exit)
    static void releaseInstance();

    // Domain Serialization/Deserialization
    OsStatus store();

    /// Count rows in table
    int getRowCount () const;

    //set methods// values can be MAPPED or REGISTER
    UtlBoolean insertRow (
        const Url& uri,
        const UtlString& permission );

    UtlBoolean insertRow ( const UtlHashMap& nvPairs );

    // Query interface always used cursor now
    void getPermissions (
        const Url& identity,
        ResultSet& rResultset ) const;

    // Query the identities associated with a particular permission
    void getIdentities (
        const UtlString& permission,
        ResultSet& rResultset ) const;

    // Query top see if a this identity has the permission set
    UtlBoolean hasPermission (
        const Url& identity,
        const UtlString& permission ) const;

    // Delete methods
    UtlBoolean removeRow(
        const Url& identity,
        const UtlString& permission );

    void removeRows ( const Url& identity );

    // Flushes the entire DB
    void removeAllRows ();

    // utility method for dumping all rows
    void getAllRows ( ResultSet& rResultset) const;

    // The 'type' attribute of the top-level 'items' element.
    static const UtlString sType;

    // The XML namespace of the top-level 'items' element.
    static const UtlString sXmlNamespace;

    // Determine if the table is loaded from xml file
    bool isLoaded();

protected:
    // implicit loader
    OsStatus load();

    // Singleton Constructor is private
    PermissionDB ( const UtlString& name );

    // There is only one singleton in this design
    static PermissionDB* spInstance;

    // Singleton and Serialization mutex
    static OsMutex sLockMutex;

    // ResultSet column Keys
    static UtlString gIdentityKey;
    static UtlString gPermissionKey;

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
    virtual ~PermissionDB();

};

#endif //PERMISSIONDB_H

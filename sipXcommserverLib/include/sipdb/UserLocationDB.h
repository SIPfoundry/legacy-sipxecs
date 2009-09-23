// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef USERLOCATIONDB_H
#define USERLOCATIONDB_H

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
 * The user location database contains a set of rows, each one 
 * mapping a user identity to a location.  As an example, this
 * database is consulted by the Fallback redirect plug-in to  
 * provide location-based gateway selection.   
 */
class UserLocationDB
{
public:
    /**
     * Singleton Accessor
     *
     * @return
     */
    static UserLocationDB* getInstance( const UtlString& name = "userlocation" );

    /// releaseInstance - cleans up the singleton (for use at exit)
    static void releaseInstance();

    // Domain Serialization/Deserialization
    OsStatus store();

    /// Count rows in table
    int getRowCount () const;

    UtlBoolean insertRow (
        const Url& identityUri,
        const UtlString& location );

    UtlBoolean insertRow ( const UtlHashMap& nvPairs );

    // Get the location(s) associated with the specified user identity
    void getLocations (
        const UtlString& identityString,
        ResultSet& rResultset ) const;

    // Query the identities associated with a particular location
    void getIdentities (
        const UtlString& location,
        ResultSet& rResultset ) const;

    // Query to see if a an identity has a location set
    UtlBoolean hasLocation( const UtlString& identitystring ) const;

    // Delete methods
    UtlBoolean removeRow(
        const Url& identityUri,
        const UtlString& location );

    void removeRows ( const Url& identityUri );

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
    UserLocationDB ( const UtlString& name );

    // There is only one singleton in this design
    static UserLocationDB* spInstance;

    // Singleton and Serialization mutex
    static OsMutex sLockMutex;

    // ResultSet column Keys
    static UtlString gIdentityKey;
    static UtlString gLocationKey;

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
    virtual ~UserLocationDB();

};

#endif //USERLOCATIONDB_H

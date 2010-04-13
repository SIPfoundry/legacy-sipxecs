// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef LOCATIONDB_H
#define LOCATIONDB_H

// SYSTEM INCLUDES


// APPLICATION INCLUDES
#include "os/OsMutex.h"
#include "digitmaps/Patterns.h"

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
class dbQuery;
class dbFieldDescriptor;
class UtlHashMap;
class ResultSet;
class TiXmlNode;

/**
 * The location database is used to contain all the attributes of a location
 * as create by the administrator. 
 */
class LocationDB
{
public:
    /**
     * Singleton Accessor
     *
     * @return
     */
    static LocationDB* getInstance( const UtlString& name = "location" );

    /// releaseInstance - cleans up the singleton (for use at exit)
    static void releaseInstance();

    // Domain Serialization/Deserialization
    OsStatus store();

    /// Count rows in table
    int getRowCount () const;

    UtlBoolean insertRow (
        const UtlString& locationName,
        const UtlString& locationDescription,
        const UtlString& locationCode,
        const UtlString& locationSubnetsString); //comma-separated list of subnets, e.g. "10.0.0.1/8,20.0.0.0/24"

    UtlBoolean insertRow ( const UtlHashMap& nvPairs );

    // Delete methods
    UtlBoolean removeRow(
        const UtlString& locationName );

    UtlBoolean removeRows(
        const UtlString& locationName );

    // Flushes the entire DB
    void removeAllRows ();

    /// get an individual row based on a matching critiria.  The caller is responsible for 
    /// calling UtlHashMap::destroyAll() on the returned UtlHashMap when it is done with it. 
    UtlBoolean getRowByName( const UtlString& locationName, UtlHashMap& nvPairs ) const;
    UtlBoolean getRowByLocationCode( const UtlString& locationCode, UtlHashMap& nvPairs ) const;
    UtlBoolean getRowByIpAddress( const UtlString& ipAddress, UtlHashMap& nvPairs ) const;

    // utility method for dumping all rows
    void getAllRows( ResultSet& rResultset ) const;

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
    LocationDB ( const UtlString& name );

    // There is only one singleton in this design
    static LocationDB* spInstance;

    // Singleton and Serialization mutex
    static OsMutex sLockMutex;

    // ResultSet column Keys
    static UtlString gNameKey;
    static UtlString gDescriptionKey;
    static UtlString gLocationCodeKey;
    static UtlString gSubnetsKey;

    // Fast DB instance
    dbDatabase* m_pFastDB;

    // the persistent filename for loading/saving
    UtlString mDatabaseName;

    // boolean indicating table is loaded
    bool mTableLoaded;

private:
    UtlBoolean getRowByQuery( dbQuery& query, UtlHashMap& nvPairs ) const;
    Patterns subnetPatternMatcher;
    
    /**
     * Virtual Destructor
     */
    virtual ~LocationDB();

};

#endif //LOCATIONDB_H

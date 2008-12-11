// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef HUNTGROUPDB_H
#define HUNTGROUPDB_H

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
 *
 */
class HuntgroupDB
{
public:
    /**
     * Singleton Accessor
     *
     * @return
     */
    static HuntgroupDB* getInstance( const UtlString& name = "huntgroup" );

    /// releaseInstance - cleans up the singleton (for use at exit)
    static void releaseInstance();

    // Serialization
    OsStatus store();

    // Insert or update a row in the Huntgroup database.
    UtlBoolean insertRow ( const Url& identity );

    // delete huntgroup
    UtlBoolean removeRow ( const Url& identity );

    // Flushes the entire DB
    void removeAllRows ();

    // utility method for dumping all rows
    void getAllRows( ResultSet& rResultSet ) const;

    // Query interface to return a set of mapped full URI
    // contacts associated with the alias
    UtlBoolean isHuntGroup ( const Url& identity ) const;

    // The 'type' attribute of the top-level 'items' element.
    static const UtlString sType;

    // The XML namespace of the top-level 'items' element.
    static const UtlString sXmlNamespace;

    // Determine if the table is loaded from xml file
    bool isLoaded();

protected:
    // deserialization
    OsStatus load();

    // Singleton Constructor is private
    HuntgroupDB ( const UtlString& name );

    // One step closer to common load/store code
    UtlBoolean insertRow ( const UtlHashMap& nvPairs );

    // There is only one singleton in this design
    static HuntgroupDB* spInstance;

    // Singleton and Serialization mutex
    static OsMutex sLockMutex;

    // ResultSet column Keys
    static UtlString gIdentityKey;

    // Fast DB instance
    dbDatabase* m_pFastDB;

    // the persistent filename for loading/saving
    UtlString mDatabaseName;

    // the working direcory for all database instances
    // the XML files are located here
    UtlString m_etcDirectory;

    // boolean indicating table is loaded
    bool mTableLoaded;

private:
    /**
     * Virtual Destructor
     */
    virtual ~HuntgroupDB();

};

#endif //HUNTGROUPDB_H

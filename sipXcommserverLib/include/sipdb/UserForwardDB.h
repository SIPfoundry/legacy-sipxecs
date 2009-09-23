// 
// 
// Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef USERFORWARDDB_H
#define USERFORWARDDB_H

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
 * This class implements the UserForward database abstract class
 */
class UserForwardDB
{
public:
    /**
     * Singleton Accessor
     *
     * @return
     */
    static UserForwardDB* getInstance( const UtlString& name = "userforward" );

    /// releaseInstance - cleans up the singleton (for use at exit)
    static void releaseInstance();

    // Domain Serialization/Deserialization
    OsStatus store();

    /// Count rows in table
    int getRowCount () const;

    // One step closer to common load/store code
    UtlBoolean insertRow ( const UtlHashMap& nvPairs );

    // Insert or update a row in the UserForwardes database.
    UtlBoolean insertRow (
        const Url& identity,
        const UtlString& cfwdTime);

    // Delete methods
    UtlBoolean removeRow (const Url& identity );

    // Flushes the entire DB
    void removeAllRows ();

    // utility method for dumping all rows
    void getAllRows(ResultSet& rResultSet) const;

    // Get the call forward timer for the user uri
    UtlBoolean getCfwdTime (
        const Url& identity,
        UtlString& cfwdtime) const;

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
    UserForwardDB ( const UtlString& name );

    // There is only one singleton in this design
    static UserForwardDB* spInstance;

    // Singleton and Serialization mutex
    static OsMutex sLockMutex;

    // ResultSet column Keys
    static UtlString gIdentityKey;
    static UtlString gCfwdtimeKey;

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
    virtual ~UserForwardDB();

};

#endif //USERFORWARDDB_H


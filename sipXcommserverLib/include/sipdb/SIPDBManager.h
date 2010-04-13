//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef SIPDBMANAGER_H
#define SIPDBMANAGER_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsMutex.h"
#include "os/OsFS.h"
#include "fastdb/fastdb.h"

// DEFINES
#define SPECIAL_IMDB_NULL_VALUE "%"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class TiXmlNode;
class ResultSet;

// TableInfo is used to keep a track of
// attached processes to the DB, first one must
// provision the DB from the XML if it exists
// and the last one out must serialize the DB
// The unload is either done cleanly by the
// application or through the signal handler
class TableInfo
{
public:
    const char* tablename;      // the name of the database
    pthread_t   pid;            // the process ID
    int4        loadchecksum;   // a checksum only used during load (to detect change)
    bool        changed;        // flag used by other tables to determine if changed
    TYPE_DESCRIPTOR(
      (KEY(tablename, INDEXED),
       FIELD(pid),
       FIELD(loadchecksum),
       FIELD(changed)));
};

/**
 * Wrapper to the fast DB singleton.  This singleton ensures
 * that there is only one named instance of fastDB running per
 * process.  This instance will have all the tables stored in it
 */
class SIPDBManager
{
public:
    /**
     * Singleton Accessor
     *
     * @return
     */
    static SIPDBManager* getInstance();

    /**
     * Virtual Destructor
     */
    virtual ~SIPDBManager();

    /** dumps the IMDB meta table info */
    void getAllTableProcesses ( ResultSet& rResultSet ) const;

    /** Number of Processes attached to tables (Processes * Tables) */
    OsStatus getProcessCount ( int& rProcessCount ) const;

    /**
     * Registerd the database and pid, opens the DB if necessary
     * and returns it
     */
    dbDatabase* getDatabase ( const UtlString& tablename ) const;

    /**
     * unregisters the database from the IMDB and calls close to
     * decrement the user count etc.
     */
    void removeDatabase ( const UtlString& tablename ) const;

    /** counts the number of processes attached to the DB */
    int getNumDatabaseProcesses ( const UtlString& tablename ) const;

    OsStatus getDatabaseInfo (UtlString& rDatabaseInfo ) const;

    /** After loading an IMDB update its checksum information */
    void updateDatabaseInfo ( const UtlString& tablename, const int& checksum ) const;

    /** Sets a change flag for the table indicating that there was an insert/update to it */
    void setDatabaseChangedFlag ( const UtlString& tablename, bool changed ) const;

    /** determines whether the database has changed or not */
    bool getDatabaseChangedFlag ( const UtlString& tablename ) const;

    /** method to ping the data store and sleeps for timeoutSecs in the transaction */
    OsStatus pingDatabase (
        const int& rTransactionLockSecs = 0,
        const UtlBoolean& rTestWriteLock = FALSE ) const;

    /** Helper Method for all IMDB tables */
    static void getFieldValue (
        const unsigned char* base,
        const dbFieldDescriptor* fd,
        UtlString& textValue);

    /** Helper Method for all IMDB tables */
    static OsStatus getAttributeValue (
        const TiXmlNode& node,
        const UtlString& key,
        UtlString& value );

    /** preload all database tables **/
    OsStatus preloadAllDatabase() const;

    /** preload database table by name **/
    OsStatus preloadDatabaseTable(const UtlString &tablename) const;

    /** release all database tables **/
    OsStatus releaseAllDatabase() const;

protected:
    /**
     * Registerd the database and pid, opens the DB if necessary
     * and returns it
     */
    dbDatabase* openDatabase () const;

    // Protected Constructor
    SIPDBManager();

    /// Override the default fastdb tmp dir
    void setFastdbTempDir();

    // Singleton instance
    static SIPDBManager* spInstance;

    // Exclusive binary lock
    static OsMutex sLockMutex;

    // Fast DB instance
    static dbDatabase* spFastDB;

    // the working directory for all database instances
    // the XML files are located here (var)
    UtlString m_absWorkingDirectory;

    // The configuration file directory (etc)
    UtlString m_absConfigDirectory;

    // Fastdb tmp dir path
    UtlString m_FastDbTmpDirPath;
};

#endif //SIPDBMANAGER_H

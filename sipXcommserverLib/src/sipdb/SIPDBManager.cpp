//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>
#include <limits.h>

// APPLICATION INCLUDES
#include "utl/UtlInt.h"
#include "os/OsLock.h"
#include "os/OsFS.h"
#include "os/OsSysLog.h"
#include "os/OsTask.h"
#include "xmlparser/tinyxml.h"
#include "sipXecsService/SipXecsService.h"
#include "sipdb/ResultSet.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/AliasDB.h"
#include "sipdb/CallerAliasDB.h"
#include "sipdb/CredentialDB.h"
#include "sipdb/ExtensionDB.h"
#include "sipdb/PermissionDB.h"
#include "sipdb/RegistrationDB.h"
#include "sipdb/SubscriptionDB.h"
#include "sipdb/UserLocationDB.h"
#include "sipdb/UserForwardDB.h"
#include "sipdb/UserStaticDB.h"

// DEFINES

#define  CREDENTIAL     "credential"
#define  ALIAS          "alias"
#define  CALLER_ALIAS   "caller-alias"
#define  PERMISSION     "permission"
#define  EXTENSION      "extension"
#define  REGISTRATION   "registration"
#define  USERLOCATION   "userlocation"
#define  USERFORWARD    "userforward"
#define  USERSTATIC     "userstatic"
#define  SUBSCRIPTION   "subscription"

REGISTER( TableInfo );

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// Declared in fastdb/sync.h, but including that header file is problematic
extern char const* keyFileDir;

// CONSTANTS

// environment variable name for static data

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

// Static Initializers
SIPDBManager* SIPDBManager::spInstance = NULL;
dbDatabase*   SIPDBManager::spFastDB = NULL;
OsMutex       SIPDBManager::sLockMutex (OsMutex::Q_FIFO);

/* Helper method to return the process id */
int
getPid()
{
    return static_cast< int >
#ifdef WIN32
    (GetCurrentProcessId());
#else
    (getpid());
#endif
}

/* ============================ CREATORS ================================== */

SIPDBManager::SIPDBManager ()
   : m_absWorkingDirectory(SipXecsService::Path(SipXecsService::TmpDirType,""))
   , m_absConfigDirectory(SipXecsService::Path(SipXecsService::ConfigurationDirType,""))
{
    // Set the fastdb tmp dir
    setFastdbTempDir();
}

SIPDBManager::~SIPDBManager ()
{
    // Critical Section here
    OsLock lock( sLockMutex );

    pid_t pid = getPid();

    // it is possible that the database was never opened
    // as it is opened only the first time a request for
    // one of its managed databases exists
    if ( spFastDB != NULL )
    {
        // Thread Local Storage
        spFastDB->attach();

        // deregister all database's associated with the PID
        dbCursor< TableInfo > cursor( dbCursorForUpdate );

        dbQuery query;
        query="pid=",pid;

        if ( cursor.select( query ) > 0 )
        {
            cursor.removeAllSelected();
        }
        spFastDB->detach(0);

        // close() is MultiProcesss aware - always call it
        spFastDB->close();
        // put this here
        delete spFastDB;
        spFastDB = NULL;
    }

    // reset the static instance pointer to NULL
    spInstance = NULL;
}

OsStatus
SIPDBManager::getProcessCount ( int& rProcessCount ) const
{
    // Critical Section here
    OsLock lock( sLockMutex );

    // Ensure that the database is opened first
    if ( spFastDB == NULL )
    {
        // Lazy Open the database without attaching a particular
        // process to the tableInfo table
        spFastDB = openDatabase ();
    }

    OsStatus result = OS_FAILED;

    // it is possible that the database was never opened
    // as it is opened only the first time a request for
    // one of its managed databases exists
    if ( spFastDB != NULL )
    {
        pid_t pid = getPid();

        // Thread Local Storage
        spFastDB->attach();

        // Initialize the process count to zero
        rProcessCount = 0;

        // Now count the number or processes
        dbCursor< TableInfo > cursor;
        dbQuery query;
        query="pid=",pid,"order by pid";

        // see if the pid is identical for all rows
        if ( cursor.select() > 0 )
        {
            // Get the PID of the first process
            pthread_t lastpid = -1;
            do {
                // > 1 pid => not last process
                if ( lastpid != cursor->pid ) {
                    lastpid = cursor->pid;
                    rProcessCount += 1;
                }
            } while ( cursor.next() );
        }
        // Commit rows to memory - multiprocess workaround
        spFastDB->detach(0);

        result = OS_SUCCESS;
    } else
    {
        rProcessCount = 0;
    }

    return result;
}

OsStatus
SIPDBManager::pingDatabase (
    const int& rTransactionLockSecs,
    const UtlBoolean& rTestWriteLock ) const
{
    OsStatus result = OS_FAILED;

    // Ensure that the database is opened first
    if ( spFastDB == NULL )
    {
        // Lazy Open the database without attaching a particular
        // process to the tableInfo table
        spFastDB = openDatabase ();
    }

    // it is possible that the database was never opened
    // as it is opened only the first time a request for
    // one of its managed databases exists
    if ( spFastDB != NULL )
    {
        pid_t pid = getPid();
        spFastDB->attach();
        // create a readonly cursor
        dbCursor< TableInfo > cursor;
        dbCursor< TableInfo > cursorWithWriteLock( dbCursorForUpdate );

        dbQuery query;
        query="pid=",pid;

        // If we want to test the IMDB for read access the cursor should be used
        // otherwise the cursorWithWriteLock requests access to a new tx with exclusive
        // write access (waiting in effect for the other readers to cease beforehand
        if (rTestWriteLock == TRUE)
        {
            cursorWithWriteLock.select( query );
        } else
        {
            cursor.select( query );
        }
        // in order to test the Ping utility we need a way
        // of holding onto the transaction lock for a configurable
        // number of seconds
        if ( rTransactionLockSecs > 0 )
        {
            OsTask::delay( rTransactionLockSecs * 1000);
        }
        spFastDB->detach(0);

        // ping succeeded
        result = OS_SUCCESS;

    }
    return result;
}

void
SIPDBManager::getAllTableProcesses ( ResultSet& rResultSet ) const
{
    // Critical Section here
    OsLock lock( sLockMutex );

    // Clear the out any previous records
    rResultSet.destroyAll();

    // Ensure that the database is opened first
    if ( spFastDB == NULL )
    {
        // Lazy Open the database without attaching a particular
        // process to the tableInfo table
        spFastDB = openDatabase ();
    }

    if ( spFastDB != NULL )
    {
        // must do this first to ensure process/tls integrity
        spFastDB->attach();

        dbCursor< TableInfo > cursor;
        if ( cursor.select() > 0 )
        {
            do {
                UtlHashMap record;
                UtlString* tablenameValue =
                    new UtlString ( cursor->tablename );
                UtlInt* pidValue =
                    new UtlInt ( cursor->pid );
                UtlInt* loadchecksumValue =
                    new UtlInt ( cursor->loadchecksum );

                UtlString* tablenameKey = new UtlString( "tablename" );
                UtlString* pidKey = new UtlString( "pid" );
                UtlString* loadchecksumKey = new UtlString( "loadchecksum" );

                record.insertKeyAndValue ( tablenameKey, tablenameValue );
                record.insertKeyAndValue ( pidKey, pidValue );
                record.insertKeyAndValue ( loadchecksumKey, loadchecksumValue );

                rResultSet.addValue(record);
            } while (cursor.next());
        }
        // commit rows and also ensure process/tls integrity
        spFastDB->detach(0);
    }
}

int
SIPDBManager::getNumDatabaseProcesses ( const UtlString& tablename ) const
{
    // Critical Section here
    OsLock lock( sLockMutex );

    int numUsers = 0;

    // Ensure that the database is opened first
    if ( spFastDB == NULL )
    {
        // Lazy Open the database without attaching a particular
        // process to the tableInfo table
        spFastDB = openDatabase ();
    }

    if ( spFastDB != NULL )
    {
        // Thread Local Storage
        spFastDB->attach();

        // Now count the number of processes
        dbCursor< TableInfo > cursor;
        dbQuery query;
        query="tablename=",tablename;

        // count the processes attached to this DB
        numUsers = cursor.select(query);

        // Commit rows to memory - multiprocess workaround
        spFastDB->detach(0);
    }

    return numUsers;
}

dbDatabase*
SIPDBManager::openDatabase () const
{
    // Critical Section here
    OsLock lock( sLockMutex );

    pid_t pid = getPid();

    dbDatabase* database =
        new dbDatabase( dbDatabase::dbAllAccess
                        /*, 8*1024*1024 */ );

    // open the fastdb proprietary persistent file
    UtlString imdbFileName = SipXecsService::Path(SipXecsService::TmpDirType,"imdb.odb");

    // test for successful return code
    if ( database->open( "imdb", imdbFileName /*, 60000 */) )
    {
        // Begin TX
        database->attach();

        // This is the first process to use the fastDB
        // fastDB restores its old contents from "imdb.odb" so
        // we must assume that everything associated with this pid
        // should be deleted
        dbCursor< TableInfo > cursor ( dbCursorForUpdate );
        dbQuery query;
        query="pid=",pid;
        if ( cursor.select( query ) > 0 )
        {
            cursor.removeAllSelected();
        }
        // End TX
        database->detach(0);
    }
    else // failed to open IMDB delete datbase & return NULL
    {
        // call the destructor on the IMDB
        delete database;
        database = NULL;
    }
    return database;
}

dbDatabase*
SIPDBManager::getDatabase ( const UtlString& tablename ) const
{
    // Critical Section here
    OsLock lock( sLockMutex );

    pid_t pid = getPid();

    // Only construct one dbDatabase object per process
    if ( spFastDB == NULL )
    {
        // Absolute Size required in diskless mode, assume 8Megs sufficient
        spFastDB = new dbDatabase(
            dbDatabase::dbAllAccess /*, 8*1024*1024 */ );
    }

    // test to see if the IMDB was already opened
    if ( !spFastDB->isOpen() )
    {
        // the file name used for persistent storage
        // (using fastDB proprietary format)
        UtlString imdbFileName = SipXecsService::Path(SipXecsService::TmpDirType,"imdb.odb");

        // test for successful return code
        if ( spFastDB->open( "imdb", imdbFileName /*, 60000 */) )
        {
            // Begin TX
            spFastDB->attach();

            // This is the first process to use the fastDB
            // fastDB restores its old contents from "imdb.odb" so
            // we must assume that the table is dirty from previous
            // calls to  close (which persists the memory to disk)
            dbCursor< TableInfo > cursor ( dbCursorForUpdate );
            dbQuery query;

            query="tablename=",tablename,"and pid=",pid;
            if ( cursor.select( query ) > 0 )
            {
                cursor.removeAllSelected();
            }

            // End TX
            spFastDB->detach(0);
        }
        else // failed to open IMDB delete datbase & die
        {
            // call the destructor on the IMDB
            delete spFastDB;

            spFastDB = NULL;

            OsSysLog::add(FAC_DB, PRI_CRIT,
                "SIPDBManager::getDatabase - failed to open IMDB.") ;
            OsSysLog::flush();

            // Kill this process.  Do not call abort()!  That will
            // be caught by the signal handler, which will come back
            // through and try to open the database, which will fail,
            // which will end up here, which will....
            exit(42) ;
        }
    }

    // potentially the spFastDB can be NULL above if the open fails
    if (spFastDB != NULL)
    {
        // fastDB must be both non null and also opened successfully at this stage

        // If the tablename/pid do not match an existing row, insert a new
        // pid/tablename to track process usage of the tables
        // Begin Tx
        spFastDB->attach();

        // ensure that the table is registered with the process id.
        dbCursor< TableInfo > cursor ( dbCursorForUpdate );
        dbQuery query;
        query="tablename=",tablename,"and pid=",pid;
        if ( cursor.select( query ) > 0 )
        {
            do {
                // this code should not be exercised
                cursor->changed = TRUE;
                cursor->loadchecksum = 0;
                cursor.update();
            } while ( cursor.next() );
        } else
        {
            TableInfo tableInfo;
            tableInfo.tablename = tablename;
            tableInfo.pid = pid;
            tableInfo.loadchecksum = 0;
            tableInfo.changed = TRUE;
            // Implicit Attach here
            insert( tableInfo );
        }
        // End Tx
        spFastDB->detach(0);
    }
    OsSysLog::flush();
    return spFastDB;
}

void
SIPDBManager::removeDatabase ( const UtlString& tablename ) const
{
    // remove all rows from the imdb with this tablename
    OsLock lock( sLockMutex );

    // one dbDatabase construction call allowed per process
    if ( spFastDB != NULL )
    {
        pid_t pid = getPid();

        spFastDB->attach();
        dbCursor< TableInfo > cursor (dbCursorForUpdate);

        dbQuery query;
        query="tablename=",tablename,"and pid=",pid;
        // cannot be 0 rows at this stage
        if (cursor.select(query) > 0)
        {
            cursor.removeAllSelected();
        }
        // Commit rows to memory - multiprocess workaround
        spFastDB->detach(0);
    }
}

void
SIPDBManager::setDatabaseChangedFlag (
    const UtlString& tablename,
    bool changed ) const
{
    // Critical Section here
    OsLock lock( sLockMutex );

    if ( spFastDB != NULL )
    {
        spFastDB->attach();
        dbCursor< TableInfo > cursor ( dbCursorForUpdate );
        dbQuery query;
        query="tablename=",tablename;
        if ( cursor.select( query ) > 0 )
        {
            do {
                // reset the checksum & flag for all processes
                cursor->loadchecksum = 0;
                cursor->changed = changed;
                cursor.update();
            } while ( cursor.next() );
        } else
        {
            OsSysLog::add(FAC_DB, PRI_ERR,
                "SIPDBManager::setDatabaseChangedFlag - "
                "ERROR database %s not in TableInfo table", tablename.data());
        }
        // Commit rows to memory - multiprocess workaround
        spFastDB->detach(0);
    }

}

OsStatus
SIPDBManager::getDatabaseInfo( UtlString& rDatabaseInfo ) const
{

    // Ensure that the database is opened first
    if ( spFastDB == NULL )
    {
        // Lazy Open the database without attaching a particular
        // process to the tableInfo table
        spFastDB = openDatabase ();
    }

    OsStatus result = OS_FAILED;

    // it is possible that the database was never opened
    // as it is opened only the first time a request for
    // one of its managed databases exists
    if ( spFastDB != NULL )
    {
        spFastDB->attach();

        long allocatedSize = spFastDB->getAllocatedSize();
        long databaseSize  = spFastDB->getDatabaseSize();
        int numReaders = spFastDB->getNumberOfReaders();
        int numWriters = spFastDB->getNumberOfWriters();
        int numBlockedReaders = spFastDB->getNumberOfBlockedReaders();
        int numBlockedWriters = spFastDB->getNumberOfBlockedWriters();
        int numUsers = spFastDB->getNumberOfUsers();

        char temp[300];

        const char* outputStringFormat =
            "Database Meta Info\n==================\nAllocated Size:\t\t%d\nDatabase Size:\t\t%d\nReaders:\t\t%d\nWriters:\t\t%d\nBlocked Readers:\t%d\nBlocked Writers:\t%d\nUsers:\t\t\t%d\n";

        sprintf (
            temp, outputStringFormat,
            allocatedSize,
            databaseSize,
            numReaders,
            numWriters,
            numBlockedReaders,
            numBlockedWriters,
            numUsers );
        rDatabaseInfo = temp;

        spFastDB->detach(0);
        result = OS_SUCCESS;
    }
    return result;
}

bool
SIPDBManager::getDatabaseChangedFlag (
    const UtlString& tablename ) const
{
    // Critical Section here
    OsLock lock( sLockMutex );

    // Ensure that the database is opened first
    if ( spFastDB == NULL )
    {
        // Lazy Open the database
        spFastDB = openDatabase ();
    }

    bool result = TRUE;

    if ( spFastDB != NULL )
    {
        spFastDB->attach();
        dbCursor< TableInfo > cursor ( dbCursorForUpdate );
        dbQuery query;
        query="tablename=",tablename;
        if ( cursor.select( query ) > 0 )
        {
            result = cursor->changed;
        }
        // Commit rows to memory - multiprocess workaround
        spFastDB->detach(0);
    }

    return result;
}

void
SIPDBManager::updateDatabaseInfo (
    const UtlString& tablename,
    const int& checksum ) const
{
    // Critical Section here
    OsLock lock( sLockMutex );

    pid_t pid = getPid();

    if (spFastDB != NULL)
    {
        spFastDB->attach();
        dbCursor< TableInfo > cursor ( dbCursorForUpdate );

        dbQuery query;
        query="tablename=",tablename,"and pid=",pid;
        // cannot be 0 rows at this stage
        if ( cursor.select( query ) > 0 )
        {
            do {
                if ( cursor->loadchecksum != checksum )
                {
                    cursor->loadchecksum = checksum;
                    cursor->changed = TRUE;
                    cursor.update();
                } else // checksum matches
                {
                    if ( cursor->changed == TRUE )
                    {
                        cursor->changed = FALSE;
                        cursor.update();
                    }
                }
            } while ( cursor.next() );

        } else // this block should never be run
        {
            // Insert or Update the process related info
            TableInfo tableInfo;
            tableInfo.tablename = tablename;
            tableInfo.pid = pid;
            tableInfo.loadchecksum = 0;
            tableInfo.changed = TRUE;
            // Implicit Attach here
            insert( tableInfo );
        }
        // Commit rows to memory - multiprocess workaround
        spFastDB->detach(0);
    }

}

SIPDBManager*
SIPDBManager::getInstance()
{
    // Critical Section here
    OsLock lock( sLockMutex );

    // See if this is the first time through for this process
    // Note that this being null => pgDatabase is also null
    if ( spInstance == NULL )
    {   // Create the singleton class for clients to use
        spInstance = new SIPDBManager();
    }

    return spInstance;
}

void
SIPDBManager::getFieldValue (
    const unsigned char* base,
    const dbFieldDescriptor* fd,
    UtlString& textValue)
{
    char tempString[100];

    // The only data types that are currently known to work are "char*", "int4",
    // and "db_int8".  All other data types are likely to be broken, so comment
    // out that code for now.  If the caller tries to use a broken data type,
    // log an error and assert(false) rather than silently returning the wrong answer.
    // In the cases that work, we're using "fd->appOffs" rather than "fd->dbsOffs"
    // so perhaps we should make this change everywhere.  But that may be wrong,
    // and would be pointless without a unit test for this method, which we should
    // implement regardless.

    switch (fd->type) {
/*
    case dbField::tpBool:
        textValue = (char*) (*(bool*)(base + fd->dbsOffs))? "true" : "false";
        break;

    case dbField::tpInt1:
        sprintf ( tempString, "%d", *(int1*)(base + fd->dbsOffs) );
        textValue = tempString;
        break;

    case dbField::tpInt2:
        sprintf ( tempString, "%d", *(int2*)(base + fd->dbsOffs) );
        textValue = tempString;
        break;
*/
    case dbField::tpInt4:
        sprintf ( tempString, "%d", *(int4*)(base + fd->appOffs) );
        textValue = tempString;
        break;

    case dbField::tpInt8:
       sprintf ( tempString,"%0#16" FORMAT_INTLL "x",
                 *(int8*)(base + fd->appOffs) );
        textValue = tempString;
        break;
/*
    case dbField::tpReal4:
        sprintf ( tempString,  "%g", *(real4*)(base + fd->dbsOffs) );
        textValue = tempString;
        break;

    case dbField::tpReal8:
        sprintf ( tempString,  "%g", *(real8*)(base + fd->dbsOffs) );
        textValue = tempString;
        break;

    case dbField::tpRawBinary:
        if ( fd->dbsSize < 100 )
        {
            memcpy(tempString, base + fd->dbsOffs, fd->dbsSize);
        }
        break;
*/
    case dbField::tpString:
        textValue = *(char**)(base + fd->appOffs);
        break;

    default:
        OsSysLog::add(FAC_DB, PRI_ERR,
                      "SIPDBManager::getFieldValue - "
                      "ERROR unsupported data type: %d", fd->type);
        assert(false);
        break;
    }
}

OsStatus
SIPDBManager::getAttributeValue (
    const TiXmlNode& node,
    const UtlString& key,
    UtlString& value )
{
    OsStatus result = OS_SUCCESS;
    TiXmlNode* configNode = (TiXmlNode*)node.FirstChild( key );

    if ( (configNode != NULL) && (configNode->Type() == TiXmlNode::ELEMENT) )
    {
        // convert the node to an element
        TiXmlElement* elem = configNode->ToElement();
        if ( elem != NULL )
        {
            TiXmlNode* childNode = elem->FirstChild();
            if( childNode && childNode->Type() == TiXmlNode::TEXT )
            {
                TiXmlText* elementValue = childNode->ToText();
                if (elementValue)
                {
                    value = elementValue->Value();
                } else
                {
                    result = OS_FAILED;
                }
            } else
            {
                result = OS_FAILED;
            }
        } else
        {
            result = OS_FAILED;
        }
    } else
    {
        result = OS_FAILED;
    }
    return result;
}

OsStatus
SIPDBManager::preloadAllDatabase() const
{
    // Preload the following tables to ensure that
    // their reference count does not go to 0
    // causing an expensive load by another process

    // If called first thing during startup, this code
    // will force the xml files to be loaded into imdb.

    CredentialDB*   pCredentialDB   = CredentialDB::getInstance();
    SubscriptionDB* pSubscriptionDB = SubscriptionDB::getInstance();
    RegistrationDB* pRegistrationDB = RegistrationDB::getInstance();
    PermissionDB*   pPermissionDB   = PermissionDB::getInstance();
    ExtensionDB*    pExtensionDB    = ExtensionDB::getInstance();
    AliasDB*        pAliasDB        = AliasDB::getInstance();

    OsStatus res = OS_FAILED;

    if (pCredentialDB &&
    pSubscriptionDB &&
    pRegistrationDB &&
    pPermissionDB &&
    pExtensionDB &&
    pAliasDB) res = OS_SUCCESS;

    return res;
}

OsStatus
SIPDBManager::releaseAllDatabase() const
{
    CredentialDB::releaseInstance();
    SubscriptionDB::releaseInstance();
    RegistrationDB::releaseInstance();
    PermissionDB::releaseInstance();
    ExtensionDB::releaseInstance();
    AliasDB::releaseInstance();

    return OS_SUCCESS;
}

OsStatus
SIPDBManager::preloadDatabaseTable(const UtlString &tableName) const
{

   OsStatus res = OS_FAILED;
   if (tableName == CREDENTIAL)
   {
       if (CredentialDB::getInstance()->isLoaded())
       {
          res = OS_SUCCESS;
       }
   }
   else if (tableName == ALIAS)
   {
       if (AliasDB::getInstance()->isLoaded())
       {
          res = OS_SUCCESS;
       }
   }
   else if (tableName == PERMISSION)
   {
       if (PermissionDB::getInstance()->isLoaded())
       {
          res = OS_SUCCESS;
       }
   }
   else if (tableName == EXTENSION)
   {
       if (ExtensionDB::getInstance()->isLoaded())
       {
          res = OS_SUCCESS;
       }
   }
   else if (tableName == CALLER_ALIAS)
   {
      if (CallerAliasDB::getInstance()->isLoaded())
      {
          res = OS_SUCCESS;
      }
   }
   else if (tableName == USERLOCATION)
   {
      if (UserLocationDB::getInstance()->isLoaded())
      {
          res = OS_SUCCESS;
      }
   }
   else if (tableName == USERFORWARD)
   {
      if (UserForwardDB::getInstance()->isLoaded())
      {
          res = OS_SUCCESS;
      }
   }
   else if (tableName == USERSTATIC)
   {
      if (UserStaticDB::getInstance()->isLoaded())
      {
          res = OS_SUCCESS;
      }
   }
   else if (tableName == REGISTRATION)
   {
      if (RegistrationDB::getInstance()->isLoaded())
      {
          res = OS_SUCCESS;
      }
   }
   else if (tableName == SUBSCRIPTION)
   {
      if (SubscriptionDB::getInstance()->isLoaded())
      {
          res = OS_SUCCESS;
      }
   }
   else
   {
      OsSysLog::add(FAC_DB, PRI_ERR,
                      "SIPDBManager::preloadDatabaseTable - "
                      "ERROR unknown table name: %s", tableName.data() );
   }
   return res;
}
/// Override the default fastdb tmp dir
void SIPDBManager::setFastdbTempDir()
{
   // This method must only be called once
   assert(m_FastDbTmpDirPath.isNull());

   // The path must end in a separator
   m_FastDbTmpDirPath = SipXecsService::Path(SipXecsService::TmpDirType,OsPath::separator);

   // Pass the string to fastdb.  It's ugly to reference someone else's pointer
   // directly like this, but allows us to avoid changing the fastdb code for now.
   keyFileDir = m_FastDbTmpDirPath;
}

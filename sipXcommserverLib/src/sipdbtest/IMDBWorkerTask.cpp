// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsgQ.h"
#include "os/OsProcessIterator.h"

#include "net/Url.h"
#include "net/NetMd5Codec.h"
#include "xmlparser/tinyxml.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/ResultSet.h"
#include "sipdb/AliasDB.h"
#include "sipdb/HuntgroupDB.h"
#include "sipdb/ExtensionDB.h"
#include "sipdb/PermissionDB.h"
#include "sipdb/RegistrationDB.h"
#include "sipdb/SubscriptionDB.h"
#include "sipdb/DialByNameDB.h"
#include "sipdb/CredentialDB.h"
#include "sipdb/RegistrationDB.h"
#include "sipdb/SIPXAuthHelper.h"
#include "IMDBTaskMonitor.h"
#include "IMDBWorkerTask.h"
#include "utl/UtlInt.h"
#include "utl/UtlSortedList.h"
#include "utl/UtlSortedListIterator.h"
#include "utl/UtlHashMapIterator.h"

// These keys were added to support dumping the process information
UtlString componentKey ("component");
UtlString uriKey ("uri");
UtlString extensionKey ("extension");
UtlString callidKey ("callid");
UtlString contactKey ("contact");
UtlString realmKey ("realm");
UtlString useridKey ("userid");
UtlString passtokenKey ("passtoken");
UtlString pintokenKey ("pintoken");
UtlString authtypeKey ("authtype");
UtlString identityKey ("identity");
UtlString userKey ("user");
UtlString permissionKey ("permission");
UtlString qvalueKey ("qvalue");
UtlString expiresKey ("expires");
UtlString primaryKey ("primary");
UtlString updateNumberKey ("update_number");
UtlString timenowKey ("timenow");
UtlString subscribecseqKey ("subscribecseq");
UtlString eventtypeKey ("eventtype");
UtlString idKey ("id");
UtlString toKey ("to");
UtlString cseqKey ("cseq");
UtlString fromKey ("from");
UtlString keyKey ("key");
UtlString recordrouteKey ("recordroute");
UtlString notifycseqKey ("notifycseq");
UtlString acceptKey ("accept");
UtlString versionKey ("version");
UtlString np_identityKey ("np_identity");
UtlString np_contactKey ("np_contact");
UtlString np_digitsKey ("np_digits");

UtlString tablenameKey ("tablename");
UtlString loadchecksumKey ("loadchecksum" );
UtlString pidKey ("pid");

OsMutex IMDBWorkerTask::sLockMutex (OsMutex::Q_FIFO);

int 
IMDBWorkerTask::getPID() const
{
    return static_cast< int >
#ifdef WIN32
    (GetCurrentProcessId());
#else
    (getpid());
#endif
}

IMDBWorkerTask::IMDBWorkerTask( 
    const UtlString& rArgument, 
    OsMsgQ& rMsgQ, 
    OsEvent& rCommandEvent ) : 
    mBusy (FALSE), 
    mpMsgQ (&rMsgQ),
    mCommandEvent ( rCommandEvent ),
    mArgument ( rArgument )
{}

//IMDBWorkerTask::~IMDBWorkerTask()
//{}

OsStatus
IMDBWorkerTask::notifyMonitor( const int& rMessageIndicator ) const
{
    // notify the task monitor that we've finished (hopefully in a timely manner)
    const OsMsg successMsg ( OsMsg::USER_START, rMessageIndicator );

    return mpMsgQ->send ( successMsg );
}

UtlBoolean 
IMDBWorkerTask::isBusy() const
{
    return mBusy;
}

void
IMDBWorkerTask::setBusy (const UtlBoolean& rFlag )
{
    // Critical Section here
    OsLock lock( sLockMutex );
    mBusy = rFlag;
}

void
IMDBWorkerTask::cleanupIMDBResources() const
{
    delete SIPDBManager::getInstance();
}

int
IMDBWorkerTask::getProcessInfo ( 
    const UtlBoolean& rIgnoreCurrentProcess, 
    UtlString& rProcessInfo ) const
{
    int exitCode = EXIT_SUCCESS;

    ResultSet resultSet;

    int currentProcessPID = getPID();

    // request all the client process info from the IMDB
    // this is a flat table with 3 columns, pid, tablename and checksum (ignored)
    SIPDBManager::getInstance()->
        getAllTableProcesses ( resultSet );

    // initialize the returned data
    rProcessInfo = "FastDB Process/Table Mappings\n" \
                   "=============================\n";

    if ( resultSet.getSize() > 0 )
    {
        // Key is pid values are tablenames
        UtlHashMap collatedResults;

        // for each pid we can have multiple tables
        for (int i=0; i< resultSet.getSize(); i++)
        {
            UtlHashMap record;

            // get the next row
            resultSet.getIndex( i, record );

            // the row's process id
            int pid = ((UtlInt*)record.findValue(&pidKey))->getValue();

            // test to see if we should ignore the test app's pid as the
            // number of clients connected to the db will be off by 1
            if ( (currentProcessPID != pid) || (rIgnoreCurrentProcess == FALSE) )
            {
                // get the table name for this record
                UtlString* collectableTableName = 
                    ((UtlString*)record.findValue(&tablenameKey));

                UtlInt pidKey (pid);

                UtlSortedList* tableNames = 
                    ((UtlSortedList*)collatedResults.findValue(&pidKey));

                if ( tableNames != NULL )
                {   
                    // Append the current table name (shallow copy) 
                    tableNames->insert( collectableTableName );
                } else
                {
                    tableNames = new UtlSortedList;
                    tableNames->insert( collectableTableName );

                    UtlInt* newPidKey = 
                        new UtlInt ( pid );

                    collatedResults.insertKeyAndValue (
                        newPidKey, tableNames );
                }
            }
        }

        // Print out the collated results here
        UtlSortedList* tableNames;
        UtlHashMapIterator iterator( collatedResults );

        while ((tableNames = (UtlSortedList*)iterator()))
        {
            int nextPid = ((UtlInt*)iterator.key())->getValue();
            OsProcess process;
            OsProcessIterator processIterator;
            // find the name of the process 
            OsStatus retval = processIterator.findFirst( process );

            while ( retval == OS_SUCCESS )
            {
                if ( process.getPID() == nextPid )
                {
                    // get the process name for this record
                    UtlString processName;
                    process.getProcessName( processName );
                    char temp[10];
                    sprintf(temp, "%d", nextPid );
                    UtlString pidStr = temp ;
                    
                    rProcessInfo += (UtlString)"pid: " + pidStr + (UtlString)"\nprocess name: " + processName + (UtlString)"\ntable(s): ";

                    UtlSortedList* tableNames =
                        (UtlSortedList*)iterator.value();

                    UtlSortedListIterator next(*tableNames);
                    UtlString* tableName;
                    while ((tableName = (UtlString*)next()))
                    {
                        UtlString tableNameStr = *tableName;
                        rProcessInfo += tableNameStr + (UtlString)" ";
                    }
                    rProcessInfo += "\n";
                    break;
                }
                retval = processIterator.findNext( process );
            }
        }
    } else 
    {
        rProcessInfo += "No IMDB Processes\n";
        exitCode = EXIT_BADSYNTAX;
    }
    return exitCode;
}

OsStatus 
IMDBWorkerTask::getDatabaseInfo ( UtlString& rDatabaseInfo ) const
{
    return SIPDBManager::getInstance()->
        getDatabaseInfo ( rDatabaseInfo );
}

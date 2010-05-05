//
//
// Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <sipdb/SIPDBManager.h>
#include "statusserver/StatusServer.h"
#include "MwiStatusService.h"

// DEFINES

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FUNCTIONS
// FORWARD DECLARATIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// GLOBAL VARIABLE INITIALIZATIONS


/* ============================ FUNCTIONS ================================= */

//
// Create the service wrapper.  It doesn't do much until run is called.
//
MwiStatusService::MwiStatusService(
      const char* serviceName, const char* servicePrefix, const char* version
      ) :
   SipXecsService(serviceName, servicePrefix, version),
   mClosingIMDB(false),
   mLockMutex(OsMutex::Q_FIFO)
{
}

MwiStatusService::~MwiStatusService()
{
}

/**
 * Description:
 * closes any open connections to the IMDB safely using a mutex lock
 */
void
MwiStatusService::closeIMDBConnections ()
{
    // Critical Section here
    OsLock lock( mLockMutex );

    // now deregister this process's database references from the IMDB
    // and also ensure that we do not cause this code recursively
    // specifically SIGABRT or SIGSEGV could cause problems here
    if ( !mClosingIMDB )
    {
        mClosingIMDB = TRUE;
        // if deleting this causes another problem in this process
        // the gClosingIMDB flag above will protect us
        delete SIPDBManager::getInstance();
    }
}

//
// Create the real Server, passing in the configured parameters
//
void MwiStatusService::run()
{
    // Fetch Pointer to the OsServer task object, note that
    // object uses the IMDB so it is important to shut this thread
    // cleanly before the signal handler exits
    StatusServer* pStatusServer = StatusServer::startStatusServer(this, &getConfigDb());

    OsServerTask* pServerTask = static_cast<OsServerTask*>(pStatusServer);

    // Do not exit, let the proxy do its stuff
    while( !getShutdownFlag() )
    {
        OsTask::delay(2000);
    }

    // Remove the current process's row from the IMDB
    // Persisting the database if necessary

    // This is a server task so gracefully shutdown the
    // server task using the waitForShutdown method, this
    // will implicitly request a shutdown for us if one is
    // not already in progress
    if ( pServerTask != NULL )
    {
        // Deleting a server task is the only way of
        // waiting for shutdown to complete cleanly
        delete pServerTask;
        pServerTask = NULL;
    }

    // now deregister this process's database references from the IMDB
    closeIMDBConnections();

}

void MwiStatusService::configDbChanged(UtlString& configDbFile)
{
   // call the super's configDbChanged, which loads the new DB and adjusts the log level
   SipXecsService::configDbChanged(configDbFile);
   // now load changes specific to this service
}

// Note that this method is called in the StdinListener thread, so the implementation must be threadsafe.
void MwiStatusService::resourceChanged(UtlString& fileType, UtlString& configFile)
{
   // load changes specific to this service
   OsSysLog::add(LOG_FACILITY, PRI_INFO,
                 "MwiStatusService::resourceChanged: %s (type %s)",
                 configFile.data(), fileType.data() );

   //TODO: make any run-time config changes
}


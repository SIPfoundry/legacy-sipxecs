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
#include <os/OsFS.h>
#include <os/OsTask.h>
#include "registry/SipRegistrar.h"
#include <sipdb/SIPDBManager.h>
#include "sipXecsService/SipXecsService.h"
#include "RegistrarService.h"

// DEFINES

#define LOG_FACILITY   FAC_SIP

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
RegistrarService::RegistrarService(
      const char* serviceName, const char* servicePrefix, const char* version
      ) :
   SipXecsService(serviceName, servicePrefix, version),
   mpServerTask(NULL),
   mLockMutex(OsMutex::Q_FIFO)
{
}

RegistrarService::~RegistrarService()
{
}

/**
 * Description:
 * closes any open connections to the IMDB safely using a mutex lock
 */
void
RegistrarService::closeIMDBConnections ()
{
   // Critical Section here
   OsLock lock( mLockMutex );
   SIPDBManager* db = SIPDBManager::getInstance();
   db->releaseAllDatabase();

   delete db;
}

//
// Pull out important parameters from the config DB.
// These parameters take effect immediately.
//
void RegistrarService::loadConfig(
    )
{
}

//
// Create the real Server, passing in the configured parameters
//
void RegistrarService::run()
{
   // Fetch Pointer to the OsServer task object, note that
   // object uses the IMDB so it is important to shut this thread
   // cleanly before the signal handler exits
   SipRegistrar* registrar = SipRegistrar::getInstance(&getConfigDb());

   registrar->start();

   mpServerTask = static_cast<OsServerTask*>(registrar);

   // Do not exit, let the services run...
   while( !getShutdownFlag() && !mpServerTask->isShutDown() )
   {
      OsTask::delay(1 * OsTime::MSECS_PER_SEC);
   }
   OsSysLog::add(LOG_FACILITY, PRI_NOTICE, "main: cleaning up.");

   // This is a server task so gracefully shut down the
   // server task using the waitForShutdown method, this
   // will implicitly request a shutdown for us if one is
   // not already in progress
   if ( mpServerTask != NULL )
   {
      // Deleting a server task is the only way of
      // waiting for shutdown to complete cleanly
      OsSysLog::add(LOG_FACILITY, PRI_DEBUG, "main: shut down server task.");
      delete mpServerTask;
      mpServerTask = NULL;
   }

   // now deregister this process's database references from the IMDB
   closeIMDBConnections();

}

void RegistrarService::configDbChanged(UtlString& configDbFile)
{
   // call the super's configDbChanged, which loads the new DB and adjusts the log level
   SipXecsService::configDbChanged(configDbFile);
   // now load changes specific to this service
   loadConfig();
}

// Note that this method is called in the StdinListener thread, so the implementation must be threadsafe.
void RegistrarService::resourceChanged(UtlString& fileType, UtlString& configFile)
{
   // load changes specific to this service
   OsSysLog::add(LOG_FACILITY, PRI_INFO,
                 "RegistrarService::resourceChanged: %s (type %s)",
                 configFile.data(), fileType.data() );

   //TODO: make any run-time config changes
}


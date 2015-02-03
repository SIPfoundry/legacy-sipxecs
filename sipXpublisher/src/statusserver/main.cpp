//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
// StatusServer.cpp : Defines the entry point for the console application.
//

// SYSTEM INCLUDES
#include <iostream>
#include <stdio.h>
#include <signal.h>

// APPLICATION INCLUDES
#include "config.h"
#include "os/OsConfigDb.h"
#include "os/OsFS.h"
#include "os/OsTask.h"
#include "os/UnixSignals.h"
#include "os/OsTimer.h"
#include "os/OsResourceLimit.h"

#include "net/NameValueTokenizer.h"
#include "sipXecsService/SipXecsService.h"    // now deregister this process's database references from the IMDB
#include "sipXecsService/daemon.h"
#include "statusserver/StatusServer.h"

#include <os/OsLogger.h>
#include <os/OsLoggerHelper.h>
#include <statusserver/CustomExceptionHandlers.h>
#include "sipXecsService/SipXApplication.h"

// DEFINES
#define SIP_STATUS_PROCESS_NAME       "SipStatus"
#define CONFIG_SETTING_LOG_FORMAT     "SIP_STATUS_LOG_FORMAT"
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// GLOBAL VARIABLE INITIALIZATIONS
OsServerTask* pServerTask   = NULL;
UtlBoolean     gClosingIMDB  = FALSE;
OsMutex*       gpLockMutex = new OsMutex(OsMutex::Q_FIFO);

using namespace std;

/* ============================ FUNCTIONS ================================= */

/** The main entry point to the StatusServer */
int
main(int argc, char* argv[] )
{
  SipXApplicationData statusData =
  {
      SIPSTATUS_APP_NAME,
      CONFIG_SETTINGS_FILE,
      CONFIG_LOG_FILE,
      "",
      CONFIG_SETTING_PREFIX,
      true, // check mongo connection
      true, // enable mongo driver logging
      true, // increase application file descriptor limits
      true, // block signals on main thread (and all other threads created by main)
            // and process them only on a dedicated thread
      SipXApplicationData::ConfigFileFormatConfigDb, // format type for configuration file
      OsMsgQShared::QUEUE_UNLIMITED,
  };

  // NOTE: this might exit application in case of failure
  SipXApplication::instance().init(argc, argv, statusData);

  // register custom exception handling for mongo
  OsExceptionHandler::instance().registerHandler(MONGO_EXCEPTION, MONGO_CONNECT_EXCEPTION, boost::bind(&customMongoConnectExceptionHandling, _1));

  const OsConfigDb& configDb = SipXApplication::instance().getConfig().getOsConfigDb();

    // Fetch Pointer to the OsServer task object, note that
    // object uses the IMDB so it is important to shut this thread
    // cleanly before the signal handler exits
    StatusServer* pStatusServer = StatusServer::getInstance();

    pServerTask = static_cast<OsServerTask*>(pStatusServer);

    // Do not exit, let the proxy do its stuff
    while(!SipXApplication::instance().terminationRequested())
    {
        OsTask::delay(2000);
    }

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

    SipXApplication::instance().terminate();

    return 0;
}

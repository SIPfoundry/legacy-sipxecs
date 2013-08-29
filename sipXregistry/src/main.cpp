//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdio.h>
#include <signal.h>
#include <iostream>

// APPLICATION INCLUDES
#include "config.h"
#include "os/OsConfigDb.h"
#include "os/OsFS.h"
#include "os/OsTime.h"
#include "os/OsTask.h"
#include "os/OsLogger.h"
#include "os/OsLoggerHelper.h"
#include "os/UnixSignals.h"
#include "os/OsTimer.h"
#include "os/OsMsgQ.h"
#include "os/OsResourceLimit.h"

#include "net/NameValueTokenizer.h"
#include "sipXecsService/SipXecsService.h"
#include "sipXecsService/daemon.h"
#include "registry/SipRegistrar.h"

//
// Exception handling
//
#include <stdexcept>
#include <execinfo.h>
#include <mongo/util/assert_util.h>
#include "sipXecsService/SipXApplication.h"

#include "sipXecsService/SipXApplication.h"

// DEFINES
#define SIPREGISTRAR_APP_NAME "SipRegistrar"
#define CONFIG_SETTINGS_FILE  "registrar-config"
#define CONFIG_LOG_FILE       "sipregistrar.log"
#define CONFIG_NODE_FILE      "node.json"
#define CONFIG_LOG_DIR        SIPX_LOGDIR
#define CONFIG_ETC_DIR        SIPX_CONFDIR

#define CONFIG_SETTING_PREFIX         "SIP_REGISTRAR"
#define CONFIG_SETTING_LOG_CONSOLE    "SIP_REGISTRAR_LOG_CONSOLE"
#define CONFIG_SETTING_LOG_DIR        "SIP_REGISTRAR_LOG_DIR"
#define LOG_FACILITY                  FAC_SIP

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

using namespace std ;

// FUNCTIONS
// FORWARD DECLARATIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// GLOBAL VARIABLE INITIALIZATIONS
OsServerTask* pServerTask   = NULL;
#if 0
// No loner needed.  IMDB is history
OsMutex*       gpLockMutex = new OsMutex(OsMutex::Q_FIFO);
#endif
/* ============================ FUNCTIONS ================================= */

/**
 * Description:
 * closes any open connections to the IMDB safely using a mutex lock
 */

/** The main entry point to the sipregistrar */
int
main(int argc, char* argv[] )
{
  SipXApplicationData rlsData =
  {
      SIPREGISTRAR_APP_NAME,
      CONFIG_SETTINGS_FILE,
      CONFIG_LOG_FILE,
      CONFIG_NODE_FILE,
      CONFIG_SETTING_PREFIX,
      false, // do not check mongo connection
      true, // increase application file descriptor limits
      SipXApplicationData::ConfigFileFormatConfigDb, // format type for configuration file
      OsMsgQShared::QUEUE_UNLIMITED,
  };

  // NOTE: this might exit application in case of failure
  SipXApplication::instance().init(argc, argv, rlsData);

  OsConfigDb& configDb = SipXApplication::instance().getConfig().getOsConfigDb();

  Os::Logger::instance().log(FAC_SIP, PRI_NOTICE,
      "SipRegistrar >>>>>>>>>>>>>>>> STARTED"
  );

  SipRegistrar* registrar = SipRegistrar::getInstance(&configDb);
  std::string nodeFilePath = SipXApplication::instance().getNodeFilePath();
  registrar->setNodeConfig(nodeFilePath.data());
  registrar->start();
  pServerTask = static_cast<OsServerTask*>(registrar);
  while( !SipXApplication::instance().terminationRequested() && !pServerTask->isShutDown())
  {
    sleep(2000);
  }
  Os::Logger::instance().log(LOG_FACILITY, PRI_NOTICE, "main: cleaning up.");

  // This is a server task so gracefully shut down the
  // server task using the waitForShutdown method, this
  // will implicitly request a shutdown for us if one is
  // not already in progress
  if ( pServerTask != NULL )
  {
    // Deleting a server task is the only way of
    // waiting for shutdown to complete cleanly
    Os::Logger::instance().log(LOG_FACILITY, PRI_DEBUG, "main: shut down server task.");
    delete pServerTask;
    pServerTask = NULL;
  }

  SipXApplication::instance().terminate();

  return 0;
}


// The infamous JNI_LightButton stub, to resolve the reference in libsipXcall.
int JNI_LightButton(long)
{
   return 0;
}

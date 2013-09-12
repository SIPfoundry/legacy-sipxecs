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

// DEFINES
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

// Initialize the OsSysLog
void
initSysLog(OsConfigDb* pConfig)
{
   UtlString logLevel;               // Controls Log Verbosity
   UtlString consoleLogging;         // Enable console logging by default?
   UtlString fileTarget;             // Path to store log file.
   UtlBoolean bSpecifiedDirError ;   // Set if the specified log dir does not
   // exist

   Os::LoggerHelper::instance().processName = "SipRegistrar";

   //
   // Get/Apply Log Filename
   //
   fileTarget.remove(0) ;
   if ((pConfig->get(CONFIG_SETTING_LOG_DIR, fileTarget) != OS_SUCCESS) ||
       fileTarget.isNull() || !OsFileSystem::exists(fileTarget))
   {
      bSpecifiedDirError = !fileTarget.isNull() ;

      // If the log file directory exists use that, otherwise place the log
      // in the current directory
      OsPath workingDirectory;
      if (OsFileSystem::exists(CONFIG_LOG_DIR))
      {
         fileTarget = CONFIG_LOG_DIR;
         OsPath path(fileTarget);
         path.getNativePath(workingDirectory);

         osPrintf("%s : %s\n", CONFIG_SETTING_LOG_DIR, workingDirectory.data()) ;
         Os::Logger::instance().log(LOG_FACILITY, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_DIR, workingDirectory.data()) ;
      }
      else
      {
         OsPath path;
         OsFileSystem::getWorkingDirectory(path);
         path.getNativePath(workingDirectory);

         osPrintf("%s : %s\n", CONFIG_SETTING_LOG_DIR, workingDirectory.data()) ;
         Os::Logger::instance().log(LOG_FACILITY, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_DIR, workingDirectory.data()) ;
      }

      fileTarget = workingDirectory +
         OsPathBase::separator +
         CONFIG_LOG_FILE;
   }
   else
   {
      bSpecifiedDirError = false ;
      osPrintf("%s : %s\n", CONFIG_SETTING_LOG_DIR, fileTarget.data()) ;
      Os::Logger::instance().log(LOG_FACILITY, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_DIR, fileTarget.data()) ;

      fileTarget = fileTarget +
         OsPathBase::separator +
         CONFIG_LOG_FILE;
   }



   //
   // Get/Apply Log Level
   //
   SipXecsService::setLogPriority(*pConfig, CONFIG_SETTING_PREFIX);
   Os::LoggerHelper::instance().initialize(fileTarget.data());
   Os::Logger::instance().setLoggingPriorityForFacility(FAC_SIP_INCOMING_PARSED, PRI_ERR);

   //
   // Get/Apply console logging
   //
   UtlBoolean bConsoleLoggingEnabled = false ;
   if ((pConfig->get(CONFIG_SETTING_LOG_CONSOLE, consoleLogging) ==
        OS_SUCCESS))
   {
      consoleLogging.toUpper();
      if (consoleLogging == "ENABLE")
      {
         Os::Logger::instance().enableConsoleOutput(true);
         bConsoleLoggingEnabled = true ;
      }
   }

   Os::Logger::instance().log(LOG_FACILITY, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_CONSOLE,
                 bConsoleLoggingEnabled ? "ENABLE" : "DISABLE") ;

   if (bSpecifiedDirError)
   {
      Os::Logger::instance().log(FAC_LOG, PRI_CRIT,
                    "Cannot access directory '%s'; please check configuration.",
                    CONFIG_SETTING_LOG_DIR);
   }
}

bool isInterrupted = false;

void signal_handler(int sig) {
    // cannot seem to log from here?
    switch(sig) {
    case SIGTERM:
	isInterrupted = true;
	break;
    }
}

// copy error information to log. registered only after logger has been configured.
void catch_global()
{
#define catch_global_print(msg)  \
  std::ostringstream bt; \
  bt << msg << std::endl; \
  void* trace_elems[20]; \
  int trace_elem_count(backtrace( trace_elems, 20 )); \
  char** stack_syms(backtrace_symbols(trace_elems, trace_elem_count)); \
  for (int i = 0 ; i < trace_elem_count ; ++i ) \
    bt << stack_syms[i] << std::endl; \
  Os::Logger::instance().log(FAC_LOG, PRI_CRIT, bt.str().c_str()); \
  std::cerr << bt.str().c_str(); \
  free(stack_syms);

  try
  {
      throw;
  }
  catch (std::string& e)
  {
    catch_global_print(e.c_str());
  }
#ifdef MONGO_assert
  catch (mongo::DBException& e)
  {
    catch_global_print(e.toString().c_str());
  }
#endif
  catch (boost::exception& e)
  {
    catch_global_print(diagnostic_information(e).c_str());
  }
  catch (std::exception& e)
  {
    catch_global_print(e.what());
  }
  catch (...)
  {
    catch_global_print("Error occurred. Unknown exception type.");
  }

  std::abort();
}

/** The main entry point to the sipregistrar */
int
main(int argc, char* argv[] )
{
    char* pidFile = NULL;
    for (int i = 1; i < argc; i++) {
        if (strncmp("-v", argv[i], 2) == 0) {
            std::cout << "Version: " << PACKAGE_VERSION << PACKAGE_REVISION << std::endl;
            exit(0);
        } else {
            pidFile = argv[i];
        }
    }
    if (pidFile) {
      daemonize(pidFile);
    }

    OsMsgQShared::setQueuePreference(OsMsgQShared::QUEUE_UNLIMITED);

   // Configuration Database (used for OsSysLog)
   OsConfigDb* configDb = new OsConfigDb();

   // initialize log file
   OsPath workingDirectory;
   if (OsFileSystem::exists(CONFIG_ETC_DIR))
   {
      workingDirectory = CONFIG_ETC_DIR;
      OsPath path(workingDirectory);
      path.getNativePath(workingDirectory);
   }
   else
   {
      OsPath path;
      OsFileSystem::getWorkingDirectory(path);
      path.getNativePath(workingDirectory);
   }

   UtlString fileName =  workingDirectory +
      OsPathBase::separator +
      CONFIG_SETTINGS_FILE;

   UtlString nodeFile =  workingDirectory +
      OsPathBase::separator +
      CONFIG_NODE_FILE;

   bool configLoaded = ( configDb->loadFromFile(fileName) == OS_SUCCESS );
   if (!configLoaded)
   {
      exit(1);
   }

   initSysLog(configDb) ;

   std::set_terminate(catch_global);
   
   //
   // Raise the file handle limit to maximum allowable
   //
   typedef OsResourceLimit::Limit Limit;
   Limit rescur = 0;
   Limit resmax = 0;
   OsResourceLimit resource;
   if (resource.setApplicationLimits("sipxrls"))
   {
     resource.getFileDescriptorLimit(rescur, resmax);
     OS_LOG_NOTICE(FAC_KERNEL, "Maximum file descriptors set to " << rescur);
   }
   else
   {
     OS_LOG_ERROR(FAC_KERNEL, "Unable to set file descriptor limit");
   }

   
   Os::Logger::instance().log(FAC_SIP, PRI_NOTICE,
                 "SipRegistrar >>>>>>>>>>>>>>>> STARTED"
                 );
   if (configLoaded)
   {
      Os::Logger::instance().log(FAC_SIP, PRI_INFO, "Read config %s", fileName.data());
   }
   else
   {
      if (configDb->storeToFile(fileName) == OS_SUCCESS)
      {
         Os::Logger::instance().log( FAC_SIP, PRI_INFO, "Default config written to: %s",
                       fileName.data()
                       );
      }
      else
      {
         Os::Logger::instance().log( FAC_SIP, PRI_ERR, "Default config write failed to: %s",
                       fileName.data());
      }
   }

   SipRegistrar* registrar = SipRegistrar::getInstance(configDb);
   registrar->setNodeConfig(nodeFile.data());
   registrar->start();
   pServerTask = static_cast<OsServerTask*>(registrar);
   signal(SIGHUP, signal_handler); // catch hangup signal
   signal(SIGTERM, signal_handler); // catch kill signal
   while( !isInterrupted && !pServerTask->isShutDown())
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

   if ( configDb != NULL )
   {
      delete configDb;
   }

   //
   // Terminate the timer thread
   //
   OsTimer::terminateTimerService();

   // Flush the log file
   Os::Logger::instance().flush();

   mongo::dbexit(mongo::EXIT_CLEAN);

   return 0;
}


// The infamous JNI_LightButton stub, to resolve the reference in libsipXcall.
int JNI_LightButton(long)
{
   return 0;
}

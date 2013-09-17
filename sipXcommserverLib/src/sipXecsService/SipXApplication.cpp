#include "sipXecsService/SipXApplication.h"
#include <boost/bind.hpp>

#include "os/OsResourceLimit.h"

#include "os/OsServiceOptions.h"

#include <os/OsExceptionHandler.h>

bool SipXApplication::init(int argc, char* argv[], const SipXApplicationData& appData)
{
  _appData = appData;

  // register default exception handler methods
  // exit for mongo tcp related exceptions, core dump for others
  OsExceptionHandler::instance();

  if (_appData._daemonize)
  {
   doDaemonize(argc, argv);
  }

  registrerSignalHandlers();

  OsMsgQShared::setQueuePreference(_appData._queuePreference);

  if (!loadConfiguration(_appData._configFilename))
  {
   fprintf(stderr, "Failed to load config DB from file '%s'", _appData._configFilename.c_str());

    exit(1);
  }

  // Initialize log file
  initSysLog(&_osServiceOptions);

  std::set_terminate(&OsExceptionHandler::catch_global);

  //
  // Raise the file handle limit to maximum allowable
  //
  typedef OsResourceLimit::Limit Limit;
  Limit rescur = 0;
  Limit resmax = 0;
  OsResourceLimit resource;
  if (resource.setApplicationLimits(_appData._appName))
  {
    resource.getFileDescriptorLimit(rescur, resmax);
    OS_LOG_NOTICE(FAC_KERNEL, "Maximum file descriptors set to " << rescur);
  }
  else
  {
    OS_LOG_ERROR(FAC_KERNEL, "Unable to set file descriptor limit");
  }

  if (_appData._checkMongo)
  {
   if (!testMongoDBConnection())
   {
     mongo::dbexit(mongo::EXIT_CLEAN);
     exit(1);
   }
  }

  Os::Logger::instance().log(FAC_SIP, PRI_NOTICE, "%s initialized", _appData._appName.c_str());
  _initialized = true;
  return true;
}

void SipXApplication::doDaemonize(int argc, char* argv[])
{
  char* pidFile = NULL;
   for(int i = 1; i < argc; i++) {
      if(strncmp("-v", argv[i], 2) == 0) {
       std::cout << "Version: " << PACKAGE_VERSION << PACKAGE_REVISION << std::endl;
       exit(0);
      } else {
       pidFile = argv[i];
      }
   }
   if (pidFile) {
    daemonize(pidFile);
   }
}

void  SipXApplication::registrerSignalHandlers()
{
  Os::UnixSignals::instance().registerSignalHandler(SIGHUP , boost::bind(&SipXApplication::handleSIGHUP, this));
  Os::UnixSignals::instance().registerSignalHandler(SIGPIPE, boost::bind(&SipXApplication::handleSIGPIPE, this));

  Os::UnixSignals::instance().registerSignalHandler(SIGTERM, boost::bind(&SipXApplication::handleTerminationSignal, this));
  Os::UnixSignals::instance().registerSignalHandler(SIGINT , boost::bind(&SipXApplication::handleTerminationSignal, this));
  Os::UnixSignals::instance().registerSignalHandler(SIGQUIT, boost::bind(&SipXApplication::handleTerminationSignal, this));
  Os::UnixSignals::instance().registerSignalHandler(SIGABRT, boost::bind(&SipXApplication::handleTerminationSignal, this));

  Os::UnixSignals::instance().registerSignalHandler(SIGUSR1 , boost::bind(&SipXApplication::handleSIGUSR1, this));
  Os::UnixSignals::instance().registerSignalHandler(SIGUSR2 , boost::bind(&SipXApplication::handleSIGUSR2, this));
}

bool SipXApplication::loadConfiguration(const std::string& configFilename)
{
  bool ret = false;

  // Load configuration file.
  OsPath workingDirectory;
  if (OsFileSystem::exists(SIPX_CONFDIR))
  {
    workingDirectory = SIPX_CONFDIR;
    OsPath path(workingDirectory);
    path.getNativePath(workingDirectory);
  }
  else
  {
    OsPath path;
    OsFileSystem::getWorkingDirectory(path);
    path.getNativePath(workingDirectory);
  }

  UtlString fileName = workingDirectory + OsPathBase::separator + configFilename.c_str();
  if (!_appData._nodeFilename.empty())
  {
    _nodeFilePath = workingDirectory + OsPathBase::separator + _appData._nodeFilename.c_str();
  }

  if (OS_SUCCESS == _osServiceOptions.loadConfigDbFromFile(fileName.data()))
  {
    Os::Logger::instance().log(FAC_SIP, PRI_NOTICE, "Loading configuration %s", fileName.data());
    ret = true;
  }
  else
  {
    Os::Logger::instance().log(FAC_SIP, PRI_ERR, "Failed loading configuration %s", fileName.data());
  }

  return ret;
}

bool SipXApplication::reloadConfiguration(const std::string& configFilename)
{
  if (configFilename.empty())
  {
   return loadConfiguration(_appData._configFilename);
  }

  return loadConfiguration(configFilename);
}

bool SipXApplication::testMongoDBConnection()
{
  bool ret = true;

  std::string errmsg;

  mongo::ConnectionString mongoConnectionString = MongoDB::ConnectionInfo::connectionStringFromFile();
  if (false == MongoDB::ConnectionInfo::testConnection(mongoConnectionString, errmsg))
  {
    Os::Logger::instance().log(FAC_SIP, PRI_CRIT,
        "Failed to connect to '%s' - %s",
        mongoConnectionString.toString().c_str(), errmsg.c_str());

    ret = false;
  }

  return ret;
}

void SipXApplication::terminate()
{
  //
  // Terminate the timer thread
  //
  OsTimer::terminateTimerService();

  // Say goodnight Gracie...
  Os::Logger::instance().log(FAC_KERNEL, PRI_NOTICE, "Exiting %s", _appData._appName.c_str()) ;
  Os::Logger::instance().flush();

  mongo::dbexit(mongo::EXIT_CLEAN);
}

// Initialize the OsSysLog
void SipXApplication::initSysLog(OsServiceOptions* pOsServiceOptions)
{
  UtlString logLevel;          // Controls Log Verbosity
  UtlString consoleLogging;      // Enable console logging by default?
  UtlString fileTarget;         // Path to store log file.
  UtlBoolean bSpecifiedDirError ;  // Set if the specified log dir does not
                        // exist
  Os::LoggerHelper::instance().processName = _appData._appName.c_str();

  std::string configSettingLogDir = _appData._configPrefix + "_LOG_DIR";
  std::string configSettingLogConsole = _appData._configPrefix + "_LOG_CONSOLE";

  //
  // Get/Apply Log Filename
  //
  fileTarget.remove(0);
  if ((pOsServiceOptions->getOption(configSettingLogDir.c_str(), fileTarget) != OS_SUCCESS) ||
    fileTarget.isNull() || !OsFileSystem::exists(fileTarget))
  {
    bSpecifiedDirError = !fileTarget.isNull();

    // If the log file directory exists use that, otherwise place the log
    // in the current directory
    OsPath workingDirectory;
    if (OsFileSystem::exists(SIPX_LOGDIR))
    {
      fileTarget = SIPX_LOGDIR;
      OsPath path(fileTarget);
      path.getNativePath(workingDirectory);

      osPrintf("%s : %s\n", configSettingLogDir.c_str(), workingDirectory.data());
      Os::Logger::instance().log(FAC_KERNEL, PRI_INFO, "%s : %s",
                configSettingLogDir.c_str(), workingDirectory.data());
    }
    else
    {
      OsPath path;
      OsFileSystem::getWorkingDirectory(path);
      path.getNativePath(workingDirectory);

      osPrintf("%s : %s\n", configSettingLogDir.c_str(), workingDirectory.data());
      Os::Logger::instance().log(FAC_KERNEL, PRI_INFO, "%s : %s",
                configSettingLogDir.c_str(), workingDirectory.data());
    }

    fileTarget = workingDirectory +
      OsPathBase::separator +
      _appData._logFilename;
  }
  else
  {
    bSpecifiedDirError = false;
    osPrintf("%s : %s\n", configSettingLogDir.c_str(), fileTarget.data());
    Os::Logger::instance().log(FAC_KERNEL, PRI_INFO, "%s : %s",
              configSettingLogDir.c_str(), fileTarget.data());

    fileTarget = fileTarget +
      OsPathBase::separator +
      _appData._logFilename;
  }


  //
  // Get/Apply Log Level
  //
  SipXecsService::setLogPriority(_osServiceOptions.getOsConfigDb(), _appData._configPrefix.c_str());
  Os::Logger::instance().setLoggingPriorityForFacility(FAC_SIP_INCOMING_PARSED, PRI_ERR);
  Os::LoggerHelper::instance().initialize(fileTarget);

  //
  // Get/Apply console logging
  //
  UtlBoolean bConsoleLoggingEnabled = false;
  if ((pOsServiceOptions->getOption(configSettingLogConsole.c_str(), consoleLogging) == OS_SUCCESS))
  {
    consoleLogging.toUpper();
    if (consoleLogging == "ENABLE")
    {
      Os::Logger::instance().enableConsoleOutput(true);
      bConsoleLoggingEnabled = true;
    }
  }

  osPrintf("%s : %s\n", configSettingLogConsole.c_str(),
      bConsoleLoggingEnabled ? "ENABLE" : "DISABLE") ;
  Os::Logger::instance().log(FAC_KERNEL, PRI_INFO, "%s : %s", configSettingLogConsole.c_str(),
      bConsoleLoggingEnabled ? "ENABLE" : "DISABLE") ;

  if (bSpecifiedDirError)
  {
    Os::Logger::instance().log(FAC_KERNEL, PRI_CRIT,
        "Cannot access %s directory; please check configuration.",
        configSettingLogDir.c_str());
  }
}

bool& SipXApplication::terminationRequested()
{
  return Os::UnixSignals::instance().isTerminateSignalReceived();
}

#include "sipXecsService/SipXApplication.h"
#include <boost/bind.hpp>

#include "os/OsResourceLimit.h"

#include "os/OsServiceOptions.h"

#include <os/OsExceptionHandler.h>

#include <sipdb/MongoDB.h>

static bool gHasDaemonized = false;

void SipXApplication::displayVersion(std::ostream& strm) const
{
  strm << "Version: " << PACKAGE_VERSION << PACKAGE_REVISION << std::endl;
  strm.flush();
}

void SipXApplication::displayUsage(std::ostream& strm)
{
  _pOsServiceOptions->displayUsage(strm);
}

void SipXApplication::showVersionHelp()
{
  if (_pOsServiceOptions->hasOption(OsServiceOptions::helpOption().pName))
  {
    displayVersion(std::cout);
    displayUsage(std::cout);
    exit(0);
  }

  if (_pOsServiceOptions->hasOption(OsServiceOptions::versionOption().pName))
  {
    displayVersion(std::cout);
    exit(0);
  }
}

bool SipXApplication::init(int argc, char* argv[], const SipXApplicationData& appData, OsServiceOptions* pOptions)
{
  _appData = appData;
  _argc = argc;
  _argv = argv;

  // register default exception handler methods
  // exit for mongo tcp related exceptions, core dump for others
  OsExceptionHandler::instance();

  doDaemonize(argc, argv);

  if (OsTaskBase::blockSignals() != OS_SUCCESS)
  {
    fprintf(stderr, "Unable to block signals\n");
  }

  registerSignalHandlers();

  OsMsgQShared::setQueuePreference(_appData._queuePreference);

  if (!pOptions)
  {
    _pOsServiceOptions->addDefaultOptions();

    if (!parse(*_pOsServiceOptions, argc, argv, _appData._configFilename))
    {
      fprintf(stderr, "Failed to load configuration\n");
      exit(1);
    }
  }
  else
  {
    //
    // The application owns config.  It should take care of doing the stuff above
    //
    delete _pOsServiceOptions;
    _pOsServiceOptions = pOptions;
    _autoDeleteConfig = false;
  }

  // checks version or help options
  showVersionHelp();

  // Initialize log file
  initLogger();

  std::set_terminate(&OsExceptionHandler::catch_global);

  // Raise the file handle limit to maximum allowable
  if (_appData._increaseResourceLimits)
  {
    increaseResourceLimits();
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

bool SipXApplication::increaseResourceLimits(const std::string& configurationPath)
{
  bool ret = false;

  //
  // Raise the file handle limit to maximum allowable
  //
  typedef OsResourceLimit::Limit Limit;
  Limit rescur = 0;
  Limit resmax = 0;
  OsResourceLimit resource;
  ret = resource.setApplicationLimits(_appData._appName, configurationPath);
  if (ret == true)
  {
    resource.getFileDescriptorLimit(rescur, resmax);
    OS_LOG_NOTICE(FAC_KERNEL, "Maximum file descriptors set to " << rescur);
  }
  else
  {
    OS_LOG_ERROR(FAC_KERNEL, "Unable to set file descriptor limit");
  }

  return ret;
}

void SipXApplication::doDaemonize(int argc, char** pArgv)
{
  if (gHasDaemonized)
    return;

  char* pidFile = NULL;

  for (int i = 0; i < argc; i++)
  {
    std::string arg = pArgv[i];
    if (arg == (std::string("--") + OsServiceOptions::pidFileOption().pName) && (i + 1) < argc)
    {
      pidFile = pArgv[i + 1];
      break;
    }
  }

  if (pidFile)
  {
    daemonize(pidFile);
    gHasDaemonized = true;
  }
}

void  SipXApplication::registerSignalHandlers()
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

bool SipXApplication::parse(OsServiceOptions& osServiceOptions, int argc, char** pArgv, const std::string& configFilename)
{
  bool ret = false;
  int parseOptionsFlags = OsServiceOptions::NoOptionsFlag;
  UtlString fileName;

  if (_appData._configFileFormat == SipXApplicationData::ConfigFileFormatConfigDb)
  {
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

    if (!configFilename.empty())
      fileName = workingDirectory + OsPathBase::separator + configFilename.c_str();

    if (!_appData._nodeFilename.empty())
    {
      _nodeFilePath = workingDirectory + OsPathBase::separator + _appData._nodeFilename.c_str();
    }

    parseOptionsFlags |= OsServiceOptions::ParseConfigDbFlag;
  }
  else if (_appData._configFileFormat == SipXApplicationData::ConfigFileFormatIni)
  {
    fileName = configFilename;
  }

  osServiceOptions.setConfigurationFile(fileName.data());
  osServiceOptions.setCommandLine(argc, pArgv);

  parseOptionsFlags |= OsServiceOptions::AddDefaultComandLineOptionsFlag;
  parseOptionsFlags |= OsServiceOptions::StopIfVersionHelpFlag;

  ret = osServiceOptions.parseOptions((OsServiceOptions::ParseOptionsFlags)parseOptionsFlags);

  return ret;
}

bool SipXApplication::loadConfiguration(OsServiceOptions& osServiceOptions, const std::string& configFilename)
{
  if (configFilename.empty())
  {
   return parse(osServiceOptions, _argc, _argv, _appData._configFilename);
  }

  return parse(osServiceOptions, _argc, _argv, configFilename);
}

bool SipXApplication::testMongoDBConnection()
{
  bool ret = true;

  std::string errmsg;
  MongoDB::ConnectionInfo ginfo = MongoDB::ConnectionInfo::globalInfo();
  mongo::ConnectionString mongoConn = ginfo.getConnectionString();
  if (false == MongoDB::ConnectionInfo::testConnection(mongoConn, errmsg))
  {
      Os::Logger::instance().log(FAC_KERNEL, PRI_CRIT,
              "Failed to connect to '%s' - %s",
              mongoConn.toString().c_str(), errmsg.c_str());

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

void SipXApplication::initLoggerByConfigurationFile()
{
  UtlString logLevel;          // Controls Log Verbosity
  UtlString consoleLogging;      // Enable console logging by default?
  UtlString fileTarget;         // Path to store log file.
  UtlBoolean bSpecifiedDirError ;  // Set if the specified log dir does not
                        // exist

  // If no log file is given don't start logger
  if (_appData._logFilename.empty())
    return;

  Os::LoggerHelper::instance().processName = _appData._appName.c_str();

  std::string configSettingLogDir = _appData._configPrefix + "_LOG_DIR";
  std::string configSettingLogConsole = _appData._configPrefix + "_LOG_CONSOLE";

  //
  // Get/Apply Log Filename
  //
  fileTarget.remove(0);
  if ((_pOsServiceOptions->getOption(configSettingLogDir.c_str(), fileTarget) != OS_SUCCESS) ||
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
  SipXecsService::setLogPriority(_pOsServiceOptions->getOsConfigDb(), _appData._configPrefix.c_str());
  Os::Logger::instance().setLoggingPriorityForFacility(FAC_SIP_INCOMING_PARSED, PRI_ERR);
  Os::LoggerHelper::instance().initialize(fileTarget);

  //
  // Get/Apply console logging
  //
  UtlBoolean bConsoleLoggingEnabled = false;
  if ((_pOsServiceOptions->getOption(configSettingLogConsole.c_str(), consoleLogging) == OS_SUCCESS))
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

void SipXApplication::initLoggerByCommandLine(bool& initialized)
{
  std::string logFile;
  int logLevel = PRI_NOTICE;
  if (_pOsServiceOptions->hasOption(OsServiceOptions::logFileOption().pName))
  {
    if (_pOsServiceOptions->getOption(OsServiceOptions::logFileOption().pName, logFile) && !logFile.empty())
    {
      if (_pOsServiceOptions->hasOption(OsServiceOptions::logLevelOption().pName))
        _pOsServiceOptions->getOption(OsServiceOptions::logLevelOption().pName, logLevel, logLevel);

      logLevel = normalizeLogLevel(logLevel);

      Os::LoggerHelper::instance().processName = _appData._appName;

      if (!Os::LoggerHelper::instance().initialize(logLevel, logFile.c_str()))
      {
        displayVersion(std::cerr);
        displayUsage(std::cerr);
        std::cerr << std::endl << "ERROR: Unable to create log file " << logFile << "!" << std::endl;
        std::cerr.flush();

        _exit(-1);
      }

      initialized = true;
    }
  }
}

// Initialize the OsSysLog
void SipXApplication::initLogger()
{
  bool initialized = false;

  initLoggerByCommandLine(initialized);

  if (initialized == false)
    initLoggerByConfigurationFile();
}

bool& SipXApplication::terminationRequested()
{
  static bool initialized = false;
  if (!initialized)
  {
    if (OsTaskBase::unBlockSignals() != OS_SUCCESS)
    {
      OS_LOG_ERROR(FAC_KERNEL, "Unable to unblock signals");
    }

    initialized = true;
  }

  return Os::UnixSignals::instance().isTerminateSignalReceived();
}

void SipXApplication::waitForTerminationRequest(int seconds)
{
  while (!terminationRequested())
  {
    sleep(seconds);
  }

  std::cout << "Termination Signal RECEIVED" << std::endl;
}

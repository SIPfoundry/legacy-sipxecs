#include "sipXecsService/SipXApplication.h"
#include <boost/bind.hpp>

#include "os/OsResourceLimit.h"

#include "os/OsServiceOptions.h"

#include <os/OsExceptionHandler.h>

#include <sipdb/MongoDB.h>
#include <mongo/util/log.h>

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

SipXApplication::SignalTask::SignalTask(int shutdownSignal) : OsTask("SignalTask"),
                                      _shutdownSignal(shutdownSignal)
{
  //OS_LOG_DEBUG(FAC_SIP, "SipXApplication::SignalTask::" << __FUNCTION__);
}

SipXApplication::SignalTask::~SignalTask()
{
  //OS_LOG_DEBUG(FAC_SIP, "SipXApplication::SignalTask::" << __FUNCTION__);
}

int SipXApplication::SignalTask::run(void* arg)
{
  int sigNum = -1;
  OsStatus status = OS_FAILED;

//  OS_LOG_INFO(FAC_SIP, "SipXApplication::SignalTask::" << __FUNCTION__
//      << " started");

  while (!Os::detail::_is_termination_signal_received() && sigNum != _shutdownSignal)
  {
    status = awaitSignal(sigNum);
    if (OS_SUCCESS != status)
    {
      OS_LOG_ERROR(FAC_SIP, "SipXApplication::SignalTask::" << __FUNCTION__
           << " awaitSignal failed. errno: " << errno);
       continue;
    }

    OS_LOG_DEBUG(FAC_SIP, "SipXApplication::SignalTask::" << __FUNCTION__
        << " awaitSignal successfully called");

    Os::detail::_handle_signal(sigNum);
  }

  OS_LOG_INFO(FAC_SIP, "SipXApplication::SignalTask::" << __FUNCTION__
      << " terminated");

  return 0;
}

void SipXApplication::startSignalTaskThread()
{
  _signalTask.reset(new SignalTask(signalHandlerShutdownSignal));
  _signalTask->start();
}

void SipXApplication::shutdownSignalTaskThread()
{
  if (!_signalTask || terminationRequested())
  {
    return;
  }

  pthread_t id;
  if (OS_SUCCESS != _signalTask->id(id))
  {
    return;
  }

  int rc = pthread_kill(id, signalHandlerShutdownSignal);
  if (0 != rc)
  {
    OS_LOG_ERROR(FAC_SIP, "SipXApplication::SignalTask::" << __FUNCTION__
                 << " shutting down signal processing thread failed. Error: " << rc);
  }
}

void SipXApplication::doBlockSignals()
{
  signalHandlerShutdownSignal = SIGRTMIN + 1;
  if (signalHandlerShutdownSignal >= SIGRTMAX)
  {
    fprintf(stderr, "Unable to initialize signal handler shutdown signal\n");
    return;
  }

  if (OS_SUCCESS != OsTaskBase::blockSignals())
  {
    fprintf(stderr, "Unable to block signals\n");
    return;
  }

  // if signals blocking succeeded start the signal processing thread
  startSignalTaskThread();
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

  if (_appData._blockSignals)
  {
    doBlockSignals();
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

  // initialize the Mongo client driver
  mongo::Status status = mongo::client::initialize();
  if (!status.isOK())
  {
    fprintf(stderr, "Failed to initialize Mongo client driver: %s\n", status.toString().c_str());
    OS_LOG_ERROR(FAC_ODBC, "Failed to initialize Mongo client driver: " << status.toString());
    exit(1);
  }

  // Raise the file handle limit to maximum allowable
  if (_appData._increaseResourceLimits)
  {
    increaseResourceLimits();
  }

  if (_appData._checkMongo)
  {
   if (!testMongoDBConnection())
   {
     mongo::client::shutdown();
     exit(1);
   }
  }

  if (_appData._enableMongoDriverLogging)
  {
    enableMongoDriverLogging();
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

  // Clear log appenders
  mongo::logger::globalLogDomain()->clearAppenders();

  mongo::Status status = mongo::client::shutdown();
  if (!status.isOK())
  {
    OS_LOG_WARNING(FAC_ODBC, "Failed to shutdown Mongo client driver: " << status.toString());
  }
}

void SipXApplication::initLoggerByConfigurationFile()
{
  UtlString logLevel;          // Controls Log Verbosity
  UtlString consoleLogging;      // Enable console logging by default?
  UtlString fileTarget;         // Path to store log file.
  UtlBoolean bSpecifiedDirError ;  // Set if the specified log dir does not
  std::ostringstream alarmLogfile; // Where to dump alarms
                        // exist

  // If no log file is given don't start logger
  if (_appData._logFilename.empty())
    return;

  Os::LoggerHelper::instance().setProcessName(_appData._appName.c_str());

  std::string configSettingLogDir = _appData._configPrefix + "_LOG_DIR";
  std::string configSettingLogConsole = _appData._configPrefix + "_LOG_CONSOLE";
  std::string configSettingLogFormat = _appData._configPrefix + "_LOG_FORMAT";

  //
  // Set active log format
  //
  UtlString filterNames;
  if ((_pOsServiceOptions->getOption(configSettingLogFormat, filterNames) == OS_SUCCESS))
  {
    Os::LoggerHelper::instance().setFilterNames(filterNames.data());
  }
  else
  {
    Os::LoggerHelper::instance().setFilterNames(DEFAULT_LOG_FORMAT);
  }

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
    
    alarmLogfile << workingDirectory.data() << OsPathBase::separator << _appData._appName << "-alarms.log";
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
    
    alarmLogfile << fileTarget.data() << OsPathBase::separator << _appData._appName << "-alarms.log";
  }


  //
  // Get/Apply Log Level
  //
  SipXecsService::setLogPriority(_pOsServiceOptions->getOsConfigDb(), _appData._configPrefix.c_str());
  Os::Logger::instance().setLoggingPriorityForFacility(FAC_SIP_INCOMING_PARSED, PRI_ERR);
  Os::LoggerHelper::instance().initialize(fileTarget.data(), alarmLogfile.str().c_str());

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

      Os::LoggerHelper::instance().setProcessName(_appData._appName.c_str());
      Os::LoggerHelper::instance().setFilterNames(DEFAULT_LOG_FORMAT);

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

// Converts to Mongo log severity
// Note: PRI_INFO and PRI_NOTICE are intentionally mapped to Log() and, respectively,
//       Info().
static mongo::logger::LogSeverity convertToMongoLogSeverity(int priority)
{
  switch (priority)
  {
  case PRI_DEBUG:
    return mongo::logger::LogSeverity::Debug(3);
  case PRI_INFO:
    return mongo::logger::LogSeverity::Log();
  case PRI_NOTICE:
    return mongo::logger::LogSeverity::Info();
  case PRI_WARNING:
    return mongo::logger::LogSeverity::Warning();
  case PRI_ERR:
    return mongo::logger::LogSeverity::Error();
  default:
    return mongo::logger::LogSeverity::Severe();
  }
}

// Converts from Mongo log severity
// Note: PRI_INFO and PRI_NOTICE are intentionally mapped to Log() and, respectively,
//       Info().
static int convertFromMongoLogSeverity(mongo::logger::LogSeverity severity)
{
  if (mongo::logger::LogSeverity::Severe() == severity)
  {
    return PRI_CRIT;
  }
  else if (mongo::logger::LogSeverity::Error() == severity)
  {
    return PRI_ERR;
  }
  else if (mongo::logger::LogSeverity::Warning() == severity)
  {
    return PRI_WARNING;
  }
  else if (mongo::logger::LogSeverity::Info() == severity)
  {
    return PRI_NOTICE;
  }
  else if (mongo::logger::LogSeverity::Log() == severity)
  {
    return PRI_INFO;
  }
  else
  {
    return PRI_DEBUG;
  }
}

// Mongo Client appender used to intercept driver's logs
mongo::Status SipXApplication::MongoClientLogAppender::append(const mongo::logger::MessageLogDomain::EventAppender::Event& event)
{
  int priority = convertFromMongoLogSeverity(event.getSeverity());

  // ignore all lower-priority logs
  if (!Os::Logger::instance().willLog(priority))
  {
    return mongo::Status::OK();
  }

  // shape the stream like: <[ContextName - ]LogMessage>
  std::ostringstream strm;

  // prefix the log message with its context (if any)
  if (!event.getContextName().empty())
  {
    strm << event.getContextName().toString() << " - ";
  }

  // set the actual message
  strm << event.getMessage().toString();

  // log the actual message
  Os::Logger::instance().log(FAC_MONGO_CLIENT, priority, "%s", strm.str().c_str());

  return mongo::Status::OK();
}

void SipXApplication::enableMongoDriverLogging() const
{
  std::string mongoClientIniFilePath = SIPX_CONFDIR "/mongo-client.ini";
  OsServiceOptions mongoClientConfig(mongoClientIniFilePath);

  mongoClientConfig.addOptionString(0, "enable-driver-logging", "", OsServiceOptions::ConfigOption, false);

  if (mongoClientConfig.parseOptions())
  {
    bool enableDriverLogging = true;
    mongoClientConfig.getOption("enable-driver-logging", enableDriverLogging);
    OS_LOG_INFO(FAC_SIP, "SipXApplication::enableMongoDriverLogging Enable mongo driver logging = " << enableDriverLogging);

    if (enableDriverLogging)
    {
      int priority = Os::Logger::instance().getLevel();
      mongo::logger::globalLogDomain()->setMinimumLoggedSeverity(convertToMongoLogSeverity(priority));

      // clear all appenders in order to add ours
      mongo::logger::globalLogDomain()->clearAppenders();
      mongo::logger::globalLogDomain()->attachAppender(mongo::logger::MessageLogDomain::AppenderAutoPtr(new MongoClientLogAppender()));
    }
  }
  else
  {
    OS_LOG_ERROR(FAC_SIP, "SipXApplication::enableMongoDriverLogging Failed parsing mongo client init file: " << mongoClientIniFilePath);
  }
}

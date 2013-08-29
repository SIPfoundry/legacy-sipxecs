#include "sipXecsService/SipXApplication.h"
#include <boost/bind.hpp>

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


bool SipXApplication::init(int argc, char* argv[], const SipXApplicationData& appData)
{
  _appData = appData;

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
  initSysLog(&_configDb);

  std::set_terminate(catch_global);

  if (_appData._checkMongo)
  {
   if (!testMongoDBConnection())
   {
     mongo::dbexit(mongo::EXIT_CLEAN);
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

  if (OS_SUCCESS == _configDb.loadFromFile(fileName))
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
     Os::Logger::instance().log(FAC_KERNEL, PRI_CRIT,
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
void SipXApplication::initSysLog(OsConfigDb* pConfig)
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
  if ((pConfig->get(configSettingLogDir.c_str(), fileTarget) != OS_SUCCESS) ||
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
  SipXecsService::setLogPriority(_configDb, _appData._configPrefix.c_str());
  Os::Logger::instance().setLoggingPriorityForFacility(FAC_SIP_INCOMING_PARSED, PRI_ERR);
  Os::LoggerHelper::instance().initialize(fileTarget);

  //
  // Get/Apply console logging
  //
  UtlBoolean bConsoleLoggingEnabled = false;
  if ((pConfig->get(configSettingLogConsole.c_str(), consoleLogging) == OS_SUCCESS))
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

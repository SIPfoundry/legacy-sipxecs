#ifndef _SIPXECS_APPLICATION_H_
#define _SIPXECS_APPLICATION_H_

// APPLICATION INCLUDES
#include <os/OsServiceOptions.h>
#include <os/OsFS.h>
#include <os/OsLogger.h>
#include <os/UnixSignals.h>
#include <os/OsTimer.h>
#include <os/OsLogger.h>
#include <os/OsLoggerHelper.h>
#include <os/OsMsgQ.h>
#include <utl/Instrumentation.h>
#include <sipXecsService/SipXecsService.h>
#include <sipXecsService/daemon.h>

struct SipXApplicationData
{
  enum ConfigFileFormat
  {
    ConfigFileFormatIni,        // configuration file format ini
    ConfigFileFormatConfigDb    // configuration file format config db
  };

  std::string _appName;           // Application name
  std::string _configFilename;    // Configuration file name
  std::string _logFilename;       // Log file name
  std::string _nodeFilename;      // Node file name
  std::string _configPrefix;      // Configuration prefix

  bool _checkMongo;               // Tells if mongo connection will be check or not
  bool _enableMongoDriverLogging;
  bool _increaseResourceLimits;   // Tells if file descriptor limits should be increased or not
  bool _blockSignals;             // Tells if the signals should be blocked in init function or
                                  // not. If the signals are blocked a dedicated thread will be used
                                  // for signal handling


  // Configuration file format. This option tells parser what format of configuration file is used
  ConfigFileFormat _configFileFormat;

  // Queue preference
  OsMsgQShared::QueuePreference _queuePreference;
};

class SipXApplication
  {
  public:
    // returns a static reference for SipXApplication class
    static SipXApplication& instance();

    /**
     * Application initialization function
     *
     * Currently this function does the following operations: sets exception handling,
     *  doemonize, registers signal handlers, load the configuration file, initialize
     *  the logger, raise the file handle limit, check mongo connection
     *
     * @param argc - Number of arguments
     * @param argv - Array of command line arguments
     * @param appData - Reference to SipXApplicationData initialization structure
     * @param pOptions - A pointer to an external OsServiveOptions instance
     * @return - True in case of success or exits with error code 1 otherwise
     */
    bool init(int argc, char* argv[], const SipXApplicationData& appData, OsServiceOptions* pOptions = 0);

    /*
     * Loads configuration file.
     *
     * The configuration is loaded in osServiceOptions parameter.
     *
     * @param osServiceOptions - OsServiceOptions class parameter passed by reference
     * @param configFilename - Configuration file name
     * @return - true in case of success or false otherwise
     */
    bool loadConfiguration(OsServiceOptions& osServiceOptions, const std::string& configFilename = "");

    /**
     * Test mongo connection
     * @return - True if the connection to monogo database is successful or false otherwise
     */
    bool testMongoDBConnection();

    /**
     * Increase file descriptor limits (including file core limit) for current application
     *
     * @return true if resource limit was successfully increased or false otherwise
     */
    bool increaseResourceLimits(const std::string& configurationPath = std::string());

    /**
     * Checks if termination was requested
     * @return true if one of the following signals was received: SIGINT, SIGQUIT, SIGABRT, SIGKILL, SIGTERM
     */
    bool& terminationRequested();

    /**
     * Force the application to close. It cancel all timers and waits the threads to terminate
     */
    void terminate();

    /**
     * Wait for termination request
     *
     * @param - Number of seconds to wait before checking again if termination was requested
     */
    void waitForTerminationRequest(int seconds);

    // Returns a reference to OsServiceOptions class
    OsServiceOptions& getConfig();

    // Returns a reference to _nodeFilePath string
    const std::string& getNodeFilePath();

    /**
     * Displays help for the application
     * @param strm - the stream used for displaying help
     */
    void displayUsage(std::ostream& strm);

    /**
     * Displays the application version
     * @param strm - the stream used for displaying the version
     */
    void displayVersion(std::ostream& strm) const;

    /**
     * Doemonize the application
     *
     * @param argc - Number of arguments
     * @param argv - Array of command line arguments
     */
    static void doDaemonize(int argc, char** pArgv);

    static int normalizeLogLevel(int logLevel);

  private:
    // The purpose of this class is to handle signals
    // The signals will be blocked on all other application threads
    // and should be processed only on this thread
    class SignalTask : public OsTask
    {
    public:
       SignalTask(int shutdownSignal);
       ~SignalTask();

       int run(void* arg);
    private:
       int _shutdownSignal;
    };

    SipXApplication();                                       // SipXApplication constructor
    ~SipXApplication();                                      // SipXApplication destructor
    SipXApplication(const SipXApplication&);                 // Copy constructor disabled
    SipXApplication& operator=(const SipXApplication&);      // Assignment operator disabled

    /**
     * Check if version or help options are present in command line. If presents the application
     * shows help or version and then exits with return code 0
     */
    void showVersionHelp();

    /**
     * Registers signal handlers. Currently are registered signal handlers for the following signals:
     * SIGHUP, SIGPIPE, SIGTERM, SIGINT, SIGQUIT, SIGABRT, SIGUSR1, SIGUSR2
     */
    void registerSignalHandlers();

    /*
     * Parse both command and configuration file.
     *
     * @param osServiceOptions - OsServiceOptions class parameter passed by reference
     * @param configFilename - Configuration file name
     * @return - true in case of success or false otherwise
     */
    bool parse(OsServiceOptions& osServiceOptions, int argc, char** pArgv, const std::string& configFilename);

    // blocks signals on main thread (and on all other thread created by main) and create
    // a dedicated thread to process them
    void doBlockSignals();

    // starts signal processing thread
    void startSignalTaskThread();

    // stops signal processing thread
    void shutdownSignalTaskThread();

    /**
     * Initializes logger
     */
    void initLogger();

    /**
     * Initializes logger using the parameters from command line
     */
    void initLoggerByCommandLine(bool& initialized);

    /**
     * Initializes logger using the parameters from configuration file
     */
    void initLoggerByConfigurationFile();

    // Signal handler function called when SIGHUP is received. Currently the action is to reload configuration
    void handleSIGHUP();

    // Signal handler function called when SIGPIPE is received. Currently SIGPIPE is ignored.
    void handleSIGPIPE();

    /**
     * Signal handler function called when SIGTERM, SIGINT, SIGQUIT, SIGABRT, SIGUSR1 or SIGUSR2 is received.
     * Currently the action is to log this situation
     */
    void handleTerminationSignal();

    /**
     * Signal handler function called when SIGUSR1 is received. Currently the action is to start portlib instrumentation
     */
    void handleSIGUSR1();

    /**
      * Signal handler function called when SIGUSR2 is received. Currently the action is to start portlib instrumentation
      */
    void handleSIGUSR2();

    void enableMongoDriverLogging() const;

    OsServiceOptions* _pOsServiceOptions; // Configuration Database (used for OsSysLog)
    SipXApplicationData _appData;       // SipXApplicationData structure
    bool _initialized;                  // Is set true after this class is initilized by init function
    std::string _nodeFilePath;          // Node file path
    int _argc;
    char** _argv;
    bool _autoDeleteConfig;
    boost::scoped_ptr<SignalTask> _signalTask;
    int signalHandlerShutdownSignal;
  };

inline SipXApplication& SipXApplication::instance()
{
  static SipXApplication sipxapp;

  return sipxapp;
}

inline OsServiceOptions& SipXApplication::getConfig()
{
  return *_pOsServiceOptions;
}

inline const std::string& SipXApplication::getNodeFilePath()
{
  return _nodeFilePath;
}

inline SipXApplication::SipXApplication()
  : _initialized(false),
    _argc(0),
    _argv(0),
    _autoDeleteConfig(true),
    _signalTask(0),
    signalHandlerShutdownSignal(0)
{
  _pOsServiceOptions = new OsServiceOptions();
}

inline SipXApplication::~SipXApplication()
{
  if (_autoDeleteConfig)
    delete _pOsServiceOptions;

  shutdownSignalTaskThread();
}

// reload config on sighup
inline void SipXApplication::handleSIGHUP()
{
  if (_initialized)
  {
    OsServiceOptions osServiceOptions;
    Os::Logger::instance().log(FAC_SIP, PRI_INFO, "SIGHUP caught. Reloading configuration.");

    osServiceOptions.addDefaultOptions();
    loadConfiguration(osServiceOptions);

    // try first to change level for ConfigFileFormatIni
    if (osServiceOptions.hasOption(OsServiceOptions::logLevelOption().pName))
    {
      int logLevel = PRI_NOTICE;

      osServiceOptions.getOption(OsServiceOptions::logLevelOption().pName, logLevel, logLevel);

      logLevel = SYSLOG_NUM_PRIORITIES - logLevel - 1;
      Os::Logger::instance().setLogPriority(logLevel);

      return;
    }

    // if 'log-level' option was not found try then to change level for ConfigFileFormatConfigDb
    SipXecsService::setLogPriority(osServiceOptions.getOsConfigDb(), _appData._configPrefix.c_str());
  }
}

// ignore sigpipe
inline void SipXApplication::handleSIGPIPE()
{
  // ignore
  Os::Logger::instance().log(FAC_SIP, PRI_INFO, "SIGPIPE caught. Ignored.");
};

inline void SipXApplication::handleTerminationSignal()
{
  Os::Logger::instance().log(FAC_SIP, PRI_INFO, "SIGTERM caught. Shutting down.");
}

inline void SipXApplication::handleSIGUSR1()
{
      system_tap_start_portlib_instrumentation(true/*Bactrace enabled*/);
      Os::Logger::instance().log(FAC_SIP, PRI_NOTICE, "SIGUSR1 caught. Starting instrumentations.");
}

inline void SipXApplication::handleSIGUSR2()
{
      system_tap_stop_portlib_instrumentation();
      Os::Logger::instance().log(FAC_SIP, PRI_NOTICE, "SIGUSR2 caught. Starting instrumentations.");
}

inline int SipXApplication::normalizeLogLevel(int logLevel)
{
  return SYSLOG_NUM_PRIORITIES - logLevel - 1;
}

#endif /* _SIPXECS_APPLICATION_H_ */

#ifndef _SIPXECS_APPLICATION_H_
#define _SIPXECS_APPLICATION_H_

// APPLICATION INCLUDES
#include <os/OsConfigDb.h>
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
#include <sipdb/MongoDB.h>

struct SipXApplicationData
{
  std::string _appName;
  std::string _configFilename;
  std::string _logFilename;
  std::string _nodeFilename;
  std::string _configPrefix;

  bool _daemonize;
  bool _checkMongo;

  OsMsgQShared::QueuePreference _queuePreference;
};

class SipXApplication
  {
  public:
    static SipXApplication& instance();

    bool init(int argc, char* argv[], const SipXApplicationData& appData);

    bool reloadConfiguration(const std::string& configFilename = "");

    bool testMongoDBConnection();

    bool& terminationRequested();

    void terminate();

    OsConfigDb& getConfigDb();

    const std::string& getNodeFilePath();

  private:
    SipXApplication();
    ~SipXApplication() {}
    SipXApplication(const SipXApplication&);
    SipXApplication& operator=(const SipXApplication&);

    void doDaemonize(int argc, char* argv[]);
    void registrerSignalHandlers();
    bool loadConfiguration(const std::string& configFilename);
    void initSysLog(OsConfigDb* appConfigDb);

    // reload config
    void handleSIGHUP();
    // ignore sigpipe
    void handleSIGPIPE();
    void handleTerminationSignal();
    void handleSIGUSR1();
    void handleSIGUSR2();

    OsConfigDb _configDb; // Configuration Database (used for OsSysLog)
    SipXApplicationData _appData;
    bool _initialized;
    std::string _nodeFilePath;
  };

inline SipXApplication& SipXApplication::instance()
{
  static SipXApplication sipxapp;

  return sipxapp;
}

inline OsConfigDb& SipXApplication::getConfigDb()
{
  return _configDb;
}


inline const std::string& SipXApplication::getNodeFilePath()
{
  return _nodeFilePath;
}

SipXApplication::SipXApplication()
  : _initialized(false)
{
};

// reload config on sighup
inline void SipXApplication::handleSIGHUP()
{
  if (_initialized)
  {
    Os::Logger::instance().log(FAC_SIP, PRI_INFO, "SIGHUP caught. Reloading configuration.");
    reloadConfiguration();
    SipXecsService::setLogPriority(_configDb, _appData._configPrefix.c_str());
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

#endif /* _SIPXECS_APPLICATION_H_ */

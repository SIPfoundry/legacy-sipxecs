

#ifndef FREESWITCHRUNNER_H
#define	FREESWITCHRUNNER_H


#include <string>
#include <sstream>
#include <boost/thread.hpp>
#include <OSS/Exec/Process.h>


namespace sipx {
namespace bridge {


class FreeSwitchRunner
{
public:
  static std::string _gFreeswitchXml;
  
  static FreeSwitchRunner* instance();
    /// Returns the singleton instance of the freeswitch runner
  
  static void delete_instance();
    /// Delete the singleton instance

  bool initialize();
    /// Initialize the freeswitch instance
    /// Returns:
    /// - true if initialization was a success.
    /// Required:
    /// - must be called prior to calling run
  
  static void daemonize(int *fds = 0);
    /// Run freeswitch as a daemon.
    /// Parameters:
    //  - fds : FDSET of the chiled process.  This variable is not used.
    /// Required:
    /// - must be called after initialize and before run methods are invoked.
  
  void run();
    /// Start the freeswitch instance.  Will exit the process on failure
    /// Required:
    /// - runner must already be initialized
  
  void stop();
    /// Stop the freeswitch instance
   
  void setConfigDirectory(const std::string& configDirectory);
    /// Set the data directory where freesswitch looks for configuration files.
    /// Parameter:
    /// - configDirectory : string containing the path to the directory
    /// Required:
    /// - The process must have read and write access to this folder.
    /// - The application name must also be set.
  
  const std::string& getConfigDirectory() const;
    /// Return the config directory where freeswitch reads its configurations
    /// Default: /tmp/$(app_name)_data/config
  
  void setApplicationName(const std::string& applicationName);
    /// Set the application name for this instance.  This is normally the same 
    /// as the process name.  This is used to identify configuration and data path folders.
    /// Parameters:
    /// - applicationName : The application name.
  
  const std::string& getApplicationName() const;
    /// Returns the application name.
  
  void setModulesDirectory(const std::string& modulesDirectory);
    /// Sets the path where the freeswitch modules are located
    /// Parameters:
    /// - modulesDirectory : String containing the path of the modules directory
  
  const std::string& getModulesDirectory() const;
    /// Returns the module directory of freeswitch
  
  void setLogDirectory(const std::string& logDirectory);
    /// Set the path where freeswitch will store the log file
    /// Parameters:
    /// - logDirectory : String containing the path to the log directory
    /// Defaults:
    /// - logDirectory : /var/log/$(app_name)
  
  const std::string& getLogDirectory() const;
    /// Returns the path of the log directory
  
  void setRunDirectory(const std::string& runDirectory);
    /// Sets the path where freeswitch will dump runtime data such as pid file
    /// Parameters:
    /// - runDirectory : String containing the path of the run directory
  
  const std::string& getRunDirectory() const;
    /// Returns the path of the run directory
  
  void setDataDirectory(const std::string& dataDirectory);
    /// Sets the directory path where freeswitch will store its data
    /// Parameters:
    /// - dataDirectory : The path pointing to the data directory
  
  const std::string& getDataDirectory() const;
    /// Returns the path of the Data Directory
    
  void setEventSocketLayerPort(int eslPort);
    /// Set the ESL port where freeswitch and the runner will communicate
    /// Parameters:
    /// - eslPort : The ESL port where the runner would listen for connections
    /// Defaults:
    /// - eslPort : 2022
  
  int getEventSocketLayerPort() const;
    /// Returns the port where the runner is listening for ESL connection
  
  void setSwitchEventSocketLayerPort(int eslPort);
    /// Set the ESL port where freeswitch is listening for inbound ESL connection
    /// Parameters:
    /// - eslPort : The ESL port where the switch would listen for connections
    /// Defaults:
    /// - eslPort : 2020
  
  int getSwitchEventSocketLayerPort() const;
    /// Returns the port where the switch is listening for ESL connection
  
  void setCodecPreference(const std::string& codecPreference);
    /// Set the inbound and outbount codec prefrence
    /// Parameters: 
    /// - codecPreference : Comma delimited list of codecs
  
  const std::string& getCodecPreference() const;
    /// Returns the codec preference list
  
  void setSipPort(int sipPort);
    /// Set the port where freeswitch would listen for TCP/UDP connections
    /// Parameters:
    /// - sipPort : Port number.  
    /// Defaults:
    /// - sipPort :  5066
  
  int getSipPort() const;
    /// Returns the port where freeswitch is listening for TCP/UDP connections
  
  void setSipAddress(const std::string& sipAddress);
    /// Set the IP address for the SIP transport
    /// Parameters:
    /// - sipAddress : String containing the IP address for SIP.
    /// Defaults:
    /// - sipAddress : $${local_ip_v4}
  
  const std::string& getSipAddress() const;
    /// Return the IP address of the SIP transport
  
protected: 
  bool generateConfig();
    
  OSS::Exec::Process::Action onDeadProcess(int consecutiveHits);
  
private:
  FreeSwitchRunner();
  ~FreeSwitchRunner();
  std::string _applicationName;
  std::string _configString;
  std::string _configDirectory;
  std::string _modDirectory;
  std::string _logDirectory;
  std::string _runDirectory;
  std::string _dataDirectory;
  std::string _storageDirectory;
  std::string _scriptsDirectory;
  std::string _htdocsDirectory;
  std::string _recordingsDirectory;
  std::string _grammarDirectory;
  std::string _certsDirectory;
  std::string _soundsDirectory;
  int _eslPort;
  int _fsEslPort;
  std::string _codecPreference;
  int _sipPort;
  std::string _sipAddress;
  OSS::Exec::Process* _pProcess;
  std::ostringstream _startupScript;
  std::ostringstream _shutdownScript;
  std::string _pidFile;
};


//
// Inlines
//

inline void FreeSwitchRunner::setConfigDirectory(const std::string& configDirectory)
{
  _configDirectory = configDirectory;
}

inline const std::string& FreeSwitchRunner::getConfigDirectory() const
{
  return _configDirectory;
}

inline void FreeSwitchRunner::setApplicationName(const std::string& applicationName)
{
  _applicationName = applicationName;
}

inline const std::string& FreeSwitchRunner::getApplicationName() const
{
  return _applicationName;
}

inline void FreeSwitchRunner::setModulesDirectory(const std::string& modulesDirectory)
{
  _modDirectory = modulesDirectory;
}

inline const std::string& FreeSwitchRunner::getModulesDirectory() const
{
  return _modDirectory;
}

inline void FreeSwitchRunner::setLogDirectory(const std::string& logDirectory)
{
  _logDirectory = logDirectory;
}
  
inline const std::string& FreeSwitchRunner::getLogDirectory() const
{
  return _logDirectory;
}

inline void FreeSwitchRunner::setRunDirectory(const std::string& runDirectory)
{
  _runDirectory = runDirectory;
}
  
inline const std::string& FreeSwitchRunner::getRunDirectory() const
{
  return _runDirectory;
}

inline void FreeSwitchRunner::setDataDirectory(const std::string& dbDirectory)
{
  _dataDirectory = dbDirectory;
}
  
inline const std::string& FreeSwitchRunner::getDataDirectory() const
{
  return _dataDirectory;
}

inline void FreeSwitchRunner::setEventSocketLayerPort(int eslPort)
{
  _eslPort = eslPort;
}
  
inline int FreeSwitchRunner::getEventSocketLayerPort() const
{
  return _eslPort;
}

inline void FreeSwitchRunner::setSwitchEventSocketLayerPort(int eslPort)
{
  _fsEslPort = eslPort;
}

inline int FreeSwitchRunner::getSwitchEventSocketLayerPort() const
{
  return _fsEslPort;
}

inline void FreeSwitchRunner::setCodecPreference(const std::string& codecPreference)
{
  _codecPreference = codecPreference;
}

inline const std::string& FreeSwitchRunner::getCodecPreference() const
{
  return _codecPreference;
}

inline void FreeSwitchRunner::setSipPort(int sipPort)
{
  _sipPort = sipPort;
}

inline int FreeSwitchRunner::getSipPort() const
{
  return _sipPort;
}

inline void FreeSwitchRunner::setSipAddress(const std::string& sipAddress)
{
  _sipAddress = sipAddress;
}
  
inline const std::string& FreeSwitchRunner::getSipAddress() const
{
  return _sipAddress;
}



} } // sipx::bridge


#endif	/* FREESWITCHRUNNER_H */


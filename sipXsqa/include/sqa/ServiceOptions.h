/*
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */

#ifndef SERVICEOPTIONS_H
#define	SERVICEOPTIONS_H

#include <cstdlib>
#include <cassert>
#include <csignal>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "os/OsLogger.h"

namespace Os
{
struct ServiceLogger
{
  std::string getHostName()
  {
    char hostname[1024];
    hostname[1023] = 0x00;
    gethostname(hostname, 1023);
    struct hostent* h = gethostbyname(hostname);
    if (h && h->h_name)
      return h->h_name;
    return "unqualified";
  }

  std::string getCurrentTask()
  {
    std::ostringstream strm;
    strm << "task-" << pthread_self();
    return strm.str();
  }

  std::string getProcessName()
  {
    return processName;
  }

  static ServiceLogger& instance()
  {
    static ServiceLogger T;
    return T;
  }

  bool initialize(int priorityLevel, const char* path)
  {
    return Logger::instance().initialize<ServiceLogger>(priorityLevel, path, *this);
  }

  bool initialize(const char* path)
  {
    return Logger::instance().initialize<ServiceLogger>(path, *this);
  }

  static bool parseLogString(const std::string& logData,
                           std::string& date,
                           std::string& eventCount,
                           std::string& facility,
                           std::string& priority,
                           std::string& hostName,
                           std::string& taskName,
                           std::string& taskId,
                           std::string& processId,
                           std::string& content)
  {
    enum parser_state
    {
      ParseDate,
      ParseEventCount,
      ParseFacility,
      ParsePriority,
      ParseHostName,
      ParseTaskName,
      ParseTaskId,
      ParseProcessId,
      ParseContent
    };

    parser_state state = ParseDate;
    for (std::string::const_iterator iter = logData.begin(); iter != logData.end(); iter++)
    {
      switch(state)
      {
      case ParseDate:
        if (*iter != ':')
          date += *iter;
        else
          state = ParseEventCount;
        break;
      case ParseEventCount:
        if (*iter != ':')
          eventCount += *iter;
        else
          state = ParseFacility;
        break;
      case ParseFacility:
        if (*iter != ':')
          facility += *iter;
        else
          state = ParsePriority;
        break;
      case ParsePriority:
        if (*iter != ':')
          priority += *iter;
        else
          state = ParseHostName;
        break;
      case ParseHostName:
        if (*iter != ':')
          hostName += *iter;
        else
          state = ParseTaskName;
        break;
      case ParseTaskName:
        if (*iter != ':')
          taskName += *iter;
        else
          state = ParseTaskId;
        break;
      case ParseTaskId:
        if (*iter != ':')
          taskId += *iter;
        else
          state = ParseProcessId;
        break;
      case ParseProcessId:
        if (*iter != ':')
          processId += *iter;
        else
          state = ParseContent;
        break;
      case ParseContent:
        if (*iter != ':')
          content += *iter;
        break;
      }
    }
    return state == ParseContent;
  }

  std::string hostName;
  std::string processName;
};
}

class ServiceOptions
{
public:
  enum OptionType
  {
    CommandLineOption,
    DaemonOption,
    ConfigOption
  };
  ServiceOptions(int argc, char** argv, const std::string& daemonName, const std::string& version = "1.0", const std::string& copyright = "All Rights Reserved.");
  ~ServiceOptions();
  //
  // Options processing
  //
  void addOptionFlag(char shortForm, const std::string& optionName, const std::string description, OptionType type = ConfigOption);
  void addOptionFlag(const std::string& optionName, const std::string description, OptionType type = ConfigOption);
  void addOptionString(char shortForm, const std::string& optionName, const std::string description, OptionType type = ConfigOption);
  void addOptionString(const std::string& optionName, const std::string description, OptionType type = ConfigOption);
  void addOptionStringVector(char shortForm, const std::string& optionName, const std::string description, OptionType type = ConfigOption);
  void addOptionStringVector(const std::string& optionName, const std::string description, OptionType type = ConfigOption);
  void addOptionInt(char shortForm, const std::string& optionName, const std::string description, OptionType type = ConfigOption);
  void addOptionInt(const std::string& optionName, const std::string description, OptionType type = ConfigOption);
  void addOptionIntVector(char shortForm, const std::string& optionName, const std::string description, OptionType type = ConfigOption);
  void addOptionIntVector(const std::string& optionName, const std::string description, OptionType type = ConfigOption);
  //
  // Standard daemon options
  //
  void addDaemonOptions();

  bool parseOptions();
  void displayUsage(std::ostream& strm) const;
  void displayVersion(std::ostream& strm) const;
  size_t hasOption(const std::string& optionName) const;
  bool getOption(const std::string& optionName, std::string& value, const std::string& defValue = std::string()) const;
  bool getOption(const std::string& optionName, std::vector<std::string>& value) const;
  bool getOption(const std::string& optionName, int& value) const;
  bool getOption(const std::string& optionName, int& value, int defValue) const;
  bool getOption(const std::string& optionName, std::vector<int>& value) const;

  virtual bool onParseUnknownOptions(int argc, char** argv) {return false;};
  void waitForTerminationRequest();

protected:
  void daemonize(const std::string& pidfile);
  void initlogger();
  int _argc;
  char** _argv;
  std::string _daemonName;
  std::string _version;
  std::string _copyright;

  boost::program_options::options_description _daemonOptions;
  boost::program_options::options_description _commandLineOptions;
  boost::program_options::options_description _configOptions;
  boost::program_options::options_description _optionItems;
  boost::program_options::variables_map _options;
  //
  // Pre-parsed values
  //
  bool _isDaemon;
  std::string _pidFile;
  std::string _configFile;
  boost::property_tree::ptree _ptree;
  bool _hasConfig;
  Os::ServiceLogger _logger;
};

inline ServiceOptions::ServiceOptions(int argc, char** argv,
  const std::string& daemonName,
  const std::string& version,
  const std::string& copyright) :
  _argc(argc),
  _argv(argv),
  _daemonName(daemonName),
  _version(version),
  _copyright(copyright),
  _daemonOptions("Daemon"),
  _commandLineOptions("Generic"),
  _configOptions("Configuration"),
  _optionItems(_daemonName  + " Options"),
  _isDaemon(false),
  _hasConfig(false)
{
}

//
// inlines
//

inline void ServiceOptions::addDaemonOptions()
{
  addOptionFlag('D', "daemonize", ": Run as system daemon.", DaemonOption);
  addOptionString('P', "pid-file", ": PID file when running as daemon.", DaemonOption);
}


inline bool ServiceOptions::parseOptions()
{
  try
  {
    addOptionFlag('h', "help", ": Display help information.", CommandLineOption);
    addOptionFlag('v', "version", ": Display version information.", CommandLineOption);
    addOptionString('C', "config-file", ": Optional daemon config file.", CommandLineOption);
    addOptionString('L', "log-file", ": Specify the application log file.", CommandLineOption);
    addOptionInt('l', "log-level",
      ": Specify the application log priority level."
      "Valid level is between 0-7.  "
      "0 (EMERG) 1 (ALERT) 2 (CRIT) 3 (ERR) 4 (WARNING) 5 (NOTICE) 6 (INFO) 7 (DEBUG)"
            , CommandLineOption);

    _optionItems.add(_commandLineOptions);
    _optionItems.add(_daemonOptions);
    _optionItems.add(_configOptions);


    boost::program_options::store(boost::program_options::parse_command_line(_argc, _argv, _optionItems), _options);
    boost::program_options::notify(_options);

    if (hasOption("help"))
    {
      displayUsage(std::cout);
      exit(0);
    }

    if (hasOption("version"))
    {
      displayVersion(std::cout);
      exit(0);
    }

    if (hasOption("pid-file"))
      getOption("pid-file", _pidFile);

    if (hasOption("daemonize"))
    {
      if (_pidFile.empty())
      {
        displayUsage(std::cerr);
        std::cerr << std::endl << "ERROR: You must specify pid-file location!" << std::endl;
        std::cerr.flush();
        exit(-1);
      }
      _isDaemon = true;
    }

    if (hasOption("config-file"))
    {
      if (getOption("config-file", _configFile) && !_configFile.empty())
      {
        std::ifstream config(_configFile.c_str());
        if (config.good())
        {
          boost::program_options::store(boost::program_options::parse_config_file(config, _optionItems, true), _options);
          boost::program_options::notify(_options);
          boost::property_tree::ini_parser::read_ini(_configFile.c_str(), _ptree);
          _hasConfig = true;
        }
        else
        {
          displayUsage(std::cerr);
          std::cerr << std::endl << "ERROR: Unable to open input file " << _configFile << "!" << std::endl;
          std::cerr.flush();
          exit(-1);
        }
      }
    }
  }
  catch(const std::exception& e)
  {
    if (!onParseUnknownOptions(_argc, _argv))
    {
      std::cerr << _daemonName << "is not able to parse the options - " << e.what() << std::endl;
      return false;
    }
  }

  displayVersion(std::cout);
  if (_isDaemon)
    daemonize(_pidFile);

  initlogger();
  return true;
}

inline void ServiceOptions::initlogger()
{
  std::string logFile;
  int priorityLevel = PRI_INFO;
  if (hasOption("log-file"))
  {
    if (getOption("log-file", logFile) && !logFile.empty())
    {
      if (hasOption("log-level"))
        getOption("log-level", priorityLevel, priorityLevel);

      int logLevel = SYSLOG_NUM_PRIORITIES - priorityLevel - 1;
      if (logLevel < 0)
        logLevel = 0;

      _logger.processName = _daemonName;

      if (!_logger.initialize(logLevel, logFile.c_str()))
      {
        displayUsage(std::cerr);
        std::cerr << std::endl << "ERROR: Unable to create log file " << logFile << "!" << std::endl;
        std::cerr.flush();
        _exit(-1);
      }
    }
  }
  else
  {
    displayUsage(std::cerr);
    std::cerr << std::endl << "ERROR: Log file not specified!" << std::endl;
    std::cerr.flush();
    _exit(-1);
  }
}


inline void ServiceOptions::addOptionFlag(char shortForm, const std::string& optionName, const std::string description, OptionType type)
{
  boost::program_options::options_description* options;
  if (type == CommandLineOption)
    options = &_commandLineOptions;
  else if (type == DaemonOption)
    options = &_daemonOptions;
  else if (type == ConfigOption)
    options = &_configOptions;
  else
    assert(false);

  std::ostringstream strm;
  strm << optionName << "," << shortForm;
  options->add_options()(strm.str().c_str(), description.c_str());
}

inline void ServiceOptions::addOptionFlag(const std::string& optionName, const std::string description, OptionType type)
{
  boost::program_options::options_description* options;
  if (type == CommandLineOption)
    options = &_commandLineOptions;
  else if (type == DaemonOption)
    options = &_daemonOptions;
  else if (type == ConfigOption)
    options = &_configOptions;
  else
    assert(false);
  
  options->add_options()(optionName.c_str(), description.c_str());
}

inline void ServiceOptions::addOptionString(char shortForm, const std::string& optionName, const std::string description, OptionType type)
{
  boost::program_options::options_description* options;
  if (type == CommandLineOption)
    options = &_commandLineOptions;
  else if (type == DaemonOption)
    options = &_daemonOptions;
  else if (type == ConfigOption)
    options = &_configOptions;
  else
    assert(false);

  std::ostringstream strm;
  strm << optionName << "," << shortForm;
  options->add_options()(strm.str().c_str(), boost::program_options::value<std::string>(), description.c_str());
}

inline void ServiceOptions::addOptionString(const std::string& optionName, const std::string description, OptionType type)
{
  boost::program_options::options_description* options;
  if (type == CommandLineOption)
    options = &_commandLineOptions;
  else if (type == DaemonOption)
    options = &_daemonOptions;
  else if (type == ConfigOption)
    options = &_configOptions;
  else
    assert(false);

  options->add_options()(optionName.c_str(), boost::program_options::value<std::string>(), description.c_str());
}

inline void ServiceOptions::addOptionStringVector(char shortForm, const std::string& optionName, const std::string description, OptionType type)
{
  boost::program_options::options_description* options;
  if (type == CommandLineOption)
    options = &_commandLineOptions;
  else if (type == DaemonOption)
    options = &_daemonOptions;
  else if (type == ConfigOption)
    options = &_configOptions;
  else
    assert(false);

  std::ostringstream strm;
  strm << optionName << "," << shortForm;
  options->add_options()(strm.str().c_str(), boost::program_options::value<std::vector<std::string> >(), description.c_str());
}

inline void ServiceOptions::addOptionStringVector(const std::string& optionName, const std::string description, OptionType type)
{
  boost::program_options::options_description* options;
  if (type == CommandLineOption)
    options = &_commandLineOptions;
  else if (type == DaemonOption)
    options = &_daemonOptions;
  else if (type == ConfigOption)
    options = &_configOptions;
  else
    assert(false);

  options->add_options()(optionName.c_str(), boost::program_options::value<std::vector<std::string> >(), description.c_str());
}

inline void ServiceOptions::addOptionInt(char shortForm, const std::string& optionName, const std::string description, OptionType type)
{
  boost::program_options::options_description* options;
  if (type == CommandLineOption)
    options = &_commandLineOptions;
  else if (type == DaemonOption)
    options = &_daemonOptions;
  else if (type == ConfigOption)
    options = &_configOptions;
  else
    assert(false);

  std::ostringstream strm;
  strm << optionName << "," << shortForm;
  options->add_options()(strm.str().c_str(), boost::program_options::value<int>(), description.c_str());
}

inline void ServiceOptions::addOptionInt(const std::string& optionName, const std::string description, OptionType type)
{
  boost::program_options::options_description* options;
  if (type == CommandLineOption)
    options = &_commandLineOptions;
  else if (type == DaemonOption)
    options = &_daemonOptions;
  else if (type == ConfigOption)
    options = &_configOptions;
  else
    assert(false);

  options->add_options()(optionName.c_str(), boost::program_options::value<int>(), description.c_str());
}

inline void ServiceOptions::addOptionIntVector(char shortForm, const std::string& optionName, const std::string description, OptionType type)
{
  boost::program_options::options_description* options;
  if (type == CommandLineOption)
    options = &_commandLineOptions;
  else if (type == DaemonOption)
    options = &_daemonOptions;
  else if (type == ConfigOption)
    options = &_configOptions;
  else
    assert(false);

  std::ostringstream strm;
  strm << optionName << "," << shortForm;
  options->add_options()(strm.str().c_str(), boost::program_options::value<std::vector<int> >(), description.c_str());
}

inline void ServiceOptions::addOptionIntVector(const std::string& optionName, const std::string description, OptionType type)
{
  boost::program_options::options_description* options;
  if (type == CommandLineOption)
    options = &_commandLineOptions;
  else if (type == DaemonOption)
    options = &_daemonOptions;
  else if (type == ConfigOption)
    options = &_configOptions;
  else
    assert(false);
  
  options->add_options()(optionName.c_str(), boost::program_options::value<std::vector<int> >(), description.c_str());
}

inline void ServiceOptions::displayUsage(std::ostream& strm) const
{
  displayVersion(strm);
  strm << _optionItems;
  strm.flush();
}

inline void ServiceOptions::displayVersion(std::ostream& strm) const
{
  strm << std::endl << _daemonName << " version " << _version << " - " << _copyright << std::endl << std::endl;
  strm.flush();
}

inline size_t ServiceOptions::hasOption(const std::string& optionName) const
{
  return _options.count(optionName.c_str());
}

inline bool ServiceOptions::getOption(const std::string& optionName, std::string& value, const std::string& defValue) const
{
  if (defValue.empty() && !hasOption(optionName))
  {
    //
    // Check if ptree has it
    //
    if (_hasConfig)
    {
      try
      {
        value = _ptree.get<std::string>(optionName.c_str());
        return true;
      }catch(...)
      {
        return false;
      }
    }
    else
    {
      return false;
    }
  }else if (!hasOption(optionName))
  {
    value = defValue;
    return true;
  }

  value = _options[optionName.c_str()].as<std::string>();
  return true;
}

inline bool ServiceOptions::getOption(const std::string& optionName, std::vector<std::string>& value) const
{
  if (!hasOption(optionName))
    return false;
  value = _options[optionName.c_str()].as<std::vector<std::string> >();
  return true;
}

inline bool ServiceOptions::getOption(const std::string& optionName, int& value) const
{
  if (!hasOption(optionName))
  {
    //
    // Check if ptree has it
    //
    if (_hasConfig)
    {
      try
      {
        value = _ptree.get<int>(optionName.c_str());
        return true;
      }catch(...)
      {
        return false;
      }
    }
  }

  value = _options[optionName.c_str()].as<int>();
  return true;
}

inline bool ServiceOptions::getOption(const std::string& optionName, int& value, int defValue) const
{
  if (!hasOption(optionName))
  {
    //
    // Check if ptree has it
    //
    if (_hasConfig)
    {
      try
      {
        value = _ptree.get<int>(optionName.c_str());
        return true;
      }catch(...)
      {
        value = defValue;
      }
    }
    else
    {
      value = defValue;
    }
  }else
  {
    value = _options[optionName.c_str()].as<int>();
  }
  return true;
}

inline bool ServiceOptions::getOption(const std::string& optionName, std::vector<int>& value) const
{
  if (!hasOption(optionName))
    return false;
  value = _options[optionName.c_str()].as<std::vector<int> >();
  return true;
}



inline ServiceOptions::~ServiceOptions()
{
}

inline void ServiceOptions::daemonize(const std::string& pidfile)
{
   int pid = 0;
   if(getppid() == 1)
     return;
   pid=fork();
   if (pid<0) exit(1); /* fork error */
   if (pid>0) exit(0); /* parent exits */
   /* child (daemon) continues */
   setsid(); /* obtain a new process group */

   for (int descriptor = getdtablesize();descriptor >= 0;--descriptor)
   {
     close(descriptor); /* close all descriptors we have inheritted from parent*/
   }

   int h = open("/dev/null",O_RDWR); dup(h); dup(h); /* handle standard I/O */

   ::close(STDIN_FILENO);

   std::ofstream pidFile(pidfile.c_str());
   pidFile << getpid() << std::endl;
}

inline void ServiceOptions::waitForTerminationRequest()
{
	sigset_t sset;
	sigemptyset(&sset);
	sigaddset(&sset, SIGINT);
	sigaddset(&sset, SIGQUIT);
	sigaddset(&sset, SIGTERM);
	sigprocmask(SIG_BLOCK, &sset, NULL);
	int sig;
	sigwait(&sset, &sig);
  std::cout << "Termination Signal RECEIVED" << std::endl;
}


#endif	/* SERVICEOPTIONS_H */


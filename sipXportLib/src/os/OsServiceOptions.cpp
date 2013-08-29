/*
 * Copyright (c) eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */


#include <cstdlib>
#include <cassert>
#include <csignal>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/stat.h>
#include <execinfo.h>
#include <fcntl.h>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <iostream>
#include "os/OsServiceOptions.h"
#include "os/OsLoggerHelper.h"

OsServiceOptions::OsServiceOptions(int argc, char** argv,
  const std::string& daemonName,
  const std::string& version,
  const std::string& copyright) :
  _argc(argc),
  _argv(argv),
  _daemonName(daemonName),
  _version(version),
  _copyright(copyright),
  _daemonOptions("Daemon"),
  _commandLineOptions("General"),
  _configOptions("Configuration"),
  _optionItems(_daemonName  + " Options"),
  _isDaemon(false),
  _hasConfig(false),
  _isConfigOnly(false),
  _configFileType(ConfigFileTypeBoost),
  _unitTestMode(false)
{
}

OsServiceOptions::OsServiceOptions():
               _argc(0),
               _argv(NULL),
               _daemonOptions("Daemon"),
               _commandLineOptions("General"),
               _configOptions("Configuration"),
               _optionItems(_daemonName  + " Options"),
               _isDaemon(false),
               _hasConfig(false),
               _isConfigOnly(false),
               _configFileType(ConfigFileTypeBoost),
               _unitTestMode(false)
{
}


OsServiceOptions::OsServiceOptions(const std::string& configFile) :
  _argc(0),
  _argv(NULL),
  _daemonName("OsServiceOptions"),
  _daemonOptions("Daemon"),
  _commandLineOptions("General"),
  _configOptions("Configuration"),
  _optionItems(_daemonName  + " Options"),
  _configFile(configFile),
  _isDaemon(false),
  _hasConfig(true),
  _isConfigOnly(true),
  _configFileType(ConfigFileTypeBoost),
  _unitTestMode(false)
{
}

OsServiceOptions::~OsServiceOptions()
{
}

void OsServiceOptions::setCommandLine(int argc, char** argv)
{
  _argc = argc;
  _argv = argv;
}

void OsServiceOptions::setConfigurationFile(const std::string& configFile,
                                            ConfigFileType configFileType)
{
  _configFile = configFile;
  _configFileType = configFileType;
  _hasConfig = true;

  if (_argc == 0 && _argv == NULL)
    _isConfigOnly = true;
  else
    _isConfigOnly = false;
}

bool OsServiceOptions::loadConfigDbFromFile(const char* pFilename)
{
  setConfigurationFile(pFilename, ConfigFileTypeConfigDb);

  return parseOptions();
}

void OsServiceOptions::addOptionFlag(char shortForm, const std::string& optionName, const std::string description, OptionType type)
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

void OsServiceOptions::addOptionFlag(const std::string& optionName, const std::string description, OptionType type)
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

void OsServiceOptions::displayUsage(std::ostream& strm) const
{
  displayVersion(strm);
  strm << _optionItems;
  strm << std::endl;
  
  if (!_required.empty())
  {
    strm << "Mandatory Parameters:" << std::endl;
    for (std::vector<std::string>::const_iterator iter = _required.begin(); iter != _required.end(); iter++)
    {
      strm << "  --" << *iter << std::endl;
    }
  }
  strm.flush();
}

void OsServiceOptions::displayVersion(std::ostream& strm) const
{
  strm << std::endl << _daemonName << " version " << _version << " - " << _copyright << std::endl << std::endl;
  strm.flush();
}

std::size_t OsServiceOptions::hasOption(const std::string& optionName, bool consolidate) const
{
  if (_isConfigOnly && consolidate)
    return _ptree.count(optionName.c_str());

  std::size_t ct = _options.count(optionName.c_str());
  if (!ct && consolidate && _hasConfig)
    ct = _ptree.count(optionName.c_str());
  return ct;
}

size_t OsServiceOptions::hasConfigOption(const std::string& optionName) const
{
  return _ptree.count(optionName.c_str());
}

bool OsServiceOptions::getOption(const std::string& optionName, std::vector<std::string>& value) const
{
  if (!hasOption(optionName, false))
    return false;
  value = _options[optionName.c_str()].as<std::vector<std::string> >();
  return true;
}


bool OsServiceOptions::getOption(const std::string& optionName, std::vector<int>& value) const
{
  if (!hasOption(optionName,false))
    return false;
  value = _options[optionName.c_str()].as<std::vector<int> >();
  return true;
}


bool OsServiceOptions::getOption(const std::string& optionName, bool& value, bool defValue) const
{
  if (!hasOption(optionName, false))
  {
    //
    // Check if ptree has it
    //
    if (_hasConfig)
    {
      try
      {
        std::string str = _ptree.get<std::string>(optionName.c_str());
        value = defValue;
        if (!str.empty())
        {
          char ch = str.at(0);
          value = (ch == '1' || ch == 't' || ch == 'T');
        }
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
    value = _options[optionName.c_str()].as<bool>();
  }
  return true;
}


void OsServiceOptions::addDaemonOptions()
{
  addOptionFlag('D', "daemonize", ": Run as system daemon.", DaemonOption);
  addOptionString('P', "pid-file", ": PID file when running as daemon.", DaemonOption);
}

bool OsServiceOptions::validateRequiredParameters()
{
  for (std::vector<std::string>::const_iterator iter = _required.begin(); iter != _required.end(); iter++)
  {
    if (!hasOption(*iter, true))
    {
      //
      // Check if this option has an alternate
      //
      std::map<std::string, std::string>::const_iterator alternate = _alternative.find(*iter);
      if (alternate != _alternative.end() && hasOption(alternate->second, true))
      {
        continue;
      }
      else
      {
        if (!_unitTestMode)
        {
          displayVersion(std::cerr);

          if (alternate == _alternative.end())
            std::cerr << "ERROR:  Mising required parameter \"" << *iter << "\"!!! Use --help to display usage." << std::endl;
          else
            std::cerr << "ERROR:  Mising required parameter \"" << *iter << "\" or " << "\"" << alternate->second << "\"!!! Use --help to display usage." << std::endl;

          std::cerr << std::endl;
          std::cerr << "Mandatory Parameters:" << std::endl;
          for (std::vector<std::string>::const_iterator iter = _required.begin(); iter != _required.end(); iter++)
          {
            std::cerr << "  --" << *iter << std::endl;
          }
        }
      }
      return false;
    }
  }
  
  return true;
}

void OsServiceOptions::registerRequiredParameters(const std::string& optionName, const std::string& altOptionName)
{
  _required.push_back(optionName);
  if (!altOptionName.empty())
  {
    _alternative[optionName] = altOptionName;
  }
}

void OsServiceOptions::addCommandLineOptions()
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
}

bool OsServiceOptions::checkCommandLineOptions()
{
  if (hasOption("help", false))
  {
    if (!_unitTestMode)
      displayUsage(std::cout);
    return false;
  }

  if (hasOption("version", false))
  {
    displayVersion(std::cout);
    return false;
  }

  return true;
}

bool OsServiceOptions::checkDaemonOptions()
{
  if (hasOption("pid-file", false))
  {
    getOption("pid-file", _pidFile);
    std::ofstream pidFile(_pidFile.c_str());
    pidFile << getpid() << std::endl;
  }

  if (hasOption("daemonize", false))
  {
    if (_pidFile.empty())
    {
      if (!_unitTestMode)
      {
        displayUsage(std::cerr);
        std::cerr << std::endl << "ERROR: You must specify pid-file location!" << std::endl;
        std::cerr.flush();
      }

      return false;
    }
    _isDaemon = true;
  }

  return true;
}

bool OsServiceOptions::checkConfigOptions()
{
  if (hasOption("config-file", false))
  {
    if (getOption("config-file", _configFile) && !_configFile.empty())
    {
      std::ifstream config(_configFile.c_str());
      if (config.good())
      {
        //boost::program_options::store(boost::program_options::parse_config_file(config, _optionItems, true), _options);
        //boost::program_options::notify(_options);
        readConfiguration();
        _hasConfig = true;
      }
      else
      {
        if (!_unitTestMode)
        {
          displayUsage(std::cerr);
          std::cerr << std::endl << "ERROR: Unable to open input file " << _configFile << "!" << std::endl;
          std::cerr.flush();
        }

        return false;
      }
    }
  }

  return true;
}

bool OsServiceOptions::checkOptions(ParseOptionsFlags parseOptionsFlags,
                                    int& exitCode)
{
  bool bRet = true;

  do
  {
    if (parseOptionsFlags & AddComandLineOptionsFlag)
    {
      bRet = checkCommandLineOptions();
      if (bRet == false)
      {
        exitCode = 0;
        break;
      }
    }

    if (parseOptionsFlags & AddDaemonOptionsFlag)
    {
      bRet = checkDaemonOptions();
      if (bRet == false)
      {
        exitCode = -1;
        break;
      }
    }

    if (parseOptionsFlags & AddConfigOptionsFlag)
    {
      bRet = checkConfigOptions();
      if (bRet == false)
      {
        exitCode = -1;
        break;
      }
    }

    if (parseOptionsFlags & ValidateRequiredParametersFlag)
    {
      bRet= validateRequiredParameters();
      if (bRet == false)
      {
        exitCode = -1;
        break;
      }
    }
  }
  while (false);

  return bRet;
}

void OsServiceOptions::dumpOptions()
{
  for(boost::program_options::variables_map::iterator it = _options.begin(); it != _options.end(); ++it)
  {
    std::cout << "first - " << it->first << ", second - " << it->second.as< ::std::string >() << "\n";
  }

//  for(boost::property_tree::ptree::iterator it = _ptree.begin(); it != _ptree.end(); ++it)
//  {
//    std::cout << "first - " << it->first<< ", second - " << it->second.data().c_str() << "\n";
//  }
}

void OsServiceOptions::readConfiguration()
{
  if (_configFileType == ConfigFileTypeBoost)
  {
    boost::property_tree::ini_parser::read_ini(_configFile.c_str(), _ptree);
  }
  else if (_configFileType == ConfigFileTypeConfigDb)
  {
    _configDb.loadFromFile(_configFile.c_str());

    UtlString currentKey = "";
    UtlString nextKey = "";
    UtlString nextValue = "";

    while (_configDb.getNext(currentKey, nextKey, nextValue) != OS_NO_MORE_DATA)
    {
      _options.insert(std::make_pair(nextKey.str(), boost::program_options::variable_value(nextValue.str(), false)));
      boost::program_options::notify(_options);

      //_ptree.push_back(std::make_pair(nextKey.str(), nextValue.str()));

      currentKey = nextKey;
    }

    dumpOptions();
  }
}

bool OsServiceOptions::parseOptions(ParseOptionsFlags parseOptionsFlags)
{
  if (_isConfigOnly)
  {
    try
    {
      readConfiguration();
      _hasConfig = true;
    }
    catch(const std::exception& e)
    {
      if (!_unitTestMode)
        OS_LOG_ERROR(FAC_KERNEL, _daemonName << " is not able to parse the options - " << e.what());

      return false;
    }
    return true;
  }

  try
  {
    if (parseOptionsFlags & AddComandLineOptionsFlag)
    {
      addCommandLineOptions();
      _optionItems.add(_commandLineOptions);
    }

    if (parseOptionsFlags & AddDaemonOptionsFlag)
      _optionItems.add(_daemonOptions);

    if (parseOptionsFlags & AddConfigOptionsFlag)
      _optionItems.add(_configOptions);

    boost::program_options::store(boost::program_options::parse_command_line(_argc, _argv, _optionItems), _options);
    boost::program_options::notify(_options);

    int exitCode = 0;

    bool bRet = checkOptions(parseOptionsFlags, exitCode);
    if (bRet == false)
    {
      if (!_unitTestMode)
        exit(exitCode);
      else
      {
        if (exitCode == 0)
          return true;
        else
          return false;
      }
    }
  }
  catch(const std::exception& e)
  {
    if (parseOptionsFlags & DisplayExceptionFlag)
      std::cerr << "Exception: " << e.what() << std::endl;

    return false;
  }

  if (parseOptionsFlags & InitLoggerFlag)
    initlogger();

  std::set_terminate(&catch_global);

  if (parseOptionsFlags & DisplayVersionOnInitFlag)
  {
    if (!_unitTestMode)
      displayVersion(std::cout);
  }

  return true;
}

void OsServiceOptions::initlogger()
{
  std::string logFile;
  int priorityLevel = PRI_INFO;
  if (hasOption("log-file", true))
  {
    if (getOption("log-file", logFile) && !logFile.empty())
    {
      if (hasOption("log-level", true))
        getOption("log-level", priorityLevel, priorityLevel);

      int logLevel = SYSLOG_NUM_PRIORITIES - priorityLevel - 1;
      if (logLevel < 0)
        logLevel = 0;

      Os::LoggerHelper::instance().processName = _daemonName;

      if (!Os::LoggerHelper::instance().initialize(logLevel, logFile.c_str()))
      {
        if (!_unitTestMode)
        {
          displayUsage(std::cerr);
          std::cerr << std::endl << "ERROR: Unable to create log file " << logFile << "!" << std::endl;
          std::cerr.flush();
        }

        if (!_unitTestMode)
          _exit(-1);
        else
          return;
      }
    }
  }
  else
  {
    if (!_unitTestMode)
    {
      displayUsage(std::cerr);
      std::cerr << std::endl << "ERROR: Log file not specified!" << std::endl;
      std::cerr.flush();
    }

    if (!_unitTestMode)
      _exit(-1);
    else
      return;
  }
}


void  OsServiceOptions::daemonize(int argc, char** argv)
{
  bool isDaemon = false;
  for (int i = 0; i < argc; i++)
  {
    std::string arg = argv[i];
    if (arg == "-D" || arg == "--daemonize")
    {
      isDaemon = true;
      break;
    }
  }

  if (isDaemon)
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
  }
}

// copy error information to log. registered only after logger has been configured.
void OsServiceOptions::catch_global()
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


void OsServiceOptions::waitForTerminationRequest()
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




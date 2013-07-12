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
  _commandLineOptions("Generic"),
  _configOptions("Configuration"),
  _optionItems(_daemonName  + " OsServiceOptions"),
  _isDaemon(false),
  _hasConfig(false),
  _isConfigOnly(false)
{
}

OsServiceOptions::OsServiceOptions(const std::string& configFile) :
  _argc(0),
  _argv(0),
  _daemonOptions("Daemon"),
  _commandLineOptions("Generic"),
  _configOptions("Configuration"),
  _optionItems(_daemonName  + " OsServiceOptions"),
  _isDaemon(false),
  _configFile(configFile),
  _hasConfig(true),
  _isConfigOnly(true)
{
}

OsServiceOptions::~OsServiceOptions()
{
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

void OsServiceOptions::addOptionString(char shortForm, const std::string& optionName, const std::string description, OptionType type)
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

void OsServiceOptions::addOptionString(const std::string& optionName, const std::string description, OptionType type)
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

void OsServiceOptions::addOptionStringVector(char shortForm, const std::string& optionName, const std::string description, OptionType type)
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

void OsServiceOptions::addOptionStringVector(const std::string& optionName, const std::string description, OptionType type)
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

void OsServiceOptions::addOptionInt(char shortForm, const std::string& optionName, const std::string description, OptionType type)
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

void OsServiceOptions::addOptionInt(const std::string& optionName, const std::string description, OptionType type)
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

void OsServiceOptions::addOptionIntVector(char shortForm, const std::string& optionName, const std::string description, OptionType type)
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

void OsServiceOptions::addOptionIntVector(const std::string& optionName, const std::string description, OptionType type)
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

void OsServiceOptions::displayUsage(std::ostream& strm) const
{
  displayVersion(strm);
  strm << _optionItems;
  strm.flush();
}

void OsServiceOptions::displayVersion(std::ostream& strm) const
{
  strm << std::endl << _daemonName << " version " << _version << " - " << _copyright << std::endl << std::endl;
  strm.flush();
}

std::size_t OsServiceOptions:: hasOption(const std::string& optionName, bool consolidate) const
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

bool OsServiceOptions::getOption(const std::string& optionName, std::string& value, const std::string& defValue) const
{
  if (defValue.empty() && !hasOption(optionName, false))
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
  }else if (!hasOption(optionName, false))
  {
    value = defValue;
    return true;
  }

  value = _options[optionName.c_str()].as<std::string>();
  return true;
}

bool OsServiceOptions::getOption(const std::string& optionName, std::vector<std::string>& value) const
{
  if (!hasOption(optionName, false))
    return false;
  value = _options[optionName.c_str()].as<std::vector<std::string> >();
  return true;
}

bool OsServiceOptions::getOption(const std::string& optionName, int& value) const
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

bool OsServiceOptions::getOption(const std::string& optionName, int& value, int defValue) const
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


bool OsServiceOptions::parseOptions()
{

  if (_isConfigOnly)
  {
    try
    {
      boost::property_tree::ini_parser::read_ini(_configFile.c_str(), _ptree);
      _hasConfig = true;
    }
    catch(const std::exception& e)
    {
        std::cerr << _daemonName << " is not able to parse the options - " << e.what() << std::endl;
        return false;
    }
    return true;
  }

  displayVersion(std::cout);

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

    if (hasOption("help", false))
    {
      displayUsage(std::cout);
      exit(0);
    }

    if (hasOption("version", false))
    {
      displayVersion(std::cout);
      exit(0);
    }

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
        displayUsage(std::cerr);
        std::cerr << std::endl << "ERROR: You must specify pid-file location!" << std::endl;
        std::cerr.flush();
        exit(-1);
      }
      _isDaemon = true;
    }

    if (hasOption("config-file", false))
    {
      if (getOption("config-file", _configFile) && !_configFile.empty())
      {
        std::ifstream config(_configFile.c_str());
        if (config.good())
        {
          //boost::program_options::store(boost::program_options::parse_config_file(config, _optionItems, true), _options);
          //boost::program_options::notify(_options);
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
    return false;
  }

  initlogger();
  std::set_terminate(&catch_global);



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




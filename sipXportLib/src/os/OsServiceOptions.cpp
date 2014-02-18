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



OsServiceOptions::OsServiceOptions(int argc, char** pArgv) :
  _argc(argc),
  _pArgv(pArgv),
  _commandLineOptionsDescription("General"),
  _configFileOptionsDescription("Configuration"),
  _allOptionsDescription("Options"),
  _hasConfig(false),
  _isParsed(false)
{
}

OsServiceOptions::OsServiceOptions():
               _argc(0),
               _pArgv(NULL),
               _commandLineOptionsDescription("General"),
               _configFileOptionsDescription("Configuration"),
               _allOptionsDescription("Options"),
               _hasConfig(false),
               _isParsed(false)
{
}


OsServiceOptions::OsServiceOptions(const std::string& configFile) :
  _argc(0),
  _pArgv(NULL),
  _commandLineOptionsDescription("General"),
  _configFileOptionsDescription("Configuration"),
  _allOptionsDescription("Options"),
  _configFile(configFile),
  _hasConfig(false),
  _isParsed(false)
{
  if (!_configFile.empty())
    _hasConfig = true;
}

OsServiceOptions::~OsServiceOptions()
{
}

void OsServiceOptions::setCommandLine(int argc, char** pArgv)
{
  _argc = argc;
  _pArgv = pArgv;
}

void OsServiceOptions::setConfigurationFile(const std::string& configFile)
{
  _configFile = configFile;
  if (!_configFile.empty())
    _hasConfig = true;
}

const OsServiceOptions::ConfigOptionDesc& OsServiceOptions::configOption()
{
  static ConfigOptionDesc configOption = {'C', "config-file", ": Optional configuration file.", OsServiceOptions::CommandLineOption};
  return configOption;
}
const OsServiceOptions::ConfigOptionDesc& OsServiceOptions::helpOption()
{
  static ConfigOptionDesc helpOption = {'h', "help", ": Display help information.", OsServiceOptions::CommandLineOption};
  return helpOption;
}

const OsServiceOptions::ConfigOptionDesc& OsServiceOptions::versionOption()
{
  static ConfigOptionDesc versionOption = {'v', "version", ": Display version information.", OsServiceOptions::CommandLineOption};
  return versionOption;
}
const OsServiceOptions::ConfigOptionDesc& OsServiceOptions::pidFileOption()
{
  static ConfigOptionDesc pidFileOption = {'P', "pid-file", ": PID file when running as daemon.", OsServiceOptions::CommandLineOption};
  return pidFileOption;
}
const OsServiceOptions::ConfigOptionDesc& OsServiceOptions::logFileOption()
{
  static ConfigOptionDesc logFileOption = {'L', "log-file", ": Specify the application log file.", OsServiceOptions::CommandLineOption};
  return logFileOption;
}
const OsServiceOptions::ConfigOptionDesc& OsServiceOptions::logLevelOption()
{
  static ConfigOptionDesc logLevelOption = {'l', "log-level",
      ": Specify the application log priority level."
      "Valid level is between 0-7.  "
      "0 (EMERG) 1 (ALERT) 2 (CRIT) 3 (ERR) 4 (WARNING) 5 (NOTICE) 6 (INFO) 7 (DEBUG)"
            , OsServiceOptions::CommandLineOption};
  return logLevelOption;
}


void OsServiceOptions::addDefaultOptions()
{
  addOptionFlag(helpOption().shortForm, helpOption().pName, helpOption().pDescription, helpOption().type);
  addOptionFlag(versionOption().shortForm, versionOption().pName, versionOption().pDescription, versionOption().type);
  addOptionString(pidFileOption().shortForm, pidFileOption().pName, pidFileOption().pDescription, pidFileOption().type);
  addOptionString(logFileOption().shortForm, logFileOption().pName, logFileOption().pDescription, logFileOption().type);
  addOptionInt(logLevelOption().shortForm, logLevelOption().pName, logLevelOption().pDescription, logLevelOption().type);
}

void OsServiceOptions::addOptionFlag(char shortForm, const std::string& optionName, const std::string description, OptionType type)
{
  boost::program_options::options_description* optionsDescription;
  if (type == CommandLineOption)
    optionsDescription = &_commandLineOptionsDescription;
  else if (type == ConfigOption)
    optionsDescription = &_configFileOptionsDescription;
  else
    assert(false);

  std::ostringstream strm;
  strm << optionName;

  if (shortForm != 0)
    strm << "," << shortForm;

  optionsDescription->add_options()(strm.str().c_str(), description.c_str());
}

void OsServiceOptions::addOptionFlag(const std::string& optionName, const std::string description, OptionType type)
{
  addOptionFlag(0, optionName, description, type);
}

std::size_t OsServiceOptions::hasOption(const std::string& optionName, OptionType* pOptionType) const
{
  // check if option is present in command line options
  std::size_t ct = _commandLineOptions.count(optionName.c_str());

  if (ct && pOptionType)
    *pOptionType = CommandLineOption;

  // if not found in command line also check configuration file
  if (!ct && _hasConfig)
  {
    ct = _configFileOptions.count(optionName.c_str());

    if (ct && pOptionType)
      *pOptionType = ConfigOption;
  }

  return ct;
}

bool OsServiceOptions::validateRequiredParameters(std::ostream& strm)
{
  for (std::vector<std::string>::const_iterator iter = _requiredOptionsNames.begin(); iter != _requiredOptionsNames.end(); iter++)
  {
    if (!hasOption(*iter))
    {
      //
      // Check if this option has an alternate
      //
      std::map<std::string, std::string>::const_iterator alternate = _alternative.find(*iter);
      if (alternate != _alternative.end() && hasOption(alternate->second))
      {
        continue;
      }
      else
      {
        if (alternate == _alternative.end())
          strm << "ERROR:  Mising required parameter \"" << *iter << "\"!!! Use --help to display usage." << std::endl;
        else
          strm << "ERROR:  Mising required parameter \"" << *iter << "\" or " << "\"" << alternate->second << "\"!!! Use --help to display usage." << std::endl;

        strm << std::endl;
        strm << "Mandatory Parameters:" << std::endl;
        for (std::vector<std::string>::const_iterator iter = _requiredOptionsNames.begin(); iter != _requiredOptionsNames.end(); iter++)
        {
          strm << "  --" << *iter << std::endl;
        }
      }
      return false;
    }
  }
  
  return true;
}

void OsServiceOptions::registerRequiredParameters(const std::string& optionName, const std::string& altOptionName)
{
  _requiredOptionsNames.push_back(optionName);
  if (!altOptionName.empty())
  {
    _alternative[optionName] = altOptionName;
  }
}

void OsServiceOptions::addDefaultCommandLineOptions()
{
  addOptionString(configOption().shortForm, configOption().pName, configOption().pDescription, configOption().type);
}


void OsServiceOptions::checkDefaultCommandLineOptions(ParseOptionsFlags parseOptionsFlags, std::ostream& strm)
{
  if (hasOption(configOption().pName))
  {
    if (getOption(configOption().pName, _configFile) && !_configFile.empty())
    {
      _hasConfig = true;
    }
  }
}

bool OsServiceOptions::checkParseConfigurationFile(ParseOptionsFlags parseOptionsFlags, std::ostream& strm)
{
  bool bRet = false;

  if (_hasConfig)
  {
    std::ifstream config(_configFile.c_str());
    if (config.good())
    {
      bRet = parseConfigurationFile(parseOptionsFlags);
      if (bRet == false)
        return bRet;
    }
    else
    {
      strm << std::endl << "ERROR: Unable to open input file " << _configFile << "!" << std::endl;
      strm.flush();

      return false;
    }
  }

  return true;
}

bool OsServiceOptions::parseConfigurationFile(ParseOptionsFlags parseOptionsFlags)
{
  if (parseOptionsFlags & ParseConfigDbFlag)
  {
    struct stat fileStat;
    int rc = 0;

    // if _configFile path is a directory return false
    rc = stat((char*)_configFile.c_str(), &fileStat);
    if(rc == 0 && fileStat.st_mode & S_IFDIR)
      return false;

    OsStatus retval = _configDb.loadFromFile(_configFile.c_str());
    if (retval != OS_SUCCESS)
      return false;

    UtlString currentKey = "";
    UtlString nextKey = "";
    UtlString nextValue = "";

    while (_configDb.getNext(currentKey, nextKey, nextValue) != OS_NO_MORE_DATA)
    {
      _unregisteredOptions.insert(std::make_pair(nextKey.str(), nextValue.str()));

      currentKey = nextKey;
    }
  }
  else
  {
    //boost::property_tree::ini_parser::read_ini(_configFile.c_str(), _configFileOptions);
    std::ifstream file(_configFile.c_str());
    if (!file)
    {
      OS_LOG_ERROR(FAC_KERNEL, "Can not open configuration file " << _configFile);
      return false;
    }

    boost::program_options::parsed_options parsedFile = boost::program_options::parse_config_file(file, _allOptionsDescription, true);
    boost::program_options::store(parsedFile, _configFileOptions);
    boost::program_options::notify(_configFileOptions);

    for (unsigned int i = 0; i < parsedFile.options.size(); ++i)
    {
      if (parsedFile.options[i].unregistered)
      {
        _unregisteredOptions.insert(std::make_pair(parsedFile.options[i].string_key, parsedFile.options[i].value[0]));
      }
    }
  }

  return true;
}

bool OsServiceOptions::parseOptions(ParseOptionsFlags parseOptionsFlags,
                                    std::ostream& strm)
{
  bool bRet = false;

  if (_isParsed == true)
    return bRet;

  try
  {
    if (parseOptionsFlags & AddDefaultComandLineOptionsFlag)
    {
      addDefaultCommandLineOptions();
    }

    if (_commandLineOptionsDescription.options().size() > 0)
      _allOptionsDescription.add(_commandLineOptionsDescription);

    if (_configFileOptionsDescription.options().size() > 0)
      _allOptionsDescription.add(_configFileOptionsDescription);

    if (_argc != 0 || _pArgv != NULL)
    {
      boost::program_options::store(boost::program_options::parse_command_line(_argc, _pArgv, _allOptionsDescription), _commandLineOptions);
      boost::program_options::notify(_commandLineOptions);
    }

    if (parseOptionsFlags & StopIfVersionHelpFlag)
    {
      if (hasOption(helpOption().pName) || hasOption(versionOption().pName))
      {
        return true;
      }
    }

    if (parseOptionsFlags & AddDefaultComandLineOptionsFlag)
    {
      checkDefaultCommandLineOptions(parseOptionsFlags, strm);
    }

    bRet = checkParseConfigurationFile(parseOptionsFlags, strm);
    if (bRet == false)
      return bRet;

    bRet = validateRequiredParameters(strm);
    if (bRet == false)
      return bRet;

  }
  catch(const std::exception& e)
  {
    strm << std::endl << "Can not parse the options - " << e.what() << std::endl;
    strm.flush();

    if (_argc == 0 && _pArgv == NULL)
      OS_LOG_ERROR(FAC_KERNEL, "Can not parse " << _configFile << " - " << e.what());

    _hasConfig = false;

    return false;
  }

  _isParsed = true;
  return true;
}

void OsServiceOptions::displayUsage(std::ostream& strm)
{
  strm << _allOptionsDescription;
  strm << std::endl;

  if (!_requiredOptionsNames.empty())
  {
    strm << "Mandatory Parameters:" << std::endl;
    for (std::vector<std::string>::const_iterator iter = _requiredOptionsNames.begin(); iter != _requiredOptionsNames.end(); iter++)
    {
      strm << "  --" << *iter << std::endl;
    }
  }

  if (!_extraHelp.empty())
  {
    strm << _extraHelp;
  }

  strm.flush();
}


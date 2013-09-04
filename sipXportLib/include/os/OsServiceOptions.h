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


#ifndef OSSERVICEOPTIONS_H_INCLUDED
#define	OSSERVICEOPTIONS_H_INCLUDED


#include <string>
#include <vector>
#include <map>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/detail/ptree_utils.hpp>
#include "os/OsLogger.h"

class OsServiceOptions
{
public:
  enum OptionType
  {
    CommandLineOption,
    DaemonOption,
    ConfigOption
  };

  enum ParseOptionsFlags
  {
    NoOptionsFlag = 0,
    AddComandLineOptionsFlag = 1 << 0,
    AddDaemonOptionsFlag = 1 << 1,
    AddConfigOptionsFlag = 1 << 2,
    InitLoggerFlag = 1 << 3,
    DisplayVersionOnInitFlag = 1 << 4,
    DisplayExceptionFlag = 1 << 5,
    ValidateRequiredParametersFlag = 1 << 6,
    DefaultOptionsFlag = AddComandLineOptionsFlag |
                        AddDaemonOptionsFlag |
                        AddConfigOptionsFlag |
                        InitLoggerFlag |
                        DisplayVersionOnInitFlag |
                        ValidateRequiredParametersFlag
  };

  OsServiceOptions(int argc, char** argv,
    const std::string& daemonName,
    const std::string& version,
    const std::string& copyright);

  OsServiceOptions(const std::string& configFile);

  ~OsServiceOptions();

  void addOptionFlag(
    char shortForm,
    const std::string& optionName,
    const std::string description,
    OptionType type);

  void addOptionFlag(
    const std::string& optionName,
    const std::string description,
    OptionType type);

  void addOptionString(
    char shortForm,
    const std::string& optionName,
    const std::string description,
    OptionType type,
    bool required = false,
    const std::string& altOptionName = std::string() /* alternative option if this option is required and is missing*/);

  void addOptionString(
    const std::string& optionName,
    const std::string description,
    OptionType type,
    bool required = false,
    const std::string& altOptionName = std::string() /* alternative option if this option is required and is missing*/);

  void addOptionStringVector(
    char shortForm,
    const std::string& optionName,
    const std::string description,
    OptionType type,
    bool required = false,
    const std::string& altOptionName = std::string() /* alternative option if this option is required and is missing*/);

  void addOptionStringVector(
    const std::string& optionName,
    const std::string description,
    OptionType type,
    bool required = false,
    const std::string& altOptionName = std::string() /* alternative option if this option is required and is missing*/);

  void addOptionInt(
    char shortForm,
    const std::string& optionName,
    const std::string description, 
    OptionType type,
    bool required = false,
    const std::string& altOptionName = std::string() /* alternative option if this option is required and is missing*/);

  void addOptionInt(
    const std::string& optionName,
    const std::string description,
    OptionType type,
    bool required = false,
    const std::string& altOptionName = std::string() /* alternative option if this option is required and is missing*/);

  void addOptionIntVector(
    char shortForm,
    const std::string& optionName,
    const std::string description,
    OptionType type,
    bool required = false,
    const std::string& altOptionName = std::string() /* alternative option if this option is required and is missing*/);

  void addOptionIntVector(
    const std::string& optionName,
    const std::string description,
    OptionType type,
    bool required = false,
    const std::string& altOptionName = std::string() /* alternative option if this option is required and is missing*/);

  void displayUsage(std::ostream& strm) const;

  void displayVersion(std::ostream& strm) const;

  std::size_t hasOption(const std::string& optionName, bool consolidate) const;

  size_t hasConfigOption(const std::string& optionName) const;

  bool getOption(
    const std::string& optionName,
    std::string& value,
    const std::string& defValue = std::string()) const;

  bool getOption(
    const std::string& optionName,
    std::vector<std::string>& value) const;

  bool getOption(const std::string& optionName, int& value) const;

  bool getOption(const std::string& optionName, int& value, int defValue) const;

  bool getOption(const std::string& optionName, std::vector<int>& value) const;

  bool getOption(
    const std::string& optionName,
    bool& value,
    bool defValue) const;

  void addDaemonOptions();

  void addCommandLineOptions();

  bool checkCommandLineOptions();

  bool checkDaemonOptions();

  bool checkConfigOptions();

  bool checkOptions(ParseOptionsFlags parseOptionsFlags, int& exitCode);

  bool parseOptions(ParseOptionsFlags parseOptionsFlags = DefaultOptionsFlag);

  void initlogger();

  static void  daemonize(int argc, char** argv);
  // copy error information to log. registered only after logger has been configured.
  static void catch_global();

  static void waitForTerminationRequest();

protected:
  void registerRequiredParameters(const std::string& optionName, const std::string& altOptionName);
  bool validateRequiredParameters();

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
  bool _isConfigOnly;
  //
  // A vector the contains required parameters
  //
  std::vector<std::string> _required;
  //
  // A map that contains alternative parameter for a required parameter.
  //
  std::map<std::string, std::string> _alternative;

  friend class OsServiceOptionsTest;
  bool _unitTestMode;
};




#endif	/// OSSERVICEOPTIONS_H_INCLUDED


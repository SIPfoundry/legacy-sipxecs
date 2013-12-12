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
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include "os/OsLogger.h"

#include "os/OsConfigDb.h"

#include "utl/UtlString.h"

class OsServiceOptions
{
public:
  enum OptionType
  {
    CommandLineOption,  // used for command line options
    ConfigOption,        // used for configuration file options
  };

  enum ParseOptionsFlags
  {
    NoOptionsFlag = 0,
    AddDefaultComandLineOptionsFlag = 1 << 0,
    ParseConfigDbFlag = 1 << 1,
    StopIfVersionHelpFlag = 1 << 2,    // stops parsing and validating parameters if help or version are present in command line
    DefaultOptionsFlag = AddDefaultComandLineOptionsFlag
  };

  typedef struct
  {
    const char shortForm;
    const char* pName;
    const char* pDescription;
    const OsServiceOptions::OptionType type;
  } ConfigOptionDesc;

  static const ConfigOptionDesc& configOption();
  static const ConfigOptionDesc& helpOption();
  static const ConfigOptionDesc& versionOption();
  static const ConfigOptionDesc& pidFileOption();
  static const ConfigOptionDesc& logFileOption();
  static const ConfigOptionDesc& logLevelOption();

  // Default constructor
  OsServiceOptions();

  /**
    * Constructor
    *
    * @param  argc - Number of arguments.
    * @param  pArgv - Array of command line arguments.
    */
  OsServiceOptions(int argc, char** pArgv);

  /**
    * Constructor
    *
    * @param  configFile - Configuration file path
    */
  OsServiceOptions(const std::string& configFile);

  //Destructor
  ~OsServiceOptions();

  /**
    * Sets command line
    *
    * @param  argc - Number of arguments.
    * @param  pArgv - Array of command line arguments.
    */
  void setCommandLine(int argc, char** pArgv);

  /**
    * Sets configuration file
    *
    * @param  configFile - Configuration file path.
    */
  void setConfigurationFile(const std::string& configFile);

  /**
   * Displays help for the application
   * @param strm - the stream used for displaying help
   */
  void displayUsage(std::ostream& strm);


  /**
   * Sets the extra help.
   *
   * @param - The string that will be displayed after help
   */
  void setExtraHelp(std::string& extraHelp);

  /**
   *  Adds configuration options
   */
  void addDefaultOptions();

  /**
    * Adds a flag option. Is used also when displaying the help for the application
    *
    * @param shortForm - Short form for the option name.
    * @param optionName - The option name
    * @param description - Option name description
    * @param type - Option type: can be CommandLineOption or ConfigOption
    */
  void addOptionFlag(char shortForm,
    const std::string& optionName,
    const std::string description,
    OptionType type);

  /**
    * Adds a flag option. Is used also when displaying the help for the application
    *
    * @param optionName - The option name
    * @param description - Option name description
    * @param type - Option type: can be CommandLineOption or ConfigOption
    */
  void addOptionFlag(const std::string& optionName,
    const std::string description,
    OptionType type);

  /**
    * Adds a string option. Is used also when displaying the help for the application
    *
    * @param shortForm - Short form for the option name.
    * @param optionName - The option name
    * @param description - Option name description
    * @param type - Option type: can be CommandLineOption or ConfigOption
    * @param required - If set true this option is mandatory
    * @param altOptionName - Alternative option if this option is required and is missing
    */
  void addOptionString(char shortForm,
    const std::string& optionName,
    const std::string description,
    OptionType type,
    bool required = false,
    const std::string& altOptionName = std::string());

  /**
    * Adds a string option. Is used also when displaying the help for the application
    *
    * @param optionName - The option name
    * @param description - Option name description
    * @param type - Option type: can be CommandLineOption or ConfigOption
    * @param required - If set true this option is mandatory
    * @param altOptionName - Alternative option if this option is required and is missing
    */
  void addOptionString(const std::string& optionName,
    const std::string description,
    OptionType type,
    bool required = false,
    const std::string& altOptionName = std::string());

  /**
    * Adds a string vector option. Is used also when displaying the help for the application
    *
    * @param shortForm - Short form for the option name.
    * @param optionName - The option name
    * @param description - Option name description
    * @param type - Option type: can be CommandLineOption or ConfigOption
    * @param required - If set true this option is mandatory
    * @param altOptionName - Alternative option if this option is required and is missing
    */
  void addOptionStringVector(char shortForm,
    const std::string& optionName,
    const std::string description,
    OptionType type,
    bool required = false,
    const std::string& altOptionName = std::string() /* alternative option if this option is required and is missing*/);

  /**
    * Adds a string vector option. Is used also when displaying the help for the application
    *
    * @param optionName - The option name
    * @param description - Option name description
    * @param type - Option type: can be CommandLineOption or ConfigOption
    * @param required - If set true this option is mandatory
    * @param altOptionName - Alternative option if this option is required and is missing
    */
  void addOptionStringVector(const std::string& optionName,
    const std::string description,
    OptionType type,
    bool required = false,
    const std::string& altOptionName = std::string() /* alternative option if this option is required and is missing*/);

  /**
    * Adds an int option. Is used also when displaying the help for the application
    *
    * @param shortForm - Short form for the option name.
    * @param optionName - The option name
    * @param description - Option name description
    * @param type - Option type: can be CommandLineOption or ConfigOption
    * @param required - If set true this option is mandatory
    * @param altOptionName - Alternative option if this option is required and is missing
    */
  void addOptionInt(char shortForm,
    const std::string& optionName,
    const std::string description, 
    OptionType type,
    bool required = false,
    const std::string& altOptionName = std::string());

  /**
    * Adds an int option. Is used also when displaying the help for the application
    *
    * @param optionName - The option name
    * @param description - Option name description
    * @param type - Option type: can be CommandLineOption or ConfigOption
    * @param required - If set true this option is mandatory
    * @param altOptionName - Alternative option if this option is required and is missing
    */
  void addOptionInt(const std::string& optionName,
    const std::string description,
    OptionType type,
    bool required = false,
    const std::string& altOptionName = std::string());

  /**
    * Adds an int vector option. Is used also when displaying the help for the application
    *
    * @param shortForm - Short form for the option name.
    * @param optionName - The option name
    * @param description - Option name description
    * @param type - Option type: can be CommandLineOption or ConfigOption
    * @param required - If set true this option is mandatory
    * @param altOptionName - Alternative option if this option is required and is missing
    */
  void addOptionIntVector(char shortForm,
    const std::string& optionName,
    const std::string description,
    OptionType type,
    bool required = false,
    const std::string& altOptionName = std::string());

  /**
    * Adds an int vector option. Is used also when displaying the help for the application
    *
    * @param optionName - The option name
    * @param description - Option name description
    * @param type - Option type: can be CommandLineOption or ConfigOption
    * @param required - If set true this option is mandatory
    * @param altOptionName - Alternative option if this option is required and is missing
    */
  void addOptionIntVector(const std::string& optionName,
    const std::string description,
    OptionType type,
    bool required = false,
    const std::string& altOptionName = std::string());

  /**
    * Function template used for adding any option type. Is used also when displaying the help for the application
    *
    * @param shortForm - Short form for the option name.
    * @param optionName - The option name
    * @param description - Option name description
    * @param type - Option type: can be CommandLineOption or ConfigOption
    * @param required - If set true this option is mandatory
    * @param altOptionName - Alternative option if this option is required and is missing
    */
  template<typename T>
  void addOption(char shortForm, const std::string& optionName, const std::string description, OptionType type,
    bool required = false,
    const std::string& altOptionName = std::string())
  {
    boost::program_options::options_description* pOptionsDescription;
    if (type == CommandLineOption)
      pOptionsDescription = &_commandLineOptionsDescription;
    else if (type == ConfigOption)
      pOptionsDescription = &_configFileOptionsDescription;
    else
      assert(false);

    std::ostringstream strm;
    strm << optionName;

    if (shortForm != 0)
      strm << "," << shortForm;

    pOptionsDescription->add_options()(strm.str().c_str(), boost::program_options::value<T>(), description.c_str());

    if (required)
      registerRequiredParameters(optionName, altOptionName);
  }

  /**
    * Function template used for adding any option type. Is used also when displaying the help for the application
    *
    * @param optionName - The option name
    * @param description - Option name description
    * @param type - Option type: can be CommandLineOption or ConfigOption
    * @param required - If set true this option is mandatory
    * @param altOptionName - Alternative option if this option is required and is missing
    */
  template<typename T>
  void addOption(const std::string& optionName, const std::string description, OptionType type,
    bool required = false,
    const std::string& altOptionName = std::string())
  {
    addOption<T>(0, optionName, description, type, required, altOptionName);
  }

  /**
    * Checks first if the option if present in command line. If not present that option is searched
    * also in configuration file
    *
    * @param optionName - The option name
    * @param pOptionType - On return is set to either CommandLineOption if this was found in command line
    *  or ConfigOption if this option was found in configuration file
    * @return - O if the option as not found or the count of values found for that option
    */
  std::size_t hasOption(const std::string& optionName, OptionType* pOptionType = NULL) const;

  // returns a reference to OsConfigDb class
  OsConfigDb& getOsConfigDb();

  /**
   * Returns the value for a given option. It searches at first in command line and if
   * not found it also searches in configuration file
   *
   * @param optionName - The option name
   * @param value - Returned value
   * @param defValue - Default value. If the value for the option is not found it will be set to default value
   */
  bool getOption(
    const std::string& optionName,
    std::string& value,
    const std::string defValue) const;

  /**
   * Returns the value for a given option. It searches at first in command line and if
   * not found it also searches in configuration file
   *
   * @param optionName - The option name
   * @param value - Returned value
   */
  bool getOption(const std::string& optionName, std::string& value) const;

  /**
   * Returns the value for a given option. It searches at first in command line and if
   * not found it also searches in configuration file
   *
   * @param optionName - The option name
   * @param value - Returned value
   * @param defValue - Default value. If the value for the option is not found it will be set to default value
   */
  bool getOption(
    const std::string& optionName,
    UtlString& value,
    const UtlString defValue) const;

  /**
   * Returns the value for a given option. It searches at first in command line and if
   * not found it also searches in configuration file
   *
   * @param optionName - The option name
   * @param value - Returned value
   */
  bool getOption(const std::string& optionName, UtlString& value) const;

  /**
   * Returns the value for a given option. It searches at first in command line and if
   * not found it also searches in configuration file
   *
   * @param optionName - The option name
   * @param value - Returned value
   * @param pDefValue - Default value. If the value for the option is not found it will be set to default value
   * @return - true if the option was found either in command line or in configuration file or false otherwise
   */
  bool getOption(const std::string& optionName, std::vector<std::string>& value) const;

  /**
   * Returns the value for a given option. It searches at first in command line and if
   * not found it also searches in configuration file
   *
   * @param optionName - The option name
   * @param value - Returned value
   * @param defValue - Default value. If the value for the option is not found it will be set to default value
   * @return - true if the option was found either in command line or in configuration file or false otherwise
   */
  bool getOption(const std::string& optionName, int& value, const int defValue) const;

  /**
   * Returns the value for a given option. It searches at first in command line and if
   * not found it also searches in configuration file
   *
   * @param optionName - The option name
   * @param value - Returned value
   * @return - true if the option was found either in command line or in configuration file or false otherwise
   */
  bool getOption(const std::string& optionName, int& value) const;

  /**
   * Returns the value for a given option. It searches at first in command line and if
   * not found it also searches in configuration file
   *
   * @param optionName - The option name
   * @param value - Returned value
   * @return - true if the option was found either in command line or in configuration file or false otherwise
   */
  bool getOption(const std::string& optionName, std::vector<int>& value) const;

  /**
   * Returns the value for a given option. It searches at first in command line and if
   * not found it also searches in configuration file
   *
   * @param optionName - The option name
   * @param value - Returned value
   * @param defValue - Default value. If the value for the option is not found it will be set to default value
   * @return - true if the option was found either in command line or in configuration file or false otherwise
   */
  bool getOption(const std::string& optionName, bool& value, const bool defValue) const;

  /**
   * Returns the value for a given option. It searches at first in command line and if
   * not found it also searches in configuration file
   *
   * @param optionName - The option name
   * @param value - Returned value
   * @return - true if the option was found either in command line or in configuration file or false otherwise
   */
  bool getOption(const std::string& optionName, bool& value) const;

  /**
   * Function template for returning the value for a given option. It searches at first in command line and if
   * not found it also searches in configuration file
   *
   * @param optionName - The option name
   * @param value - Returned value
   * @param defValue - Default value. If the value for the option is not found it will be set to default value
   * @return - true if the option was found either in command line or in configuration file or a default value is given and false otherwise
   */
  template<typename T>
  bool getOption(const std::string& optionName, T& value, const T defValue) const
  {
    return getOption<T>(optionName, value, defValue, true);
  }

  /**
   * Function template for returning the value for a given option. It searches at first in command line and if
   * not found it also searches in configuration file
   *
   * @param optionName - The option name
   * @param value - Returned value
   * @return - true if the option was found either in command line or in configuration file or a default value is given and false otherwise
   */
  template<typename T>
  bool getOption(const std::string& optionName, T& value) const
  {
    return getOption<T>(optionName, value, value, false);
  }

  /**
   * Parse both command line and configuration file
   * @param parseOptionsFlags - Parse flags
   * @param strm - There stream where to send erros
   * @return true if no error occurred during parse or false otherwise
   */
  bool parseOptions(ParseOptionsFlags parseOptionsFlags = DefaultOptionsFlag, std::ostream& strm = std::cerr);

protected:
  /**
   * Registers required and alternative options names
   *
   * @param optionName - The option name
   * @param altOptionName - The alternative option name
   */
  void registerRequiredParameters(const std::string& optionName, const std::string& altOptionName);

  /**
   * Validate both required and alternative options names
   *
   * @param strm - The stream where to send errors
   * @return - true in case of success or false otherwise
   */
  bool validateRequiredParameters(std::ostream& strm);

  /**
   * Internal function for parsing configuration file
   *
   * @param parseOptionsFlags - Parse flags
   * @return - true in case of success or false otherwise
   */
  bool parseConfigurationFile(ParseOptionsFlags parseOptionsFlags);

  // Adds default command line options
  void addDefaultCommandLineOptions();

  /**
   * Checks default options for command line
   *
   * @param parseOptionsFlags - Parse flags
   * @param strm - The stream where to send errors
   */
  void checkDefaultCommandLineOptions(ParseOptionsFlags parseOptionsFlags, std::ostream& strm);

  /**
   * Checks and parse configuration file
   *
   * @param parseOptionsFlags - Parse flags
   * @param strm - The stream where to send errors
   * @return - true in case of success or false otherwise
   */
  bool checkParseConfigurationFile(ParseOptionsFlags parseOptionsFlags, std::ostream& strm);

  /**
   * Function template for returning the value for a given option. It searches at first in command line and if
   * not found it also searches in configuration file. It searches only in registered options
   *
   * @param optionName - The option name
   * @param value - Returned value
   * @return - true if the option was found either in command line or in configuration file and false otherwise
   */
  template<typename T>
  bool getRegisteredOption(const std::string& optionName, T& value) const
  {
    OptionType optionType;
    bool retCode = false;

    do
    {
      if (!hasOption(optionName, &optionType))
        break;

      //
      // Check if _configFileOptions ptree has it
      //
      if (optionType == ConfigOption)
      {
        try
        {
          value = _configFileOptions[optionName.c_str()].as<T>();
          retCode = true;
          break;
        }
        catch(...)
        {
          break;
        }
      }

      if (optionType == CommandLineOption)
      {
        value = _commandLineOptions[optionName.c_str()].as<T>();
        retCode = true;
        break;
      }
    }
    while (false);

    return retCode;
  }

  // Function template for returning the value for a given option. It searches only in unregistered options
  template<typename T>
  bool getUnregisteredOption(const std::string& optionName, T& value) const
  {
    bool rc = false;

    do
    {
      std::map<std::string, std::string>::const_iterator it;
      it = _unregisteredOptions.find(optionName);
      if (_unregisteredOptions.end() == it)
      {
        break;
      }

      try
      {
        value = boost::lexical_cast<T>(it->second);
        rc = true;
        break;
      }
      catch(...)
      {
        break;
      }
    }
    while (false);

    return rc;
  }

  // Function template for returning the value for a given option. It searches both registered and unregistered options
  template<typename T>
  bool getOption(const std::string& optionName, T& value, const T defValue, bool haveDefaultValue) const
  {
    bool rc = false;

    do
    {
      rc = getRegisteredOption<T>(optionName, value);
      if (true == rc)
      {
        break;
      }

      // search also in unregistered options
      rc = getUnregisteredOption<T>(optionName, value);
      if (true == rc)
      {
        break;
      }

      if (haveDefaultValue)
      {
        value = defValue;
        rc = true;
        break;
      }
    }
    while (false);

    return rc;
  }

  int _argc;        // Number of arguments
  char** _pArgv;    // Array of command line arguments

  //
  boost::program_options::options_description _commandLineOptionsDescription; // Command line options description
  boost::program_options::options_description _configFileOptionsDescription;  // Configuration file options description
  boost::program_options::options_description _allOptionsDescription;         // All options description

  // A map that contains the options that are unregistered before starting parsing.
  std::map<std::string, std::string> _unregisteredOptions;
  
  boost::program_options::variables_map _commandLineOptions;

  std::string _configFile;                                       // Configuration file path

  
  boost::program_options::variables_map _configFileOptions;                // Configuration file options
  bool _hasConfig;                                               // Is set true if configuration file is set
  bool _isParsed;                                                // Is set true if configuration file and command line are already parsed

  std::vector<std::string> _requiredOptionsNames;                // A vector that contains required parameters

  // A map that contains alternative parameter for a required parameter.
  std::map<std::string, std::string> _alternative;

  /* Configuration Database */
  OsConfigDb _configDb;

  // The string that will be displayed after the normal help
  std::string _extraHelp;

  friend class OsServiceOptionsTest;                // friend clase used for unit tests
};

//
// Inlines
//

inline void OsServiceOptions::setExtraHelp(std::string& extraHelp)
{
  _extraHelp = extraHelp;
}

inline OsConfigDb& OsServiceOptions::getOsConfigDb()
{
  return _configDb;
}

inline bool OsServiceOptions::getOption(const std::string& optionName, int& value, const int defValue) const
{
  return getOption<int>(optionName, value, defValue, true);
}

inline bool OsServiceOptions::getOption(const std::string& optionName, int& value) const
{
  return getOption<int>(optionName, value, value, false);
}

inline bool OsServiceOptions::getOption(const std::string& optionName, std::string& value, const std::string defValue) const
{
  return getOption<std::string>(optionName, value, defValue, true);
}

inline bool OsServiceOptions::getOption(const std::string& optionName, std::string& value) const
{
  return getOption<std::string>(optionName, value, value, false);
}

inline bool OsServiceOptions::getOption(const std::string& optionName, std::vector<std::string>& value) const
{
  return getRegisteredOption<std::vector<std::string> >(optionName, value);
}


inline bool OsServiceOptions::getOption(const std::string& optionName, std::vector<int>& value) const
{
  return getRegisteredOption<std::vector<int> >(optionName, value);
}

inline bool OsServiceOptions::getOption(const std::string& optionName, UtlString& value, const UtlString defValue) const
{
  std::string stringValue;
  std::string stringDefValue;

  stringDefValue = defValue;

  bool rc = getOption<std::string>(optionName, stringValue, stringDefValue, true);
  value = stringValue;

  return rc;
}

inline bool OsServiceOptions::getOption(const std::string& optionName, UtlString& value) const
{
  std::string stringValue;


  bool rc = getOption<std::string>(optionName, stringValue, stringValue, false);
  value = stringValue;

  return rc;
}

inline bool OsServiceOptions::getOption(const std::string& optionName, bool& value, const bool defValue) const
{
  std::string stringValue;
  std::string stringDefValue = "false";

  if (defValue == true)
    stringDefValue = "true";

  bool rc = getOption<std::string>(optionName, stringValue, stringDefValue, true);

  if (!stringValue.empty())
  {
    char ch = stringValue.at(0);
    value = (ch == '1' || ch == 't' || ch == 'T');
  }

  return rc;
}

inline bool OsServiceOptions::getOption(const std::string& optionName, bool& value) const
{
  std::string stringValue;

  bool rc = getOption<std::string>(optionName, stringValue, stringValue, false);

  if (!stringValue.empty())
  {
    char ch = stringValue.at(0);
    value = (ch == '1' || ch == 't' || ch == 'T');
  }

  return rc;
}

inline void OsServiceOptions::addOptionInt(char shortForm,
  const std::string& optionName,
  const std::string description,
  OptionType type,
  bool required,
  const std::string& altOptionName)
{
  addOption<int>(shortForm, optionName, description, type, required, altOptionName);
}

inline void OsServiceOptions::addOptionInt(const std::string& optionName,
  const std::string description,
  OptionType type,
  bool required,
  const std::string& altOptionName)
{
  addOption<int>(0, optionName, description, type, required, altOptionName);
}

inline void OsServiceOptions::addOptionString(char shortForm,
  const std::string& optionName,
  const std::string description,
  OptionType type,
  bool required,
  const std::string& altOptionName)
{
  addOption<std::string>(shortForm, optionName, description, type, required, altOptionName);
}

inline void OsServiceOptions::addOptionString(const std::string& optionName,
  const std::string description,
  OptionType type,
  bool required,
  const std::string& altOptionName)
{
  addOption<std::string>(0, optionName, description, type, required, altOptionName);
}

inline void OsServiceOptions::addOptionStringVector(char shortForm,
  const std::string& optionName,
  const std::string description,
  OptionType type,
  bool required,
  const std::string& altOptionName)
{
  addOption< std::vector<std::string> >(shortForm, optionName, description, type, required, altOptionName);
}

inline void OsServiceOptions::addOptionStringVector(const std::string& optionName,
  const std::string description,
  OptionType type,
  bool required,
  const std::string& altOptionName)
{
  addOption< std::vector<std::string> >(0, optionName, description, type, required, altOptionName);
}

inline void OsServiceOptions::addOptionIntVector(char shortForm,
  const std::string& optionName,
  const std::string description,
  OptionType type,
  bool required,
  const std::string& altOptionName)
{
  addOption< std::vector<int> >(shortForm, optionName, description, type, required, altOptionName);
}

inline void OsServiceOptions::addOptionIntVector(const std::string& optionName,
  const std::string description,
  OptionType type,
  bool required,
  const std::string& altOptionName)
{
  addOption< std::vector<int> >(0, optionName, description, type, required, altOptionName);
}

#endif	/// OSSERVICEOPTIONS_H_INCLUDED


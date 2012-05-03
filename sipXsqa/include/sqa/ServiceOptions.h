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
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/detail/ptree_utils.hpp>
#include <stdexcept>
#include <locale>
#include "os/OsLogger.h"







namespace boost { namespace property_tree { namespace sipx_ini_parser
{
    /**
     * Determines whether the @c flags are valid for use with the ini_parser.
     * @param flags value to check for validity as flags to ini_parser.
     * @return true if the flags are valid, false otherwise.
     */
    inline bool validate_flags(int flags)
    {
        return flags == 0;
    }

    /** Indicates an error parsing INI formatted data. */
    class sipx_ini_parser_error: public file_parser_error
    {
    public:
        /**
         * Construct an @c sipx_ini_parser_error
         * @param message Message describing the parser error.
         * @param filename The name of the file being parsed containing the
         *                 error.
         * @param line The line in the given file where an error was
         *             encountered.
         */
        sipx_ini_parser_error(const std::string &message,
                         const std::string &filename,
                         unsigned long line)
            : file_parser_error(message, filename, line)
        {
        }
    };

    /**
     * Read INI from a the given stream and translate it to a property tree.
     * @note Clears existing contents of property tree. In case of error
     *       the property tree is not modified.
     * @throw sipx_ini_parser_error If a format violation is found.
     * @param stream Stream from which to read in the property tree.
     * @param[out] pt The property tree to populate.
     */
    template<class Ptree>
    void sipx_read_ini(std::basic_istream<
                    typename Ptree::key_type::value_type> &stream,
                  Ptree &pt,
                  char pairDelimiter)
    {
        typedef typename Ptree::key_type::value_type Ch;
        typedef std::basic_string<Ch> Str;
        const Ch semicolon = stream.widen(';');
        const Ch hash = stream.widen('#');
        const Ch lbracket = stream.widen('[');
        const Ch rbracket = stream.widen(']');

        Ptree local;
        unsigned long line_no = 0;
        Ptree *section = 0;
        Str line;

        // For all lines
        while (stream.good())
        {

            // Get line from stream
            ++line_no;
            std::getline(stream, line);
            if (!stream.good() && !stream.eof())
                BOOST_PROPERTY_TREE_THROW(sipx_ini_parser_error(
                    "read error", "", line_no));

            // If line is non-empty
            line = property_tree::detail::trim(line, stream.getloc());
            if (!line.empty())
            {
                // Comment, section or key?
                if (line[0] == semicolon || line[0] == hash)
                {
                    // Ignore comments
                }
                else if (line[0] == lbracket)
                {
                    // If the previous section was empty, drop it again.
                    if (section && section->empty())
                        local.pop_back();
                    typename Str::size_type end = line.find(rbracket);
                    if (end == Str::npos)
                        BOOST_PROPERTY_TREE_THROW(sipx_ini_parser_error(
                            "unmatched '['", "", line_no));
                    Str key = property_tree::detail::trim(
                        line.substr(1, end - 1), stream.getloc());
                    if (local.find(key) != local.not_found())
                        BOOST_PROPERTY_TREE_THROW(sipx_ini_parser_error(
                            "duplicate section name", "", line_no));
                    section = &local.push_back(
                        std::make_pair(key, Ptree()))->second;
                }
                else
                {
                    Ptree &container = section ? *section : local;
                    typename Str::size_type eqpos = line.find(Ch(pairDelimiter));
                    if (eqpos == Str::npos)
                        BOOST_PROPERTY_TREE_THROW(sipx_ini_parser_error(
                            "'=' character not found in line", "", line_no));
                    if (eqpos == 0)
                        BOOST_PROPERTY_TREE_THROW(sipx_ini_parser_error(
                            "key expected", "", line_no));
                    Str key = property_tree::detail::trim(
                        line.substr(0, eqpos), stream.getloc());
                    Str data = property_tree::detail::trim(
                        line.substr(eqpos + 1, Str::npos), stream.getloc());
                    if (container.find(key) != container.not_found())
                        BOOST_PROPERTY_TREE_THROW(sipx_ini_parser_error(
                            "duplicate key name", "", line_no));
                    container.push_back(std::make_pair(key, Ptree(data)));
                }
            }
        }
        // If the last section was empty, drop it again.
        if (section && section->empty())
            local.pop_back();

        // Swap local ptree with result ptree
        pt.swap(local);

    }

    /**
     * Read INI from a the given file and translate it to a property tree.
     * @note Clears existing contents of property tree.  In case of error the
     *       property tree unmodified.
     * @throw sipx_ini_parser_error In case of error deserializing the property tree.
     * @param filename Name of file from which to read in the property tree.
     * @param[out] pt The property tree to populate.
     * @param loc The locale to use when reading in the file contents.
     */
    template<class Ptree>
    void sipx_read_ini(const std::string &filename,
                  Ptree &pt,
                  const std::locale &loc = std::locale(),
                  char pairDelimiter = ':')
    {
        std::basic_ifstream<typename Ptree::key_type::value_type>
            stream(filename.c_str());
        if (!stream)
            BOOST_PROPERTY_TREE_THROW(sipx_ini_parser_error(
                "cannot open file", filename, 0));
        stream.imbue(loc);
        try {
            sipx_read_ini(stream, pt, pairDelimiter);
        }
        catch (sipx_ini_parser_error &e) {
            BOOST_PROPERTY_TREE_THROW(sipx_ini_parser_error(
                e.message(), filename, e.line()));
        }
    }

    namespace detail
    {
        template<class Ptree>
        void check_dupes(const Ptree &pt)
        {
            if(pt.size() <= 1)
                return;
            const typename Ptree::key_type *lastkey = 0;
            typename Ptree::const_assoc_iterator it = pt.ordered_begin(),
                                                 end = pt.not_found();
            lastkey = &it->first;
            for(++it; it != end; ++it) {
                if(*lastkey == it->first)
                    BOOST_PROPERTY_TREE_THROW(sipx_ini_parser_error(
                        "duplicate key", "", 0));
                lastkey = &it->first;
            }
        }
    }

    /**
     * Translates the property tree to INI and writes it the given output
     * stream.
     * @pre @e pt cannot have data in its root.
     * @pre @e pt cannot have keys both data and children.
     * @pre @e pt cannot be deeper than two levels.
     * @pre There cannot be duplicate keys on any given level of @e pt.
     * @throw sipx_ini_parser_error In case of error translating the property tree to
     *                         INI or writing to the output stream.
     * @param stream The stream to which to write the INI representation of the
     *               property tree.
     * @param pt The property tree to tranlsate to INI and output.
     * @param flags The flags to use when writing the INI file.
     *              No flags are currently supported.
     */
    template<class Ptree>
    void sipx_write_ini(std::basic_ostream<
                       typename Ptree::key_type::value_type
                   > &stream,
                   const Ptree &pt,
                   int flags = 0,
                   char pairDelimiter = ':')
    {
        using detail::check_dupes;

        typedef typename Ptree::key_type::value_type Ch;
        typedef std::basic_string<Ch> Str;

        BOOST_ASSERT(validate_flags(flags));
        (void)flags;

        if (!pt.data().empty())
            BOOST_PROPERTY_TREE_THROW(sipx_ini_parser_error(
                "ptree has data on root", "", 0));
        check_dupes(pt);

        for (typename Ptree::const_iterator it = pt.begin(), end = pt.end();
             it != end; ++it)
        {
            check_dupes(it->second);
            if (it->second.empty()) {
                stream << it->first << Ch(pairDelimiter)
                    << it->second.template get_value<
                        std::basic_string<Ch> >()
                    << Ch('\n');
            } else {
                if (!it->second.data().empty())
                    BOOST_PROPERTY_TREE_THROW(sipx_ini_parser_error(
                        "mixed data and children", "", 0));
                stream << Ch('[') << it->first << Ch(']') << Ch('\n');
                for (typename Ptree::const_iterator it2 = it->second.begin(),
                         end2 = it->second.end(); it2 != end2; ++it2)
                {
                    if (!it2->second.empty())
                        BOOST_PROPERTY_TREE_THROW(sipx_ini_parser_error(
                            "ptree is too deep", "", 0));
                    stream << it2->first << Ch(pairDelimiter)
                        << it2->second.template get_value<
                            std::basic_string<Ch> >()
                        << Ch('\n');
                }
            }
        }

    }

    /**
     * Translates the property tree to INI and writes it the given file.
     * @pre @e pt cannot have data in its root.
     * @pre @e pt cannot have keys both data and children.
     * @pre @e pt cannot be deeper than two levels.
     * @pre There cannot be duplicate keys on any given level of @e pt.
     * @throw info_parser_error In case of error translating the property tree
     *                          to INI or writing to the file.
     * @param filename The name of the file to which to write the INI
     *                 representation of the property tree.
     * @param pt The property tree to tranlsate to INI and output.
     * @param flags The flags to use when writing the INI file.
     *              The following flags are supported:
     * @li @c skip_ini_validity_check -- Skip check if ptree is a valid ini. The
     *     validity check covers the preconditions but takes <tt>O(n log n)</tt>
     *     time.
     * @param loc The locale to use when writing the file.
     */
    template<class Ptree>
    void sipx_write_ini(const std::string &filename,
                   const Ptree &pt,
                   int flags = 0,
                   const std::locale &loc = std::locale(),
                   char pairDelimiter = ':')
    {
        std::basic_ofstream<typename Ptree::key_type::value_type>
            stream(filename.c_str());
        if (!stream)
            BOOST_PROPERTY_TREE_THROW(sipx_ini_parser_error(
                "cannot open file", filename, 0));
        stream.imbue(loc);
        try {
            sipx_write_ini(stream, pt, flags, pairDelimiter);
        }
        catch (sipx_ini_parser_error &e) {
            BOOST_PROPERTY_TREE_THROW(sipx_ini_parser_error(
                e.message(), filename, e.line()));
        }
    }

} } }

namespace boost { namespace property_tree
{
    using sipx_ini_parser::sipx_ini_parser_error;
    using sipx_ini_parser::sipx_read_ini;
    using sipx_ini_parser::sipx_write_ini;
} }


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
  size_t hasOption(const std::string& optionName, bool consolidate = true) const;
  size_t hasConfigOption(const std::string& optionName) const;
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
      getOption("pid-file", _pidFile);

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
          boost::property_tree::sipx_ini_parser::sipx_read_ini(_configFile.c_str(), _ptree);
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
      std::cerr << _daemonName << " is not able to parse the options - " << e.what() << std::endl;
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
  if (hasOption("log-file", true))
  {
    if (getOption("log-file", logFile) && !logFile.empty())
    {
      if (hasOption("log-level", true))
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

inline std::size_t ServiceOptions::hasOption(const std::string& optionName, bool consolidate) const
{
  std::size_t ct = _options.count(optionName.c_str());
  if (!ct && consolidate && _hasConfig)
    ct = _ptree.count(optionName.c_str());
  return ct;
}

inline size_t ServiceOptions::hasConfigOption(const std::string& optionName) const
{
  return _ptree.count(optionName.c_str());
}

inline bool ServiceOptions::getOption(const std::string& optionName, std::string& value, const std::string& defValue) const
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

inline bool ServiceOptions::getOption(const std::string& optionName, std::vector<std::string>& value) const
{
  if (!hasOption(optionName, false))
    return false;
  value = _options[optionName.c_str()].as<std::vector<std::string> >();
  return true;
}

inline bool ServiceOptions::getOption(const std::string& optionName, int& value) const
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

inline bool ServiceOptions::getOption(const std::string& optionName, int& value, int defValue) const
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

inline bool ServiceOptions::getOption(const std::string& optionName, std::vector<int>& value) const
{
  if (!hasOption(optionName,false))
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


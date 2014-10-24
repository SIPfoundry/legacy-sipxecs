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

#include "AppConfig.h"

#include "os/OsServiceOptions.h"

#include "sipdb/DbHelper.h"

#include <boost/format.hpp>

#include <sipXecsService/SipXApplication.h>


using namespace MongoDBTool;

const char* MongoDBTool::pListEntriesConfOpt                          = "print-entries";
const char* MongoDBTool::pDeleteEntriesConfOpt                        = "delete-entries";
const char* MongoDBTool::pDatabaseNameConfOpt                         = "select-database";
const char* MongoDBTool::pMultipleLinesConfOpt                        = "multiple-lines";
const char* MongoDBTool::pWhereConfOpt                                = "where";
const char* MongoDBTool::pNodeRegistrarDbName                         = "node.registrar";
const char* MongoDBTool::pImdbEntityDbName                            = "imdb.entity";

#define SIPXREG_TOOL_APP_NAME "sipXregTool"

AppConfig::AppConfig(int argc, char** pArgv) :
                                                _hasOptListEntries(false),
                                                _hasOptDeleteEntries(false),
                                                _hasOptMultipleLines(false),
                                                _hasOptDatabaseName(false),
                                                _hasOptWhere(false),
                                                _argc(argc),
                                                _pArgv(pArgv)
{
}

AppConfig::~AppConfig()
{
}

void AppConfig::createExtraHelp(std::ostream& strm) const
{
  strm << boost::format("Examples:\n");
  strm << boost::format("%s -p -s <database_name> -w '<left_operand> <logical_operator> <right_operand>' where:\n") % _pArgv[0];
  strm << boost::format("%|2t|<database_name> %|30t|- can be imdb.entity or node.registrar\n");
  strm << boost::format("%|2t|<left_operand> %|30t|- the key from db that is used for creating the filter condition\n");
  strm << boost::format("%|2t|<logical_operator> %|30t|- can be: <, >, <=, >=, = or !=\n");
  strm << boost::format("%|2t|<right_operand> %|30t|- the value to which the key is compared\n");
  strm << boost::format("\n");
  strm << boost::format("%s -p -w 'cseq < 812' -w 'expirationTime < $now'   Print only entries that have cseq lower than 812 and expirationTime lower than current time\n") % _pArgv[0];
  strm << boost::format("%s -d -w 'cseq = 812'   Delete only entries that have cseq equal to 812\n") % _pArgv[0];
  strm << boost::format("%s -p   Print all entries\n") % _pArgv[0];
  strm << boost::format("%s -d   Delete all entries\n") % _pArgv[0];
}

bool AppConfig::setDefaultOptions()
{
  SipXApplication& sipXApplication = SipXApplication::instance();
  OsServiceOptions& osServiceOptions = sipXApplication.getConfig();

  osServiceOptions.addOptionString('s', pDatabaseNameConfOpt, ": Use database. By default node.registrar is chosen.", OsServiceOptions::ConfigOption);
  osServiceOptions.addOptionFlag('p', pListEntriesConfOpt, ": Print entries from selected database.", OsServiceOptions::ConfigOption);
  osServiceOptions.addOptionFlag('d', pDeleteEntriesConfOpt, ": Delete entries from selected database.", OsServiceOptions::ConfigOption);
  osServiceOptions.addOptionFlag('m', pMultipleLinesConfOpt, ": Display database entry on multiple lines.", OsServiceOptions::ConfigOption);
  osServiceOptions.addOptionStringVector('w', pWhereConfOpt, ": The filter condition used for listing or deleting entries", OsServiceOptions::ConfigOption);


  return true;
}

void AppConfig::displayUsage(std::ostream& strm) const
{
  SipXApplication& sipXApplication = SipXApplication::instance();

  sipXApplication.displayUsage(strm);
}

void AppConfig::checkOptions()
{
  SipXApplication& sipXApplication = SipXApplication::instance();
  OsServiceOptions& osServiceOptions = sipXApplication.getConfig();

  if (osServiceOptions.hasOption(pListEntriesConfOpt))
  {
    _hasOptListEntries = true;
  }

  if (osServiceOptions.hasOption(pDeleteEntriesConfOpt))
  {
    _hasOptDeleteEntries = true;
  }

  if (osServiceOptions.hasOption(pMultipleLinesConfOpt))
  {
    _hasOptMultipleLines = true;
  }

  if (osServiceOptions.hasOption(pDatabaseNameConfOpt))
  {
    osServiceOptions.getOption(MongoDBTool::pDatabaseNameConfOpt, _databaseName);
    _hasOptDatabaseName = true;
  }
  else
  {
    _databaseName = pNodeRegistrarDbName;
  }

  if (osServiceOptions.hasOption(pWhereConfOpt))
  {
    osServiceOptions.getOption(MongoDBTool::pWhereConfOpt, _whereOptVector);
    _hasOptWhere = true;
  }

  if (osServiceOptions.hasOption(pDeleteEntriesConfOpt) &&
      osServiceOptions.hasOption(pListEntriesConfOpt))
  {
    std::cerr  << boost::format("%s is mutual exclusive with %s. Please use only one option.\n") % pListEntriesConfOpt % pDeleteEntriesConfOpt;
    exit(-1);
  }
}

bool AppConfig::parseOptions()
{
  SipXApplicationData regToolData =
  {
      SIPXREG_TOOL_APP_NAME,
      "",
      "",
      "",
      "",

      false, // do not check mongo connection
      false, // do not enable mongo driver logging
      false, // increase application file descriptor limits
      true, // block signals on main thread (and all other threads created by main)
            // and process them only on a dedicated thread
      SipXApplicationData::ConfigFileFormatIni, // format type for configuration file
      OsMsgQShared::QUEUE_LIMITED, //limited queue
  };

  SipXApplication& sipXApplication = SipXApplication::instance();               // SipXApplication class
  OsServiceOptions& osServiceOptions = sipXApplication.getConfig();

  std::ostringstream stream;
  createExtraHelp(stream);

  std::string extraHelp = stream.str();
  osServiceOptions.setExtraHelp(extraHelp);


  sipXApplication.init(_argc, _pArgv, regToolData);

  checkOptions();



  return true;
}

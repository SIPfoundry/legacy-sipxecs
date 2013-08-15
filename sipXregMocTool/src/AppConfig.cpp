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


using namespace MongoDBTool;

const char* MongoDBTool::pListEntriesConfOpt                          = "list-entries";
const char* MongoDBTool::pDeleteEntriesConfOpt                        = "delete-entries";
const char* MongoDBTool::pDatabaseNameConfOpt                         = "select-database";
const char* MongoDBTool::pMultipleLinesConfOpt                        = "multiple-lines";
const char* MongoDBTool::pWhereConfOpt                                = "where";

AppConfig::AppConfig(int argc, char** pArgv) : OsServiceOptions(argc, pArgv, "sipXregMocTool", "1.0", "Ezuce Inc. All Rights Reserved"),
                                              _hasOptListEntries(false),
                                              _hasOptDeleteEntries(false),
                                              _hasOptMultipleLines(false),
                                              _hasOptDatabaseName(false),
                                              _hasOptWhere(false)
{
}

AppConfig::~AppConfig()
{
}

void AppConfig::displayUsage(std::ostream& strm) const
{
   OsServiceOptions::displayUsage(strm);

   strm << boost::format("Examples:\n");
   strm << boost::format("%s -l -s <database_name> -w '<left_operand> <logical_operator> <right_operand>' where:\n") % _argv[0];
   strm << boost::format("%|2t|<database_name> %|30t|- can be imdb.entity or node.registrar\n");
   strm << boost::format("%|2t|<left_operand> %|30t|- the key from db that is used for creating the filter condition\n");
   strm << boost::format("%|2t|<logical_operator> %|30t|- can be: <, >, <=, >=, = or !=\n");
   strm << boost::format("%|2t|<right_operand> %|30t|- the value to which the key is compared\n");
   strm << boost::format("\n");
   strm << boost::format("%s -l -w 'cseq < 812' -w 'expirationTime < $now'   List only entries that have cseq lower than 812 and expirationTime lower than current time\n") % _argv[0];
   strm << boost::format("%s -d -w 'cseq = 812'   Delete only entries that have cseq equal to 812\n") % _argv[0];
   strm << boost::format("%s -l   List all entries\n") % _argv[0];
   strm << boost::format("%s -d   Delete all entries\n") % _argv[0];
}

bool AppConfig::setDefaultOptions()
{
   try
   {
      addOptionFlag('h', "help", ": Display help information.", CommandLineOption);
      addOptionFlag('v', "version", ": Display version information.", CommandLineOption);
      addOptionString('s', pDatabaseNameConfOpt, ": Use database. By default node.registrar is chosen.", CommandLineOption);
      addOptionFlag('l', pListEntriesConfOpt, ": List entries from selected database.", CommandLineOption);
      addOptionFlag('d', pDeleteEntriesConfOpt, ": Delete entries from selected database.", CommandLineOption);
      addOptionFlag('m', pMultipleLinesConfOpt, ": Display database entry on multiple lines.", CommandLineOption);
      addOptionStringVector('w', pWhereConfOpt, ": The filter condition used for listing or deleting entries", CommandLineOption);
   }
   catch(const std::exception& e)
   {
      return false;
   }

   return true;
}

void AppConfig::checkOptions()
{
   if (OsServiceOptions::hasOption(pListEntriesConfOpt, false))
   {
      _hasOptListEntries = true;
   }

   if (OsServiceOptions::hasOption(pDeleteEntriesConfOpt, false))
   {
      _hasOptDeleteEntries = true;
   }

   if (OsServiceOptions::hasOption(pMultipleLinesConfOpt, false))
   {
      _hasOptMultipleLines = true;
   }

   if (OsServiceOptions::hasOption(pDatabaseNameConfOpt, false))
   {
      OsServiceOptions::getOption(MongoDBTool::pDatabaseNameConfOpt, _databaseName);
      _hasOptDatabaseName = true;
   }
   else
   {
      _databaseName = DbHelper::pNodeRegistrarDbName;
   }

   if (OsServiceOptions::hasOption(pWhereConfOpt, false))
   {
      OsServiceOptions::getOption(MongoDBTool::pWhereConfOpt, _whereOptVector);
      _hasOptWhere = true;
   }

   if (OsServiceOptions::hasOption(pDeleteEntriesConfOpt, false) &&
       OsServiceOptions::hasOption(pListEntriesConfOpt, false))
   {
      std::cerr  << boost::format("%s is mutual exclusive with %s. Please use only one option.\n") % pListEntriesConfOpt % pDeleteEntriesConfOpt;
      exit(-1);
   }
}

bool AppConfig::parseOptions()
{
   _optionItems.add(_commandLineOptions);

   OsServiceOptions::parseOptions(OsServiceOptions::DisplayExceptionFlag);

   if (OsServiceOptions::hasOption("help", false))
   {
      displayUsage(std::cout);
      exit(0);
   }

   if (OsServiceOptions::hasOption("version", false))
   {
      displayVersion(std::cout);
      exit(0);
   }

   checkOptions();



   return true;
}

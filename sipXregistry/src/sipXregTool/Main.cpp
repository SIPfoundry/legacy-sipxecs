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

#include <string>

#include "Main.h"

#include "AppConfig.h"
#include "AppPerform.h"

#include <boost/exception/get_error_info.hpp>

int main(int argc, char** pArgv)
{
   try
   {
      // initialize the Mongo client driver
      mongo::Status status = mongo::client::initialize();
      if (!status.isOK())
      {
        fprintf(stderr, "Failed to initialize Mongo client driver: %s\n", status.toString().c_str());
        OS_LOG_ERROR(FAC_ODBC, "Failed to initialize Mongo client driver: " << status.toString());
        exit(1);
      }

      MongoDBTool::AppConfig appConfig(argc, pArgv);
      MongoDBTool::AppPerform appPerform;


      /* Set configuration options */
      appConfig.setDefaultOptions();

      /* Parse configuration options */
      if (false == appConfig.parseOptions())
         return -1;

      /* Pass a pointer to configuration AppConfig class */
      appPerform.setConfig(&appConfig);


      if (appConfig.hasOptListEntries())
      {
         /* Print database entries */
         appPerform.printDbEntries(appConfig.whereOptVector(), appConfig.databaseName(), appConfig.hasOptMultipleLines());
      }
      else if (appConfig.hasOptDeleteEntries())
      {
         /* Delete entries from selected database */
         appPerform.deleteDbEntries(appConfig.whereOptVector(), appConfig.databaseName());
      }
      else
      {
         /* Display application help */
         appConfig.displayUsage(std::cout);
      }
   }
   catch(const std::exception& e)
   {
      if (std::string const* pExtra  = boost::get_error_info<DbHelperTagInfo>(e) )
      {
          std::cerr << "Exception: " << *pExtra << std::endl;
      }
      else
      {
         std::cerr << "Exception: " << e.what() << std::endl;
      }


      //
      return -1;
   }

   return 0;
}

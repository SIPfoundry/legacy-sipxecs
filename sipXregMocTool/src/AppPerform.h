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

#ifndef APP_PERFORM_H
#define APP_PERFORM_H

#include "sipdb/DbHelper.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/greg_month.hpp>

#include <boost/format.hpp>



namespace MongoDBTool
{
class AppConfig;

extern const char* pTimeNowMacro;


/**
 * Application perform class
 * Used for printing or deleting entries from selected database
 */
class AppPerform : public DbHelper
{
public:

   // Constructor
   AppPerform();

   // Destructor
   ~AppPerform();

   /** Print entries from selected database. It can either print all entries or
    *  only some of them filtered by a certain field id
    */
   void printDbEntries(std::vector<std::string>& whereOptVector,
                                    const std::string& databaseName,
                                    bool multipleLines);

   /** Delete entries from selected database. It can either delete all entries or
    * only some of them filtered by a certain field id
    */
   void deleteDbEntries(std::vector<std::string>& whereOptVector,
                        const std::string& databaseName);

   // Save internally the pointer to AppConfig class
   void setConfig(AppConfig* pAppConfig);

protected:

private:
   AppConfig* _pAppConfig;                         // Pointer to the AppConfig class
   const MongoDB::ConnectionInfo* _connectionInfo; // Pointer to a ConnectionInfo class
};

inline void AppPerform::setConfig(AppConfig* pAppConfig){_pAppConfig = pAppConfig;}

};




#endif   /* APP_PERFORM_H */

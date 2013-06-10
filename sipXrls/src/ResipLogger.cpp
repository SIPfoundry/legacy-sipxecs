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


#include "ResipLogger.h"

namespace resip {


  ResipLogger::ResipLogger()
  {
  }

  ResipLogger::~ResipLogger()
  {
  }

   /** return true to also do default logging, false to supress default logging. */
   bool ResipLogger::operator()(Log::Level level,
                           const Subsystem& subsystem,
                           const Data& appName,
                           const char* file,
                           int line,
                           const Data& message,
                           const Data& messageWithHeaders)
   {
     OS_LOG_INFO(FAC_SIP, message.c_str());
     return false;
   }
}

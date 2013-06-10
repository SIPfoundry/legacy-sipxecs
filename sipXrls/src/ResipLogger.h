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

#ifndef REPROLOGGER_H_INCLUDED
#define	REPROLOGGER_H_INCLUDED

#include "rutil/Log.hxx"
#include "rutil/Logger.hxx"
#include "os/OsLogger.h"

namespace resip {


class ResipLogger : public ExternalLogger
{
public:
   ResipLogger();

   virtual ~ResipLogger();
   /** return true to also do default logging, false to supress default logging. */
   virtual bool operator()(Log::Level level,
                           const Subsystem& subsystem,
                           const Data& appName,
                           const char* file,
                           int line,
                           const Data& message,
                           const Data& messageWithHeaders);
};


} // resip



#endif	/// REPROLOGGER_H_INCLUDED


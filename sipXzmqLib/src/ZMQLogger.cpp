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

#include "ZMQLogger.h"

ZMQLogger::CallBack ZMQLogger::cb;
int ZMQLogger::verbosity = 0;
boost::mutex ZMQLogger::_mutex;

void ZMQLogger::log(int level, std::ostringstream& strm)
{
  if (!ZMQLogger::verbosity || level > ZMQLogger::verbosity)
    return;
  if (ZMQLogger::cb)
  {
    ZMQLogger::cb(strm.str());
  }
  else
  {
    ZMQLogger::_mutex.lock();
    std::cerr << strm.str() << std::endl;
    ZMQLogger::_mutex.unlock();
  }
}


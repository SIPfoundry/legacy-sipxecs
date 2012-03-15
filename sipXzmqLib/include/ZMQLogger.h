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

#ifndef ZMQLOGGER_H
#define	ZMQLOGGER_H

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <sstream>

class ZMQLogger
{
public:
  typedef boost::function<void(const std::string& )> CallBack;

  enum Verbosity
  {
    ERROR = 1,
    INFO,
    NOTICE,
    DEBUG
  };

  static void log(int level, std::ostringstream& strm);
  static CallBack cb;
  static int verbosity;
  static boost::mutex _mutex;
};

#define ZMQ_LOG(level, data) \
{ \
std::ostringstream strm; \
strm << data; \
ZMQLogger::log(level, strm); \
}

#define ZMQ_LOG_ERROR(data) ZMQ_LOG(ZMQLogger::ERROR, data)
#define ZMQ_LOG_INFO(data) ZMQ_LOG(ZMQLogger::INFO, data)
#define ZMQ_LOG_NOTICE(data) ZMQ_LOG(ZMQLogger::NOTICE, data)
#define ZMQ_LOG_DEBUG(data) ZMQ_LOG(ZMQLogger::DEBUG, data)

#endif	/* ZMQLOGGER_H */


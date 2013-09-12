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


#include <iostream>
#include <errno.h>

#include "os/OsResourceLimit.h"
#include "os/OsLogger.h"
#include "os/OsServiceOptions.h"

const char* RLIM_DEFAULT_INI = SIPX_CONFDIR "/resource-limits.ini";

OsResourceLimit::ReverseEUID::ReverseEUID(const OsResourceLimit& limit) : _limit(limit)
{
  seteuid(_limit.getUID());
}

OsResourceLimit::ReverseEUID::~ReverseEUID()
{
  seteuid(_limit.getEUID());
}

OsResourceLimit::CurrentLimit::CurrentLimit(int resource) :
  _resource(resource),
  _soft(0),
  _hard(0),
  _error(false)
{
  struct rlimit limit;
  if (getrlimit(resource, &limit) == 0)
  {
    _soft = limit.rlim_cur;
    _hard = limit.rlim_max;
  }
  else
  {
    _error = true;
  }
}


OsResourceLimit::OsResourceLimit()
{
  _uid = getuid();
  _euid = geteuid();
}


static std::string getResourceString(int resource)
{
  switch(resource)
  {
  case RLIMIT_AS:
    return "RLIMIT_AS";
  case RLIMIT_CORE:
    return "RLIMIT_CORE";
  case RLIMIT_CPU:
    return "RLIMIT_CPU";
  case RLIMIT_DATA:
    return "RLIMIT_DATA";
  case RLIMIT_FSIZE:
    return "RLIMIT_FSIZE";
  case RLIMIT_LOCKS:
    return "RLIMIT_LOCKS";
  case RLIMIT_MEMLOCK:
    return "RLIMIT_MEMLOCK";
  case RLIMIT_MSGQUEUE:
    return "RLIMIT_MSGQUEUE";
  case RLIMIT_NICE:
    return "RLIMIT_NICE";
  case RLIMIT_NOFILE:
    return "RLIMIT_NOFILE";
  case RLIMIT_NPROC:
    return "RLIMIT_NPROC";
  case RLIMIT_RSS:
    return "RLIMIT_RSS";
  case RLIMIT_RTPRIO:
    return "RLIMIT_RTPRIO";
  case RLIMIT_SIGPENDING:
    return "RLIMIT_SIGPENDING";
  case RLIMIT_STACK:
    return "RLIMIT_STACK";
  default:
    return "RLIMIT_UNKNOWN";
  }

}



bool OsResourceLimit::setCurrentLimit(CurrentLimit& limit_) const
{
  ReverseEUID uid(*this);
  struct rlimit limit;
  Limit newSoft = limit.rlim_cur = limit_._soft;
  Limit newMax = limit.rlim_max = limit_._hard;
  if (setrlimit(limit_._resource, &limit) == 0)
  {
    Limit hard = 0;
    Limit soft = 0;
    getResourceLimit(limit_._resource, soft, hard);

    if (newSoft == soft && newMax == hard)
    {
      OS_LOG_NOTICE(FAC_KERNEL, "OsResourceLimit::setCurrentLimit: Resource " << getResourceString(limit_._resource)  << " set to soft: " << newSoft << " hard: " << newMax);
      return true;
    }
    else
    {
      OS_LOG_ERROR(FAC_KERNEL, "OsResourceLimit::setCurrentLimit: Unable to set resource " << getResourceString(limit_._resource) << " limit.  Error No: " << errno );
      return false;
    }
  }
  else
  {
    OS_LOG_ERROR(FAC_KERNEL, "OsResourceLimit::setCurrentLimit: Unable to set resource " << getResourceString(limit_._resource) << " limit.  Error No: " << errno );
    return false;
  }
}

bool OsResourceLimit::setResourceLimit(int resource, Limit soft) const
{
  CurrentLimit limit(resource);
  limit._soft = soft;
  return setCurrentLimit(limit);
}

bool OsResourceLimit::setResourceLimit(int resource, Limit soft, Limit hard) const
{
  CurrentLimit limit(resource);
  limit._soft = soft;
  limit._hard = hard;
  return setCurrentLimit(limit);
}

bool OsResourceLimit::getResourceLimit(int resource, Limit& soft, Limit& hard) const
{
  CurrentLimit limit(resource);
  if (limit._error)
  {
    OS_LOG_ERROR(FAC_KERNEL, "OsResourceLimit::getResourceLimit: Unable to get resource " << getResourceString(resource) << " limit.  Error No: " << errno );
    return false;
  }

  soft = limit._soft;
  hard = limit._hard;
  return true;
}


bool OsResourceLimit::setResourceLimitMaximum(int resource) const
{
  Limit soft = 0;
  Limit hard = 0;

  if (getResourceLimit(resource, soft, hard) && setResourceLimit(resource, hard))
  {
    OS_LOG_NOTICE(FAC_KERNEL, "OsResourceLimit::setResourceLimitMaximum: Resource " << getResourceString(resource)  << " set to " << hard);
    return true;
  }
  else
  {
    OS_LOG_ERROR(FAC_KERNEL, "OsResourceLimit::setResourceLimitMaximum: Unable to set resource " << getResourceString(resource) << " to maximum limit.  Error No: " << errno);
    return false;
  }
}

bool OsResourceLimit::setApplicationLimits(const std::string& executableName, const std::string& configPath_) const
{
  std::string configPath = configPath_;
  if (configPath.empty())
  {
    //
    // Use the default ini file
    //
    configPath = RLIM_DEFAULT_INI;
  }

  OsServiceOptions options(configPath);
  if (!options.parseOptions())
  {
    OS_LOG_ERROR(FAC_KERNEL, "OsResourceLimit::setApplicationLimits: Unable to load options from " << configPath);
    return false;
  }

  std::string fdCurName = executableName + RLIM_FD_CUR_SUFFIX;
  std::string fdMaxName = executableName + RLIM_FD_MAX_SUFFIX;

  std::string coreEnabledName = executableName + RLIM_CORE_ENABLED_SUFFIX;
  std::string coreCurName = executableName + RLIM_CORE_CUR_SUFFIX;
  std::string coreMaxName = executableName + RLIM_CORE_MAX_SUFFIX;

  Limit fdCur = 0;
  Limit fdMax = 0;
  int optionCount = 0;


  if (options.getOption<Limit>(fdCurName, fdCur))
  {
    optionCount++;
    if (options.getOption<Limit>(fdMaxName, fdMax))
    {
      if (!setFileDescriptorLimit(fdCur, fdMax))
      {
        //
        // There as an error setting the fdlimit.  Check if it was due to the fact that
        // nothing has changed
        //

        Limit oldFdCur;
        Limit oldFdMax;
        getFileDescriptorLimit(oldFdCur, oldFdMax);
        if (oldFdCur != fdCur && oldFdMax != fdMax)
          optionCount--;
      }
    }
    else
    {
      if (!setFileDescriptorLimit(fdCur))
        optionCount--;
    }


  }
  
  Limit coreCur = 0;
  Limit coreMax = 0;
  bool coreEnabled = false;

  if (options.getOption(coreEnabledName, coreEnabled) && coreEnabled)
  {
    optionCount++;
    if (options.getOption<Limit>(coreCurName, coreCur))
    {
      if (options.getOption<Limit>(coreMaxName, coreMax))
      {
        if (!setCoreFileLimit(coreCur, coreMax))
          optionCount--;
      }
      else
      {
        if (!setCoreFileLimit(coreCur))
          optionCount--;
      }
    }
    else
    {
      //
      // Explicit value is not specified.  Lets raise it to the maximum allowable
      //

      if (getCoreFileLimit(coreCur, coreMax))
      {
        if (!setCoreFileLimit(coreMax, coreMax))
        {
          optionCount--;
        }
      }
    }
  }


  return optionCount;
}




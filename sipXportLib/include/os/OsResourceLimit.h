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


#ifndef OSRESOURCELIMIT_H_INCLUDED
#define	OSRESOURCELIMIT_H_INCLUDED

#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>

#include <boost/noncopyable.hpp>


const char* RLIM_FD_CUR_SUFFIX = "-fd-soft";
const char* RLIM_FD_MAX_SUFFIX = "-fd-hard";
const char* RLIM_CORE_ENABLED_SUFFIX = "-core-enabled";
const char* RLIM_CORE_CUR_SUFFIX = "-core-soft";
const char* RLIM_CORE_MAX_SUFFIX = "-core-hard";

class OsResourceLimit : boost::noncopyable
{
public:
  typedef rlim_t Limit;

  class ReverseEUID
  {
  protected:
    ReverseEUID(const OsResourceLimit& limit);
    ~ReverseEUID();
  private:
    const OsResourceLimit& _limit;
    friend class OsResourceLimit;
  };

  class CurrentLimit
  {
  protected:
    CurrentLimit(int resource);
    int _resource;
    Limit _soft;
    Limit _hard;
    bool _error;
    friend class OsResourceLimit;
  };

  OsResourceLimit();

  bool setCurrentLimit(CurrentLimit& limit_) const;

  bool setResourceLimit(int resource, Limit soft) const;

  bool setResourceLimit(int resource, Limit soft, Limit hard) const;

  bool setResourceLimitMaximum(int resource) const;

  bool getResourceLimit(int resource, Limit& soft, Limit& hard) const;

  bool setFileDescriptorLimit(Limit soft) const;
  
  bool setFileDescriptorLimit(Limit soft, Limit hard) const;

  bool setFileDescriptorLimitToMaximum() const;

  bool getFileDescriptorLimit(Limit& soft, Limit& hard) const;

  bool setCoreFileLimit(Limit soft) const;

  bool setCoreFileLimit(Limit soft, Limit hard) const;

  bool setCoreFileLimitToMaximum() const;

  bool getCoreFileLimit(Limit& soft, Limit& hard) const;

  bool setApplicationLimits(const std::string& executableName, const std::string& configPath = std::string()) const;

protected:
  uid_t getEUID() const;
  void setEUID(uid_t euid);

  uid_t getUID() const;
  void setUID(uid_t uid);
private:
  uid_t _euid;
  uid_t _uid;
  friend class ReverseEUID;
};


//
// Inlines
//

inline uid_t OsResourceLimit::getEUID() const
{
  return _euid;
}

inline void OsResourceLimit::setEUID(uid_t euid)
{
  _euid = euid;
}

inline uid_t OsResourceLimit::getUID() const
{
  return _uid;
}

inline void OsResourceLimit::setUID(uid_t uid)
{
  _uid = uid;
}

inline bool OsResourceLimit::setFileDescriptorLimit(Limit soft) const
{
  return setResourceLimit(RLIMIT_NOFILE, soft);
}

inline bool OsResourceLimit::setFileDescriptorLimit(Limit soft, Limit hard) const
{
  return setResourceLimit(RLIMIT_NOFILE, soft, hard);
}

inline bool OsResourceLimit::setFileDescriptorLimitToMaximum() const
{
  return setResourceLimitMaximum(RLIMIT_NOFILE);
}

inline bool OsResourceLimit::getFileDescriptorLimit(Limit& soft, Limit& hard) const
{
  return getResourceLimit(RLIMIT_NOFILE, soft, hard);
}

inline bool OsResourceLimit::setCoreFileLimit(Limit soft) const
{
  return setResourceLimit(RLIMIT_CORE, soft);
}

inline bool OsResourceLimit::setCoreFileLimit(Limit soft, Limit hard) const
{
  return setResourceLimit(RLIMIT_CORE, soft, hard);
}

inline bool OsResourceLimit::setCoreFileLimitToMaximum() const
{
  return setResourceLimitMaximum(RLIMIT_CORE);
}

inline bool OsResourceLimit::getCoreFileLimit(Limit& soft, Limit& hard) const
{
  return getResourceLimit(RLIMIT_CORE, soft, hard);
}

#endif	/// OSRESOURCELIMIT_H_INCLUDED


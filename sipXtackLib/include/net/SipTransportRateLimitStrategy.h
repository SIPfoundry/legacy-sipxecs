/*
 *
 *
 * Copyright (c) 2010 eZuce, Inc. All rights reserved.
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


#ifndef SipTransportRateLimitStrategy_H
#define	SipTransportRateLimitStrategy_H


#include <set>
#include <map>
#include <sstream>
#include <vector>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/algorithm/string.hpp>


class SipTransportRateLimitStrategy
{
public:
    typedef boost::function<
        void(const boost::asio::ip::address& address,
        unsigned long thresholdViolationRate)> ViolationCallback;

  SipTransportRateLimitStrategy();
  ~SipTransportRateLimitStrategy();

  void logPacket(const boost::asio::ip::address& source, std::size_t bytesRead);

  bool isBannedAddress(const boost::asio::ip::address& source) const;

  bool isWhiteListedAddressRange(const boost::asio::ip::address& source) const;

  bool isBannedAddressRange(const boost::asio::ip::address& source) const;

  void banAddress(const boost::asio::ip::address& source, bool permanently);

  void clearAddress(const boost::asio::ip::address& source, bool addToWhiteList);

  unsigned long getPacketsPerSecondThreshold() const;

  void setPacketsPerSecondThreshold(unsigned long threshold);

  unsigned long getThresholdViolationRate() const;

  void setThresholdViolationRate(unsigned long threshold);

  unsigned long getCurrentIterationCount() const;

  bool& autoBanThresholdViolators();

  void setBanLifeTime(int lifetime);

  int getBanLifeTime() const;
  bool& enabled();

  void setPermanentWhiteList(const std::string& whiteList);

  void setPermanentBlackList(const std::string& blackList);

  ViolationCallback& threshHoldViolationCallBack();

  static bool cidr_verify(const std::string& ip, const std::string& cidr);
  static bool cidr_verify(const boost::asio::ip::address_v4& ip, const std::string& cidr);
private:
  unsigned long _packetsPerSecondThreshold;
  unsigned long _thresholdViolationRate;
  unsigned long _currentIterationCount;
  bool _autoBanThresholdViolators;
  int _banLifeTime;
  mutable boost::recursive_mutex _packetCounterMutex;
  std::map<boost::asio::ip::address, unsigned int> _packetCounter;
  std::set<boost::asio::ip::address> _whiteList;
  std::vector<std::string> _whiteListRange;
  std::vector<std::string> _blackListRange;
  std::map<boost::asio::ip::address, boost::posix_time::ptime> _blackList;
  boost::posix_time::ptime _lastTime;
  bool _enabled;
  ViolationCallback _threshHoldViolationCallBack;
};

//
// Inlines
//

inline bool& SipTransportRateLimitStrategy::enabled()
{
    return _enabled;
}

inline unsigned long SipTransportRateLimitStrategy::getPacketsPerSecondThreshold() const
{
  return _packetsPerSecondThreshold;
}

inline void SipTransportRateLimitStrategy::setPacketsPerSecondThreshold(unsigned long threshold)
{
  _packetsPerSecondThreshold = threshold;
}

inline unsigned long SipTransportRateLimitStrategy::getThresholdViolationRate() const
{
  return _thresholdViolationRate;
}

inline unsigned long SipTransportRateLimitStrategy::getCurrentIterationCount() const
{
  return _currentIterationCount;
}

inline void SipTransportRateLimitStrategy::setThresholdViolationRate(unsigned long threshold)
{
  _thresholdViolationRate = threshold;
}

inline bool& SipTransportRateLimitStrategy::autoBanThresholdViolators()
{
  return _autoBanThresholdViolators;
}

inline void SipTransportRateLimitStrategy::setBanLifeTime(int lifetime)
{
  _banLifeTime = lifetime;
}

inline int SipTransportRateLimitStrategy::getBanLifeTime() const
{
  return _banLifeTime;
}

inline SipTransportRateLimitStrategy::ViolationCallback& SipTransportRateLimitStrategy::threshHoldViolationCallBack()
{
    return _threshHoldViolationCallBack;
}



#endif	/* SipTransportRateLimitStrategy_H */


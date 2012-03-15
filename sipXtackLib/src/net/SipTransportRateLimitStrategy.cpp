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


#include "net/SipTransportRateLimitStrategy.h"


template <typename T>
T string_to_number(const char* str)
  /// Convert a string to a numeric value
{
  T value;
  std::istringstream iss(str);
  iss >> value;
  return value;
}

SipTransportRateLimitStrategy::SipTransportRateLimitStrategy() :
    _packetsPerSecondThreshold(100),
    _thresholdViolationRate(50),
    _currentIterationCount(0),
    _autoBanThresholdViolators(true),
    _banLifeTime(0),
    _enabled(false)
{
    _lastTime = boost::posix_time::microsec_clock::local_time();
}

SipTransportRateLimitStrategy::~SipTransportRateLimitStrategy()
{
}

void SipTransportRateLimitStrategy::logPacket(const boost::asio::ip::address& source, std::size_t bytesRead)
{
    if (!_enabled)
        return;

    _packetCounterMutex.lock();

    std::map<boost::asio::ip::address, unsigned int>::iterator iter = _packetCounter.find(source);
    if (iter != _packetCounter.end())
    {
        _packetCounter[source] = ++iter->second;
    }else
    {
        _packetCounter[source] = 1;
    }

    boost::posix_time::ptime now(boost::posix_time::microsec_clock::local_time());
    boost::posix_time::time_duration timeDiff = now - _lastTime;

    if (++_currentIterationCount >= _packetsPerSecondThreshold)
    {
        _currentIterationCount = 0;
        
        
        if (timeDiff.total_milliseconds() <=  1000)
        {

            //
            // We got a ratelimit violation
            //

            unsigned long watermark = 0;
            boost::asio::ip::address suspect;
            for (std::map<boost::asio::ip::address, unsigned int>::iterator iter = _packetCounter.begin();
            iter != _packetCounter.end(); iter++)
            {
                if (iter->second > watermark)
                {
                    watermark = iter->second;
                    suspect = iter->first;
                }
            }

            if (watermark >= _thresholdViolationRate && _autoBanThresholdViolators)
            {
                if (_whiteList.find(source) == _whiteList.end())
                {
                    if (_threshHoldViolationCallBack)
                    {
                        _threshHoldViolationCallBack(source, watermark);
                    }
                    _blackList[source] = now;
                }
            }
        }

        //
        // Reset
        //


        _packetCounter.clear();
        _lastTime = now;

        
    }

    //
      // Check for parole
      //
      if (_banLifeTime > 0)
      {
          std::vector<boost::asio::ip::address> parole;
          for (std::map<boost::asio::ip::address, boost::posix_time::ptime>::iterator iter = _blackList.begin();
              iter != _blackList.end(); iter++)
          {
              timeDiff = now - iter->second;
              if (timeDiff.total_milliseconds() >  _banLifeTime * 1000)
              parole.push_back(iter->first);
          }

          for (std::vector<boost::asio::ip::address>::iterator iter = parole.begin(); iter != parole.end(); iter++)
          {
              _blackList.erase(*iter);
          }
      }

    _packetCounterMutex.unlock();

}

bool SipTransportRateLimitStrategy::cidr_verify(const boost::asio::ip::address_v4& ipv4, const std::string& cidr)
{
    typedef std::vector<std::string> split_vector_type;
    split_vector_type cidr_tokens;
    boost::split( cidr_tokens, cidr, boost::is_any_of("/"));

    unsigned bits = 24;
    std::string start_ip;
    if (cidr_tokens.size() == 2)
    {
        bits = string_to_number<unsigned>(cidr_tokens[1].c_str());
        start_ip = cidr_tokens[0];
    }
    else
    {
        start_ip = cidr;
    }

    boost::system::error_code ec;
    unsigned long ipv4ul = ipv4.to_ulong();
    boost::asio::ip::address_v4 start_ip_ipv4;
    start_ip_ipv4 = boost::asio::ip::address_v4::from_string(start_ip, ec);
    if (!ec)
    {
      long double numHosts = pow((long double)2, (int)(32-bits)) - 1;
      unsigned long start_ip_ipv4ul = start_ip_ipv4.to_ulong();
      unsigned long ceiling = start_ip_ipv4ul + (unsigned long)numHosts;
      return ipv4ul >= start_ip_ipv4ul && ipv4ul <= ceiling;
    }

    return false;
}

bool SipTransportRateLimitStrategy::cidr_verify(const std::string& ip, const std::string& cidr)
{
  boost::system::error_code ec;
  boost::asio::ip::address_v4 ipv4;
  ipv4 = boost::asio::ip::address_v4::from_string(ip, ec);
  if (!ec)
      return cidr_verify(ipv4, cidr);
  return false;
}

bool SipTransportRateLimitStrategy::isWhiteListedAddressRange(const boost::asio::ip::address& source) const
{
    bool isWhiteListed = false;
    _packetCounterMutex.lock();

    for (std::vector<std::string>::const_iterator iter = _whiteListRange.begin();
        iter != _whiteListRange.end(); iter++)
    {
        if (cidr_verify(source.to_v4(), *iter))
        {
            isWhiteListed = true;
            break;
        }
    }

    _packetCounterMutex.unlock();

    return isWhiteListed;
}

bool SipTransportRateLimitStrategy::isBannedAddressRange(const boost::asio::ip::address& source) const
{
    bool isBlackListed = false;
    _packetCounterMutex.lock();

    for (std::vector<std::string>::const_iterator iter = _blackListRange.begin();
        iter != _blackListRange.end(); iter++)
    {
        if (cidr_verify(source.to_v4(), *iter))
        {
            isBlackListed = true;
            break;
        }
    }

    _packetCounterMutex.unlock();
    return isBlackListed;
}

bool SipTransportRateLimitStrategy::isBannedAddress(const boost::asio::ip::address& source) const
{
    if (!_enabled)
    {
        return false;
    }

    if (isWhiteListedAddressRange(source))
    {
        return false;
    }else if (isBannedAddressRange(source))
    {
        return true;
    }else
    {
        bool banned = false;
        _packetCounterMutex.lock();
        if (_whiteList.find(source) != _whiteList.end())
            banned = false;
        else
            banned = _blackList.find(source) != _blackList.end();
        _packetCounterMutex.unlock();
        return banned;
    }

}

void SipTransportRateLimitStrategy::banAddress(const boost::asio::ip::address& source, bool permanently)
{
    _packetCounterMutex.lock();
    if (permanently)
    {
        static std::stringstream ss("3000-Jan-01 00:00:00");
        boost::posix_time::ptime forever;
        ss >> forever;
        _blackList[source] = forever;
    }else
    {
        _blackList[source] =  boost::posix_time::ptime(boost::posix_time::microsec_clock::local_time());
    }
    _packetCounterMutex.unlock();
}

void SipTransportRateLimitStrategy::clearAddress(const boost::asio::ip::address& source, bool addToWhiteList)
{
    _packetCounterMutex.lock();
    _blackList.erase(source);
    if (addToWhiteList)
        _whiteList.insert(source);
    _packetCounterMutex.unlock();
}


void SipTransportRateLimitStrategy::setPermanentWhiteList(const std::string& whiteList)
{
    typedef std::vector<std::string> split_vector_type;
    split_vector_type splitVec; 
    boost::split( splitVec, whiteList, boost::is_any_of(", "));
    _packetCounterMutex.lock();
    for (std::vector<std::string>::iterator iter = splitVec.begin(); iter != splitVec.end(); iter++)
    {
        if (iter->find("/") == std::string::npos)
        {
            boost::asio::ip::address remoteIp = boost::asio::ip::address::from_string(iter->c_str());
            _whiteList.insert(remoteIp);
        }
        else
        {
            _whiteListRange.push_back(*iter);
        }
    }
    _packetCounterMutex.unlock();
}

void SipTransportRateLimitStrategy::setPermanentBlackList(const std::string& blackList)
{
    typedef std::vector<std::string> split_vector_type;
    split_vector_type splitVec;
    boost::split( splitVec, blackList, boost::is_any_of(", "));
    for (std::vector<std::string>::iterator iter = splitVec.begin(); iter != splitVec.end(); iter++)
    {
        if (iter->find("/") == std::string::npos)
        {
            boost::asio::ip::address remoteIp = boost::asio::ip::address::from_string(iter->c_str());
            banAddress(remoteIp, true);
        }
        else
        {
            _packetCounterMutex.lock();
            _blackListRange.push_back(*iter);
            _packetCounterMutex.unlock();
        }
    }
}












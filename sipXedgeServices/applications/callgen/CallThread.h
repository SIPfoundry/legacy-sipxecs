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

#ifndef CALLTHREAD_H_INCLUDED
#define	CALLTHREAD_H_INCLUDED


#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <string>
#include <sstream>
#include <esl/EslConnection.h>


#if defined(__LP64__)
  typedef signed long        Int64;
  typedef unsigned long      UInt64;
#else
  typedef signed long long   Int64;
  typedef unsigned long long UInt64;
#endif

class CallManager;

class CallThread
{

public:

  typedef boost::recursive_mutex mutex;
  typedef boost::lock_guard<mutex> mutex_lock;

  CallThread(CallManager& manager, int id);
  ~CallThread();

  void run();

  bool makeCall();

  void setTargetUri(const std::string& targetUri);

  void setAuthUser(const std::string& authUser);

  void setAuthPass(const std::string& authPass);

  void setFromUser(const std::string& fromUser);

  void setDomain(const std::string& domain);

  bool eslConnect();
protected:
  void internalRun();

  CallManager& _manager;
  std::string _targetUri;
  std::string _authUser;
  std::string _authPass;
  std::string _fromUser;
  std::string _domain;
  boost::posix_time::ptime _setupTime;
  boost::posix_time::ptime _disconnectTime;
  std::ostringstream _originateString;
  int _id;
  boost::thread* _pThread;
  EslConnection* _pEsl;

  friend class CallManager;
};


//
// Inlines
//

inline void CallThread::setTargetUri(const std::string& targetUri)
{
  _targetUri = targetUri;
}

inline void CallThread::setAuthUser(const std::string& authUser)
{
  _authUser = authUser;
}

inline void CallThread::setAuthPass(const std::string& authPass)
{
  _authPass = authPass;
}

inline void CallThread::setFromUser(const std::string& fromUser)
{
  _fromUser = fromUser;
}

inline void CallThread::setDomain(const std::string& domain)
{
  _domain = domain;
}

#endif	// CALLTHREAD_H_INCLUDED


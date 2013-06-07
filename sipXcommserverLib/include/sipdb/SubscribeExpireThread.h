/*
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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

#ifndef SUBSCRIBEEXPIRETHREAD_H
#define	SUBSCRIBEEXPIRETHREAD_H

#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include "sipdb/SubscribeDB.h"


class SubscribeExpireThread
{
public:
  SubscribeExpireThread() :
    _pDb(0),
    _pThread(0),
    _seconds(60),
    _pTimer(0),
    _pOldTimer(0)
  {
  }

  ~SubscribeExpireThread()
  {
    _timerService.stop();
    if (_pThread)
    {
      _pThread->join();
      delete _pThread;
      delete _pTimer;
      delete _pOldTimer;
    }
  }

  void run(SubscribeDB* pDb, int seconds = 60)
  {
    if (_pThread || _pDb || !pDb)
      return;
    _seconds = seconds;
    _pDb = pDb;
    _pThread = new boost::thread(boost::bind(&SubscribeExpireThread::internal_run, this));
  }
private:

  void internal_run()
  {
    _pTimer = new boost::asio::deadline_timer(_timerService, boost::posix_time::seconds(_seconds));
    _pTimer->async_wait(boost::bind(&SubscribeExpireThread::onTimerTick, this, boost::asio::placeholders::error));
    _timerService.run(); // <<----  This will block
  }

  void onTimerTick(const boost::system::error_code& e)
  {
    if (!e && _pDb)
    {
      try
      {
      _pDb->removeAllExpired();
      }
      catch (...)
      {
      }

      delete _pOldTimer;
      _pOldTimer = _pTimer;
      if (_pTimer)
      {
        boost::system::error_code ec;
        _pTimer->cancel(ec);
      }
      _pTimer = new boost::asio::deadline_timer(_timerService, boost::posix_time::seconds(_seconds));
      _pTimer->async_wait(boost::bind(&SubscribeExpireThread::onTimerTick, this, boost::asio::placeholders::error));
    }
  }

  SubscribeDB* _pDb;
  boost::thread* _pThread;
  boost::asio::io_service _timerService;
  int _seconds;
  boost::asio::deadline_timer* _pTimer;
  boost::asio::deadline_timer* _pOldTimer;
  
};


#endif	/* SUBSCRIBEEXPIRETHREAD_H */


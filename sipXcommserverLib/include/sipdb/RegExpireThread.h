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

#ifndef REGEXPIRETHREAD_H
#define	REGEXPIRETHREAD_H

#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include "sipdb/RegDB.h"


class RegExpireThread
{
public:
  RegExpireThread() :
    _pDb(0),
    _pThread(0),
    _seconds(60),
    _pTimer(0),
    _pOldTimer(0)
  {
  }

  ~RegExpireThread()
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

  void run(RegDB* pDb, int seconds = 60)
  {
    if (_pThread || _pDb || !pDb)
      return;
    _seconds = seconds;
    _pDb = pDb;
    _pThread = new boost::thread(boost::bind(&RegExpireThread::internal_run, this));
  }
private:

  void internal_run()
  {
    _pTimer = new boost::asio::deadline_timer(_timerService, boost::posix_time::seconds(_seconds));
    _pTimer->async_wait(boost::bind(&RegExpireThread::onTimerTick, this, boost::asio::placeholders::error));
    _timerService.run(); // <<----  This will block
  }

  void onTimerTick(const boost::system::error_code& e)
  {
    try
    {
      if (!e && _pDb)
      {
        _pDb->removeAllExpired();
        delete _pOldTimer;
        _pOldTimer = _pTimer;
        if (_pTimer)
        {
          boost::system::error_code ec;
          _pTimer->cancel(ec);
        }
        _pTimer = new boost::asio::deadline_timer(_timerService, boost::posix_time::seconds(_seconds));
        _pTimer->async_wait(boost::bind(&RegExpireThread::onTimerTick, this, boost::asio::placeholders::error));
      }
    }
    catch(...)
    {
      //
      // We will drop any mongo exception so it doesn't cause a crash when mongo is down
      //
    }
  }

  RegDB* _pDb;
  boost::thread* _pThread;
  boost::asio::io_service _timerService;
  int _seconds;
  boost::asio::deadline_timer* _pTimer;
  boost::asio::deadline_timer* _pOldTimer;

};


#endif	/* REGEXPIRETHREAD_H */


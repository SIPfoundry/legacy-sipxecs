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

#ifndef SEMAPHORE_H
#define	SEMAPHORE_H

#include <boost/thread.hpp>


class Semaphore
{
    //The current semaphore count.
    unsigned int _count;

    //_mutex protects _count.
    //Any code that reads or writes the _count data must hold a lock on
    //the mutex.
    boost::mutex _mutex;

    //Code that increments _count must notify the condition variable.
    boost::condition_variable _condition;

public:
    explicit Semaphore(unsigned int initial_count = 0)
       : _count(initial_count),
         _mutex(),
         _condition()
    {
    }


    void signal() //called "release" in Java
    {
        boost::unique_lock<boost::mutex> lock(_mutex);

        ++_count;

        //Wake up any waiting threads.
        //Always do this, even if _count wasn't 0 on entry.
        //Otherwise, we might not wake up enough waiting threads if we
        //get a number of signal() calls in a row.
        _condition.notify_one();
    }

    void wait() //called "acquire" in Java
    {
        boost::unique_lock<boost::mutex> lock(_mutex);
        while (_count == 0)
        {
             _condition.wait(lock);
        }
        --_count;
    }

    bool wait(int milliseconds)
    {
      boost::unique_lock<boost::mutex> lock(_mutex);
      boost::system_time const timeout = boost::get_system_time()+ boost::posix_time::milliseconds(milliseconds);
      while (_count == 0)
      {
        if (!_condition.timed_wait(lock,timeout))
          return false;
      }
      --_count;
      return true;
    }
};

#endif	/* SEMAPHORE_H */


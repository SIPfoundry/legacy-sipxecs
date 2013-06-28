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

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <boost/array.hpp>

#include "os/OsThreadPool.h"


void thread_sleep( unsigned long milliseconds )
{
	timeval sTimeout = { (long int)(milliseconds / 1000), (long int)(( milliseconds % 1000 ) * 1000) };
	select( 0, 0, 0, 0, &sTimeout );
}


class OsThreadPoolTest : public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE(OsThreadPoolTest);
  CPPUNIT_TEST(testThreadPool);
  CPPUNIT_TEST(testThreadPoolMaxedOut);
  CPPUNIT_TEST(testThreadPoolAddCapacity);
  CPPUNIT_TEST_SUITE_END();
  
public:
  boost::array<int, 100> _evArray;
  bool _noSleep;

  void processEvent(int ev)
  {
    _evArray[ev] = ev;

    if (!_noSleep)
      thread_sleep( ev );
  }

  void testThreadPool()
  {
    _noSleep = false;

    OsThreadPool<int> tp;
    //
    // Initialize all the members of the array to -1.
    // processEvent should set the value equal to its index
    //
    for (int i = 0; i < 100; i++)
      _evArray[i] = -1;
     
    for (int i = 0; i < 100; i++)
    {
      CPPUNIT_ASSERT(tp.schedule(boost::bind(&OsThreadPoolTest::processEvent, this, _1), i));
    }

    //
    // wait for all tasks to finish
    //
    tp.threadPool().joinAll();

    for (int i = 0; i < 100; i++)
    {
      CPPUNIT_ASSERT(_evArray[i] == i);
    }
  }

  void testThreadPoolMaxedOut()
  {
    _noSleep = true;

    //
    // Create a threadpool with 1 initial thread with maximum of two
    //
    OsThreadPool<int> tp(1,2);
    //
    // Initialize all the members of the array to -1.
    // processEvent should set the value equal to its index
    //
    for (int i = 0; i < 100; i++)
      _evArray[i] = -1;

    int failures = 0;
    for (int i = 0; i < 100; i++)
    {
      if (!tp.schedule(boost::bind(&OsThreadPoolTest::processEvent, this, _1), i))
        failures++;
    }

    //
    // wait for all tasks to finish
    //
    tp.threadPool().joinAll();

    int negatives = 0;
    for (int i = 0; i < 100; i++)
    {
      if (_evArray[i] == -1)
        negatives++;
    }

    CPPUNIT_ASSERT(failures != 0 && failures == negatives);
  }


  void testThreadPoolAddCapacity()
  {
    _noSleep = false;
    //
    // Create a threadpool with 1 initial thread with maximum of two
    //
    OsThreadPool<int> tp(1,2);
    //
    // Initialize all the members of the array to -1.
    // processEvent should set the value equal to its index
    //
    for (int i = 0; i < 100; i++)
      _evArray[i] = -1;

    int failures = 0;
    for (int i = 0; i < 100; i++)
    {
      if (!tp.schedule(boost::bind(&OsThreadPoolTest::processEvent, this, _1), i))
      {
        //
        // If it fails, grow the threadPool by 1.
        //
        failures--;
        tp.threadPool().addCapacity(1);
        CPPUNIT_ASSERT(tp.schedule(boost::bind(&OsThreadPoolTest::processEvent, this, _1), i));
      }
    }

    //
    // wait for all tasks to finish
    //
    tp.threadPool().joinAll();

    for (int i = 0; i < 100; i++)
    {
      CPPUNIT_ASSERT(_evArray[i] == i);
    }
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(OsThreadPoolTest);
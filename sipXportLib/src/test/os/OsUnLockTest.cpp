//
// Copyright (C) 2009 Nortel Networks, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <os/OsBSem.h>
#include <os/OsLock.h>
#include <os/OsTask.h>
#include <os/OsUnLock.h>


// Support functions for testUnlock.

// Global state variable.
// During the test, state sequences through 0, 1, 2, ....
// We assume that reading and writing state is atomic.
volatile int state;

// A delay in msec that will be used inside busy-wait loops.
#define SHORT_DELAY 10
// A delay in msec used in a thread to attempt to get another
// thread to progress.
#define LONG_DELAY (10 * SHORT_DELAY)

// A binary semaphore.
OsBSem sem(OsBSem::Q_PRIORITY, OsBSem::FULL);

// Test and control function.
void w(int minimum,          // state must be > minimum upon calling w().
       int wait,             // wait until state is >= wait
       int set               // then set state to this value
   )
{
   // Check that state is >= the minimum acceptable state.
   CPPUNIT_ASSERT(state >= minimum);
   // Check that state is < the value we will eventually give it.
   CPPUNIT_ASSERT(state < set);
   // Wait until state is >= the value to wait for.
   while (state < wait)
   {
      OsTask::delay(SHORT_DELAY);
   }
   // Set the new state value.
   state = set;
}

class TestThreadA : public OsTask
{
   virtual int run(void* pArg);
};

int TestThreadA::run(void* pArg)
{
   {
      OsLock lock(sem);
      // (Critical section)
      /* A1 */ w(0, 0, 1);
      // Try to get thread B to seize the lock while we are holding it.
      OsTask::delay(LONG_DELAY);
      {
         /* A2 */ w(1, 2, 3);
         OsUnLock unlock(lock);
         w(3, 4, 5);            // Wait till B3 has been passed.
         // Destroy unlock and thus re-seize the lock.
      }
      // (Critical section)
      /* A3 */ w(6, 6, 7);               // Verify that B1 has been passed.
   }
   /* A4 */ w(7, 7, 8);
   return 0;
}

TestThreadA testThreadA;

class TestThreadB : public OsTask
{
   virtual int run(void* pArg);
};

int TestThreadB::run(void* pArg)
{
   // Wait until thread A has created its OsLock.
   w(0, 1, 2);                  // Waits for A1 to finish.
   {
      // We should wait until thread A creates its OsUnLock.
      OsLock lock(sem);
      // (Critical section)
      /* B3 */ w(3, 3, 4);               // Verify that A2 has been passed.
      // Try to get thread A to re-seize the lock while we are holding it.
      OsTask::delay(LONG_DELAY);
      /* B1 */ w(5, 5, 6);
   }
   /* B2 */ w(6, 8, 9);                 // Verify that A4 has been passed.
   return 0;
}

TestThreadB testThreadB;

class OsLockTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(OsLockTest);
   CPPUNIT_TEST(testUnLock);
   CPPUNIT_TEST_SUITE_END();


public:

   void testUnLock()
      {
         // The test sets up two threads that fight over one lock.
         // Thread A will seize the lock with OsLock, and then release it
         // with an OsUnLock.  Meanwhile, once thread A has seized the lock,
         // thread B will attempt to seize the lock with OsLock.
         // The various calls of w() are used to force the threads to perform
         // these actions in the correct order, and to check that the locks
         // enforce the critical section behavior.

         // Initialize state.
         state = 0;
         // Start threads A and B.
         testThreadA.start();
         testThreadB.start();
         // Wait for the threads to finish.
         w(0, 8, 9);            // Verify that B2 has been passed.
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsLockTest);

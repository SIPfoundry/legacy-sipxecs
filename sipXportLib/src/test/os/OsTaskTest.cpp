//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <sipxunit/TestUtilities.h>
#include <os/OsTask.h>
#include <os/OsFS.h>
#include <os/OsProcess.h>

#define NUMBER_OF_ITERATIONS 100

class OsTaskTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(OsTaskTest);
   CPPUNIT_TEST(testStartNormalTask);
   CPPUNIT_TEST(testStartShortTask);
   CPPUNIT_TEST(testDeleteNormalTask);
   CPPUNIT_TEST(testDeleteShortTask);
   CPPUNIT_TEST(testReStartShortTask);

// The Runaway tests currently fail, because OsTaskLinux will assert
// if the task doesn't shutdown as requested (in order to help debug why!)

//   CPPUNIT_TEST(testStartRunawayTask); 
//   CPPUNIT_TEST(testDeleteRunawayTask);
   CPPUNIT_TEST_SUITE_END();

public:

   // OsProcess doesn't provide any thread info so this method returns
   // the number of threads running under the process given by PID.
   // FIXME: Only implemented for linux, always returns 1 otherwise.
   int getNumThreads( PID myPID )
   {
       int numThreads = 1;

#ifdef __linux__
       // /proc parsing stolen from OsProcessIteratorLinux.cpp
       OsStatus retval = OS_FAILED;
       char pidString[PID_STR_LEN];
       snprintf(pidString, PID_STR_LEN, "%ld", (long)myPID);

       OsPath fullProcName = "/proc/";
       fullProcName += pidString;
       fullProcName += "/status";
       OsFileLinux procFile(fullProcName);
       if (procFile.open(OsFile::READ_ONLY) == OS_SUCCESS)
       {
           size_t len = 5000; //since the length is always 0 for these files, lets try to read 5k
           char *buffer = new char[len+1];
           if (buffer)
           {
               size_t bytesRead;
               procFile.read((void *)buffer, len, bytesRead);

               if (bytesRead)
               {
                   procFile.close();
                   //null-terminate the string
                   buffer[bytesRead] = 0;
                   //now parse the info we need
                   char *ptr = strtok(buffer,"\n");
                   while(ptr)
                   {
                       if (memcmp(ptr,"Threads:",8) == 0)
                       {
                           numThreads = atoi(ptr+8);
                       }

                       ptr = strtok(NULL,"\n");
                   }

                   //say we are successful
                   retval = OS_SUCCESS;
               }
               else
                   osPrintf("Couldn't read bytes in readProcFile\n");

               delete [] buffer;
           }

           procFile.close();
       }
#endif
       return numThreads;
   }

   class SimpleTask : public OsTask {

   public:
   enum Commands
   {
      SIMPLE_TASK_NO_OP = 0,
      SIMPLE_TASK_RUN = 1,
      SIMPLE_TASK_SHUTDOWN = 2,
      SIMPLE_TASK_DONOT_SHUTDOWN = 3
   };

   enum State
   {
      SIMPLE_TASK_NOT_STARTED = 0,
      SIMPLE_TASK_INSIDE_RUN = 1,
      SIMPLE_TASK_OUT_RUN = 2
   };

     SimpleTask() : m_command(SimpleTask::SIMPLE_TASK_NO_OP), m_state(SimpleTask::SIMPLE_TASK_NOT_STARTED), m_numOfRuns(0) {}
   //:Default constructor

     virtual ~SimpleTask() { waitUntilShutDown(); }
   //:Destructor

   virtual int run(void* pArg) {

     OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "SimpleTask::run task %p", (void *)this );
     m_numOfRuns++;
     m_state = SIMPLE_TASK_INSIDE_RUN;
     if ( m_command == SIMPLE_TASK_SHUTDOWN ) {}
     else if ( m_command == SIMPLE_TASK_RUN ) { while (!isShuttingDown()) OsTask::delay(20); }
     else if ( m_command == SIMPLE_TASK_DONOT_SHUTDOWN ) { while (TRUE) OsTask::delay(20); }

     OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "SimpleTask::run task %p done", (void *)this );
     m_state = SIMPLE_TASK_OUT_RUN;
     return 0;
   }
   
   void setCommand(Commands command) { m_command = command; }
   State getState() { return m_state; }
   int getNumOfRuns() { return m_numOfRuns; }

   private:
   Commands m_command;
   State m_state;
   int m_numOfRuns;
   };


   void testStartNormalTask()
   {
      PID myPID = OsProcess::getCurrentPID();
      int startingThreads = getNumThreads(myPID);

      for ( int j = 0 ; j < NUMBER_OF_ITERATIONS ; j++ ) {
         SimpleTask * pTask = new SimpleTask();
         CPPUNIT_ASSERT_EQUAL(pTask->getState(), SimpleTask::SIMPLE_TASK_NOT_STARTED);
         pTask->setCommand(SimpleTask::SIMPLE_TASK_RUN);
         pTask->start();
         int i = 0;
         while (i++<100 && pTask->getState() == SimpleTask::SIMPLE_TASK_NOT_STARTED) OsTask::delay(10);
         CPPUNIT_ASSERT_EQUAL(pTask->getState(), SimpleTask::SIMPLE_TASK_INSIDE_RUN);

         CPPUNIT_ASSERT_EQUAL(startingThreads + 1,getNumThreads(myPID));

         // shutting down
         pTask->requestShutdown();
         i = 0;
         while (i++<100 && pTask->getState() == SimpleTask::SIMPLE_TASK_INSIDE_RUN) OsTask::delay(10);
         CPPUNIT_ASSERT_EQUAL(pTask->getState(), SimpleTask::SIMPLE_TASK_OUT_RUN);

         i = 0;
         while (i++<100 && getNumThreads(myPID) != startingThreads) OsTask::delay(10);
         CPPUNIT_ASSERT_EQUAL(startingThreads,getNumThreads(myPID));

         delete pTask;
      }
   }

   void testStartShortTask()
   {
      PID myPID = OsProcess::getCurrentPID();
      int startingThreads = getNumThreads(myPID);

      for ( int j = 0 ; j < NUMBER_OF_ITERATIONS ; j++ ) {
         SimpleTask * pTask = new SimpleTask();
         CPPUNIT_ASSERT_EQUAL(pTask->getState(), SimpleTask::SIMPLE_TASK_NOT_STARTED);
         pTask->setCommand(SimpleTask::SIMPLE_TASK_SHUTDOWN);
         pTask->start();

         int i = 0;
         if ( j%2 ) {
            i = 0;
            while (i++<100 && pTask->getState() == SimpleTask::SIMPLE_TASK_NOT_STARTED) OsTask::delay(10);
            i = 0;
            while (i++<100 && pTask->getState() == SimpleTask::SIMPLE_TASK_INSIDE_RUN) OsTask::delay(10);

            CPPUNIT_ASSERT_EQUAL(pTask->getState(), SimpleTask::SIMPLE_TASK_OUT_RUN);

            CPPUNIT_ASSERT_EQUAL(startingThreads,getNumThreads(myPID));

            // shutting down
            pTask->requestShutdown();

	 } else {

            // shutting down
            pTask->requestShutdown();
            i = 0;
            while (i++<100 && pTask->getState() != SimpleTask::SIMPLE_TASK_OUT_RUN) OsTask::delay(10);
	 }

         CPPUNIT_ASSERT_EQUAL(pTask->getState(), SimpleTask::SIMPLE_TASK_OUT_RUN);

         delete pTask;
         i = 0;
         while (i++<100 && getNumThreads(myPID) != startingThreads) OsTask::delay(10);
         CPPUNIT_ASSERT_EQUAL(startingThreads,getNumThreads(myPID));
      }
   }

   void testStartRunawayTask()
   {
      PID myPID = OsProcess::getCurrentPID();
      int startingThreads = getNumThreads(myPID);

      for ( int j = 0 ; j < NUMBER_OF_ITERATIONS ; j++ ) {
         SimpleTask * pTask = new SimpleTask();
         CPPUNIT_ASSERT_EQUAL(pTask->getState(), SimpleTask::SIMPLE_TASK_NOT_STARTED);
         pTask->setCommand(SimpleTask::SIMPLE_TASK_DONOT_SHUTDOWN);
         pTask->start();
         int i = 0;
         while (i++<100 && pTask->getState() == SimpleTask::SIMPLE_TASK_NOT_STARTED) OsTask::delay(10);
         CPPUNIT_ASSERT_EQUAL(pTask->getState(), SimpleTask::SIMPLE_TASK_INSIDE_RUN);

         CPPUNIT_ASSERT_EQUAL(startingThreads + 1,getNumThreads(myPID));

         // shutting down
         pTask->requestShutdown();
         i = 0;
         while (i++<100 && pTask->getState() == SimpleTask::SIMPLE_TASK_INSIDE_RUN) OsTask::delay(10);
         CPPUNIT_ASSERT_EQUAL(pTask->getState(), SimpleTask::SIMPLE_TASK_INSIDE_RUN);

         delete pTask;
         i = 0;
         while (i++<100 && getNumThreads(myPID) != startingThreads) OsTask::delay(10);
         CPPUNIT_ASSERT_EQUAL(startingThreads,getNumThreads(myPID));
      }
   }



   void testDeleteNormalTask()
   {
      PID myPID = OsProcess::getCurrentPID();
      int startingThreads = getNumThreads(myPID);

      for ( int j = 0 ; j < NUMBER_OF_ITERATIONS ; j++ ) {
         SimpleTask * pTask = new SimpleTask();
	 OsSysLog::add("main", myPID, FAC_KERNEL, PRI_DEBUG, "Created task %p in iteration %d", (void *)pTask,  j );
         pTask->setCommand(SimpleTask::SIMPLE_TASK_RUN);
         pTask->start();
	 OsSysLog::add("main", myPID, FAC_KERNEL, PRI_DEBUG, "Started task %p in iteration %d", (void *)pTask,  j );
         delete pTask;
	 OsSysLog::add("main", myPID, FAC_KERNEL, PRI_DEBUG, "Deleted task %p in iteration %d", (void *)pTask,  j );
      }
      CPPUNIT_ASSERT_EQUAL(startingThreads,getNumThreads(myPID));
   }

   void testDeleteShortTask()
   {
      PID myPID = OsProcess::getCurrentPID();
      int startingThreads = getNumThreads(myPID);

      for ( int j = 0 ; j < NUMBER_OF_ITERATIONS ; j++ ) {
         SimpleTask * pTask = new SimpleTask();
         pTask->setCommand(SimpleTask::SIMPLE_TASK_SHUTDOWN);
         pTask->start();
         delete pTask;
      }
      CPPUNIT_ASSERT_EQUAL(startingThreads,getNumThreads(myPID));
   }

   void testDeleteRunawayTask()
   {
      PID myPID = OsProcess::getCurrentPID();
      int startingThreads = getNumThreads(myPID);

      for ( int j = 0 ; j < NUMBER_OF_ITERATIONS ; j++ ) {
         SimpleTask * pTask = new SimpleTask();
	 OsSysLog::add("main", myPID, FAC_KERNEL, PRI_DEBUG, "Created task %p in iteration %d", (void *)pTask,  j );
         pTask->setCommand(SimpleTask::SIMPLE_TASK_DONOT_SHUTDOWN);
         pTask->start();
	 OsSysLog::add("main", myPID, FAC_KERNEL, PRI_DEBUG, "Started task %p in iteration %d", (void *)pTask,  j );
         delete pTask;
      }
      CPPUNIT_ASSERT_EQUAL(startingThreads,getNumThreads(myPID));
   }

   void testReStartShortTask()
   {
      PID myPID = OsProcess::getCurrentPID();
      int startingThreads = getNumThreads(myPID);
      SimpleTask * pTask = new SimpleTask();
      CPPUNIT_ASSERT_EQUAL(pTask->getState(), SimpleTask::SIMPLE_TASK_NOT_STARTED);
      pTask->setCommand(SimpleTask::SIMPLE_TASK_SHUTDOWN);

      for ( int j = 0 ; j < NUMBER_OF_ITERATIONS ; j++ ) {
         pTask->start();

         int i = 0;
         if ( j%2 ) {
            i = 0;
            while (i++<100 && pTask->getState() == SimpleTask::SIMPLE_TASK_NOT_STARTED) OsTask::delay(10);
            i = 0;
            while (i++<100 && pTask->getState() == SimpleTask::SIMPLE_TASK_INSIDE_RUN) OsTask::delay(10);

            CPPUNIT_ASSERT_EQUAL(pTask->getState(), SimpleTask::SIMPLE_TASK_OUT_RUN);


            // shutting down
            pTask->requestShutdown();

	 } else {

            // shutting down
            pTask->requestShutdown();
	 }
         i = 0;
         while (i++<100 && pTask->getState() != SimpleTask::SIMPLE_TASK_OUT_RUN) OsTask::delay(10);

         CPPUNIT_ASSERT_EQUAL(pTask->getState(), SimpleTask::SIMPLE_TASK_OUT_RUN);
      }
      delete pTask;
      int i = 0;
      while (i++<100 && getNumThreads(myPID) != startingThreads) OsTask::delay(10);
      CPPUNIT_ASSERT_EQUAL(startingThreads,getNumThreads(myPID));
   }

};

CPPUNIT_TEST_SUITE_REGISTRATION(OsTaskTest);

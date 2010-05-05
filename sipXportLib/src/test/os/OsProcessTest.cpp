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
#include <os/OsProcess.h>
#include "utl/UtlTokenizer.h"

class OsProcessTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(OsProcessTest);
#ifndef _WIN32
    CPPUNIT_TEST(testLaunch);
    CPPUNIT_TEST(testCaptureOutput);
    CPPUNIT_TEST(testSendInput);
#endif
    CPPUNIT_TEST_SUITE_END();

public:

   class SimpleTask : public OsTask
   {
   public:
      SimpleTask(OsProcess* process) : OsTask("SimpleTask"), m_process(process) {}
      virtual ~SimpleTask() { waitUntilShutDown(); }
      virtual int run(void* pArg) {
         OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "SimpleTask::run task %p", (void *)this );
         OsTask::delay(500);
         UtlString stdinMsg[3];
         stdinMsg[0] = "well\n";
         stdinMsg[1] = "hello\n";
         stdinMsg[2] = "goodbye\n";
         int rc;

         // send "well", then "hello", and expect "hello" back
         OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "calling sendInput");

         for ( int i=0; i<3; i++ )
         {
            OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "about to send input %s", stdinMsg[i].data());
            if ((rc = m_process->sendInput(stdinMsg[i])) <= 0)
            {
               OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "sendInput failed, rc=%d, errno %d (%s)", rc, errno, strerror(errno));
               CPPUNIT_ASSERT_MESSAGE("sendInput failed", rc == 0);
            }
            else
               OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "done sending input %s, rc=%d", stdinMsg[i].data(), rc);
         }
         OsTask::delay(500);
         return 0;
      }
   private:
      OsProcess* m_process;
   };

    void testLaunch()
    {
        OsStatus stat;

        UtlString appName = "ping";
        UtlString params[10];
        params[0] = "127.0.0.1";

#ifdef _WIN32  //need to do this only on win32, linux already does this by default
        params[1] = "-t";
#endif

        OsProcess process;

        UtlString envKey =   "TESTKEY1";
        UtlString envValue = "TESTVALUE1";
        process.setEnv(envKey,envValue);

        envKey =  "TESTKEY2";
        envValue ="TESTVALUE2";
        process.setEnv(envKey,envValue);

        envKey = "TESTKEY3";
        envValue = "TESTVALUE3";
        process.setEnv(envKey,envValue);

        OsPath startupDir = ".";

        //std::cout << "Launching process: " << appName.data() << std::endl;
        stat = process.launch(appName,params,startupDir);
        CPPUNIT_ASSERT_MESSAGE("Launched application", stat == OS_SUCCESS);
        CPPUNIT_ASSERT_MESSAGE("Application running", process.isRunning());

        int priority;
        process.setPriority(1);
        process.getPriority(priority);
        KNOWN_BUG("INTERMITTENT on F8 with 64Bit changes", "XECS-480");
        CPPUNIT_ASSERT_MESSAGE("Set priority ok", priority == 1);

        OsProcess newProcess;
        stat = OsProcess::getByPID(process.getPID(), newProcess);
        CPPUNIT_ASSERT_MESSAGE("Got process pid ok", stat == OS_SUCCESS);

        //std::cout << "Waiting 5 secs before killing process..." << std::endl;
        OsTask::delay(5000);
        stat = newProcess.kill();
        CPPUNIT_ASSERT_MESSAGE("Able to kill process", stat == OS_SUCCESS);
    }

    void testCaptureOutput()
    {
        OsStatus stat;

        // this command will produce a mix of stdout and stderr
        UtlString appName = "ls";
        UtlString params[10];
        params[0] = "-d";
        params[1] = "/madeUpNameDoesNotExist";
        params[2] = ".";
        params[3] = NULL;

        OsProcess process;

        OsPath startupDir = ".";

        // std::cout << "Launching process: " << appName.data() << std::endl;
        OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "launching process %s %s", appName.data(), params[0].data());
        stat = process.launch(appName, params, startupDir,
                              OsProcessBase::NormalPriorityClass, false,
                              false/* don't ignore child signals*/);
        CPPUNIT_ASSERT(stat == OS_SUCCESS);
        CPPUNIT_ASSERT(process.isRunning());

        UtlString stdoutMsg, stderrMsg;
        int rc;

        // can't guarantee in what order we'll get the output, just check for both
        bool bGotStdout = false;
        bool bGotStderr = false;

        OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "calling getOutput");
        while ( (rc = process.getOutput(&stdoutMsg, &stderrMsg)) > 0 )
        {
           if ( stdoutMsg.length() > 0 )
           {
              OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "got stdout: %s", stdoutMsg.data());
              bGotStdout = true;
           }
           if ( stderrMsg.length() > 0 )
           {
              OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "got stderr: %s", stderrMsg.data());
              bGotStderr = true;
           }
        }

        CPPUNIT_ASSERT(bGotStdout==true);
        CPPUNIT_ASSERT(bGotStderr==true);
        CPPUNIT_ASSERT(rc == 0);

        // since we forced an invalid command, we expect a non-zero return code
        rc = process.wait(0);
        CPPUNIT_ASSERT(rc != 0);
    }

    void testSendInput()
    {
        OsStatus stat;
        OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "testSendInput");

        // this command will produce a mix of stdout and stderr
        UtlString appName = "sh";
        UtlString params[10];
        params[0] = "-c";
        params[1] = "cat";
        params[2] = NULL;

        OsProcess process;

        OsPath startupDir = ".";

        // std::cout << "Launching process: " << appName.data() << std::endl;
        OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "launching process %s %s", appName.data(), params[0].data());
        stat = process.launch(appName, params, startupDir,
                              OsProcessBase::NormalPriorityClass, false,
                              false/* don't ignore child signals*/);
        CPPUNIT_ASSERT(stat == OS_SUCCESS);
        CPPUNIT_ASSERT(process.isRunning());

        UtlString stdoutMsg, stderrMsg;
        int rc;

        // send "well", then "hello", and expect "goodbye" back
        bool bGotGoodbye = false;

        OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "starting sendInput task");
        SimpleTask * pTask = new SimpleTask(&process);
        pTask->start();

        OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "calling getOutput");
        while ( !bGotGoodbye && pTask->isStarted() && (rc = process.getOutput(&stdoutMsg, &stderrMsg)) > 0 )
        {
           if ( stdoutMsg.length() > 0 )
           {
              // The output is sure to contain newlines, and may contain several lines.
              // Clean it up before dispatching.
              UtlTokenizer tokenizer(stdoutMsg);
              UtlString    msg;
              while ( tokenizer.next(msg, "\r\n") )
              {
                 OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "got stdout: %s", msg.data());
                 if ( msg == "goodbye" ) {
                    OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "got goodbye command");
                    bGotGoodbye = true;
                 }
              }

           }
           if ( stderrMsg.length() > 0 )
           {
              OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "got stderr: %s", stderrMsg.data());
           }
        }

        CPPUNIT_ASSERT(bGotGoodbye==true);
        pTask->requestShutdown();
        delete pTask;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsProcessTest);

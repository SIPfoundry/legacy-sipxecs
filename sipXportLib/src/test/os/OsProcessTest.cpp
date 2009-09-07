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

class OsProcessTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(OsProcessTest);
#ifndef _WIN32
    CPPUNIT_TEST(testLaunch);
    CPPUNIT_TEST(testCaptureOutput);
#endif
    CPPUNIT_TEST_SUITE_END();

public:

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
      //CPPUNIT_ASSERT_EQUAL_MESSAGE("Set priority ok", 1, priority);

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

        while ( (rc = process.getOutput(&stdoutMsg, &stderrMsg)) > 0 )
        {
           if ( stdoutMsg.length() > 0 ) bGotStdout = true;
           if ( stderrMsg.length() > 0 ) bGotStderr = true;
        }

        CPPUNIT_ASSERT(bGotStdout==true);
        CPPUNIT_ASSERT(bGotStderr==true);
        CPPUNIT_ASSERT(rc == 0);

        // since we forced an invalid command, we expect a non-zero return code
        rc = process.wait(0);
        CPPUNIT_ASSERT(rc != 0);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsProcessTest);

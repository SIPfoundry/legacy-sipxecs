//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <string.h>

#include "os/OsFS.h"
#include "os/OsTask.h"
#include "utl/UtlString.h"
#include "sipxunit/TestUtilities.h"
#include "testlib/FileTestContext.h"

#include "SipxProcess.h"
#include "SipxProcessManager.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

class SipxProcessStateTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipxProcessStateTest);
   CPPUNIT_TEST(stateSetting);
   CPPUNIT_TEST_SUITE_END();

public:
   UtlString mLogFile;
   
   void setUp()
   {
      OsSysLog::initialize(0, "processFsm");
      mLogFile = "processFsmTest.log";
      OsSysLog::setOutputFile(0, mLogFile);
      OsSysLog::setLoggingPriority(PRI_DEBUG);
      OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG, "processFsmTest::setUp");
   }
   
   void tearDown()
   {
//      SipxProcessManager::getInstance()->shutdown();
   }


   void stateSetting()
      {
         printf("TEST_DATA_DIR=%s, TEST_WORK_DIR=%s\n", TEST_DATA_DIR,TEST_WORK_DIR );
         FileTestContext testContext(TEST_DATA_DIR "processState",
                                     TEST_WORK_DIR "processState"
                                     );
         
         // copy test files into testContext structure
         testContext.inputFile("newprocess.xml");
         testContext.inputFile("newprocess1.sh");
         testContext.inputFile("newprocess2.sh");
         testContext.inputFile("newprocess3.sh");
         testContext.inputFile("notherprocess.sh");
         UtlString exePath;
         testContext.workingFilePath("newprocess1.sh", exePath);
         printf("working exe is %s\n", exePath.data());
         chmod(exePath.data(), S_IREAD | S_IWRITE | S_IEXEC);
         testContext.workingFilePath("newprocess2.sh", exePath);
         chmod(exePath.data(), S_IREAD | S_IWRITE | S_IEXEC);
         testContext.workingFilePath("newprocess3.sh", exePath);
         chmod(exePath.data(), S_IREAD | S_IWRITE | S_IEXEC);

         testContext.inputFile("another-process.xml");
         testContext.workingFilePath("notherprocess.sh", exePath);
         chmod(exePath.data(), S_IREAD | S_IWRITE | S_IEXEC);
         
         testContext.setSipxDir(SipXecsService::VarDirType, "var");
         testContext.setSipxDir(SipXecsService::LogDirType);

         UtlHashMap status;
         SipxProcessManager::getInstance()->getProcessStateAll(status);

         size_t existingProcesses = status.entries();

         UtlString  path;
         SipxProcess*   process1;
         SipxProcess*   process2;
         
         testContext.inputFilePath("newprocess.xml", path);
         CPPUNIT_ASSERT((process1 = SipxProcess::createFromDefinition(path)));
         OsTask::delay(500);   // give task some time to get up and running

         ASSERT_STR_EQUAL("New", process1->data());
         ASSERT_STR_EQUAL("1.0.0", process1->mVersion.data());

         CPPUNIT_ASSERT_EQUAL(SipxProcess::pDisabled->name(), process1->GetCurrentState()->name());
         CPPUNIT_ASSERT_EQUAL(SipxProcess::pDisabled->name(), process1->mpDesiredState->name());
         CPPUNIT_ASSERT(!process1->isEnabled());

         testContext.inputFilePath("another-process.xml", path);
         CPPUNIT_ASSERT((process2 = SipxProcess::createFromDefinition(path)));
         OsTask::delay(500);   // give task some time to get up and running

         ASSERT_STR_EQUAL("Nother", process2->data());
         ASSERT_STR_EQUAL("1.0.0", process2->mVersion.data());

         ASSERT_STR_EQUAL(SipxProcess::pDisabled->name(), process2->GetCurrentState()->name());
         ASSERT_STR_EQUAL(SipxProcess::pDisabled->name(), process2->mpDesiredState->name());

         process1->enable();
         OsTask::delay(2000);   // give task some time to get up and running
         CPPUNIT_ASSERT(process1->isEnabled());
         ASSERT_STR_EQUAL(SipxProcess::pRunning->name(), process1->mpDesiredState->name());
         ASSERT_STR_EQUAL(SipxProcess::pRunning->name(), process1->GetCurrentState()->name());

         process2->enable();
         OsTask::delay(2000);   // give task some time to get up and running
         ASSERT_STR_EQUAL(SipxProcess::pRunning->name(), process2->GetCurrentState()->name());
         CPPUNIT_ASSERT(process2->isEnabled());

         SipxProcessManager::getInstance()->getProcessStateAll(status);

         CPPUNIT_ASSERT_EQUAL(existingProcesses + 2U, status.entries());
         UtlString* statusValue;

         UtlString process1name("New");
         CPPUNIT_ASSERT(statusValue = dynamic_cast<UtlString*>(status.findValue(&process1name)));
         ASSERT_STR_EQUAL("Running", statusValue->data());

         UtlString process2name("Nother");
         CPPUNIT_ASSERT(statusValue = dynamic_cast<UtlString*>(status.findValue(&process2name)));
         ASSERT_STR_EQUAL("Running", statusValue->data());
         
         process2->shutdown();
         CPPUNIT_ASSERT(process2->isEnabled());
         ASSERT_STR_EQUAL(SipxProcess::pRunning->name(), process2->mpDesiredState->name());
         OsTask::delay(2000);   // give task some time to shutdown
         ASSERT_STR_EQUAL(SipxProcess::pShutDown->name(), process2->GetCurrentState()->name());
         
         // when ProcessMgr shuts down, it deletes all the processes
         delete process1;
         OsTask::delay(2000);   // give task some time to shutdown

      };
   
   
};



CPPUNIT_TEST_SUITE_REGISTRATION(SipxProcessStateTest);

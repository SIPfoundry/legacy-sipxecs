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
#include "os/OsTime.h"
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
   CPPUNIT_TEST(dependentProcess);
   CPPUNIT_TEST_SUITE_END();

public:

   void setUp()
   {
      OsSysLog::initialize(0, "processFsm");
      OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG, "processFsmTest::setUp");
   }

   void tearDown()
   {
      SipxProcessManager::getInstance()->shutdown();
   }

   void stateSetting()
   {
      FileTestContext testContext(TEST_DATA_DIR "processState",
            TEST_WORK_DIR "processState");

      // copy test files into testContext structure
      testContext.inputFile("newprocess.xml");
      testContext.inputFile("another-process.xml");
      testContext.inputFile("notherprocess.sh");

      UtlString exePath;
      testContext.workingFilePath("notherprocess.sh", exePath);
      chmod(exePath.data(), S_IREAD | S_IWRITE | S_IEXEC);

      testContext.setSipxDir(SipXecsService::VarDirType, "var");
      testContext.setSipxDir(SipXecsService::LogDirType);

      UtlHashMap status;
      SipxProcessManager::getInstance()->getProcessStateAll(status);

      size_t existingProcesses = status.entries();

      UtlString path;
      SipxProcess* process1;
      SipxProcess* process2;

      testContext.inputFilePath("newprocess.xml", path);
      CPPUNIT_ASSERT((process1 = SipxProcess::createFromDefinition(path)));
      testContext.inputFilePath("another-process.xml", path);
      CPPUNIT_ASSERT((process2 = SipxProcess::createFromDefinition(path)));
      OsTask::delay(1 * OsTime::MSECS_PER_SEC); // give task some time to get up and running

      ASSERT_STR_EQUAL("New", process1->data());
      ASSERT_STR_EQUAL("1.0.0", process1->mVersion.data());

      CPPUNIT_ASSERT_EQUAL(SipxProcess::pDisabled->name(), process1->GetCurrentState()->name());
      CPPUNIT_ASSERT_EQUAL(SipxProcess::pDisabled->name(), process1->mpDesiredState->name());
      CPPUNIT_ASSERT(!process1->isEnabled());

      ASSERT_STR_EQUAL("Nother", process2->data());
      ASSERT_STR_EQUAL("1.0.0", process2->mVersion.data());

      ASSERT_STR_EQUAL(SipxProcess::pDisabled->name(), process2->GetCurrentState()->name());
      ASSERT_STR_EQUAL(SipxProcess::pDisabled->name(), process2->mpDesiredState->name());

      // set the expected version ourselves, since no-one else can in unit test!
      UtlString newCfgVersion("1.0.0");
      process1->setConfigurationVersion(newCfgVersion);
      UtlString currCfgVersion;
      process2->getConfigurationVersion(currCfgVersion);
      OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                    "process2 config version = %s", currCfgVersion.data());

      process1->enable();
      process2->enable();

      int retries = 5;
      while ( retries-- &&
            !(process1->GetCurrentState()->name()==SipxProcess::pRunning->name() &&
              process2->GetCurrentState()->name()==SipxProcess::pConfigurationMismatch->name()) )
      {
         OsTask::delay(1 * OsTime::MSECS_PER_SEC);
      }

      OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                    "test that process2 does not start until config version is set");
      ASSERT_STR_EQUAL(SipxProcess::pConfigurationMismatch->name(), process2->GetCurrentState()->name());

      process2->setConfigurationVersion(newCfgVersion);
      retries = 5;
      while ( retries-- && strcmp(process2->GetCurrentState()->name(),SipxProcess::pRunning->name()) )
      {
         OsTask::delay(1 * OsTime::MSECS_PER_SEC);
      }

      CPPUNIT_ASSERT(process1->isEnabled());
      CPPUNIT_ASSERT(process2->isEnabled());
      ASSERT_STR_EQUAL(SipxProcess::pRunning->name(), process1->mpDesiredState->name());

      ASSERT_STR_EQUAL(SipxProcess::pRunning->name(), process1->GetCurrentState()->name());
      ASSERT_STR_EQUAL(SipxProcess::pRunning->name(), process2->GetCurrentState()->name());

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

      retries = 5;
      while ( retries-- && strcmp(process2->GetCurrentState()->name(),SipxProcess::pShutDown->name()) )
      {
         OsTask::delay(1 * OsTime::MSECS_PER_SEC); // give task some time to shutdown
      }
      ASSERT_STR_EQUAL(SipxProcess::pShutDown->name(), process2->GetCurrentState()->name());

   }


   void dependentProcess()
   {
      FileTestContext testContext(TEST_DATA_DIR "processState",
            TEST_WORK_DIR "processState");

      // copy test files into testContext structure
      testContext.inputFile("parent-process.xml");
      testContext.inputFile("dependent-process.xml");
      testContext.inputFile("notherprocess.sh");

      UtlString exePath;
      testContext.workingFilePath("notherprocess.sh", exePath);
      chmod(exePath.data(), S_IREAD | S_IWRITE | S_IEXEC);

      testContext.setSipxDir(SipXecsService::VarDirType, "var");
      testContext.setSipxDir(SipXecsService::LogDirType);

      UtlString path;
      SipxProcess* process1;
      SipxProcess* process2;

      testContext.inputFilePath("parent-process.xml", path);
      CPPUNIT_ASSERT((process1 = SipxProcess::createFromDefinition(path)));
      testContext.inputFilePath("dependent-process.xml", path);
      CPPUNIT_ASSERT((process2 = SipxProcess::createFromDefinition(path)));
      OsTask::delay(1 * OsTime::MSECS_PER_SEC); // give task some time to get up and running

      ASSERT_STR_EQUAL("Parent", process1->data());
      ASSERT_STR_EQUAL("1.0.0", process1->mVersion.data());

      CPPUNIT_ASSERT_EQUAL(SipxProcess::pDisabled->name(), process1->GetCurrentState()->name());
      CPPUNIT_ASSERT_EQUAL(SipxProcess::pDisabled->name(), process1->mpDesiredState->name());
      CPPUNIT_ASSERT(!process1->isEnabled());

      ASSERT_STR_EQUAL("Dependent", process2->data());
      ASSERT_STR_EQUAL("1.0.0", process2->mVersion.data());

      ASSERT_STR_EQUAL(SipxProcess::pDisabled->name(), process2->GetCurrentState()->name());
      ASSERT_STR_EQUAL(SipxProcess::pDisabled->name(), process2->mpDesiredState->name());

      // set the expected version ourselves, since no-one else can in unit test!
      UtlString newCfgVersion("1.0.0");
      process1->setConfigurationVersion(newCfgVersion);
      process2->setConfigurationVersion(newCfgVersion);

      OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                    "test that process2 does not start until process1 is Running");
      process2->enable();

      int retries = 5;
      while ( retries-- && strcmp(process2->GetCurrentState()->name(),SipxProcess::pResourceRequired->name()) )
      {
         OsTask::delay(1 * OsTime::MSECS_PER_SEC);
      }

      ASSERT_STR_EQUAL(SipxProcess::pResourceRequired->name(), process2->GetCurrentState()->name());
      ASSERT_STR_EQUAL(SipxProcess::pDisabled->name(), process1->GetCurrentState()->name());

      process1->enable();

      retries = 8;
      while ( retries-- &&
            !(process1->GetCurrentState()->name()==SipxProcess::pRunning->name() &&
              process2->GetCurrentState()->name()==SipxProcess::pRunning->name()) )
      {
         OsTask::delay(1 * OsTime::MSECS_PER_SEC);
      }

      ASSERT_STR_EQUAL(SipxProcess::pRunning->name(), process1->GetCurrentState()->name());
      ASSERT_STR_EQUAL(SipxProcess::pRunning->name(), process2->GetCurrentState()->name());

      /*
      OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG, "This should assert fail as it comes in the wrong thread:");
      process2->configurationVersionChangeInTask();
      */

   }
};



CPPUNIT_TEST_SUITE_REGISTRATION(SipxProcessStateTest);

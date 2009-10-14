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

#include "SipxCommand.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

class SipxCommandTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipxCommandTest);
   CPPUNIT_TEST(runCommand);
   CPPUNIT_TEST(blockSimultaneousCommand);
   CPPUNIT_TEST(killRunningCommand);
   CPPUNIT_TEST_SUITE_END();

public:

   void runCommand()
   {
      FileTestContext testContext(TEST_DATA_DIR "commandDef",
            TEST_WORK_DIR "commandDef");

      // copy test files into testContext structure
      testContext.inputFile("goodcommand.xml");
      testContext.inputFile("goodcommand.sh");
      UtlString exePath;
      testContext.workingFilePath("goodcommand.sh", exePath);
      chmod(exePath.data(), S_IREAD | S_IWRITE | S_IEXEC);

      testContext.setSipxDir(SipXecsService::VarDirType, "var");
      testContext.setSipxDir(SipXecsService::LogDirType);

      UtlString path;
      SipxCommand* command1;

      testContext.inputFilePath("goodcommand.xml", path);

      CPPUNIT_ASSERT((command1 = SipxCommand::createFromDefinition(path)));

      ASSERT_STR_EQUAL("Good", command1->data());

      UtlSList msgs;
      command1->getCommandMessages(msgs);
      CPPUNIT_ASSERT(0 == msgs.entries());

      command1->execute();
      OsTask::delay(500); // give task some time to get up and running

      CPPUNIT_ASSERT(true == command1->isRunning());

      OsTask::delay(1000); // give task some time to finish
      CPPUNIT_ASSERT(false == command1->isRunning());

      command1->getCommandMessages(msgs);

      CPPUNIT_ASSERT(3 == msgs.entries());

      ASSERT_STR_EQUAL("stdout.msg-1: goodprocess.sh" , ((UtlString*)msgs.at(0))->data());
      ASSERT_STR_EQUAL("return.code: 11" , ((UtlString*)msgs.at(2))->data());
      msgs.destroyAll();

      delete command1;
      OsTask::delay(1000); // give task some time to shutdown
   }

   void blockSimultaneousCommand()
   {
      FileTestContext testContext(TEST_DATA_DIR "commandDef",
            TEST_WORK_DIR "commandDef");

      // copy test files into testContext structure
      testContext.inputFile("goodcommand.xml");
      testContext.inputFile("goodcommand.sh");
      UtlString exePath;
      testContext.workingFilePath("goodcommand.sh", exePath);
      chmod(exePath.data(), S_IREAD | S_IWRITE | S_IEXEC);

      testContext.setSipxDir(SipXecsService::VarDirType, "var");
      testContext.setSipxDir(SipXecsService::LogDirType);

      UtlString path;
      SipxCommand* command1;

      testContext.inputFilePath("goodcommand.xml", path);

      CPPUNIT_ASSERT((command1 = SipxCommand::createFromDefinition(path)));
      ASSERT_STR_EQUAL("Good", command1->data());

      command1->execute();
      OsTask::delay(500); // give task some time to get up and running

      // another invocation should fail: command is already running
      CPPUNIT_ASSERT(false == command1->execute());

      OsTask::delay(1000); // give task some time to shutdown

      delete command1;
   }

   void killRunningCommand()
   {
      FileTestContext testContext(TEST_DATA_DIR "commandDef",
            TEST_WORK_DIR "commandDef");

      // copy test files into testContext structure
      testContext.inputFile("goodcommand.xml");
      testContext.inputFile("goodcommand.sh");
      UtlString exePath;
      testContext.workingFilePath("goodcommand.sh", exePath);
      chmod(exePath.data(), S_IREAD | S_IWRITE | S_IEXEC);

      testContext.setSipxDir(SipXecsService::VarDirType, "var");
      testContext.setSipxDir(SipXecsService::LogDirType);

      UtlString path;
      SipxCommand* command1;

      testContext.inputFilePath("goodcommand.xml", path);

      CPPUNIT_ASSERT((command1 = SipxCommand::createFromDefinition(path)));
      ASSERT_STR_EQUAL("Good", command1->data());

      command1->execute();
      OsTask::delay(500); // give task some time to get up and running

      // shut it down while it is running
      delete command1;
      OsTask::delay(1000); // give task some time to shutdown
   }
};



CPPUNIT_TEST_SUITE_REGISTRATION(SipxCommandTest);

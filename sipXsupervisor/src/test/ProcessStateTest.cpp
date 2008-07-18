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
#include "utl/UtlString.h"
#include "sipxunit/TestUtilities.h"
#include "testlib/FileTestContext.h"

#include "Process.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

class ProcessStateTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(ProcessStateTest);
   CPPUNIT_TEST(stringToState);
   CPPUNIT_TEST(stateToString);
   CPPUNIT_TEST(stateSetting);
   CPPUNIT_TEST_SUITE_END();

public:

   void stringToState()
      {
         UtlString strUndefined("Undefined");
         CPPUNIT_ASSERT_EQUAL(Process::Undefined,
                              Process::state(strUndefined));

         UtlString strDisabled("Disabled");
         CPPUNIT_ASSERT_EQUAL(Process::Disabled,
                              Process::state(strDisabled));

         UtlString strTesting("Testing");
         CPPUNIT_ASSERT_EQUAL(Process::Testing,
                              Process::state(strTesting));

         UtlString strResourceRequired("ResourceRequired");
         CPPUNIT_ASSERT_EQUAL(Process::ResourceRequired,
                              Process::state(strResourceRequired));

         UtlString strConfigurationMismatch("ConfigurationMismatch");
         CPPUNIT_ASSERT_EQUAL(Process::ConfigurationMismatch,
                              Process::state(strConfigurationMismatch));

         UtlString strConfigurationTestFailed("ConfigurationTestFailed");
         CPPUNIT_ASSERT_EQUAL(Process::ConfigurationTestFailed,
                              Process::state(strConfigurationTestFailed));

         UtlString strStarting("Starting");
         CPPUNIT_ASSERT_EQUAL(Process::Starting,
                              Process::state(strStarting));

         UtlString strRunning("Running");
         CPPUNIT_ASSERT_EQUAL(Process::Running,
                              Process::state(strRunning));

         UtlString strAwaitingReferences("AwaitingReferences");
         CPPUNIT_ASSERT_EQUAL(Process::AwaitingReferences,
                              Process::state(strAwaitingReferences));

         UtlString strStopping("Stopping");
         CPPUNIT_ASSERT_EQUAL(Process::Stopping,
                              Process::state(strStopping));

         UtlString strFailed("Failed");
         CPPUNIT_ASSERT_EQUAL(Process::Failed,
                              Process::state(strFailed));
      };

   void stateToString()
      {
         ASSERT_STR_EQUAL("Undefined",
                          Process::state(Process::Undefined));

         ASSERT_STR_EQUAL("Disabled",
                          Process::state(Process::Disabled));

         ASSERT_STR_EQUAL("Testing",
                          Process::state(Process::Testing));

         ASSERT_STR_EQUAL("ResourceRequired",
                          Process::state(Process::ResourceRequired));

         ASSERT_STR_EQUAL("ConfigurationMismatch",
                          Process::state(Process::ConfigurationMismatch));

         ASSERT_STR_EQUAL("ConfigurationTestFailed",
                          Process::state(Process::ConfigurationTestFailed));

         ASSERT_STR_EQUAL("Starting",
                          Process::state(Process::Starting));

         ASSERT_STR_EQUAL("Running",
                          Process::state(Process::Running));

         ASSERT_STR_EQUAL("AwaitingReferences",
                          Process::state(Process::AwaitingReferences));

         ASSERT_STR_EQUAL("Stopping",
                          Process::state(Process::Stopping));

         ASSERT_STR_EQUAL("Failed",
                          Process::state(Process::Failed));
      };

   void stateSetting()
      {
         FileTestContext testContext(TEST_DATA_DIR "processState",
                                     TEST_WORK_DIR "processState"
                                     );
         testContext.setSipxDir(SipXecsService::VarDirType, "var");

         UtlString  path;
         Process*   process;

         testContext.inputFilePath("newprocess.xml", path);
         CPPUNIT_ASSERT((process = Process::createFromDefinition(path)));

         ASSERT_STR_EQUAL("New", process->data());
         ASSERT_STR_EQUAL("1.0.0", process->mVersion.data());

         CPPUNIT_ASSERT_EQUAL(Process::Undefined, process->getState());
         CPPUNIT_ASSERT_EQUAL(Process::Disabled, process->mDesiredState);
         CPPUNIT_ASSERT(!process->isEnabled());

         process->enable();
         CPPUNIT_ASSERT(process->isEnabled());
         CPPUNIT_ASSERT_EQUAL(Process::Running, process->mDesiredState);
         CPPUNIT_ASSERT_EQUAL(Process::Undefined, process->getState());
      };
   
   
};



CPPUNIT_TEST_SUITE_REGISTRATION(ProcessStateTest);

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

#include "SipxProcess.h"
#include "SipxProcessManager.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

class SipxProcessStateTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipxProcessStateTest);
   CPPUNIT_TEST(stringToState);
   CPPUNIT_TEST(stateToString);
   CPPUNIT_TEST(stateSetting);
   CPPUNIT_TEST_SUITE_END();

public:

   void stringToState()
      {
         UtlString strUndefined("Undefined");
         CPPUNIT_ASSERT_EQUAL(SipxProcess::Undefined,
                              SipxProcess::state(strUndefined));

         UtlString strDisabled("Disabled");
         CPPUNIT_ASSERT_EQUAL(SipxProcess::Disabled,
                              SipxProcess::state(strDisabled));

         UtlString strTesting("Testing");
         CPPUNIT_ASSERT_EQUAL(SipxProcess::Testing,
                              SipxProcess::state(strTesting));

         UtlString strResourceRequired("ResourceRequired");
         CPPUNIT_ASSERT_EQUAL(SipxProcess::ResourceRequired,
                              SipxProcess::state(strResourceRequired));

         UtlString strConfigurationMismatch("ConfigurationMismatch");
         CPPUNIT_ASSERT_EQUAL(SipxProcess::ConfigurationMismatch,
                              SipxProcess::state(strConfigurationMismatch));

         UtlString strConfigurationTestFailed("ConfigurationTestFailed");
         CPPUNIT_ASSERT_EQUAL(SipxProcess::ConfigurationTestFailed,
                              SipxProcess::state(strConfigurationTestFailed));

         UtlString strStarting("Starting");
         CPPUNIT_ASSERT_EQUAL(SipxProcess::Starting,
                              SipxProcess::state(strStarting));

         UtlString strRunning("Running");
         CPPUNIT_ASSERT_EQUAL(SipxProcess::Running,
                              SipxProcess::state(strRunning));

         UtlString strAwaitingReferences("AwaitingReferences");
         CPPUNIT_ASSERT_EQUAL(SipxProcess::AwaitingReferences,
                              SipxProcess::state(strAwaitingReferences));

         UtlString strStopping("Stopping");
         CPPUNIT_ASSERT_EQUAL(SipxProcess::Stopping,
                              SipxProcess::state(strStopping));

         UtlString strFailed("Failed");
         CPPUNIT_ASSERT_EQUAL(SipxProcess::Failed,
                              SipxProcess::state(strFailed));
      };

   void stateToString()
      {
         ASSERT_STR_EQUAL("Undefined",
                          SipxProcess::state(SipxProcess::Undefined));

         ASSERT_STR_EQUAL("Disabled",
                          SipxProcess::state(SipxProcess::Disabled));

         ASSERT_STR_EQUAL("Testing",
                          SipxProcess::state(SipxProcess::Testing));

         ASSERT_STR_EQUAL("ResourceRequired",
                          SipxProcess::state(SipxProcess::ResourceRequired));

         ASSERT_STR_EQUAL("ConfigurationMismatch",
                          SipxProcess::state(SipxProcess::ConfigurationMismatch));

         ASSERT_STR_EQUAL("ConfigurationTestFailed",
                          SipxProcess::state(SipxProcess::ConfigurationTestFailed));

         ASSERT_STR_EQUAL("Starting",
                          SipxProcess::state(SipxProcess::Starting));

         ASSERT_STR_EQUAL("Running",
                          SipxProcess::state(SipxProcess::Running));

         ASSERT_STR_EQUAL("AwaitingReferences",
                          SipxProcess::state(SipxProcess::AwaitingReferences));

         ASSERT_STR_EQUAL("Stopping",
                          SipxProcess::state(SipxProcess::Stopping));

         ASSERT_STR_EQUAL("Failed",
                          SipxProcess::state(SipxProcess::Failed));
      };

   void stateSetting()
      {
         FileTestContext testContext(TEST_DATA_DIR "processState",
                                     TEST_WORK_DIR "processState"
                                     );
         testContext.setSipxDir(SipXecsService::VarDirType, "var");

         UtlHashMap status;
         SipxProcessManager::getInstance()->getProcessStateAll(status);

         size_t existingProcesses = status.entries();

         UtlString  path;
         SipxProcess*   process1;
         SipxProcess*   process2;
         
         testContext.inputFilePath("newprocess.xml", path);
         CPPUNIT_ASSERT((process1 = SipxProcess::createFromDefinition(path)));

         ASSERT_STR_EQUAL("New", process1->data());
         ASSERT_STR_EQUAL("1.0.0", process1->mVersion.data());

         CPPUNIT_ASSERT_EQUAL(SipxProcess::Disabled, process1->getState());
         CPPUNIT_ASSERT_EQUAL(SipxProcess::Disabled, process1->mDesiredState);
         CPPUNIT_ASSERT(!process1->isEnabled());

         testContext.inputFilePath("another-process.xml", path);
         CPPUNIT_ASSERT((process2 = SipxProcess::createFromDefinition(path)));

         ASSERT_STR_EQUAL("Nother", process2->data());
         ASSERT_STR_EQUAL("1.0.0", process2->mVersion.data());

         CPPUNIT_ASSERT_EQUAL(SipxProcess::Disabled, process2->getState());
         CPPUNIT_ASSERT_EQUAL(SipxProcess::Disabled, process2->mDesiredState);

         process1->enable();
         CPPUNIT_ASSERT(process1->isEnabled());
         CPPUNIT_ASSERT_EQUAL(SipxProcess::Running, process1->mDesiredState);
         CPPUNIT_ASSERT_EQUAL(SipxProcess::Disabled, process1->getState());

         process2->enable();
         process2->mState = SipxProcess::Running; // HACK
         CPPUNIT_ASSERT_EQUAL(SipxProcess::Running, process2->getState());
         CPPUNIT_ASSERT(process2->isEnabled());

         SipxProcessManager::getInstance()->getProcessStateAll(status);

         CPPUNIT_ASSERT_EQUAL(existingProcesses + 2U, status.entries());
         UtlString* statusValue;

         UtlString process1name("New");
         CPPUNIT_ASSERT(statusValue = dynamic_cast<UtlString*>(status.findValue(&process1name)));
         ASSERT_STR_EQUAL("Disabled", statusValue->data());

         UtlString process2name("Nother");
         CPPUNIT_ASSERT(statusValue = dynamic_cast<UtlString*>(status.findValue(&process2name)));
         ASSERT_STR_EQUAL("Running", statusValue->data());
      };
   
   
};



CPPUNIT_TEST_SUITE_REGISTRATION(SipxProcessStateTest);

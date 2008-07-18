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

class ProcessVersionTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(ProcessVersionTest);
   CPPUNIT_TEST(versionStartup);
   CPPUNIT_TEST(versionSetting);
   CPPUNIT_TEST_SUITE_END();

public:

   void versionStartup()
      {
         FileTestContext testContext(TEST_DATA_DIR "processVersions",
                                     TEST_WORK_DIR "processVersions"
                                     );
         testContext.setSipxDir(SipXecsService::VarDirType, "var");
         testContext.inputFile("var/process-cfgver/Past");

         UtlString  path;
         Process*   process;

         testContext.inputFilePath("pastprocess.xml", path);
         CPPUNIT_ASSERT((process = Process::createFromDefinition(path)));

         ASSERT_STR_EQUAL("Past", process->data());
         ASSERT_STR_EQUAL("1.0.0", process->mVersion.data());
         ASSERT_STR_EQUAL("0.9.0", process->mConfigVersion.data());
         CPPUNIT_ASSERT(!process->configurationVersionMatches());
      };
   
   void versionSetting()
      {
         FileTestContext testContext(TEST_DATA_DIR "processVersions",
                                     TEST_WORK_DIR "processVersions"
                                     );
         testContext.setSipxDir(SipXecsService::VarDirType, "var");

         UtlString  path;
         Process*   process;

         testContext.inputFilePath("someprocess.xml", path);
         CPPUNIT_ASSERT((process = Process::createFromDefinition(path)));

         ASSERT_STR_EQUAL("Some", process->data());
         ASSERT_STR_EQUAL("1.0.0", process->mVersion.data());
         ASSERT_STR_EQUAL("", process->mConfigVersion.data());
         CPPUNIT_ASSERT(!process->configurationVersionMatches());

         UtlString sameCfgVersion("1.0.0");
         process->setConfigurationVersion(sameCfgVersion);
         ASSERT_STR_EQUAL("1.0.0", process->mVersion.data());
         ASSERT_STR_EQUAL("1.0.0", process->mConfigVersion.data());
         CPPUNIT_ASSERT(process->configurationVersionMatches());

         UtlString newCfgVersion("2.1.0");
         process->setConfigurationVersion(newCfgVersion);
         ASSERT_STR_EQUAL("1.0.0", process->mVersion.data());
         ASSERT_STR_EQUAL("2.1.0", process->mConfigVersion.data());
         CPPUNIT_ASSERT(!process->configurationVersionMatches());
      };
   
   
};



CPPUNIT_TEST_SUITE_REGISTRATION(ProcessVersionTest);

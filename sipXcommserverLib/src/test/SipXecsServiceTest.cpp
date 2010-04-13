//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include "testlib/FileTestContext.h"
#include "sipXecsService/SipXecsService.h"

class SipXecsServiceTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipXecsServiceTest);

   CPPUNIT_TEST(testConfigurationPath);
   CPPUNIT_TEST(testDomainConfigurationPath);
   CPPUNIT_TEST(testLogPath);
   CPPUNIT_TEST(testRunPath);
   CPPUNIT_TEST(testTmpPath);
   CPPUNIT_TEST(testBinPath);
   CPPUNIT_TEST(testLibExecPath);
   CPPUNIT_TEST(testDatabasePath);
   CPPUNIT_TEST(testName);

   CPPUNIT_TEST_SUITE_END();


public:

   FileTestContext* mFileTestContext;

   void setUp()
      {
         mFileTestContext = new FileTestContext(TEST_DATA_DIR "/servicedata",
                                                TEST_WORK_DIR "/servicedata");
      }

   void tearDown()
      {
         delete mFileTestContext;
         unsetenv(SipXecsService::ConfigurationDirType);
         unsetenv(SipXecsService::LocalStateDirType);
         unsetenv(SipXecsService::LogDirType);
         unsetenv(SipXecsService::RunDirType);
         unsetenv(SipXecsService::TmpDirType);
         unsetenv(SipXecsService::BinDirType);
         unsetenv(SipXecsService::LibExecDirType);
         unsetenv(SipXecsService::DatabaseDirType);
      }

   void testConfigurationPath()
      {
         OsPath testConfigurationPath;

         testConfigurationPath = SipXecsService::Path(SipXecsService::ConfigurationDirType,
                                                     "test-config");

         ASSERT_STR_EQUAL(SIPX_CONFDIR "/test-config", testConfigurationPath.data());

         setenv(SipXecsService::ConfigurationDirType, "/override/cfg", true /* overwrite */);

         testConfigurationPath = SipXecsService::Path(SipXecsService::ConfigurationDirType,
                                                      "test-config");

         ASSERT_STR_EQUAL( "/override/cfg/test-config", testConfigurationPath.data());

         setenv(SipXecsService::ConfigurationDirType, "/override/cfg/", true /* overwrite */);

         testConfigurationPath = SipXecsService::Path(SipXecsService::ConfigurationDirType,
                                                      "test-config");

         ASSERT_STR_EQUAL( "/override/cfg/test-config", testConfigurationPath.data());
      }

   void testDomainConfigurationPath()
      {
         OsPath testDomainConfigurationPath;

         testDomainConfigurationPath = SipXecsService::domainConfigPath();

         ASSERT_STR_EQUAL(SIPX_CONFDIR "/domain-config", testDomainConfigurationPath.data());

         setenv(SipXecsService::ConfigurationDirType, "/override/cfg", true /* overwrite */);

         testDomainConfigurationPath = SipXecsService::domainConfigPath();

         ASSERT_STR_EQUAL( "/override/cfg/domain-config", testDomainConfigurationPath.data());
      }


   void testLogPath()
      {
         OsPath testLogPath;

         testLogPath = SipXecsService::Path(SipXecsService::LogDirType,
                                                     "test.log");

         ASSERT_STR_EQUAL(SIPX_LOGDIR "/test.log", testLogPath.data());

         setenv(SipXecsService::LogDirType, "/override/log", true /* overwrite */);

         testLogPath = SipXecsService::Path(SipXecsService::LogDirType,
                                                     "test.log");

         ASSERT_STR_EQUAL( "/override/log/test.log", testLogPath.data());
      }

   void testRunPath()
      {
         OsPath testRunPath;

         testRunPath = SipXecsService::Path(SipXecsService::RunDirType,
                                                     "test.pid");

         ASSERT_STR_EQUAL(SIPX_RUNDIR "/test.pid", testRunPath.data());

         setenv(SipXecsService::RunDirType, "/override/run", true /* overwrite */);

         testRunPath= SipXecsService::Path(SipXecsService::RunDirType,
                                                     "test.pid");

         ASSERT_STR_EQUAL( "/override/run/test.pid", testRunPath.data());
      }

   void testTmpPath()
      {
         OsPath testTmpPath;

         testTmpPath= SipXecsService::Path(SipXecsService::TmpDirType,
                                                     "test.tmp");

         ASSERT_STR_EQUAL(SIPX_TMPDIR "/test.tmp", testTmpPath.data());

         setenv(SipXecsService::TmpDirType, "/override/tmp", true /* overwrite */);

         testTmpPath= SipXecsService::Path(SipXecsService::TmpDirType,
                                                     "test.tmp");

         ASSERT_STR_EQUAL( "/override/tmp/test.tmp", testTmpPath.data());
      }

   void testBinPath()
      {
         OsPath testBinPath;

         testBinPath= SipXecsService::Path(SipXecsService::BinDirType,
                                                     "test.bin");

         ASSERT_STR_EQUAL(SIPX_BINDIR "/test.bin", testBinPath.data());

         setenv(SipXecsService::BinDirType, "/override/bin", true /* overwrite */);

         testBinPath= SipXecsService::Path(SipXecsService::BinDirType,
                                                     "test.bin");

         ASSERT_STR_EQUAL( "/override/bin/test.bin", testBinPath.data());
      }

   void testLibExecPath()
      {
         OsPath testLibExecPath;

         testLibExecPath= SipXecsService::Path(SipXecsService::LibExecDirType,
                                                     "test.exe");

         ASSERT_STR_EQUAL(SIPX_LIBEXECDIR "/test.exe", testLibExecPath.data());

         setenv(SipXecsService::LibExecDirType, "/override/libexec", true /* overwrite */);

         testLibExecPath= SipXecsService::Path(SipXecsService::LibExecDirType,
                                                     "test.exe");

         ASSERT_STR_EQUAL( "/override/libexec/test.exe", testLibExecPath.data());
      }

   void testDatabasePath()
      {
         OsPath testDatabasePath;

         testDatabasePath= SipXecsService::Path(SipXecsService::DatabaseDirType,
                                                     "test.db");

         ASSERT_STR_EQUAL(SIPX_DBDIR "/test.db", testDatabasePath.data());

         setenv(SipXecsService::DatabaseDirType, "/override/data", true /* overwrite */);

         testDatabasePath= SipXecsService::Path(SipXecsService::DatabaseDirType,
                                                     "test.db");

         ASSERT_STR_EQUAL( "/override/data/test.db", testDatabasePath.data());
      }

   void testName()
   {
      ASSERT_STR_EQUAL(SIPXECS_NAME, SipXecsService::Name());

      setenv(SipXecsService::NameType, "newSipXecsName", true /* overwrite */);

      ASSERT_STR_EQUAL("newSipXecsName", SipXecsService::Name());
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SipXecsServiceTest);

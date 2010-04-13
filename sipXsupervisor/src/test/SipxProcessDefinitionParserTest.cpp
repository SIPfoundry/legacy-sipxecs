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

#include "utl/UtlString.h"
#include "os/OsFS.h"
#include "xmlparser/tinyxml.h"

#include "sipxunit/TestUtilities.h"
#include "testlib/FileTestContext.h"

#include "sipXecsService/SipXecsService.h"

#include "SipxProcess.h"
#include "SipxProcessCmd.h"
#include "SipxProcessManager.h"
#include "FileResource.h"
#include "FileResourceManager.h"
#include "DirectoryResource.h"
#include "DirectoryResourceManager.h"
#include "ImdbResource.h"
#include "ImdbResourceManager.h"
#include "SqldbResource.h"
#include "SqldbResourceManager.h"
#include "SipxProcessResource.h"
#include "SipxProcessResourceManager.h"

class SipxProcessDefinitionParserTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipxProcessDefinitionParserTest);
   CPPUNIT_TEST(badXml);
   CPPUNIT_TEST(badNames);
   CPPUNIT_TEST(badVersions);
   CPPUNIT_TEST(badCommands);
   CPPUNIT_TEST(badStatus);
   CPPUNIT_TEST(badResources);
   CPPUNIT_TEST(badPattern);
   CPPUNIT_TEST(goodProcess);
   CPPUNIT_TEST(duplicateProcess);
   CPPUNIT_TEST_SUITE_END();

public:

   void setUp()
   {
      OsSysLog::initialize(0, "processDefn");
      OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG, "processDefnTest::setUp");
   }

   void badXml()
      {
         FileTestContext testContext(TEST_DATA_DIR "processDef",
                                     TEST_WORK_DIR "processDef"
                                     );
         UtlString badXmlPath;
         SipxProcess* process;

         testContext.inputFilePath("notwellformed.xml", badXmlPath);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(badXmlPath)));

         testContext.inputFilePath("badNamespace.xml", badXmlPath);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(badXmlPath)));

         testContext.inputFilePath("badRoot.xml", badXmlPath);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(badXmlPath)));
      };

   void badNames()
      {
         FileTestContext testContext(TEST_DATA_DIR "processDef",
                                     TEST_WORK_DIR "processDef"
                                     );
         UtlString path;
         SipxProcess* process;

         testContext.inputFilePath("noname.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));

         testContext.inputFilePath("emptyname.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));

         testContext.inputFilePath("namenottext.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));
      };

   void badVersions()
      {
         FileTestContext testContext(TEST_DATA_DIR "processDef",
                                     TEST_WORK_DIR "processDef"
                                     );
         UtlString path;
         SipxProcess* process;

         testContext.inputFilePath("noversion.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));

         testContext.inputFilePath("emptyversion.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));

         testContext.inputFilePath("versionnottext.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));
      };

   void badCommands()
      {
         FileTestContext testContext(TEST_DATA_DIR "processDef",
                                     TEST_WORK_DIR "processDef"
                                     );
         UtlString path;
         SipxProcess* process;

         // copy test files into testContext structure
         testContext.inputFile("nocommands.xml");
         testContext.inputFile("noconfigtest.xml");

         testContext.workingFilePath("nocommands.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));

         testContext.inputFilePath("emptycommands.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));

         testContext.inputFilePath("noconfigtest.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));

         testContext.inputFilePath("nostart.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));

         testContext.inputFilePath("noexecute.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));

         testContext.inputFilePath("emptyexecute.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));

         testContext.inputFilePath("badexecute.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));

         testContext.inputFilePath("notaparameter.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));

      };

   void badStatus()
      {
         FileTestContext testContext(TEST_DATA_DIR "processDef",
                                     TEST_WORK_DIR "processDef"
                                     );
         UtlString path;
         SipxProcess* process;

         testContext.inputFilePath("nostatus.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));

         testContext.inputFilePath("unknownstatus1.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));

         testContext.inputFilePath("emptypid.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));

         testContext.inputFilePath("badpid.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));

         testContext.inputFilePath("unknownstatus2.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));

         testContext.inputFilePath("emptylog.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));

         testContext.inputFilePath("badlog.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));

         testContext.inputFilePath("notalog.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));
      };

   void badResources()
      {
         FileTestContext testContext(TEST_DATA_DIR "processDef",
                                     TEST_WORK_DIR "processDef"
                                     );
         UtlString path;
         SipxProcess* process;

         testContext.inputFilePath("notresources.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));

         testContext.inputFilePath("unknownresource.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));

         testContext.inputFilePath("emptyfile.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));

         testContext.inputFilePath("badfile.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));

         testContext.inputFilePath("badfileattr.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));

         testContext.inputFilePath("badconfigaccess.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));

         testContext.inputFilePath("badrequired.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));

      };

   void badPattern()
      {
         FileTestContext testContext(TEST_DATA_DIR "processDef",
                                     TEST_WORK_DIR "processDef"
                                     );
         UtlString path;
         SipxProcess* process;

         testContext.inputFilePath("baddiraccess.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));

         testContext.inputFilePath("badpatterns.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));

         testContext.inputFilePath("nopath.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));

         testContext.inputFilePath("nopattern.xml", path);
         CPPUNIT_ASSERT(NULL == (process = SipxProcess::createFromDefinition(path)));
      };

   void goodProcess()
      {
         FileTestContext testContext(TEST_DATA_DIR "processDef",
                                     TEST_WORK_DIR "processDef"
                                     );
         testContext.setSipxDir(SipXecsService::VarDirType, "var");

         UtlString  path;
         SipxProcess*   process;
         UtlString* paramValue;
         UtlString  description;

         const char* defaultUser = SipXecsService::User();
         const char* defaultDir  = SipXecsService::Path(SipXecsService::LogDirType,"");

         /*
          * If you change any of the expectations in this test, you will need to
          * update the duplicateProcess test below.  That test retrieves the
          * defintion that this one creates (having verified that another file
          * that defines a process with the same name has failed to create a
          * new SipxProcess).
          */

         testContext.inputFilePath("goodprocess.xml", path);
         CPPUNIT_ASSERT((process = SipxProcess::createFromDefinition(path)));
         CPPUNIT_ASSERT_EQUAL(process, SipxProcessManager::getInstance()->findProcess("Good"));

         ASSERT_STR_EQUAL("Good", process->data());
         ASSERT_STR_EQUAL("1.0.0", process->mVersion.data());

         SipxProcessResource* processResource;
         CPPUNIT_ASSERT(processResource = SipxProcessResourceManager::getInstance()->find("Good"));
         CPPUNIT_ASSERT_EQUAL(processResource, process->resource());
         CPPUNIT_ASSERT( ! processResource->isWriteable());
         description.remove(0);
         processResource->appendDescription(description);
         ASSERT_STR_EQUAL("process 'Good'",description.data());

         ASSERT_STR_EQUAL("/bin/goodprocess1.sh", process->mConfigtest->mExecutable.data());
         ASSERT_STR_EQUAL("/var/log/goodprocess", process->mConfigtest->mWorkingDirectory.data());
         ASSERT_STR_EQUAL(defaultUser, process->mConfigtest->mUser.data());
         CPPUNIT_ASSERT(1 == process->mConfigtest->mParameters.entries());
         CPPUNIT_ASSERT(paramValue =
                        dynamic_cast<UtlString*>(process->mConfigtest->mParameters.first()));
         ASSERT_STR_EQUAL("--configtest",paramValue->data());

         ASSERT_STR_EQUAL("/bin/goodprocess2.sh", process->mStart->mExecutable.data());
         ASSERT_STR_EQUAL(defaultDir, process->mStart->mWorkingDirectory.data());
         ASSERT_STR_EQUAL("gooduser", process->mStart->mUser.data());
         CPPUNIT_ASSERT(process->mStart->mParameters.isEmpty());

         ASSERT_STR_EQUAL("/bin/goodprocess3.sh", process->mStop->mExecutable.data());
         ASSERT_STR_EQUAL(defaultDir, process->mStop->mWorkingDirectory.data());
         ASSERT_STR_EQUAL(defaultUser, process->mStop->mUser.data());
         CPPUNIT_ASSERT(2 == process->mStop->mParameters.entries());
         CPPUNIT_ASSERT(paramValue =
                        dynamic_cast<UtlString*>(process->mStop->mParameters.first()));
         ASSERT_STR_EQUAL("--stop",paramValue->data());
         CPPUNIT_ASSERT(paramValue =
                        dynamic_cast<UtlString*>(process->mStop->mParameters.last()));
         ASSERT_STR_EQUAL("--really",paramValue->data());

         FileResource* fileResource;
         CPPUNIT_ASSERT((fileResource =
                         FileResourceManager::getInstance()->find("/var/log/goodprocess.log")));
         CPPUNIT_ASSERT( ! fileResource->isWriteable());
         description.remove(0);
         fileResource->appendDescription(description);
         ASSERT_STR_EQUAL("file '/var/log/goodprocess.log'",description.data());
         CPPUNIT_ASSERT(!process->mRequiredResources.containsReference(fileResource));
         CPPUNIT_ASSERT(fileResource->mUsedBy.containsReference(processResource));

         CPPUNIT_ASSERT((fileResource =
                         FileResourceManager::getInstance()->find("/etc/sipxpbx/goodprocess.xml")));
         CPPUNIT_ASSERT( fileResource->isWriteable());
         description.remove(0);
         fileResource->appendDescription(description);
         ASSERT_STR_EQUAL("file '/etc/sipxpbx/goodprocess.xml'",description.data());
         CPPUNIT_ASSERT(process->mRequiredResources.containsReference(fileResource));
         CPPUNIT_ASSERT(fileResource->mUsedBy.containsReference(processResource));

         CPPUNIT_ASSERT((fileResource =
                         FileResourceManager::getInstance()->find("/etc/sipxpbx/goodprocess-config")));
         CPPUNIT_ASSERT( fileResource->isWriteable());
         description.remove(0);
         fileResource->appendDescription(description);
         ASSERT_STR_EQUAL("file '/etc/sipxpbx/goodprocess-config'",description.data());
         CPPUNIT_ASSERT(fileResource->mUsedBy.containsReference(processResource));

         ImdbResource* imdbResource;
         CPPUNIT_ASSERT((imdbResource =
                         ImdbResourceManager::getInstance()->find("goodtable")));
         CPPUNIT_ASSERT( ! imdbResource->isWriteable());
         description.remove(0);
         imdbResource->appendDescription(description);
         ASSERT_STR_EQUAL("imdb 'goodtable'",description.data());
         CPPUNIT_ASSERT(imdbResource->mUsedBy.containsReference(processResource));

         SqldbResource* sqldbResource;
         CPPUNIT_ASSERT((sqldbResource =
                         SqldbResourceManager::getInstance()->find("GOODdbserverdbuser")));
         CPPUNIT_ASSERT( sqldbResource->isWriteable());
         description.remove(0);
         sqldbResource->appendDescription(description);
         ASSERT_STR_EQUAL("SQL database 'GOODdbserverdbuser'",description.data());
         CPPUNIT_ASSERT(sqldbResource->mUsedBy.containsReference(processResource));

         DirectoryResource* directoryResource;
         CPPUNIT_ASSERT((directoryResource =
                         DirectoryResourceManager::getInstance()->find("/etc/sipxpbx/goodprocess", "")));
         CPPUNIT_ASSERT( directoryResource->isWriteable());
         description.remove(0);
         directoryResource->appendDescription(description);
         ASSERT_STR_EQUAL("directory '/etc/sipxpbx/goodprocess'",description.data());
         CPPUNIT_ASSERT(directoryResource->mUsedBy.containsReference(processResource));

         FileResource* fileInDirResource;
         CPPUNIT_ASSERT((fileInDirResource =
                         FileResourceManager::getInstance()->find("/etc/sipxpbx/goodprocess/XMLFILE.xml")));
         CPPUNIT_ASSERT( fileInDirResource->isWriteable());
         CPPUNIT_ASSERT( fileInDirResource->isReadable());
         description.remove(0);
         fileInDirResource->appendDescription(description);
         ASSERT_STR_EQUAL("directory '/etc/sipxpbx/goodprocess' pattern '*.xml'",description.data());
         CPPUNIT_ASSERT(fileInDirResource->mUsedBy.containsReference(processResource));

         CPPUNIT_ASSERT((fileResource =
                         FileResourceManager::getInstance()->find("/etc/sipxpbx/goodprocess/secrets/foo.readable")));
         CPPUNIT_ASSERT( ! fileResource->isWriteable());
         CPPUNIT_ASSERT( fileResource->isReadable());
         description.remove(0);
         fileResource->appendDescription(description);
         ASSERT_STR_EQUAL("directory '/etc/sipxpbx/goodprocess/secrets' pattern '*.readable'",description.data());
         CPPUNIT_ASSERT(! process->mRequiredResources.containsReference(fileResource));
         CPPUNIT_ASSERT(fileResource->mUsedBy.containsReference(processResource));

         CPPUNIT_ASSERT((fileResource =
                         FileResourceManager::getInstance()->find("/etc/sipxpbx/goodprocess/secrets/foo.writable")));
         CPPUNIT_ASSERT( fileResource->isWriteable());
         CPPUNIT_ASSERT( ! fileResource->isReadable());
         description.remove(0);
         fileResource->appendDescription(description);
         ASSERT_STR_EQUAL("directory '/etc/sipxpbx/goodprocess/secrets' pattern '*.writable'",description.data());
         CPPUNIT_ASSERT(! process->mRequiredResources.containsReference(fileResource));
         CPPUNIT_ASSERT(fileResource->mUsedBy.containsReference(processResource));
      };

   void duplicateProcess()
      {
         FileTestContext testContext(TEST_DATA_DIR "processDef",
                                     TEST_WORK_DIR "processDef"
                                     );
         testContext.setSipxDir(SipXecsService::VarDirType, "var");

         UtlString  path;
         SipxProcess*   process;
         UtlString* paramValue;
         UtlString  description;

         const char* defaultUser = SipXecsService::User();
         const char* defaultDir  = SipXecsService::Path(SipXecsService::LogDirType,"");

         testContext.inputFilePath("dupprocess.xml", path);
         CPPUNIT_ASSERT(!(process = SipxProcess::createFromDefinition(path)));
         CPPUNIT_ASSERT((process = SipxProcessManager::getInstance()->findProcess("Good")));

         /*
          * All of the results below should match those in goodProcess above,
          * since it is the SipxProcess object created by that test that we should
          * have gotten back.
          */

         ASSERT_STR_EQUAL("Good", process->data());
         ASSERT_STR_EQUAL("1.0.0", process->mVersion.data());

         SipxProcessResource* processResource;
         CPPUNIT_ASSERT(processResource = SipxProcessResourceManager::getInstance()->find("Good"));
         CPPUNIT_ASSERT_EQUAL(processResource, process->resource());
         CPPUNIT_ASSERT( ! processResource->isWriteable());
         description.remove(0);
         processResource->appendDescription(description);
         ASSERT_STR_EQUAL("process 'Good'",description.data());

         ASSERT_STR_EQUAL("/bin/goodprocess1.sh", process->mConfigtest->mExecutable.data());
         ASSERT_STR_EQUAL("/var/log/goodprocess", process->mConfigtest->mWorkingDirectory.data());
         ASSERT_STR_EQUAL(defaultUser, process->mConfigtest->mUser.data());
         CPPUNIT_ASSERT(1 == process->mConfigtest->mParameters.entries());
         CPPUNIT_ASSERT(paramValue =
                        dynamic_cast<UtlString*>(process->mConfigtest->mParameters.first()));
         ASSERT_STR_EQUAL("--configtest",paramValue->data());

         ASSERT_STR_EQUAL("/bin/goodprocess2.sh", process->mStart->mExecutable.data());
         ASSERT_STR_EQUAL(defaultDir, process->mStart->mWorkingDirectory.data());
         ASSERT_STR_EQUAL("gooduser", process->mStart->mUser.data());
         CPPUNIT_ASSERT(process->mStart->mParameters.isEmpty());

         ASSERT_STR_EQUAL("/bin/goodprocess3.sh", process->mStop->mExecutable.data());
         ASSERT_STR_EQUAL(defaultDir, process->mStop->mWorkingDirectory.data());
         ASSERT_STR_EQUAL(defaultUser, process->mStop->mUser.data());
         CPPUNIT_ASSERT(2 == process->mStop->mParameters.entries());
         CPPUNIT_ASSERT(paramValue =
                        dynamic_cast<UtlString*>(process->mStop->mParameters.first()));
         ASSERT_STR_EQUAL("--stop",paramValue->data());
         CPPUNIT_ASSERT(paramValue =
                        dynamic_cast<UtlString*>(process->mStop->mParameters.last()));
         ASSERT_STR_EQUAL("--really",paramValue->data());

         FileResource* fileResource;
         CPPUNIT_ASSERT((fileResource =
                         FileResourceManager::getInstance()->find("/var/log/goodprocess.log")));
         CPPUNIT_ASSERT( ! fileResource->isWriteable());
         description.remove(0);
         fileResource->appendDescription(description);
         ASSERT_STR_EQUAL("file '/var/log/goodprocess.log'",description.data());
         CPPUNIT_ASSERT(!process->mRequiredResources.containsReference(fileResource));
         CPPUNIT_ASSERT(fileResource->mUsedBy.containsReference(processResource));

         CPPUNIT_ASSERT((fileResource =
                         FileResourceManager::getInstance()->find("/etc/sipxpbx/goodprocess.xml")));
         CPPUNIT_ASSERT( fileResource->isWriteable());
         description.remove(0);
         fileResource->appendDescription(description);
         ASSERT_STR_EQUAL("file '/etc/sipxpbx/goodprocess.xml'",description.data());
         CPPUNIT_ASSERT(process->mRequiredResources.containsReference(fileResource));
         CPPUNIT_ASSERT(fileResource->mUsedBy.containsReference(processResource));

         CPPUNIT_ASSERT((fileResource =
                         FileResourceManager::getInstance()->find("/etc/sipxpbx/goodprocess-config")));
         CPPUNIT_ASSERT( fileResource->isWriteable());
         description.remove(0);
         fileResource->appendDescription(description);
         ASSERT_STR_EQUAL("file '/etc/sipxpbx/goodprocess-config'",description.data());
         CPPUNIT_ASSERT(fileResource->mUsedBy.containsReference(processResource));

         ImdbResource* imdbResource;
         CPPUNIT_ASSERT((imdbResource =
                         ImdbResourceManager::getInstance()->find("goodtable")));
         CPPUNIT_ASSERT( ! imdbResource->isWriteable());
         description.remove(0);
         imdbResource->appendDescription(description);
         ASSERT_STR_EQUAL("imdb 'goodtable'",description.data());
         CPPUNIT_ASSERT(imdbResource->mUsedBy.containsReference(processResource));

         SqldbResource* sqldbResource;
         CPPUNIT_ASSERT((sqldbResource =
                         SqldbResourceManager::getInstance()->find("GOODdbserverdbuser")));
         CPPUNIT_ASSERT( sqldbResource->isWriteable());
         description.remove(0);
         sqldbResource->appendDescription(description);
         ASSERT_STR_EQUAL("SQL database 'GOODdbserverdbuser'",description.data());
         CPPUNIT_ASSERT(sqldbResource->mUsedBy.containsReference(processResource));
      };

};



CPPUNIT_TEST_SUITE_REGISTRATION(SipxProcessDefinitionParserTest);

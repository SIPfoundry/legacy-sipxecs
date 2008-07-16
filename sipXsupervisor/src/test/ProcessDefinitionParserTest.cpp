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

#include "Process.h"
#include "ProcessCmd.h"
#include "ProcessManager.h"
#include "FileResource.h"
#include "FileResourceManager.h"
#include "ImdbResource.h"
#include "ImdbResourceManager.h"
#include "SqldbResource.h"
#include "SqldbResourceManager.h"
#include "ProcessResource.h"
#include "ProcessResourceManager.h"

class ProcessDefinitionParserTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(ProcessDefinitionParserTest);
   CPPUNIT_TEST(badXml);
   CPPUNIT_TEST(badNames);
   CPPUNIT_TEST(badVersions);
   CPPUNIT_TEST(badCommands);
   CPPUNIT_TEST(badStatus);
   CPPUNIT_TEST(badResources);
   CPPUNIT_TEST(goodProcess);
   CPPUNIT_TEST_SUITE_END();

public:

   void badXml()
      {
         FileTestContext testContext(TEST_DATA_DIR "processDef",
                                     TEST_WORK_DIR "processDef"
                                     );
         UtlString badXmlPath;
         Process* process;

         testContext.inputFilePath("notwellformed.xml", badXmlPath);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(badXmlPath)));

         testContext.inputFilePath("badNamespace.xml", badXmlPath);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(badXmlPath)));

         testContext.inputFilePath("badRoot.xml", badXmlPath);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(badXmlPath)));
      };

   void badNames()
      {
         FileTestContext testContext(TEST_DATA_DIR "processDef",
                                     TEST_WORK_DIR "processDef"
                                     );
         UtlString path;
         Process* process;

         testContext.inputFilePath("noname.xml", path);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(path)));

         testContext.inputFilePath("emptyname.xml", path);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(path)));

         testContext.inputFilePath("namenottext.xml", path);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(path)));
      };

   void badVersions()
      {
         FileTestContext testContext(TEST_DATA_DIR "processDef",
                                     TEST_WORK_DIR "processDef"
                                     );
         UtlString path;
         Process* process;

         testContext.inputFilePath("noversion.xml", path);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(path)));

         testContext.inputFilePath("emptyversion.xml", path);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(path)));

         testContext.inputFilePath("versionnottext.xml", path);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(path)));
      };

   void badCommands()
      {
         FileTestContext testContext(TEST_DATA_DIR "processDef",
                                     TEST_WORK_DIR "processDef"
                                     );
         UtlString path;
         Process* process;

         testContext.inputFilePath("nocommands.xml", path);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(path)));

         testContext.inputFilePath("emptycommands.xml", path);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(path)));

         testContext.inputFilePath("noconfigtest.xml", path);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(path)));

         testContext.inputFilePath("nostart.xml", path);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(path)));

         testContext.inputFilePath("noexecute.xml", path);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(path)));

         testContext.inputFilePath("emptyexecute.xml", path);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(path)));

         testContext.inputFilePath("badexecute.xml", path);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(path)));

         testContext.inputFilePath("notaparameter.xml", path);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(path)));

      };

   void badStatus()
      {
         FileTestContext testContext(TEST_DATA_DIR "processDef",
                                     TEST_WORK_DIR "processDef"
                                     );
         UtlString path;
         Process* process;

         testContext.inputFilePath("nostatus.xml", path);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(path)));

         testContext.inputFilePath("unknownstatus1.xml", path);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(path)));

         testContext.inputFilePath("emptypid.xml", path);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(path)));

         testContext.inputFilePath("badpid.xml", path);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(path)));

         testContext.inputFilePath("unknownstatus2.xml", path);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(path)));

         testContext.inputFilePath("emptylog.xml", path);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(path)));

         testContext.inputFilePath("badlog.xml", path);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(path)));

         testContext.inputFilePath("notalog.xml", path);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(path)));
      };

   void badResources()
      {
         FileTestContext testContext(TEST_DATA_DIR "processDef",
                                     TEST_WORK_DIR "processDef"
                                     );
         UtlString path;
         Process* process;

         testContext.inputFilePath("notresources.xml", path);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(path)));

         testContext.inputFilePath("unknownresource.xml", path);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(path)));

         testContext.inputFilePath("emptyfile.xml", path);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(path)));

         testContext.inputFilePath("badfile.xml", path);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(path)));

         testContext.inputFilePath("badfileattr.xml", path);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(path)));

         testContext.inputFilePath("badconfigaccess.xml", path);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(path)));

         testContext.inputFilePath("badrequired.xml", path);
         CPPUNIT_ASSERT(NULL == (process = Process::createFromDefinition(path)));

      };

   void goodProcess()
      {
         FileTestContext testContext(TEST_DATA_DIR "processDef",
                                     TEST_WORK_DIR "processDef"
                                     );
         UtlString  path;
         Process*   process;
         UtlString* paramValue;
         UtlString  description;

         const char* defaultUser = SipXecsService::User();
         const char* defaultDir  = SipXecsService::Path(SipXecsService::LogDirType,"");

         testContext.inputFilePath("goodprocess.xml", path);
         CPPUNIT_ASSERT((process = Process::createFromDefinition(path)));
         /*
          * Cannot check ProcessManager::findProcess("Good") because
          * the Process is only saved in ProcessManager::instantiateProcesses,
          * which this test didn't use.
          */

         ASSERT_STR_EQUAL("Good", process->data());
         ASSERT_STR_EQUAL("1.0.0", process->mVersion.data());

         ProcessResource* processResource;
         CPPUNIT_ASSERT(processResource = ProcessResourceManager::getInstance()->find("Good"));
         CPPUNIT_ASSERT(processResource == process->resource());
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
                         SqldbResourceManager::getInstance()->find("GOOD")));
         CPPUNIT_ASSERT( sqldbResource->isWriteable());
         description.remove(0);
         sqldbResource->appendDescription(description);
         ASSERT_STR_EQUAL("SQL database 'GOOD'",description.data());
         CPPUNIT_ASSERT(sqldbResource->mUsedBy.containsReference(processResource));
      };
   
};



CPPUNIT_TEST_SUITE_REGISTRATION(ProcessDefinitionParserTest);

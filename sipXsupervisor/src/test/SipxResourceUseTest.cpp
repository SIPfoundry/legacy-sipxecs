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
#include "ImdbResource.h"
#include "ImdbResourceManager.h"
#include "SqldbResource.h"
#include "SqldbResourceManager.h"
#include "SipxProcessResource.h"
#include "SipxProcessResourceManager.h"

class SipxResourceUseTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipxResourceUseTest);
   CPPUNIT_TEST(firstProcess);
   CPPUNIT_TEST(secondProcess);
   CPPUNIT_TEST(checkUsage);
   CPPUNIT_TEST_SUITE_END();

public:

   void firstProcess()
      {
         FileTestContext testContext(TEST_DATA_DIR "resourceUse",
                                     TEST_WORK_DIR "resourceUse"
                                     );
         testContext.setSipxDir(SipXecsService::VarDirType, "var");

         UtlString  path;
         SipxProcess* process;
         UtlString  description;

         /*
          * If you change any of the expectations in this test, you will need to
          * update the secondProcess test below.  That test retrieves the
          * definition that this one creates.
          */

         testContext.inputFilePath("firstprocess.xml", path);
         CPPUNIT_ASSERT((process = SipxProcess::createFromDefinition(path)));
         CPPUNIT_ASSERT_EQUAL(process, SipxProcessManager::getInstance()->findProcess("First"));

         ASSERT_STR_EQUAL("First", process->data());
         ASSERT_STR_EQUAL("1.0.0", process->mVersion.data());

         SipxProcessResource* processResource;
         CPPUNIT_ASSERT(processResource = SipxProcessResourceManager::getInstance()->find("First"));
         CPPUNIT_ASSERT_EQUAL(processResource, process->resource());
         CPPUNIT_ASSERT( ! processResource->isWriteable());
         description.remove(0);
         processResource->appendDescription(description);
         ASSERT_STR_EQUAL("process 'First'",description.data());

         FileResource* fileResource;
         CPPUNIT_ASSERT((fileResource =
                         FileResourceManager::getInstance()->find("/var/log/firstprocess.log")));
         CPPUNIT_ASSERT( ! fileResource->isWriteable());
         description.remove(0);
         fileResource->appendDescription(description);
         ASSERT_STR_EQUAL("file '/var/log/firstprocess.log'",description.data());
         CPPUNIT_ASSERT(!process->mRequiredResources.containsReference(fileResource));
         CPPUNIT_ASSERT(fileResource->mUsedBy.containsReference(processResource));

         CPPUNIT_ASSERT((fileResource =
                         FileResourceManager::getInstance()->find("/etc/sipxpbx/firstprocess.xml")));
         CPPUNIT_ASSERT( fileResource->isWriteable());
         description.remove(0);
         fileResource->appendDescription(description);
         ASSERT_STR_EQUAL("file '/etc/sipxpbx/firstprocess.xml'",description.data());
         CPPUNIT_ASSERT(process->mRequiredResources.containsReference(fileResource));
         CPPUNIT_ASSERT(fileResource->mUsedBy.containsReference(processResource));

         CPPUNIT_ASSERT((fileResource =
                         FileResourceManager::getInstance()->find("/etc/sipxpbx/common.xml")));
         CPPUNIT_ASSERT( fileResource->isWriteable());
         description.remove(0);
         fileResource->appendDescription(description);
         ASSERT_STR_EQUAL("file '/etc/sipxpbx/common.xml'",description.data());
         CPPUNIT_ASSERT(process->mRequiredResources.containsReference(fileResource));
         CPPUNIT_ASSERT(fileResource->mUsedBy.containsReference(processResource));

         CPPUNIT_ASSERT((fileResource =
                         FileResourceManager::getInstance()->find("/etc/sipxpbx/firstprocess-config")));
         CPPUNIT_ASSERT( fileResource->isWriteable());
         description.remove(0);
         fileResource->appendDescription(description);
         ASSERT_STR_EQUAL("file '/etc/sipxpbx/firstprocess-config'",description.data());
         CPPUNIT_ASSERT(fileResource->mUsedBy.containsReference(processResource));

         CPPUNIT_ASSERT((fileResource =
                         FileResourceManager::getInstance()->find("/etc/sipxpbx/commondir/ok-first.xml")));
         CPPUNIT_ASSERT( fileResource->isWriteable());
         CPPUNIT_ASSERT( ! fileResource->isReadable());
         description.remove(0);
         fileResource->appendDescription(description);
         ASSERT_STR_EQUAL("directory '/etc/sipxpbx/commondir' pattern '*-first.xml'",description.data());
         CPPUNIT_ASSERT(fileResource->mUsedBy.containsReference(processResource));

         CPPUNIT_ASSERT((fileResource =
                         FileResourceManager::getInstance()->find("/etc/sipxpbx/commondir/ok-common.xml")));
         CPPUNIT_ASSERT( fileResource->isWriteable());
         CPPUNIT_ASSERT( ! fileResource->isReadable());
         description.remove(0);
         fileResource->appendDescription(description);
         ASSERT_STR_EQUAL("directory '/etc/sipxpbx/commondir' pattern '*-common.xml'",description.data());
         CPPUNIT_ASSERT(fileResource->mUsedBy.containsReference(processResource));

         ImdbResource* imdbResource;
         CPPUNIT_ASSERT((imdbResource =
                         ImdbResourceManager::getInstance()->find("firsttable")));
         CPPUNIT_ASSERT( ! imdbResource->isWriteable());
         description.remove(0);
         imdbResource->appendDescription(description);
         ASSERT_STR_EQUAL("imdb 'firsttable'",description.data());
         CPPUNIT_ASSERT(imdbResource->mUsedBy.containsReference(processResource));

         SqldbResource* sqldbResource;
         CPPUNIT_ASSERT((sqldbResource =
                         SqldbResourceManager::getInstance()->find("FIRSTlocalhostpostgres")));
         CPPUNIT_ASSERT( sqldbResource->isWriteable());
         description.remove(0);
         sqldbResource->appendDescription(description);
         ASSERT_STR_EQUAL("SQL database 'FIRSTlocalhostpostgres'",description.data());
         CPPUNIT_ASSERT(sqldbResource->mUsedBy.containsReference(processResource));
      };

   void secondProcess()
      {
         FileTestContext testContext(TEST_DATA_DIR "resourceUse",
                                     TEST_WORK_DIR "resourceUse"
                                     );
         testContext.setSipxDir(SipXecsService::VarDirType, "var");

         UtlString  path;
         SipxProcess* process;
         UtlString  description;

         /*
          * This test uses the resources created by the firstProcess test above
          */

         testContext.inputFilePath("secondprocess.xml", path);
         CPPUNIT_ASSERT((process = SipxProcess::createFromDefinition(path)));
         CPPUNIT_ASSERT_EQUAL(process, SipxProcessManager::getInstance()->findProcess("Second"));

         ASSERT_STR_EQUAL("Second", process->data());
         ASSERT_STR_EQUAL("1.0.0", process->mVersion.data());

         SipxProcessResource* processResource;
         CPPUNIT_ASSERT(processResource = SipxProcessResourceManager::getInstance()->find("Second"));
         CPPUNIT_ASSERT_EQUAL(processResource, process->resource());
         CPPUNIT_ASSERT( ! processResource->isWriteable());
         description.remove(0);
         processResource->appendDescription(description);
         ASSERT_STR_EQUAL("process 'Second'",description.data());

         FileResource* fileResource;
         CPPUNIT_ASSERT((fileResource =
                         FileResourceManager::getInstance()->find("/var/log/secondprocess.log")));
         CPPUNIT_ASSERT( ! fileResource->isWriteable());
         description.remove(0);
         fileResource->appendDescription(description);
         ASSERT_STR_EQUAL("file '/var/log/secondprocess.log'",description.data());
         CPPUNIT_ASSERT(!process->mRequiredResources.containsReference(fileResource));
         CPPUNIT_ASSERT(fileResource->mUsedBy.containsReference(processResource));

         CPPUNIT_ASSERT((fileResource =
                         FileResourceManager::getInstance()->find("/etc/sipxpbx/secondprocess.xml")));
         CPPUNIT_ASSERT( fileResource->isWriteable());
         description.remove(0);
         fileResource->appendDescription(description);
         ASSERT_STR_EQUAL("file '/etc/sipxpbx/secondprocess.xml'",description.data());
         CPPUNIT_ASSERT(process->mRequiredResources.containsReference(fileResource));
         CPPUNIT_ASSERT(fileResource->mUsedBy.containsReference(processResource));

         CPPUNIT_ASSERT((fileResource =
                         FileResourceManager::getInstance()->find("/etc/sipxpbx/common.xml")));
         CPPUNIT_ASSERT( fileResource->isWriteable());
         description.remove(0);
         fileResource->appendDescription(description);
         ASSERT_STR_EQUAL("file '/etc/sipxpbx/common.xml'",description.data());
         CPPUNIT_ASSERT(process->mRequiredResources.containsReference(fileResource));
         CPPUNIT_ASSERT(fileResource->mUsedBy.containsReference(processResource));

         CPPUNIT_ASSERT((fileResource =
                         FileResourceManager::getInstance()->find("/etc/sipxpbx/secondprocess-config")));
         CPPUNIT_ASSERT( fileResource->isWriteable());
         description.remove(0);
         fileResource->appendDescription(description);
         ASSERT_STR_EQUAL("file '/etc/sipxpbx/secondprocess-config'",description.data());
         CPPUNIT_ASSERT(fileResource->mUsedBy.containsReference(processResource));

         CPPUNIT_ASSERT((fileResource =
                         FileResourceManager::getInstance()->find("/etc/sipxpbx/commondir/ok-second.xml")));
         CPPUNIT_ASSERT( ! fileResource->isWriteable());
         CPPUNIT_ASSERT( fileResource->isReadable());
         description.remove(0);
         fileResource->appendDescription(description);
         ASSERT_STR_EQUAL("directory '/etc/sipxpbx/commondir' pattern '*-second.xml'",description.data());
         CPPUNIT_ASSERT(fileResource->mUsedBy.containsReference(processResource));

         // because the first process defined this to be writable,
         // it is even though our def declares it write-only
         CPPUNIT_ASSERT((fileResource =
                         FileResourceManager::getInstance()->find("/etc/sipxpbx/commondir/ok-common.xml")));
         CPPUNIT_ASSERT( fileResource->isWriteable());
         CPPUNIT_ASSERT( fileResource->isReadable());
         description.remove(0);
         fileResource->appendDescription(description);
         ASSERT_STR_EQUAL("directory '/etc/sipxpbx/commondir' pattern '*-common.xml'",description.data());
         CPPUNIT_ASSERT(fileResource->mUsedBy.containsReference(processResource));

         ImdbResource* imdbResource;
         CPPUNIT_ASSERT((imdbResource =
                         ImdbResourceManager::getInstance()->find("secondtable")));
         CPPUNIT_ASSERT( ! imdbResource->isWriteable());
         description.remove(0);
         imdbResource->appendDescription(description);
         ASSERT_STR_EQUAL("imdb 'secondtable'",description.data());
         CPPUNIT_ASSERT(imdbResource->mUsedBy.containsReference(processResource));

         SqldbResource* sqldbResource;
         CPPUNIT_ASSERT((sqldbResource =
                         SqldbResourceManager::getInstance()->find("SECONDlocalhostpostgres")));
         CPPUNIT_ASSERT( sqldbResource->isWriteable());
         description.remove(0);
         sqldbResource->appendDescription(description);
         ASSERT_STR_EQUAL("SQL database 'SECONDlocalhostpostgres'",description.data());
         CPPUNIT_ASSERT(sqldbResource->mUsedBy.containsReference(processResource));
      };

   void checkUsage()
      {
         FileTestContext testContext(TEST_DATA_DIR "resourceUse",
                                     TEST_WORK_DIR "resourceUse"
                                     );
         testContext.setSipxDir(SipXecsService::VarDirType, "var");

         FileResource* fileResource;
         UtlString  description;

         CPPUNIT_ASSERT((fileResource =
                         FileResourceManager::getInstance()->find("/etc/sipxpbx/common.xml")));
         CPPUNIT_ASSERT( fileResource->isWriteable());
         description.remove(0);
         fileResource->appendDescription(description);
         ASSERT_STR_EQUAL("file '/etc/sipxpbx/common.xml'",description.data());

         CPPUNIT_ASSERT_EQUAL((size_t)2, fileResource->mUsedBy.entries());
      };

};



CPPUNIT_TEST_SUITE_REGISTRATION(SipxResourceUseTest);

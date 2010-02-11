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
#include "SipxProcessResource.h"
#include "SipxProcessResourceManager.h"

class FileDirResourceTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(FileDirResourceTest);
   CPPUNIT_TEST(conflictTest);
   CPPUNIT_TEST_SUITE_END();

public:

   void conflictTest()
      {
         FileTestContext testContext(TEST_DATA_DIR "fileDir",
                                     TEST_WORK_DIR "fileDir"
                                     );
         testContext.setSipxDir(SipXecsService::VarDirType, "var");

         UtlString  path;
         SipxProcess* process;
         UtlString  description;

         testContext.inputFilePath("firstprocess.xml", path);
         CPPUNIT_ASSERT((process = SipxProcess::createFromDefinition(path)));
         CPPUNIT_ASSERT_EQUAL(process, SipxProcessManager::getInstance()->findProcess("FileDir"));

         FileResource* fileResource;

         CPPUNIT_ASSERT((fileResource =
                         FileResourceManager::getInstance()->find("/common/specific.xml")));
         CPPUNIT_ASSERT( fileResource->isWriteable());
         description.remove(0);
         fileResource->appendDescription(description);
         ASSERT_STR_EQUAL("file '/common/specific.xml'",description.data());

         CPPUNIT_ASSERT((fileResource =
                         FileResourceManager::getInstance()->find("/common/default.xml")));
         CPPUNIT_ASSERT( ! fileResource->isWriteable());
         description.remove(0);
         fileResource->appendDescription(description);
         ASSERT_STR_EQUAL("directory '/common' pattern '*.xml'",description.data());

         CPPUNIT_ASSERT((fileResource =
                         FileResourceManager::getInstance()->find("/common/implicit/rw/pattern.xml")));
         CPPUNIT_ASSERT( ! fileResource->isWriteable());
         description.remove(0);
         fileResource->appendDescription(description);
         ASSERT_STR_EQUAL("directory '/common/implicit/rw' pattern '*.xml'",description.data());

         CPPUNIT_ASSERT((fileResource =
                         FileResourceManager::getInstance()->find("/common/implicit/rw/pattern.abc")));
         CPPUNIT_ASSERT( fileResource->isWriteable());
         description.remove(0);
         fileResource->appendDescription(description);
         ASSERT_STR_EQUAL("directory '/common/implicit/rw' pattern '*.abc'",description.data());

         CPPUNIT_ASSERT((fileResource =
                         FileResourceManager::getInstance()->find("/common/implicit/rw/specific.xml")));
         CPPUNIT_ASSERT( ! fileResource->isWriteable());
         description.remove(0);
         fileResource->appendDescription(description);
         ASSERT_STR_EQUAL("file '/common/implicit/rw/specific.xml'",description.data());

         CPPUNIT_ASSERT((fileResource =
                         FileResourceManager::getInstance()->find("/common/explicit/rw/pattern.xml")));
         CPPUNIT_ASSERT( ! fileResource->isWriteable());
         description.remove(0);
         fileResource->appendDescription(description);
         ASSERT_STR_EQUAL("directory '/common/explicit/rw' pattern '*.xml'",description.data());

         CPPUNIT_ASSERT((fileResource =
                         FileResourceManager::getInstance()->find("/common/explicit/rw/pattern.abc")));
         CPPUNIT_ASSERT( fileResource->isWriteable());
         description.remove(0);
         fileResource->appendDescription(description);
         ASSERT_STR_EQUAL("directory '/common/explicit/rw' pattern '*.abc'",description.data());

         CPPUNIT_ASSERT((fileResource =
                         FileResourceManager::getInstance()->find("/common/explicit/rw/specific.xml")));
         CPPUNIT_ASSERT( fileResource->isWriteable());
         description.remove(0);
         fileResource->appendDescription(description);
         ASSERT_STR_EQUAL("file '/common/explicit/rw/specific.xml'",description.data());

      };
};

CPPUNIT_TEST_SUITE_REGISTRATION(FileDirResourceTest);

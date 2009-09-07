//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsFS.h>
#include <os/OsTestUtilities.h>

class OsPathTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(OsPathTest);
    CPPUNIT_TEST(testPathInfo);
    CPPUNIT_TEST_SUITE_END();

    /** where all tests should r/w data */
    OsPath mRootPath;

public:

    void setUp()
    {
        //OsTestUtilities::createTestDir(mRootPath);
    }


    void tearDown()
    {
        //OsTestUtilities::removeTestDir(mRootPath);
    }

    void testPathInfo()
    {
        UtlString testDir = "../../filename.ext";
        OsPath testPath = testDir;

        UtlString parentPath;
        parentPath.append("..").append(OsPath::separator).append("..").append(OsPath::separator);

        ASSERT_STR_EQUAL_MESSAGE("Extension", ".ext", testPath.getExt().data());
        ASSERT_STR_EQUAL_MESSAGE("Parent Path", parentPath, testPath.getDirName().data());
        ASSERT_STR_EQUAL_MESSAGE("Volume", "", testPath.getVolume().data());
        ASSERT_STR_EQUAL_MESSAGE("Filename no extension", "filename",
                                     testPath.getFilename().data());

        // little risky, may not have permissions, but '../../' is too
        // good a test to not try. Refactor if this is bad assumption
        OsFile file(testPath);
        file.touch();

        OsPath nativePath;
        testPath.getNativePath(nativePath);

        // dont' know what's right, but know whats wrong
        CPPUNIT_ASSERT_MESSAGE("Resolved relative path", !nativePath.contains(".."));
        //printf("Native path is %s\n", nativePath.data());

        file.remove();
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(OsPathTest);

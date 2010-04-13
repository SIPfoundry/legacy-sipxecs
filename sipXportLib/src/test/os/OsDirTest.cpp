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

/**
 * Directory testing.  There are missing tests, however overlapping
 * tests are located in OsFileSytemTest. There is redundancy in API
 * because OsFileSystem is a convience wrapper to OsDir, and other
 * file related operations. Longterm we select 1 API and 1 set of
 * tests to maintain.
 */
class OsDirTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(OsDirTest);
    CPPUNIT_TEST(testCreateDir);
    CPPUNIT_TEST(testRemoveDir);
    CPPUNIT_TEST(testRenameDir);
    CPPUNIT_TEST_SUITE_END();

    /** where all tests should r/w data */
    OsPath mRootPath;

public:
    void setUp()
    {
        OsTestUtilities::createTestDir(mRootPath);
    }

    void tearDown()
    {
        OsTestUtilities::removeTestDir(mRootPath);
    }

    void testCreateDir()
    {
        OsStatus stat;
        OsPath testPath = mRootPath + OsPath::separator + "testCreateDir";
        OsDir testDir(testPath);

        stat = testDir.create();
        CPPUNIT_ASSERT_MESSAGE("Create directory ok", stat == OS_SUCCESS);
        CPPUNIT_ASSERT_MESSAGE("Created dir actually exists", testDir.exists());
    }

    void testRemoveDir()
    {
        OsStatus stat;
        OsPath testPath = mRootPath + OsPath::separator + "testRemoveDir";
        OsDir testDir(testPath);

        stat = testDir.create();
        CPPUNIT_ASSERT(stat == OS_SUCCESS);
        CPPUNIT_ASSERT(testDir.exists());
        stat = testDir.remove(FALSE, TRUE);

        CPPUNIT_ASSERT_MESSAGE("Delete ok", stat == OS_SUCCESS);
        CPPUNIT_ASSERT_MESSAGE("Deleted dir !exist", !testDir.exists());
    }

    void testRenameDir()
    {
        OsStatus stat;
        OsPath fromPath = mRootPath + OsPath::separator + "testRenameDirFrom";
        OsPath toPath = mRootPath + OsPath::separator + "testRenameDirTo";
        OsDir fromDir(fromPath);

        stat = fromDir.create();
        CPPUNIT_ASSERT(stat == OS_SUCCESS);
        CPPUNIT_ASSERT(fromDir.exists());

        stat = fromDir.rename(toPath);
        OsDir toDir(toPath);
        CPPUNIT_ASSERT_MESSAGE("Rename dir ok", stat == OS_SUCCESS);
        CPPUNIT_ASSERT_MESSAGE("Rename dir exists", toDir.exists());
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(OsDirTest);

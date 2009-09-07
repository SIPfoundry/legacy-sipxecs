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

#include <os/OsFS.h>
#include <os/OsTestUtilities.h>
#include <sipxunit/TestUtilities.h>

class OsFileSystemTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(OsFileSystemTest);
    CPPUNIT_TEST(testCreateDir);
    CPPUNIT_TEST(testRemoveDir);
    CPPUNIT_TEST(testCreateRecursiveDir);
    CPPUNIT_TEST(testRenameDir);
    CPPUNIT_TEST(testGetInfo);
    CPPUNIT_TEST(testRemoveTree);
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

        OsPath testDir = mRootPath + OsPath::separator + "testCreateDir";

        stat = OsFileSystem::createDir(testDir);
        CPPUNIT_ASSERT_MESSAGE("Creating directory", stat == OS_SUCCESS);

        CPPUNIT_ASSERT_MESSAGE("Directory actually exists", OsFileSystem::exists(testDir));
    }

    void testCreateRecursiveDir()
    {
        OsStatus stat;

        OsPath testDir = mRootPath + OsPath::separator + "testCreateParentDir" + OsPath::separator + "testCreateDir";

        stat = OsFileSystem::createDir(testDir, TRUE);
        CPPUNIT_ASSERT_MESSAGE("Creating directory", stat == OS_SUCCESS);

        CPPUNIT_ASSERT_MESSAGE("Directory actually exists", OsFileSystem::exists(testDir));
    }

    void testRemoveDir()
    {
        OsStatus stat;

        OsPath testDir = mRootPath + OsPath::separator + "testRemoveDir";
        stat = OsFileSystem::createDir(testDir);
        CPPUNIT_ASSERT_MESSAGE("Creating directory", stat == OS_SUCCESS);

        stat = OsFileSystem::remove(testDir, FALSE, TRUE);
        CPPUNIT_ASSERT_MESSAGE("Removed directory", stat == OS_SUCCESS);

        CPPUNIT_ASSERT_MESSAGE("Directory actually removed", !OsFileSystem::exists(testDir));
    }


    void testRenameDir()
    {
        OsStatus stat;
        OsPath renameFrom = mRootPath + OsPath::separator + "testRenameDirFrom";
        OsPath renameTo = mRootPath + OsPath::separator + "testRenameDirTo";

        stat = OsFileSystem::createDir(renameFrom);
        CPPUNIT_ASSERT_MESSAGE("Creating directory", stat == OS_SUCCESS);

        stat = OsFileSystem::rename(renameFrom, renameTo);
        CPPUNIT_ASSERT_MESSAGE("Renamed dir", stat == OS_SUCCESS);

        CPPUNIT_ASSERT_MESSAGE("New dir name there", OsFileSystem::exists(renameTo));
        CPPUNIT_ASSERT_MESSAGE("Old dir name not there", !OsFileSystem::exists(renameFrom));
    }

    void testGetInfo()
    {
        OsStatus stat;
        OsPath testDir = mRootPath + OsPath::separator + "testGetInfo";

        OsFileInfo info;
        OsPath path(testDir);

        stat = OsFileSystem::getFileInfo(testDir, info);
        CPPUNIT_ASSERT_MESSAGE("Fail getting info on non-exists dir", stat != OS_SUCCESS);

        stat = OsFileSystem::createDir(testDir);
        CPPUNIT_ASSERT_MESSAGE("Creating directory", stat == OS_SUCCESS);

        stat = OsFileSystem::getFileInfo(testDir, info);
        CPPUNIT_ASSERT_MESSAGE("Fail getting info on non-exists dir", stat == OS_SUCCESS);

        OsTime createTime;
        info.getCreateTime(createTime);
        //time_t etime = createTime.seconds();
        //osPrintf("Got dir info. now %d created %d\n", now, createTime.seconds());
    }

    void testRemoveTree()
    {
        OsStatus stat;
        OsPath level1dir = mRootPath + OsPath::separator + "level1";
        OsPath level2dir = level1dir + OsPath::separator + "level2";
        OsPath level3dir = level2dir + OsPath::separator + "level3";

        stat = OsFileSystem::createDir(level1dir);
        CPPUNIT_ASSERT(stat == OS_SUCCESS);

        stat = OsFileSystem::createDir(level2dir);
        CPPUNIT_ASSERT(stat == OS_SUCCESS);

        stat = OsFileSystem::createDir(level3dir);
        CPPUNIT_ASSERT(stat == OS_SUCCESS);

        OsPath  filename;
        //now create the files under each dir
        for (int loop = 0; loop < 30;loop++)
        {
            UtlString levelStr = "level1_";
            filename = level1dir + OsPath::separator + levelStr;
            if (loop > 9)
            {
                levelStr = "level2_";
                filename = level2dir + OsPath::separator + levelStr;
            }
            if (loop > 19)
            {
                levelStr = "level3_";
                filename = level3dir + OsPath::separator + levelStr;
            }
            char buf[10];
            sprintf(buf,"%d",loop);
            filename.append(buf);
            OsFile *tmpfile = new OsFile(filename);
            tmpfile->touch();
            delete tmpfile;
        }

        //now delete the tree
        OsPath delPath(level1dir);

        stat = OsFileSystem::remove(delPath, FALSE, TRUE);
        CPPUNIT_ASSERT_MESSAGE("Should fail to delete recursively", stat != OS_SUCCESS);

        KNOWN_BUG("INTERMITTENT failures", "XECS-1588");
        stat = OsFileSystem::remove(delPath, TRUE, TRUE);
        CPPUNIT_ASSERT_MESSAGE("Should succeed to delete recursively", stat == OS_SUCCESS);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsFileSystemTest);

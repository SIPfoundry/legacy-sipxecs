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
#include "os/OsTask.h"

#include <stdlib.h>

/**
 * Test Description
 */
class OsFileTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(OsFileTest);
    CPPUNIT_TEST(testCreateFile);
    CPPUNIT_TEST(testDeleteFile);
    CPPUNIT_TEST(testReadWriteBuffer);
    CPPUNIT_TEST(testCopyFile);
    CPPUNIT_TEST(testReadOnly);
    CPPUNIT_TEST(testTouch);
#ifndef _WIN32
    CPPUNIT_TEST(testReadLock);
#endif
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

    /**
     * Create empty file
     */
    void testCreateFile()
    {
        OsPath testFile = mRootPath + OsPath::separator + "testCreateFile";

        OsFile file(testFile);
        file.touch();
        CPPUNIT_ASSERT_MESSAGE("Touched file exists", file.exists());
    }

    /**
     * Create and delete empty file
     */
    void testDeleteFile()
    {
        OsStatus stat;
        OsPath testFile = mRootPath + OsPath::separator + "testDeleteFile";

        OsFile file(testFile);
        file.touch();
        CPPUNIT_ASSERT(file.exists());
        stat = file.remove();
        CPPUNIT_ASSERT_MESSAGE("File deleted", stat == OS_SUCCESS);
        CPPUNIT_ASSERT(!file.exists());
    }

    /**
     * Creates a new dummy file then reads it back in to verify it's
     * contents, buffer by buffer.
     */
    void testReadWriteBuffer()
    {
        ///////////////////////
        //       W R I T E
        ///////////////////////
        OsStatus stat;
        OsPath testFile = mRootPath + OsPath::separator + "testWriteBuffer";
        char wbuff[10000];
        size_t wbuffsize = (size_t)sizeof(wbuff);

        OsTestUtilities::initDummyBuffer(wbuff, sizeof(wbuff));

        OsFile wfile(testFile);
        stat = wfile.open(OsFile::CREATE);
        CPPUNIT_ASSERT(stat == OS_SUCCESS);

        size_t wposition = 0;
        int i;
        for (i = 0; wposition < wbuffsize; i++)
        {
            size_t remaining = wbuffsize - wposition;
            size_t byteswritten = 0;
            stat = wfile.write(wbuff + wposition, remaining, byteswritten);
            CPPUNIT_ASSERT(stat == OS_SUCCESS);
            wposition += byteswritten;
        }

        //close the file after working with it.
        wfile.close();

        ///////////////////////
        //       R E A D
        ///////////////////////
        char rbuff[256];
        size_t rbuffsize = (size_t)sizeof(rbuff);
        OsFile rfile(testFile);
        stat = rfile.open();
        CPPUNIT_ASSERT(stat == OS_SUCCESS);

        size_t rposition = 0;
        for (i = 0; rposition < wbuffsize; i++)
        {
            size_t remaining = (wbuffsize - rposition);
            size_t readsize = remaining < rbuffsize ? remaining : rbuffsize;
            size_t bytesread = 0;
            stat = rfile.read(rbuff, readsize, bytesread);
            CPPUNIT_ASSERT_MESSAGE("Read buffer", stat == OS_SUCCESS);
            UtlBoolean ok = OsTestUtilities::testDummyBuffer(rbuff, bytesread, rposition);
            CPPUNIT_ASSERT_MESSAGE("Test buffer data", ok);
            rposition += bytesread;
        }

        // proper EOF
        size_t zeroread = 0;
        stat = rfile.read(rbuff, 1, zeroread);
        CPPUNIT_ASSERT_MESSAGE("End of file", stat == OS_FILE_EOF);
        CPPUNIT_ASSERT_MESSAGE("No bytes read", zeroread == 0);
    }

    /**
     * Creates a dummy file, copies it into a new file then verifies
     * it's contents
     */
    void testCopyFile()
    {
        OsStatus stat;
        OsPath copyFrom = mRootPath + OsPath::separator + "testCopyFileFrom";
        OsPath copyTo = mRootPath + OsPath::separator + "testCopyFileTo";

        stat = OsTestUtilities::createDummyFile(copyFrom, 1000);
        CPPUNIT_ASSERT_MESSAGE("Create test file", stat == OS_SUCCESS);

        OsFile copyFromFile(copyFrom);
        copyFromFile.copy(copyTo);

        CPPUNIT_ASSERT_MESSAGE("Copies file exists", OsFileSystem::exists(copyTo));
        UtlBoolean ok = OsTestUtilities::verifyDummyFile(copyTo, 1000);
        CPPUNIT_ASSERT_MESSAGE("Test file verified", ok);
    }

    void testReadOnly()
    {
        OsStatus stat;
        OsPath testPath = mRootPath + OsPath::separator + "testReadOnly";
        OsFile testFile(testPath);

        testFile.touch();

        stat = testFile.setReadOnly(TRUE);
        CPPUNIT_ASSERT_MESSAGE("No error setting read only", stat == OS_SUCCESS);
        CPPUNIT_ASSERT_MESSAGE("Read only", testFile.isReadonly());

        stat = testFile.setReadOnly(FALSE);
        CPPUNIT_ASSERT_MESSAGE("No error setting read only", stat == OS_SUCCESS);
        CPPUNIT_ASSERT_MESSAGE("Not Read only", !testFile.isReadonly());
    }

    void testTouch()
    {
        OsFileInfoBase info;
        OsTime time1, time2;
        OsPath testFile = mRootPath + OsPath::separator + "testTouch";
        OsFile file(testFile);
        // Create the file
        file.touch();
        file.getFileInfo(info);
        info.getModifiedTime(time1);
        // Get the modification time and sleep for a few seconds
        OsTask::delay(2000);
        // Touch the file and get the modification time again
        file.touch();
        file.getFileInfo(info);
        info.getModifiedTime(time2);
        // The modification times must differ by the sleep amount
        CPPUNIT_ASSERT_MESSAGE("The time is the same", time1.seconds() != time2.seconds());
    }

    /**
     * Created dummy files and attempts to gain and deny access to locks
     */
    void testReadLock()
    {
        OsStatus stat;
        OsPath testPath = mRootPath + OsPath::separator + "testReadLock";
        OsFile testFile(testPath);
        OsFile testFile2(testPath);
        testFile.touch();

        stat = testFile.open(OsFile::READ_ONLY | OsFile::FSLOCK_READ);
        CPPUNIT_ASSERT_MESSAGE("Get a read lock on a file", stat == OS_SUCCESS);

        stat = testFile2.open(OsFile::READ_ONLY | OsFile::FSLOCK_READ);

        // According to OsFile::open, two READs are allowed even though
        // mode bitmask has locks.  This was discovered 07/08/04 by DLH
        // it's conceivable that OsFile is wrong, but it explicitly says
        // it's ok and it's possible changing it may have negative impact
        CPPUNIT_ASSERT_MESSAGE("Should get a read lock even though a file already locked",
            stat == OS_SUCCESS);

        stat = testFile.fileunlock();
        CPPUNIT_ASSERT_MESSAGE("Release a read lock on a file", stat == OS_SUCCESS);

        stat = testFile2.open(OsFile::READ_ONLY | OsFile::FSLOCK_READ);
        CPPUNIT_ASSERT_MESSAGE("Read lock is release", stat == OS_SUCCESS);
        testFile2.close();

        // TODO: Test FSLOCK_WRITE and FSLOCK_WAIT
    }

};


CPPUNIT_TEST_SUITE_REGISTRATION(OsFileTest);

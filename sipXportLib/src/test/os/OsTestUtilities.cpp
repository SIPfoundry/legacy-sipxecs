//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include <cppunit/TestCase.h>
#include <cppunit/SourceLine.h>
#include <cppunit/Asserter.h>
#include <string>

//Application Includes
#include <os/OsDefs.h>
#include <os/OsFileSystem.h>
#include <sipxunit/TestUtilities.h>
#include <os/OsTestUtilities.h>

void OsTestUtilities::createTestDir(OsPath& root)
{
    OsStatus stat;

    OsFileSystem::getWorkingDirectory(root);
    root.append(OsPath::separator).append("OsFileSystemTest");

    if (OsFileSystem::exists(root))
    {
        removeTestDir(root);
    }

    stat = OsFileSystem::createDir(root);
    CPPUNIT_ASSERT_MESSAGE("setup root test dir", stat == OS_SUCCESS);
}

void OsTestUtilities::removeTestDir(OsPath &root)
{
    OsStatus stat;
    if (OsFileSystem::exists(root))
    {
        stat = OsFileSystem::remove(root, TRUE, TRUE);

         KNOWN_BUG("Fails randomly on build server and fails everytime, the first time its run on a new machine",
           "XPL-191");

        CPPUNIT_ASSERT_MESSAGE("teardown root test dir", stat == OS_SUCCESS);
    }
}

void OsTestUtilities::initDummyBuffer(char *buff, int size)
{
    for (int i = 0; i < size; i++)
    {
        buff[i] = (char)(i % 256);
    }
}

UtlBoolean OsTestUtilities::testDummyBuffer(char *buff, unsigned long size, unsigned long position)
{
    for (unsigned long i = 0; i < size; i++)
    {
        char expected = (char)((position + i) % 256);
        if (buff[i] != expected)
        {
            printf("buff[%li] = %i, expected = %i\n", position, buff[i], expected);
            return false;
        }
    }

    return true;
}


OsStatus OsTestUtilities::createDummyFile(OsPath testFile, unsigned long size)
{
    OsStatus stat;
    char wbuff[10000];
    size_t wbuffsize = (size_t)sizeof(wbuff);
    OsTestUtilities::initDummyBuffer(wbuff, sizeof(wbuff));

    OsFile wfile(testFile);
    stat = wfile.open(OsFile::CREATE);
    if (stat == OS_SUCCESS)
    {
        size_t wposition = 0;
        for (int i = 0; stat == OS_SUCCESS && wposition < wbuffsize; i++)
        {
            size_t remaining = wbuffsize - wposition;
            size_t byteswritten = 0;
            stat = wfile.write(wbuff + wposition, remaining, byteswritten);
            wposition += byteswritten;
        }

        wfile.close();
    }

    return stat;
}

UtlBoolean OsTestUtilities::verifyDummyFile(OsPath testFile, unsigned long size)
{
    OsStatus stat;
    UtlBoolean ok = false;
    char rbuff[256];
    size_t rbuffsize = (size_t)sizeof(rbuff);
    OsFile rfile(testFile);
    stat = rfile.open();
    if (stat == OS_SUCCESS)
    {
        size_t rposition = 0;
        ok = true;
        for (int i = 0; ok && rposition < size; i++)
        {
            size_t remaining = (size - rposition);
            size_t readsize = remaining < rbuffsize ? remaining : rbuffsize;
            size_t bytesread = 0;
            stat = rfile.read(rbuff, readsize, bytesread);
            if (stat != OS_SUCCESS)
            {
                ok = false;
                printf("Error reading file, status = %i", stat);
            }
            else
            {
                ok = OsTestUtilities::testDummyBuffer(rbuff, bytesread, rposition);
                rposition += bytesread;
            }
        }

        rfile.close();
    }

    return ok;
}

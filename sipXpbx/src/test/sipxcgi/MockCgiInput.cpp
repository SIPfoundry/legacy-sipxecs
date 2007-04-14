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

#include <string>
#include <cgicc/CgiInput.h>
#include <os/OsStatus.h>
#include <os/OsFS.h>
#include <os/OsConfigDb.h>
#include <MockCgiInput.h>

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */


MockCgiInput::MockCgiInput(char *filename)
    : m_read(NULL)
{
    OsPath srcPath(SRCDIR);
    OsPath filepath = srcPath + OsPath::separator + filename;
    OsFile file(filepath);
    CPPUNIT_ASSERT_MESSAGE("Cgi test input file exists", file.exists());

    m_env = new OsConfigDb();
    OsStatus stat = m_env->loadFromFile(filepath.data());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cgi test input file exists", OS_SUCCESS, stat);
}

void MockCgiInput::setReadFile(const char *readfile)
{
    OsPath srcPath(SRCDIR);
    OsPath filepath = srcPath + OsPath::separator + readfile;
    m_read = new OsFile(filepath);
}

MockCgiInput::~MockCgiInput()
{
    if (m_read != NULL)
    {
        m_read->close();
        delete m_read;
    }

    delete m_env;
}

std::string MockCgiInput::getenv(const char *envName)
{
    // See CgiEnvironment.cpp for list of standard http env. values

    std::string envValue;

    if (envName != NULL)
    {
        UtlString key(envName);
        UtlString value;
        OsStatus stat = m_env->get(key, value);
        if (stat == OS_SUCCESS)
        {
            envValue += value.data();
        }
    }

    return envValue;
}

size_t MockCgiInput::read(char *data, size_t length)
{

    unsigned long bytesRead = 0;
    OsStatus rc = m_read->read(data, (unsigned long)length, bytesRead);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Read from cgi test input", OS_SUCCESS, rc);

    return (size_t)bytesRead;
}

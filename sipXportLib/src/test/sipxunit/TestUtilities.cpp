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
#include <cppunit/Message.h>
#include <cppunit/Exception.h>
#include <iostream>
#include <string.h>
#include <cstdarg>

// Application Includes
#include <sipxunit/TestUtilities.h>
#include <sipxunit/TestMonitor.h>

// Static initializers
bool TestUtilities::m_testingKnownBug = false;

char TestUtilities::m_bugMessage[MAX_BUG_MESSAGE_LEN];

/* ========================= UTILITY METHODS ================================= */

void TestUtilities::createMessage(int count, std::string* createdMessage, ...)
{
    createdMessage -> erase() ;
    va_list arguments ;
    va_start(arguments, createdMessage) ;
    for(int i = 0 ; i < count ; i++)
    {
        const char* strTemp = va_arg(arguments, const char*) ;
        createdMessage -> append(strTemp) ;
    }
}

std::string TestUtilities::printString(const char* sz)
{
    std::string str;
    if (sz == NULL)
    {
        str.append("NULL");
    }
    else
    {
        str.append("\"").append(sz).append("\"");
    }

    return std::string(str);
}

void TestUtilities::assertEquals(const char* expected, const char* actual,
        CppUnit::SourceLine sourceLine, const std::string &message)
{
    if (strcmp(expected, actual) != 0)
    {
        CppUnit::Asserter::failNotEqual(printString(expected),
                                        printString(actual),
                                        sourceLine,
                                        message);
    }
}

void TestUtilities::knownBug(const char* message, const char* bugNo,
         CppUnit::SourceLine sourceLine)
{
    m_testingKnownBug = true;
    CppUnit::Message msg(printBug(message, bugNo));
    CppUnit::Exception err(msg, sourceLine);
    strncpy(m_bugMessage, err.what(), MAX_BUG_MESSAGE_LEN);

    // return after and allow non-fatal error to occur
}

void TestUtilities::knownFatalBug(const char* message, const char* bugNo,
         CppUnit::SourceLine sourceLine)
{
    m_testingKnownBug = true;
    std::string bugMsg = printBug(message, bugNo);

    // throw an assertion so as so avoid fatal code that would normally
    // follow this call
    CppUnit::Asserter::fail(bugMsg, sourceLine);
}

void TestUtilities::knownEfenceBug(const char* message, const char* bugNo,
         CppUnit::SourceLine sourceLine)
{
    // Electric fence causes segfaults at location of error so all tests die
    // so effectively efence on will avoid error.
#ifdef HAVE_EFENCE
    knownFatalBug(message, bugNo, sourceLine);
#endif
}

std::string TestUtilities::printBug(const char* message, const char *bugNo)
{
    std::string bugMsg("BUG ");
    bugMsg += bugNo;
    bugMsg += ": ";
    bugMsg += message;

    return bugMsg;
}

const char *TestUtilities::getKnownBugMessage()
{
    return (const char*)m_bugMessage;
}

bool TestUtilities::testingKnownBug()
{
    return m_testingKnownBug;
}

void TestUtilities::resetKnownBugTesting()
{
    m_testingKnownBug = false;
    m_bugMessage[0] = 0;
}

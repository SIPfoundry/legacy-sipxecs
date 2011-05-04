//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <cppunit/Test.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/Message.h>
#include <cppunit/Exception.h>
#include <cppunit/TestFailure.h>
#include <sipxunit/TestUtilities.h>
#include <sipxunit/TestMonitor.h>
#include "os/OsLogger.h"

TestMonitor::TestMonitor()
    : CppUnit::TestResultCollector()
{
    m_wasSuccessful = true;
    m_verbose = true;
}

TestMonitor::~TestMonitor()
{
}

void TestMonitor::startTest(CppUnit::Test *test)
{
    // set static context, assumes tests run in serial
    TestUtilities::resetKnownBugTesting();
    if (m_verbose)
    {
        std::cout << "[TEST]:" << test->getName() << std::endl;
    }

    TestResultCollector::startTest(test);
}

void TestMonitor::addFailure(const CppUnit::TestFailure &failure)
{
    // query static context, assumes tests run in serial
    if (!TestUtilities::testingKnownBug())
    {
        m_wasSuccessful = false;
    }

    CppUnit::Message msg;
    msg = failure.thrownException()->message();

    // Add bug details to final message
    if (strlen(TestUtilities::getKnownBugMessage()) != 0)
    {
        msg.addDetail(TestUtilities::getKnownBugMessage());
        failure.thrownException()->setMessage(msg);
    }

    TestResultCollector::addFailure(failure);

    std::string stdMessage;
    stdMessage = msg.shortDescription();
    stdMessage += "\n";
    stdMessage += msg.details();

    Os::Logger::instance().log(FAC_KERNEL, PRI_ERR, "%s", stdMessage.c_str());
}

bool TestMonitor::wasSuccessful() const
{
    return m_wasSuccessful;
}

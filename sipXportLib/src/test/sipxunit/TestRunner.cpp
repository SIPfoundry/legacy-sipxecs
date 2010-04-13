//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

// The cppunit/TestRunner.h file has to be included twice
//   once before cppunit/TestResult.h for cppunit 1.11
#include <cppunit/TestRunner.h>
#include <cppunit/TestResult.h>
//   once after cppunit/TestResult.h for cppunit 1.10
#include <sipxunit/TestRunner.h>

#include <sipxunit/TestOutputter.h>
#include <sipxunit/TestMonitor.h>

#include <sipxunit/TestOsSysLogListener.h>

TestRunner::TestRunner()
    : CppUnit::TestRunner()
{
    m_monitor = new TestMonitor();
    m_outputter = new TestOutputter(m_monitor);
    m_result = new CppUnit::TestResult();

    m_logger = new TestOsSysLogListener();

    m_result->addListener(m_monitor);

    // To disable the use of OsSysLog during unit tests, comment out the following line
    m_result->addListener(m_logger);
}


TestRunner::~TestRunner()
{
    delete m_result;
    delete m_outputter;
    delete m_monitor;
    delete m_logger;
}

bool TestRunner::run()
{
    CppUnit::TestRunner *pthis = this;
    pthis->run(*m_result);
    m_outputter->write();

    return m_monitor->wasSuccessful();
}

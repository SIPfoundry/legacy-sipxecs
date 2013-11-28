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

#include <iostream>

TestRunner::TestRunner()
    : CppUnit::TestRunner()
{
    m_monitor = new TestMonitor();
    m_outputter = new TestOutputter(m_monitor);
    m_result = new CppUnit::TestResult();

    m_resultCollector = new CppUnit::TestResultCollector();

    m_logger = new TestOsSysLogListener();

    m_result->addListener(m_monitor);

    // To disable the use of OsSysLog during unit tests, comment out the following line
    m_result->addListener(m_logger);

    m_result->addListener(m_resultCollector);

    m_xmlout.open("sipxunit-test-result.xml");
    m_xmlOutputter = new CppUnit::XmlOutputter(m_resultCollector, m_xmlout);
}

TestRunner::~TestRunner()
{
    delete m_xmlOutputter;
    delete m_logger;
    delete m_resultCollector;
    delete m_result;
    delete m_outputter;
    delete m_monitor;

    m_xmlout.close();
}

bool TestRunner::run()
{
    CppUnit::TestRunner *pthis = this;
    pthis->run(*m_result);
    m_outputter->write();
    m_xmlOutputter->write();

    return m_monitor->wasSuccessful();
}

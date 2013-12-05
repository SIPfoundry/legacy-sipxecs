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

#include <os/OsFS.h>

#include <iostream>


void TestRunner::init()
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

  m_xmlout.open(m_xmlFileName.c_str());
  m_xmlOutputter = new CppUnit::XmlOutputter(m_resultCollector, m_xmlout);
}

std::string TestRunner::getXmlFileName()
{
  std::size_t pos = 0;
  std::string xmlFileName;

  xmlFileName = m_pArgv[0];

  pos = xmlFileName.rfind(OsPathBase::separator.str());
  if (std::string::npos != pos)
  {
    xmlFileName.erase(0, pos + 1);
  }

  xmlFileName += "-sipxunit-test-result.xml";

  return xmlFileName;
}

TestRunner::TestRunner(char** pArgv)
    : CppUnit::TestRunner(),
      m_pArgv(pArgv)
{
  m_xmlFileName = getXmlFileName();

  init();
}

TestRunner::TestRunner()
    : CppUnit::TestRunner(),
      m_pArgv(0)
{
  m_xmlFileName = "sipxunit-test-result.xml";

  init();
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

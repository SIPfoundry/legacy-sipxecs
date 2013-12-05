//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#ifndef _TestRunner_h_
#define _TestRunner_h_

#include <cppunit/TestRunner.h>
#include <cppunit/XmlOutputter.h>
#include <sipxunit/TestOutputter.h>
#include <sipxunit/TestOsSysLogListener.h>

#include <string>

class TestMonitor;

/**
 * Instantiates nec. objects for running a test suite for main method
 */
class TestRunner : public CppUnit::TestRunner
{
 public:
    TestRunner();
    TestRunner(char** pArgv);

    virtual ~TestRunner();

    bool run();

 private:
    // compute xml file name
    std::string getXmlFileName();
    void init();

    TestMonitor *m_monitor;
    TestOsSysLogListener *m_logger;

    CppUnit::TestResult *m_result;
    CppUnit::TestResultCollector* m_resultCollector;

    CppUnit::XmlOutputter* m_xmlOutputter;
    std::string m_xmlFileName;

    std::ofstream m_xmlout;

    TestOutputter *m_outputter;
    char** m_pArgv;
};

#endif

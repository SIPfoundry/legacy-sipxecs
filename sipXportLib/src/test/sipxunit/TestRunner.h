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
#include <sipxunit/TestOutputter.h>
#include <sipxunit/TestOsSysLogListener.h>

class TestMonitor;

/**
 * Instantiates nec. objects for running a test suite for main method
 */
class TestRunner : public CppUnit::TestRunner
{
 public:

    TestRunner();

    virtual ~TestRunner();

    bool run();

 private:

    TestMonitor *m_monitor;
    TestOsSysLogListener *m_logger;

    CppUnit::TestResult *m_result;

    TestOutputter *m_outputter;
};

#endif

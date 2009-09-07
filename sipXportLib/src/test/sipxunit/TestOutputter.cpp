//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <cppunit/TestResultCollector.h>
#include <sipxunit/TestOutputter.h>

TestOutputter::TestOutputter(CppUnit::TestResultCollector *collector)
    : CppUnit::TextOutputter(collector, std::cout)
{
    // Default implementation doesn't send ALL failure out
    // to stderr so use this delegate for failure outputting
    m_err = new CppUnit::TextOutputter(collector, std::cerr);
}

TestOutputter::~TestOutputter()
{
    delete m_err;
}

void TestOutputter::printFailures()
{
    m_err->printFailures();
}

void TestOutputter::printFailureWarning()
{
    m_err->printFailureWarning();
}

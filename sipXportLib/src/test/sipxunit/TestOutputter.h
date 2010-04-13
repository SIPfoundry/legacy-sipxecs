//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#ifndef _TestOutputter_h_
#define _TestOutputter_h_

#include <iostream>
#include <cppunit/Outputter.h>
#include <cppunit/TextOutputter.h>

/**
 * Handle actually printing to stdout, stderr and report file
 */
class TestOutputter : public CppUnit::TextOutputter
{
 public:

    TestOutputter(CppUnit::TestResultCollector *collector);

    virtual ~TestOutputter();

 protected:

    void printFailures();

    void printFailureWarning();

 private:

    CppUnit::TextOutputter *m_err;
};

#endif

//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <os/OsProcess.h>
#include <os/OsStatus.h>
#include <sipxunit/TestUtilities.h>

class OsProcessIteratorTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(OsProcessIteratorTest);
    CPPUNIT_TEST(testIterator);
    CPPUNIT_TEST_SUITE_END();


public:

    /**
     * Just excersizes AIP. Unclear how to create pass/fail tests
     */
    void testIterator()
    {
        OsStatus stat;
        OsProcess process;
        OsProcessIterator pi;

        stat = pi.findFirst(process);
        KNOWN_BUG("Unknown failure", "XPL-12");
        CPPUNIT_ASSERT_MESSAGE("First process", stat == OS_SUCCESS);

        while (stat == OS_SUCCESS)
        {
            UtlString name;
            process.getProcessName(name);
            #if defined(WIN32) || defined(__hpux)
            /*on Windows and HP-UX, the system process is pid 0 */
            CPPUNIT_ASSERT_MESSAGE("Valid PID",process.getPID() >= 0);
            #else
            CPPUNIT_ASSERT_MESSAGE("Valid PID", process.getPID() != 0);
            #endif
            CPPUNIT_ASSERT_MESSAGE("Valid Parent PID", process.getParentPID() >= 0);
            CPPUNIT_ASSERT_MESSAGE("Valid process name", name.data() != NULL);

            stat = pi.findNext(process);
        }
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsProcessIteratorTest);

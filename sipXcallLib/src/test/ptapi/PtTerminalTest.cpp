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
#include <sipxunit/TestUtilities.h>

#include <ptapi/PtTerminal.h>

/**
 * Unittest for PtTerminal
 */
class PtTerminalTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(PtTerminalTest);
    CPPUNIT_TEST(testCreators);
    CPPUNIT_TEST(testManipulators);
    CPPUNIT_TEST(testAccessors);
    CPPUNIT_TEST_SUITE_END();


public:
    void testCreators()
    {
        PtTerminal*  pTempPtTerminal;
        PtTerminal*  pTempPtTerminal_1;

        pTempPtTerminal = new PtTerminal("testName");
        delete pTempPtTerminal;

        pTempPtTerminal = new PtTerminal();
        delete pTempPtTerminal;

        pTempPtTerminal = new PtTerminal("original");
        pTempPtTerminal_1 = new PtTerminal(*pTempPtTerminal);

        char name[128];
        CPPUNIT_ASSERT_EQUAL_MESSAGE("get name", PT_SUCCESS,
                pTempPtTerminal_1->getName(name, sizeof(name) - 1));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("correct terminal name", 0, strcmp(name, "original"));

        delete pTempPtTerminal;
        delete pTempPtTerminal_1;
    }

    void testManipulators()
    {
        PtTerminal*  pTempPtTerminal;
        PtTerminal*  pTempPtTerminal_1;

        pTempPtTerminal = new PtTerminal("first");
        pTempPtTerminal_1 = new PtTerminal("second");
        *pTempPtTerminal_1 = *pTempPtTerminal;

        char name[128];
        CPPUNIT_ASSERT_EQUAL_MESSAGE("get name", PT_SUCCESS,
                pTempPtTerminal->getName(name, sizeof(name) - 1));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("correct terminal name", 0, strcmp(name, "first"));

        delete pTempPtTerminal;
        delete pTempPtTerminal_1;

        pTempPtTerminal = new PtTerminal("first");
        pTempPtTerminal_1 = new PtTerminal();
        *pTempPtTerminal_1 = *pTempPtTerminal;

        char name_1[128];
        CPPUNIT_ASSERT_EQUAL_MESSAGE("get name", PT_SUCCESS,
                pTempPtTerminal_1->getName(name_1, sizeof(name_1) - 1));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("correct terminal name", 0, strcmp(name_1, "first"));

        delete pTempPtTerminal;
        delete pTempPtTerminal_1;
    }

    void testAccessors()
    {
        PtTerminal*  pTempPtTerminal;
        char         pTestName[64];

        pTempPtTerminal = new PtTerminal("hello");
        pTempPtTerminal->getName(pTestName, 63);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("get name", PT_SUCCESS,
                pTempPtTerminal->getName(pTestName, 63));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("correct terminal name", 0, strcmp(pTestName, "hello"));
        delete pTempPtTerminal;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(PtTerminalTest);

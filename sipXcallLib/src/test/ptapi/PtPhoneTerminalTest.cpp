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

#include <ptapi/PtPhoneTerminal.h>

/**
 * Unittest for PtPhoneTermal
 */
class PtPhoneTerminalTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(PtPhoneTerminalTest);
    CPPUNIT_TEST(testCreators);
    CPPUNIT_TEST(testManipulators);
    CPPUNIT_TEST(testAccessors);
    CPPUNIT_TEST_SUITE_END();


public:
    void testCreators()
    {
        PtPhoneTerminal*     pTempPtPhoneTerminal;
        PtPhoneTerminal*     pTempPtPhoneTerminal_1;

        pTempPtPhoneTerminal = new PtPhoneTerminal("testName");
        delete pTempPtPhoneTerminal;

        pTempPtPhoneTerminal = new PtPhoneTerminal();
        delete pTempPtPhoneTerminal;

        pTempPtPhoneTerminal = new PtPhoneTerminal("original");
        pTempPtPhoneTerminal_1 = new PtPhoneTerminal(*pTempPtPhoneTerminal);

        char name[128];
        CPPUNIT_ASSERT_EQUAL_MESSAGE("get name", PT_SUCCESS,
                pTempPtPhoneTerminal_1->getName(name, sizeof(name) - 1));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("correct terminal name", 0, strcmp(name, "original"));
        delete pTempPtPhoneTerminal;
        delete pTempPtPhoneTerminal_1;
    }

    void testManipulators()
    {
        PtPhoneTerminal*  pTempPtPhoneTerminal;
        PtPhoneTerminal*  pTempPtPhoneTerminal_1;

        pTempPtPhoneTerminal = new PtPhoneTerminal("first");
        pTempPtPhoneTerminal_1 = new PtPhoneTerminal("second");
        *pTempPtPhoneTerminal_1 = *pTempPtPhoneTerminal;

        char name[128];
        CPPUNIT_ASSERT_EQUAL_MESSAGE("get name", PT_SUCCESS,
                pTempPtPhoneTerminal->getName(name, sizeof(name) - 1));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("correct terminal name", 0, strcmp(name, "first"));

        delete pTempPtPhoneTerminal;
        delete pTempPtPhoneTerminal_1;

        pTempPtPhoneTerminal = new PtPhoneTerminal("first");
        pTempPtPhoneTerminal_1 = new PtPhoneTerminal();
        *pTempPtPhoneTerminal_1 = *pTempPtPhoneTerminal;

        char name_1[128];
        CPPUNIT_ASSERT_EQUAL_MESSAGE("get name", PT_SUCCESS,
                pTempPtPhoneTerminal_1->getName(name_1, sizeof(name_1) - 1));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("correct terminal name", 0, strcmp(name_1, "first"));

        delete pTempPtPhoneTerminal;
        delete pTempPtPhoneTerminal_1;
    }

    void testAccessors()
    {
        PtPhoneTerminal*     pTempPtPhoneTerminal;
        char         pTestName[64];

        pTempPtPhoneTerminal = new PtPhoneTerminal("hello");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("get name", PT_SUCCESS,
                pTempPtPhoneTerminal->getName(pTestName, 63));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("correct terminal name", 0, strcmp(pTestName, "hello"));
        delete pTempPtPhoneTerminal;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(PtPhoneTerminalTest);

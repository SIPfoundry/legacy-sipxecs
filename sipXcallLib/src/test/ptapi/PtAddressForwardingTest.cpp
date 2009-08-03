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

#include <ptapi/PtAddressForwarding.h>

/**
 * Unittest for PtAddressForwaring
 */
class PtAddressForwardingTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(PtAddressForwardingTest);
    CPPUNIT_TEST(testCreators);
    CPPUNIT_TEST(testManipulators);
    CPPUNIT_TEST(testAccessors);
    CPPUNIT_TEST_SUITE_END();


public:
    void testCreators()
    {
        PtAddressForwarding* pTempAddress;
        PtAddressForwarding* pTempAddress_1;

        pTempAddress = new PtAddressForwarding("2345");
        delete pTempAddress;

        pTempAddress = new PtAddressForwarding();
        delete pTempAddress;

        pTempAddress = new PtAddressForwarding("2345");
        pTempAddress_1 = new PtAddressForwarding(*pTempAddress);
        delete pTempAddress;
        delete pTempAddress_1;
    }

    void testManipulators()
    {
        PtAddressForwarding* pTempAddress;
        PtAddressForwarding* pTempAddress_1;

        // test the assignment method (if implemented)
        pTempAddress = new PtAddressForwarding("1234");
        pTempAddress_1 = new PtAddressForwarding("2345");
        *pTempAddress_1 = *pTempAddress;
        delete pTempAddress;
        delete pTempAddress_1;
    }

    void testAccessors()
    {
        PtAddressForwarding* pTempAddress;
        char pTempChar[128];

        pTempAddress = new PtAddressForwarding("myComputer");
        pTempAddress->getDestinationAddress(pTempChar, 127);
        delete pTempAddress;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(PtAddressForwardingTest);

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

#include <ptapi/PtAddress.h>

/**
 * Unittest for PtAddress
 */
class PtAddressTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(PtAddressTest);
    CPPUNIT_TEST(testCreators);
    CPPUNIT_TEST_SUITE_END();


public:
    void testCreators()
    {
        PtAddress *pTempAddress = new PtAddress("2345");
        delete pTempAddress;

        // test the default constructor (if implemented)
        pTempAddress = new PtAddress();
        delete pTempAddress;

        // test the copy constructor (if implemented)
        pTempAddress = new PtAddress("2345");
        PtAddress *pTempAddress_1 = new PtAddress(*pTempAddress);
        delete pTempAddress;
        delete pTempAddress_1;
    }

    void testManipulators()
    {
        PtAddress*   pTempAddress;
        PtAddress*   pTempAddress_1;

        pTempAddress = new PtAddress("1234");
        pTempAddress_1 = new PtAddress("2345");
        *pTempAddress_1 = *pTempAddress;
        delete pTempAddress;
        delete pTempAddress_1;
    }

    void testAccessors()
    {
        PtAddress*   pTempAddress;
        char         pTempChar[128];

        pTempAddress = new PtAddress("myComputer");
        pTempAddress->getName(pTempChar, 127);
        delete pTempAddress;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(PtAddressTest);

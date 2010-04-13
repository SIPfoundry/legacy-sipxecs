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

#include <ptapi/PtConnection.h>
#include <ptapi/PtAddress.h>
#include <ptapi/PtCall.h>

/**
 * Unittest for PtConnection
 */
class PtConnectionTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(PtConnectionTest);
    CPPUNIT_TEST(testCreators);
    CPPUNIT_TEST(testManipulators);
    CPPUNIT_TEST(testAccessors);
    CPPUNIT_TEST_SUITE_END();


public:
    void testCreators()
    {
        PtConnection*        pTempConn;
        PtConnection*        pTempConn_1;

        pTempConn  = new PtConnection("address", "callId");
        delete pTempConn;

        pTempConn  = new PtConnection();
        delete pTempConn;

        pTempConn  = new PtConnection("address", "callId");
        pTempConn_1 = new PtConnection(*pTempConn);
        delete pTempConn;
        delete pTempConn_1;
    }

    void testManipulators()
    {
        PtConnection*        pTempConn;
        PtConnection*        pTempConn_1;

        pTempConn = new PtConnection("address", "callId");
        pTempConn_1 = new PtConnection();
        *pTempConn_1 = *pTempConn;
        delete pTempConn;
        delete pTempConn_1;
    }

    void testAccessors()
    {
        PtAddress*           pTempAddress;
        PtCall*              pTempCall;
        PtConnection*        pTempConn;

        pTempAddress = new PtAddress("MyComputer");
        pTempCall = new PtCall("thisCall");
        pTempConn = new PtConnection("address", "callId");
        pTempConn->getAddress(*pTempAddress);
        pTempConn->getCall(*pTempCall);
        delete pTempConn;
        delete pTempAddress;
        delete pTempCall;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(PtConnectionTest);

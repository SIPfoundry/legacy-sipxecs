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

#include <ptapi/PtCall.h>
#include <ptapi/PtAddress.h>

/**
 * Unittest for PtCall
 */
class PtCallTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(PtCallTest);
    CPPUNIT_TEST(testCreators);
    CPPUNIT_TEST(testManipulators);
    CPPUNIT_TEST(testAccessors);
    CPPUNIT_TEST_SUITE_END();


public:
    void testCreators()
    {
        PtCall*              pPtCall;
        PtCall*              pPtCall_1;

        // test protected constructor
        pPtCall = new PtCall("callId");
        delete pPtCall;

        // test the default constructor (if implemented)
        pPtCall = new PtCall();
        delete pPtCall;

        pPtCall = new PtCall("callID");
        pPtCall_1 = new PtCall(*pPtCall);
        delete pPtCall;
        delete pPtCall_1;
    }

    void testManipulators()
    {
        PtCall*              pTempPtCall;
        PtCall*              pTempPtCall_1;

        pTempPtCall = new PtCall("callID");
        pTempPtCall_1 = new  PtCall("callID_1");
        *pTempPtCall_1 = *pTempPtCall;
        delete pTempPtCall;
        delete pTempPtCall_1;
   }

    void testAccessors()
    {
        PtAddress*   pTempAddress;
        PtCall*              pTempPtCall;

        pTempPtCall = new PtCall("callId");
        pTempAddress = new PtAddress("MyComputer");
        // pTempPtCall->getCalledAddress(*pTempAddress);
        //:WARNING: unless the proxy server already started, we cann't achieve delta = 0
        delete pTempAddress;
        delete pTempPtCall;

        // test getCallingAddress
        pTempPtCall = new PtCall("callId");
        pTempAddress = new PtAddress("MyComputer");
        // pTempPtCall->getCallingAddress(*pTempAddress);
        //:WARNING: unless the proxy server already started, we cann't achieve delta = 0
        delete pTempAddress;
        delete pTempPtCall;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(PtCallTest);

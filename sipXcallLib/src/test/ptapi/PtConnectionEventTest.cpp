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

#include <ptapi/PtConnectionEvent.h>

/**
 * Unittest for PtConnectionEvent
 */
class PtConnectionEventTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(PtConnectionEventTest);
    CPPUNIT_TEST(testCreators);
    CPPUNIT_TEST(testManipulators);
    CPPUNIT_TEST_SUITE_END();


public:
    void testCreators()
    {
        PtConnectionEvent* pTempTermConnEvt;
        PtConnectionEvent* pTempTermConnEvt_1;

        pTempTermConnEvt = new PtConnectionEvent();
        delete pTempTermConnEvt;

        pTempTermConnEvt = new PtConnectionEvent(PtEvent::CALL_INVALID,
           0,
           "terminalName",
           "callId",
           0, 0);
        delete pTempTermConnEvt;

        pTempTermConnEvt = new PtConnectionEvent(PtEvent::CALL_INVALID,
           0,
           "terminalName",
           "callId",
           0, 0);

        pTempTermConnEvt_1 = new PtConnectionEvent(*pTempTermConnEvt);
        delete pTempTermConnEvt;
        delete pTempTermConnEvt_1;
    }

    void testManipulators()
    {
        PtConnectionEvent* pTempTermConnEvt;
        PtConnectionEvent* pTempTermConnEvt_1;

        pTempTermConnEvt = new PtConnectionEvent(PtEvent::CALL_INVALID,
           0,
           "terminalName",
           "callId",
           0, 0);

        pTempTermConnEvt_1 = new PtConnectionEvent(PtEvent::PROVIDER_IN_SERVICE,
           0,
           "terminalName_1",
           "callId_1",
           0, 0);
        *pTempTermConnEvt_1 = *pTempTermConnEvt;
        delete pTempTermConnEvt;
        delete pTempTermConnEvt_1;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(PtConnectionEventTest);

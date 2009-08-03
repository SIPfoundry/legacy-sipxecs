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

#include <ptapi/PtEvent.h>

/**
 * Unittest for PtEvent
 */
class PtEventTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(PtEventTest);
    CPPUNIT_TEST(testCreators);
    CPPUNIT_TEST(testManipulators);
    CPPUNIT_TEST(testAccessors);
    CPPUNIT_TEST_SUITE_END();


public:
    void testCreators()
    {
        PtEvent*             pTempPtEvent;
        PtEvent*             pTempPtEvent_1;

        // test the default constructor (if implemented)
        pTempPtEvent = new PtEvent();
        delete pTempPtEvent;

        pTempPtEvent = new PtEvent(PtEvent::CALL_INVALID);
        PtEvent::PtEventId id;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get event id", PT_SUCCESS, pTempPtEvent->getId(id));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get correct id", PtEvent::CALL_INVALID, id);
        delete pTempPtEvent;

        pTempPtEvent = new PtEvent(PtEvent::CALL_INVALID);
        pTempPtEvent_1 = new PtEvent(*pTempPtEvent);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get event id", PT_SUCCESS, pTempPtEvent_1->getId(id));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get correct id", PtEvent::CALL_INVALID, id);
        delete pTempPtEvent;
        delete pTempPtEvent_1;
    }

    void testManipulators()
    {
        PtEvent*       pTempPtEvent;
        PtEvent*       pTempPtEvent_1;

        pTempPtEvent = new PtEvent(PtEvent::CALL_INVALID);
        pTempPtEvent_1 = new PtEvent();
        *pTempPtEvent_1 = *pTempPtEvent;
        PtEvent::PtEventId id;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get event id", PT_SUCCESS, pTempPtEvent->getId(id));
        PtEvent::PtEventId id_1;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get event id", PT_SUCCESS, pTempPtEvent_1->getId(id_1));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get correct id", id, id_1);
        delete pTempPtEvent;
        delete pTempPtEvent_1;
    }

    void testAccessors()
    {
        PtEvent*     pTempPtEvent;
        PtEvent::PtEventId*   pTempPtEventId;

        pTempPtEventId = new PtEvent::PtEventId(PtEvent::CALL_INVALID);
        pTempPtEvent = new PtEvent(PtEvent::PROVIDER_IN_SERVICE);
        pTempPtEvent->getId(*pTempPtEventId);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("event id accessor", PtEvent::PROVIDER_IN_SERVICE,
                *pTempPtEventId);
        delete pTempPtEventId;
        delete pTempPtEvent;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(PtEventTest);

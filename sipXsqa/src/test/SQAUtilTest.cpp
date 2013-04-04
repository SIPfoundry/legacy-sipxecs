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

#include "sqa/SQAUtils.h"

class SQAUtilTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(SQAUtilTest);

//    CPPUNIT_TEST(SQAClientTypeTest);
//    CPPUNIT_TEST(getClientStrTest);
//    CPPUNIT_TEST(getConnectionEventStrTest);
//    CPPUNIT_TEST(generateRecordIdTest);
//    CPPUNIT_TEST(generateZmqEventIdTest);
//    CPPUNIT_TEST(generateIdValidateIdTest);
//    CPPUNIT_TEST(validateIdHexComponentTest);

    CPPUNIT_TEST_SUITE_END();


public:
    void SQAClientTypeTest()
    {
      CPPUNIT_ASSERT(!SQAUtil::isPublisher(SQAUtil::SQAClientUnknown));
      CPPUNIT_ASSERT(SQAUtil::isPublisher(SQAUtil::SQAClientPublisher));
      CPPUNIT_ASSERT(SQAUtil::isPublisher(SQAUtil::SQAClientDealer));
      CPPUNIT_ASSERT(!SQAUtil::isPublisher(SQAUtil::SQAClientWatcher));
      CPPUNIT_ASSERT(!SQAUtil::isPublisher(SQAUtil::SQAClientWorker));

      CPPUNIT_ASSERT(!SQAUtil::isPublisherOnly(SQAUtil::SQAClientUnknown));
      CPPUNIT_ASSERT(SQAUtil::isPublisherOnly(SQAUtil::SQAClientPublisher));
      CPPUNIT_ASSERT(!SQAUtil::isPublisherOnly(SQAUtil::SQAClientDealer));
      CPPUNIT_ASSERT(!SQAUtil::isPublisherOnly(SQAUtil::SQAClientWatcher));
      CPPUNIT_ASSERT(!SQAUtil::isPublisherOnly(SQAUtil::SQAClientWorker));

      CPPUNIT_ASSERT(!SQAUtil::isDealer(SQAUtil::SQAClientUnknown));
      CPPUNIT_ASSERT(!SQAUtil::isDealer(SQAUtil::SQAClientPublisher));
      CPPUNIT_ASSERT(SQAUtil::isDealer(SQAUtil::SQAClientDealer));
      CPPUNIT_ASSERT(!SQAUtil::isDealer(SQAUtil::SQAClientWatcher));
      CPPUNIT_ASSERT(!SQAUtil::isDealer(SQAUtil::SQAClientWorker));

      CPPUNIT_ASSERT(!SQAUtil::isWatcher(SQAUtil::SQAClientUnknown));
      CPPUNIT_ASSERT(!SQAUtil::isWatcher(SQAUtil::SQAClientPublisher));
      CPPUNIT_ASSERT(!SQAUtil::isWatcher(SQAUtil::SQAClientDealer));
      CPPUNIT_ASSERT(SQAUtil::isWatcher(SQAUtil::SQAClientWatcher));
      CPPUNIT_ASSERT(SQAUtil::isWatcher(SQAUtil::SQAClientWorker));

      CPPUNIT_ASSERT(!SQAUtil::isWatcherOnly(SQAUtil::SQAClientUnknown));
      CPPUNIT_ASSERT(!SQAUtil::isWatcherOnly(SQAUtil::SQAClientPublisher));
      CPPUNIT_ASSERT(!SQAUtil::isWatcherOnly(SQAUtil::SQAClientDealer));
      CPPUNIT_ASSERT(SQAUtil::isWatcherOnly(SQAUtil::SQAClientWatcher));
      CPPUNIT_ASSERT(!SQAUtil::isWatcherOnly(SQAUtil::SQAClientWorker));

      CPPUNIT_ASSERT(!SQAUtil::isWorker(SQAUtil::SQAClientUnknown));
      CPPUNIT_ASSERT(!SQAUtil::isWorker(SQAUtil::SQAClientPublisher));
      CPPUNIT_ASSERT(!SQAUtil::isWorker(SQAUtil::SQAClientDealer));
      CPPUNIT_ASSERT(!SQAUtil::isWorker(SQAUtil::SQAClientWatcher));
      CPPUNIT_ASSERT(SQAUtil::isWorker(SQAUtil::SQAClientWorker));
    }

    void getClientStrTest()
    {

      CPPUNIT_ASSERT(0 == strcmp("unknown", SQAUtil::getClientStr(SQAUtil::SQAClientUnknown)));
      CPPUNIT_ASSERT(0 == strcmp("publisher", SQAUtil::getClientStr(SQAUtil::SQAClientPublisher)));
      CPPUNIT_ASSERT(0 == strcmp("dealer", SQAUtil::getClientStr(SQAUtil::SQAClientDealer)));
      CPPUNIT_ASSERT(0 == strcmp("watcher", SQAUtil::getClientStr(SQAUtil::SQAClientWatcher)));
      CPPUNIT_ASSERT(0 == strcmp("worker", SQAUtil::getClientStr(SQAUtil::SQAClientWorker)));
    }

    void getConnectionEventStrTest()
    {
      CPPUNIT_ASSERT(0 == strcmp("unknown", SQAUtil::getConnectionEventStr(ConnectionEventUnknown)));
      CPPUNIT_ASSERT(0 == strcmp("established", SQAUtil::getConnectionEventStr(ConnectionEventEstablished)));
      CPPUNIT_ASSERT(0 == strcmp("sigin", SQAUtil::getConnectionEventStr(ConnectionEventSignin)));
      CPPUNIT_ASSERT(0 == strcmp("keepalive", SQAUtil::getConnectionEventStr(ConnectionEventKeepAlive)));
      CPPUNIT_ASSERT(0 == strcmp("logout", SQAUtil::getConnectionEventStr(ConnectionEventLogout)));
      CPPUNIT_ASSERT(0 == strcmp("terminate", SQAUtil::getConnectionEventStr(ConnectionEventTerminate)));
      CPPUNIT_ASSERT(0 == strcmp("unknown", SQAUtil::getConnectionEventStr(ConnectionEventNum)));
    }

    void generateRecordIdTest()
    {
      std::string recordId;
      std::string recordStart = PublisherWatcherPrefix;
      recordStart += ".connection.";

      SQAUtil::generateRecordId(recordId, ConnectionEventUnknown);
      CPPUNIT_ASSERT(boost::starts_with(recordId, recordStart));
      CPPUNIT_ASSERT(boost::ends_with(recordId, SQAUtil::getConnectionEventStr(ConnectionEventUnknown)));

      SQAUtil::generateRecordId(recordId, ConnectionEventEstablished);
      CPPUNIT_ASSERT(boost::starts_with(recordId, recordStart));
      CPPUNIT_ASSERT(boost::ends_with(recordId, SQAUtil::getConnectionEventStr(ConnectionEventEstablished)));

      SQAUtil::generateRecordId(recordId, ConnectionEventSignin);
      CPPUNIT_ASSERT(boost::starts_with(recordId, recordStart));
      CPPUNIT_ASSERT(boost::ends_with(recordId, SQAUtil::getConnectionEventStr(ConnectionEventSignin)));

      SQAUtil::generateRecordId(recordId, ConnectionEventKeepAlive);
      CPPUNIT_ASSERT(boost::starts_with(recordId, recordStart));
      CPPUNIT_ASSERT(boost::ends_with(recordId, SQAUtil::getConnectionEventStr(ConnectionEventKeepAlive)));

      SQAUtil::generateRecordId(recordId, ConnectionEventLogout);
      CPPUNIT_ASSERT(boost::starts_with(recordId, recordStart));
      CPPUNIT_ASSERT(boost::ends_with(recordId, SQAUtil::getConnectionEventStr(ConnectionEventLogout)));

      SQAUtil::generateRecordId(recordId, ConnectionEventTerminate);
      CPPUNIT_ASSERT(boost::starts_with(recordId, recordStart));
      CPPUNIT_ASSERT(boost::ends_with(recordId, SQAUtil::getConnectionEventStr(ConnectionEventTerminate)));
    }

    void generateIdValidateIdTest()
    {
      std::string id;
      CPPUNIT_ASSERT(SQAUtil::generateId(id, SQAUtil::SQAClientPublisher, "reg"));
      CPPUNIT_ASSERT(boost::starts_with(id, "sqw.reg"));
      CPPUNIT_ASSERT(SQAUtil::validateId(id, SQAUtil::SQAClientPublisher, "reg"));

      CPPUNIT_ASSERT(SQAUtil::generateId(id, SQAUtil::SQAClientDealer, "reg"));
      CPPUNIT_ASSERT(boost::starts_with(id, "sqa.reg"));
      CPPUNIT_ASSERT(SQAUtil::validateId(id, SQAUtil::SQAClientDealer, "reg"));

      CPPUNIT_ASSERT(SQAUtil::generateId(id, SQAUtil::SQAClientWatcher, "reg"));
      CPPUNIT_ASSERT(boost::starts_with(id, "sqw.reg"));
      CPPUNIT_ASSERT(SQAUtil::validateId(id, SQAUtil::SQAClientWatcher, "reg"));

      CPPUNIT_ASSERT(SQAUtil::generateId(id, SQAUtil::SQAClientWorker, "reg"));
      CPPUNIT_ASSERT(boost::starts_with(id, "sqa.reg"));
      CPPUNIT_ASSERT(SQAUtil::validateId(id, SQAUtil::SQAClientWorker, "reg"));

      CPPUNIT_ASSERT(!SQAUtil::generateId(id, SQAUtil::SQAClientUnknown, "reg"));
      CPPUNIT_ASSERT(!SQAUtil::validateId(id, SQAUtil::SQAClientUnknown, "reg"));

      CPPUNIT_ASSERT(!SQAUtil::validateId("", SQAUtil::SQAClientPublisher, "reg"));
      CPPUNIT_ASSERT(!SQAUtil::validateId("sq.reg.1111-1111", SQAUtil::SQAClientPublisher, "reg"));
      CPPUNIT_ASSERT(!SQAUtil::validateId("sqw.reg", SQAUtil::SQAClientPublisher, "reg"));
      CPPUNIT_ASSERT(!SQAUtil::validateId("sqw.reg1111-1111", SQAUtil::SQAClientPublisher, "reg"));

      CPPUNIT_ASSERT(!SQAUtil::validateId("sqa.reg.1111-1111", SQAUtil::SQAClientPublisher, "reg"));
      CPPUNIT_ASSERT(!SQAUtil::validateId("sqa.reg.1111-1111", SQAUtil::SQAClientWatcher, "reg"));
      CPPUNIT_ASSERT(!SQAUtil::validateId("sqw.reg.1111-1111", SQAUtil::SQAClientDealer, "reg"));
      CPPUNIT_ASSERT(!SQAUtil::validateId("sqw.reg.1111-1111", SQAUtil::SQAClientWorker, "reg"));

      CPPUNIT_ASSERT(!SQAUtil::validateId("sqw.reg.111-1111", SQAUtil::SQAClientPublisher, "reg"));
      CPPUNIT_ASSERT(!SQAUtil::validateId("sqw.reg.1111-111", SQAUtil::SQAClientWatcher, "reg"));
      CPPUNIT_ASSERT(!SQAUtil::validateId("sqa.reg.111-1111", SQAUtil::SQAClientDealer, "reg"));
      CPPUNIT_ASSERT(!SQAUtil::validateId("sqa.reg.1111-111", SQAUtil::SQAClientWorker, "reg"));
    }

    //TODO: DO Test for this variant too
    // static bool validateId(const std::string &id, int clientType);

    void validateIdHexComponentTest()
    {
      CPPUNIT_ASSERT(SQAUtil::validateIdHexComponent("1234"));
      CPPUNIT_ASSERT(SQAUtil::validateIdHexComponent("12D3"));
      CPPUNIT_ASSERT(SQAUtil::validateIdHexComponent("AABB"));
      CPPUNIT_ASSERT(!SQAUtil::validateIdHexComponent(""));
      CPPUNIT_ASSERT(!SQAUtil::validateIdHexComponent("..."));
      CPPUNIT_ASSERT(!SQAUtil::validateIdHexComponent("a2D3"));
      CPPUNIT_ASSERT(!SQAUtil::validateIdHexComponent("123"));
      CPPUNIT_ASSERT(!SQAUtil::validateIdHexComponent("12D322"));
      CPPUNIT_ASSERT(!SQAUtil::validateIdHexComponent("12322"));
    }

    void generateZmqEventIdTest()
    {
      std::string zmqEventId;
      std::string eventId = "reg";

      CPPUNIT_ASSERT(SQAUtil::generateZmqEventId(zmqEventId, SQAUtil::SQAClientWatcher, eventId));
      CPPUNIT_ASSERT(0 == strcmp("sqw.reg", zmqEventId.data()));

      CPPUNIT_ASSERT(SQAUtil::generateZmqEventId(zmqEventId, SQAUtil::SQAClientWorker, eventId));
      CPPUNIT_ASSERT(0 == strcmp("sqa.reg", zmqEventId.data()));

      CPPUNIT_ASSERT(!SQAUtil::generateZmqEventId(zmqEventId, SQAUtil::SQAClientPublisher, eventId));
      CPPUNIT_ASSERT(!SQAUtil::generateZmqEventId(zmqEventId, SQAUtil::SQAClientDealer, eventId));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SQAUtilTest);

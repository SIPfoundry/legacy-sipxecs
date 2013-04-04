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

#include "SQAAgentUtil.h"

class StateQueueMessageTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(StateQueueMessageTest);
//    CPPUNIT_TEST(StateQueueMessage_Constructor2Test);
//    CPPUNIT_TEST(StateQueueMessage_Constructor3Test);
//    CPPUNIT_TEST(StateQueueMessage_setgetServiceTypeTest);
    CPPUNIT_TEST_SUITE_END();

public:

    void checkConstructor2(int messageType, int serviceType)
    {
      StateQueueMessage request((StateQueueMessage::Type)messageType, serviceType);

      CPPUNIT_ASSERT(messageType == request.getType());
      CPPUNIT_ASSERT(serviceType == request.getServiceType());
      std::string messageId;
      request.get("message-id", messageId);
      CPPUNIT_ASSERT(SQAUtil::validateId(messageId, serviceType));
    }

    void checkConstructor3(int messageType, int serviceType, std::string& eventId)
    {
      StateQueueMessage request((StateQueueMessage::Type)messageType, serviceType, eventId);

      CPPUNIT_ASSERT(messageType == request.getType());
      CPPUNIT_ASSERT(serviceType == request.getServiceType());
      std::string messageId;
      request.get("message-id", messageId);
      CPPUNIT_ASSERT(SQAUtil::validateId(messageId, serviceType, eventId));
    }

    // Test for constructor StateQueueMessage(Type type, int serviceType);
    void StateQueueMessage_Constructor2Test()
    {
      for (int messageType = 1; messageType < StateQueueMessage::NumType; messageType++)
      {
        checkConstructor2(messageType, SQAUtil::SQAClientPublisher);
        checkConstructor2(messageType, SQAUtil::SQAClientDealer);
        checkConstructor2(messageType, SQAUtil::SQAClientWatcher);
        checkConstructor2(messageType, SQAUtil::SQAClientWorker);
      }
    }

    // Test for constructor StateQueueMessage(Type type, int serviceType, const std::string &eventId);
    void StateQueueMessage_Constructor3Test()
    {
      std::string eventId = "reg";
      for (int messageType = 1; messageType < StateQueueMessage::NumType; messageType++)
      {
        checkConstructor3(messageType, SQAUtil::SQAClientPublisher, eventId);
        checkConstructor3(messageType, SQAUtil::SQAClientDealer, eventId);
        checkConstructor3(messageType, SQAUtil::SQAClientWatcher, eventId);
        checkConstructor3(messageType, SQAUtil::SQAClientWorker, eventId);
      }
    }

    //Test for setServiceTypeTest and getServiceTypeTest
    void StateQueueMessage_setgetServiceTypeTest()
    {
      StateQueueMessage request1(StateQueueMessage::Signin);
      request1.setServiceType(SQAUtil::SQAClientPublisher);
      CPPUNIT_ASSERT(SQAUtil::SQAClientPublisher == request1.getServiceType());

      StateQueueMessage request2(StateQueueMessage::Signin);
      request2.setServiceType(SQAUtil::SQAClientDealer);
      CPPUNIT_ASSERT(SQAUtil::SQAClientDealer == request2.getServiceType());

      StateQueueMessage request3(StateQueueMessage::Signin);
      request3.setServiceType(SQAUtil::SQAClientWatcher);
      CPPUNIT_ASSERT(SQAUtil::SQAClientWatcher == request3.getServiceType());

      StateQueueMessage request4(StateQueueMessage::Signin);
      request4.setServiceType(SQAUtil::SQAClientWorker);
      CPPUNIT_ASSERT(SQAUtil::SQAClientWorker == request4.getServiceType());

    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(StateQueueMessageTest);

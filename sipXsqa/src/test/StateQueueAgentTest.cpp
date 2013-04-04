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
#include "SQAClientUtil.h"

class StateQueueAgentTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(StateQueueAgentTest);
    CPPUNIT_TEST(StateQueueAgent_handleSigninLogoutOKTest);
    CPPUNIT_TEST(StateQueueAgent_handleSigninFailTest);
    CPPUNIT_TEST(StateQueueAgent_handleLogoutFailTest);
    CPPUNIT_TEST(StateQueueAgent_handlePingPongOKTest);
    CPPUNIT_TEST(StateQueueAgent_handleEnqueueTest);
    CPPUNIT_TEST(StateQueueAgent_handleEnqueueFailTest);
    CPPUNIT_TEST(StateQueueAgent_handlePublishOKTest);
    CPPUNIT_TEST(StateQueueAgent_handlePublishFailTest);
    CPPUNIT_TEST(StateQueueAgent_handlePopTest);
    CPPUNIT_TEST(StateQueueAgent_handlePopFailTest);
    CPPUNIT_TEST(StateQueueAgent_handleUnknownRequestTest);
    CPPUNIT_TEST_SUITE_END();

    /// Helper class overloading functionality of publisher to be able to
    /// check what an agent is publishing
    class StateQueuePublisherSimple : public StateQueuePublisher
    {
    public:
      StateQueuePublisherSimple(StateQueueAgent *pAgent) : StateQueuePublisher(pAgent) {}

      ~StateQueuePublisherSimple() {}

      void publish(const StateQueueRecord& record)
      {
        if (_hasRecord)
        {
          CPPUNIT_ASSERT(_record.id == record.id);
          CPPUNIT_ASSERT(_record.data == record.data);

          CPPUNIT_ASSERT(_record.exclude.size() == record.exclude.size());
          CPPUNIT_ASSERT(std::equal(_record.exclude.begin(), _record.exclude.end(), record.exclude.begin()));

          CPPUNIT_ASSERT(_record.retry = record.retry);
          CPPUNIT_ASSERT(_record.expires = record.expires);
          CPPUNIT_ASSERT(_record.watcherData == record.watcherData);
        }
        else if (_hasData)
        {
          CPPUNIT_ASSERT(_data == record.data);
        }
        else
        {
          CPPUNIT_ASSERT(false);
        }
      }

      void addSubscriber(const std::string& ev, const std::string& applicationId, int expires)
      {
        CPPUNIT_ASSERT(_subscriberEvent == ev);
        CPPUNIT_ASSERT(_subscriberApplicationId == applicationId);
      }
      void removeSubscriber(const std::string& ev, const std::string& applicationId)
      {
        CPPUNIT_ASSERT(_subscriberEvent == ev);
        CPPUNIT_ASSERT(_subscriberApplicationId == applicationId);
      }

      void expectPublish(StateQueueRecord& record)
      {
        _record = record;
        _hasRecord = true;
      }

      void expectPublishedData(const std::string& data)
      {
        _data = data;
        _hasData = true;
      }

      void expectSubscriber(const std::string& ev, const std::string& applicationId)
      {
        _subscriberEvent = ev;
        _subscriberApplicationId = applicationId;
      }

      void clearExpected()
      {
        _record.id = "";
        _record.data = "";
        _record.exclude.clear();
        _record.retry = 0;
        _record.expires = 0;
        _record.watcherData = false;
        _hasRecord = false;

        _data = "";
        _hasData = false;

        _subscriberEvent = "";
        _subscriberApplicationId = "";

      }

      StateQueueRecord _record;
      bool _hasRecord;

      std::string _data;
      bool _hasData;

      std::string _subscriberEvent;
      std::string _subscriberApplicationId;
    };

public:
    void setUp()
    {
      _util.setProgram("/usr/local/sipxecs-local-feature-branch/bin/sipxsqa");
      //prepare one local sqa agent data (no ha)
      _util.generateSQAAgentData(_agentsData, 1, false);

      // prepare publisher for events of type "reg"
      _clientPData = new SQAClientData(0, SQAUtil::SQAClientPublisher, "Publisher",_agentsData[0]->sqaControlAddress,_agentsData[0]->sqaControlAddressAll,_agentsData[0]->sqaControlPort, "reg", 1, SQA_CONN_READ_TIMEOUT, SQA_CONN_WRITE_TIMEOUT, 2, 1000, SQA_FALLBACK_TIMEOUT);
      _clientPublisher = new StateQueueClient(_clientPData->_type, _clientPData->_applicationId, _clientPData->_servicesAddresses, _clientPData->_servicePort, _clientPData->_zmqEventId, _clientPData->_poolSize, _clientPData->_readTimeout, _clientPData->_writeTimeout, _clientPData->_keepAliveTimeout, _clientPData->_signinTimeout);
      // cancel all timers so that signin and ping can be manipulate manually
      _clientPublisher->_core->_signinTimer->cancel();
      _clientPublisher->_core->_keepAliveTimer->cancel();
      _clientPublisher->_core->_isAlive = true;

      //same for a worker client
      _clientWData = new SQAClientData(0, SQAUtil::SQAClientWorker, "Worker",_agentsData[0]->sqaControlAddress,_agentsData[0]->sqaControlAddressAll,_agentsData[0]->sqaControlPort, "reg", 1, SQA_CONN_READ_TIMEOUT, SQA_CONN_WRITE_TIMEOUT, 2, 1000, SQA_FALLBACK_TIMEOUT);
      _clientWorker = new StateQueueClient(_clientWData->_type, _clientWData->_applicationId, _clientWData->_servicesAddresses, _clientWData->_servicePort, _clientPData->_zmqEventId, _clientPData->_poolSize, _clientPData->_readTimeout, _clientPData->_writeTimeout, _clientPData->_keepAliveTimeout, _clientPData->_signinTimeout);
      _clientWorker->_core->_signinTimer->cancel();
      _clientWorker->_core->_keepAliveTimer->cancel();
      _clientWorker->_core->_isAlive = true;

      //start agent
      CPPUNIT_ASSERT(_util.startSQAAgent(_agentsData[0]));
      //give agent time to start
      boost::this_thread::sleep(boost::posix_time::milliseconds(3000));

      //set the overloaded publisher
      _fakeAgent = _agentsData[0]->agent;
      _publisher = new StateQueuePublisherSimple(_fakeAgent);
      _fakeAgent->setPublisher(_publisher);
    }

    void tearDown()
    {
      // release and clean everything
      _clientPublisher->terminate();
      _clientWorker->terminate();

      std::vector<SQAAgentData::Ptr>::iterator it;
      for (it = _agentsData.begin(); it != _agentsData.end(); it++)
      {
        SQAAgentData::Ptr data = *it;

        _util.stopSQAAgent(data);
      }

      delete  _clientPublisher;
      delete _clientPData;

      delete  _clientWorker;
      delete _clientWData;

    }

    // Check that signin and logout work as expected in positive scenario
    void checkSigninLogoutOKTest(StateQueueClient* client)
    {
      // construct a signin request based on the _clientPublisher
      StateQueueMessage signinRequest(StateQueueMessage::Signin, client->_type);
      signinRequest.set("message-app-id", client->_applicationId.c_str());
      signinRequest.set("subscription-expires", client->_core->_signinTimeout);
      signinRequest.set("subscription-event", client->_zmqEventId.c_str());
      std::string clientType = SQAUtil::getClientStr(client->_type);
      signinRequest.set("service-type", clientType);

      if (SQAUtil::isWorker(client->_type))
      {
        // expect a subscribe only for worker client
        _publisher->expectSubscriber(_clientPublisher->_zmqEventId, _clientPublisher->_applicationId);
      }
      else
      {
        // expect no subscribe only for publisher client
        // TEST: This would cause test to fail in case a subscriber is added
        _publisher->expectSubscriber("none", "none");
      }

      std::string expectedPublishedData = client->_applicationId;
      expectedPublishedData += "|";
      expectedPublishedData += _agentsData[0]->sqaControlAddress;
      // expect a signin event to be published with client data
      _publisher->expectPublishedData(expectedPublishedData);

      StateQueueMessage signinResponse;
      // TEST: send/recv signin message should succeed
      CPPUNIT_ASSERT(client->_core->sendAndReceive(signinRequest, signinResponse));

      std::string requestMessageId;
      std::string responseMessageId;
      signinResponse.get("message-id", responseMessageId);
      signinRequest.get("message-id", requestMessageId);
      // TEST: response should have the same message id
      CPPUNIT_ASSERT(responseMessageId == requestMessageId);
      std::string response;
      // TEST: response should be "ok"
      signinResponse.get("message-response", response);
      CPPUNIT_ASSERT(response == "ok");
      // TEST: response type should be the same
      CPPUNIT_ASSERT(StateQueueMessage::Signin == signinResponse.getType());
      std::string data;
      std::string publisherAddress = "tcp://"; publisherAddress += _agentsData[0]->sqaZmqSubscriptionAddress; publisherAddress +=":"; publisherAddress +=_agentsData[0]->sqaZmqSubscriptionPort;
      signinResponse.get("message-data", data);
      // TEST: message data should contain the publisher address
      CPPUNIT_ASSERT(data == publisherAddress);

      // construct a logout request for the previous signin
      StateQueueMessage logoutRequest(StateQueueMessage::Logout, client->_type);
      logoutRequest.set("message-app-id", client->_applicationId.c_str());
      logoutRequest.set("subscription-event", client->_zmqEventId.c_str());
      logoutRequest.set("service-type", clientType);

      StateQueueMessage logoutResponse;
      // TEST: send/recv logout message should succeed
      CPPUNIT_ASSERT(client->_core->sendAndReceive(logoutRequest, logoutResponse));

      logoutResponse.get("message-id", responseMessageId);
      logoutRequest.get("message-id", requestMessageId);
      // TEST: response should have the same message id
      CPPUNIT_ASSERT(responseMessageId == requestMessageId);
      logoutResponse.get("message-response", response);
      // TEST: response should be "ok"
      CPPUNIT_ASSERT(response == "ok");
      logoutResponse.get("message-data", data);
      // TEST: data should be bfn?
      CPPUNIT_ASSERT(data == "bfn!");
      // TEST: response type should be the same
      CPPUNIT_ASSERT(StateQueueMessage::Logout == logoutResponse.getType());
    }

    // Check that signin and logout work as expected in positive scenario
    void StateQueueAgent_handleSigninLogoutOKTest()
    {
      //TEST: Signin/logout ok for publisher
      checkSigninLogoutOKTest(_clientPublisher);
      //TEST: Signin/logout ok for worker
      checkSigninLogoutOKTest(_clientWorker);
    }

    // Helper method to check that an agent responde with expected message error when sending an invalid request
    void checkHandleRequestError(StateQueueMessage& request, const std::string& error)
    {
      StateQueueMessage response;
      //TEST: sending the request should succeed and receiving an error response
      CPPUNIT_ASSERT(_clientPublisher->_core->sendAndReceive(request, response));

      std::string messageResponse;
      response.get("message-response", messageResponse);
      //TEST: response should always be error
      CPPUNIT_ASSERT(messageResponse == "error");

      std::string messageError;
      response.get("message-error", messageError);
      //TEST: the error message should be as expected
      CPPUNIT_ASSERT(messageError == error);
    }

    // TEST: Agent responds with error for an incomplete signin request
    void StateQueueAgent_handleSigninFailTest()
    {
      //TEST: no subscriber should be added to publisher
      _publisher->expectSubscriber("none", "none");

      // construct a signin request based on the _clientPublisher
      StateQueueMessage request(StateQueueMessage::Signin, _clientPublisher->_type);
      checkHandleRequestError(request, "Missing required argument message-app-id.");

      request.set("message-app-id", _clientPublisher->_applicationId.c_str());
      checkHandleRequestError(request, "Missing required argument subscription-expires.");

      request.set("subscription-expires", _clientPublisher->_core->_signinTimeout);
      checkHandleRequestError(request, "Missing required argument subscription-event.");

      request.set("subscription-event", _clientPublisher->_zmqEventId.c_str());
      checkHandleRequestError(request, "Missing required argument service-type.");
    }

    // TEST: Agent responds with error for an incomplete logout request
    void StateQueueAgent_handleLogoutFailTest()
    {
      _publisher->expectSubscriber("none", "none");

      // construct a signin request based on the _clientPublisher
      StateQueueMessage request(StateQueueMessage::Logout, _clientPublisher->_type);
      checkHandleRequestError(request, "Missing required argument message-app-id.");

      request.set("message-app-id", _clientPublisher->_applicationId.c_str());
      checkHandleRequestError(request, "Missing required argument subscription-event.");

      request.set("subscription-event", _clientPublisher->_zmqEventId.c_str());
      checkHandleRequestError(request, "Missing required argument service-type.");
    }

    // Check that agent responds with a pong response to a ping request
    void checkPingPongOKTest(StateQueueClient* client)
    {
      // construct a ping request based on the _clientPublisher
      StateQueueMessage request(StateQueueMessage::Ping, client->_type);
      request.set("message-app-id", client->_applicationId.c_str());

      // expect agent to publish some data about the pinger
      std::string expectedPublishedData = client->_applicationId;
      expectedPublishedData += "|";
      expectedPublishedData += _agentsData[0]->sqaControlAddress;
      _publisher->expectPublishedData(expectedPublishedData);

      StateQueueMessage response;
      //TEST: send ping recv pong should be ok
      CPPUNIT_ASSERT(client->_core->sendAndReceive(request, response));
      //TEST: a response to ping should always be pong
      CPPUNIT_ASSERT(StateQueueMessage::Pong == response.getType());
    }

    void StateQueueAgent_handlePingPongOKTest()
    {
      //check ping pong for a publisher
      checkPingPongOKTest(_clientPublisher);
      //check ping pong for a worker
      checkPingPongOKTest(_clientWorker);
    }


    void StateQueueAgent_handleEnqueueTest()
    {
      CPPUNIT_FAIL("Not implemented yet!");
    }

    void StateQueueAgent_handleEnqueueFailTest()
    {
      CPPUNIT_FAIL("Not implemented yet!");
    }

    // check that agent handles ok a publish request
    void checkhandlePublishOKTest(StateQueueClient* client)
    {
      //construct a regular publish request
      StateQueueMessage request(StateQueueMessage::Publish);
      std::string messageId;
      std::string eventId = "reg";
      SQAUtil::generateId(messageId, client->_type, eventId);
      request.set("message-id", messageId.c_str());
      request.set("message-app-id", client->_applicationId.c_str());
      std::string data = "data";
      request.set("message-data", data);

      // TEST: publisher should expect no subscriber to be added (this will fail otherwise)
      _publisher->expectSubscriber("none", "none");

      StateQueueRecord record;
      _fakeAgent->fillEventRecord(record, messageId, data, true);
      // TEST: What to expect from agent to publish
      _publisher->expectPublish(record);

      StateQueueMessage response;
      // TEST: send/recv should work
      CPPUNIT_ASSERT(client->_core->sendAndReceive(request, response));

      std::string msgResponse;
      response.get("message-response", msgResponse);
      //TEST: response should be ok
      CPPUNIT_ASSERT(msgResponse == "ok");
      // TEST: message response type should be publish
      CPPUNIT_ASSERT(StateQueueMessage::Publish == response.getType());

      // TEST: sending with no response should work
      request.set("noresponse", true);
      CPPUNIT_ASSERT(client->_core->sendNoResponse(request));
    }

    void StateQueueAgent_handlePublishOKTest()
    {
      checkhandlePublishOKTest(_clientPublisher);
    }

    void StateQueueAgent_handlePublishFailTest()
    {
      _publisher->expectSubscriber("none", "none");

      // construct a signin request based on the _clientPublisher
      StateQueueMessage request(StateQueueMessage::Publish);
      checkHandleRequestError(request, "Missing required argument message-app-id.");

      request.set("message-app-id", _clientPublisher->_applicationId.c_str());
      checkHandleRequestError(request, "Missing required argument message-id.");

      std::string messageId;
      std::string eventId = "reg";
      SQAUtil::generateId(messageId, _clientPublisher->_type, eventId);
      request.set("message-id", messageId.c_str());

      checkHandleRequestError(request, "Missing required argument message-data.");
    }

    void StateQueueAgent_handlePopTest()
    {
      CPPUNIT_FAIL("Not implemented yet!");
    }

    void StateQueueAgent_handlePopFailTest()
    {
      CPPUNIT_FAIL("Not implemented yet!");
    }

    // Test that an agent discards unknown requests with an error response
    void StateQueueAgent_handleUnknownRequestTest()
    {
      //construct an unknown request
      StateQueueMessage request(StateQueueMessage::Unknown, _clientPublisher->_type);
      request.set("message-app-id", _clientPublisher->_applicationId.c_str());

      //TEST: expect no subscribers to be added and no published data
      _publisher->expectSubscriber("none", "none");
      _publisher->expectPublishedData("none");

      checkHandleRequestError(request, "Invalid Command!");
    }


    StateQueueConnection* _conn;
    StateQueuePublisherSimple* _publisher;
    StateQueueAgent *_fakeAgent;

    std::vector<SQAAgentData::Ptr> _agentsData;
    SQAAgentUtil _util;
    SQAClientData* _clientPData;
    StateQueueClient* _clientPublisher;
    SQAClientData* _clientWData;
    StateQueueClient* _clientWorker;
};

CPPUNIT_TEST_SUITE_REGISTRATION(StateQueueAgentTest);

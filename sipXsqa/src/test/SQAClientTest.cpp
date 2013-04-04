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

#include <iostream>     // std::cout
#include <algorithm>    // std::mismatch
#include <vector>       // std::vector
#include <utility>      // std::pair

class SQAClientTest : public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE(SQAClientTest);
//  CPPUNIT_TEST(StateQueueClient_clientConnectivityAgentUpDownTest);
//  CPPUNIT_TEST(StateQueueClient_publishWatchOKTest);
//  CPPUNIT_TEST(StateQueueClient_publishNoSQAAgentTest);
//  CPPUNIT_TEST(StateQueueClient_publishRestrictionToNonPublishersTest);
//  CPPUNIT_TEST(StateQueueClient_enqueuePopOKTest);
//  CPPUNIT_TEST(StateQueueClient_enqueueNoSQAAgentTest);
//  CPPUNIT_TEST(StateQueueClient_enqueueRestrictionToNonDealersTest);
//
//  CPPUNIT_TEST(StateQueueClient_setSQAClientConfigTest);
//  CPPUNIT_TEST(StateQueueClient_getClientOptionsTest);
//  CPPUNIT_TEST(StateQueueClient_pushAddressesTest);
//  CPPUNIT_TEST(StateQueueClient_checkMessageResponseTest);
//
//  CPPUNIT_TEST(StateQueueClient_getIoServiceTest);
//  CPPUNIT_TEST(StateQueueClient_getLocalAddressTest);
//  CPPUNIT_TEST(StateQueueClient_isConnectedTest);
//  CPPUNIT_TEST(StateQueueClient_getClassNameTest);
//  CPPUNIT_TEST(StateQueueClient_getEventQueueTest);
//
//  CPPUNIT_TEST(StateQueueClient_setFallbackServicesTest);
//  CPPUNIT_TEST(StateQueueClient_watchTest);
//  CPPUNIT_TEST(StateQueueClient_publishTest);
//  CPPUNIT_TEST(StateQueueClient_publishAndSetTest);
//  CPPUNIT_TEST(StateQueueClient_eraseTest);
//  CPPUNIT_TEST(StateQueueClient_persistTest);
//  CPPUNIT_TEST(StateQueueClient_setTest);
//  CPPUNIT_TEST(StateQueueClient_msetTest);
//  CPPUNIT_TEST(StateQueueClient_getTest);
//  CPPUNIT_TEST(StateQueueClient_mgetTest);
//  CPPUNIT_TEST(StateQueueClient_mgetmTest);
//  CPPUNIT_TEST(StateQueueClient_mgetiTest);
//  CPPUNIT_TEST(StateQueueClient_removeTest);
//
//  CPPUNIT_TEST(StateQueueClient_startTest);
//  CPPUNIT_TEST(StateQueueClient_tryPrimaryCoreTest);
//  CPPUNIT_TEST(StateQueueClient_trySecondaryCoreTest);
//  CPPUNIT_TEST(StateQueueClient_createFallbackCoreTest);
//  CPPUNIT_TEST(StateQueueClient_checkFallbackTest);
//  CPPUNIT_TEST(StateQueueClient_setFallbackTimerTest);
//  CPPUNIT_TEST(StateQueueClient_fallbackLoopTest);
//  CPPUNIT_TEST(StateQueueClient_popTest);
//  CPPUNIT_TEST(StateQueueClient_enqueueTest);
//  CPPUNIT_TEST(StateQueueClient_internal_publishTest);
//
//  CPPUNIT_TEST(SQAClientCore_getClassNameTest);
//  CPPUNIT_TEST(SQAClientCore_getLocalAddressTest);
//  CPPUNIT_TEST(SQAClientCore_terminateTest);
//  CPPUNIT_TEST(SQAClientCore_isConnectedTest);
//  CPPUNIT_TEST(SQAClientCore_isConnectedNowTest);
//
//  CPPUNIT_TEST(SQAClientCore_setExpiresTest);
//  CPPUNIT_TEST(SQAClientCore_signinTest);
//  CPPUNIT_TEST(SQAClientCore_logoutTest);
//  CPPUNIT_TEST(SQAClientCore_subscribeTest);
//
//  CPPUNIT_TEST(SQAClientCore_sendNoResponseTest);
//  CPPUNIT_TEST(SQAClientCore_sendAndReceiveTest);
//  CPPUNIT_TEST(SQAClientCore_initTest);
//
//
//  CPPUNIT_TEST(SQAClientCore_sendKeepAliveTest);
//  CPPUNIT_TEST(SQAClientCore_setKeepAliveTimerTest);
//  CPPUNIT_TEST(SQAClientCore_keepAliveLoopTest);
//
//  CPPUNIT_TEST(SQAClientCore_setSigninTimerTest);
//  CPPUNIT_TEST(SQAClientCore_signinLoopTest);
//
//  CPPUNIT_TEST(SQAClientCore_eventLoopTest);
//  CPPUNIT_TEST(SQAClientCore_do_popTest);
//  CPPUNIT_TEST(SQAClientCore_do_watchTest);
//  CPPUNIT_TEST(SQAClientCore_readEventTest);
//  CPPUNIT_TEST(SQAClientCore_zmq_freeTest);
//  CPPUNIT_TEST(SQAClientCore_zmq_sendTest);
//  CPPUNIT_TEST(SQAClientCore_zmq_sendmoreTest);
//  CPPUNIT_TEST(SQAClientCore_zmq_receiveTest);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    try{
    _util.setProgram("/usr/local/sipxecs-local-feature-branch/bin/sipxsqa");
    //prepare one local sqa agent data (no ha)
    _util.generateSQAAgentData(_agentsData, 3, true);

    // prepare publisher for events of type "reg"
    _pd = new SQAClientData(0, SQAUtil::SQAClientPublisher, "Publisher",_agentsData[0]->sqaControlAddress,_agentsData[0]->sqaControlAddressAll,_agentsData[0]->sqaControlPort, "reg");
    _publisher = new StateQueueClient(_pd->_type, _pd->_applicationId, _pd->_servicesAddresses, _pd->_servicePort, _pd->_zmqEventId, _pd->_poolSize, _pd->_readTimeout, _pd->_writeTimeout, _pd->_keepAliveTimeout, _pd->_signinTimeout);

    // TEST: Publisher started but cannot connect to agent (agent is not started)
    _pd->setServiceDown();
    SQAClientUtil::checkPublisherAfterStartup(*_publisher, _pd);

    // prepare publisher for events of type "reg"
    _dd = new SQAClientData(0, SQAUtil::SQAClientDealer, "Dealer",_agentsData[0]->sqaControlAddress,_agentsData[0]->sqaControlAddressAll,_agentsData[0]->sqaControlPort, "reg");
    _dealer = new StateQueueClient(_dd->_type, _dd->_applicationId, _dd->_servicesAddresses, _dd->_servicePort, _dd->_zmqEventId, _dd->_poolSize, _pd->_readTimeout, _pd->_writeTimeout, _pd->_keepAliveTimeout, _pd->_signinTimeout);

    // TEST: Publisher started but cannot connect to agent (agent is not started)
    _dd->setServiceDown();
    SQAClientUtil::checkPublisherAfterStartup(*_dealer, _dd);

    // prepare a watcher for events of type "reg"
    _wd = new SQAClientData(0, SQAUtil::SQAClientWatcher, "Watcher", _agentsData[0]->sqaControlAddress, _agentsData[0]->sqaControlAddressAll, _agentsData[0]->sqaControlPort, "reg");
    _wdVec.push_back(_wd);
    _watcher = new StateQueueClient(_wd->_type, _wd->_applicationId, _wd->_servicesAddresses, _wd->_servicePort, _wd->_zmqEventId, _wd->_poolSize, _pd->_readTimeout, _pd->_writeTimeout, _pd->_keepAliveTimeout, _pd->_signinTimeout);

    // TEST: Watcher started but cannot connect to agent (agent is not started yet)
    _wd->setServiceDown();
    SQAClientUtil::checkWatcherAfterStartup(*_watcher, _wdVec);

    // prepare a watcher for events of type "reg"
    _wod = new SQAClientData(0, SQAUtil::SQAClientWorker, "Worker", _agentsData[0]->sqaControlAddress, _agentsData[0]->sqaControlAddressAll, _agentsData[0]->sqaControlPort, "reg");
    _wodVec.push_back(_wod);
    _worker = new StateQueueClient(_wod->_type, _wod->_applicationId, _wod->_servicesAddresses, _wod->_servicePort, _wod->_zmqEventId, _wod->_poolSize, _pd->_readTimeout, _pd->_writeTimeout, _pd->_keepAliveTimeout, _pd->_signinTimeout);

    // TEST: Watcher started but cannot connect to agent (agent is not started yet)
    _wod->setServiceDown();
    SQAClientUtil::checkWatcherAfterStartup(*_worker, _wodVec);
    }catch(std::exception&e){std::cout << e.what()<<std::endl;}
  }

  void tearDown()
  {
    _publisher->terminate();
    SQAClientUtil::checkSQAClientAfterStop(*_publisher);

    _dealer->terminate();
    SQAClientUtil::checkSQAClientAfterStop(*_dealer);

    _watcher->terminate();
    SQAClientUtil::checkSQAClientAfterStop(*_watcher);

    _worker->terminate();
    SQAClientUtil::checkSQAClientAfterStop(*_worker);

    std::vector<SQAAgentData::Ptr>::iterator it;
    for (it = _agentsData.begin(); it != _agentsData.end(); it++)
    {
      SQAAgentData::Ptr data = *it;

      _util.stopSQAAgent(data);
    }

    delete _publisher;
    delete _pd;

    delete _dealer;
    delete _dd;

    delete _watcher;
    delete _wd;

    delete _worker;
    delete _wod;

    _agentsData.clear();
  }

  void StateQueueClient_setSQAClientConfigTest()
  {
    _publisher->setSQAClientConfig("test");
    CPPUNIT_ASSERT(_publisher->_clientConfig == "test");
  }

  void StateQueueClient_getClientOptionsTest()
  {
    _util.generateSQAClientIni(_agentsData[0], false);
    _publisher->setSQAClientConfig(_agentsData[0]->clientIniFilePath);

    std::string servicesAddresses;
    std::string servicesAddressesAll;
    std::string port;

    //TEST: function call succeeds
    CPPUNIT_ASSERT(_publisher->getClientOptions(servicesAddresses, servicesAddressesAll, port));
    //TEST: expected values were retrieved from file
    CPPUNIT_ASSERT(servicesAddresses == _agentsData[0]->sqaControlAddress);
    CPPUNIT_ASSERT(servicesAddressesAll == _agentsData[0]->sqaControlAddressAll);
    CPPUNIT_ASSERT(port == _agentsData[0]->sqaControlPort);
  }

  void StateQueueClient_pushAddressesTest()
  {
    std::vector<std::string> v;
    std::string excludeAddress = "192.168.13.3";
    std::string newAddresses = "192.168.13.1, 192.168.13.2, 192.168.13.3";
    std::vector<std::string> vres;
    vres.push_back("192.168.13.1");vres.push_back("192.168.13.2");
    _publisher->pushAddresses(v, excludeAddress, newAddresses);

    //TEST: push does what is expected
    CPPUNIT_ASSERT(v.size() == vres.size());
    CPPUNIT_ASSERT(std::equal(vres.begin(), vres.end(), v.begin()));

    excludeAddress = "";
    vres.push_back("192.168.13.3");
    _publisher->pushAddresses(v, excludeAddress, newAddresses);
    CPPUNIT_ASSERT(v.size() == vres.size());
    CPPUNIT_ASSERT(std::equal(vres.begin(), vres.end(), v.begin()));
  }

  void StateQueueClient_checkMessageResponseTest()
  {
    std::string messageId = "id";
    std::string messageError = "error";

    //TEST: check should fail in case message-response is != ok
    StateQueueMessage responseError;
    responseError.setType(StateQueueMessage::Publish);
    responseError.set("message-id", messageId);
    responseError.set("message-response", "error");
    responseError.set("message-error", messageError);
    CPPUNIT_ASSERT(!_publisher->checkMessageResponse(responseError));

    //TEST: check should be ok in case message-response is ok
    StateQueueMessage responseOK;
    responseOK.setType(StateQueueMessage::Publish);
    responseOK.set("message-id", messageId);
    responseOK.set("message-response", "ok");
    CPPUNIT_ASSERT(_publisher->checkMessageResponse(responseOK));
  }

  void StateQueueClient_clientConnectivityAgentUpDownTest()
  {
    //start agent
    CPPUNIT_ASSERT(_util.startSQAAgent(_agentsData[0]));
    //give agent time to start
    boost::this_thread::sleep(boost::posix_time::milliseconds(3000));

    // TEST: Client connected to agent
    _pd->setServiceUp();
    SQAClientUtil::checkPublisherAfterStartup(*_publisher, _pd);

    // TEST: Client connected to agent
    _wd->setServiceUp();
    SQAClientUtil::checkWatcherAfterStartup(*_watcher, _wdVec);

    // TEST: Client connected to agent
    _dd->setServiceUp();
    SQAClientUtil::checkPublisherAfterStartup(*_dealer, _dd);

    // TEST: Client connected to agent
    _wod->setServiceUp();
    SQAClientUtil::checkWatcherAfterStartup(*_worker, _wodVec);

    // take down agent
    CPPUNIT_ASSERT(_util.stopSQAAgent(_agentsData[0]));
    boost::this_thread::sleep(boost::posix_time::milliseconds(3000));

    // TEST: Client NOT connected to agent anymore
    _pd->setServiceDown();
    SQAClientUtil::checkPublisherAfterStartup(*_publisher, _pd);

    // TEST: Client NOT connected to agent anymore
    _wd->setServiceDown();
    SQAClientUtil::checkWatcherAfterStartup(*_watcher, _wdVec);

    // TEST: Client NOT connected to agent anymore
    _dd->setServiceDown();
    SQAClientUtil::checkPublisherAfterStartup(*_dealer, _dd);

    // TEST: Client NOT connected to agent anymore
    _wod->setServiceDown();
    SQAClientUtil::checkWatcherAfterStartup(*_worker, _wodVec);
  }

  void StateQueueClient_publishWatchOKTest()
  {
    //start agent
    CPPUNIT_ASSERT(_util.startSQAAgent(_agentsData[0]));

    // give time to publisher and watcher to connect
    boost::this_thread::sleep(boost::posix_time::milliseconds(3000));

    // TEST: Publisher connected to agent
    _pd->setServiceUp();
    SQAClientUtil::checkPublisherAfterStartup(*_publisher, _pd);

    // TEST: Watcher connected to agent
    _wd->setServiceUp();
    SQAClientUtil::checkWatcherAfterStartup(*_watcher, _wdVec);

    // TEST: Regular publish/watch should work with or without response
    SQAClientUtil::checkRegularPublishWatch(*_publisher, *_watcher, "reg", "reg.1.data", true);
    SQAClientUtil::checkRegularPublishWatch(*_publisher, *_watcher, "reg", "reg.1.data", false);

    std::string regData = "regular-event-data";
    // TEST: Empty eventID should not be accepted for publish
    CPPUNIT_ASSERT(!(_publisher->publish("", regData, false)));
    // TEST: Empty data should not be accepted for publish
    CPPUNIT_ASSERT(!(_publisher->publish("reg", "", false)));

    // TEST: Publisher can publish other events too
    std::string otherData = "other-event-data";
    CPPUNIT_ASSERT(_publisher->publish("other", otherData, true));
    //but watcher won't recv it
    std::string wEventId;
    std::string wEventData;
    CPPUNIT_ASSERT(!_watcher->watch(wEventId, wEventData, 50));

    // take down agent
    CPPUNIT_ASSERT(_util.stopSQAAgent(_agentsData[0]));
  }

// Test behavior of publish() when there is no connection to SQA Agent
  void StateQueueClient_publishNoSQAAgentTest()
  {
    std::string regData = "reg-data";
    // TEST: Publish should fail
    CPPUNIT_ASSERT(!_publisher->publish("reg", regData, true));
    CPPUNIT_ASSERT(!_publisher->publish("reg", regData, false));
    CPPUNIT_ASSERT(!_publisher->publish("reg", regData.c_str(), regData.size(), true));
    CPPUNIT_ASSERT(!_publisher->publish("reg", regData.c_str(), regData.size(), false));
  }

  // Test behavior of publish() when used by other types of services
  void StateQueueClient_publishRestrictionToNonPublishersTest()
  {
    //start agent
    CPPUNIT_ASSERT(_util.startSQAAgent(_agentsData[0]));
    //give agent time to start
    boost::this_thread::sleep(boost::posix_time::milliseconds(3000));

    // TEST: Client connected to agent
    _wd->setServiceUp();
    SQAClientUtil::checkWatcherAfterStartup(*_watcher, _wdVec);
    // TEST: Client connected to agent
    _wod->setServiceUp();
    SQAClientUtil::checkWatcherAfterStartup(*_worker, _wodVec);

    std::string regData = "reg-data";
    // TEST: Publish should fail if tried by watcher
    CPPUNIT_ASSERT(!_watcher->publish("reg", regData, true));
    CPPUNIT_ASSERT(!_watcher->publish("reg", regData, false));
    CPPUNIT_ASSERT(!_watcher->publish("reg", regData.c_str(), regData.size(), true));
    CPPUNIT_ASSERT(!_watcher->publish("reg", regData.c_str(), regData.size(), false));

    // TEST: Publish should fail if tried by worker
    CPPUNIT_ASSERT(!_worker->publish("reg", regData, true));
    CPPUNIT_ASSERT(!_worker->publish("reg", regData, false));
    CPPUNIT_ASSERT(!_worker->publish("reg", regData.c_str(), regData.size(), true));
    CPPUNIT_ASSERT(!_worker->publish("reg", regData.c_str(), regData.size(), false));

    // take down agent
    CPPUNIT_ASSERT(_util.stopSQAAgent(_agentsData[0]));
  }

  void StateQueueClient_enqueuePopOKTest()
  {
    //start agent
    CPPUNIT_ASSERT(_util.startSQAAgent(_agentsData[0]));
    // give time to dealer and worker to connect
    boost::this_thread::sleep(boost::posix_time::milliseconds(3000));

    // TEST: Dealer connected to agent
    _dd->setServiceUp();
    SQAClientUtil::checkDealerAfterStartup(*_dealer, _dd);

    // TEST: Worker connected to agent
    _wod->setServiceUp();
    SQAClientUtil::checkWorkerAfterStartup(*_worker, _wodVec);

    // TEST: Regular enqueue/pop should work
    SQAClientUtil::checkRegularEnqueuePop(*_dealer, *_worker, "reg", "reg.1.data", 0);

    // TEST: Empty data should not be accepted for enqueue
    CPPUNIT_ASSERT(!(_dealer->enqueue("")));
    std::string wEventId;
    std::string wEventData;
    int wServiceId=0;
    CPPUNIT_ASSERT(!_worker->pop(wEventId, wEventData, wServiceId, 50));

    // take down agent
    CPPUNIT_ASSERT(_util.stopSQAAgent(_agentsData[0]));
  }

// Test behavior of enqueue() when there is no connection to SQA Agent
  void StateQueueClient_enqueueNoSQAAgentTest()
  {
    std::string regData = "reg-data";
    // TEST: enqueue should fail because there is no agent
    CPPUNIT_ASSERT(!_dealer->enqueue(regData));

    std::string eventId;
    std::string eventData;
    int serviceId=0;
    // TEST: pop should timeout because nothing was enqueued
    CPPUNIT_ASSERT(!_worker->pop(eventId, eventData, serviceId, 50));
  }

  // Test behavior of enqueue() when used by other types of services
  void StateQueueClient_enqueueRestrictionToNonDealersTest()
  {
    //start agent
    CPPUNIT_ASSERT(_util.startSQAAgent(_agentsData[0]));
    //give agent time to start
    boost::this_thread::sleep(boost::posix_time::milliseconds(3000));

    // TEST: Client connected to agent
    _wd->setServiceUp();
    SQAClientUtil::checkWatcherAfterStartup(*_watcher, _wdVec);
    // TEST: Client connected to agent
    _wod->setServiceUp();
    SQAClientUtil::checkWatcherAfterStartup(*_worker, _wodVec);

    std::string regData = "reg-data";
    // TEST: Enqueue should fail if tried by watcher
    CPPUNIT_ASSERT(!_watcher->enqueue(regData));
    // TEST: Enqueue should fail if tried by worker
    CPPUNIT_ASSERT(!_worker->enqueue(regData));

    // take down agent
    CPPUNIT_ASSERT(_util.stopSQAAgent(_agentsData[0]));
  }

  void StateQueueClient_getIoServiceTest()
  {
    CPPUNIT_ASSERT(_publisher->getIoService() == &_publisher->_ioService);

  }
  void StateQueueClient_getLocalAddressTest()
  {
    CPPUNIT_ASSERT(_util.startSQAAgent(_agentsData[0]));
    boost::this_thread::sleep(boost::posix_time::milliseconds(3000));
    _pd->setServiceUp();
    SQAClientUtil::checkPublisherAfterStartup(*_publisher, _pd);

    // TEST: Local client address should match agent address (because they are on the same machine)
    //CPPUNIT_ASSERT(0 == _publisher->getLocalAddress().compare(_agentsData[0]->sqaControlAddress));
    //TODO: This fails because it returns "0.0.0.0", need to investigate

    CPPUNIT_ASSERT(_util.stopSQAAgent(_agentsData[0]));
  }

  void StateQueueClient_isConnectedTest()
  {
    // TEST: no agent, return false;
    CPPUNIT_ASSERT(!_publisher->isConnected());

    //start agent
    CPPUNIT_ASSERT(_util.startSQAAgent(_agentsData[0]));
    // give time to publisher and watcher to connect
    boost::this_thread::sleep(boost::posix_time::milliseconds(3000));

    // TEST: Publisher connected to agent
    _pd->setServiceUp();
    SQAClientUtil::checkPublisherAfterStartup(*_publisher, _pd);

    // TEST: agent up, return true;
    CPPUNIT_ASSERT(_publisher->isConnected());

    // take down agent
    CPPUNIT_ASSERT(_util.stopSQAAgent(_agentsData[0]));

    // TEST: agent down, return true;
    // WARNING: isConnected is not reliable in case connection is suddenly lost
    CPPUNIT_ASSERT(!_publisher->isConnected());
  }

  void StateQueueClient_getClassNameTest()
  {
    CPPUNIT_ASSERT(0 == strcmp(_publisher->getClassName(), "StateQueueClient"));
  }
  void StateQueueClient_getEventQueueTest()
  {
    CPPUNIT_ASSERT(_publisher->getEventQueue() == &_publisher->_eventQueue);
  }
  void StateQueueClient_setFallbackServicesTest()
  {
    //TEST: by default fallback data is initialized to neutral values;
    SQAClientUtil::checkFallbackDataAFterStartup(*_publisher, _pd);

    // TEST: set fallback does not work for non publishers
    CPPUNIT_ASSERT(!_watcher->setFallbackServices(_wd->_servicesAddressesAll, 20));
    CPPUNIT_ASSERT(!_worker->setFallbackServices(_wd->_servicesAddressesAll, 20));

    // TEST: set fallback does not work in case no addresses provided
    CPPUNIT_ASSERT(!_publisher->setFallbackServices("", 20));

    // TEST: set fallback work as expected for publishers
    CPPUNIT_ASSERT(_publisher->setFallbackServices(_pd->_servicesAddressesAll, 10));
    _pd->setHasFallbackData();
    SQAClientUtil::checkFallbackDataAFterStartup(*_publisher, _pd);

    // TEST: set fallback cannot be set twice for publishers
    CPPUNIT_ASSERT(!_publisher->setFallbackServices(_pd->_servicesAddressesAll, 10));
  }

  void StateQueueClient_watchTest()
  {
    // done in StateQueueClient_publishWatchOKTest
  }
  void StateQueueClient_publishTest()
  {
    // done in StateQueueClient_publishWatchOKTest
  }
  void StateQueueClient_publishAndSetTest()
  {
    CPPUNIT_FAIL("Not implemented yet!");
  }
  void StateQueueClient_eraseTest()
  {
    CPPUNIT_FAIL("Not implemented yet!");
  }
  void StateQueueClient_persistTest()
  {
    CPPUNIT_FAIL("Not implemented yet!");
  }
  void StateQueueClient_setTest()
  {
    CPPUNIT_FAIL("Not implemented yet!");
  }
  void StateQueueClient_msetTest()
  {
    CPPUNIT_FAIL("Not implemented yet!");
  }
  void StateQueueClient_getTest()
  {
    CPPUNIT_FAIL("Not implemented yet!");
  }
  void StateQueueClient_mgetTest()
  {
    CPPUNIT_FAIL("Not implemented yet!");
  }
  void StateQueueClient_mgetmTest()
  {
    CPPUNIT_FAIL("Not implemented yet!");
  }
  void StateQueueClient_mgetiTest()
  {
    CPPUNIT_FAIL("Not implemented yet!");
  }
  void StateQueueClient_removeTest()
  {
    CPPUNIT_FAIL("Not implemented yet!");
  }
  void StateQueueClient_startTest()
  {
    // tested by SQAClientUtil::checkStateQueueClientAfterStartup
  }
  void StateQueueClient_tryPrimaryCoreTest()
  {
    // test implemented in StateQueueClient_checkFallbackTest
  }
  void StateQueueClient_trySecondaryCoreTest()
  {
    // test implemented in StateQueueClient_checkFallbackTest
  }
  void StateQueueClient_createFallbackCoreTest()
  {
    // test implemented in StateQueueClient_checkFallbackTest
  }
  void StateQueueClient_checkFallbackTest()
  {
    //TEST:Simulate a fallback scenario with 3 agents, which are initially all down,
    // after that they are started one by one and the fallback will kick in
    // NOTE: This test is done by manually calling the fallback logic

    // TEST: set fallback to trigger after 2 failed connect attempts
    _pd->_fallbackTimeout = 2;
    CPPUNIT_ASSERT(_publisher->setFallbackServices(_pd->_servicesAddressesAll, 2));
    _pd->setHasFallbackData();
    SQAClientUtil::checkFallbackDataAFterStartup(*_publisher, _pd);

    // cancel fallback timer so the fallback logic can be manually manipulated from here
    _publisher->_fallbackTimer->cancel();

    // TEST: failed connect counter is 1 because there was one checkFallback call
    CPPUNIT_ASSERT(_publisher->_currentFailedConnects == 1);
    // TEST: fallback is not active yet
    CPPUNIT_ASSERT(_publisher->_isFallbackActive == false);

    // trigger two checks (simulate two fallback timer ticks)
    _publisher->checkFallback();
    _publisher->checkFallback();

    //TEST: After 2 more failed connects the fallback tried to kick in but no agent
    CPPUNIT_ASSERT(_publisher->_currentFailedConnects == 3);
    CPPUNIT_ASSERT(_publisher->_isFallbackActive == false);
    CPPUNIT_ASSERT(_publisher->_cores.size() == 3);
    // TEST: not connected yet because no agent is up
    CPPUNIT_ASSERT(!_publisher->isConnected());
    //TEST: this should fail because primary core is down
    CPPUNIT_ASSERT(!_publisher->tryPrimaryCore());
    //TEST: this should fail because both secondaries are down
    CPPUNIT_ASSERT(!_publisher->trySecondaryCore());

    //start first agent
    CPPUNIT_ASSERT(_util.startSQAAgent(_agentsData[0]));
    boost::this_thread::sleep(boost::posix_time::milliseconds(500));

    //TEST: this should work because primary core is up
    CPPUNIT_ASSERT(_publisher->tryPrimaryCore());
    //TEST: this should fail because both secondaries are down
    CPPUNIT_ASSERT(!_publisher->trySecondaryCore());
    // TEST: fallback data was reset because client got back to primary
    CPPUNIT_ASSERT(_publisher->_currentFailedConnects == 0);
    CPPUNIT_ASSERT(_publisher->_isFallbackActive == false);
    // TEST: all created cores are untouched
    CPPUNIT_ASSERT(_publisher->_cores.size() == 3);
    // TEST: agent up, return true;
    CPPUNIT_ASSERT(_publisher->isConnected());

    // take down agent
    CPPUNIT_ASSERT(_util.stopSQAAgent(_agentsData[0]));

    // trigger three checks (simulate two fallback timer ticks)
    _publisher->checkFallback();
    _publisher->checkFallback();
    _publisher->checkFallback();

    //TEST: After 3 failed connects the fallback tried to kick in but no agent
    CPPUNIT_ASSERT(_publisher->_currentFailedConnects == 3);
    CPPUNIT_ASSERT(_publisher->_isFallbackActive == false);
    //TEST: number of cores is untouched
    CPPUNIT_ASSERT(_publisher->_cores.size() == 3);
    // TEST: not connected yet because no agent is up
    CPPUNIT_ASSERT(!_publisher->isConnected());

    //start second agent
    CPPUNIT_ASSERT(_util.startSQAAgent(_agentsData[1]));
    boost::this_thread::sleep(boost::posix_time::milliseconds(500));

    //TEST: this should fail because primary core is down
    CPPUNIT_ASSERT(!_publisher->tryPrimaryCore());
    //TEST: this should work because a secondary is up
    CPPUNIT_ASSERT(_publisher->trySecondaryCore());
    // TEST: fallback data was reset because client got a secondary
    CPPUNIT_ASSERT(_publisher->_currentFailedConnects == 0);
    CPPUNIT_ASSERT(_publisher->_isFallbackActive == true);
    // TEST: all created cores are untouched
    CPPUNIT_ASSERT(_publisher->_cores.size() == 3);
    // TEST: agent up, return true;
    CPPUNIT_ASSERT(_publisher->isConnected());

    // take down second agent
    CPPUNIT_ASSERT(_util.stopSQAAgent(_agentsData[1]));

    // trigger three checks (simulate two fallback timer ticks)
    _publisher->checkFallback();
    _publisher->checkFallback();
    _publisher->checkFallback();

    //TEST: After 3 failed connects the fallback tried to kick in but no agent
    CPPUNIT_ASSERT(_publisher->_currentFailedConnects == 3);
    CPPUNIT_ASSERT(_publisher->_isFallbackActive == true);
    //TEST: number of cores is untouched
    CPPUNIT_ASSERT(_publisher->_cores.size() == 3);
    // TEST: not connected yet because no agent is up
    CPPUNIT_ASSERT(!_publisher->isConnected());

    //start third agent
    CPPUNIT_ASSERT(_util.startSQAAgent(_agentsData[2]));
    boost::this_thread::sleep(boost::posix_time::milliseconds(500));

    //TEST: this should fail because primary core is down
    CPPUNIT_ASSERT(!_publisher->tryPrimaryCore());
    //TEST: this should work because a secondary is up
    CPPUNIT_ASSERT(_publisher->trySecondaryCore());
    // TEST: fallback data was reset because client got a secondary
    CPPUNIT_ASSERT(_publisher->_currentFailedConnects == 0);
    CPPUNIT_ASSERT(_publisher->_isFallbackActive == true);
    // TEST: all created cores are untouched
    CPPUNIT_ASSERT(_publisher->_cores.size() == 3);
    // TEST: agent up, return true;
    CPPUNIT_ASSERT(_publisher->isConnected());

    // take down second agent
    CPPUNIT_ASSERT(_util.stopSQAAgent(_agentsData[2]));
  }

  void StateQueueClient_setFallbackTimerTest()
  {
    // done by tests from SQAClientHATest class
  }
  void StateQueueClient_fallbackLoopTest()
  {
    // done by tests from SQAClientHATest class
  }
  void StateQueueClient_popTest()
  {
    //done in StateQueueClient_enqueuePopOKTest
  }
  void StateQueueClient_enqueueTest()
  {
    //done in StateQueueClient_enqueuePopOKTest
  }
  void StateQueueClient_internal_publishTest()
  {
    //done in StateQueueClient_publishWatchOKTest
  }

  void SQAClientCore_getClassNameTest()
  {
    //TEST: verify class name is expected
    CPPUNIT_ASSERT(0 == strcmp(_publisher->_core->getClassName(), "SQAClientCore"));
  }
  void SQAClientCore_getLocalAddressTest()
  {
    CPPUNIT_ASSERT(_util.startSQAAgent(_agentsData[0]));
    boost::this_thread::sleep(boost::posix_time::milliseconds(3000));
    _pd->setServiceUp();
    SQAClientUtil::checkPublisherAfterStartup(*_publisher, _pd);

    // TEST: Local client address should match agent address (because they are on the same machine)
    //CPPUNIT_ASSERT(0 == _publisher->_core->getLocalAddress().compare(_agentsData[0]->sqaControlAddress));
    //TODO: This fails because it returns "0.0.0.0", need to investigate

    CPPUNIT_ASSERT(_util.stopSQAAgent(_agentsData[0]));
  }
  void SQAClientCore_terminateTest()
  {
    //not much to test here
    _publisher->_core->terminate();
    SQAClientUtil::checkSQAClientCoreAfterStop(_publisher->_core);
  }

  void SQAClientCore_isConnectedTest()
  {
    // TEST: no agent, return false;
    CPPUNIT_ASSERT(!_publisher->_core->isConnected());

    //start agent
    CPPUNIT_ASSERT(_util.startSQAAgent(_agentsData[0]));
    // give time to publisher and watcher to connect
    boost::this_thread::sleep(boost::posix_time::milliseconds(3000));

    // TEST: Publisher connected to agent
    _pd->setServiceUp();
    SQAClientUtil::checkPublisherAfterStartup(*_publisher, _pd);

    // TEST: agent up, return true;
    CPPUNIT_ASSERT(_publisher->_core->isConnected());

    // take down agent
    CPPUNIT_ASSERT(_util.stopSQAAgent(_agentsData[0]));
  }

  void SQAClientCore_isConnectedNowTest()
  {
    // TEST: no agent, return false;
    CPPUNIT_ASSERT(!_publisher->_core->isConnectedNow());

    //start agent
    CPPUNIT_ASSERT(_util.startSQAAgent(_agentsData[0]));
    // give time to publisher and watcher to connect
    boost::this_thread::sleep(boost::posix_time::milliseconds(3000));

    // TEST: Publisher connected to agent
    _pd->setServiceUp();
    SQAClientUtil::checkPublisherAfterStartup(*_publisher, _pd);

    // TEST: agent up, return true;
    CPPUNIT_ASSERT(_publisher->_core->isConnectedNow());

    // take down agent
    CPPUNIT_ASSERT(_util.stopSQAAgent(_agentsData[0]));

    // TEST: agent down, return false;
    // WARNING: isConnected IS reliable in case connection is suddenly lost
    CPPUNIT_ASSERT(!_publisher->_core->isConnectedNow());
  }

  void SQAClientCore_setExpiresTest()
  {
    //TEST: By default expires is 10 sec
    CPPUNIT_ASSERT(_publisher->_core->_expires == 10);

    //TEST: Changing expires work
    _publisher->_core->setExpires(1000);
    CPPUNIT_ASSERT(_publisher->_core->_expires == 1000);

    // put it back
    _publisher->_core->setExpires(10);

    // TODO: Add tests to check that expires is indeed used in all places that it should be used
  }

  void SQAClientCore_signinTest()
  {
    //cancel timers to be able to manually manipulate the signin logic
    _publisher->_core->_signinTimer->cancel();
    _publisher->_core->_keepAliveTimer->cancel();

    int signinAttempts = _publisher->_core->_signinAttempts;

    //TEST: signin should fail if no agent connected
    CPPUNIT_ASSERT(!_publisher->_core->signin());
    //TEST: state is failed if signin failed
    CPPUNIT_ASSERT(_publisher->_core->_signinState == SQAOpFailed);
    //TEST: logout should fail if no agent connected
    CPPUNIT_ASSERT(!_publisher->_core->logout());
    //TEST: counter is incremented by every signin attempt
    CPPUNIT_ASSERT(_publisher->_core->_signinAttempts == signinAttempts + 1);

    //start agent and give it time to start
    CPPUNIT_ASSERT(_util.startSQAAgent(_agentsData[0]));
    boost::this_thread::sleep(boost::posix_time::milliseconds(3000));
    _publisher->_core->sendKeepAlive();

    //TEST: signin should succeed
    CPPUNIT_ASSERT(_publisher->_core->signin());
    //TEST: state is ok when signin ok
    CPPUNIT_ASSERT(_publisher->_core->_signinState == SQAOpOK);
    //TEST: logout works after signin ok
    CPPUNIT_ASSERT(_publisher->_core->logout());
    //TEST: counter is incremented by every signin attempt
    CPPUNIT_ASSERT(_publisher->_core->_signinAttempts == signinAttempts + 2);

    // take down agent
    CPPUNIT_ASSERT(_util.stopSQAAgent(_agentsData[0]));
  }

  void SQAClientCore_logoutTest()
  {
    //tested in SQAClientCore_signinTest
  }
  void SQAClientCore_subscribeTest()
  {
    CPPUNIT_FAIL("TODO: How to test this!");
  }
  void SQAClientCore_sendNoResponseTest()
  {
    // DONE in SQAClientCore_sendAndReceiveTest
  }
  void SQAClientCore_sendAndReceiveTest()
  {
    StateQueueMessage request;
    StateQueueMessage response;

    //TEST: it should fail when client not connected to agent
    CPPUNIT_ASSERT(!_publisher->_core->sendAndReceive(request, response));
    CPPUNIT_ASSERT(!_publisher->_core->sendNoResponse(request));

    //start agent and give clients time to connect to it
    CPPUNIT_ASSERT(_util.startSQAAgent(_agentsData[0]));
    boost::this_thread::sleep(boost::posix_time::milliseconds(2000));

    // TEST: Publisher connected to agent
    _pd->setServiceUp();
    SQAClientUtil::checkPublisherAfterStartup(*_publisher, _pd);

    //TEST: send should work when client is connected to agent
    CPPUNIT_ASSERT(_publisher->_core->sendAndReceive(request, response));
    CPPUNIT_ASSERT(_publisher->_core->sendNoResponse(request));

    //TEST: send should fail if no connections available in pool
    StateQueueClient::SQAClientCore::BlockingTcpClient::Ptr conn;
    _publisher->_core->_clientPool.dequeue(conn);
    CPPUNIT_ASSERT(!_publisher->_core->sendAndReceive(request, response));
    CPPUNIT_ASSERT(!_publisher->_core->sendNoResponse(request));
    _publisher->_core->_clientPool.enqueue(conn);

    // take down agent
    CPPUNIT_ASSERT(_util.stopSQAAgent(_agentsData[0]));
  }

  void SQAClientCore_initTest()
  {
    //tested by checkSQAClientCoreAfterStartup
  }

  void SQAClientCore_sendKeepAliveTest()
  {
    //stop timers to be able to manual control keepalive logic
    _publisher->_core->_signinTimer->cancel();
    _publisher->_core->_keepAliveTimer->cancel();

    //TEST: One keep alive is attempted upon start
    CPPUNIT_ASSERT(_publisher->_core->_keepAliveAttempts == 1);
    //TEST: sendKeepAlive fails if no agent connected
    CPPUNIT_ASSERT(!_publisher->_core->sendKeepAlive());
    //TEST: keepalive state is failed when send failed
    CPPUNIT_ASSERT(_publisher->_core->_keepAliveState == SQAOpFailed);
    //TEST: sendKeepAlive increment counter at every attempt
    CPPUNIT_ASSERT(_publisher->_core->_keepAliveAttempts == 2);

    //TODO: Add signin manipulation here as sign in is manipulated from sendKeepAlive

    //start agent and give it time
    CPPUNIT_ASSERT(_util.startSQAAgent(_agentsData[0]));
    boost::this_thread::sleep(boost::posix_time::milliseconds(1000));

    //TEST: sendKeepAlive ok if agent connected
    CPPUNIT_ASSERT(_publisher->_core->sendKeepAlive());
    //TEST: keepalive state is ok when send ok
    CPPUNIT_ASSERT(_publisher->_core->_keepAliveState == SQAOpOK);
    //TEST: sendKeepAlive increment counter at every attempt
    CPPUNIT_ASSERT(_publisher->_core->_keepAliveAttempts == 3);

    // take down agent
    CPPUNIT_ASSERT(_util.stopSQAAgent(_agentsData[0]));
  }

  void SQAClientCore_setKeepAliveTimerTest()
  {
    // tests done in SQAClientHATest show that this works
  }
  void SQAClientCore_keepAliveLoopTest()
  {
    // tests done in SQAClientHATest show that this works
  }
  void SQAClientCore_setSigninTimerTest()
  {
    // tests done in SQAClientHATest show that this works
  }
  void SQAClientCore_signinLoopTest()
  {
    // tests done in SQAClientHATest show that this works
  }
  void SQAClientCore_eventLoopTest()
  {
    CPPUNIT_FAIL("TODO: How to test this!");
  }
  void SQAClientCore_do_popTest()
  {
    CPPUNIT_FAIL("TODO: How to test this!");
  }
  void SQAClientCore_do_watchTest()
  {
    CPPUNIT_FAIL("TODO: How to test this!");
  }
  void SQAClientCore_readEventTest()
  {
    CPPUNIT_FAIL("TODO: How to test this!");
  }
  void SQAClientCore_zmq_freeTest()
  {
    CPPUNIT_FAIL("TODO: How to test this!");
  }
  void SQAClientCore_zmq_sendTest()
  {
    CPPUNIT_FAIL("TODO: How to test this!");
  }
  void SQAClientCore_zmq_sendmoreTest()
  {
    CPPUNIT_FAIL("TODO: How to test this!");
  }
  void SQAClientCore_zmq_receiveTest()
  {
    CPPUNIT_FAIL("TODO: How to test this!");
  }

  std::vector<SQAAgentData::Ptr> _agentsData;
  SQAAgentUtil _util;

  SQAClientData* _pd;
  StateQueueClient* _publisher;

  SQAClientData* _dd;
  StateQueueClient* _dealer;

  SQAClientData* _wd;
  std::vector<SQAClientData*> _wdVec;
  StateQueueClient* _watcher;

  SQAClientData* _wod;
  std::vector<SQAClientData*> _wodVec;
  StateQueueClient* _worker;
};


CPPUNIT_TEST_SUITE_REGISTRATION(SQAClientTest);

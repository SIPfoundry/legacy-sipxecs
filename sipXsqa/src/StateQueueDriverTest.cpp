/*
 * Copyright (c) eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */

#include "sqa/StateQueueDriverTest.h"
#include <iostream>
#include "sqa/UnitTest.h"

//
// DEFINE_UNIT_TEST - Define a Test Group.  Must be called prior to DEFINE_TEST
// DEFINE_TEST - Define a new unit test belonging to a defined group
// DEFINE_RESOURCE - Register a resource that is accessible to unit tests in the same group
// GET_RESOURCE - Get the value of the resource that was previously created by DEFINE_RESOURCE
// ASSERT_COND(cond) - Assert if the logical condition is false
// ASSERT_STR_EQ(var1, var2) - Assert that two strings are  equal
// ASSERT_STR_CASELESS_EQ(var1, var2) - Assert that two strings are equal but ignoring case comparison
// ASSERT_STR_NEQ(var1, var2) - Asserts that two strings are not eual
// ASSERT_EQ(var1, var2) - Asserts that the two values are equal
// ASSERT_NEQ(var1, var2) - Asserts that the the values are not equal
// ASSERT_LT(var1, var2) - Asserts that the value var1 is less than value of var2
// ASSERT_GT(var1, var2)  Asserts that the value var1 is greater than value of var2
//

DEFINE_UNIT_TEST(TestDriver);

DEFINE_TEST(TestDriver, TestSimplePop)
{
  StateQueueClient* pClient = GET_RESOURCE(TestDriver, StateQueueClient*, "simple_pop_client");
  StateQueueClient* pPublisher = GET_RESOURCE(TestDriver, StateQueueClient*, "simple_publisher");
  pPublisher->enqueue("Hello SQA!");
  std::string messageId;
  std::string messageData;
  ASSERT_COND(pClient->pop(messageId, messageData));
  ASSERT_STR_EQ(messageData, "Hello SQA!");
  ASSERT_COND(pClient->erase(messageId));
}

DEFINE_TEST(TestDriver, TestMultiplePop)
{
  StateQueueAgent* _pAgent = GET_RESOURCE(TestDriver, StateQueueAgent*, "state_agent");
  std::string address;
  std::string port;
  std::string publisher;
  _pAgent->options().getOption("sqa-control-address", address);
  _pAgent->options().getOption("sqa-control-port", port);
  _pAgent->options().getOption("zmq-subscription-address", publisher);
  StateQueueClient* pPublisher = GET_RESOURCE(TestDriver, StateQueueClient*, "simple_publisher");
  //
  // Create three threaded clients
  //
  ThreadedPop client1("StateQueueDriverTest-C1", address, port, "reg");
  ThreadedPop client2("StateQueueDriverTest-C2", address, port, "reg");
  ThreadedPop client3("StateQueueDriverTest-C3", address, port, "reg");

  boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
  
  client1.start();
  client2.start();
  client3.start();

  int currentMax = 1500;
  for (int x = 0; x < 5; x++)
  {
    for (int i = 0; i < 500; i++)
    {
      pPublisher->enqueue("test multiple poppers-1", 10);
      pPublisher->enqueue("test multiple poppers-2", 10);
      pPublisher->enqueue("test multiple poppers-3", 10);
    }

    while(client1.total + client2.total + client3.total < currentMax)
    {
      boost::this_thread::sleep(boost::posix_time::milliseconds(5));
    }

    std::cout << std::endl << "Iteration " << x << std::endl;
    std::cout << "Client 1 processed " << client1.total << " events." << std::endl;
    std::cout << "Client 2 processed " << client2.total << " events." << std::endl;
    std::cout << "Client 3 processed " << client3.total << " events." << std::endl;
    
    currentMax += 1500;
  }
}

DEFINE_TEST(TestDriver, TestGetSetErase)
{
  StateQueueClient* pClient = GET_RESOURCE(TestDriver, StateQueueClient*, "simple_pop_client");
  ASSERT_COND(pClient->set(1, "sample-set-data-id", "sample-set-data", 10));

  std::string sampleData;
  ASSERT_COND(pClient->get(1, "sample-set-data-id", sampleData));
  ASSERT_STR_EQ(sampleData, "sample-set-data");
  ASSERT_COND(pClient->remove(1, "sample-set-data-id"));
  ASSERT_COND(!pClient->get(1, "sample-set-data-id", sampleData));
}

DEFINE_TEST(TestDriver, TestSimplePersistGetErase)
{
  StateQueueClient* pClient = GET_RESOURCE(TestDriver, StateQueueClient*, "simple_pop_client");
  StateQueueClient* pPublisher = GET_RESOURCE(TestDriver, StateQueueClient*, "simple_publisher");
  ASSERT_COND(pPublisher->enqueue("Hello SQA!"));

  std::string eventId;
  std::string eventData;
  ASSERT_COND(pClient->pop(eventId, eventData));
  ASSERT_COND(pClient->persist(1, eventId, 10));
  std::string sampleData;
  ASSERT_COND(pClient->get(1, eventId, sampleData));
  ASSERT_STR_EQ(sampleData, eventData);
  ASSERT_COND(pClient->remove(1, eventId));
  ASSERT_COND(!pClient->get(1, eventId, sampleData));
}

DEFINE_TEST(TestDriver, TestWatcher)
{
  StateQueueAgent* _pAgent = GET_RESOURCE(TestDriver, StateQueueAgent*, "state_agent");
  std::string address;
  std::string port;
  _pAgent->options().getOption("sqa-control-address", address);
  _pAgent->options().getOption("sqa-control-port", port);
  StateQueueClient* pPublisher = GET_RESOURCE(TestDriver, StateQueueClient*, "simple_publisher");
  StateQueueClient watcher(StateQueueClient::Watcher, "StateQueueDriverTest", address, port, "watcher-data",  1);
  boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
  ASSERT_COND(pPublisher->publish("watcher-data-sample", "Hello SQA!", false));
  std::string watcherData;
  std::string eventId;
  ASSERT_COND(watcher.watch(eventId, watcherData));
  ASSERT_STR_EQ(watcherData, "Hello SQA!");
}

DEFINE_TEST(TestDriver, TestPublishAndPersist)
{
  StateQueueAgent* _pAgent = GET_RESOURCE(TestDriver, StateQueueAgent*, "state_agent");
  std::string address;
  std::string port;
  _pAgent->options().getOption("sqa-control-address", address);
  _pAgent->options().getOption("sqa-control-port", port);

  SQAPublisher publisher("TestPublishAndPersist", address.c_str(), port.c_str(), 1, 100, 100);
  SQAWatcher watcher("TestPublishAndPersist", address.c_str(), port.c_str(), "pub&persist", 1, 100, 100);
  boost::this_thread::sleep(boost::posix_time::milliseconds(100));
  ASSERT_COND(publisher.publishAndPersist(5, "pub&persist", "test-data", 10));
  SQAEvent* pEvent = watcher.watch();
  ASSERT_COND(pEvent);
  ASSERT_STR_EQ(pEvent->data, "test-data");
  
  char* data = watcher.get(5, pEvent->id);
  ASSERT_COND(data);
  ASSERT_STR_EQ(data, "test-data");
  free(data);
  delete pEvent;
}

DEFINE_TEST(TestDriver, TestDealAndPublish)
{
  StateQueueAgent* _pAgent = GET_RESOURCE(TestDriver, StateQueueAgent*, "state_agent");
  std::string address;
  std::string port;
  _pAgent->options().getOption("sqa-control-address", address);
  _pAgent->options().getOption("sqa-control-port", port);
  /*
   inline SQADealer::SQADealer(
  const char* applicationId, // Unique application ID that will identify this watcher to SQA
  const char* serviceAddress, // The IP address of the SQA
  const char* servicePort, // The port where SQA is listening for connections
  const char* eventId, // Event ID of the event being watched. Example: "sqa.not"
  int poolSize // Number of active connections to SQA
)
   */
  SQADealer dealer("TestDealAndPublish", address.c_str(), port.c_str(), "not", 1, 100, 100);
  SQAWatcher watcher("TestDealAndPublish", address.c_str(), port.c_str(), "not", 1, 100, 100);
  SQAWorker worker("TestDealAndPublish", address.c_str(), port.c_str(), "not", 1, 100, 100);
  boost::this_thread::sleep(boost::posix_time::milliseconds(100));
  ASSERT_COND(dealer.dealAndPublish("test-data", 20));
  SQAEvent* pEvent = worker.fetchTask();
  ASSERT_COND(pEvent);
  ASSERT_STR_EQ(pEvent->data, "test-data");
  delete pEvent;
  pEvent = 0;
  pEvent = watcher.watch();
  ASSERT_COND(pEvent);
  ASSERT_STR_EQ(pEvent->data, "test-data");
  delete pEvent;
}

DEFINE_TEST(TestDriver, TestTimedMap)
{
  TimedMap set;
  std::string item1Value = "item-1";
  std::string item2Value = "item-2";
  set.insert("my-set-id", "item-1", item1Value, 1);
  set.insert("my-set-id", "item-2", item2Value, 1);
  boost::any item1, item2;
  ASSERT_COND(set.getItem("my-set-id", "item-1", item1));
  ASSERT_COND(set.getItem("my-set-id", "item-2", item2));
  ASSERT_STR_EQ(boost::any_cast<std::string&>(item1).c_str(),  item1Value.c_str());
  ASSERT_STR_EQ(boost::any_cast<std::string&>(item2).c_str(),  item2Value.c_str());
  //
  // Test getting all items
  //
  TimedMap::Items items;
  ASSERT_COND(set.getItems("my-set-id", items));
  ASSERT_STR_EQ(boost::any_cast<std::string&>(items["item-1"]).c_str(), item1Value.c_str());
  ASSERT_STR_EQ(boost::any_cast<std::string&>(items["item-2"]).c_str(), item2Value.c_str());
  //
  // Wait two seconds for items to expire.
  //
  boost::this_thread::sleep(boost::posix_time::milliseconds(2000));
  set.cleanup();
  ASSERT_COND(!set.getItem("my-set-id", "item-1", item1));
  ASSERT_COND(!set.getItem("my-set-id", "item-2", item2));
}

DEFINE_TEST(TestDriver, TestMapGetSet)
{
  StateQueueClient* pClient = GET_RESOURCE(TestDriver, StateQueueClient*, "simple_pop_client");
  ASSERT_COND(pClient->mset(1, "sample-set-data-id", "cseq", "1", 10));

  std::string sampleData;
  ASSERT_COND(pClient->mget(1, "sample-set-data-id", "cseq", sampleData));
  ASSERT_STR_EQ(sampleData, "1");
  ASSERT_COND(pClient->mgeti(1, "sample-set-data-id", "cseq", sampleData));
  ASSERT_STR_EQ(sampleData, "2");
  ASSERT_COND(pClient->mgeti(1, "sample-set-data-id", "cseq", sampleData));
  ASSERT_STR_EQ(sampleData, "3");
}

DEFINE_TEST(TestDriver, TestMapGetSetPlugin)
{
  StateQueueAgent* _pAgent = GET_RESOURCE(TestDriver, StateQueueAgent*, "state_agent");
  std::string address;
  std::string port;
  _pAgent->options().getOption("sqa-control-address", address);
  _pAgent->options().getOption("sqa-control-port", port);
  SQAWatcher watcher("TestMapGetSetPlugin", address.c_str(), port.c_str(), "dummy", 1, 100, 100);
  watcher.mset(10, "TestMapGetSetPlugin", "cseq", "0", 10);
  char* cseq = watcher.mget(10, "TestMapGetSetPlugin", "cseq");
  ASSERT_STR_EQ(cseq, "0");
  free(cseq);
  int incremented = -1;
  ASSERT_COND(watcher.mgeti(10, "TestMapGetSetPlugin", "cseq", incremented));
  ASSERT_COND(incremented == 1);
  cseq = watcher.mget(10, "TestMapGetSetPlugin", "cseq");
  ASSERT_STR_EQ(cseq, "1");
  free(cseq);
  watcher.mset(10, "TestMapGetSetPlugin", "call-id", "test-call-id", 10);
  std::map<std::string, std::string> smap = watcher.mgetAll(10,"TestMapGetSetPlugin");
  ASSERT_COND(smap.find("cseq") != smap.end());
  ASSERT_COND(smap.find("call-id") != smap.end());
  ASSERT_STR_EQ(smap.find("cseq")->second.c_str(), "1");
  ASSERT_STR_EQ(smap.find("call-id")->second.c_str(), "test-call-id");
}

bool StateQueueDriverTest::runTests()
{
  
  std::string address;
  std::string port;

  _agent.options().getOption("sqa-control-address", address);
  _agent.options().getOption("sqa-control-port", port);

  //
  // Define common resource accessible by all unit tests
  //
  DEFINE_RESOURCE(TestDriver, "state_agent", &_agent);
  DEFINE_RESOURCE(TestDriver, "simple_pop_client", new StateQueueClient(StateQueueClient::Worker, "StateQueueDriverTest", address, port, "reg",  2));
  DEFINE_RESOURCE(TestDriver, "simple_publisher", new StateQueueClient(StateQueueClient::Publisher, "StateQueueDriverTest", address, port, "reg",  2));

  boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
  
  //
  // Run the unit tests
  //
  VERIFY_TEST(TestDriver, TestTimedMap);
  VERIFY_TEST(TestDriver, TestMapGetSet);
  VERIFY_TEST(TestDriver, TestMapGetSetPlugin)
  VERIFY_TEST(TestDriver, TestSimplePop);
  VERIFY_TEST(TestDriver, TestMultiplePop);
  VERIFY_TEST(TestDriver, TestGetSetErase);
  VERIFY_TEST(TestDriver, TestSimplePersistGetErase);
  VERIFY_TEST(TestDriver, TestWatcher);
  VERIFY_TEST(TestDriver, TestPublishAndPersist);
  VERIFY_TEST(TestDriver, TestDealAndPublish)
  //
  // Delete simple_pop_client so it does not participate in popping events
  //
  delete GET_RESOURCE(TestDriver, StateQueueClient*, "simple_pop_client");
  //VERIFY_TEST(TestDriver, TestMultiplePop);
  //
  // Delete the common resource because the are heap allocated using new()!
  //
  delete GET_RESOURCE(TestDriver, StateQueueClient*, "simple_publisher");
  

  END_UNIT_TEST(TestDriver);

  boost::this_thread::sleep(boost::posix_time::milliseconds(1000));

  return TEST_RESULT(TestDriver);
}


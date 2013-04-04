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
#include "SQAClientUtil.h"

void SQAClientUtil::checkSQAClientCoreAfterStartup(StateQueueClient* client, StateQueueClient::SQAClientCore* core, SQAClientData* cd)
{
  CPPUNIT_ASSERT(core->_owner == client);
  CPPUNIT_ASSERT(core->_idx == cd->_idx);
  CPPUNIT_ASSERT(core->_type == cd->_type);
  CPPUNIT_ASSERT(core->_keepAliveTimer != NULL);
  CPPUNIT_ASSERT(core->_signinTimer != NULL);

  CPPUNIT_ASSERT(core->_poolSize == cd->_poolSize);
  CPPUNIT_ASSERT(core->_clientPoolSize == cd->_poolSize);

  CPPUNIT_ASSERT( std::string::npos != cd->_servicesAddresses.find(core->_serviceAddress));
  CPPUNIT_ASSERT( std::string::npos != cd->_servicesAddressesAll.find(core->_serviceAddress));
  CPPUNIT_ASSERT(core->_servicePort == cd->_servicePort);

  CPPUNIT_ASSERT(core->_terminate == false);
  if (!SQAUtil::isPublisher(cd->_type))
  {
    CPPUNIT_ASSERT(core->_zmqContext != NULL);
    CPPUNIT_ASSERT(core->_zmqSocket != NULL);
    CPPUNIT_ASSERT(core->_pEventThread != NULL);
  }
  else
  {
    CPPUNIT_ASSERT(core->_zmqContext == NULL);
    CPPUNIT_ASSERT(core->_zmqSocket == NULL);
    CPPUNIT_ASSERT(core->_pEventThread == NULL);
  }

  CPPUNIT_ASSERT(core->_zmqEventId == client->_zmqEventId);
  CPPUNIT_ASSERT(core->_applicationId == cd->_coreApplicationId);
  CPPUNIT_ASSERT(core->_clientPointers.size() == cd->_poolSize);
  CPPUNIT_ASSERT(core->_expires == 10);

  CPPUNIT_ASSERT(core->_backoffCount == 0);
  CPPUNIT_ASSERT(!core->_localAddress.empty());
  if (cd->_serviceIsUp)
  {
    CPPUNIT_ASSERT(core->_isAlive == true);
  }
  else
  {
    CPPUNIT_ASSERT(core->_isAlive == false);
  }

  CPPUNIT_ASSERT(core->_signinTimeout == cd->_signinTimeout);
  if (cd->_serviceIsUp)
  {
    CPPUNIT_ASSERT(core->_signinState == SQAOpOK);
  }
  else
  {
    CPPUNIT_ASSERT(core->_signinState == SQAOpFailed);
  }
  CPPUNIT_ASSERT(core->_signinAttempts >= 1);

  CPPUNIT_ASSERT(core->_keepAliveTimeout == cd->_keepAliveTimeout);
  if (cd->_serviceIsUp)
  {
    CPPUNIT_ASSERT(core->_keepAliveState == SQAOpOK);
  }
  else
  {
    CPPUNIT_ASSERT(core->_keepAliveState == SQAOpFailed);
  }
  CPPUNIT_ASSERT(core->_keepAliveAttempts >= 1);

  if (SQAUtil::isPublisher(core->_type))
  {
    CPPUNIT_ASSERT(core->_subscribeAttempts == 0);
    CPPUNIT_ASSERT(core->_subscribeState == SQAOpNotDone);
  }
  else
  {
    if (cd->_serviceWasUp)
    {
      CPPUNIT_ASSERT(core->_subscribeState == SQAOpOK);
      CPPUNIT_ASSERT(core->_subscribeAttempts >= 1);
    }
    else
    {
      CPPUNIT_ASSERT(core->_subscribeState == SQAOpNotDone);
      CPPUNIT_ASSERT(core->_subscribeAttempts == 0);
    }
  }

  if (SQAUtil::isWatcher(cd->_type) && core->_subscribeState == SQAOpOK)
  {
    CPPUNIT_ASSERT(!core->_publisherAddress.empty());
  }
  else
  {
    CPPUNIT_ASSERT(core->_publisherAddress.empty());
  }

}

void SQAClientUtil::checkSQAClientCoreAfterStop(StateQueueClient::SQAClientCore* core)
{
  CPPUNIT_ASSERT(core->_terminate == true);
  CPPUNIT_ASSERT(core->_zmqContext == 0);
  CPPUNIT_ASSERT(core->_zmqSocket == 0);
  CPPUNIT_ASSERT(core->_pEventThread == 0);
  CPPUNIT_ASSERT(core->_keepAliveTimer == 0);
  CPPUNIT_ASSERT(core->_signinTimer == 0);
}

void SQAClientUtil::checkStateQueueClientAfterStartup(StateQueueClient& client, SQAClientData* cd, int coreIdx)
{
  try{
  CPPUNIT_ASSERT(client._type == cd->_type);
  CPPUNIT_ASSERT(client._applicationId  == cd->_applicationId);

  CPPUNIT_ASSERT( std::string::npos != cd->_servicesAddresses.find(client._serviceAddress));
  CPPUNIT_ASSERT( std::string::npos != cd->_servicesAddressesAll.find(client._serviceAddress));
  CPPUNIT_ASSERT(client._servicePort == cd->_servicePort);

  CPPUNIT_ASSERT(client._terminate == false);
  //std::string _zmqEventId;
  CPPUNIT_ASSERT(client._rawEventId == cd->_zmqEventId);

  CPPUNIT_ASSERT(client._poolSize == cd->_poolSize);
  CPPUNIT_ASSERT(client._readTimeout == cd->_readTimeout);
  CPPUNIT_ASSERT(client._writeTimeout == cd->_writeTimeout);
  CPPUNIT_ASSERT(client._keepAliveTimeout == cd->_keepAliveTimeout);
  CPPUNIT_ASSERT(client._signinTimeout == cd->_signinTimeout);

  CPPUNIT_ASSERT(client.getEventQueue() == &client._eventQueue);
  CPPUNIT_ASSERT(client._expires  == 10);
  CPPUNIT_ASSERT(client._core);
  CPPUNIT_ASSERT(client._core == client._cores[coreIdx]);
  CPPUNIT_ASSERT(client.getIoService() == &client._ioService);
  CPPUNIT_ASSERT(0 == strcmp(client.getClassName(), "StateQueueClient"));

  CPPUNIT_ASSERT(client._pIoServiceThread);
  CPPUNIT_ASSERT(client._clientConfig.empty());

  checkFallbackDataAFterStartup(client, cd);
  }
  catch (std::exception&e)
  {
    std::cout << e.what() <<std::endl;
  }
}

void SQAClientUtil::checkFallbackDataAFterStartup(StateQueueClient& client, SQAClientData* cd)
{
  if (SQAUtil::isWatcher(cd->_type) || !cd->_fallbackDataSet)
  {
    CPPUNIT_ASSERT(client._fallbackServicesAddresses.size() == 0);
    CPPUNIT_ASSERT(client._fallbackServiceIdx == 0);
    CPPUNIT_ASSERT(client._fallbackTimeout == 10);
    CPPUNIT_ASSERT(!client._fallbackTimer);
    CPPUNIT_ASSERT(client._currentFailedConnects == 0);
    CPPUNIT_ASSERT(!client._isFallbackActive);
  }
  else if (cd->_fallbackDataSet)
  {
    for (unsigned int i = 0; i < client._fallbackServicesAddresses.size(); i++)
    {
      CPPUNIT_ASSERT(std::string::npos != cd->_servicesAddressesAll.find(client._fallbackServicesAddresses[i]));
    }
    CPPUNIT_ASSERT(client._fallbackServiceIdx == 0);
    CPPUNIT_ASSERT(client._fallbackTimeout == cd->_fallbackTimeout);
    CPPUNIT_ASSERT(client._fallbackTimer);
    if (cd->_serviceIsUp)
    {
      CPPUNIT_ASSERT(client._currentFailedConnects == 0);
    }
    else
    {
      CPPUNIT_ASSERT(client._currentFailedConnects >= 1);
    }

    CPPUNIT_ASSERT(client._isFallbackActive == cd->_fallbackActive);
  }
}

void SQAClientUtil::checkSQAClientAfterStop(StateQueueClient& client)
{
  checkSQAClientCoreAfterStop(client._core);

  CPPUNIT_ASSERT(client._terminate == true);

  CPPUNIT_ASSERT(!client._fallbackTimer);
}

void SQAClientUtil::checkPublisherAfterStartup(
    StateQueueClient& publisher,
    SQAClientData* cd)
{
  checkStateQueueClientAfterStartup(publisher, cd);
  CPPUNIT_ASSERT(publisher._cores.size() == 1);

  checkSQAClientCoreAfterStartup(&publisher, publisher._core, cd);
}

void SQAClientUtil::checkPublisherFallback(
    StateQueueClient& publisher,
    std::vector<SQAClientData*> cd, int fallbackIdx)
{
  CPPUNIT_ASSERT(publisher._cores.size() == cd.size());

  if (fallbackIdx > -1)
  {
    CPPUNIT_ASSERT(publisher._cores[fallbackIdx] == publisher._core);
  }

  checkStateQueueClientAfterStartup(publisher, cd[fallbackIdx], fallbackIdx);

  for (unsigned int i = 0; i < cd.size(); i++)
  {
    checkSQAClientCoreAfterStartup(&publisher, publisher._cores[i], cd[i]);
  }
}

void SQAClientUtil::checkDealerAfterStartup(
    StateQueueClient& publisher,
    SQAClientData* cd)
{
  checkPublisherAfterStartup(publisher, cd);
}

void SQAClientUtil::checkWatcherAfterStartup(StateQueueClient& watcher,
    std::vector<SQAClientData*> cd)

{
  checkStateQueueClientAfterStartup(watcher, cd[0]);

  CPPUNIT_ASSERT(watcher._cores.size() == cd.size());

  for (unsigned int i = 0; i < cd.size(); i++)
  {
    checkSQAClientCoreAfterStartup(&watcher, watcher._cores[i], cd[i]);
  }
}

void SQAClientUtil::checkWorkerAfterStartup(StateQueueClient& worker,
    std::vector<SQAClientData*> cd)

{
  checkWatcherAfterStartup(worker, cd);
}

void SQAClientUtil::checkRegularPublishWatch(StateQueueClient& publisher, StateQueueClient& watcher, const std::string& eventId, const std::string& data, bool noresponse)
{
  std::string wEventId;
  std::string wEventData;

 // TEST: Regular no-response publish / watch should work
 CPPUNIT_ASSERT(publisher.publish(eventId, data, noresponse));
 CPPUNIT_ASSERT(watcher.watch(wEventId, wEventData, 50));

 std::string sqwEventId = "sqw." + eventId;
 // TEST: Watched event id matches the published event id
 CPPUNIT_ASSERT(boost::starts_with(wEventId.c_str(), sqwEventId));
 // TEST: Verify that the eventId has the proper format with sqw.eventId.hex4-hex4
 CPPUNIT_ASSERT(SQAUtil::validateId(wEventId, SQAUtil::SQAClientPublisher, eventId));
 // TEST: Verify that received data match what was sent
 CPPUNIT_ASSERT(0 == wEventData.compare(data));
}

void SQAClientUtil::checkRegularEnqueuePop(StateQueueClient& dealer, StateQueueClient& worker, const std::string& eventId, const std::string& data, int serviceId)
{
  std::string wEventId;
  std::string wEventData;
  int wServiceId=0;


 // TEST: Regular enqueue/pop should work
 CPPUNIT_ASSERT(dealer.enqueue(data));
 CPPUNIT_ASSERT(worker.pop(wEventId, wEventData, wServiceId, 50));

 std::string sqaEventId = "sqa." + eventId;
 // TEST: Popped event id matches the enqueued event id
  CPPUNIT_ASSERT(boost::starts_with(wEventId.c_str(), sqaEventId));
 // TEST: Verify that the eventId has the proper format with sqa.eventId.hex4-hex4
 CPPUNIT_ASSERT(SQAUtil::validateId(wEventId, SQAUtil::SQAClientDealer, eventId));
 // TEST: Verify that received data match what was sent
 CPPUNIT_ASSERT(0 == wEventData.compare(data));

 CPPUNIT_ASSERT(serviceId == wServiceId);
 CPPUNIT_ASSERT(worker.erase(wEventId, wServiceId));
}

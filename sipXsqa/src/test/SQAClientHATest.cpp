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

class SQAClientHATest : public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE(SQAClientHATest);
//  CPPUNIT_TEST(OnePublisherTwoHAAgentsOneWatcherTest);
//  CPPUNIT_TEST(TwoDealersTwoHAAgentsOneWorkerTest);
//  CPPUNIT_TEST(OnePublisherThreeHAAgentsFallbackOneWatcherTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _util.setProgram("/usr/local/sipxecs-local-feature-branch/bin/sipxsqa");
  }

  void tearDown()
  {
    std::vector<SQAAgentData::Ptr>::iterator it;
    for (it = _agentsData.begin(); it != _agentsData.end(); it++)
    {
      SQAAgentData::Ptr data = *it;

      _util.stopSQAAgent(data);
    }

    _agentsData.clear();
  }

  void createClient(std::string appId, int type, SQAAgentData::Ptr agentData, StateQueueClient** client, SQAClientData** _pd)
  {
    SQAClientData* pd = new SQAClientData(0, type, appId, agentData->sqaControlAddress, agentData->sqaControlAddressAll, agentData->sqaControlPort, "reg",1,SQA_CONN_READ_TIMEOUT, SQA_CONN_WRITE_TIMEOUT, 2, 2, 2);
    *_pd = pd;
    *client = new StateQueueClient(pd->_type, pd->_applicationId, pd->_servicesAddresses, pd->_servicePort, pd->_zmqEventId, pd->_poolSize, pd->_readTimeout, pd->_writeTimeout, pd->_keepAliveTimeout, pd->_signinTimeout);
  }

  void createClient(std::string appId, int type, std::vector<SQAAgentData::Ptr>& agentsData, StateQueueClient **client,  std::vector<SQAClientData*>& wvec)
  {
    for (unsigned int i=0; i < agentsData.size(); i++)
    {
      wvec.push_back(new SQAClientData(i, type, appId, agentsData[i]->sqaControlAddress, agentsData[i]->sqaControlAddressAll, agentsData[i]->sqaControlPort, "reg",1,SQA_CONN_READ_TIMEOUT, SQA_CONN_WRITE_TIMEOUT, 2, 2, 2));
    }
    SQAClientData* wd = wvec[0];
    *client = new StateQueueClient(wd->_type, wd->_applicationId, wd->_servicesAddressesAll, wd->_servicePort, wd->_zmqEventId, wd->_poolSize, wd->_readTimeout, wd->_writeTimeout, wd->_keepAliveTimeout, wd->_signinTimeout);
  }

  void OnePublisherTwoHAAgentsOneWatcherTest()
  {
    _agentsData.clear();
    _util.generateSQAAgentData(_agentsData, 2, true);

    // prepare two publishers for events of type "reg"
    SQAClientData* pd;
    StateQueueClient* publisher;
    createClient("Publisher", SQAUtil::SQAClientPublisher, _agentsData[0], &publisher, &pd);
    SQAClientData* pd2;
    StateQueueClient* publisher2;
    createClient("Publisher2", SQAUtil::SQAClientPublisher, _agentsData[1], &publisher2, &pd2);

    // prepare a watcher for events of type "reg"
    SQAClientData* wd0;
    SQAClientData* wd1;
    std::vector<SQAClientData*> wdVec;
    StateQueueClient* watcher;
    createClient("Watcher", SQAUtil::SQAClientWatcher, _agentsData, &watcher, wdVec);
    wd0 = wdVec[0]; wd1 = wdVec[1];

    // TEST: All 3 clients started but cannot connect to agent (agent is not started yet)
    pd->setServiceDown();
    SQAClientUtil::checkPublisherAfterStartup(*publisher, pd);
    pd2->setServiceDown();
    SQAClientUtil::checkPublisherAfterStartup(*publisher2, pd2);
    wd0->setServiceDown(); wd1->setServiceDown();
    SQAClientUtil::checkWatcherAfterStartup(*watcher, wdVec);

    // start agents and give them time
    CPPUNIT_ASSERT(_util.startSQAAgent(_agentsData[0]));
    CPPUNIT_ASSERT(_util.startSQAAgent(_agentsData[1]));
    boost::this_thread::sleep(boost::posix_time::milliseconds(3500));

    // TEST: All 3 clients connected to agents
    pd->setServiceUp();
    SQAClientUtil::checkPublisherAfterStartup(*publisher, pd);
    pd2->setServiceUp();
    SQAClientUtil::checkPublisherAfterStartup(*publisher2, pd2);
    wd0->setServiceUp(); wd1->setServiceUp();
    SQAClientUtil::checkWatcherAfterStartup(*watcher, wdVec);

    // TEST: Regular publish watch should work for both publishers and watcher
    SQAClientUtil::checkRegularPublishWatch(*publisher, *watcher, "reg", "reg.1.data", true);
    SQAClientUtil::checkRegularPublishWatch(*publisher, *watcher, "reg", "reg.1.data", false);
    SQAClientUtil::checkRegularPublishWatch(*publisher2, *watcher, "reg", "reg.2.data", true);
    SQAClientUtil::checkRegularPublishWatch(*publisher2, *watcher, "reg", "reg.2.data", false);

    //cleanup
    publisher->terminate();
    SQAClientUtil::checkSQAClientAfterStop(*publisher);
    delete publisher; delete pd;

    publisher2->terminate();
    SQAClientUtil::checkSQAClientAfterStop(*publisher2);
    delete publisher2; delete pd2;

    watcher->terminate();
    SQAClientUtil::checkSQAClientAfterStop(*watcher);
    delete watcher; delete wd0; delete wd1;

    CPPUNIT_ASSERT(_util.stopSQAAgent(_agentsData[0]));
    CPPUNIT_ASSERT(_util.stopSQAAgent(_agentsData[1]));
    _agentsData.clear();
  }

  void OnePublisherThreeHAAgentsFallbackOneWatcherTest()
  {
    _agentsData.clear();
    _util.generateSQAAgentData(_agentsData, 2, true);

    // prepare a publisher for events of type "reg"
    SQAClientData* pd;
    StateQueueClient* publisher;
    createClient("Publisher", SQAUtil::SQAClientPublisher, _agentsData[0], &publisher, &pd);
    SQAClientData* pd1 = new SQAClientData(1, SQAUtil::SQAClientPublisher, "Publisher", _agentsData[1]->sqaControlAddress, _agentsData[1]->sqaControlAddressAll, _agentsData[1]->sqaControlPort, "reg",1,SQA_CONN_READ_TIMEOUT, SQA_CONN_WRITE_TIMEOUT, 2, 2, 2);
    std::vector<SQAClientData*> pdVec;
    pdVec.push_back(pd);

    // prepare a watcher for events of type "reg"
    SQAClientData* wd0;
    SQAClientData* wd1;
    std::vector<SQAClientData*> wdVec;
    StateQueueClient* watcher;
    createClient("Watcher", SQAUtil::SQAClientWatcher, _agentsData, &watcher, wdVec);
    wd0 = wdVec[0]; wd1 = wdVec[1];

    // TEST: Both clients started but cannot connect to agent (agent is not started yet)
    pd->setServiceDown(); pd1->setServiceDown();
    SQAClientUtil::checkPublisherAfterStartup(*publisher, pd);
    wd0->setServiceDown(); wd1->setServiceDown();
    SQAClientUtil::checkWatcherAfterStartup(*watcher, wdVec);

    // start agents and give them time to start
    CPPUNIT_ASSERT(_util.startSQAAgent(_agentsData[0]));
    CPPUNIT_ASSERT(_util.startSQAAgent(_agentsData[1]));
    boost::this_thread::sleep(boost::posix_time::milliseconds(4000));

    // TEST: Both clients connected to agents
    pd->setServiceUp(); pd1->setServiceUp();
    SQAClientUtil::checkPublisherAfterStartup(*publisher, pd);
    wd0->setServiceUp(); wd1->setServiceUp();
    SQAClientUtil::checkWatcherAfterStartup(*watcher, wdVec);

    // TEST: Regular publish/watch should work
    SQAClientUtil::checkRegularPublishWatch(*publisher, *watcher, "reg", "reg.1.data", true);

    // TEST: set publisher fallback to the second agent
    CPPUNIT_ASSERT(publisher->setFallbackServices(pd->_servicesAddressesAll, 2));
    pd->setHasFallbackData(); pd1->setHasFallbackData();
    SQAClientUtil::checkFallbackDataAFterStartup(*publisher, pd);

    //stop first agent and give publisher time to initiate fallback to 2nd agent
    CPPUNIT_ASSERT(_util.stopSQAAgent(_agentsData[0]));
    boost::this_thread::sleep(boost::posix_time::milliseconds(4000));

    //TEST: publisher should have done fallback to 2nd agent
    pd->setServiceDown(); pdVec.push_back(pd1);
    pd->setFallbackActive();pd1->setFallbackActive();
    SQAClientUtil::checkPublisherFallback(*publisher, pdVec, 1);

    // TEST: Regular publish watch should work after fallback
    SQAClientUtil::checkRegularPublishWatch(*publisher, *watcher, "reg", "reg.1.data", true);

    // stop 2nd agent and resurect first agent and give it time to start
    CPPUNIT_ASSERT(_util.stopSQAAgent(_agentsData[1]));
    CPPUNIT_ASSERT(_util.startSQAAgent(_agentsData[0]));
    boost::this_thread::sleep(boost::posix_time::milliseconds(3000));

    //TEST: publisher should have connected back to first agent
    pd->setServiceUp();pd1->setServiceDown();
    pd->setFallbackInactive();pd1->setFallbackInactive();
    SQAClientUtil::checkPublisherFallback(*publisher, pdVec, 0);

    // TEST: Regular publish watch should work after publisher got back
    SQAClientUtil::checkRegularPublishWatch(*publisher, *watcher, "reg", "reg.1.data", true);

    //cleanup
    publisher->terminate();
    SQAClientUtil::checkSQAClientAfterStop(*publisher);
    delete publisher; delete pd; delete pd1;

    watcher->terminate();
    SQAClientUtil::checkSQAClientAfterStop(*watcher);
    delete watcher; delete wd0; delete wd1;

    CPPUNIT_ASSERT(_util.stopSQAAgent(_agentsData[0]));
    _agentsData.clear();
  }

  void TwoDealersTwoHAAgentsOneWorkerTest()
  {
    _agentsData.clear();
    _util.generateSQAAgentData(_agentsData, 2, true);

    // prepare two dealers for events of type "reg"
    SQAClientData* dd;
    StateQueueClient* dealer;
    createClient("Dealer", SQAUtil::SQAClientDealer, _agentsData[0], &dealer, &dd);
    SQAClientData* dd2;
    StateQueueClient* dealer2;
    createClient("Dealer2", SQAUtil::SQAClientDealer, _agentsData[1], &dealer2, &dd2);

    // prepare a worker for events of type "reg"
    SQAClientData* wd0;
    SQAClientData* wd1;
    std::vector<SQAClientData*> wdVec;
    StateQueueClient* worker;
    createClient("Worker", SQAUtil::SQAClientWorker, _agentsData, &worker, wdVec);
    wd0 = wdVec[0]; wd1 = wdVec[1];

    // TEST: All 3 clients started but cannot connect to agent (agent is not started yet)
    dd->setServiceDown();
    SQAClientUtil::checkDealerAfterStartup(*dealer, dd);
    dd2->setServiceDown();
    SQAClientUtil::checkDealerAfterStartup(*dealer2, dd2);
    wd0->setServiceDown(); wd1->setServiceDown();
    SQAClientUtil::checkWorkerAfterStartup(*worker, wdVec);

    //start agents and give them time to start
    CPPUNIT_ASSERT(_util.startSQAAgent(_agentsData[0]));
    CPPUNIT_ASSERT(_util.startSQAAgent(_agentsData[1]));
    boost::this_thread::sleep(boost::posix_time::milliseconds(5500));

    // TEST: All 3 clients connected to agents
    dd->setServiceUp();
    SQAClientUtil::checkDealerAfterStartup(*dealer, dd);
    dd2->setServiceUp();
    SQAClientUtil::checkDealerAfterStartup(*dealer2, dd2);
    wd0->setServiceUp(); wd1->setServiceUp();
    SQAClientUtil::checkWorkerAfterStartup(*worker, wdVec);

    // TEST: Regular enqueue/pop should work
    SQAClientUtil::checkRegularEnqueuePop(*dealer, *worker, "reg", "reg.data", 0);
    SQAClientUtil::checkRegularEnqueuePop(*dealer2, *worker, "reg", "reg2.data", 1);

    //cleanup
    dealer->terminate();
    SQAClientUtil::checkSQAClientAfterStop(*dealer);
    delete dealer; delete dd;

    dealer2->terminate();
    SQAClientUtil::checkSQAClientAfterStop(*dealer2);
    delete dealer2; delete dd2;

    worker->terminate();
    SQAClientUtil::checkSQAClientAfterStop(*worker);
    delete worker; delete wd0; delete wd1;

    CPPUNIT_ASSERT(_util.stopSQAAgent(_agentsData[0]));
    CPPUNIT_ASSERT(_util.stopSQAAgent(_agentsData[1]));
    _agentsData.clear();
  }

  std::vector<SQAAgentData::Ptr> _agentsData;
  SQAAgentUtil _util;
};


CPPUNIT_TEST_SUITE_REGISTRATION(SQAClientHATest);

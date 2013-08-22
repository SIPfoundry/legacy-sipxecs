#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <sipxunit/TestUtilities.h>
#include <sipdb/SubscribeDB.h>
#include <sipdb/SubscribeExpireThread.h>
#include <sipdb/MongoDB.h>
#include <os/OsDateTime.h>
#include <mongo/util/net/hostandport.h>
#include <mongo/client/connpool.h>

#include <boost/format.hpp>


using namespace std;

extern mongo::DBConnectionPool pool;

class SubscribeExpireThreadTest: public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE(SubscribeExpireThreadTest);
  CPPUNIT_TEST(testSubscribeExpireThread_Run);
  CPPUNIT_TEST_SUITE_END();

  SubscribeDB* _db;
  const MongoDB::ConnectionInfo _info;
  int _timeNow;
  const std::string _databaseName;
public:
  SubscribeExpireThreadTest() : _info(MongoDB::ConnectionInfo(mongo::ConnectionString(mongo::HostAndPort("localhost")))),
                                _databaseName("test.SubscribeExpireThreadTest")
  {
  }

  void setUp()
  {
    _timeNow = (int) OsDateTime::getSecsSinceEpoch();

    _db = new SubscribeDB(_info, NULL, _databaseName);
    MongoDB::ScopedDbConnectionPtr pConn(mongo::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));
    pConn->get()->remove(_databaseName, mongo::Query());
    pConn->done();
  }

  void tearDown()
  {
    delete _db;
    _db = 0;
  }

  void testSubscribeExpireThread_upsertAndGetAll()
  {
    _db->upsert("component", "uri", "callId_0", "contact", _timeNow + 3600,
                1067, "eventTypeKey_0", "eventType", "id_0", "toUri_0", "fromUri_0",
                "key", "recordRoute", 1066, "accept", 20);

    SubscribeDB::Subscriptions subscriptions;
    _db->getAll(subscriptions);

    CPPUNIT_ASSERT(subscriptions.size() == 1);
  }


  void testSubscribeExpireThread_Run()
  {
    testSubscribeExpireThread_upsertAndGetAll();

    // create a new Subscription entry that will expire in 1 second
    _db->upsert("component_1", "uri_1", "callId_1", "contact_1", _timeNow + 1,
                1067, "eventTypeKey_1", "eventType_1", "id_1", "toUri_1", "fromUri_1",
                "key", "recordRoute_1", 1067, "accept_1", 20);

    SubscribeDB::Subscriptions subscriptions;
    _db->getAll(subscriptions);


    CPPUNIT_ASSERT(subscriptions.size() == 2);

    SubscribeExpireThread subscribeExpireThread;

    // start subscribe Expire thread that will run every two second and will remove all expired records
    subscribeExpireThread.run(_db, 2);

    // wait 3 seconds to be sure that the thread removed all expired records
    sleep(3);

    //_db->removeExpired("component_1", _timeNow + 1802);
    subscriptions.clear();
    _db->getAll(subscriptions);

    // check that the number of entries in test.RegExpireThreadTest database is one
    CPPUNIT_ASSERT(subscriptions.size() == 1);

  }




};

CPPUNIT_TEST_SUITE_REGISTRATION(SubscribeExpireThreadTest);

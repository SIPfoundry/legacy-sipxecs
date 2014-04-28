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

#include "MongoDbVerifier.h"


using namespace std;

const char* gLocalHostAddr = "localhost";
const char* gDatabaseName = "test.SubscribeExpireThreadTest";

typedef struct
{
  const char* pOid;
  const char* pComponent;
  const char* pUri;
  const char* pCallId;
  const char* pContact;
  const char* pEventTypeKey;
  const char* pEventType;
  const char* pId;
  const char* pToUri;
  const char* pFromUri;
  const char* pKey;
  const char* pRecordRoute;
  const char* pAccept;
  const char* pFile;
  unsigned int notifyCseq;
  unsigned int subscribeCseq;
  unsigned int version;
  unsigned int expires;
} SubscriptionTestData;

SubscriptionTestData subscriptionTestData[] =
{
  {
      "12ab1c4c101212ab1c4c1012",
      "component",
      "uri",
      "callId_0",
      "contact",
      "eventTypeKey_0",
      "eventType",
      "id_0",
      "toUri_0",
      "fromUri_0",
      "key",
      "recordRoute",
      "accept",
      "file",
      1066,
      1067,
      20,
      3600
  },
  {
      "12ab1c4c101212ab1c4c1013",
      "component_1",
      "uri_1",
      "callId_1",
      "contact_1",
      "eventTypeKey_1",
      "eventType_1",
      "id_1",
      "toUri_1",
      "fromUri_1",
      "key",
      "recordRoute_1",
      "accept_1",
      "file",
      1067,
      1067,
      20,
      2
  }
};

class SubscribeExpireThreadTest: public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE(SubscribeExpireThreadTest);
  CPPUNIT_TEST(testSubscribeExpireThread_Run);
  CPPUNIT_TEST_SUITE_END();

  SubscribeDB* _db;
  const MongoDB::ConnectionInfo _info;
  int _timeNow;
  const std::string _databaseName;
  int MAX_SECONDS_TO_WAIT;
public:
  SubscribeExpireThreadTest() : _info(MongoDB::ConnectionInfo(mongo::ConnectionString(mongo::HostAndPort(gLocalHostAddr)))),
                                _databaseName(gDatabaseName)
  {
  }

  void setUp()
  {
    MAX_SECONDS_TO_WAIT = 10;

    _db = new SubscribeDB(_info, NULL, _databaseName);
    MongoDB::ScopedDbConnectionPtr pConn(mongoMod::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));
    pConn->get()->remove(_databaseName, mongo::Query());

    MongoDbVerifier _mongoDbVerifier(pConn, _databaseName, MAX_SECONDS_TO_WAIT * 1000);
    _mongoDbVerifier.waitUtilEmpty();

    pConn->done();

    _timeNow = (int) OsDateTime::getSecsSinceEpoch();
  }

  void tearDown()
  {
    delete _db;
    _db = 0;
  }

  void upsertSubscriptionTestData(int index)
  {
    _db->upsert(subscriptionTestData[index].pComponent,
                subscriptionTestData[index].pUri,
                subscriptionTestData[index].pCallId,
                subscriptionTestData[index].pContact,
                _timeNow + subscriptionTestData[index].expires,
                subscriptionTestData[index].subscribeCseq,
                subscriptionTestData[index].pEventTypeKey,
                subscriptionTestData[index].pEventType,
                subscriptionTestData[index].pId,
                subscriptionTestData[index].pToUri,
                subscriptionTestData[index].pFromUri,
                subscriptionTestData[index].pKey,
                subscriptionTestData[index].pRecordRoute,
                subscriptionTestData[index].notifyCseq,
                subscriptionTestData[index].pAccept,
                subscriptionTestData[index].version);
  }

  void testSubscribeExpireThread_upsertAndGetAll()
  {
    upsertSubscriptionTestData(0);

    SubscribeDB::Subscriptions subscriptions;
    _db->getAll(subscriptions);

    CPPUNIT_ASSERT(subscriptions.size() == 1);
  }

  void testSubscribeExpireThread_Run()
  {
    upsertSubscriptionTestData(0);

    // create a new Subscription entry that will expire in 2 seconds
    upsertSubscriptionTestData(1);

    SubscribeDB::Subscriptions subscriptions;

    int seconds = 0;
    while (subscriptions.size() < 2 && seconds < MAX_SECONDS_TO_WAIT)
    {
      subscriptions.clear();
      _db->getAll(subscriptions);
      sleep(1);
      seconds++;
    }

    // TEST: Check that the number of entries in test.RegExpireThreadTest database is two
    CPPUNIT_ASSERT(subscriptions.size() == 2);

    SubscribeExpireThread subscribeExpireThread;

    // start subscribe Expire thread that will run every two second and will remove all expired records
    subscribeExpireThread.run(_db, 2);

    // wait until 10 seconds to be sure that the thread removed all expired records
    seconds = 0;
    while (subscriptions.size() != 1  && seconds < MAX_SECONDS_TO_WAIT)
    {
      subscriptions.clear();
      _db->getAll(subscriptions);
      sleep(1);
      seconds++;
    }

    // TEST: Check that the number of entries in test.RegExpireThreadTest database is one
    CPPUNIT_ASSERT(subscriptions.size() == 1);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SubscribeExpireThreadTest);

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <sipxunit/TestUtilities.h>
#include <sipdb/SubscribeDB.h>
#include <sipdb/MongoDB.h>
#include <os/OsDateTime.h>
#include <mongo/util/net/hostandport.h>
#include <mongo/client/connpool.h>

#include <boost/format.hpp>


using namespace std;

extern mongo::DBConnectionPool pool;

class SubscribeDBTest: public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE(SubscribeDBTest);
  CPPUNIT_TEST(testSubscribeDB_upsertAndGetAll);
  CPPUNIT_TEST(testSubscribeDB_remove);
  CPPUNIT_TEST(testSubscribeDB_removeError);
  CPPUNIT_TEST(testSubscribeDB_subscriptionExists);
  CPPUNIT_TEST(testSubscribeDB_removeExpired);
  CPPUNIT_TEST(testSubscribeDB_getUnexpiredSubscriptions);
  CPPUNIT_TEST(testSubscribeDB_getUnexpiredContactsFieldsContaining);
  CPPUNIT_TEST(testSubscribeDB_updateNotifyUnexpiredSubscription);
  CPPUNIT_TEST(testSubscribeDB_getMaxVersion);
  CPPUNIT_TEST(testSubscribeDB_updateToTag);
  CPPUNIT_TEST(testSubscribeDB_findFromAndTo);
  CPPUNIT_TEST_SUITE_END();

  SubscribeDB* _db;
  const MongoDB::ConnectionInfo _info;
  int _timeNow;
  const std::string _databaseName;
public:
  SubscribeDBTest() : _info(MongoDB::ConnectionInfo(mongo::ConnectionString(mongo::HostAndPort("localhost")))),
                      _databaseName("test.SubscribeDBTest")
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

  void testSubscribeDB_upsertAndGetAll()
  {
    // insert a new entry in test.SubscribeDBTest database
    _db->upsert("component", "uri", "callId_0", "contact", _timeNow + 3600,
                1067, "eventTypeKey_0", "eventType", "id_0", "toUri_0", "fromUri_0",
                "key", "recordRoute", 1066, "accept", 20);

    SubscribeDB::Subscriptions subscriptions;
    _db->getAll(subscriptions);

    // check that returned number of subscriptions is one
    CPPUNIT_ASSERT(subscriptions.size() == 1);
  }

  void testSubscribeDB_remove()
  {
    // insert a new entry in test.SubscribeDBTest database
    _db->upsert("component", "uri", "callId_0", "contact", _timeNow + 3600,
                1067, "eventTypeKey_0", "eventType", "id_0", "toUri_0", "fromUri_0",
                "key", "recordRoute", 1068, "accept", 20);

    // remove the entry that match to specified component, toUri, fromUri, callId and
    // have the subscribeCseq lower then specified one
    _db->remove("component", "toUri_0", "fromUri_0", "callId_0", 1068);

    SubscribeDB::Subscriptions subscriptions;
    _db->getAll(subscriptions);

    // check that returned number of subscriptions is zero
    CPPUNIT_ASSERT(subscriptions.size() == 0);
  }

  void testSubscribeDB_removeError()
  {
    // insert a default entry in test.SubscribeDBTest database
    testSubscribeDB_upsertAndGetAll();

    // remove the entry that match to specified conditions
    _db->removeError("component", "toUri_0", "fromUri_0", "callId_0");

    SubscribeDB::Subscriptions subscriptions;
    _db->getAll(subscriptions);

    // check that returned number of subscriptions is zero
    CPPUNIT_ASSERT(subscriptions.size() == 0);
  }

  void testSubscribeDB_subscriptionExists()
  {
    // insert a default entry in test.SubscribeDBTest database
    testSubscribeDB_upsertAndGetAll();

    // check that the entry that match to filtered conditions exists
    CPPUNIT_ASSERT(_db->subscriptionExists("component", "toUri_0", "fromUri_0", "callId_0", _timeNow) == true);
  }

  void testSubscribeDB_removeExpired()
  {
    // insert a default entry in test.SubscribeDBTest database
    testSubscribeDB_upsertAndGetAll();

    // insert a new entry that will expire in 1800 seconds
    _db->upsert("component_1", "uri_1", "callId_1", "contact_1", _timeNow + 1800,
                1067, "eventTypeKey_1", "eventType_1", "id_1", "toUri_1", "fromUri_1",
                "key", "recordRoute_1", 1067, "accept_1", 20);

    SubscribeDB::Subscriptions subscriptions;
    _db->getAll(subscriptions);

    // check that returned size of subscriptions is two
    CPPUNIT_ASSERT(subscriptions.size() == 2);

    // remove the entries that have expire time lower than 1802 seconds
    _db->removeExpired("component_1", _timeNow + 1802);
    subscriptions.clear();
    _db->getAll(subscriptions);

    // check that returned number of subscriptions is one
    CPPUNIT_ASSERT(subscriptions.size() == 1);

  }

  void testSubscribeDB_getUnexpiredSubscriptions()
  {
    // insert a default entry in test.SubscribeDBTest database
    testSubscribeDB_upsertAndGetAll();

    SubscribeDB::Subscriptions subscriptions;

    // retrieve unexpired subscriptions
    _db->getUnexpiredSubscriptions("component", "key", "eventTypeKey_0", _timeNow, subscriptions);


    // check that returned number of subscriptions is one
    CPPUNIT_ASSERT(subscriptions.size() == 1);
  }

  void testSubscribeDB_getUnexpiredContactsFieldsContaining()
  {
    // insert a default entry in test.SubscribeDBTest database
    testSubscribeDB_upsertAndGetAll();

    std::vector<string> matchingContactFields;

    // get unexpired contacs that match to substringToMatch parameter
    UtlString substringToMatch("cont");
    _db->getUnexpiredContactsFieldsContaining(substringToMatch, _timeNow, matchingContactFields);

    // check that returned number of subscriptions is one
    CPPUNIT_ASSERT(matchingContactFields.size() == 1);
  }

  void testSubscribeDB_updateNotifyUnexpiredSubscription()
  {
    // insert a default entry in test.SubscribeDBTest database
    testSubscribeDB_upsertAndGetAll();

    // update the specified subscription
    _db->updateNotifyUnexpiredSubscription("component", "toUri_0", "fromUri_0", "callId_0",
                                          "eventTypeKey_0", "id_0", _timeNow, 1068, 22);

    SubscribeDB::Subscriptions subscriptions;
    _db->getAll(subscriptions);

    // check the updated values of the subscription
    CPPUNIT_ASSERT(subscriptions[0]._notifyCseq == 1068);
    CPPUNIT_ASSERT(subscriptions[0]._version == 22);
  }

  void testSubscribeDB_getMaxVersion()
  {
    // insert a default entry in test.SubscribeDBTest database
    testSubscribeDB_upsertAndGetAll();

    // insert a new entry in test.SubscribeDBTest database
    _db->upsert("component", "uri", "callId_1", "contact", _timeNow + 3600,
                1067, "eventTypeKey_1", "eventType", "id_1", "toUri_1", "fromUri_1",
                "key", "recordRoute", 1066, "accept", 23);

    SubscribeDB::Subscriptions subscriptions;
    _db->getAll(subscriptions);

    CPPUNIT_ASSERT(subscriptions.size() == 2);

    // check that the max version of subscriptions that have specified uri is 23
    CPPUNIT_ASSERT(_db->getMaxVersion("uri") == 23);
  }

  void testSubscribeDB_updateToTag()
  {
    // insert a new entry in test.SubscribeDBTest database
    _db->upsert("component", "uri", "callId_0", "contact", _timeNow + 3600,
                1067, "eventTypeKey_0", "eventType", "id_0", "user1@host1",
                "user2@host2;tag=from_tag", "key", "recordRoute", 1066, "accept", 20);


    // update the tag in toUri for the matching entry that have the specified from_tag
    _db->updateToTag("callId_0", "from_tag", "to_tag");


    SubscribeDB::Subscriptions subscriptions;
    _db->getAll(subscriptions);

    CPPUNIT_ASSERT("user2@host2;tag=from_tag" == subscriptions[0]._fromUri);
    CPPUNIT_ASSERT("<sip:user1@host1>;tag=to_tag" == subscriptions[0]._toUri);
  }

  void testSubscribeDB_findFromAndTo()
  {
    // insert a new entry in test.SubscribeDBTest database
    _db->upsert("component", "uri", "callId_0", "contact", _timeNow + 3600,
                1067, "eventTypeKey_0", "eventType", "id_0", "user1@host1;tag=to_tag",
                "user2@host2;tag=from_tag", "key", "recordRoute", 1066, "accept", 20);

    UtlString from;
    UtlString to;

    // Find fromUri and toUri that are matching to from_tag and to_tag
    _db->findFromAndTo("callId_0", "from_tag", "to_tag", from, to);

    SubscribeDB::Subscriptions subscriptions;
    _db->getAll(subscriptions);

    CPPUNIT_ASSERT(from.str() == subscriptions[0]._fromUri);
    CPPUNIT_ASSERT(to.str() == subscriptions[0]._toUri);
  }



};

CPPUNIT_TEST_SUITE_REGISTRATION(SubscribeDBTest);

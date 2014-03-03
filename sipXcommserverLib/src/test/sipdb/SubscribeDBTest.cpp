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

const char* gLocalHostAddr = "localhost";
const char* gDatabaseName = "test.SubscribeDBTest";
const char* gSubstringToMatch = "cont";
const char* gFromTag = "from_tag";
const char* gToTag = "to_tag";

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
      "recordRoute",
      "accept_1",
      "file",
      1067,
      1067,
      20,
      1800
  },
  {
      "12ab1c4c101212ab1c4c1014",
      "component_2",
      "uri",
      "callId_2",
      "contact_2",
      "eventTypeKey_2",
      "eventType_2",
      "id_2",
      "toUri_2",
      "fromUri_2",
      "key",
      "recordRoute",
      "accept_2",
      "file",
      1066,
      1067,
      23,
      3600
  },
  {
      "12ab1c4c101212ab1c4c1013",
      "component_3",
      "uri",
      "callId_3",
      "contact_3",
      "eventTypeKey_3",
      "eventType_3",
      "id_3",
      "<sip:user3@host3>",
      "user3@host3;tag=from_tag",
      "key",
      "recordRoute",
      "accept_3",
      "file",
      1066,
      1067,
      20,
      3600
  },
  {
      "12ab1c4c101212ab1c4c1017",
      "component_",
      "uri",
      "callId_4",
      "contact_4",
      "eventTypeKey_4",
      "eventType_4",
      "id_4",
      "user4@host4;tag=to_tag",
      "user4@host4;tag=from_tag",
      "key",
      "recordRoute",
      "accept_4",
      "file",
      1066,
      1067,
      20,
      3600
  }
};

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
  unsigned long _timeNow;
  const std::string _databaseName;
public:
  SubscribeDBTest() : _info(MongoDB::ConnectionInfo(mongo::ConnectionString(mongo::HostAndPort(gLocalHostAddr)))),
                      _databaseName(gDatabaseName)
  {
  }

  void setUp()
  {
    _timeNow = OsDateTime::getSecsSinceEpoch();

    _db = new SubscribeDB(_info, NULL, _databaseName);
    MongoDB::ScopedDbConnectionPtr pConn(mongoMod::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));
    pConn->get()->remove(_databaseName, mongo::Query());
    pConn->done();
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

  void testSubscribeDB_upsertAndGetAll()
  {
    // insert a new entry in test.SubscribeDBTest database
    upsertSubscriptionTestData(0);

    SubscribeDB::Subscriptions subscriptions;
    _db->getAll(subscriptions);

    // check that returned number of subscriptions is one
    CPPUNIT_ASSERT(subscriptions.size() == 1);
  }

  void testSubscribeDB_remove()
  {
    // insert a new entry in test.SubscribeDBTest database
    upsertSubscriptionTestData(0);

    // remove the entry that match to specified component, toUri, fromUri, callId and
    // have the subscribeCseq lower then specified one
    _db->remove(subscriptionTestData[0].pComponent,
                subscriptionTestData[0].pToUri,
                subscriptionTestData[0].pFromUri,
                subscriptionTestData[0].pCallId,
                subscriptionTestData[0].subscribeCseq + 1);

    SubscribeDB::Subscriptions subscriptions;
    _db->getAll(subscriptions);

    // TEST: Check that returned number of subscriptions is zero
    CPPUNIT_ASSERT(subscriptions.size() == 0);
  }

  void testSubscribeDB_removeError()
  {
    // insert a default entry in test.SubscribeDBTest database
    upsertSubscriptionTestData(0);

    // remove the entry that match to specified conditions
    _db->removeError(subscriptionTestData[0].pComponent,
                      subscriptionTestData[0].pToUri,
                      subscriptionTestData[0].pFromUri,
                      subscriptionTestData[0].pCallId);

    SubscribeDB::Subscriptions subscriptions;
    _db->getAll(subscriptions);

    // TEST: Check that returned number of subscriptions is zero
    CPPUNIT_ASSERT(subscriptions.size() == 0);
  }

  void testSubscribeDB_subscriptionExists()
  {
    // insert a default entry in test.SubscribeDBTest database
    upsertSubscriptionTestData(0);

    // TEST: Check that the entry that match to filtered conditions exists
    CPPUNIT_ASSERT(_db->subscriptionExists(subscriptionTestData[0].pComponent,
                                            subscriptionTestData[0].pToUri,
                                            subscriptionTestData[0].pFromUri,
                                            subscriptionTestData[0].pCallId,
                                            _timeNow) == true);
  }

  void testSubscribeDB_removeExpired()
  {
    // insert a default entry in test.SubscribeDBTest database
    upsertSubscriptionTestData(0);

    // insert a new entry that will expire in 1800 seconds
    upsertSubscriptionTestData(1);

    SubscribeDB::Subscriptions subscriptions;
    _db->getAll(subscriptions);

    // check that returned size of subscriptions is two
    CPPUNIT_ASSERT(subscriptions.size() == 2);

    // remove the entries that have expire time lower than 1802 seconds
    _db->removeExpired(subscriptionTestData[1].pComponent, _timeNow + subscriptionTestData[1].expires + 2);
    subscriptions.clear();
    _db->getAll(subscriptions);

    // TEST: Check that returned number of subscriptions is one
    CPPUNIT_ASSERT(subscriptions.size() == 1);

  }

  void testSubscribeDB_getUnexpiredSubscriptions()
  {
    // insert a default entry in test.SubscribeDBTest database
    upsertSubscriptionTestData(0);

    SubscribeDB::Subscriptions subscriptions;

    // retrieve unexpired subscriptions
    _db->getUnexpiredSubscriptions(subscriptionTestData[0].pComponent,
                                   subscriptionTestData[0].pKey,
                                   subscriptionTestData[0].pEventTypeKey,
                                   _timeNow,
                                   subscriptions);


    // TEST: Check that returned number of subscriptions is one
    CPPUNIT_ASSERT(subscriptions.size() == 1);
  }

  void testSubscribeDB_getUnexpiredContactsFieldsContaining()
  {
    // insert a default entry in test.SubscribeDBTest database
    upsertSubscriptionTestData(0);

    std::vector<string> matchingContactFields;

    // get unexpired contacs that match to substringToMatch parameter
    UtlString substringToMatch(gSubstringToMatch);
    _db->getUnexpiredContactsFieldsContaining(substringToMatch, _timeNow, matchingContactFields);

    // TEST: Check that returned number of subscriptions is one
    CPPUNIT_ASSERT(matchingContactFields.size() == 1);
  }

  void testSubscribeDB_updateNotifyUnexpiredSubscription()
  {
    const unsigned int newNotifyCseq = 1068;
    const unsigned int newVersion = 22;
    // insert a default entry in test.SubscribeDBTest database
    upsertSubscriptionTestData(0);

    // update the specified subscription
    _db->updateNotifyUnexpiredSubscription(subscriptionTestData[0].pComponent,
                                           subscriptionTestData[0].pToUri,
                                           subscriptionTestData[0].pFromUri,
                                           subscriptionTestData[0].pCallId,
                                           subscriptionTestData[0].pEventTypeKey,
                                           subscriptionTestData[0].pId,
                                           _timeNow,
                                           newNotifyCseq,
                                           newVersion);

    SubscribeDB::Subscriptions subscriptions;
    _db->getAll(subscriptions);

    // TEST: Check the updated values of the subscription
    CPPUNIT_ASSERT(subscriptions[0]._notifyCseq == newNotifyCseq);
    CPPUNIT_ASSERT(subscriptions[0]._version == newVersion);
  }

  void testSubscribeDB_getMaxVersion()
  {
    // insert a default entry in test.SubscribeDBTest database
    upsertSubscriptionTestData(0);

    // insert a new entry in test.SubscribeDBTest database
    upsertSubscriptionTestData(2);

    SubscribeDB::Subscriptions subscriptions;
    _db->getAll(subscriptions);

    CPPUNIT_ASSERT(subscriptions.size() == 2);

    // TEST: Check that the max version of subscriptions that have specified uri is 23
    CPPUNIT_ASSERT((unsigned int)_db->getMaxVersion("uri") == subscriptionTestData[2].version);
  }

  void testSubscribeDB_updateToTag()
  {
    // insert a new entry in test.SubscribeDBTest database
    upsertSubscriptionTestData(3);


    // update the tag in toUri for the matching entry that have the specified from_tag
    _db->updateToTag(subscriptionTestData[3].pCallId, gFromTag, gToTag);


    SubscribeDB::Subscriptions subscriptions;
    _db->getAll(subscriptions);

    std::string toUri = (boost::format("%s;tag=%s") % subscriptionTestData[3].pToUri % gToTag).str();

    // TEST: Check fromUri and toUri values
    CPPUNIT_ASSERT(subscriptionTestData[3].pFromUri == subscriptions[0]._fromUri);
    CPPUNIT_ASSERT(toUri == subscriptions[0]._toUri);
  }

  void testSubscribeDB_findFromAndTo()
  {
    // insert a new entry in test.SubscribeDBTest database
    upsertSubscriptionTestData(4);

    UtlString from;
    UtlString to;

    // Find fromUri and toUri that are matching to from_tag and to_tag
    _db->findFromAndTo(subscriptionTestData[4].pCallId, gFromTag, gToTag, from, to);

    SubscribeDB::Subscriptions subscriptions;
    _db->getAll(subscriptions);

    // TEST: Check fromUri and toUri values
    CPPUNIT_ASSERT(from.str() == subscriptions[0]._fromUri);
    CPPUNIT_ASSERT(to.str() == subscriptions[0]._toUri);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SubscribeDBTest);

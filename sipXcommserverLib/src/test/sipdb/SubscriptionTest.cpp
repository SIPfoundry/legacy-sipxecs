#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <sipxunit/TestUtilities.h>
#include <sipdb/Subscription.h>
#include <mongo/util/net/hostandport.h>
#include <mongo/client/connpool.h>


using namespace std;

const char* gLocalHostAddr = "localhost";
const char* gDatabaseName = "test.SubscriptionTest";

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
      "callId",
      "contact",
      "eventTypeKey",
      "eventType",
      "id",
      "toUri",
      "fromUri",
      "key",
      "recordRoute",
      "accept",
      "file",
      1066,
      1067,
      20,
      3600
  }
};

class SubscriptionTest: public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SubscriptionTest);
   CPPUNIT_TEST(testSubscriptionConstructor_WithSubscriptionParameter);
   CPPUNIT_TEST(testSubscriptionConstructor_WithSwapFunction);
   CPPUNIT_TEST(testSubscriptionConstructor_OperatorEqual_WithSubscriptionParameter);
   CPPUNIT_TEST(testSubscriptionConstructor_WithBSONObjParameter);
   CPPUNIT_TEST(testSubscriptionConstructor_OperatorEqual_WithBSONObjParameter);
   CPPUNIT_TEST_SUITE_END();

   Subscription* _db;
   const MongoDB::ConnectionInfo _info;
   std::string _databaseName;
public:
   SubscriptionTest() : _info(MongoDB::ConnectionInfo(mongo::ConnectionString(mongo::HostAndPort(gLocalHostAddr)))),
                       _databaseName(gDatabaseName)
   {
   }

   void setSubscription(Subscription& subscription)
   {
      subscription._oid = subscriptionTestData[0].pOid;
      subscription._component = subscriptionTestData[0].pComponent;
      subscription._uri = subscriptionTestData[0].pUri;
      subscription._callId = subscriptionTestData[0].pCallId;
      subscription._contact = subscriptionTestData[0].pContact;
      subscription._eventTypeKey = subscriptionTestData[0].pEventTypeKey;
      subscription._eventType = subscriptionTestData[0].pEventType;
      subscription._id = subscriptionTestData[0].pId;
      subscription._toUri = subscriptionTestData[0].pToUri;
      subscription._fromUri = subscriptionTestData[0].pFromUri;
      subscription._key = subscriptionTestData[0].pKey;
      subscription._recordRoute = subscriptionTestData[0].pRecordRoute;
      subscription._accept = subscriptionTestData[0].pAccept;
      subscription._file = subscriptionTestData[0].pFile;
      subscription._notifyCseq = subscriptionTestData[0].notifyCseq;
      subscription._subscribeCseq = subscriptionTestData[0].subscribeCseq;
      subscription._version = subscriptionTestData[0].version;
      subscription._expires = subscriptionTestData[0].expires;
   }

   void createBSONObj(Subscription& subscription,  mongo::BSONObj& bsonObj)
   {
      mongo::BSONObjBuilder bsonObjBuilder;

      mongo::OID oid(subscription.oid());

      bsonObjBuilder << subscription.oid_fld() << oid <<                            // "_id"
               subscription.component_fld() << subscription.component() <<          // "component"
               subscription.uri_fld() << subscription.uri() <<                      // "uri"
               subscription.callId_fld() << subscription.callId() <<                // "callId"
               subscription.contact_fld() << subscription.contact() <<              // "contact"
               subscription.eventTypeKey_fld() << subscription.eventTypeKey() <<    // "eventTypeKey"
               subscription.eventType_fld() << subscription.eventType() <<          // "eventType"
               subscription.id_fld() << subscription.id() <<                        // "id"
               subscription.toUri_fld() << subscription.toUri() <<                  // "toUri"
               subscription.fromUri_fld() << subscription.fromUri() <<              // "fromUri"
               subscription.key_fld() << subscription.key() <<                      // "key"
               subscription.recordRoute_fld() << subscription.recordRoute() <<      // "recordRoute"
               subscription.accept_fld() << subscription.accept() <<                // "accept"
               subscription.file_fld() << subscription.file() <<                    // "file"
               subscription.notifyCseq_fld() << subscription.notifyCseq() <<        // "notifyCseq"
               subscription.subscribeCseq_fld() << subscription.subscribeCseq() <<  // "subscribeCseq"
               subscription.version_fld() << subscription.version() <<              // "version"
               subscription.expires_fld() <<  subscription.expires();               // "expires"

      bsonObj = bsonObjBuilder.obj();
   }

   void setUp()
   {
      MongoDB::ScopedDbConnectionPtr pConn(mongo::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));
      pConn->get()->remove(_databaseName, mongo::Query());

      pConn->done();
   }

   void tearDown()
   {
      delete _db;
      _db = 0;
   }

   void testSubscriptionParameters(Subscription& subscription)
   {
      CPPUNIT_ASSERT(subscriptionTestData[0].pOid == subscription.oid());
      CPPUNIT_ASSERT(subscriptionTestData[0].pComponent == subscription.component());
      CPPUNIT_ASSERT(subscriptionTestData[0].pUri == subscription.uri());
      CPPUNIT_ASSERT(subscriptionTestData[0].pCallId == subscription.callId());
      CPPUNIT_ASSERT(subscriptionTestData[0].pContact == subscription.contact());
      CPPUNIT_ASSERT(subscriptionTestData[0].pEventTypeKey == subscription.eventTypeKey());
      CPPUNIT_ASSERT(subscriptionTestData[0].pEventType == subscription.eventType());
      CPPUNIT_ASSERT(subscriptionTestData[0].pId == subscription.id());
      CPPUNIT_ASSERT(subscriptionTestData[0].pToUri == subscription.toUri());
      CPPUNIT_ASSERT(subscriptionTestData[0].pFromUri == subscription.fromUri());
      CPPUNIT_ASSERT(subscriptionTestData[0].pKey == subscription.key());
      CPPUNIT_ASSERT(subscriptionTestData[0].pRecordRoute == subscription.recordRoute());
      CPPUNIT_ASSERT(subscriptionTestData[0].pAccept == subscription.accept());
      CPPUNIT_ASSERT(subscriptionTestData[0].pFile == subscription.file());
      CPPUNIT_ASSERT(subscriptionTestData[0].notifyCseq == subscription.notifyCseq());
      CPPUNIT_ASSERT(subscriptionTestData[0].subscribeCseq == subscription.subscribeCseq());
      CPPUNIT_ASSERT(subscriptionTestData[0].version == subscription.version());
      CPPUNIT_ASSERT(subscriptionTestData[0].expires == subscription.expires());

   }

   void testSubscriptionConstructor_WithSubscriptionParameter()
   {
      Subscription subscription;

      // init Subscription with default values
      setSubscription(subscription);


      // copy values using Subscription constructor
      Subscription subscriptionConstructor(subscription);

      // TEST: Check the values for the new created Subscription structure
      testSubscriptionParameters(subscriptionConstructor);
   }

   void testSubscriptionConstructor_WithSwapFunction()
   {
      Subscription subscription;

      // init Subscription with default values
      setSubscription(subscription);

      Subscription subscriptionConstructor;

      // use the swap function in order to copy values
      subscriptionConstructor.swap(subscription);

      // TEST: Check the values for the new created Subscription structure
      testSubscriptionParameters(subscriptionConstructor);
   }

   void testSubscriptionConstructor_OperatorEqual_WithSubscriptionParameter()
   {
      Subscription subscription;

      // init Subscription with default values
      setSubscription(subscription);

      // copy the values using the operator= function
      Subscription subscriptionConstructor = subscription;

      // TEST: Check the values for the new created Subscription structure
      testSubscriptionParameters(subscriptionConstructor);
   }

   void testSubscriptionConstructor_WithBSONObjParameter()
   {
      Subscription subscription;

      // init Subscription with default values
      setSubscription(subscription);

      mongo::BSONObj bsonObj;
      createBSONObj(subscription, bsonObj);

      // copy values using Subscription constructor
      Subscription subscriptionConstructor(bsonObj);

      // TEST: Check the values for the new created Subscription structure
      testSubscriptionParameters(subscriptionConstructor);
   }

   void testSubscriptionConstructor_OperatorEqual_WithBSONObjParameter()
   {
      Subscription subscription;

      // init Subscription with default values
      setSubscription(subscription);

      mongo::BSONObj bsonObj;
      createBSONObj(subscription, bsonObj);

      // copy the values using the operator= function
      Subscription subscriptionConstructor = bsonObj;

      // TEST: Check the values for the new created Subscription structure
      testSubscriptionParameters(subscriptionConstructor);
   }

};

CPPUNIT_TEST_SUITE_REGISTRATION(SubscriptionTest);

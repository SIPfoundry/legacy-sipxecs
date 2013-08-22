#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <sipxunit/TestUtilities.h>
#include <sipdb/Subscription.h>
#include <mongo/util/net/hostandport.h>
#include <mongo/client/connpool.h>


using namespace std;



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
   SubscriptionTest() : _info(MongoDB::ConnectionInfo(mongo::ConnectionString(mongo::HostAndPort("localhost")))),
                       _databaseName("test.RegDBTest")
   {
   }

   void setSubscription(Subscription& subscription)
   {
      subscription._oid = "12ab1c4c101212ab1c4c1012";
      subscription._component = "component";
      subscription._uri = "uri";
      subscription._callId = "callId";
      subscription._contact = "contact";
      subscription._eventTypeKey = "eventTypeKey";
      subscription._eventType = "eventType";
      subscription._id = "id";
      subscription._toUri = "toUri";
      subscription._fromUri = "fromUri";
      subscription._key = "key";
      subscription._recordRoute = "recordRoute";
      subscription._accept = "accept";
      subscription._file = "file";
      subscription._notifyCseq = 1066;
      subscription._subscribeCseq = 1067;
      subscription._version = 20;
      subscription._expires = 3600;
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

   void testSubscriptionWithCorrectParameters(Subscription& subscription)
   {
      CPPUNIT_ASSERT(string("12ab1c4c101212ab1c4c1012") == subscription.oid());
      CPPUNIT_ASSERT(string("component") == subscription.component());
      CPPUNIT_ASSERT(string("uri") == subscription.uri());
      CPPUNIT_ASSERT(string("callId") == subscription.callId());
      CPPUNIT_ASSERT(string("contact") == subscription.contact());
      CPPUNIT_ASSERT(string("eventTypeKey") == subscription.eventTypeKey());
      CPPUNIT_ASSERT(string("eventType") == subscription.eventType());
      CPPUNIT_ASSERT(string("id") == subscription.id());
      CPPUNIT_ASSERT(string("toUri") == subscription.toUri());
      CPPUNIT_ASSERT(string("fromUri") == subscription.fromUri());
      CPPUNIT_ASSERT(string("key") == subscription.key());
      CPPUNIT_ASSERT(string("recordRoute") == subscription.recordRoute());
      CPPUNIT_ASSERT(string("accept") == subscription.accept());
      CPPUNIT_ASSERT(string("file") == subscription.file());
      CPPUNIT_ASSERT((unsigned int)1066 == subscription.notifyCseq());
      CPPUNIT_ASSERT((unsigned int)1067 == subscription.subscribeCseq());
      CPPUNIT_ASSERT((unsigned int)20 == subscription.version());
      CPPUNIT_ASSERT((unsigned int)3600 == subscription.expires());

   }

   void testSubscriptionWithIncorrectParameters(Subscription& subscription)
   {
      CPPUNIT_ASSERT(string("12ab1c4c101212ab1c4c1013") != subscription.oid());
      CPPUNIT_ASSERT(string("__component") != subscription.component());
      CPPUNIT_ASSERT(string("__uri") != subscription.uri());
      CPPUNIT_ASSERT(string("__callId") != subscription.callId());
      CPPUNIT_ASSERT(string("__contact") != subscription.contact());
      CPPUNIT_ASSERT(string("__eventTypeKey") != subscription.eventTypeKey());
      CPPUNIT_ASSERT(string("__eventType") != subscription.eventType());
      CPPUNIT_ASSERT(string("__id") != subscription.id());
      CPPUNIT_ASSERT(string("__toUri") != subscription.toUri());
      CPPUNIT_ASSERT(string("__fromUri") != subscription.fromUri());
      CPPUNIT_ASSERT(string("__key") != subscription.key());
      CPPUNIT_ASSERT(string("__recordRoute") != subscription.recordRoute());
      CPPUNIT_ASSERT(string("__accept") != subscription.accept());
      CPPUNIT_ASSERT(string("__file") != subscription.file());
      CPPUNIT_ASSERT((unsigned int)10668 != subscription.notifyCseq());
      CPPUNIT_ASSERT((unsigned int)10678 != subscription.subscribeCseq());
      CPPUNIT_ASSERT((unsigned int)208 != subscription.version());
      CPPUNIT_ASSERT((unsigned int)36008 != subscription.expires());

   }

   void testSubscriptionConstructor_WithSubscriptionParameter()
   {
      Subscription subscription;

      // init Subscription with default values
      setSubscription(subscription);


      // copy values using Subscription constructor
      Subscription subscriptionConstructor(subscription);

      // check the values for the new created Subscription structure
      testSubscriptionWithCorrectParameters(subscriptionConstructor);
      testSubscriptionWithIncorrectParameters(subscriptionConstructor);
   }

   void testSubscriptionConstructor_WithSwapFunction()
   {
      Subscription subscription;

      // init Subscription with default values
      setSubscription(subscription);

      Subscription subscriptionConstructor;

      // use the swap function in order to copy values
      subscriptionConstructor.swap(subscription);

      // check the values for the new created Subscription structure
      testSubscriptionWithCorrectParameters(subscriptionConstructor);
      testSubscriptionWithIncorrectParameters(subscriptionConstructor);
   }

   void testSubscriptionConstructor_OperatorEqual_WithSubscriptionParameter()
   {
      Subscription subscription;

      // init Subscription with default values
      setSubscription(subscription);

      // copy the values using the operator= function
      Subscription subscriptionConstructor = subscription;

      // check the values for the new created Subscription structure
      testSubscriptionWithCorrectParameters(subscriptionConstructor);
      testSubscriptionWithIncorrectParameters(subscriptionConstructor);
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

      // check the values for the new created Subscription structure
      testSubscriptionWithCorrectParameters(subscriptionConstructor);
      testSubscriptionWithIncorrectParameters(subscriptionConstructor);
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

      // check the values for the new created Subscription structure
      testSubscriptionWithCorrectParameters(subscriptionConstructor);
      testSubscriptionWithIncorrectParameters(subscriptionConstructor);
   }

};

CPPUNIT_TEST_SUITE_REGISTRATION(SubscriptionTest);

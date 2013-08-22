#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <sipxunit/TestUtilities.h>
#include <sipdb/RegBinding.h>
#include <os/OsDateTime.h>
#include <mongo/util/net/hostandport.h>
#include <mongo/client/connpool.h>


using namespace std;



class RegBindingTest: public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(RegBindingTest);
   CPPUNIT_TEST(testBindingConstructor_WithRegBindingParameter);
   CPPUNIT_TEST(testBindingConstructor_WithSwapFunction);
   CPPUNIT_TEST(testBindingConstructor_OperatorEqual_WithRegBindingParameter);
   CPPUNIT_TEST(testBindingConstructor_WithBSONObjParameter);
   CPPUNIT_TEST(testBindingConstructor_OperatorEqual_WithBSONObjParameter);
   CPPUNIT_TEST_SUITE_END();

   RegBinding* _db;

   int _timeNow;
public:

   RegBindingTest()
   {
   }

   void setBinding(RegBinding& regBinding)
   {

      regBinding.setIdentity("alice@atlanta.com");
      regBinding.setUri("sip:alice@atalanta.com");
      regBinding.setCallId("hpwqlziwkduiqkg@alice.atlanta.com");
      regBinding.setContact("<sip:alice@atlanta.com;x-sipX-nonat>");
      regBinding.setQvalue("0");
      regBinding.setInstanceId("");
      regBinding.setGruu("");
      regBinding.setPath("sip:proxy01.atlanta.com");
      regBinding.setCseq(1066);
      regBinding.setExpirationTime(_timeNow + 3600);
      regBinding.setInstrument("instrument-test");
      regBinding.setLocalAddress("172.16.10.50/R");
      regBinding.setTimestamp(_timeNow);
      regBinding.setExpired(false);
   }

   void createBSONObj(const RegBinding& regBinding,  mongo::BSONObj& bsonObj)
   {
      mongo::BSONObjBuilder bsonObjBuilder;

      bsonObjBuilder << regBinding.identity_fld() << regBinding.getIdentity() <<          // "identity"
               regBinding.uri_fld() << regBinding.getUri() <<                             // "uri"
               regBinding.callId_fld() << regBinding.getCallId() <<                       // "callId"
               regBinding.contact_fld() << regBinding.getContact() <<                     // "contact"
               regBinding.qvalue_fld() << regBinding.getQvalue() <<                       // "qvalue"
               regBinding.instanceId_fld() << regBinding.getInstanceId() <<               // "instanceId"
               regBinding.gruu_fld() << regBinding.getGruu() <<                           // "gruu"
               regBinding.path_fld() << regBinding.getPath() <<                           // "path"
               regBinding.cseq_fld() << regBinding.getCseq() <<                           // "cseq"
               regBinding.expirationTime_fld() << regBinding.getExpirationTime() <<       // "expirationTime"
               regBinding.instrument_fld() << regBinding.getInstrument() <<               // "instrument"
               regBinding.localAddress_fld() << regBinding.getLocalAddress() <<           // "localAddress"
               regBinding.timestamp_fld() << regBinding.getTimestamp() <<                 // "timestamp"
               regBinding.expired_fld() << regBinding.getExpired();                       // "expired"

      bsonObj = bsonObjBuilder.obj();
   }

   void setUp()
   {
      _timeNow = (int) OsDateTime::getSecsSinceEpoch();
   }

   void tearDown()
   {
   }

   void testBindingWithCorrectParameters(RegBinding& regBinding)
   {
      CPPUNIT_ASSERT_EQUAL(string("alice@atlanta.com"), regBinding.getIdentity());
      CPPUNIT_ASSERT_EQUAL(string("sip:alice@atalanta.com"), regBinding.getUri());
      CPPUNIT_ASSERT_EQUAL(string("hpwqlziwkduiqkg@alice.atlanta.com"), regBinding.getCallId());
      CPPUNIT_ASSERT_EQUAL(string("<sip:alice@atlanta.com;x-sipX-nonat>"), regBinding.getContact());
      CPPUNIT_ASSERT_EQUAL(string("0"), regBinding.getQvalue());
      CPPUNIT_ASSERT_EQUAL(string(""), regBinding.getInstanceId());
      CPPUNIT_ASSERT_EQUAL(string(""), regBinding.getGruu());
      CPPUNIT_ASSERT_EQUAL(string("sip:proxy01.atlanta.com"), regBinding.getPath());
      CPPUNIT_ASSERT_EQUAL((unsigned int)1066, regBinding.getCseq());
      CPPUNIT_ASSERT_EQUAL((unsigned int)_timeNow + 3600, regBinding.getExpirationTime());
      CPPUNIT_ASSERT_EQUAL(string("instrument-test"), regBinding.getInstrument());
      CPPUNIT_ASSERT_EQUAL(string("172.16.10.50/R"), regBinding.getLocalAddress());
      CPPUNIT_ASSERT_EQUAL(_timeNow, regBinding.getTimestamp());
      CPPUNIT_ASSERT_EQUAL(false, regBinding.getExpired());
   }

   void testBindingWithIncorrectParameters(RegBinding& regBinding)
   {
      CPPUNIT_ASSERT(string("bob@atlanta.com") != regBinding.getIdentity());
      CPPUNIT_ASSERT(string("sip:bob@atalanta.com") != regBinding.getUri());
      CPPUNIT_ASSERT(string("hpwqlziwkduiqkg@bob.atlanta.com") != regBinding.getCallId());
      CPPUNIT_ASSERT(string("<sip:bob@atlanta.com;x-sipX-nonat>") != regBinding.getContact());
      CPPUNIT_ASSERT(string("1") != regBinding.getQvalue());
      CPPUNIT_ASSERT(string(" ") != regBinding.getInstanceId());
      CPPUNIT_ASSERT(string(" ") != regBinding.getGruu());
      CPPUNIT_ASSERT(string("sip:proxy02.atlanta.com") != regBinding.getPath());
      CPPUNIT_ASSERT((unsigned int)1065 != regBinding.getCseq());
      CPPUNIT_ASSERT((unsigned int)_timeNow + 2900 != regBinding.getExpirationTime());
      CPPUNIT_ASSERT(string("instrument-test-2") != regBinding.getInstrument());
      CPPUNIT_ASSERT(string("172.16.10.50/R-2") != regBinding.getLocalAddress());
      CPPUNIT_ASSERT(_timeNow + 1 != regBinding.getTimestamp());
      CPPUNIT_ASSERT(true != regBinding.getExpired());

   }

   void testBindingConstructor_WithRegBindingParameter()
   {
      RegBinding regBinding;

      // init Reg binding with default values
      setBinding(regBinding);

      // copy the values using Reg binding constructor
      RegBinding regBindingConstructor(regBinding);

      // check the values for the new Reg binding class
      testBindingWithCorrectParameters(regBindingConstructor);
      testBindingWithIncorrectParameters(regBindingConstructor);
   }

   void testBindingConstructor_WithSwapFunction()
   {
      RegBinding regBinding;

      // init Reg binding with default values
      setBinding(regBinding);

      RegBinding regBindingConstructor;

      // use the swap function in order to copy values
      regBindingConstructor.swap(regBinding);

      // check the values for the new Reg binding class
      testBindingWithCorrectParameters(regBindingConstructor);
      testBindingWithIncorrectParameters(regBindingConstructor);
   }

   void testBindingConstructor_OperatorEqual_WithRegBindingParameter()
   {
      RegBinding regBinding;

      // init Reg binding with default values
      setBinding(regBinding);

      // copy the values using Reg binding operator= function
      RegBinding regBindingConstructor = regBinding;

      // check the values for the new Reg binding class
      testBindingWithCorrectParameters(regBindingConstructor);
      testBindingWithIncorrectParameters(regBindingConstructor);
   }

   void testBindingConstructor_WithBSONObjParameter()
   {
      RegBinding regBinding;

      // init Reg binding with default values
      setBinding(regBinding);


      mongo::BSONObj bsonObj;

      // create BSONObj from Reg binding class
      createBSONObj(regBinding, bsonObj);

      // copy the values using Reg binding constructor
      RegBinding regBindingConstructor(bsonObj);

      // check the values for the new Reg binding class
      testBindingWithCorrectParameters(regBindingConstructor);
      testBindingWithIncorrectParameters(regBindingConstructor);
   }

   void testBindingConstructor_OperatorEqual_WithBSONObjParameter()
   {
      RegBinding regBinding;

      // init Reg binding with default values
      setBinding(regBinding);

      mongo::BSONObj bsonObj;

      // create BSONObj from Reg binding class
      createBSONObj(regBinding, bsonObj);

      // copy the values using Reg binding operator= function
      RegBinding regBindingConstructor = bsonObj;

      // check the values for the new Reg binding class
      testBindingWithCorrectParameters(regBindingConstructor);
      testBindingWithIncorrectParameters(regBindingConstructor);
   }

};

CPPUNIT_TEST_SUITE_REGISTRATION(RegBindingTest);

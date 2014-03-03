#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <sipxunit/TestUtilities.h>
#include <sipdb/RegBinding.h>
#include <os/OsDateTime.h>
#include <mongo/util/net/hostandport.h>
#include <mongo/client/connpool.h>


using namespace std;

typedef struct
{
  const char* pIdentity;
  const char* pUri;
  const char* pCallId;
  const char* pContact;
  const char* pQValue;
  const char* pInstanceId;
  const char* pGruu;
  const char* pPath;
  unsigned int cseq;
  unsigned int expirationTime;
  const char* pInstrument;
  const char* pLocalAddress;
  int timeStamp;
  bool expired;
} RegBindingTestData;

RegBindingTestData regBindingTestData[] =
{
  {
    "alice@atlanta.com",
    "sip:alice@atalanta.com",
    "hpwqlziwkduiqkg@alice.atlanta.com",
    "<sip:alice@atlanta.com;x-sipX-nonat>",
    "0",
    "",
    "",
    "sip:proxy01.atlanta.com",
    1066,
    3600,
    "instrument-test",
    "172.16.10.50/R",
    0,
    false
  }
};

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

   unsigned long _timeNow;
public:

   RegBindingTest()
   {
   }

   void setBinding(RegBinding& regBinding)
   {
      regBinding.setIdentity(regBindingTestData[0].pIdentity);
      regBinding.setUri(regBindingTestData[0].pUri);
      regBinding.setCallId(regBindingTestData[0].pCallId);
      regBinding.setContact(regBindingTestData[0].pContact);
      regBinding.setQvalue(regBindingTestData[0].pQValue);
      regBinding.setInstanceId(regBindingTestData[0].pInstanceId);
      regBinding.setGruu(regBindingTestData[0].pGruu);
      regBinding.setPath(regBindingTestData[0].pPath);
      regBinding.setCseq(regBindingTestData[0].cseq);
      regBinding.setExpirationTime(_timeNow + regBindingTestData[0].expirationTime);
      regBinding.setInstrument(regBindingTestData[0].pInstrument);
      regBinding.setLocalAddress(regBindingTestData[0].pLocalAddress);
      regBinding.setTimestamp(_timeNow + regBindingTestData[0].timeStamp);
      regBinding.setExpired(regBindingTestData[0].expired);
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
      _timeNow = OsDateTime::getSecsSinceEpoch();
   }

   void tearDown()
   {
   }

   void testBindingParameters(RegBinding& regBinding)
   {
      CPPUNIT_ASSERT(regBindingTestData[0].pIdentity == regBinding.getIdentity());
      CPPUNIT_ASSERT(regBindingTestData[0].pUri == regBinding.getUri());
      CPPUNIT_ASSERT(regBindingTestData[0].pCallId == regBinding.getCallId());
      CPPUNIT_ASSERT(regBindingTestData[0].pContact == regBinding.getContact());
      CPPUNIT_ASSERT(regBindingTestData[0].pQValue == regBinding.getQvalue());
      CPPUNIT_ASSERT(regBindingTestData[0].pInstanceId == regBinding.getInstanceId());
      CPPUNIT_ASSERT(regBindingTestData[0].pGruu == regBinding.getGruu());
      CPPUNIT_ASSERT(regBindingTestData[0].pPath == regBinding.getPath());
      CPPUNIT_ASSERT(regBindingTestData[0].cseq == regBinding.getCseq());
      CPPUNIT_ASSERT((unsigned int)_timeNow + regBindingTestData[0].expirationTime == regBinding.getExpirationTime());
      CPPUNIT_ASSERT(regBindingTestData[0].pInstrument == regBinding.getInstrument());
      CPPUNIT_ASSERT(regBindingTestData[0].pLocalAddress == regBinding.getLocalAddress());
      CPPUNIT_ASSERT(_timeNow + regBindingTestData[0].timeStamp == regBinding.getTimestamp());
      CPPUNIT_ASSERT(regBindingTestData[0].expired == regBinding.getExpired());
   }

   void testBindingConstructor_WithRegBindingParameter()
   {
      RegBinding regBinding;

      // init Reg binding with default values
      setBinding(regBinding);

      // copy the values using Reg binding constructor
      RegBinding regBindingConstructor(regBinding);

      // TEST: Check the values for the new Reg binding class
      testBindingParameters(regBindingConstructor);
   }

   void testBindingConstructor_WithSwapFunction()
   {
      RegBinding regBinding;

      // init Reg binding with default values
      setBinding(regBinding);

      RegBinding regBindingConstructor;

      // use the swap function in order to copy values
      regBindingConstructor.swap(regBinding);

      // TEST: Check the values for the new Reg binding class
      testBindingParameters(regBindingConstructor);
   }

   void testBindingConstructor_OperatorEqual_WithRegBindingParameter()
   {
      RegBinding regBinding;

      // init Reg binding with default values
      setBinding(regBinding);

      // copy the values using Reg binding operator= function
      RegBinding regBindingConstructor = regBinding;

      // TEST: Check the values for the new Reg binding class
      testBindingParameters(regBindingConstructor);
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

      // TEST: Check the values for the new Reg binding class
      testBindingParameters(regBindingConstructor);
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

      // TEST: Check the values for the new Reg binding class
      testBindingParameters(regBindingConstructor);
   }

};

CPPUNIT_TEST_SUITE_REGISTRATION(RegBindingTest);

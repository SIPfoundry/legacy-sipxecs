#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <sipxunit/TestUtilities.h>
#include <sipdb/RegDB.h>
#include <sipdb/MongoDB.h>
#include <os/OsDateTime.h>
#include <mongo/util/net/hostandport.h>
#include <mongo/client/connpool.h>


using namespace std;

const char* gLocalHostAddr = "localhost";
const char* gTestRegDbName = "test.RegDBTest";
const char* gAtlantaDotCom = "atlanta.com";

typedef struct
{
  const char* pContact;
  int expirationTimeDelta;
  const char* pQValue;
  const char* pInstanceId;
  const char* pGruu;
  const char* pPath;
  const char* pInstrument;
  const char* pCallId;
  unsigned int cseq;
  const char* pIdentity;
  const char* pUri;
} RegBindingTestData;

RegBindingTestData regBindingTestData[] =
{
  {
    "sip:alice@host1.atlanta.com;transport=tcp",
    3600,
    "0",
    "",
    "",
    "sip:proxy01.atlanta.com",
    "instrument-test",
    "call-id@12345",
    1,
    "alice@atlanta.com",
    "sip:alice@atalanta.com"
  },
  {
    "sip:alice@host2.atlanta.com;transport=tcp",
    3600,
    "0",
    "",
    "",
    "sip:proxy01.atlanta.com",
    "instrument-test",
    "call-id@45678",
    1,
    "alice@atlanta.com",
    "sip:alice@atalanta.com"
  },
  {
    "sip:alice@host1.atlanta.com;transport=tcp",
    3600,
    "0",
    "",
    "",
    "sip:proxy01.atlanta.com",
    "instrument-test",
    "call-id@34567--",
    1,
    "alice@atlanta.com",
    "sip:alice@atalanta.com"
  },
  {
    "sip:bob@host1.biloxy.com;transport=tcp",
    3600,
    "0",
    "",
    "",
    "sip:proxy01.biloxy.com",
    "instrument-test",
    "call-id@bob12345",
    1000,
    "bob@biloxy.com",
    "sip:bob@biloxy.com"
  },
  {
    "sip:bob@host2.biloxy.com;transport=tcp",
    -3600,
    "0",
    "",
    "",
    "sip:proxy01.biloxy.com",
    "instrument-test",
    "call-id@bob45678",
    1,
    "bob@biloxy.com",
    "sip:bob@biloxy.com"
  },
  {
    "-sip:generic@host1.sipfoundry.org",
    3600,
    "0",
    "",
    "",
    "sip:sipfoundry.org",
    "instrument-test",
    "-call-id-generic@host1.sipfoundry.org",
    1,
    "-generic@sipfoundry.org",
    "sip:generic@sipfoundry.org"
  }
};

class RegDBTest: public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE(RegDBTest);
  CPPUNIT_TEST(testUpdateBinding_AndAWholeBunchOfOtherStuffThatShouldBeInSeparateTests);
  CPPUNIT_TEST_SUITE_END();

  RegDB* _db;
  const MongoDB::ConnectionInfo _info;
  std::string _databaseName;
  unsigned long _timeNow;
public:
  RegDBTest() : _info(MongoDB::ConnectionInfo(mongo::ConnectionString(mongo::HostAndPort(gLocalHostAddr)))),
  _databaseName(gTestRegDbName)
{
}

  void setUp()
  {
    _timeNow = OsDateTime::getSecsSinceEpoch();
    _db = new RegDB(_info, NULL, _databaseName);

    MongoDB::ScopedDbConnectionPtr pConn(mongoMod::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));
    //mongo::ScopedDbConnection conn(_info.getConnectionString().toString());
    pConn->get()->remove(_databaseName, mongo::Query());
    pConn->done();
  }

  void tearDown()
  {
    delete _db;
    _db = 0;
  }

  void updateRegBindingTestData(RegBinding::Ptr& binding, int index)
  {
    binding->setContact(regBindingTestData[index].pContact);
    binding->setExpirationTime(_timeNow + regBindingTestData[index].expirationTimeDelta);
    binding->setQvalue(regBindingTestData[index].pQValue);
    binding->setInstanceId(regBindingTestData[index].pInstanceId);
    binding->setGruu(regBindingTestData[index].pGruu);
    binding->setPath(regBindingTestData[index].pPath);
    binding->setInstrument(regBindingTestData[index].pInstrument);
    binding->setCallId(regBindingTestData[index].pCallId);
    binding->setCseq(regBindingTestData[index].cseq);
    binding->setIdentity(regBindingTestData[index].pIdentity);
    binding->setUri(regBindingTestData[index].pUri);
    _db->updateBinding(binding);
  }

  void testUpdateBinding_AndAWholeBunchOfOtherStuffThatShouldBeInSeparateTests()
  {
    //
    // Create a binding that will expire in an hour
    //
    RegBinding::Ptr binding_0 = RegBinding::Ptr(new RegBinding());
    updateRegBindingTestData(binding_0, 0);

    //
    // TEST: There should be 1 unexpired contacts for alice
    //
    RegDB::Bindings bindings;
    CPPUNIT_ASSERT(_db->getUnexpiredContactsUser(regBindingTestData[0].pIdentity, _timeNow, bindings));
    CPPUNIT_ASSERT_EQUAL(1, (int) bindings.size());

    //
    // Insert a new contact for alice
    //
    RegBinding::Ptr binding_1 = RegBinding::Ptr(new RegBinding());
    updateRegBindingTestData(binding_1, 1);

    //
    // TEST: There should now be 2 unexpired contacts for alice
    //
    bindings.clear();
    CPPUNIT_ASSERT(_db->getUnexpiredContactsUser(regBindingTestData[0].pIdentity, _timeNow, bindings));
    CPPUNIT_ASSERT(bindings.size() == 2);

    //
    // reinsert bindings_0 but with different call-id
    //
    RegBinding::Ptr binding_2 = RegBinding::Ptr(new RegBinding());
    updateRegBindingTestData(binding_2, 2);

    //
    // TEST: There should still be 2 unexpired contacts for alice
    //
    bindings.clear();
    CPPUNIT_ASSERT(_db->getUnexpiredContactsUser(regBindingTestData[0].pIdentity, _timeNow, bindings));
    CPPUNIT_ASSERT_EQUAL(2, (int) bindings.size());

    //
    // Insert bob into the picture
    //
    RegBinding::Ptr binding_3 = RegBinding::Ptr(new RegBinding());
    updateRegBindingTestData(binding_3, 3);

    //
    // Make sure we are getting back exactly what we inserted
    //

    //
    // TEST: There should be 1 unexpired contacts for bob
    //
    bindings.clear();
    CPPUNIT_ASSERT(_db->getUnexpiredContactsUser(regBindingTestData[3].pIdentity, _timeNow, bindings));
    CPPUNIT_ASSERT_EQUAL(1, (int) bindings.size());
    CPPUNIT_ASSERT_EQUAL(string(regBindingTestData[3].pContact), bindings[0].getContact());
    CPPUNIT_ASSERT_EQUAL(string(regBindingTestData[3].pCallId), bindings[0].getCallId());

    //
    // TEST: There should still be 2 unexpired contacts for alice
    //
    bindings.clear();
    CPPUNIT_ASSERT(_db->getUnexpiredContactsUser(regBindingTestData[0].pIdentity, _timeNow, bindings));
    CPPUNIT_ASSERT_EQUAL(2, (int) bindings.size());

    //
    // TEST: There should be 2 unexpired contacts for atlanta.com
    //
    bindings.clear();
    CPPUNIT_ASSERT(_db->getUnexpiredContactsUserContaining(gAtlantaDotCom, _timeNow, bindings));
    CPPUNIT_ASSERT_EQUAL(2, (int) bindings.size());

    //
    // Insert and expire contact for bob
    //
    RegBinding::Ptr binding_4 = RegBinding::Ptr(new RegBinding());
    updateRegBindingTestData(binding_4, 4);

    //
    // TEST: There should be be only one record for bob because what we have inserted is expired
    //
    bindings.clear();
    CPPUNIT_ASSERT(_db->getUnexpiredContactsUser(regBindingTestData[4].pIdentity, _timeNow, bindings));
    CPPUNIT_ASSERT(bindings.size() == 1);

    //
    // TEST: Bob should be returned as part of the old bindings vector
    //
    bindings.clear();
    CPPUNIT_ASSERT(getAllOldBindings(_timeNow, bindings));
    CPPUNIT_ASSERT(bindings.size() == 1);

    //
    // TEST: Must return one contact for bob for instrument-test
    //
    bindings.clear();
    CPPUNIT_ASSERT(_db->getUnexpiredContactsUserInstrument(regBindingTestData[3].pIdentity, regBindingTestData[3].pInstrument, _timeNow, bindings));
    CPPUNIT_ASSERT(bindings.size() == 1);

    //
    // TEST: Alice has two unexpired bindins for instrument-test
    //
    bindings.clear();
    CPPUNIT_ASSERT(_db->getUnexpiredContactsUserInstrument(regBindingTestData[0].pIdentity, regBindingTestData[0].pInstrument, _timeNow, bindings));
    CPPUNIT_ASSERT(bindings.size() == 2);


    //
    // TEST: Alice has two unexpired bindins and bob have 1 unexpired binding for instrument-test
    //
    bindings.clear();
    CPPUNIT_ASSERT(_db->getUnexpiredContactsInstrument(regBindingTestData[0].pInstrument, _timeNow, bindings));
    CPPUNIT_ASSERT(bindings.size() == 3);

    //
    // removed entry with identity='bob@biloxy.com' callId='call-id@bob12345' and cseq<1001
    // TEST: Now bob has zero unexpired bindings
    //
    bindings.clear();
    _db->expireOldBindings(regBindingTestData[3].pIdentity, regBindingTestData[3].pCallId, regBindingTestData[3].cseq + 1, _timeNow);
    CPPUNIT_ASSERT(_db->getUnexpiredContactsUser(regBindingTestData[3].pIdentity, _timeNow, bindings) == false);
    CPPUNIT_ASSERT(bindings.size() == 0);

    //
    // removed all entries with identity='alice@atlanta.com'
    // TEST: Now alice has zero unexpired bindings
    //
    bindings.clear();
    _db->expireAllBindings(regBindingTestData[0].pIdentity, "", 0, _timeNow);
    CPPUNIT_ASSERT(_db->getUnexpiredContactsUserInstrument(regBindingTestData[0].pIdentity, regBindingTestData[0].pInstrument, _timeNow, bindings) == false);
    CPPUNIT_ASSERT(bindings.size() == 0);

    //
    //  TEST: We have one expired binding
    //
    bindings.clear();
    CPPUNIT_ASSERT(getAllOldBindings(_timeNow, bindings));
    CPPUNIT_ASSERT(bindings.size() == 1);


    //
    // TEST: We have no expired bindings, after removing all of them
    //
    bindings.clear();
    _db->removeAllExpired();
    CPPUNIT_ASSERT(getAllOldBindings(_timeNow, bindings) == false);
    CPPUNIT_ASSERT(bindings.size() == 0);

    //
    // TEST: Clean and persist contacts
    //
    _db->cleanAndPersist(_timeNow);
    bindings.clear();
    CPPUNIT_ASSERT(!getAllOldBindings(_timeNow, bindings));
    CPPUNIT_ASSERT(bindings.size() == 0);

    //
    // Insert 10000 users half of which are expired
    //
    std::cout << "Computing time elapsed inserting 1000 bindings.  This will take a while." << std::endl;
    std::cout.flush();

    unsigned long startTime = OsDateTime::getSecsSinceEpoch();
    for (int i = 1; i <= 1000; i++)
    {
      _timeNow = OsDateTime::getSecsSinceEpoch();
      RegBinding::Ptr binding = RegBinding::Ptr(new RegBinding());
      bool expired = i % 2;

      std::ostringstream contact;
      contact << i << regBindingTestData[5].pContact;
      binding->setContact(contact.str());

      if (!expired)
        binding->setExpirationTime(_timeNow + regBindingTestData[5].expirationTimeDelta);
      else
        binding->setExpirationTime(_timeNow - regBindingTestData[5].expirationTimeDelta);

      binding->setPath(regBindingTestData[5].pPath);

      binding->setInstrument(regBindingTestData[5].pInstrument);

      std::ostringstream callId;
      callId << i << regBindingTestData[5].pCallId;
      binding->setCallId(callId.str());

      binding->setCseq(i);

      std::ostringstream identity;
      identity << "i" << regBindingTestData[5].pIdentity;
      binding->setIdentity(identity.str());

      _db->updateBinding(binding);
    }

    std::cout << "Time elapsed inserting 1000 bindings: " << (int) OsDateTime::getSecsSinceEpoch()
    - startTime << " seconds" << std::endl;
    std::cout.flush();

    startTime = OsDateTime::getSecsSinceEpoch();
    bindings.clear();

    // TEST: Check that number of unexpired bindings is 500
    CPPUNIT_ASSERT(getAllOldBindings(_timeNow, bindings));
    CPPUNIT_ASSERT_EQUAL(500, (int) bindings.size());

    std::cout << "Time elapsed getting expired bindings: " << (int) OsDateTime::getSecsSinceEpoch()
    - startTime << " seconds" << std::endl;
    std::cout.flush();
  }

  bool getAllOldBindings(int timeNow, RegDB::Bindings& bindings)
  {
    mongo::BSONObj query = BSON( RegBinding::expirationTime_fld() << BSON_LESS_THAN((long long)timeNow));
    MongoDB::ScopedDbConnectionPtr pConn(mongoMod::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));
    auto_ptr<mongo::DBClientCursor> pCursor = pConn->get()->query(_databaseName, query);
    if (pCursor.get() && pCursor->more())
    {
      while (pCursor->more())
      {
        RegBinding binding(pCursor->next());
        bindings.push_back(binding);
      }
      pConn->done();
      return bindings.size() > 0;;
    }

    pConn->done();
    return false;
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(RegDBTest);


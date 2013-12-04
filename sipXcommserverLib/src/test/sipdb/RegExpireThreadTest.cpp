#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <sipxunit/TestUtilities.h>
#include <sipdb/RegDB.h>
#include <sipdb/RegExpireThread.h>
#include <sipdb/MongoDB.h>
#include <os/OsDateTime.h>
#include <mongo/util/net/hostandport.h>
#include <mongo/client/connpool.h>


using namespace std;

const char* gLocalHostAddr = "localhost";
const char* gDatabaseName = "test.RegExpireThreadTest";

typedef struct
{
  const char* pContact;
  unsigned int expirationTime;
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
    -3600,
    "0",
    "",
    "",
    "sip:proxy01.atlanta.com",
    "instrument-test",
    "call-id@12345",
    1,
    "alice@atlanta.com",
    "sip:alice@atalanta.com"
  }
};

class RegExpireThreadTest: public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE(RegExpireThreadTest);
  CPPUNIT_TEST(testRegExpireThreadTest_Run);
  CPPUNIT_TEST_SUITE_END();

  RegDB* _db;
  const MongoDB::ConnectionInfo _info;
  const std::string _databaseName;
  int _timeNow;
public:
  RegExpireThreadTest() : _info(MongoDB::ConnectionInfo(mongo::ConnectionString(mongo::HostAndPort(gLocalHostAddr)))),
                          _databaseName(gDatabaseName)
  {
  }

  void setUp()
  {
    _timeNow = (int) OsDateTime::getSecsSinceEpoch();
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
    binding->setExpirationTime(_timeNow + regBindingTestData[index].expirationTime);
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

  void testRegExpireThreadTest_Run()
  {
    //
    // Create a binding that expired an hour ago
    //
    RegBinding::Ptr binding_0 = RegBinding::Ptr(new RegBinding());
    updateRegBindingTestData(binding_0, 0);

    RegDB::Bindings bindings;

    // TEST: Check that we successfully inserted one entry
    CPPUNIT_ASSERT(getAllOldBindings(_timeNow, bindings) == true);
    CPPUNIT_ASSERT(bindings.size() == 1);

    bindings.clear();

    RegExpireThread regExpireThread;

    // start reg Expire thread that will run every two second and will remove all expired records
    regExpireThread.run(_db, 2);

    // wait 3 seconds to be sure that the thread removed all expired entries
    sleep(3);

    // TEST: Check that there are no entries in test.RegExpireThreadTest database
    CPPUNIT_ASSERT(getAllOldBindings(_timeNow, bindings) == false);
    CPPUNIT_ASSERT(bindings.size() == 0);
  }

  bool getAllOldBindings(int timeNow, RegDB::Bindings& bindings)
  {
    mongo::BSONObj query = BSON(RegBinding::expirationTime_fld() << BSON_LESS_THAN(timeNow));
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
      return true;
    }

    pConn->done();
    return false;
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(RegExpireThreadTest);


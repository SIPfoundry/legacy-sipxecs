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

extern mongo::DBConnectionPool pool;

class RegExpireThreadTest: public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE(RegExpireThreadTest);
  CPPUNIT_TEST(testRegExpireThreadTest_Run);
  CPPUNIT_TEST_SUITE_END();

  RegDB* _db;
  const MongoDB::ConnectionInfo _info;
  const std::string _databaseName;
public:
  RegExpireThreadTest() : _info(MongoDB::ConnectionInfo(mongo::ConnectionString(mongo::HostAndPort("localhost")))),
                          _databaseName("test.RegExpireThreadTest")
  {
  }

  void setUp()
  {
    _db = new RegDB(_info, NULL, _databaseName);
    MongoDB::ScopedDbConnectionPtr pConn(mongo::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));
    //mongo::ScopedDbConnection conn(_info.getConnectionString().toString());
    pConn->get()->remove(_databaseName, mongo::Query());
    pConn->done();
  }

  void tearDown()
  {
    delete _db;
    _db = 0;
  }

  void testRegExpireThreadTest_Run()
  {
    int timeNow = (int) OsDateTime::getSecsSinceEpoch();
    //
    // Create a binding that expired an hour ago
    //
    RegBinding::Ptr binding_0 = RegBinding::Ptr(new RegBinding());
    binding_0->setContact("sip:alice@host1.atlanta.com;transport=tcp");
    binding_0->setExpirationTime(timeNow - 3600);
    binding_0->setQvalue("0");
    binding_0->setInstanceId("");
    binding_0->setGruu("");
    binding_0->setPath("sip:proxy01.atlanta.com");
    binding_0->setInstrument("instrument-test");
    binding_0->setCallId("call-id@12345");
    binding_0->setCseq(1);
    binding_0->setIdentity("alice@atlanta.com");
    binding_0->setUri("sip:alice@atalanta.com");
    _db->updateBinding(binding_0);

    RegDB::Bindings bindings;

    // check that we successfully inserted one entry
    CPPUNIT_ASSERT(getAllOldBindings(timeNow, bindings) == true);
    CPPUNIT_ASSERT(bindings.size() == 1);

    bindings.clear();

    RegExpireThread regExpireThread;

    // start reg Expire thread that will run every two second and will remove all expired records
    regExpireThread.run(_db, 2);

    // wait 3 seconds to be sure that the thread removed all expired entries
    sleep(3);

    // check that there are no entries in test.RegExpireThreadTest database
    CPPUNIT_ASSERT(getAllOldBindings(timeNow, bindings) == false);
    CPPUNIT_ASSERT(bindings.size() == 0);
  }

  bool getAllOldBindings(int timeNow, RegDB::Bindings& bindings)
  {
    mongo::BSONObj query = BSON( "expirationTime" << BSON_LESS_THAN(timeNow));
    MongoDB::ScopedDbConnectionPtr pConn(mongo::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));
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


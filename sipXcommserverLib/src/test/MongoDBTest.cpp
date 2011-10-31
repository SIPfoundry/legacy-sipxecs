#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipdb/MongoDB.h>
#include <mongo/client/dbclient.h>
#include <mongo/client/connpool.h>

using namespace std;

class MongoDBTest: public CppUnit::TestCase
{
	CPPUNIT_TEST_SUITE(MongoDBTest);
	CPPUNIT_TEST(testReadHAConfig);
    CPPUNIT_TEST(testReadSingleConfig);
	CPPUNIT_TEST_SUITE_END();

public:

    void testReadSingleConfig()
    {
        mongo::ConnectionString s = MongoDB::ConnectionInfo::connectionStringFromFile(TEST_DATA_DIR "/sipxmongo-single-config");
        CPPUNIT_ASSERT_EQUAL(string("sipxecs"), s.getSetName());
        std::vector<mongo::HostAndPort> servers = s.getServers();
        CPPUNIT_ASSERT_EQUAL(1, (int) servers.size());
        mongo::HostAndPort first = servers.front();
        CPPUNIT_ASSERT_EQUAL(string("localhost"), first.host());
        CPPUNIT_ASSERT_EQUAL(27017, first.port());
    }

	void testReadHAConfig()
	{
		mongo::ConnectionString s = MongoDB::ConnectionInfo::connectionStringFromFile(TEST_DATA_DIR "/sipxmongo-ha-config");
		CPPUNIT_ASSERT_EQUAL(string("sipxecs"), s.getSetName());
		std::vector<mongo::HostAndPort> servers = s.getServers();
		CPPUNIT_ASSERT_EQUAL(2, (int) servers.size());
		mongo::HostAndPort first = servers.front();
		CPPUNIT_ASSERT_EQUAL(string("localhost"), first.host());
		CPPUNIT_ASSERT_EQUAL(27017, first.port());
		mongo::HostAndPort second = servers.back();
		CPPUNIT_ASSERT_EQUAL(string("localhost"), second.host());
		CPPUNIT_ASSERT_EQUAL(27018, second.port());
	}
};
CPPUNIT_TEST_SUITE_REGISTRATION(MongoDBTest);


class BaseDBTest: public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(BaseDBTest);
    CPPUNIT_TEST(testForEach);
    CPPUNIT_TEST_SUITE_END();

    const MongoDB::ConnectionInfo _info;
    int _row;

public:

    BaseDBTest() :
        _info(MongoDB::ConnectionInfo(mongo::ConnectionString(mongo::HostAndPort("localhost")), string("test.BaseDBTest")))
    {
    }

    void forEachFunction(mongo::BSONObj& record)
    {
        CPPUNIT_ASSERT_EQUAL(_row * 10, record.getIntField("a"));
        _row++;
    }

    void testForEach()
    {
        _row = 0;
        mongo::ScopedDbConnection conn(_info.getConnectionString());
        mongo::BSONObj query;
        conn->remove(_info.getNS(), query);
        conn->insert(_info.getNS(), BSON("a" << 0));
        conn->insert(_info.getNS(), BSON("a" << 10));
        conn.done();

        MongoDB::BaseDB db(_info);
        db.forEach(query, bind(&BaseDBTest::forEachFunction, this, _1));
        CPPUNIT_ASSERT_EQUAL(2, _row);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(BaseDBTest);

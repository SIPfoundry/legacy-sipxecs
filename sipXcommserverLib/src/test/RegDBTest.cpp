#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <sipxunit/TestUtilities.h>
#include <sipdb/RegDB.h>
#include <os/OsDateTime.h>
//#include <json/json_spirit.h>
#include <mongo/util/net/hostandport.h>
#include <mongo/client/connpool.h>


using namespace std;

extern mongo::DBConnectionPool pool;

class RegDBTest: public CppUnit::TestCase
{
	CPPUNIT_TEST_SUITE(RegDBTest);
	CPPUNIT_TEST(testUpdateBinding_AndAWholeBunchOfOtherStuffThatShouldBeInSeparateTests);
	CPPUNIT_TEST_SUITE_END();

	RegDB* _db;
	const MongoDB::ConnectionInfo _info;
public:
	RegDBTest() : _info(MongoDB::ConnectionInfo(mongo::ConnectionString(mongo::HostAndPort("localhost")), string("test.RegDBTest")))
	{
	}

	void setUp()
	{
		_db = new RegDB(_info);
		mongo::ScopedDbConnection* conn = mongo::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString());
		(*conn)->remove(_info.getNS(), mongo::Query());
		(*conn).done();
		delete conn;
	}

	void tearDown()
	{
		delete _db;
		_db = 0;
	}

	void testUpdateBinding_AndAWholeBunchOfOtherStuffThatShouldBeInSeparateTests()
	{
		int timeNow = (int) OsDateTime::getSecsSinceEpoch();
		//
		// Create a binding that will expire in an hour
		//
		RegBinding::Ptr binding_0 = RegBinding::Ptr(new RegBinding());
		binding_0->setContact("sip:alice@host1.atlanta.com;transport=tcp");
		binding_0->setExpirationTime(timeNow + 3600);
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
		CPPUNIT_ASSERT(_db->getUnexpiredContactsUser("alice@atlanta.com", timeNow, bindings));
		CPPUNIT_ASSERT_EQUAL(1, (int) bindings.size());

		//
		// Insert a new contact for alice
		//
		RegBinding::Ptr binding_1 = RegBinding::Ptr(new RegBinding());
		binding_1->setContact("sip:alice@host2.atlanta.com;transport=tcp");
		binding_1->setExpirationTime(timeNow + 3600);
		binding_1->setQvalue("0");
		binding_1->setInstanceId("");
		binding_1->setGruu("");
		binding_1->setPath("sip:proxy01.atlanta.com");
		binding_1->setInstrument("instrument-test");
		binding_1->setCallId("call-id@45678");
		binding_1->setCseq(1);
		binding_1->setIdentity("alice@atlanta.com");
		binding_1->setUri("sip:alice@atalanta.com");
		_db->updateBinding(binding_1);

		//
		// Thre should now be 2 unexpired contacts for alice
		//
		bindings.clear();
		CPPUNIT_ASSERT(_db->getUnexpiredContactsUser("alice@atlanta.com", timeNow, bindings));
		CPPUNIT_ASSERT(bindings.size() == 2);

		//
		// reinsert bindings_0 but with different call-id
		//
		RegBinding::Ptr binding_2 = RegBinding::Ptr(new RegBinding());
		binding_2->setContact("sip:alice@host1.atlanta.com;transport=tcp");
		binding_2->setExpirationTime(timeNow + 3600);
		binding_2->setQvalue("0");
		binding_2->setInstanceId("");
		binding_2->setGruu("");
		binding_2->setPath("sip:proxy01.atlanta.com");
		binding_2->setInstrument("instrument-test");
		binding_2->setCallId("call-id@34567--");
		binding_2->setCseq(1);
		binding_2->setIdentity("alice@atlanta.com");
		binding_2->setUri("sip:alice@atalanta.com");
		_db->updateBinding(binding_2);

		//
		// Thre should still be 2 unexpired contacts for alice
		//
		bindings.clear();
		CPPUNIT_ASSERT(_db->getUnexpiredContactsUser("alice@atlanta.com", timeNow, bindings));
		CPPUNIT_ASSERT_EQUAL(2, (int) bindings.size());

		//
		// Insert bob into the picture
		//
		RegBinding::Ptr binding_3 = RegBinding::Ptr(new RegBinding());
		binding_3->setContact("sip:bob@host1.biloxy.com;transport=tcp");
		binding_3->setExpirationTime(timeNow + 3600);
		binding_3->setQvalue("0");
		binding_3->setInstanceId("");
		binding_3->setGruu("");
		binding_3->setPath("sip:proxy01.biloxy.com");
		binding_3->setInstrument("instrument-test");
		binding_3->setCallId("call-id@bob12345");
		binding_3->setCseq(1000);
		binding_3->setIdentity("bob@biloxy.com");
		binding_3->setUri("sip:bob@biloxy.com");
		_db->updateBinding(binding_3);

		//
		// Make sure we are getting back exactly what we inserted
		//

		//
		// There should be 1 unexpired contacts for bob
		//
		bindings.clear();
		CPPUNIT_ASSERT(_db->getUnexpiredContactsUser("bob@biloxy.com", timeNow, bindings));
		CPPUNIT_ASSERT_EQUAL(1, (int) bindings.size());
		CPPUNIT_ASSERT_EQUAL(string("sip:bob@host1.biloxy.com;transport=tcp"), bindings[0].getContact());
		CPPUNIT_ASSERT_EQUAL(string("call-id@bob12345"), bindings[0].getCallId());

		//
		// Thre should still be 2 unexpired contacts for alice
		//
		bindings.clear();
		CPPUNIT_ASSERT(_db->getUnexpiredContactsUser("alice@atlanta.com", timeNow, bindings));
		CPPUNIT_ASSERT_EQUAL(2, (int) bindings.size());

		//
		// there should be 2 unexpired contacts for atlanta.com
		//
		bindings.clear();
		CPPUNIT_ASSERT(_db->getUnexpiredContactsUserContaining("atlanta.com", timeNow, bindings));
		CPPUNIT_ASSERT_EQUAL(2, (int) bindings.size());

		//
		// Insert and expire contact for bob
		//
		RegBinding::Ptr binding_4 = RegBinding::Ptr(new RegBinding());
		binding_4->setContact("sip:bob@host2.biloxy.com;transport=tcp");
		binding_4->setExpirationTime(timeNow - 3600);
		binding_4->setQvalue("0");
		binding_4->setInstanceId("");
		binding_4->setGruu("");
		binding_4->setPath("sip:proxy01.biloxy.com");
		binding_4->setInstrument("instrument-test");
		binding_4->setCallId("call-id@bob45678");
		binding_4->setCseq(1);
		binding_4->setIdentity("bob@biloxy.com");
		binding_4->setUri("sip:bob@biloxy.com");
		_db->updateBinding(binding_4);

		//
		// The should be be only one record for bob because what we have inserted is expired
		//
		bindings.clear();
		CPPUNIT_ASSERT(_db->getUnexpiredContactsUser("bob@biloxy.com", timeNow, bindings));
		CPPUNIT_ASSERT(bindings.size() == 1);

		//
		// bob should be returned as part of the old bindings vector
		//
		bindings.clear();
		CPPUNIT_ASSERT(getAllOldBindings(timeNow, bindings));
		CPPUNIT_ASSERT(bindings.size() == 1);

		//
		// Must return one contact for bob for instrument-test
		//
		bindings.clear();
		CPPUNIT_ASSERT(_db->getUnexpiredContactsUserInstrument("bob@biloxy.com", "instrument-test", timeNow, bindings));
		CPPUNIT_ASSERT(bindings.size() == 1);

		//
		// alice has two unexpired bindins for instrument-test
		//
		bindings.clear();
		CPPUNIT_ASSERT(_db->getUnexpiredContactsUserInstrument("alice@atlanta.com", "instrument-test", timeNow, bindings));
		CPPUNIT_ASSERT(bindings.size() == 2);

		//
		// Clean and persist contacts
		//
		_db->cleanAndPersist(timeNow);
		bindings.clear();
		CPPUNIT_ASSERT(!getAllOldBindings(timeNow, bindings));
		CPPUNIT_ASSERT(bindings.size() == 0);

		//
		// Insert 10000 users half of which are expired
		//
		std::cout << "Computing time elapsed inserting 1000 bindings.  This will take a while." << std::endl;
		std::cout.flush();

		int startTime = (int) OsDateTime::getSecsSinceEpoch();
		for (int i = 1; i <= 1000; i++)
		{
			timeNow = (int) OsDateTime::getSecsSinceEpoch();
			RegBinding::Ptr binding = RegBinding::Ptr(new RegBinding());
			bool expired = i % 2;

			std::ostringstream contact;
			contact << "sip:" << i << "@host1.sipfoundry.org";
			binding->setContact(contact.str());

			if (!expired)
				binding->setExpirationTime(timeNow + 3600);
			else
				binding->setExpirationTime(timeNow - 3600);

			binding->setPath("sip:sipfoundry.org");

			binding->setInstrument("instrument-test");

			std::ostringstream callId;
			callId << "call-id-" << i << "@host1.sipfoundry.org";
			binding->setCallId(callId.str());

			binding->setCseq(i);

			std::ostringstream identity;
			identity << "i" << "@sipfoundry.org";
			binding->setIdentity(identity.str());

			_db->updateBinding(binding);
		}

		std::cout << "Time elapsed inserting 1000 bindings: " << (int) OsDateTime::getSecsSinceEpoch()
				- startTime << " seconds" << std::endl;
		std::cout.flush();

		startTime = (int) OsDateTime::getSecsSinceEpoch();
		bindings.clear();
		CPPUNIT_ASSERT(getAllOldBindings(timeNow, bindings));
		CPPUNIT_ASSERT_EQUAL(500, (int) bindings.size());

		std::cout << "Time elapsed getting expired bindings: " << (int) OsDateTime::getSecsSinceEpoch()
				- startTime << " seconds" << std::endl;
		std::cout.flush();
	}

	bool getAllOldBindings(int timeNow, RegDB::Bindings& bindings)
	{
		mongo::BSONObj query = BSON( "expirationTime" << BSON_LESS_THAN(timeNow));
		mongo::ScopedDbConnection* conn = mongo::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString());
		auto_ptr<mongo::DBClientCursor> pCursor = (*conn)->query(_info.getNS(), query);
		if (pCursor.get() && pCursor->more())
		{
			while (pCursor->more())
			{
				RegBinding binding(pCursor->next());
				bindings.push_back(binding);
			}
		}
		delete conn;
		return bindings.size() > 0;
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(RegDBTest);


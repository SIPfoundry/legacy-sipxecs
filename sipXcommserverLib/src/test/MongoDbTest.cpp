#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <sipdb/MongoDB.h>
#include <sipdb/EntityDB.h>
#include <sipdb/RegDB.h>
#include <sipdb/SubscribeDB.h>

#include <os/OsDateTime.h>
#include <json/json_spirit.h>

class MongoDbTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(MongoDbTest);

   CPPUNIT_TEST(testRegistarOperations);
   CPPUNIT_TEST(testRegistrarNodesConfig);
   CPPUNIT_TEST(testRegistrarSweepReplication);
   CPPUNIT_TEST_SUITE_END();

   MongoDB::Collection<RegDB>* _pRegDb;
   MongoDB::Collection<RegDB>* _pRegDbHA1;
   MongoDB::Collection<RegDB>* _pRegDbHA2;
   MongoDB::Collection<RegDB>* _pRegDbHA3;
public:

  void setUp()
  {
    _pRegDb = new MongoDB::Collection<RegDB>("sipXcommserverLib.registrar", "localhost:27017");
    _pRegDbHA1 = new MongoDB::Collection<RegDB>("sipXcommserverLib.registrar1", "localhost:27017");
    _pRegDbHA2 = new MongoDB::Collection<RegDB>("sipXcommserverLib.registrar2", "localhost:27017");
    _pRegDbHA3 = new MongoDB::Collection<RegDB>("sipXcommserverLib.registrar3", "localhost:27017");

    
  }

  void tearDown()
  {
    delete _pRegDb;
    _pRegDb = 0;
    delete _pRegDbHA1;
    _pRegDbHA1 = 0;
    delete _pRegDbHA2;
    _pRegDbHA2 = 0;
    delete _pRegDbHA3;
    _pRegDbHA3 = 0;
  }

  void testRegistrarNodesConfig()
  {
    namespace json = json_spirit;

    json::Array a;

    json::Object node1;
    node1.push_back( json::Pair( "node.server", "localhost:27017" ) );
    node1.push_back( json::Pair( "node.internalAddress", "host1" ) );
    node1.push_back( json::Pair( "node.collection",  "sipXcommserverLib.registrar1" ) );
    a.push_back(node1);

    json::Object node2;
    node2.push_back( json::Pair( "node.server", "localhost:27017" ) );
    node2.push_back( json::Pair( "node.internalAddress", "host2" ) );
    node2.push_back( json::Pair( "node.collection",  "sipXcommserverLib.registrar2" ) );
    a.push_back(node2);

    json::Object node3;
    node3.push_back( json::Pair( "node.server", "localhost:27017" ) );
    node3.push_back( json::Pair( "node.internalAddress", "host3" ) );
    node3.push_back( json::Pair( "node.collection",  "sipXcommserverLib.registrar3" ) );
    a.push_back(node3);

    std::ofstream os( "node.json" );
    json::write_formatted( a, os );
    os.close();

    std::ifstream is("node.json");

    json::Value value;
    json::read( is, value );
    const json::Array& nodesArray = value.get_array();
    
    CPPUNIT_ASSERT(nodesArray.size() == 3);

    json::Object nodeObj1 = nodesArray[0].get_obj();
    json::Object nodeObj2 = nodesArray[1].get_obj();
    json::Object nodeObj3 = nodesArray[2].get_obj();

    CPPUNIT_ASSERT(nodeObj1.size() == 3);
    const json::Pair& serverPair1 = nodeObj1[0];
    CPPUNIT_ASSERT(serverPair1.name_ == "node.server");
    CPPUNIT_ASSERT( serverPair1.value_ == "localhost:27017");
    const json::Pair& internalAddressPair1 = nodeObj1[1];
    CPPUNIT_ASSERT(internalAddressPair1.name_ == "node.internalAddress");
    CPPUNIT_ASSERT( internalAddressPair1.value_ == "host1");
    const json::Pair& collectionPair1 = nodeObj1[2];
    CPPUNIT_ASSERT(collectionPair1.name_ == "node.collection");
    CPPUNIT_ASSERT( collectionPair1.value_ == "sipXcommserverLib.registrar1");


    CPPUNIT_ASSERT(nodeObj2.size() == 3);
    const json::Pair& serverPair2 = nodeObj2[0];
    CPPUNIT_ASSERT(serverPair2.name_ == "node.server");
    CPPUNIT_ASSERT( serverPair2.value_ == "localhost:27017");
    const json::Pair& internalAddressPair2 = nodeObj2[1];
    CPPUNIT_ASSERT(internalAddressPair2.name_ == "node.internalAddress");
    CPPUNIT_ASSERT( internalAddressPair2.value_ == "host2");
    const json::Pair& collectionPair2 = nodeObj2[2];
    CPPUNIT_ASSERT(collectionPair2.name_ == "node.collection");
    CPPUNIT_ASSERT( collectionPair2.value_ == "sipXcommserverLib.registrar2");

    CPPUNIT_ASSERT(nodeObj3.size() == 3);
    const json::Pair& serverPair3 = nodeObj3[0];
    CPPUNIT_ASSERT(serverPair3.name_ == "node.server");
    CPPUNIT_ASSERT( serverPair3.value_ == "localhost:27017");
    const json::Pair& internalAddressPair3 = nodeObj3[1];
    CPPUNIT_ASSERT(internalAddressPair3.name_ == "node.internalAddress");
    CPPUNIT_ASSERT( internalAddressPair3.value_ == "host3");
    const json::Pair& collectionPair3 = nodeObj3[2];
    CPPUNIT_ASSERT(collectionPair3.name_ == "node.collection");
    CPPUNIT_ASSERT( collectionPair3.value_ == "sipXcommserverLib.registrar3");

  }

  void testRegistrarSweepReplication()
  {
    _pRegDbHA1->collection().setLocalAddress("host1");
    _pRegDbHA2->collection().setLocalAddress("host2");
    _pRegDbHA3->collection().setLocalAddress("host3");


    _pRegDbHA1->collection().clearAllBindings();
    _pRegDbHA2->collection().clearAllBindings();
    _pRegDbHA3->collection().clearAllBindings();

    RegDB::Bindings bindings;
    int timeNow = (int) OsDateTime::getSecsSinceEpoch();

    _pRegDbHA1->collection().cleanAndPersist(timeNow, "node.json");
    _pRegDbHA2->collection().cleanAndPersist(timeNow, "node.json");
    _pRegDbHA3->collection().cleanAndPersist(timeNow, "node.json");


    for (int i = 1; i <= 1000; i++)
    {
      timeNow = (int) OsDateTime::getSecsSinceEpoch();
      RegBinding::Ptr binding = RegBinding::Ptr(new RegBinding());
      bool expired = i % 2;

      int user = i + 1000;
      std::ostringstream contact;
      contact << "sip:" << user << "@host.sipfoundry.org";
      binding->setContact(contact.str());

      if (!expired)
        binding->setExpirationTime(timeNow + 3600);
      else
        binding->setExpirationTime(timeNow - 3600);

      binding->setLocalAddress("host1/sipXcommserverLib.registrar1");

      binding->setPath("sip:sipfoundry.org");

      binding->setInstrument("instrument-test");

      std::ostringstream callId;
      callId << "call-id-reg1" << i << "@sipfoundry.org";
      binding->setCallId(callId.str());

      binding->setCseq(i);

      std::ostringstream identity;
      identity << user << "@sipfoundry.org";
      binding->setIdentity(identity.str());

      _pRegDbHA1->collection().updateBinding(binding);
    }

    bindings.clear();
    CPPUNIT_ASSERT(_pRegDbHA1->collection().getAllOldBindings(timeNow, bindings));
    CPPUNIT_ASSERT(bindings.size() == 500);

    for (int i = 1; i <= 1000; i++)
    {
      timeNow = (int) OsDateTime::getSecsSinceEpoch();
      RegBinding::Ptr binding = RegBinding::Ptr(new RegBinding());
      bool expired = i % 2;

      int user = i + 2000;
      std::ostringstream contact;
      contact << "sip:" << user << "@host.sipfoundry.org";
      binding->setContact(contact.str());

      binding->setLocalAddress("host2/sipXcommserverLib.registrar2");

      if (!expired)
        binding->setExpirationTime(timeNow + 3600);
      else
        binding->setExpirationTime(timeNow - 3600);


      binding->setPath("sip:sipfoundry.org");

      binding->setInstrument("instrument-test");

      std::ostringstream callId;
      callId << "call-id-reg2" << i << "@sipfoundry.org";
      binding->setCallId(callId.str());

      binding->setCseq(i);

      std::ostringstream identity;
      identity << user << "@sipfoundry.org";
      binding->setIdentity(identity.str());

      _pRegDbHA2->collection().updateBinding(binding);
    }

    bindings.clear();
    CPPUNIT_ASSERT(_pRegDbHA2->collection().getAllOldBindings(timeNow, bindings));
    CPPUNIT_ASSERT(bindings.size() == 500);

    for (int i = 1; i <= 1000; i++)
    {
      timeNow = (int) OsDateTime::getSecsSinceEpoch();
      RegBinding::Ptr binding = RegBinding::Ptr(new RegBinding());
      bool expired = i % 2;

      int user = i + 3000;
      std::ostringstream contact;
      contact << "sip:" << user << "@host.sipfoundry.org";
      binding->setContact(contact.str());

      binding->setLocalAddress("host3/sipXcommserverLib.registrar3");

      if (!expired)
        binding->setExpirationTime(timeNow + 3600);
      else
        binding->setExpirationTime(timeNow - 3600);


      binding->setPath("sip:sipfoundry.org");

      binding->setInstrument("instrument-test");

      std::ostringstream callId;
      callId << "call-id-reg3" << i << "@sipfoundry.org";
      binding->setCallId(callId.str());

      binding->setCseq(i);

      std::ostringstream identity;
      identity << user << "@sipfoundry.org";
      binding->setIdentity(identity.str());

      _pRegDbHA3->collection().updateBinding(binding);
    }

    bindings.clear();
    CPPUNIT_ASSERT(_pRegDbHA3->collection().getAllOldBindings(timeNow, bindings));
    CPPUNIT_ASSERT(bindings.size() == 500);

    bindings.clear();
    CPPUNIT_ASSERT(_pRegDbHA1->collection().getAllBindings(bindings));
    std::cout << "HA1 records before replication " << bindings.size() << std::endl;
    CPPUNIT_ASSERT(bindings.size() == 1000);

    bindings.clear();
    CPPUNIT_ASSERT(_pRegDbHA2->collection().getAllBindings(bindings));
    std::cout << "HA2 records before replication " << bindings.size() << std::endl;
    CPPUNIT_ASSERT(bindings.size() == 1000);

    bindings.clear();
    CPPUNIT_ASSERT(_pRegDbHA3->collection().getAllBindings(bindings));
    std::cout << "HA3 records before replication " << bindings.size() << std::endl;
    CPPUNIT_ASSERT(bindings.size() == 1000);

   
    timeNow = (int) OsDateTime::getSecsSinceEpoch();
    _pRegDbHA1->collection().cleanAndPersist(timeNow, "node.json");
    _pRegDbHA2->collection().cleanAndPersist(timeNow, "node.json");
    _pRegDbHA3->collection().cleanAndPersist(timeNow, "node.json");
    _pRegDbHA1->collection().cleanAndPersist(timeNow, "node.json");
    _pRegDbHA2->collection().cleanAndPersist(timeNow, "node.json");
    _pRegDbHA3->collection().cleanAndPersist(timeNow, "node.json");

    bindings.clear();
    CPPUNIT_ASSERT(_pRegDbHA1->collection().getAllBindings(bindings));
    std::cout << "HA1 records after replication " << bindings.size() << std::endl;
    CPPUNIT_ASSERT(bindings.size() == 1500);
    
    bindings.clear();
    CPPUNIT_ASSERT(_pRegDbHA2->collection().getAllBindings(bindings));
    std::cout << "HA2 records after replication " << bindings.size() << std::endl;
    CPPUNIT_ASSERT(bindings.size() == 1500);

    bindings.clear();
    CPPUNIT_ASSERT(_pRegDbHA3->collection().getAllBindings(bindings));
    std::cout << "HA3 records after replication " << bindings.size() << std::endl;
    CPPUNIT_ASSERT(bindings.size() == 1500);
  }

  void testRegistarOperations()
  {
    CPPUNIT_ASSERT(_pRegDb);

    _pRegDb->collection().clearAllBindings();

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
    _pRegDb->collection().updateBinding(binding_0);


    
    RegDB::Bindings bindings;
    CPPUNIT_ASSERT(_pRegDb->collection().getUnexpiredContactsUser("alice@atlanta.com", timeNow, bindings));
    CPPUNIT_ASSERT(bindings.size() == 1);

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
    _pRegDb->collection().updateBinding(binding_1);

    //
    // Thre should now be 2 unexpired contacts for alice
    //
    bindings.clear();
    CPPUNIT_ASSERT(_pRegDb->collection().getUnexpiredContactsUser("alice@atlanta.com", timeNow, bindings));
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
    _pRegDb->collection().updateBinding(binding_2);

    //
    // Thre should still be 2 unexpired contacts for alice
    //
    bindings.clear();
    CPPUNIT_ASSERT(_pRegDb->collection().getUnexpiredContactsUser("alice@atlanta.com", timeNow, bindings));
    CPPUNIT_ASSERT(bindings.size() == 2);

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
    _pRegDb->collection().updateBinding(binding_3);

    //
    // Make sure we are getting back exactly what we inserted
    //

    //
    // There should be 1 unexpired contacts for bob
    //
    bindings.clear();
    CPPUNIT_ASSERT(_pRegDb->collection().getUnexpiredContactsUser("bob@biloxy.com", timeNow, bindings));
    CPPUNIT_ASSERT(bindings.size() == 1);
    CPPUNIT_ASSERT(bindings[0].getContact() == "sip:bob@host1.biloxy.com;transport=tcp");
    CPPUNIT_ASSERT(bindings[0].getCallId() == "call-id@bob12345");
    //
    // Check out of sequence
    //
    CPPUNIT_ASSERT(_pRegDb->collection().isOutOfSequence("bob@biloxy.com", "call-id@bob12345", 999));
    CPPUNIT_ASSERT(_pRegDb->collection().isOutOfSequence("bob@biloxy.com", "call-id@bob12345", 1000));
    CPPUNIT_ASSERT(!_pRegDb->collection().isOutOfSequence("bob@biloxy.com", "call-id@bob12345", 1001));


    //
    // Thre should still be 2 unexpired contacts for alice
    //
    bindings.clear();
    CPPUNIT_ASSERT(_pRegDb->collection().getUnexpiredContactsUser("alice@atlanta.com", timeNow, bindings));
    CPPUNIT_ASSERT(bindings.size() == 2);


    //
    // there should be 2 unexpired contacts for atlanta.com
    //
    bindings.clear();
    CPPUNIT_ASSERT(_pRegDb->collection().getUnexpiredContactsUserContaining("atlanta.com", timeNow, bindings));
    CPPUNIT_ASSERT(bindings.size() == 2);


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
    _pRegDb->collection().updateBinding(binding_4);

    //
    // The should be be only one record for bob because what we have inserted is expired
    //
    bindings.clear();
    CPPUNIT_ASSERT(_pRegDb->collection().getUnexpiredContactsUser("bob@biloxy.com", timeNow, bindings));
    CPPUNIT_ASSERT(bindings.size() == 1);

    //
    // bob should be returned as part of the old bindings vector
    //
    bindings.clear();
    CPPUNIT_ASSERT(_pRegDb->collection().getAllOldBindings(timeNow, bindings));
    CPPUNIT_ASSERT(bindings.size() == 1);

    //
    // Must return one contact for bob for instrument-test
    //
    bindings.clear();
    CPPUNIT_ASSERT(_pRegDb->collection().getUnexpiredContactsUserInstrument("bob@biloxy.com", "instrument-test", timeNow, bindings));
    CPPUNIT_ASSERT(bindings.size() == 1);

    //
    // alice has two unexpired bindins for instrument-test
    //
    bindings.clear();
    CPPUNIT_ASSERT(_pRegDb->collection().getUnexpiredContactsUserInstrument("alice@atlanta.com", "instrument-test", timeNow, bindings));
    CPPUNIT_ASSERT(bindings.size() == 2);


    //
    // Clean and persist contacts
    //
    CPPUNIT_ASSERT(_pRegDb->collection().cleanAndPersist(timeNow));
    bindings.clear();
    CPPUNIT_ASSERT(!_pRegDb->collection().getAllOldBindings(timeNow, bindings));
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

      _pRegDb->collection().updateBinding(binding);
    }

    std::cout << "Time elapsed inserting 1000 bindings: " << (int) OsDateTime::getSecsSinceEpoch() - startTime << " seconds" << std::endl;
    std::cout.flush();

    startTime = (int) OsDateTime::getSecsSinceEpoch();
    bindings.clear();
    CPPUNIT_ASSERT(_pRegDb->collection().getAllOldBindings(timeNow, bindings));
    CPPUNIT_ASSERT(bindings.size() == 500);

    std::cout << "Time elapsed getting expired bindings: " << (int) OsDateTime::getSecsSinceEpoch() - startTime << " seconds" << std::endl;
    std::cout.flush();
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MongoDbTest);


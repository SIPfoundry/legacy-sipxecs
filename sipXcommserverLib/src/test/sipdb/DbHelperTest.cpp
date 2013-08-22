//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsDateTime.h>

#include "net/Url.h"
#include "sipdb/DbHelper.h"


using namespace std;

class DbHelperTest : public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE(DbHelperTest);
  CPPUNIT_TEST(testDbHelper_printDbEntriesRegBinding);
  CPPUNIT_TEST(testDbHelper_printDbEntriesEntityRecord);
  CPPUNIT_TEST(testDbHelper_deleteDbEntriesRegBinding);
  CPPUNIT_TEST(testDbHelper_deleteDbEntriesEntityRecord);
  CPPUNIT_TEST_SUITE_END();

  const MongoDB::ConnectionInfo _info;
  const std::string _dbHelperTest_RegBinding;
  const std::string _dbHelperTest_EntityRecord;
  DbHelper _dbHelper;

public:
  DbHelperTest() : _info(MongoDB::ConnectionInfo(mongo::ConnectionString(mongo::HostAndPort("localhost")))),
                   _dbHelperTest_RegBinding("test.DbHelperTest_RegBinding"),
                   _dbHelperTest_EntityRecord("test.DbHelperTest_EntityRecord")
  {
  }

  void setUp()
  {
    MongoDB::ScopedDbConnectionPtr pConnRegBinding(mongo::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));
    pConnRegBinding->get()->remove(_dbHelperTest_RegBinding, mongo::Query());
    pConnRegBinding->done();

    MongoDB::ScopedDbConnectionPtr pConnEntityRecord(mongo::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));
    pConnEntityRecord->get()->remove(_dbHelperTest_EntityRecord, mongo::Query());
    pConnEntityRecord->done();
  }

  void tearDown()
  {
  }

  void updateEntityRecord(EntityRecord& entityRecord)
  {

    mongo::BSONObj query = BSON(entityRecord.identity_fld() << entityRecord.identity());


    mongo::BSONObjBuilder bsonObjBuilder;
    bsonObjBuilder << entityRecord.userId_fld() << entityRecord.userId() <<                                           // "uid"
        entityRecord.identity_fld() << entityRecord.identity() <<                                       // "ident"
        entityRecord.realm_fld() << entityRecord.realm() <<                                             // "rlm"
        entityRecord.password_fld() << entityRecord.password() <<                                       // "pstk"
        entityRecord.pin_fld() << entityRecord.pin() <<                                                 // "pntk"
        entityRecord.authType_fld() << entityRecord.authType() <<                                       // "authtp"
        entityRecord.location_fld() << entityRecord.location() <<                                       // "loc"

        entityRecord.callerId_fld() << entityRecord.callerId().id <<                                    // "clrid"
        entityRecord.callerIdEnforcePrivacy_fld() << entityRecord.callerId().enforcePrivacy <<          // "blkcid"
        entityRecord.callerIdIgnoreUserCalleId_fld() << entityRecord.callerId().ignoreUserCalleId <<    // "ignorecid"
        entityRecord.callerIdTransformExtension_fld() << entityRecord.callerId().transformExtension <<  // "trnsfrmext"
        entityRecord.callerIdExtensionLength_fld() << entityRecord.callerId().extensionLength <<        // "kpdgts"
        entityRecord.callerIdExtensionPrefix_fld() << entityRecord.callerId().extensionPrefix <<        // "pfix"
        entityRecord.callForwardTime_fld() << entityRecord.callForwardTime() <<                         // "cfwdtm"

        entityRecord.permission_fld() << entityRecord.permissions();                                    // "prm"

    mongo::BSONObj update;
    update = BSON("$set" << bsonObjBuilder.obj());

    MongoDB::ScopedDbConnectionPtr conn(mongo::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));
    mongo::DBClientBase* client = conn->get();

    //client->insert(_info.getNS(), update);
    client->update(_dbHelperTest_EntityRecord, query, update, true, false);
    client->ensureIndex(_dbHelperTest_EntityRecord, BSON( entityRecord.identity_fld() << 1 ));

    conn->done();
  }

  void testDbHelper_printDbEntriesEntityRecord()
  {
    //
    // Create a entity record entry
    //
    EntityRecord entityRecord;
    entityRecord._userId = "userId";
    entityRecord._identity = "identity@atlanta.com";
    entityRecord._realm = "realm";
    entityRecord._password = "password";
    entityRecord._pin = "pin";
    entityRecord._authType = "authType";
    entityRecord._location = "location";

    // insert created entity record in test.DbHelperTest_EntityRecord database
    updateEntityRecord(entityRecord);

    std::ostringstream strm;
    std::vector<std::string> whereOptVector;

    // add filter condition
    whereOptVector.push_back("ident=identity@atlanta.com");
    _dbHelper.printDbEntries(strm, &_info, _dbHelperTest_EntityRecord, whereOptVector, DbHelper::DbTypeEntityRecord, false);


    // verify printed entries in strm
    CPPUNIT_ASSERT(std::string("\"Nr\":0   \"ident\":identity@atlanta.com   \"uid\":userId   \"rlm\":realm   \"pstk\""
        ":password   \"pntk\":pin   \"authtp\":authType   \"cfwdtm\":0   \"loc\":location   \"clrid\":   \"blkcid\""
        ":0   \"ignorecid\":0   \"trnsfrmext\":0   \"kpdgts\":0   \"pfix\":   \"prm\":[]   \"als\":[]   \"stc\":[]   \n") == strm.str());

    strm.str("");
    strm.clear();
    whereOptVector.clear();

    // should print no line as filter condition contain now '!='
    whereOptVector.push_back("ident!=identity@atlanta.com");
    _dbHelper.printDbEntries(strm, &_info, _dbHelperTest_EntityRecord, whereOptVector, DbHelper::DbTypeEntityRecord, false);

    CPPUNIT_ASSERT(std::string("") == strm.str());
  }


  void testDbHelper_printDbEntriesRegBinding()
  {
    RegDB regDb(_info, NULL, _dbHelperTest_RegBinding);

    //
    // Create a reg binding class
    //
    RegBinding::Ptr binding_0 = RegBinding::Ptr(new RegBinding());
    binding_0->setContact("sip:alice@host1.atlanta.com;transport=tcp");
    binding_0->setExpirationTime(1377098487);
    binding_0->setTimestamp(1377094887);
    binding_0->setQvalue("0");
    binding_0->setInstanceId("");
    binding_0->setGruu("");
    binding_0->setPath("sip:proxy01.atlanta.com");
    binding_0->setInstrument("instrument-test");
    binding_0->setCallId("call-id@12345");
    binding_0->setCseq(1);
    binding_0->setIdentity("alice@atlanta.com");
    binding_0->setUri("sip:alice@atalanta.com");

    // insert created reg binding in test.DbHelperTest_RegBinding database
    regDb.updateBinding(binding_0);

    std::ostringstream strm;
    std::vector<std::string> whereOptVector;

    // add filter condition
    whereOptVector.push_back("cseq=1");
    _dbHelper.printDbEntries(strm, &_info, _dbHelperTest_RegBinding, whereOptVector, DbHelper::DbTypeRegBinding, false);


    // verify printed entries in strm
    CPPUNIT_ASSERT(std::string("\"Nr\":0   \"callId\":call-id@12345   \"contact\":sip:alice@host1.atlanta.com;transport=tcp   \"cseq\":1   \"expirationTime\""
    ":1377098487   \"expirationTime\":2013-Aug-21 18:21:27   \"expired\":0   \"gruu\":   \"identity\":alice@atlanta.com   \"instanceId\""
    ":   \"instrument\":instrument-test   \"localAddress\":   \"path\":sip:proxy01.atlanta.com   \"qvalue\":0   \"timestamp\":1377094887"
    "   \"timestamp\":2013-Aug-21 17:21:27   \"uri\":sip:alice@atalanta.com   \n") == strm.str());

    strm.str("");
    strm.clear();
    whereOptVector.clear();

    // add filter condition
    whereOptVector.push_back("cseq=2");

    // should print nothing as now the filter condition is 'cseq=2'
    _dbHelper.printDbEntries(strm, &_info, _dbHelperTest_RegBinding, whereOptVector, DbHelper::DbTypeRegBinding, false);

    CPPUNIT_ASSERT(std::string("") == strm.str());
  }

  void testDbHelper_deleteDbEntriesRegBinding()
  {
    RegDB regDb(_info, NULL, _dbHelperTest_RegBinding);

    //
    // Create a reg binding
    //
    RegBinding::Ptr binding_0 = RegBinding::Ptr(new RegBinding());
    binding_0->setContact("sip:alice@host1.atlanta.com;transport=tcp");
    binding_0->setExpirationTime(1377098487);
    binding_0->setTimestamp(1377094887);
    binding_0->setQvalue("0");
    binding_0->setInstanceId("");
    binding_0->setGruu("");
    binding_0->setPath("sip:proxy01.atlanta.com");
    binding_0->setInstrument("instrument-test");
    binding_0->setCallId("call-id@12345");
    binding_0->setCseq(1);
    binding_0->setIdentity("alice@atlanta.com");
    binding_0->setUri("sip:alice@atalanta.com");

    // insert created reg binding in test.DbHelperTest_RegBinding database
    regDb.updateBinding(binding_0);


    std::vector<std::string> whereOptVector;

    // should delete all alements as filter condition is empty
    _dbHelper.deleteDbEntries(&_info, _dbHelperTest_RegBinding, whereOptVector);

    std::ostringstream strm;

    // should print nothing as all elements were deleted
    _dbHelper.printDbEntries(strm, &_info, _dbHelperTest_RegBinding, whereOptVector, DbHelper::DbTypeRegBinding, false);

    CPPUNIT_ASSERT(std::string("") == strm.str());
  }

  void testDbHelper_deleteDbEntriesEntityRecord()
  {
    //
    // Create a entity record entry
    //
    EntityRecord entityRecord;
    entityRecord._userId = "userId";
    entityRecord._identity = "identity@atlanta.com";
    entityRecord._realm = "realm";
    entityRecord._password = "password";
    entityRecord._pin = "pin";
    entityRecord._authType = "authType";
    entityRecord._location = "location";

    // insert created entity record in test.DbHelperTest_EntityRecord database
    updateEntityRecord(entityRecord);

    std::vector<std::string> whereOptVector;

    // should delete all alements as filter condition is empty
    _dbHelper.deleteDbEntries(&_info, _dbHelperTest_EntityRecord, whereOptVector);

    std::ostringstream strm;

    // should print nothing as all elements were deleted
    _dbHelper.printDbEntries(strm, &_info, _dbHelperTest_EntityRecord, whereOptVector, DbHelper::DbTypeEntityRecord, false);

    CPPUNIT_ASSERT(std::string("") == strm.str());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(DbHelperTest);

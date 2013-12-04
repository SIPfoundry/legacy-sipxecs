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

const char* gLocalHostAddr = "localhost";
const char* gTestRegBindingDbName = "test.DbHelperTest_RegBinding";
const char* gTestEntityRecordDbName = "test.DbHelperTest_EntityRecord";
const char* gMongoSetOperator = "$set";

typedef struct
{
  const char* pUserId;
  const char* pIdentity;
  const char* pRealm;
  const char* pPassword;
  const char* pPin;
  const char* pAuthType;
  const char* pCallForwardTime;
  const char* pLocation;
  const char* pcallerId;
  const char* pCallerIdEnforcePrivacy;
  const char* pCallerIdIgnoreUserCalleId;
  const char* pCallerIdTransformExtension;
  const char* pCallerIdExtensionLength;
  const char* pCallerIdExtensionPrefix;
  const char* pPermission;
  const char* pAliases;
  const char* pStaticUserLoc;
  const char* pGruu;
  const char* pPath;
} EntityRecordTestData;

EntityRecordTestData entityRecordTestData[] =
{
  {
    "userId",
    "identity@atlanta.com",
    "realm",
    "password",
    "pin",
    "authType",
    "0",
    "location",
    "",
    "0",
    "0",
    "0",
    "0",
    "",
    "[]",
    "[]",
    "[]",
    "",
    ""
  }
};

typedef struct
{
  const char* pContact;
  unsigned int expirationTime;
  unsigned int timeStamp;
  bool expired;
  const char* pQValue;
  const char* pInstanceId;
  const char* pGruu;
  const char* pPath;
  const char* pInstrument;
  const char* pCallId;
  unsigned int cseq;
  const char* pIdentity;
  const char* pUri;
  const char* pLocalAddress;
} RegBindingTestData;

RegBindingTestData regBindingTestData[] =
{
  {
    "sip:alice@host1.atlanta.com;transport=tcp",
    1377098487,
    1377094887,
    0,
    "0",
    "",
    "",
    "sip:proxy01.atlanta.com",
    "instrument-test",
    "call-id@12345",
    1,
    "alice@atlanta.com",
    "sip:alice@atlanta.com",
    ""
  }
};

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
  DbHelperTest() : _info(MongoDB::ConnectionInfo(mongo::ConnectionString(mongo::HostAndPort(gLocalHostAddr)))),
                   _dbHelperTest_RegBinding(gTestRegBindingDbName),
                   _dbHelperTest_EntityRecord(gTestEntityRecordDbName)
  {
  }

  void setUp()
  {
    MongoDB::ScopedDbConnectionPtr pConnRegBinding(mongoMod::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));
    pConnRegBinding->get()->remove(_dbHelperTest_RegBinding, mongo::Query());
    pConnRegBinding->done();

    MongoDB::ScopedDbConnectionPtr pConnEntityRecord(mongoMod::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));
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
    update = BSON(gMongoSetOperator << bsonObjBuilder.obj());

    MongoDB::ScopedDbConnectionPtr conn(mongoMod::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));
    mongo::DBClientBase* client = conn->get();

    //client->insert(_info.getNS(), update);
    client->update(_dbHelperTest_EntityRecord, query, update, true, false);
    client->ensureIndex(_dbHelperTest_EntityRecord, BSON( entityRecord.identity_fld() << 1 ));

    conn->done();
  }

  void setEntityRecordDefaultValues(EntityRecord& entityRecord)
  {
    entityRecord._userId = entityRecordTestData[0].pUserId;
    entityRecord._identity = entityRecordTestData[0].pIdentity;
    entityRecord._realm = entityRecordTestData[0].pRealm;
    entityRecord._password = entityRecordTestData[0].pPassword;
    entityRecord._pin = entityRecordTestData[0].pPin;
    entityRecord._authType = entityRecordTestData[0].pAuthType;
    entityRecord._location = entityRecordTestData[0].pLocation;
  }

  void testDbHelper_printDbEntriesEntityRecord()
  {
    //
    // Create a entity record entry
    //
    EntityRecord entityRecord;
    setEntityRecordDefaultValues(entityRecord);

    // insert created entity record in test.DbHelperTest_EntityRecord database
    updateEntityRecord(entityRecord);

    std::ostringstream strm;
    std::vector<std::string> whereOptVector;
    std::string filterCondition = (boost::format("%s=%s") % entityRecord.identity_fld() % entityRecordTestData[0].pIdentity).str();

    // add filter condition
    whereOptVector.push_back(filterCondition);
    _dbHelper.printDbEntries(strm, &_info, _dbHelperTest_EntityRecord, whereOptVector, DbHelper::DbTypeEntityRecord, false);

    std::string result = (boost::format("\"Nr\":0   \"%s\":%s   \"%s\":%s   \"%s\":%s   \"%s\":%s   "
        "\"%s\":%s   \"%s\":%s   \"%s\":%s   \"%s\":%s   \"%s\":%s   \"%s\":%s   "
        "\"%s\":%s   \"%s\":%s   \"%s\":%s   \"%s\":%s   \"%s\":%s   \"%s\":%s   \"%s\":%s   \n") %
        entityRecord.identity_fld() % entityRecordTestData[0].pIdentity %
        entityRecord.userId_fld() % entityRecordTestData[0].pUserId %
        entityRecord.realm_fld() % entityRecordTestData[0].pRealm %
        entityRecord.password_fld() % entityRecordTestData[0].pPassword %
        entityRecord.pin_fld() % entityRecordTestData[0].pPin %
        entityRecord.authType_fld() % entityRecordTestData[0].pAuthType %
        entityRecord.callForwardTime_fld() % entityRecordTestData[0].pCallForwardTime %
        entityRecord.location_fld() % entityRecordTestData[0].pLocation %
        entityRecord.callerId_fld() % entityRecordTestData[0].pcallerId %
        entityRecord.callerIdEnforcePrivacy_fld() % entityRecordTestData[0].pCallerIdEnforcePrivacy %
        entityRecord.callerIdIgnoreUserCalleId_fld() % entityRecordTestData[0].pCallerIdIgnoreUserCalleId %
        entityRecord.callerIdTransformExtension_fld() % entityRecordTestData[0].pCallerIdTransformExtension %
        entityRecord.callerIdExtensionLength_fld() % entityRecordTestData[0].pCallerIdExtensionLength %
        entityRecord.callerIdExtensionPrefix_fld() % entityRecordTestData[0].pCallerIdExtensionPrefix %
        entityRecord.permission_fld() % entityRecordTestData[0].pPermission %
        entityRecord.aliases_fld() % entityRecordTestData[0].pAliases %
        entityRecord.staticUserLoc_fld() % entityRecordTestData[0].pStaticUserLoc
        ).str();

    // TEST: Verify printed entries in strm
    CPPUNIT_ASSERT(result == strm.str());

    strm.str("");
    strm.clear();
    whereOptVector.clear();
    filterCondition.clear();
    filterCondition = (boost::format("%s!=%s") % entityRecord.identity_fld() % entityRecordTestData[0].pIdentity).str();

    // should print no line as filter condition contain now '!='
    whereOptVector.push_back(filterCondition);
    _dbHelper.printDbEntries(strm, &_info, _dbHelperTest_EntityRecord, whereOptVector, DbHelper::DbTypeEntityRecord, false);

    // TEST: Verify that the result is empty
    CPPUNIT_ASSERT(std::string("") == strm.str());
  }

  void setRegBindingDefaultValues(const RegBinding::Ptr& pBinding)
  {
    RegBinding::Ptr binding_0 = RegBinding::Ptr(new RegBinding());
    pBinding->setContact(regBindingTestData[0].pContact);
    pBinding->setExpirationTime(regBindingTestData[0].expirationTime);
    pBinding->setTimestamp(regBindingTestData[0].timeStamp);
    pBinding->setQvalue(regBindingTestData[0].pQValue);
    pBinding->setInstanceId(regBindingTestData[0].pInstanceId);
    pBinding->setGruu(regBindingTestData[0].pGruu);
    pBinding->setPath(regBindingTestData[0].pPath);
    pBinding->setInstrument(regBindingTestData[0].pInstrument);
    pBinding->setCallId(regBindingTestData[0].pCallId);
    pBinding->setCseq(regBindingTestData[0].cseq);
    pBinding->setIdentity(regBindingTestData[0].pIdentity);
    pBinding->setUri(regBindingTestData[0].pUri);
  }

  void testDbHelper_printDbEntriesRegBinding()
  {
    RegDB regDb(_info, NULL, _dbHelperTest_RegBinding);

    //
    // Create a reg binding class
    //
    RegBinding::Ptr binding_0 = RegBinding::Ptr(new RegBinding());
    setRegBindingDefaultValues(binding_0);

    // insert created reg binding in test.DbHelperTest_RegBinding database
    regDb.updateBinding(binding_0);

    std::ostringstream strm;
    std::vector<std::string> whereOptVector;
    std::string filterCondition = (boost::format("%s=%s") % RegBinding::cseq_fld() % regBindingTestData[0].cseq).str();

    // add filter condition "cseq=1"
    whereOptVector.push_back(filterCondition);
    _dbHelper.printDbEntries(strm, &_info, _dbHelperTest_RegBinding, whereOptVector, DbHelper::DbTypeRegBinding, false);

    std::string result = (boost::format("\"Nr\":0   \"%s\":%s   \"%s\":%s   \"%s\":%d   \"%s\":%d   "
        "\"%s\":%s   \"%s\":%d   \"%s\":%s   \"%s\":%s   \"%s\":%s   "
        "\"%s\":%s   \"%s\":%s   \"%s\":%s   \"%s\":%s   \"%s\":%d"
        "   \"%s\":%s   \"%s\":%s   \n") %
        RegBinding::callId_fld() % regBindingTestData[0].pCallId %
        RegBinding::contact_fld() % regBindingTestData[0].pContact %
        RegBinding::cseq_fld() % regBindingTestData[0].cseq %
        RegBinding::expirationTime_fld() % regBindingTestData[0].expirationTime %
        RegBinding::expirationTime_fld() % _dbHelper.convertSecondsToLocalTime(regBindingTestData[0].expirationTime) %
        RegBinding::expired_fld() % regBindingTestData[0].expired %
        RegBinding::gruu_fld() % regBindingTestData[0].pGruu %
        RegBinding::identity_fld() % regBindingTestData[0].pIdentity%
        RegBinding::instanceId_fld() % regBindingTestData[0].pInstanceId %
        RegBinding::instrument_fld() % regBindingTestData[0].pInstrument %
        RegBinding::localAddress_fld() % regBindingTestData[0].pLocalAddress %
        RegBinding::path_fld() % regBindingTestData[0].pPath %
        RegBinding::qvalue_fld() % regBindingTestData[0].pQValue %
        RegBinding::timestamp_fld() % regBindingTestData[0].timeStamp %
        RegBinding::timestamp_fld() % _dbHelper.convertSecondsToLocalTime(regBindingTestData[0].timeStamp) %
        RegBinding::uri_fld() % regBindingTestData[0].pUri
        ).str();

    // TEST: Verify printed entries in strm
    CPPUNIT_ASSERT(result == strm.str());

    strm.str("");
    strm.clear();
    whereOptVector.clear();
    filterCondition.clear();
    filterCondition = (boost::format("%s=%s") % RegBinding::cseq_fld() % 2).str();

    // add filter condition "cseq=2"
    whereOptVector.push_back(filterCondition);

    // should print nothing as now the filter condition is 'cseq=2'
    _dbHelper.printDbEntries(strm, &_info, _dbHelperTest_RegBinding, whereOptVector, DbHelper::DbTypeRegBinding, false);

    // TEST: Verify that the result is empty
    CPPUNIT_ASSERT(std::string("") == strm.str());
  }

  void testDbHelper_deleteDbEntriesRegBinding()
  {
    RegDB regDb(_info, NULL, _dbHelperTest_RegBinding);

    //
    // Create a reg binding
    //
    RegBinding::Ptr binding_0 = RegBinding::Ptr(new RegBinding());
    setRegBindingDefaultValues(binding_0);

    // insert created reg binding in test.DbHelperTest_RegBinding database
    regDb.updateBinding(binding_0);


    std::vector<std::string> whereOptVector;

    // should delete all alements as filter condition is empty
    _dbHelper.deleteDbEntries(&_info, _dbHelperTest_RegBinding, whereOptVector);

    std::ostringstream strm;

    // should print nothing as all elements were deleted
    _dbHelper.printDbEntries(strm, &_info, _dbHelperTest_RegBinding, whereOptVector, DbHelper::DbTypeRegBinding, false);

    // TEST: Verify that the result is empty
    CPPUNIT_ASSERT(std::string("") == strm.str());
  }

  void testDbHelper_deleteDbEntriesEntityRecord()
  {
    //
    // Create a entity record entry
    //
    EntityRecord entityRecord;
    setEntityRecordDefaultValues(entityRecord);

    // insert created entity record in test.DbHelperTest_EntityRecord database
    updateEntityRecord(entityRecord);

    std::vector<std::string> whereOptVector;

    // should delete all alements as filter condition is empty
    _dbHelper.deleteDbEntries(&_info, _dbHelperTest_EntityRecord, whereOptVector);

    std::ostringstream strm;

    // should print nothing as all elements were deleted
    _dbHelper.printDbEntries(strm, &_info, _dbHelperTest_EntityRecord, whereOptVector, DbHelper::DbTypeEntityRecord, false);

    // TEST: Verify that the result is empty
    CPPUNIT_ASSERT(std::string("") == strm.str());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(DbHelperTest);

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <sipxunit/TestUtilities.h>
#include <sipdb/EntityRecord.h>
#include <os/OsDateTime.h>
#include <mongo/util/net/hostandport.h>
#include <mongo/client/connpool.h>

#include <boost/format.hpp>


using namespace std;

typedef struct
{
  const char* pOid;
  const char* pUserId;
  const char* pIdentity;
  const char* pRealm;
  const char* pPassword;
  const char* pPin;
  const char* pAuthType;
  const char* pLocation;
  const char* pCallerId;
  bool callerIdEnforcePrivacy;
  bool callerIdIgnoreUserCalleId;
  bool callerIdTransformExtension;
  int callerIdExtensionLength;
  const char* pCallerIdExtensionPrefix;
  const char* pCallerIdType;
  int callForwardTime;
  bool vmOnDnd;
} EntityRecordTestData;

EntityRecordTestData entityRecordTestData[] =
{
  {
    "oid",
    "userId",
    "identity@atlanta.com",
    "realm",
    "password",
    "pin",
    "authType",
    "location",
    "id",
    true,
    true,
    true,
    128,
    "extensionPrefix",
    "user",
    30,
    true
  }
};

class EntityRecordTest: public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE(EntityRecordTest);
  CPPUNIT_TEST(testEntityRecordConstructor_WithEntityRecordParameter);
  CPPUNIT_TEST(testEntityRecordConstructor_WithSwapFunction);
  CPPUNIT_TEST(testEntityRecordConstructor_OperatorEqual_WithEntityRecordParameter);
  CPPUNIT_TEST(testEntityRecordConstructor_OperatorEqual_WithBSONObjParameter);
  CPPUNIT_TEST_SUITE_END();

public:

  EntityRecordTest()
{
}

  void setEntityRecord(EntityRecord& entityRecord)
  {

    entityRecord._oid = entityRecordTestData[0].pOid;
    entityRecord._userId = entityRecordTestData[0].pUserId;
    entityRecord._identity = entityRecordTestData[0].pIdentity;
    entityRecord._realm = entityRecordTestData[0].pRealm;
    entityRecord._password = entityRecordTestData[0].pPassword;
    entityRecord._pin = entityRecordTestData[0].pPin;
    entityRecord._authType = entityRecordTestData[0].pAuthType;
    entityRecord._location = entityRecordTestData[0].pLocation;

    entityRecord._callerId.id = entityRecordTestData[0].pCallerId;
    entityRecord._callerId.type = entityRecordTestData[0].pCallerIdType;
    entityRecord._callerId.enforcePrivacy = entityRecordTestData[0].callerIdEnforcePrivacy;
    entityRecord._callerId.ignoreUserCalleId = entityRecordTestData[0].callerIdIgnoreUserCalleId;
    entityRecord._callerId.transformExtension = entityRecordTestData[0].callerIdTransformExtension;
    entityRecord._callerId.extensionLength = entityRecordTestData[0].callerIdExtensionLength;
    entityRecord._callerId.extensionPrefix = entityRecordTestData[0].pCallerIdExtensionPrefix;

    entityRecord._callForwardTime = entityRecordTestData[0].callForwardTime;

    // add permissions
    for (int i = 1; i <= 2; i++)
    {
      entityRecord._permissions.insert((boost::format("%s_%d") % EntityRecord::permission_fld() % i).str());
    }

    // add aliases
    for (int i = 0; i < 3; i++)
    {
      EntityRecord::Alias alias;
      alias.id = (boost::format("%s_%d") % EntityRecord::aliasesId_fld() % i).str();
      alias.relation = (boost::format("%s_%d") % EntityRecord::aliasesRelation_fld() % i).str();
      alias.contact = (boost::format("%s_%d") % EntityRecord::aliasesContact_fld() % i).str();

      entityRecord._aliases.push_back(alias);
    }

    // add static user locations
    for (int i = 0; i < 4; i++)
    {
      EntityRecord::StaticUserLoc staticUserLoc;
      staticUserLoc.event = (boost::format("%s_%d") % EntityRecord::staticUserLocEvent_fld() % i).str();
      staticUserLoc.contact = (boost::format("%s_%d") % EntityRecord::staticUserLocContact_fld() % i).str();
      staticUserLoc.fromUri = (boost::format("%s_%d") % EntityRecord::staticUserLocFromUri_fld() % i).str();
      staticUserLoc.toUri = (boost::format("%s_%d") % EntityRecord::staticUserLocToUri_fld() % i).str();
      staticUserLoc.callId = (boost::format("%s_%d") % EntityRecord::staticUserLocCallId_fld() % i).str();

      entityRecord._staticUserLoc.push_back(staticUserLoc);
    }

    entityRecord._vmOnDnd = entityRecordTestData[0].vmOnDnd;
  }

  void createBSONObj(EntityRecord& entityRecord,  mongo::BSONObj& bsonObj)
  {
    mongo::BSONObjBuilder bsonObjBuilder;


    bsonObjBuilder << entityRecord.oid_fld() << entityRecord.oid() <<                                     // "_id"
        entityRecord.userId_fld() << entityRecord.userId() <<                                           // "uid"
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


    // build aliases
    mongo::BSONArrayBuilder bsonArrayBuilderAliases;
    for (std::vector<EntityRecord::Alias>::iterator iter = entityRecord._aliases.begin();
        iter != entityRecord._aliases.end(); iter++)
    {
      mongo::BSONObjBuilder bsonObjBuilderAlias;

      bsonObjBuilderAlias << entityRecord.aliasesId_fld() << iter->id;                                // "id"
      bsonObjBuilderAlias << entityRecord.aliasesContact_fld() << iter->contact;                      // "cnt"
      bsonObjBuilderAlias << entityRecord.aliasesRelation_fld() << iter->relation;                    // "rln"

      bsonArrayBuilderAliases.append(bsonObjBuilderAlias.obj());
    }
    bsonObjBuilder.append(entityRecord.aliases_fld(), bsonArrayBuilderAliases.arr());                   // "als"

    // build staticUserLocation
    mongo::BSONArrayBuilder bsonArrayBuilderStaticUserLoc;
    for (std::vector<EntityRecord::StaticUserLoc>::iterator iter = entityRecord._staticUserLoc.begin();
        iter != entityRecord._staticUserLoc.end(); iter++)
    {
      mongo::BSONObjBuilder bsonObjBuilderStaticUserLoc;

      bsonObjBuilderStaticUserLoc << entityRecord.staticUserLocEvent_fld() << iter->event;                    // "evt"
      bsonObjBuilderStaticUserLoc << entityRecord.staticUserLocContact_fld() << iter->contact;                // "cnt"
      bsonObjBuilderStaticUserLoc << entityRecord.staticUserLocFromUri_fld() << iter->fromUri;                // "from"
      bsonObjBuilderStaticUserLoc << entityRecord.staticUserLocToUri_fld() << iter->toUri;                    // "to"
      bsonObjBuilderStaticUserLoc << entityRecord.staticUserLocCallId_fld() << iter->callId;                  // "cid"

      bsonArrayBuilderStaticUserLoc.append(bsonObjBuilderStaticUserLoc.obj());
    }
    bsonObjBuilder.append(entityRecord.staticUserLoc_fld(), bsonArrayBuilderStaticUserLoc.arr());             // "stc"


    bsonObjBuilder << entityRecord.vmOnDnd_fld() << entityRecord.vmOnDnd();                                   // "vmondnd"


    bsonObj = bsonObjBuilder.obj();
  }

  void setUp()
  {
  }

  void tearDown()
  {
  }

  void testEntityRecordParameters(EntityRecord& entityRecord)
  {
    CPPUNIT_ASSERT(entityRecordTestData[0].pOid == entityRecord.oid());
    CPPUNIT_ASSERT(entityRecordTestData[0].pUserId == entityRecord.userId());
    CPPUNIT_ASSERT(entityRecordTestData[0].pIdentity == entityRecord.identity());
    CPPUNIT_ASSERT(entityRecordTestData[0].pRealm == entityRecord.realm());
    CPPUNIT_ASSERT(entityRecordTestData[0].pPassword == entityRecord.password());
    CPPUNIT_ASSERT(entityRecordTestData[0].pPin == entityRecord.pin());
    CPPUNIT_ASSERT(entityRecordTestData[0].pAuthType == entityRecord.authType());
    CPPUNIT_ASSERT(entityRecordTestData[0].pLocation == entityRecord.location());

    CPPUNIT_ASSERT(entityRecordTestData[0].pCallerId == entityRecord.callerId().id);
    CPPUNIT_ASSERT(entityRecordTestData[0].callerIdEnforcePrivacy == entityRecord.callerId().enforcePrivacy);
    CPPUNIT_ASSERT(entityRecordTestData[0].callerIdIgnoreUserCalleId == entityRecord.callerId().ignoreUserCalleId);
    CPPUNIT_ASSERT(entityRecordTestData[0].callerIdTransformExtension == entityRecord.callerId().transformExtension);
    CPPUNIT_ASSERT(entityRecordTestData[0].callerIdExtensionLength == entityRecord.callerId().extensionLength);
    CPPUNIT_ASSERT(entityRecordTestData[0].pCallerIdExtensionPrefix == entityRecord.callerId().extensionPrefix);
    CPPUNIT_ASSERT(entityRecordTestData[0].pCallerIdType == entityRecord.callerId().type);

    CPPUNIT_ASSERT(entityRecordTestData[0].callForwardTime == entityRecord._callForwardTime);

    // TEST: Check permissions
    CPPUNIT_ASSERT(2 == entityRecord._permissions.size());
    int permissionNr = 1;
    for (std::set<std::string>::iterator iter = entityRecord._permissions.begin();
        iter != entityRecord._permissions.end(); iter++)
    {
      std::string permission = (boost::format("%s_%d") % EntityRecord::permission_fld() % permissionNr).str();
      CPPUNIT_ASSERT(permission == *iter);
      permissionNr++;
    }

    // TEST: Check aliases
    CPPUNIT_ASSERT(3 == entityRecord._aliases.size());
    int aliasNr = 0;
    for (std::vector<EntityRecord::Alias>::iterator iter = entityRecord._aliases.begin();
        iter != entityRecord._aliases.end(); iter++)
    {
      std::string aliasId = (boost::format("%s_%d") % EntityRecord::aliasesId_fld() % aliasNr).str();
      std::string aliasContact = (boost::format("%s_%d") % EntityRecord::aliasesContact_fld() % aliasNr).str();
      std::string aliasRelation = (boost::format("%s_%d") % EntityRecord::aliasesRelation_fld() % aliasNr).str();

      CPPUNIT_ASSERT(aliasId == iter->id);
      CPPUNIT_ASSERT(aliasContact == iter->contact);
      CPPUNIT_ASSERT(aliasRelation == iter->relation);

      aliasNr++;
    }

    // TEST: check staticUserLoc
    CPPUNIT_ASSERT(4 == entityRecord._staticUserLoc.size());
    int staticUserLocNr = 0;
    for (std::vector<EntityRecord::StaticUserLoc>::iterator iter = entityRecord._staticUserLoc.begin();
        iter != entityRecord._staticUserLoc.end(); iter++)
    {
      std::string staticUserLocEvent = (boost::format("%s_%d") % EntityRecord::staticUserLocEvent_fld() % staticUserLocNr).str();
      std::string staticUserLocContact = (boost::format("%s_%d") % EntityRecord::staticUserLocContact_fld() % staticUserLocNr).str();
      std::string staticUserLocFromUri = (boost::format("%s_%d") % EntityRecord::staticUserLocFromUri_fld() % staticUserLocNr).str();
      std::string staticUserLocToUri = (boost::format("%s_%d") % EntityRecord::staticUserLocToUri_fld() % staticUserLocNr).str();
      std::string staticUserLocCallId = (boost::format("%s_%d") % EntityRecord::staticUserLocCallId_fld() % staticUserLocNr).str();

      CPPUNIT_ASSERT(staticUserLocEvent == iter->event);
      CPPUNIT_ASSERT(staticUserLocContact == iter->contact);
      CPPUNIT_ASSERT(staticUserLocFromUri == iter->fromUri);
      CPPUNIT_ASSERT(staticUserLocToUri == iter->toUri);
      CPPUNIT_ASSERT(staticUserLocCallId == iter->callId);

      staticUserLocNr++;
    }

    CPPUNIT_ASSERT(entityRecordTestData[0].vmOnDnd == entityRecord.vmOnDnd());
  }

  void testEntityRecordConstructor_WithEntityRecordParameter()
  {
    EntityRecord entityRecord;

    // init Entity Record with default values
    setEntityRecord(entityRecord);

    // Create a new Entity Record using the constructor that have Entity Record as input
    // parameter
    EntityRecord entityRecordConstructor(entityRecord);

    // TEST: Check the values of the new created Entity Record structure
    testEntityRecordParameters(entityRecordConstructor);
  }

  void testEntityRecordConstructor_WithSwapFunction()
  {
    EntityRecord entityRecord;

    // init Entity Record with default values
    setEntityRecord(entityRecord);

    EntityRecord entityRecordConstructor;

    // use the swap function in order to copy values
    entityRecordConstructor.swap(entityRecord);

    // TEST: Check the values for the new created Entity Record structure
    testEntityRecordParameters(entityRecordConstructor);
  }

  void testEntityRecordConstructor_OperatorEqual_WithEntityRecordParameter()
  {
    EntityRecord entityRecord;

    // init Entity Record with default values
    setEntityRecord(entityRecord);

    // copy the values using the operator= function
    EntityRecord entityRecordConstructor = entityRecord;

    // TEST: Check the values for the new created Entity Record structure
    testEntityRecordParameters(entityRecordConstructor);
  }

  void testEntityRecordConstructor_OperatorEqual_WithBSONObjParameter()
  {
    EntityRecord entityRecord;

    setEntityRecord(entityRecord);

    mongo::BSONObj bsonObj;

    // create BSONObj from Entity Record class
    createBSONObj(entityRecord, bsonObj);


    EntityRecord entityRecordConstructor;

    // copy the values using operator= function
    entityRecordConstructor = bsonObj;

    // TEST: Check the values for the new created Entity Record structure
    testEntityRecordParameters(entityRecordConstructor);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(EntityRecordTest);


#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <sipxunit/TestUtilities.h>
#include <sipdb/EntityRecord.h>
#include <os/OsDateTime.h>
#include <mongo/util/net/hostandport.h>
#include <mongo/client/connpool.h>

#include <boost/format.hpp>


using namespace std;



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

    entityRecord._oid = "oid";
    entityRecord._userId = "userId";
    entityRecord._identity = "identity";
    entityRecord._realm = "realm";
    entityRecord._password = "password";
    entityRecord._pin = "pin";
    entityRecord._authType = "authType";
    entityRecord._location = "location";

    entityRecord._callerId.id = "id";
    entityRecord._callerId.type = "type";
    entityRecord._callerId.enforcePrivacy = true;
    entityRecord._callerId.ignoreUserCalleId = true;
    entityRecord._callerId.transformExtension = true;
    entityRecord._callerId.extensionLength = 128;
    entityRecord._callerId.extensionPrefix = "extensionPrefix";
    entityRecord._callerId.type = "user";

    entityRecord._callForwardTime = 30;

    // add permissions
    for (int i = 1; i <= 2; i++)
    {
      entityRecord._permissions.insert((boost::format("permission_%d") % i).str());
    }

    // add aliases
    for (int i = 0; i < 3; i++)
    {
      EntityRecord::Alias alias;
      alias.id = (boost::format("id_%d") % i).str();
      alias.relation = (boost::format("relation_%d") % i).str();
      alias.contact = (boost::format("contact_%d") % i).str();

      entityRecord._aliases.push_back(alias);
    }

    // add static user locations
    for (int i = 0; i < 4; i++)
    {
      EntityRecord::StaticUserLoc staticUserLoc;
      staticUserLoc.event = (boost::format("event_%d") % i).str();
      staticUserLoc.contact = (boost::format("contact_%d") % i).str();
      staticUserLoc.fromUri = (boost::format("fromUri_%d") % i).str();
      staticUserLoc.toUri = (boost::format("toUri_%d") % i).str();
      staticUserLoc.callId = (boost::format("callId_%d") % i).str();

      entityRecord._staticUserLoc.push_back(staticUserLoc);
    }

    entityRecord._vmOnDnd = true;


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

  void testEntityRecordWithCorrectParameters(EntityRecord& entityRecord)
  {
    CPPUNIT_ASSERT(string("oid") == entityRecord.oid());
    CPPUNIT_ASSERT(string("userId") == entityRecord.userId());
    CPPUNIT_ASSERT(string("identity") == entityRecord.identity());
    CPPUNIT_ASSERT(string("realm") == entityRecord.realm());
    CPPUNIT_ASSERT(string("password") == entityRecord.password());
    CPPUNIT_ASSERT(string("pin") == entityRecord.pin());
    CPPUNIT_ASSERT(string("authType") == entityRecord.authType());
    CPPUNIT_ASSERT(string("location") == entityRecord.location());

    CPPUNIT_ASSERT(string("id") == entityRecord.callerId().id);
    CPPUNIT_ASSERT(true == entityRecord.callerId().enforcePrivacy);
    CPPUNIT_ASSERT(true == entityRecord.callerId().ignoreUserCalleId);
    CPPUNIT_ASSERT(true == entityRecord.callerId().transformExtension);
    CPPUNIT_ASSERT(128 == entityRecord.callerId().extensionLength);
    CPPUNIT_ASSERT(string("extensionPrefix") == entityRecord.callerId().extensionPrefix);
    CPPUNIT_ASSERT(string("user") == entityRecord.callerId().type);

    CPPUNIT_ASSERT(30 == entityRecord._callForwardTime);

    // check permissions
    CPPUNIT_ASSERT(2 == entityRecord._permissions.size());
    int permissionNr = 1;
    for (std::set<std::string>::iterator iter = entityRecord._permissions.begin();
      iter != entityRecord._permissions.end(); iter++)
    {
      std::string permission = (boost::format("permission_%d") % permissionNr).str();
      CPPUNIT_ASSERT(permission == *iter);
      permissionNr++;
    }

    // check aliases
    CPPUNIT_ASSERT(3 == entityRecord._aliases.size());
    int aliasNr = 0;
    for (std::vector<EntityRecord::Alias>::iterator iter = entityRecord._aliases.begin();
      iter != entityRecord._aliases.end(); iter++)
    {
      std::string aliasId = (boost::format("id_%d") % aliasNr).str();
      std::string aliasContact = (boost::format("contact_%d") % aliasNr).str();
      std::string aliasRelation = (boost::format("relation_%d") % aliasNr).str();

      CPPUNIT_ASSERT(aliasId == iter->id);
      CPPUNIT_ASSERT(aliasContact == iter->contact);
      CPPUNIT_ASSERT(aliasRelation == iter->relation);

      aliasNr++;
    }

    // check staticUserLoc
    CPPUNIT_ASSERT(4 == entityRecord._staticUserLoc.size());
    int staticUserLocNr = 0;
    for (std::vector<EntityRecord::StaticUserLoc>::iterator iter = entityRecord._staticUserLoc.begin();
      iter != entityRecord._staticUserLoc.end(); iter++)
    {
      std::string staticUserLocEvent = (boost::format("event_%d") % staticUserLocNr).str();
      std::string staticUserLocContact = (boost::format("contact_%d") % staticUserLocNr).str();
      std::string staticUserLocFromUri = (boost::format("fromUri_%d") % staticUserLocNr).str();
      std::string staticUserLocToUri = (boost::format("toUri_%d") % staticUserLocNr).str();
      std::string staticUserLocCallId = (boost::format("callId_%d") % staticUserLocNr).str();

      CPPUNIT_ASSERT(staticUserLocEvent == iter->event);
      CPPUNIT_ASSERT(staticUserLocContact == iter->contact);
      CPPUNIT_ASSERT(staticUserLocFromUri == iter->fromUri);
      CPPUNIT_ASSERT(staticUserLocToUri == iter->toUri);
      CPPUNIT_ASSERT(staticUserLocCallId == iter->callId);

      staticUserLocNr++;
    }

    CPPUNIT_ASSERT(true == entityRecord.vmOnDnd());
  }

  void testEntityRecordWithIncorrectParameters(EntityRecord& entityRecord)
  {
    CPPUNIT_ASSERT(string("__oid") != entityRecord.oid());
    CPPUNIT_ASSERT(string("__userId") != entityRecord.userId());
    CPPUNIT_ASSERT(string("__identity") != entityRecord.identity());
    CPPUNIT_ASSERT(string("__realm") != entityRecord.realm());
    CPPUNIT_ASSERT(string("__password") != entityRecord.password());
    CPPUNIT_ASSERT(string("__pin") != entityRecord.pin());
    CPPUNIT_ASSERT(string("__authType") != entityRecord.authType());
    CPPUNIT_ASSERT(string("__location") != entityRecord.location());

    CPPUNIT_ASSERT(string("__id") != entityRecord.callerId().id);
    CPPUNIT_ASSERT(false != entityRecord.callerId().enforcePrivacy);
    CPPUNIT_ASSERT(false != entityRecord.callerId().ignoreUserCalleId);
    CPPUNIT_ASSERT(false != entityRecord.callerId().transformExtension);
    CPPUNIT_ASSERT(1288 != entityRecord.callerId().extensionLength);
    CPPUNIT_ASSERT(string("__extensionPrefix") != entityRecord.callerId().extensionPrefix);
    CPPUNIT_ASSERT(string("__user") != entityRecord.callerId().type);

    CPPUNIT_ASSERT(308 != entityRecord._callForwardTime);

    // check permissions
    CPPUNIT_ASSERT(2 == entityRecord._permissions.size());
    int permissionNr = 1;
    for (std::set<std::string>::iterator iter = entityRecord._permissions.begin();
      iter != entityRecord._permissions.end(); iter++)
    {
      std::string permission = (boost::format("__permission_%d") % permissionNr).str();
      CPPUNIT_ASSERT(permission != *iter);
      permissionNr++;
    }

    // check aliases
    CPPUNIT_ASSERT(3 == entityRecord._aliases.size());
    int aliasNr = 0;
    for (std::vector<EntityRecord::Alias>::iterator iter = entityRecord._aliases.begin();
      iter != entityRecord._aliases.end(); iter++)
    {
      std::string aliasId = (boost::format("__id_%d") % aliasNr).str();
      std::string aliasContact = (boost::format("__contact_%d") % aliasNr).str();
      std::string aliasRelation = (boost::format("__relation_%d") % aliasNr).str();

      CPPUNIT_ASSERT(aliasId != iter->id);
      CPPUNIT_ASSERT(aliasContact != iter->contact);
      CPPUNIT_ASSERT(aliasRelation != iter->relation);

      permissionNr++;
    }

    // check staticUserLoc
    CPPUNIT_ASSERT(4 == entityRecord._staticUserLoc.size());
    int staticUserLocNr = 0;
    for (std::vector<EntityRecord::StaticUserLoc>::iterator iter = entityRecord._staticUserLoc.begin();
      iter != entityRecord._staticUserLoc.end(); iter++)
    {
      std::string staticUserLocEvent = (boost::format("__event_%d") % staticUserLocNr).str();
      std::string staticUserLocContact = (boost::format("__contact_%d") % staticUserLocNr).str();
      std::string staticUserLocFromUri = (boost::format("__fromUri_%d") % staticUserLocNr).str();
      std::string staticUserLocToUri = (boost::format("__toUri_%d") % staticUserLocNr).str();
      std::string staticUserLocCallId = (boost::format("__callId_%d") % staticUserLocNr).str();

      CPPUNIT_ASSERT(staticUserLocEvent != iter->event);
      CPPUNIT_ASSERT(staticUserLocContact != iter->contact);
      CPPUNIT_ASSERT(staticUserLocFromUri != iter->fromUri);
      CPPUNIT_ASSERT(staticUserLocToUri != iter->toUri);
      CPPUNIT_ASSERT(staticUserLocCallId != iter->callId);

      staticUserLocNr++;
    }

    CPPUNIT_ASSERT(false != entityRecord.vmOnDnd());
  }

   void testEntityRecordConstructor_WithEntityRecordParameter()
   {
     EntityRecord entityRecord;

     // init Entity Record with default values
     setEntityRecord(entityRecord);

     // Create a new Entity Record using the constructor that have Entity Record as input
     // parameter
     EntityRecord entityRecordConstructor(entityRecord);

     // check the values of the new created Entity Record structure
     testEntityRecordWithCorrectParameters(entityRecordConstructor);
     testEntityRecordWithIncorrectParameters(entityRecordConstructor);
   }

   void testEntityRecordConstructor_WithSwapFunction()
   {
     EntityRecord entityRecord;

     // init Entity Record with default values
     setEntityRecord(entityRecord);

     EntityRecord entityRecordConstructor;

     // use the swap function in order to copy values
     entityRecordConstructor.swap(entityRecord);

     // check the values for the new created Entity Record structure
     testEntityRecordWithCorrectParameters(entityRecordConstructor);
     testEntityRecordWithIncorrectParameters(entityRecordConstructor);
   }

   void testEntityRecordConstructor_OperatorEqual_WithEntityRecordParameter()
   {
     EntityRecord entityRecord;

     // init Entity Record with default values
     setEntityRecord(entityRecord);

     // copy the values using the operator= function
     EntityRecord entityRecordConstructor = entityRecord;

     // check the values for the new created Entity Record structure
     testEntityRecordWithCorrectParameters(entityRecordConstructor);
     testEntityRecordWithIncorrectParameters(entityRecordConstructor);
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

     // check the values for the new created Entity Record structure
     testEntityRecordWithCorrectParameters(entityRecordConstructor);
     testEntityRecordWithIncorrectParameters(entityRecordConstructor);
   }

};

CPPUNIT_TEST_SUITE_REGISTRATION(EntityRecordTest);


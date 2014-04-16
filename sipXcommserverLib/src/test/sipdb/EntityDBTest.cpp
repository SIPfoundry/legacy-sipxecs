#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <sipxunit/TestUtilities.h>
#include <sipdb/EntityDB.h>
#include <sipdb/MongoDB.h>
//#include <os/OsDateTime.h>
#include <mongo/util/net/hostandport.h>
#include <mongo/client/connpool.h>

#include <boost/format.hpp>


using namespace std;

const char* gLocalHostAddr = "localhost";
const char* gTestEntityDbName = "test.EntityDBTest";
const char* gLocalOplogDbName = "local.oplog";
const char* gMongoSetOperator = "$set";
const char* gEntityRecordTestUri = "id_1@atlanta.com";

typedef struct
{
  const char* pUserId;
  const char* pIdentity;
  const char* pRealm;
  const char* pPassword;
  const char* pPin;
  const char* pAuthType;
  const char* pLocation;
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
    "location"
  }
};

typedef struct
{
  const char* pId;
  const char* pRelation;
  const char* pContact;
} EntityRecordAliasTestData;

EntityRecordAliasTestData entityRecordAliasTestData[] =
{
  {
    "id_0",
    "relation_0",
    "contact_0",
  },
  {
    "id_1",
    "relation_1",
    "contact_1",
  },
  {
    "id_2",
    "relation_2",
    "contact_2",
  },
};

typedef struct
{
  const char* pTs;
  const char* pId;
  const char* pH;
  const char* pV;
  const char* pOperation;
  const char* pNs;
  const char* pO;
} OpLogTestData;

OpLogTestData opLogTestData[] =
{
  {
    "TS time:Wed Aug 07 11:45:34 EEST 2013 inc:1",
    "2",
    "8224307453475422225",
    "10",
    "i",
    "imdb.entity",
    "name"
  },
  {
    "TS time:Wed Aug 07 11:45:34 EEST 2013 inc:1",
    "3",
    "8224307453475422225",
    "10",
    "d",
    "imdb.entity",
    "name"
  },
  {
    "TS time:Wed Aug 07 11:45:34 EEST 2013 inc:1",
    "4",
    "8224307453475422225",
    "10",
    "i",
    "imdb.entity",
    "name"
  },
  {
    "TS time:Wed Aug 07 11:45:34 EEST 2013 inc:1",
    "1",
    "8224307453475422225",
    "10",
    "i",
    "imdb.entity",
    "name"
  },
  {
    "TS time:Wed Aug 07 11:45:34 EEST 2013 inc:1",
    "5",
    "8224307453475422225",
    "10",
    "i",
    "imdb.entity",
    "name"
  }
};

class OpLog
{
public:
  OpLog(){}

  OpLog(const std::string& ts,
        const std::string& id,
        const std::string& h,
        const std::string& v,
        const std::string& operation,
        const std::string& ns,
        const std::string& o) : _ts(ts), _id(id), _h(h), _v(v), _operation(operation), _ns(ns), _o(o)
  {
  }

  std::string& getTs(){return _ts;}
  std::string& getId(){return _id;}
  std::string& getH(){return _h;}
  std::string& getV(){return _v;}
  std::string& getOperation(){return _operation;}
  std::string& getNs(){return _ns;}
  std::string& getO(){return _o;}

  void setTs(std::string ts){_ts = ts;}
  void setId(std::string id){_id = id;}
  void setH(std::string h){_h = h;}
  void setV(std::string v){_v = v;}
  void setOperation(std::string operation){_operation = operation;}
  void setNs(std::string ns){_ns = ns;}
  void setO(std::string o){_o = o;}

  static const char* id_fld(){ static std::string fld = "_id"; return fld.c_str(); }
  static const char* ts_fld(){ static std::string fld = "ts"; return fld.c_str(); }
  static const char* h_fld(){ static std::string fld = "h"; return fld.c_str(); }
  static const char* v_fld(){ static std::string fld = "v"; return fld.c_str(); }
  static const char* opearation_fld(){ static std::string fld = "op"; return fld.c_str(); }
  static const char* ns_fld(){ static std::string fld = "ns"; return fld.c_str(); }
  static const char* o_fld(){ static std::string fld = "o"; return fld.c_str(); }

protected:

private:
  std::string _ts;
  std::string _id;
  std::string _h;
  std::string _v;
  std::string _operation;
  std::string _ns;
  std::string _o;
};

class EntityDBTest: public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE(EntityDBTest);
  CPPUNIT_TEST(testEntityDB_getAll);
  CPPUNIT_TEST(testEntityDB_findByUserId);
  CPPUNIT_TEST(testEntityDB_findByIdentityOrAlias);
  CPPUNIT_TEST(testEntityDB_getAliasContacts);
  CPPUNIT_TEST(testEntityDB_tailFunction);
  CPPUNIT_TEST_SUITE_END();

  typedef boost::scoped_ptr<EntityRecord> EntityRecordPtr;
  typedef boost::scoped_ptr<EntityDB> EntityDBPtr;

  const MongoDB::ConnectionInfo _info;
  std::string _entityDbName;
  std::string _oplogDbName;
  EntityRecordPtr _entityRecord;
  MongoDB::ScopedDbConnectionPtr _conn;
  EntityDBPtr _db;
  int MAX_SECONDS_TO_WAIT;
public:
  EntityDBTest() : _info(MongoDB::ConnectionInfo(mongo::ConnectionString(mongo::HostAndPort(gLocalHostAddr)))),
              _entityDbName(gTestEntityDbName),
              _oplogDbName(gLocalOplogDbName),
              _entityRecord(new EntityRecord),
              _conn(mongoMod::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString())),
              _db(new EntityDB(_info, _entityDbName))
  {
    MAX_SECONDS_TO_WAIT = 10;

    _conn->get()->dropCollection(_oplogDbName);
    _conn->get()->remove(_entityDbName, mongo::Query());

    waitUtilDbIsEmpty(_conn);

    // Initialise Entity record structure
    setEntityRecord(*_entityRecord);

    // Insert Entity record entry in test.EntityDBTest
    updateEntityRecord(*_entityRecord);

    waitUtilDbHaveAtLeastOneEntry(_conn);
  }

  void waitUtilDb(MongoDB::ScopedDbConnectionPtr& pConn, bool empty)
  {
    int seconds = 0;
    while (empty == pConn->get()->findOne(_entityDbName, mongo::BSONObj()).isEmpty() &&
           seconds < MAX_SECONDS_TO_WAIT)
    {
      sleep(1);
      seconds++;
    }
  }

  void waitUtilDbIsEmpty(MongoDB::ScopedDbConnectionPtr& pConn)
  {
    waitUtilDb(pConn, false);
  }

  void waitUtilDbHaveAtLeastOneEntry(MongoDB::ScopedDbConnectionPtr& pConn)
  {
    waitUtilDb(pConn, true);
  }

  ~EntityDBTest()
  {
    _conn->done();
  }

  // this function is called before the run of each test
  void setUp()
  {

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

    mongo::BSONObj update;
    update = BSON(gMongoSetOperator << bsonObjBuilder.obj());

    mongo::DBClientBase* client = _conn->get();

    //client->insert(_info.getNS(), update);
    client->update(_entityDbName, query, update, true, false);
    client->ensureIndex(_entityDbName, BSON(entityRecord.identity_fld() << 1 ));
  }

  void setEntityRecord(EntityRecord& entityRecord)
  {
    entityRecord._userId = entityRecordTestData[0].pUserId;
    entityRecord._identity = entityRecordTestData[0].pIdentity;
    entityRecord._realm = entityRecordTestData[0].pRealm;
    entityRecord._password = entityRecordTestData[0].pPassword;
    entityRecord._pin = entityRecordTestData[0].pPin;
    entityRecord._authType = entityRecordTestData[0].pAuthType;
    entityRecord._location = entityRecordTestData[0].pLocation;

    for (int i = 0; i < 3; i++)
    {
      EntityRecord::Alias alias;
      alias.id = entityRecordAliasTestData[i].pId;
      alias.relation = entityRecordAliasTestData[i].pRelation;
      alias.contact = entityRecordAliasTestData[i].pContact;

      entityRecord._aliases.push_back(alias);
    }
  }

  void testEntityRecordParameters(EntityRecord& entityRecord)
  {
    CPPUNIT_ASSERT(string(entityRecordTestData[0].pUserId) == entityRecord.userId());
    CPPUNIT_ASSERT(string(entityRecordTestData[0].pIdentity) == entityRecord.identity());
    CPPUNIT_ASSERT(string(entityRecordTestData[0].pRealm) == entityRecord.realm());
    CPPUNIT_ASSERT(string(entityRecordTestData[0].pPassword) == entityRecord.password());
    CPPUNIT_ASSERT(string(entityRecordTestData[0].pPin) == entityRecord.pin());
    CPPUNIT_ASSERT(string(entityRecordTestData[0].pAuthType) == entityRecord.authType());
    CPPUNIT_ASSERT(string(entityRecordTestData[0].pLocation) == entityRecord.location());
  }

  void testEntityDB_getAll()
  {
    EntityRecord entityRecord;
    std::string identity(entityRecordTestData[0].pIdentity);

    // find the Entity Record in test.EntityDBTest database filtered by the given identity
    bool ret = _db->findByIdentity(identity, entityRecord);
    // TEST: check that return code is true
    CPPUNIT_ASSERT(true == ret);

    // TEST: Check entity record parameters
    testEntityRecordParameters(entityRecord);
  }

  void testEntityDB_findByUserId()
  {
    EntityRecord entityRecord;
    std::string userId(entityRecordTestData[0].pUserId);

    // find the Entity Record in test.EntityDBTest database filtered by the given user id
    bool ret = _db->findByUserId(userId, entityRecord);
    // TEST: check that return code is true
    CPPUNIT_ASSERT(true == ret);


    // TEST: Check entity record parameters
    testEntityRecordParameters(entityRecord);
  }

  void testEntityDB_findByIdentityOrAlias()
  {
    EntityRecord entityRecord;

    Url uri(gEntityRecordTestUri);

    // find the Entity Record in test.EntityDBTest database filtered by the given url
    bool ret = _db->findByIdentityOrAlias(uri, entityRecord);
    // TEST: check that return code is true
    CPPUNIT_ASSERT(true == ret);

    // TEST: Check entity record parameters
    testEntityRecordParameters(entityRecord);
  }

  void testEntityDB_getAliasContacts()
  {
    EntityRecord entityRecord;
    EntityDB::Aliases aliases;
    bool isUserIdentity = false;

    Url aliasIdentity(gEntityRecordTestUri);

    // find the alias contacts in test.EntityDBTest database filtered by the given url
    _db->getAliasContacts(aliasIdentity, aliases, isUserIdentity);

    // TEST: Check that aliases size is 1
    CPPUNIT_ASSERT(aliases.size() == 1);
  }

  void updateMongoOpLog(OpLog& opLog)
  {

    mongo::BSONObj query = BSON(opLog.id_fld() << opLog.getId());


    mongo::BSONObjBuilder bsonObjBuilder;
    bsonObjBuilder <<
        opLog.ts_fld() << opLog.getTs() <<                                       // "ts"
        opLog.h_fld() << opLog.getH() <<                                         // "h"
        opLog.v_fld() << opLog.getV() <<                                         // "v"
        opLog.opearation_fld() << opLog.getOperation() <<                        // "op"
        opLog.ns_fld() << opLog.getNs() <<                                       // "ns"
        opLog.o_fld() << opLog.getO();                                           // "o"

    mongo::BSONObj update;
    update = BSON(gMongoSetOperator << bsonObjBuilder.obj());

    mongo::DBClientBase* client = _conn->get();

    // create a capped collect in order to work with tailer cursor
    client->createCollection(_oplogDbName, 1024*1024, true, 0, 0);
    client->update(_oplogDbName, query, update, true, false);
    client->ensureIndex(_oplogDbName, BSON(opLog.id_fld() << 1 ));
  }

  void testEntityDB_tailFunction()
  {
    OpLog opLog(opLogTestData[0].pTs, opLogTestData[0].pId, opLogTestData[0].pH,
                opLogTestData[0].pV, opLogTestData[0].pOperation, opLogTestData[0].pNs, opLogTestData[0].pO);

    opLog.setId(opLogTestData[0].pId);
    updateMongoOpLog(opLog);

    opLog.setOperation(opLogTestData[1].pOperation);
    opLog.setId(opLogTestData[1].pId);

    updateMongoOpLog(opLog);

    opLog.setOperation(opLogTestData[2].pOperation);
    opLog.setId(opLogTestData[2].pId);
    opLog.setNs(opLogTestData[2].pNs);

    updateMongoOpLog(opLog);

    opLog.setOperation(opLogTestData[3].pOperation);
    opLog.setId(opLogTestData[3].pId);
    opLog.setNs(opLogTestData[3].pNs);

    updateMongoOpLog(opLog);

    std::vector<std::string> opLogs;

    _db->tail(opLogs);

//    for (std::vector<std::string>::const_iterator iter = opLogs.begin();
//         iter != opLogs.end(); iter++)
//    {
//       std::cout << boost::format("%s\n") % *iter;
//    }

    // TEST: Check that the number of opLogs elements is 3
    CPPUNIT_ASSERT(opLogs.size() == 3);


    opLog.setOperation(opLogTestData[4].pOperation);
    opLog.setId(opLogTestData[4].pId);
    opLog.setNs(opLogTestData[4].pNs);

    updateMongoOpLog(opLog);

    opLogs.clear();

    _db->tail(opLogs);

//    for (std::vector<std::string>::const_iterator iter = opLogs.begin();
//         iter != opLogs.end(); iter++)
//    {
//       std::cout << boost::format("%s\n") % *iter;
//    }

    // TEST: Check that the number of opLogs elements is 1
    CPPUNIT_ASSERT(opLogs.size() == 1);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(EntityDBTest);

 

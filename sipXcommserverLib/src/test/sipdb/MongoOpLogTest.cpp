#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <sipxunit/TestUtilities.h>
#include <sipdb/MongoOpLog.h>
#include <os/OsDateTime.h>
#include <mongo/util/net/hostandport.h>
#include <mongo/client/connpool.h>

#include <boost/format.hpp>


#include <boost/function.hpp>


using namespace std;

const char* gLocalHostAddr = "localhost";
const char* gMongoSetOperator = "$set";
const char* gDbDataOpLogName = "test.dbDataOplog";
const char* gLocalOplogRs = "local.oplog.rs";

typedef struct
{
  const char* pName;
  const char* pValue;
  const char* pId;
} DbTestData;


DbTestData dbTestData[] =
{
  {
    "name_1",
    "value_1",
    "8224307453475422225",
  },
  {
    "name_2",
    "value_2",
    "8224307453475422226",
  },
  {
    "name_3",
    "value_3",
    "8224307453475422225",
  },
  {
    "name_4",
    "value_4",
    "8224307453475422226",
  },
  {
    "name_5",
    "value_5",
    "8224307453475422225",
  }
};

class DbData
{
public:
  DbData(){}

  DbData(const std::string& name,
        const std::string& value,
        const std::string& id) : _name(name), _value(value), _id(id)
  {
  }

  std::string& getName(){return _name;}
  std::string& getValue(){return _value;}
  std::string& getId(){return _id;}

  void setName(std::string name){_name = name;}
  void setValue(std::string value){_value = value;}
  void setId(std::string id){_id = id;}

  static const char* name_fld(){ static std::string fld = "name"; return fld.c_str(); }
  static const char* value_fld(){ static std::string fld = "value"; return fld.c_str(); }
  static const char* id_fld(){ static std::string fld = "_id"; return fld.c_str(); }

protected:

private:
  std::string _name;
  std::string _value;
  std::string _id;
};


class MongoOpLogTest: public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE(MongoOpLogTest);
  CPPUNIT_TEST(test_DataAddedBeforeStartingMongoOpLog);
  CPPUNIT_TEST(test_DataAddedAfterStartingMongoOpLog);
  CPPUNIT_TEST_SUITE_END();


  const MongoDB::ConnectionInfo _info;
  int _updateNr;
  int _insertNr;
  int _allNr;
  int _deleteNr;
  const std::string _databaseName;
public:
  MongoOpLogTest() : _info(MongoDB::ConnectionInfo(mongo::ConnectionString(mongo::HostAndPort(gLocalHostAddr)))),
                    _updateNr(0),
                    _insertNr(0),
                    _allNr(0),
                    _deleteNr(0),
                    _databaseName(gDbDataOpLogName)
  {
  }

  void setUp()
  {
    MongoDB::ScopedDbConnectionPtr pConn(mongoMod::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));
    pConn->get()->dropCollection(_databaseName);
    pConn->done();
  }

  void tearDown()
  {
  }

  void updateDbData(DbData& dbData)
  {

    mongo::BSONObj query = BSON(dbData.id_fld() << dbData.getId());

    mongo::BSONObjBuilder bsonObjBuilder;
    bsonObjBuilder <<
        dbData.name_fld() << dbData.getName() <<                                       // "ts"
        dbData.value_fld() << dbData.getValue();                                         // "h"

    mongo::BSONObj update;
    update = BSON(gMongoSetOperator << bsonObjBuilder.obj());

    MongoDB::ScopedDbConnectionPtr pConn(mongoMod::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));
    mongo::DBClientBase* client = pConn->get();

    client->update(_databaseName, query, update, true, false);
    client->ensureIndex(_databaseName, BSON(dbData.id_fld() << 1 ));

    pConn->done();
  }

  void deleteDbData()
  {
    MongoDB::ScopedDbConnectionPtr pConn(mongoMod::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));

    mongo::BSONObj queryBSONObj;
    pConn->get()->remove(_databaseName, queryBSONObj);
    pConn->done();
  }

  void OpLogCallBackInsert(const mongo::BSONObj& bSONObj)
  {
    std::cout << boost::format("insert=%s\n") % bSONObj.toString();
    _insertNr++;
  }

  void OpLogCallBackDelete(const mongo::BSONObj& bSONObj)
  {
    std::cout << boost::format("delete=%s\n") % bSONObj.toString();
    _deleteNr++;
  }

  void OpLogCallBackAll(const mongo::BSONObj& bSONObj)
  {
    std::cout << boost::format("all=%s\n") % bSONObj.toString();
    _allNr++;
  }

  void OpLogCallBackUpdate(const mongo::BSONObj& bSONObj)
  {
    std::cout << boost::format("update=%s\n") % bSONObj.toString();
    _updateNr++;
  }

  void test_DataAddedBeforeStartingMongoOpLog()
  {
    // Get startup time
    unsigned long currentTime = OsDateTime::getSecsSinceEpoch();

    DbData dbData(dbTestData[0].pName, dbTestData[0].pValue, dbTestData[0].pId);

    updateDbData(dbData);

    dbData.setName(dbTestData[1].pName);
    dbData.setId(dbTestData[1].pId);
    dbData.setValue(dbTestData[1].pValue);

    updateDbData(dbData);

    dbData.setName(dbTestData[2].pName);
    dbData.setId(dbTestData[2].pId);
    dbData.setValue(dbTestData[2].pValue);

    updateDbData(dbData);

    dbData.setName(dbTestData[3].pName);
    dbData.setId(dbTestData[3].pId);
    dbData.setValue(dbTestData[3].pValue);

    updateDbData(dbData);

    dbData.setName(dbTestData[4].pName);
    dbData.setId(dbTestData[4].pId);
    dbData.setValue(dbTestData[4].pValue);

    updateDbData(dbData);

    deleteDbData();

//    mongo::mutex::scoped_lock lk(mongo::OpTime::m);
//    unsigned long long timestamp = mongo::OpTime::now(lk).asDate();

    MongoOpLog mongoOpLog(_info, BSON("ns" << _databaseName), 0, currentTime);

    // register callback for insert
    mongoOpLog.registerCallback(MongoOpLog::Insert, boost::bind(&MongoOpLogTest::OpLogCallBackInsert, this, _1));

    // register callback for delete
    mongoOpLog.registerCallback(MongoOpLog::Delete, boost::bind(&MongoOpLogTest::OpLogCallBackDelete, this, _1));

    // register callback for all
    mongoOpLog.registerCallback(MongoOpLog::All, boost::bind(&MongoOpLogTest::OpLogCallBackAll, this, _1));

    // register callback for update
    mongoOpLog.registerCallback(MongoOpLog::Update, boost::bind(&MongoOpLogTest::OpLogCallBackUpdate, this, _1));

    mongoOpLog.run();

    sleep(1);

    // TEST: Check that insert callback was called at least 3 times
    CPPUNIT_ASSERT(_insertNr >= 2);

    // TEST: Check that delete callback was called at least 3 times
    CPPUNIT_ASSERT(_deleteNr >= 1);

    // TEST: Check that update callback was called at least 3 times
    CPPUNIT_ASSERT(_updateNr >= 1);

    // TEST: Check that all callback was called at least 3 times
    CPPUNIT_ASSERT(_allNr >= 5);
  }

  void test_DataAddedAfterStartingMongoOpLog()
  {
    // Get startup time
    unsigned long currentTime = OsDateTime::getSecsSinceEpoch();

    MongoOpLog mongoOpLog(_info, BSON("ns" << _databaseName), 0, currentTime);

    // register callback for insert
    mongoOpLog.registerCallback(MongoOpLog::Insert, boost::bind(&MongoOpLogTest::OpLogCallBackInsert, this, _1));

    // register callback for delete
    mongoOpLog.registerCallback(MongoOpLog::Delete, boost::bind(&MongoOpLogTest::OpLogCallBackDelete, this, _1));

    // register callback for all
    mongoOpLog.registerCallback(MongoOpLog::All, boost::bind(&MongoOpLogTest::OpLogCallBackAll, this, _1));

    // register callback for update
    mongoOpLog.registerCallback(MongoOpLog::Update, boost::bind(&MongoOpLogTest::OpLogCallBackUpdate, this, _1));

    mongoOpLog.run();

    sleep(1);

    DbData dbData(dbTestData[0].pName, dbTestData[0].pValue, dbTestData[0].pId);

    updateDbData(dbData);

    dbData.setName(dbTestData[1].pName);
    dbData.setId(dbTestData[1].pId);
    dbData.setValue(dbTestData[1].pValue);

    updateDbData(dbData);

    dbData.setName(dbTestData[2].pName);
    dbData.setId(dbTestData[2].pId);
    dbData.setValue(dbTestData[2].pValue);

    updateDbData(dbData);

    dbData.setName(dbTestData[3].pName);
    dbData.setId(dbTestData[3].pId);
    dbData.setValue(dbTestData[3].pValue);

    updateDbData(dbData);

    dbData.setName(dbTestData[4].pName);
    dbData.setId(dbTestData[4].pId);
    dbData.setValue(dbTestData[4].pValue);

    updateDbData(dbData);

    deleteDbData();


    sleep(1);

    // TEST: Check that insert callback was called at least 3 times
    CPPUNIT_ASSERT(_insertNr >= 2);

    // TEST: Check that delete callback was called at least 3 times
    CPPUNIT_ASSERT(_deleteNr >= 1);

    // TEST: Check that update callback was called at least 3 times
    CPPUNIT_ASSERT(_updateNr >= 1);

    // TEST: Check that all callback was called at least 3 times
    CPPUNIT_ASSERT(_allNr >= 5);
  }


};

CPPUNIT_TEST_SUITE_REGISTRATION(MongoOpLogTest);



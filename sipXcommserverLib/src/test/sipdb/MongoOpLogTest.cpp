#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <sipxunit/TestUtilities.h>
#include <sipdb/MongoOpLog.h>
//#include <os/OsDateTime.h>
#include <mongo/util/net/hostandport.h>
#include <mongo/client/connpool.h>

#include <boost/format.hpp>


#include <boost/function.hpp>


using namespace std;

const char* gLocalHostAddr = "localhost";
const char* gMongoSetOperator = "$set";
const char* gLocalOplogDbName = "local.oplog";

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
    "1",
    "8224307453475422225",
    "1",
    "i",
    "local.oplog",
    "name"
  },
  {
    "TS time:Wed Aug 07 11:45:34 EEST 2013 inc:1",
    "2",
    "8224307453475422225",
    "1",
    "d",
    "local.oplog",
    "name"
  },
  {
    "TS time:Wed Aug 07 11:45:34 EEST 2013 inc:1",
    "5",
    "8224307453475422225",
    "1",
    "i",
    "local.oplog",
    "name"
  },
  {
    "TS time:Wed Aug 07 11:45:34 EEST 2013 inc:1",
    "6",
    "8224307453475422225",
    "1",
    "i",
    "local.oplog",
    "name"
  },
  {
    "TS time:Wed Aug 07 11:45:34 EEST 2013 inc:1",
    "7",
    "8224307453475422225",
    "1",
    "u",
    "local.oplog",
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


class MongoOpLogTest: public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE(MongoOpLogTest);
  CPPUNIT_TEST(testMongoOpLog_Run);
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
                    _databaseName(gLocalOplogDbName)
  {
  }

  void setUp()
  {
    MongoDB::ScopedDbConnectionPtr pConn(mongo::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));
    pConn->get()->dropCollection(_databaseName);
    pConn->done();
  }

  void tearDown()
  {
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

    MongoDB::ScopedDbConnectionPtr conn(mongo::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString()));
    mongo::DBClientBase* client = conn->get();

    // create a capped collect in order to work with tailer cursor
    client->createCollection(_databaseName, 1024*1024, true, 0, 0);
    client->update(_databaseName, query, update, true, false);
    client->ensureIndex(_databaseName, BSON(opLog.id_fld() << 1 ));

    conn->done();
  }

  void OpLogCallBackInsert(const std::string& opLog)
  {
    //std::cout << boost::format("insert=%s\n") % opLog;
    _insertNr++;
  }

  void OpLogCallBackDelete(const std::string& opLog)
  {
   // std::cout << boost::format("delete=%s\n") % opLog;
    _deleteNr++;
  }

  void OpLogCallBackAll(const std::string& opLog)
  {
    //std::cout << boost::format("all=%s\n") % opLog;
    _allNr++;
  }

  void OpLogCallBackUpdate(const std::string& opLog)
  {
    //std::cout << boost::format("update=%s\n") % opLog;
    _updateNr++;
  }


  void testMongoOpLog_Run()
  {
    OpLog opLog(opLogTestData[0].pTs, opLogTestData[0].pId, opLogTestData[0].pH,
                opLogTestData[0].pV, opLogTestData[0].pOperation, opLogTestData[0].pNs, opLogTestData[0].pO);

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

    opLog.setOperation(opLogTestData[4].pOperation);
    opLog.setId(opLogTestData[4].pId);
    opLog.setNs(opLogTestData[4].pNs);

    updateMongoOpLog(opLog);

    MongoOpLog mongoOpLog(_info, _databaseName);

    // register callback for insert
    mongoOpLog.registerCallback(MongoOpLog::Insert, boost::bind(&MongoOpLogTest::OpLogCallBackInsert, this, _1));

    // register callback for delete
    mongoOpLog.registerCallback(MongoOpLog::Delete, boost::bind(&MongoOpLogTest::OpLogCallBackDelete, this, _1));

    // register callback for all
    mongoOpLog.registerCallback(MongoOpLog::All, boost::bind(&MongoOpLogTest::OpLogCallBackAll, this, _1));

    // register callback for update
    mongoOpLog.registerCallback(MongoOpLog::Update, boost::bind(&MongoOpLogTest::OpLogCallBackUpdate, this, _1));

    mongoOpLog.run();

    sleep(3);

    // TEST: Check that insert callback was called at least 3 times
    CPPUNIT_ASSERT(_insertNr >= 3);

    // TEST: Check that delete callback was called at least 3 times
    CPPUNIT_ASSERT(_deleteNr >= 1);

    // TEST: Check that update callback was called at least 3 times
    CPPUNIT_ASSERT(_updateNr >= 1);

    // TEST: Check that all callback was called at least 3 times
    CPPUNIT_ASSERT(_allNr >= 5);
  }


};

CPPUNIT_TEST_SUITE_REGISTRATION(MongoOpLogTest);



#include <string>
#include <iostream>
#include <fstream>
//#include <boost/program_options.hpp>
#include <boost/config.hpp>
//#include <boost/scoped_ptr.hpp>
#include <boost/program_options/detail/config_file.hpp>
#include <boost/program_options/parsers.hpp>
//#include <mongo/client/dbclient.h>
#include <mongo/client/connpool.h>
#include "sipdb/MongoDB.h"
#include <os/OsLogger.h>
#include <os/OsDateTime.h>

const int READ_TIMER_SAMPLES = 5; // Number of samples for getting the read delay
const int UPDATE_TIMER_SAMPLES = 5; // Number of samples for getting the update delay
const int MAX_READ_DELAY_MS = 100; // Maximum allowable delay for reads in milliseconds
const int MAX_UPDATE_DELAY_MS = 500; // Maximum allowable delay for updates in milliseconds
const int ALARM_RATE_SEC = 300; // Send alarm ever nth seconds when threshold is violated
  
using namespace std;

namespace pod = boost::program_options::detail;

namespace MongoDB
{
  

  BaseDB::BaseDB(const ConnectionInfo& info, const std::string& ns) :
    _ns(ns),
    _info(info),
    _updateTimerSamples(UPDATE_TIMER_SAMPLES),
    _readTimerSamples(READ_TIMER_SAMPLES),
    _lastReadSpeed(0),
    _lastUpdateSpeed(0),
    _lastAlarmLog(0)
  {  
  }

  bool ConnectionInfo::testConnection(const mongo::ConnectionString &connectionString, string& errmsg)
  {
      bool ret = false;

      try
      {
          MongoDB::ScopedDbConnectionPtr conn(mongoMod::ScopedDbConnection::getScopedDbConnection(connectionString.toString(), 5));
          ret = conn->ok();
          conn->done();
      }
      catch( mongo::DBException& e )
      {
          ret = false;
          errmsg = e.what();
      }

      return ret;
  }

  const ConnectionInfo ConnectionInfo::globalInfo()
  {
    const char* fname = SIPX_CONFDIR "/mongo-client.ini";
    ifstream file(fname);
      if (!file)
      {
          BOOST_THROW_EXCEPTION(ConfigError() <<  errmsg_info(std::string("Missing file ")  + fname));
      }
      return ConnectionInfo(file);
  }

  const ConnectionInfo ConnectionInfo::localInfo()
  {
    ifstream file(SIPX_CONFDIR "/mongo-local.ini");
      if (!file)
      {
        return ConnectionInfo();
      }
      return ConnectionInfo(file);
  }

  ConnectionInfo::ConnectionInfo(ifstream& file) : _shard(0), _useReadTags(false)
  {
    set<string> options;
      options.insert("*");
      string connectionString;
      for (boost::program_options::detail::config_file_iterator i(file, options), e; i != e; ++i) {
          if (i->string_key == "connectionString") {
            connectionString = i->value[0];
          }
          if (i->string_key == "shardId") {
            _shard = atoi(i->value[0].c_str());
          }
          if (i->string_key == "useReadTags") {
        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, i->value[0].c_str());
      if (strncmp(i->value[0].c_str(), "true", 4) == 0) {
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "useReadTags enabled");
        _useReadTags = true;
      }
          }
      }
      file.close();
      if (connectionString.size() == 0)
      {
          BOOST_THROW_EXCEPTION(ConfigError() << errmsg_info(std::string("Invalid contents, missing parameter 'connectionString' in file ")));
      }

    string errmsg;
    _connectionString = mongo::ConnectionString::parse(connectionString, errmsg);
    if (!_connectionString.isValid()) {
        BOOST_THROW_EXCEPTION(ConfigError() << errmsg_info(errmsg));
    }
      Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "loaded db connection info for %s", connectionString.c_str());
  }

  void  BaseDB::setReadPreference(mongo::BSONObjBuilder& builder, mongo::BSONObj query, const char* readPreferrence) const {
    if (_info.useReadTags()) {
      Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "Using read preferences tags for ");
      std::string shardIdStr = boost::to_string(getShardId());
      mongo::BSONArray tags = BSON_ARRAY(BSON("shardId" << shardIdStr) << BSON("clusterId" << "1"));
      builder.append("$readPreference", BSON("mode" << readPreferrence << "tags" << tags));
    } else {
      builder.append("$readPreference", BSON("mode" << readPreferrence));
    }
    builder.append("query", query);
  }

  void  BaseDB::nearest(mongo::BSONObjBuilder& builder, mongo::BSONObj query) const
  {
    setReadPreference(builder, query, "nearest");
  }

  void  BaseDB::primaryPreferred(mongo::BSONObjBuilder& builder, mongo::BSONObj query) const
  {
    setReadPreference(builder, query, "primaryPreferred");
  }


  void BaseDB::forEach(mongo::BSONObj& query, const std::string& ns, boost::function<void(mongo::BSONObj)> doSomething)
  {
      MongoDB::ScopedDbConnectionPtr conn(mongoMod::ScopedDbConnection::getScopedDbConnection(_info.getConnectionString().toString(), 5));
      auto_ptr<mongo::DBClientCursor> pCursor = conn->get()->query(ns, query, 0, 0, 0, mongo::QueryOption_SlaveOk);
      if (pCursor.get() && pCursor->more())
      {
          while (pCursor->more())
          {
              doSomething(pCursor->next());
          }
      }

      conn->done();
  }

  void BaseDB::registerTimer(const UpdateTimer* pTimer)
  {
    boost::lock_guard<boost::mutex> lock(_updateTimerSamplesMutex);

    _lastUpdateSpeed = pTimer->_end - pTimer->_start;
    _updateTimerSamples.push_back(_lastReadSpeed);
    
    if (_lastUpdateSpeed > MAX_UPDATE_DELAY_MS)
    {
      OsTime time;
      OsDateTime::getCurTimeSinceBoot(time);
      long now = time.seconds();

      if (!_lastAlarmLog || now >= _lastAlarmLog + ALARM_RATE_SEC)
      {
        _lastAlarmLog = now;
        OS_LOG_EMERGENCY(FAC_SIP, "ALARM_MONGODB_SLOW_UPDATE Last Mongo update took a long time:" 
            << " document: " << _ns
            << " delay: " << _lastUpdateSpeed << " milliseconds");
      }
    }
  }

  void BaseDB::registerTimer(const ReadTimer* pTimer)
  {
    boost::lock_guard<boost::mutex> lock(_readTimerSamplesMutex);
    

    _lastReadSpeed = pTimer->_end - pTimer->_start;
    _readTimerSamples.push_back(_lastReadSpeed);
    
    if (_lastReadSpeed > MAX_READ_DELAY_MS)
    {
      OsTime time;
      OsDateTime::getCurTimeSinceBoot(time);
      long now = time.seconds();

      if (!_lastAlarmLog || now >= _lastAlarmLog + ALARM_RATE_SEC)
      {
        _lastAlarmLog = now;
        OS_LOG_EMERGENCY(FAC_SIP, "ALARM_MONGODB_SLOW_READ Last Mongo update took a long time:" 
            << " document: " << _ns
            << " delay: " << _lastReadSpeed << " milliseconds");
      }
    }
  }
  
  Int64 BaseDB::getUpdateAverageSpeed() const
  {
    boost::lock_guard<boost::mutex> lock(_updateTimerSamplesMutex);
    
    Int64 sum = 0;
    for (boost::circular_buffer<Int64>::const_iterator iter = _updateTimerSamples.begin(); iter != _updateTimerSamples.end(); iter++)
    {
      sum += *iter;
    }
    
    if (_updateTimerSamples.empty())
      return 0;
    
    return sum / _updateTimerSamples.size();
  }
  
  Int64 BaseDB::getLastUpdateSpeed() const
  {
    boost::lock_guard<boost::mutex> lock(_updateTimerSamplesMutex);
    return _lastUpdateSpeed;
  }
  
  Int64 BaseDB::getReadAverageSpeed() const
  {
    boost::lock_guard<boost::mutex> lock(_readTimerSamplesMutex);
    
    Int64 sum = 0;
    for (boost::circular_buffer<Int64>::const_iterator iter = _readTimerSamples.begin(); iter != _readTimerSamples.end(); iter++)
    {
      sum += *iter;
    }
    
    if (_readTimerSamples.empty())
      return 0;
    
    return sum / _readTimerSamples.size();
  }
  
  Int64 BaseDB::getLastReadSpeed() const
  {
    boost::lock_guard<boost::mutex> lock(_readTimerSamplesMutex);
    return _lastReadSpeed;
  }

  UpdateTimer::UpdateTimer(BaseDB& db) :
    _db(db)
  {
    struct timeval sTimeVal;
    gettimeofday( &sTimeVal, NULL );
    _start = (Int64)( sTimeVal.tv_sec * 1000 + ( sTimeVal.tv_usec / 1000 ) );
  }
  UpdateTimer::~UpdateTimer()
  {
    struct timeval sTimeVal;
    gettimeofday( &sTimeVal, NULL );
    _end = (Int64)( sTimeVal.tv_sec * 1000 + ( sTimeVal.tv_usec / 1000 ) );
    _db.registerTimer(this);
  }

  ReadTimer::ReadTimer(BaseDB& db) :
    _db(db)
  {
    struct timeval sTimeVal;
    gettimeofday( &sTimeVal, NULL );
    _start = (Int64)( sTimeVal.tv_sec * 1000 + ( sTimeVal.tv_usec / 1000 ) );
  }

  ReadTimer::~ReadTimer()
  {
    struct timeval sTimeVal;
    gettimeofday( &sTimeVal, NULL );
    _end = (Int64)( sTimeVal.tv_sec * 1000 + ( sTimeVal.tv_usec / 1000 ) );
    _db.registerTimer(this);
  }
  
} // namespace MongoDB



#ifndef _CUSTOM_CONN_POOL_MOD_H__
#define _CUSTOM_CONN_POOL_MOD_H__

#include <os/OsLoggerHelper.h>
#include <mongo/util/assert_util.h>
#include "mongo/util/background.h"
#include "mongo/client/dbclientinterface.h"

#include <mongo/client/dbclient.h>
#include <mongo/client/dbclient_rs.h>
#include <mongo/client/connpool.h>


using namespace std;

namespace mongoMod
{
class DBConnectionPool;

/**
 * @note This is a copy of PoolForHost class which was modified to not remove
 * from pool failed DBConnectionReplicaSet instances. For other connection
 * types the behavior remains unchanged.
 *
 * not thread safe
 * thread safety is handled by DBConnectionPool
 */
class PoolForHost {
public:
  PoolForHost()
      : _created(0), _minValidCreationTimeMicroSec(0) {}

  PoolForHost( const PoolForHost& other )
  {
    OS_LOG_AND_ASSERT((other._pool.size() == 0), FAC_DB, "cannot copy pool with size > 0");
    _created = other._created;
    _minValidCreationTimeMicroSec = other._minValidCreationTimeMicroSec;
    OS_LOG_AND_ASSERT((_created == 0), FAC_DB, "cannot copy used pool ");
  }

  ~PoolForHost();

  int numAvailable() const { return (int)_pool.size(); }

  void createdOne(mongo::DBClientBase * base );
  long long numCreated() const { return _created; }

  mongo::ConnectionString::ConnectionType type() const { OS_LOG_AND_ASSERT((_created > 0), FAC_DB, "_type not valid for pool with _created 0"); return _type; }

  /**
   * gets a connection or return NULL
   */
  mongo::DBClientBase * get(mongoMod::DBConnectionPool * pool , double socketTimeout );

  // Deletes all connections in the pool
  void clear();

  void done(mongoMod::DBConnectionPool * pool , mongo::DBClientBase * c );

  void flush();

  void getStaleConnections( vector<mongo::DBClientBase*>& stale );

  /**
   * Sets the lower bound for creation times that can be considered as
   *     good connections.
   */
  void reportBadConnectionAt(uint64_t microSec);

  /**
   * @return true if the given creation time is considered to be not
   *     good for use.
   */
  bool isBadSocketCreationTime(uint64_t microSec);

  /**
   * Sets the host name to a new one, only if it is currently empty.
   */
  void initializeHostName(const std::string& hostName);

  static void setMaxPerHost( unsigned max ) { _maxPerHost = max; }
  static unsigned getMaxPerHost() { return _maxPerHost; }

private:

  struct StoredConnection
  {
    StoredConnection(mongo::DBClientBase * c );

    bool ok( time_t now );

    mongo::DBClientBase* conn;
    time_t when;
  };

  std::string _hostName;
  std::stack<StoredConnection> _pool;

  int64_t _created;
  uint64_t _minValidCreationTimeMicroSec;
  mongo::ConnectionString::ConnectionType _type;

  static unsigned _maxPerHost;
};

class DBConnectionPool : public mongo::PeriodicTask
{
public:
  DBConnectionPool();
  ~DBConnectionPool();

  void flush();

  mongo::DBClientBase *get(const string& host, double socketTimeout = 0);
  mongo::DBClientBase *get(const mongo::ConnectionString& host, double socketTimeout = 0);

  void release(const string& host, mongo::DBClientBase *c);

  /**
   * Clears all connections for all host.
   */
  void clear();

  /**
   * Checks whether the connection for a given host is black listed or not.
   *
   * @param hostName the name of the host the connection connects to.
   * @param conn the connection to check.
   *
   * @return true if the connection is not bad, meaning, it is good to keep it for
   *     future use.
   */
  bool isConnectionGood(const string& host, mongo::DBClientBase* conn);

  // Removes and deletes all connections from the pool for the host (regardless of timeout)
  void removeHost( const string& host );

  /** compares server namees, but is smart about replica set names */
  struct serverNameCompare {
      bool operator()( const string& a , const string& b ) const;
  };

  virtual string taskName() const { return "DBConnectionPool-cleaner"; }
  virtual void taskDoWork();

private:
  mongo::DBClientBase* _get( const string& ident , double socketTimeout );

  mongo::DBClientBase* _finishCreate( const string& ident , double socketTimeout, mongo::DBClientBase* conn );

  DBConnectionPool( DBConnectionPool& p );

  struct PoolKey {
      PoolKey( const std::string& i , double t ) : ident( i ) , timeout( t ) {}
      string ident;
      double timeout;
  };

  struct poolKeyCompare {
      bool operator()( const PoolKey& a , const PoolKey& b ) const;
  };

  typedef map<PoolKey,PoolForHost,poolKeyCompare> PoolMap; // servername -> pool

  mongo::mutex _mutex;
  string _name;

  PoolMap _pools;
};

extern DBConnectionPool pool;

class ScopedDbConnection : public mongo::AScopedConnection {
private:
    /** the main constructor you want to use
        throws UserException if can't connect
        */
  explicit ScopedDbConnection(const string& host, double socketTimeout = 0) : _host(host), _conn(pool.get(host, socketTimeout)), _socketTimeout( socketTimeout )
  {
    _setSocketTimeout();
  }

  explicit ScopedDbConnection(const mongo::ConnectionString& host, double socketTimeout = 0) : _host(host.toString()), _conn(pool.get(host, socketTimeout)), _socketTimeout( socketTimeout )
  {
    _setSocketTimeout();
  }

  ScopedDbConnection() : _host( "" ) , _conn(0), _socketTimeout( 0 ) {}

public:
  // Factory functions for getting ScopedDbConnections.  The caller owns the resulting object
  // and is responsible for deleting it when finished. This should be used when running a
  // command on a shard from the mongos and the command should run with the client's
  // authentication.  If the command should be run with full permissions regardless
  // of whether or not the user is authorized, then use getInternalScopedDbConnection().
  static ScopedDbConnection* getScopedDbConnection(const string& host,
                                                   double socketTimeout = 0);
  static ScopedDbConnection* getScopedDbConnection(const mongo::ConnectionString& host,
                                                   double socketTimeout = 0);
  static ScopedDbConnection* getScopedDbConnection();

  static void clearPool();

  ~ScopedDbConnection();

  /** get the associated connection object */
  mongo::DBClientBase* operator->()
  {
    OS_LOG_AND_ASSERT((_conn != NULL), FAC_DB, "connection was returned to the pool already");
    return _conn;
  }

  /** get the associated connection object */
  mongo::DBClientBase& conn()
  {
    OS_LOG_AND_ASSERT((_conn != NULL), FAC_DB, "connection was returned to the pool already");
    return *_conn;
  }

  /** get the associated connection object */
  mongo::DBClientBase* get()
  {
    OS_LOG_AND_ASSERT((_conn != NULL), FAC_DB, "connection was returned to the pool already");
    return _conn;
  }

  bool ok() const { return _conn > 0; }

  string getHost() const { return _host; }

  /** Force closure of the connection.  You should call this if you leave it in
      a bad state.  Destructor will do this too, but it is verbose.
  */
  void kill()
  {
    delete _conn;
    _conn = 0;
  }

  /** Call this when you are done with the connection.

      If you do not call done() before this object goes out of scope,
      we can't be sure we fully read all expected data of a reply on the socket.  so
      we don't try to reuse the connection in that situation.
  */
  void done()
  {
    if ( ! _conn )
        return;

    /* we could do this, but instead of assume one is using autoreconnect mode on the connection
    if ( _conn->isFailed() )
        kill();
    else
    */
    pool.release(_host, _conn);
    _conn = 0;
  }

private:

    void _setSocketTimeout();

    const string _host;
    mongo::DBClientBase *_conn;
    const double _socketTimeout;
};

} // namespace mongoMod
#endif /* _CUSTOM_CONN_POOL_MOD_H__ */

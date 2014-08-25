#if 0
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <iostream>

#include "sipdb/CustomConnPool.h"
#include <mongo/client/dbclient_rs.h>
#include <mongo/client/dbclient.h>

using namespace std;

namespace mongoMod
{

  // ------ PoolForHostMod ------

  PoolForHost::~PoolForHost()
  {
    clear();
  }

  void PoolForHost::clear()
  {
    while ( ! _pool.empty() )
    {
      StoredConnection sc = _pool.top();
      delete sc.conn;
      _pool.pop();
    }
  }

  void PoolForHost::done(mongoMod::DBConnectionPool * pool, mongo::DBClientBase * c )
  {
    if (c->isFailed())
    {
      if (NULL != dynamic_cast<mongo::DBClientReplicaSet*>(c))
      {
        _pool.push(c);
      }
      else
      {
        reportBadConnectionAt(c->getSockCreationMicroSec());
        delete c;
      }
    }
    else if (_pool.size() >= _maxPerHost ||
            c->getSockCreationMicroSec() < _minValidCreationTimeMicroSec)
    {
      delete c;
    }
    else
    {
      _pool.push(c);
    }
  }

  void PoolForHost::reportBadConnectionAt(uint64_t microSec)
  {
      if (microSec != mongo::DBClientBase::INVALID_SOCK_CREATION_TIME &&
              microSec > _minValidCreationTimeMicroSec) {
          _minValidCreationTimeMicroSec = microSec;
          mongo::log() << "Detected bad connection created at " << _minValidCreationTimeMicroSec
                  << " microSec, clearing pool for " << _hostName << endl;
          clear();
      }
  }

  bool PoolForHost::isBadSocketCreationTime(uint64_t microSec)
  {
      return microSec != mongo::DBClientBase::INVALID_SOCK_CREATION_TIME &&
              microSec <= _minValidCreationTimeMicroSec;
  }

  mongo::DBClientBase * PoolForHost::get(mongoMod::DBConnectionPool * pool , double socketTimeout )
  {
      time_t now = time(0);

      while ( ! _pool.empty() ) {
        StoredConnection sc = _pool.top();
          _pool.pop();

          if ( ! sc.ok( now ) )  {
              delete sc.conn;
              continue;
          }

          return sc.conn;

      }

      return NULL;
  }

  void PoolForHost::flush()
  {
      vector<StoredConnection> all;
      while ( ! _pool.empty() ) {
          StoredConnection c = _pool.top();
          _pool.pop();
          bool res;
          bool alive = false;
          try {
              c.conn->isMaster( res );
              alive = true;
          } catch ( const mongo::DBException e ) {
              // There's something wrong with this connection, swallow the exception and do not
              // put the connection back in the pool.
              LOG(1) << "Exception thrown when checking pooled connection to " <<
                  c.conn->getServerAddress() << ": " << causedBy(e) << endl;
              delete c.conn;
              c.conn = NULL;
          }
          if ( alive ) {
              all.push_back( c );
          }
      }

      for ( vector<StoredConnection>::iterator i=all.begin(); i != all.end(); ++i ) {
          _pool.push( *i );
      }
  }

  void PoolForHost::getStaleConnections( vector<mongo::DBClientBase*>& stale )
  {
      time_t now = time(0);

      vector<StoredConnection> all;
      while ( ! _pool.empty() ) {
          StoredConnection c = _pool.top();
          _pool.pop();

          if ( c.ok( now ) )
              all.push_back( c );
          else
              stale.push_back( c.conn );
      }

      for ( size_t i=0; i<all.size(); i++ ) {
          _pool.push( all[i] );
      }
  }


  PoolForHost::StoredConnection::StoredConnection(mongo::DBClientBase * c ) {
      conn = c;
      when = time(0);
  }

  bool PoolForHost::StoredConnection::ok( time_t now ) {
      // if connection has been idle for 30 minutes, kill it
      return ( now - when ) < 1800;
  }

  void PoolForHost::createdOne(mongo::DBClientBase * base) {
      if ( _created == 0 )
          _type = base->type();
      _created++;
  }

  void PoolForHost::initializeHostName(const std::string& hostName) {
      if (_hostName.empty()) {
          _hostName = hostName;
      }
  }

  unsigned PoolForHost::_maxPerHost = 50;

  // ------ DBConnectionPool ------
  DBConnectionPool::DBConnectionPool()
      : _mutex("DBConnectionPool"),
        _name( "DBConnectionPool" )
  {
  }


  DBConnectionPool::~DBConnectionPool() {
      // connection closing is handled by ~PoolForHost
  }

  mongo::DBClientBase* DBConnectionPool::_get(const string& ident , double socketTimeout )
  {
    OS_LOG_AND_ASSERT((!mongo::inShutdown()), FAC_DB, "mongo is in shutdown");
    mongo::scoped_lock L(_mutex);
    PoolForHost& p = _pools[PoolKey(ident,socketTimeout)];
    p.initializeHostName(ident);
    return p.get( this , socketTimeout );
  }

  mongo::DBClientBase* DBConnectionPool::_finishCreate( const string& host , double socketTimeout , mongo::DBClientBase* conn )
  {
    mongo::scoped_lock L(_mutex);
    PoolForHost& p = _pools[PoolKey(host,socketTimeout)];
    p.initializeHostName(host);
    p.createdOne( conn );

    return conn;
  }

  mongo::DBClientBase* DBConnectionPool::get(const mongo::ConnectionString& url, double socketTimeout)
  {
      mongo::DBClientBase * c = _get( url.toString() , socketTimeout );
      if ( c ) {
          return c;
      }

      string errmsg;
      c = url.connect( errmsg, socketTimeout );
//      uassert( 13328 ,  _name + ": connect failed " + url.toString() + " : " + errmsg , c );

      return _finishCreate( url.toString() , socketTimeout , c );
  }

  mongo::DBClientBase* DBConnectionPool::get(const string& host, double socketTimeout)
  {
      mongo::DBClientBase * c = _get( host , socketTimeout );
      if ( c ) {
          return c;
      }

      string errmsg;
      mongo::ConnectionString cs = mongo::ConnectionString::parse( host , errmsg );
//      uassert( 13071 , (string)"invalid hostname [" + host + "]" + errmsg , cs.isValid() );

      c = cs.connect( errmsg, socketTimeout );
      if ( ! c )
          throw mongo::SocketException( mongo::SocketException::CONNECT_ERROR , host , 11002 , mongo::str::stream() << _name << " error: " << errmsg );
      return _finishCreate( host , socketTimeout , c );
  }

  void DBConnectionPool::release(const string& host, mongo::DBClientBase *c)
  {
    mongo::scoped_lock L(_mutex);
    _pools[PoolKey(host,c->getSoTimeout())].done(this,c);
  }

  void DBConnectionPool::flush()
  {
    mongo::scoped_lock L(_mutex);
    for ( PoolMap::iterator i = _pools.begin(); i != _pools.end(); i++ ) {
        PoolForHost& p = i->second;
        p.flush();
    }
  }

  void DBConnectionPool::clear()
  {
    mongo::scoped_lock L(_mutex);
    for (PoolMap::iterator iter = _pools.begin(); iter != _pools.end(); ++iter) {
        iter->second.clear();
    }
  }

  void DBConnectionPool::removeHost( const string& host )
  {
    mongo::scoped_lock L(_mutex);
    //LOG(2) << "Removing connections from all pools for host: " << host << endl;
    for ( PoolMap::iterator i = _pools.begin(); i != _pools.end(); ++i ) {
        const string& poolHost = i->first.ident;
        if ( !serverNameCompare()(host, poolHost) && !serverNameCompare()(poolHost, host) ) {
            // hosts are the same
            i->second.clear();
        }
    }
  }


  bool DBConnectionPool::serverNameCompare::operator()( const string& a , const string& b ) const{
      const char* ap = a.c_str();
      const char* bp = b.c_str();

      while (true){
          if (*ap == '\0' || *ap == '/'){
              if (*bp == '\0' || *bp == '/')
                  return false; // equal strings
              else
                  return true; // a is shorter
          }

          if (*bp == '\0' || *bp == '/')
              return false; // b is shorter

          if ( *ap < *bp)
              return true;
          else if (*ap > *bp)
              return false;

          ++ap;
          ++bp;
      }
      verify(false);
  }

  bool DBConnectionPool::poolKeyCompare::operator()( const PoolKey& a , const PoolKey& b ) const {
      if (DBConnectionPool::serverNameCompare()( a.ident , b.ident ))
          return true;

      if (DBConnectionPool::serverNameCompare()( b.ident , a.ident ))
          return false;

      return a.timeout < b.timeout;
  }

  bool DBConnectionPool::isConnectionGood(const string& hostName, mongo::DBClientBase* conn)
  {
    if (conn == NULL) {
      return false;
    }

    if (conn->isFailed() && (NULL == dynamic_cast<mongo::DBClientReplicaSet*>(conn))) {
      return false;
    }

    {
      mongo::scoped_lock sl(_mutex);
      PoolForHost& pool = _pools[PoolKey(hostName, conn->getSoTimeout())];
      if (pool.isBadSocketCreationTime(conn->getSockCreationMicroSec())) 
      {
        return false;
      }
    }

    return true;
  }

  void DBConnectionPool::taskDoWork()
  {
      vector<mongo::DBClientBase*> toDelete;

      {
        // we need to get the connections inside the lock
        // but we can actually delete them outside
        mongo::scoped_lock lk(_mutex);
        for ( PoolMap::iterator i=_pools.begin(); i!=_pools.end(); ++i ) {
            i->second.getStaleConnections( toDelete );
        }
      }

      for ( size_t i=0; i<toDelete.size(); i++ ) {
          try {
              delete toDelete[i];
          }
          catch ( ... ) {
              // we don't care if there was a socket error
          }
      }
  }

      // ------ ScopedDbConnection ------

  DBConnectionPool pool;

  ScopedDbConnection* ScopedDbConnection::getScopedDbConnection() {
      return new ScopedDbConnection();
  }

  ScopedDbConnection* ScopedDbConnection::getScopedDbConnection(const string& host,
                                                                double socketTimeout) {
      return new ScopedDbConnection(host, socketTimeout);
  }

  ScopedDbConnection* ScopedDbConnection::getScopedDbConnection(const mongo::ConnectionString& host,
                                                                double socketTimeout) {
      return new ScopedDbConnection(host, socketTimeout);
  }

  void ScopedDbConnection::_setSocketTimeout(){
      if( ! _conn ) return;
      if( _conn->type() == mongo::ConnectionString::MASTER )
          (( mongo::DBClientConnection* ) _conn)->setSoTimeout( _socketTimeout );
      else if( _conn->type() == mongo::ConnectionString::SYNC )
          (( mongo::SyncClusterConnection* ) _conn)->setAllSoTimeouts( _socketTimeout );
  }

  ScopedDbConnection::~ScopedDbConnection() {
      if ( _conn ) {
          if (_conn->isFailed()) {
              if (_conn->getSockCreationMicroSec() ==
                      mongo::DBClientBase::INVALID_SOCK_CREATION_TIME) {
                  kill();
              }
              else {
                  // The pool takes care of deleting the failed connection - this
                  // will also trigger disposal of older connections in the pool
                  done();
              }
          }
          else {
              /* see done() comments above for why we log this line */
              mongo::log() << "scoped connection to " << _conn->getServerAddress()
                      << " not being returned to the pool" << endl;
              kill();
          }
      }
  }

  void ScopedDbConnection::clearPool() {
      pool.clear();
  }

  //    ScopedDbConnection* sc = ScopedDbConnection::getScopedDbConnection(host, 0);
  //    mongo::DBClientBase *conn = sc->get();

} //namespace mongoMod
#endif // if 0

/* 
 * File:   MongoDB.h
 * Author: joegen
 *
 * Created on January 10, 2011, 6:24 PM
 */

#ifndef MONGODB_H
#define	MONGODB_H

#include <queue>
#include <mongo/client/dbclient.h>
#include <mongo/db/jsobj.h>
#include <boost/thread/mutex.hpp>
#include <boost/noncopyable.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/functional/hash.hpp>


#define BSON_LESS_THAN(val) BSON("$lt"<< val)
#define BSON_LESS_THAN_EQUAL(val) BSON("$lte"<< val)
#define BSON_GREATER_THAN(val) BSON("$gt" << val)
#define BSON_GREATER_THAN_EQUAL(val) BSON("$gte" << val)

class MongoDB : boost::noncopyable
{
public:
    typedef boost::shared_ptr<MongoDB> Ptr;
    typedef boost::shared_ptr<mongo::DBClientConnection> Client;
    typedef std::auto_ptr<mongo::DBClientCursor> Cursor;
    typedef mongo::BSONObj BSONObj;
    typedef mongo::BSONElement BSONElement;
    typedef mongo::BSONObjBuilder BSONObjBuilder;
    typedef mongo::BSONArray BSONArray;
    typedef mongo::Query Query;
    typedef boost::recursive_mutex Mutex;
    typedef boost::lock_guard<Mutex> mutex_lock;

    class PooledConnection
    {
    public:
        PooledConnection(MongoDB& db) : _db(db){_client = _db.acquire();}
        ~PooledConnection(){ _db.relinquish(_client); }
        MongoDB::Client& operator->(){return _client;}
        bool operator!(){return _client.get() == 0;}
        MongoDB::Client& target(){return _client;}
    private:
        MongoDB& _db;
        MongoDB::Client _client;
    };

    template  <class T>
    class Collection
    {
    public:
        Collection(const std::string& ns, const std::string& server = "localhost") :
            _pDb(MongoDB::acquireServer(server)),
            _collection(*_pDb, ns){}
        T& collection(){ return _collection; }
        MongoDB& db(){ return *_pDb; }
    private:
        MongoDB::Ptr _pDb;
        T _collection;
    };

    class DBInterface : boost::noncopyable
    {
    public:
        DBInterface(MongoDB& db, const std::string& ns) : _db(db), _ns(ns){}
        const std::string& getNameSpace() const { return _ns; };
        MongoDB& db() { return _db; }
    protected:
        MongoDB& _db;
        std::string _ns;
    };

    MongoDB();

    MongoDB(const std::string& server);

    ~MongoDB();

    Query createQuery(const std::string& json) const;
    
    Cursor find(
        const std::string& ns,
        const BSONObj& query,
        std::string& error);

    bool insert(
        const std::string& ns,
        const BSONObj& obj,
        std::string& error);

    bool insertUnique(
        const std::string& ns,
        const BSONObj& query,
        const BSONObj& obj,
        std::string& error);

    bool update(
        const std::string& ns,
        const BSONObj& query,
        const mongo::BSONObj& obj,
        std::string& error,
        bool allowInsert = false,
        bool allowMultiple = false);

    bool updateUnique(
        const std::string& ns,
        const BSONObj& query,
        const BSONObj& obj,
        std::string& error);

    bool updateOrInsert(
        const std::string& ns,
        const BSONObj& query,
        const BSONObj& obj,
        std::string& error);

    bool updateMultiple(
        const std::string& ns,
        const BSONObj& query,
        const BSONObj& obj,
        std::string& error);

    bool remove(
        const std::string& ns,
        const BSONObj& query,
        std::string& error);

    void createInitialPool(size_t initialCount = 10);

    void relinquish(const Client& pClient);

    Client acquire();

    const std::string& getServer() const;

    void setServer(const std::string& server);

    static MongoDB::Ptr acquireServer(const std::string& server);

    static void releaseServers();
private:
    Mutex _mutex;
    std::queue<Client> _queue;
    std::string _server;
    static Mutex _serversMutex;
    static std::map<std::string, MongoDB::Ptr> _dbServers;
};



//
// Inlines
//

inline bool MongoDB::updateOrInsert(
    const std::string& ns,
    const BSONObj& query,
    const BSONObj& obj,
    std::string& error)
{
    return update(ns, query, obj, error, true, false);
}

inline bool MongoDB::updateMultiple(
    const std::string& ns,
    const BSONObj& query,
    const BSONObj& obj,
    std::string& error)
{
    return update(ns, query, obj, error, false, true);
}

inline const std::string& MongoDB::getServer() const
{
    return _server;
}

inline void MongoDB::setServer(const std::string& server)
{
    _server = server;
}


#endif	/* MONGODB_H */


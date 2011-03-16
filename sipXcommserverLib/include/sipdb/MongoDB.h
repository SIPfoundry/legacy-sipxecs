/*
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */

#ifndef MONGODB_H
#define	MONGODB_H

#include <queue>
#include <vector>
#undef VERSION
#include <mongo/client/dbclient.h>
#include <mongo/db/jsobj.h>
#include <boost/thread/mutex.hpp>
#include <boost/noncopyable.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/functional/hash.hpp>

#include "os/OsSysLog.h"

#define BSON_NOT_EQUAL(val) BSON("$ne"<< val)
#define BSON_LESS_THAN(val) BSON("$lt"<< val)
#define BSON_LESS_THAN_EQUAL(val) BSON("$lte"<< val)
#define BSON_GREATER_THAN(val) BSON("$gt" << val)
#define BSON_GREATER_THAN_EQUAL(val) BSON("$gte" << val)
#define BSON_ELEM_MATCH(val) BSON("$elemMatch" << val)
#define BSON_OR(val) BSON("$or" << val)
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
        typedef boost::shared_ptr<DBInterface> Ptr;
        DBInterface(MongoDB& db, const std::string& ns) : _db(db), _ns(ns){}
        const std::string& getNameSpace() const { return _ns; };
        MongoDB& db() { return _db; }
        MongoDB::Cursor items()
        {
            MongoDB::BSONObj query;
            std::string error;
            return db().find(
                _ns,
                query,
                error);
        }
    protected:
        MongoDB& _db;
        std::string _ns;
    };

    class DBInterfaceSet : public std::vector<DBInterface::Ptr>
    {
    public:
        DBInterfaceSet(const std::string& ns) :
            std::vector<DBInterface::Ptr>(),
            _ns(ns)
        {
        }
        ~DBInterfaceSet()
        {
        }

        bool attachNode(const std::string& nodeAddress)
        {
            MongoDB::Ptr pNode = MongoDB::acquireServer(nodeAddress);
            if (!pNode)
                return false;
            push_back(DBInterface::Ptr(new DBInterface(*(pNode.get()), _ns)));
            _nodes.push_back(pNode);
            return true;
        }
    protected:
        std::string _ns;
        std::vector<MongoDB::Ptr> _nodes;
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


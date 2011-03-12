
#include "sipdb/MongoDB.h"


std::map<std::string, MongoDB::Ptr> MongoDB::_dbServers;
MongoDB::Mutex MongoDB::_serversMutex;

MongoDB::Ptr MongoDB::acquireServer(const std::string& server)
{
    MongoDB::mutex_lock lock(MongoDB::_serversMutex);
    if (MongoDB::_dbServers.find(server) != MongoDB::_dbServers.end())
        return _dbServers[server];
    MongoDB::Ptr pServer = MongoDB::Ptr(new MongoDB(server));
    pServer->createInitialPool();
    MongoDB::_dbServers[server] = pServer;
    return pServer;
}

void MongoDB::releaseServers()
{
    MongoDB::mutex_lock lock(MongoDB::_serversMutex);
    MongoDB::_dbServers.clear();
}

MongoDB::MongoDB()
{
}

MongoDB::MongoDB(const std::string& server)
{
}

MongoDB::~MongoDB()
{
    {
        mutex_lock lock(_mutex);
        while(_queue.size() > 0)
            _queue.pop();
    }
}

void MongoDB::createInitialPool(size_t initialCount)
{
    mutex_lock lock(_mutex);
    for (size_t i = 0; i < initialCount; i++)
    {
        mongo::DBClientConnection* pConnection = new mongo::DBClientConnection();
        if (_server.empty())
            _server = "localhost";

        std::string errorMessage;
        if (pConnection->connect(_server, errorMessage))
        {
            _queue.push(Client(pConnection));
        }
        else
        {
            delete pConnection;
        }
    }
}

MongoDB::Client MongoDB::acquire()
{
    mutex_lock lock(_mutex);

    if (_queue.size() > 0)
    {
        Client pConnection = _queue.back();
        _queue.pop();
        return pConnection;
    }
    else
    {
        mongo::DBClientConnection* pConnection = new mongo::DBClientConnection();
        std::string errorMessage;
        if (pConnection->connect(_server, errorMessage))
        {
            return Client(pConnection);
        }
        else
        {
            delete pConnection;
            return Client();
        }
    }
    return Client();
}

void MongoDB::relinquish(const Client& pConnection)
{
    if (!pConnection)
        return;
    mutex_lock lock(_mutex);
    _queue.push(pConnection);
}

MongoDB::Cursor MongoDB::find(
    const std::string& ns,
    const BSONObj& query,
    std::string& error)
{
    try
    {
        MongoDB::PooledConnection pConnection(*this);
        if (!pConnection)
            return MongoDB::Cursor();
        return pConnection->query(ns, Query(query));
    }
    catch(std::exception& e)
    {
        error = e.what();
        return MongoDB::Cursor();
    }
    return MongoDB::Cursor();
}


bool MongoDB::insert(
    const std::string& ns,
    const mongo::BSONObj& obj,
    std::string& error)
{
    try
    {
        MongoDB::PooledConnection pConnection(*this);
        if (!pConnection)
            return false;
        pConnection->insert( ns , obj);
        return true;
    }
    catch(std::exception& e)
    {
        error = e.what();
        return false;
    }
    return false;
}

bool MongoDB::insertUnique(
    const std::string& ns,
    const BSONObj& query,
    const BSONObj& obj,
    std::string& error)
{
    try
    {
        MongoDB::PooledConnection pConnection(*this);
        if (!pConnection)
            return false;
        MongoDB::Cursor pCursor = find(ns, query, error);
        if (pCursor.get() && pCursor->more())
        {
            error = "Unique Record Exception";
            return false;
        }
        pConnection->insert( ns , obj);
        return true;
    }
    catch(std::exception& e)
    {
        error = e.what();
        return false;
    }
    return false;
}

bool MongoDB::update(
    const std::string& ns,
    const BSONObj& query,
    const BSONObj& obj,
    std::string& error,
    bool allowInsert,
    bool allowMultiple)
{
    try
    {
        Query updateQuery(query);

        MongoDB::PooledConnection pConnection(*this);
        if (!pConnection)
            return false;
        pConnection->update(ns, updateQuery, obj, allowInsert, allowMultiple);
    }
    catch(std::exception& e)
    {
        error = e.what();
        return false;
    }
    return false;
}

bool MongoDB::updateUnique(
    const std::string& ns,
    const BSONObj& query,
    const BSONObj& obj,
    std::string& error)
{
    try
    {
        Query updateQuery(query);

        MongoDB::PooledConnection pConnection(*this);
        if (!pConnection)
            return false;
        MongoDB::Cursor pCursor = find(ns, query, error);
        if (pCursor.get() && pCursor->itcount() > 1)
        {
            error = "Unique Record Exception";
            return false;
        }
        pConnection->update(ns, updateQuery, obj, false, false);
    }
    catch(std::exception& e)
    {
        error = e.what();
        return false;
    }
    return false;
}

bool MongoDB::remove(
    const std::string& ns,
    const BSONObj& query,
    std::string& error)
{
    try
    {
        MongoDB::PooledConnection pConnection(*this);
        if (!pConnection)
            return false;
        pConnection->remove(ns, Query(query), false);
    }
    catch(std::exception& e)
    {
        error = e.what();
        return false;
    }
    return false;
}


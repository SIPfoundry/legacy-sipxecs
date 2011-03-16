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

#include "sipdb/RegDB.h"
#include "os/OsDateTime.h"

std::string RegDB::_defaultNamespace = "node.registrar";
std::string& RegDB::defaultNamespace()
{
    return RegDB::_defaultNamespace;
}

MongoDB::Collection<RegDB>& RegDB::defaultCollection()
{
    static MongoDB::Collection<RegDB> collection(RegDB::_defaultNamespace);
    return collection;
}

RegDB::RegDB(MongoDB& db, const std::string& ns) :
    MongoDB::DBInterface(db, ns),
    _replicationNodes(ns),
    _firstIncrement(true)
{
}

RegDB::~RegDB()
{
}

void RegDB::updateBinding(RegBinding& binding)
{
    mutex_lock lock(_mutex);

    if (binding.getTimestamp() == 0)
        binding.setTimestamp((int) OsDateTime::getSecsSinceEpoch());

    if (binding.getLocalAddress().empty())
        binding.setLocalAddress(_localAddress);

    MongoDB::BSONObj query = BSON(
        "identity" << binding.getIdentity() <<
        "contact" << binding.getContact());

    MongoDB::BSONObj update;
    if (binding.getExpirationTime() > 0)
    {
        update = BSON("$set" << BSON(
            "timestamp" << binding.getTimestamp() <<
            "localAddress" << binding.getLocalAddress() <<
            "identity" << binding.getIdentity() <<
            "uri" << binding.getUri() <<
            "callId" << binding.getCallId() <<
            "contact" << binding.getContact() <<
            "qvalue" << binding.getQvalue() <<
            "instanceId" << binding.getInstanceId() <<
            "gruu" << binding.getGruu() <<
            "path" << binding.getPath() <<
            "cseq" << binding.getCseq() <<
            "expirationTime" << binding.getExpirationTime() <<
            "instrument" << binding.getInstrument() <<
            "expired" << binding.getExpired() ));
    }
    else
    {
        //
        // This is an unregister.  mark it as inactive
        //
        update = BSON("$set" << BSON(
            "cseq" << binding.getCseq() <<
            "expired" << true));
    }

    std::string error;
    if (!_db.updateOrInsert(_ns, query, update, error))
    {
        //
        // Log error string here
        //
        std::cerr << error;
    }
}


void RegDB::expireOldBindings(
    const std::string& identity,
    const std::string& callId,
    unsigned int cseq,
    unsigned int timeNow)
{
    mutex_lock lock(_mutex);
    unsigned int expirationTime = timeNow-1;
    MongoDB::BSONObj query = BSON(
            "identity" << identity <<
            "callId"<< callId <<
            "cseq" << BSON_LESS_THAN(cseq) <<
            "expirationTime" << BSON_GREATER_THAN_EQUAL(expirationTime));

    MongoDB::BSONObj update = BSON("$set" << BSON(
        "expired" << true <<
        "cseq" << cseq));

    std::string error;
    if (!_db.update(_ns, query, update, error))
    {
        //
        // Log error string here
        //
        std::cerr << error;
    }
}

void RegDB::expireAllBindings(
    const std::string& identity,
    const std::string& callId,
    unsigned int cseq,
    unsigned int timeNow)
{
    mutex_lock lock(_mutex);
    unsigned int expirationTime = timeNow-1;
    MongoDB::BSONObj query = BSON(
            "identity" << identity <<
            "expirationTime" << BSON_GREATER_THAN_EQUAL(expirationTime));

    MongoDB::BSONObj update = BSON("$set" << BSON(
        "expired" << true <<
        "cseq" << cseq));

    std::string error;
    if (!_db.update(_ns, query, update, error))
    {
        //
        // Log error string here
        //
        std::cerr << error;
    }
}


bool RegDB::isOutOfSequence(
    const std::string& identity,
    const std::string& callId,
    unsigned int cseq) const
{
    mutex_lock lock(_mutex);
    MongoDB::BSONObj query = BSON(
            "identity" << identity <<
            "callId" << callId);

    std::string error;
    MongoDB::Cursor pCursor = _db.find(_ns, query, error);
    if (pCursor.get() && pCursor->more())
    {
      while(pCursor->more())
      {
        RegBinding binding = pCursor->next();
        if (binding.getCseq() >= cseq)
          return true;
      }
    }

    return false;
}

bool RegDB::getUnexpiredContactsUser (
    const std::string& identity,
    int timeNow,
    Bindings& bindings) const
{
    mutex_lock lock(_mutex);
    static std::string gruuPrefix = GRUU_PREFIX;

    bool isGruu = identity.substr(0, gruuPrefix.size()) == gruuPrefix;
    MongoDB::BSONObj query;

    if (isGruu)
    {
        std::string searchString(identity);
        searchString += SIP_GRUU_URI_PARAM;
        query = BSON(
            "gruu" << searchString <<
            "expirationTime" << BSON_GREATER_THAN(timeNow) <<
            "expired" << false);
    }
    else
    {
        query = BSON(
            "identity" << identity <<
            "expirationTime" << BSON_GREATER_THAN(timeNow) <<
            "expired" << false);
    }

    std::string error;
    MongoDB::Cursor pCursor = _db.find(_ns, query, error);
    if (pCursor.get() && pCursor->more())
    {
        while (pCursor->more())
            bindings.push_back(RegBinding(pCursor->next()));
        return true;
    }

    return false;
}

bool RegDB::getUnexpiredContactsUserContaining(
        const std::string& matchIdentity,
        int timeNow,
        Bindings& bindings) const
{
    mutex_lock lock(_mutex);
    MongoDB::BSONObj query = BSON("expirationTime" << BSON_GREATER_THAN(timeNow) <<
        "expired" << false);

    std::string error;
    MongoDB::Cursor pCursor = _db.find(_ns, query, error);
    if (pCursor.get() && pCursor->more())
    {
        while (pCursor->more())
        {
            RegBinding binding(pCursor->next());
            if (binding.getIdentity().find(matchIdentity) != std::string::npos)
                bindings.push_back(binding);
        }
        return bindings.size() > 0;
    }

    return false;
}

bool RegDB::getUnexpiredContactsUserInstrument(
        const std::string& identity,
        const std::string& instrument,
        int timeNow,
        Bindings& bindings) const
{
    mutex_lock lock(_mutex);

    MongoDB::BSONObj query = BSON(
        "identity" << identity <<
        "instrument" << instrument <<
        "expirationTime" << BSON_GREATER_THAN(timeNow) <<
        "expired" << false);

    std::string error;
    MongoDB::Cursor pCursor = _db.find(_ns, query, error);
    if (pCursor.get() && pCursor->more())
    {
        while (pCursor->more())
            bindings.push_back(RegBinding(pCursor->next()));
        return true;
    }

    return false;
}

bool RegDB::getUnexpiredContactsInstrument(
        const std::string& instrument,
        int timeNow,
        Bindings& bindings) const
{
    mutex_lock lock(_mutex);
    MongoDB::BSONObj query = BSON(
        "instrument" << instrument <<
        "expirationTime" << BSON_GREATER_THAN(timeNow) <<
        "expired" << false);

    std::string error;
    MongoDB::Cursor pCursor = _db.find(_ns, query, error);
    if (pCursor.get() && pCursor->more())
    {
        while (pCursor->more())
            bindings.push_back(RegBinding(pCursor->next()));
        return true;
    }

    return false;
}

bool RegDB::getAllOldBindings(int timeNow, Bindings& bindings)
{
    mutex_lock lock(_mutex);
    MongoDB::BSONObj query = BSON(
        "expirationTime" << BSON_LESS_THAN(timeNow));

    std::string error;
    MongoDB::Cursor pCursor = _db.find(_ns, query, error);
    if (pCursor.get() && pCursor->more())
    {
        while (pCursor->more())
        {
            RegBinding binding(pCursor->next());
            if (binding.getCallId() != "#")
                bindings.push_back(binding);
        }
        return true;
    }
    return false;
}

bool RegDB::addReplicationNode(const std::string& nodeAddress)
{
    _nodeTimeStamps[nodeAddress] = 0;
    return _replicationNodes.attachNode(nodeAddress);
}

void RegDB::updateReplicationTimeStamp()
{
    for (MongoDB::DBInterfaceSet::iterator iter = _replicationNodes.begin();
        iter != _replicationNodes.end(); iter++)
    {
        MongoDB::DBInterface::Ptr pNode = *iter;
        const std::string& server = pNode->db().getServer();
        int timeStamp = _nodeTimeStamps[server];

        MongoDB::BSONObj query = BSON(
        "timestamp" << BSON_GREATER_THAN (timeStamp) <<
        "localAddress" << server);

        std::string error;
        MongoDB::Cursor pCursor = _db.find(_ns, query, error);
        if (pCursor.get() && pCursor->more())
        {
            int maxTimeStamp = 0;
            while (pCursor->more())
            {
                MongoDB::BSONObj bson = pCursor->next();
                if (bson.hasField("timestamp"))
                {
                    int t = bson.getIntField("timestamp");
                    if (t > maxTimeStamp)
                        maxTimeStamp = t;
                }
            }
            _nodeTimeStamps[server] = maxTimeStamp;
        }
    }
}

void RegDB::replicate()
{
    for (MongoDB::DBInterfaceSet::iterator iter = _replicationNodes.begin();
        iter != _replicationNodes.end(); iter++)
    {
        MongoDB::DBInterface::Ptr pNode = *iter;
        bool success = false;
        for (MongoDB::DBInterfaceSet::iterator nodeIter = _replicationNodes.begin();
            nodeIter != _replicationNodes.end(); nodeIter++)
        {
            const std::string& server = pNode->db().getServer();
            int timeStamp = _nodeTimeStamps[server];

            MongoDB::BSONObj query = BSON(
            "timestamp" << BSON_GREATER_THAN (timeStamp) <<
            "localAddress" << server);
            std::string error;

            MongoDB::Cursor pCursor = pNode->db().find(_ns, query, error);
            if (!error.empty())
                break;

            while (pCursor->more())
            {
                if (!success)
                    success = true;
                RegBinding binding(pCursor->next());
                updateBinding(binding);
            }
        }
        if (success)
            break;
    }
}

bool RegDB::cleanAndPersist(int currentExpireTime)
{
    mutex_lock lock(_mutex);

    if (!_firstIncrement)
        updateReplicationTimeStamp();
    else
        fetchNodes();
    _firstIncrement = false;

    replicate();

    MongoDB::BSONObj query = BSON("expirationTime" << BSON_LESS_THAN(currentExpireTime));
    std::string error;
    return _db.remove(_ns, query, error);
}

void RegDB::fetchNodes()
{
    NodeDB::defaultCollection().collection().localAddress() = _localAddress;
    NodeDB::Nodes nodes;
    if (NodeDB::defaultCollection().collection().getNodes(nodes, NodeDB::AlgoSweep))
    {
        for (NodeDB::Nodes::iterator iter = nodes.begin(); iter != nodes.end(); iter++)
            addReplicationNode(iter->ipAddress());
    }
}




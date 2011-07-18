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

#include <fstream>
#include "sipdb/RegDB.h"
#include "os/OsDateTime.h"
#include "os/OsLogger.h"
#include <json/json_spirit.h>

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
    MongoDB::DBInterface(db, "", ns),
    _replicationNodes(ns)
{
  _st_mtime = 0;
}

RegDB::~RegDB()
{
}

void RegDB::updateBinding(RegBinding& binding)
{
    if (binding.getTimestamp() == 0)
        binding.setTimestamp((int) OsDateTime::getSecsSinceEpoch());

    if (binding.getLocalAddress().empty())
    {
        std::string serverId = _localAddress;
        serverId += "/";
        serverId += _ns;
        binding.setLocalAddress(serverId);
    }

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

bool RegDB::getAllBindings(Bindings& bindings)
{
   MongoDB::BSONObj query;
    std::string error;
    MongoDB::Cursor pCursor = _db.find(_ns, query, error);
    if (pCursor.get() && pCursor->more())
    {
        while (pCursor->more())
        {
            RegBinding binding(pCursor->next());
            bindings.push_back(binding);
        }
        return true;
    }
    return false;
}


bool RegDB::getAllOldBindings(int timeNow, Bindings& bindings)
{
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
  return addReplicationNode(nodeAddress, nodeAddress, _ns);
}

bool RegDB::addReplicationNode(const std::string& nodeAddress, const std::string& internalAddress, const std::string& ns)
{
  std::string id = internalAddress + "/" + ns;
  _nodeTimeStamps[id] = 0;
  return _replicationNodes.attachNode(nodeAddress, internalAddress, ns);
}

void RegDB::updateReplicationTimeStamp()
{
    for (MongoDB::DBInterfaceSet::iterator iter = _replicationNodes.begin();
        iter != _replicationNodes.end(); iter++)
    {
        MongoDB::DBInterface::Ptr pNode = *iter;

        std::string id = pNode->getInternalAddress() + "/" + pNode->getNameSpace();
        int timeStamp = _nodeTimeStamps[id];

        MongoDB::BSONObj query = BSON(
        "timestamp" << BSON_GREATER_THAN (timeStamp) <<
        "localAddress" << id);


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
            _nodeTimeStamps[id] = maxTimeStamp;
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
            std::string id = pNode->getInternalAddress() + "/" + pNode->getNameSpace();
            int timeStamp = _nodeTimeStamps[id];

            MongoDB::BSONObj query = BSON(
            "timestamp" << BSON_GREATER_THAN (timeStamp) <<
            "localAddress" << id);
            std::string error;

            MongoDB::Cursor pCursor = pNode->db().find(pNode->getNameSpace(), query, error);
            if (!error.empty())
                break;

            while (pCursor.get() && pCursor->more())
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

bool RegDB::cleanAndPersist(int currentExpireTime, const std::string& nodeConfig, bool nodeFetch)
{
    if (nodeFetch)
      fetchNodes(nodeConfig);
    
    updateReplicationTimeStamp();       
    replicate();

    MongoDB::BSONObj query = BSON("expirationTime" << BSON_LESS_THAN(currentExpireTime));
    std::string error;
    return _db.remove(_ns, query, error);
}

void RegDB::fetchNodes(const std::string& nodeConfig)
{
  if (nodeConfig.empty())
    return;

  struct stat st;
  if (stat(nodeConfig.c_str(), &st) != 0)
  {
    OS_LOG_INFO(FAC_ODBC, "Unable to stat " << nodeConfig);
    return;
  }


  if (st.st_mtime == _st_mtime)
    return;

  if (_st_mtime)
  {
    //
    // This is a reload attempt due to an in-process change of the node configuration
    //
    _replicationNodes.clear();
    _nodeTimeStamps.clear();
  }

  _st_mtime = st.st_mtime;

  OS_LOG_INFO(FAC_ODBC, "Node time stamp has changed.  Reloading " << nodeConfig << ".");

  std::ifstream is(nodeConfig.c_str());

  json::Value value;
  json::read(is, value);
  const json::Array& nodesArray = value.get_array();
  //
  // Determine the index of this node
  //
  //std::string nodeId = _localAddress;
  //nodeId += "/";
  //nodeId += _ns;

  int localIndex = -1;
  for (size_t i = 0; i < nodesArray.size(); i++)
  {
    json::Object obj = nodesArray[i].get_obj();
    for (size_t j = 0; j < obj.size(); j++)
    {
      const json::Pair& pair = obj[j];
      if (pair.name_ == "node.internalAddress")
      {
        if (pair.value_ == _localAddress)
          localIndex = i;
      }
    }
  }

  if (localIndex == -1)
  {
    OS_LOG_ERROR(FAC_ODBC, "Cannot determine local index from nodes configuration!");
    return;
  }

  OS_LOG_INFO(FAC_ODBC, "Local Node index determined at " << localIndex << "/" << nodesArray.size());

  //
  // Add the nodes to the right
  //
  for (size_t i = localIndex + 1; i < nodesArray.size(); i++)
  {
    json::Object obj = nodesArray[i].get_obj();
    std::string server;
    std::string internalAddress;
    std::string collection;
    for (size_t j = 0; j < obj.size(); j++)
    {
      const json::Pair& pair = obj[j];
      
      if (pair.name_ == "node.server")
      {
        server = pair.value_.get_str();
      }
      else if (pair.name_ == "node.internalAddress")
      {
        internalAddress = pair.value_.get_str();
      }
      else if (pair.name_ == "node.collection")
      {
        collection = pair.value_.get_str();
      }
    }

    if (!server.empty() && !internalAddress.empty())
    {
      if (collection.empty())
        collection = _ns;
      OS_LOG_INFO(FAC_ODBC, "Adding replication node " << server << ":" << internalAddress << ":" << collection);
      addReplicationNode(server, internalAddress, collection);
    }
  }


  if (localIndex != 0)
  {
    //
    // Add more nodes from the start to complete the circular dependency of nodes
    //
    for (size_t i = 0; i < (size_t)localIndex; i++)
    {
      json::Object obj = nodesArray[i].get_obj();

      std::string server;
      std::string internalAddress;
      std::string collection;
      for (size_t j = 0; j < obj.size(); j++)
      {
        const json::Pair& pair = obj[j];

        if (pair.name_ == "node.server")
        {
          server = pair.value_.get_str();
        }
        else if (pair.name_ == "node.internalAddress")
        {
          internalAddress = pair.value_.get_str();
        }
        else if (pair.name_ == "node.collection")
        {
          collection = pair.value_.get_str();
        }
      }
      if (!server.empty() && !internalAddress.empty())
      {
        if (collection.empty())
          collection = _ns;
        OS_LOG_INFO(FAC_ODBC, "Adding replication node " << server << ":" << internalAddress << ":" << collection);
        addReplicationNode(server, internalAddress, collection);
      }
    }
  }
}

bool RegDB::clearAllBindings()
{
  return _db.removeAll(_ns);
}




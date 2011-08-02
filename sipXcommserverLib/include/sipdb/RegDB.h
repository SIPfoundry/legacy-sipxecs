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

#ifndef RegDB_H
#define	RegDB_H


#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "sipdb/RegBinding.h"
#include "boost/noncopyable.hpp"
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <map>

#ifndef GRUU_PREFIX
#define GRUU_PREFIX "~~gr~"
#endif

#ifndef SIP_GRUU_URI_PARAM
#define SIP_GRUU_URI_PARAM "gr"
#endif

class RegDB : MongoDB::DBInterface
{
public:
    typedef boost::recursive_mutex Mutex;
    typedef boost::lock_guard<Mutex> mutex_lock;
    typedef std::vector<RegBinding> Bindings;
    typedef MongoDB::Collection<RegDB> RegDBCollection;
    typedef boost::shared_ptr<RegDBCollection> Ptr;

    //
    // Nodes database
    //

    class NodesDb : MongoDB::DBInterface
    {
    public:
      NodesDb(MongoDB& db, const std::string& ns = "node.nodes") :
         MongoDB::DBInterface(db, "", ns)
      {
      }
    };

    RegDB(
        MongoDB& db,
        const std::string& ns = RegDB::_defaultNamespace);

    ~RegDB();

    void updateBinding(const RegBinding::Ptr& pBinding);

    void updateBinding(RegBinding& binding);

    void expireOldBindings(
        const std::string& identity,
        const std::string& callId,
        unsigned int cseq,
        unsigned int timeNow);

    void expireAllBindings(unsigned int timeNow);

    void expireAllBindings(
        const std::string& identity,
        const std::string& callId,
        unsigned int cseq,
        unsigned int timeNow);

    bool isOutOfSequence(
        const std::string& identity,
        const std::string& callId,
        unsigned int cseq) const;


    bool getUnexpiredContactsUser(
        const std::string& identity,
        int timeNow,
        Bindings& bindings) const;

    bool getUnexpiredContactsUserContaining(
        const std::string& matchIdentity,
        int timeNow,
        Bindings& bindings) const;

    bool getUnexpiredContactsUserInstrument(
        const std::string& identity,
        const std::string& instrument,
        int timeNow,
        Bindings& bindings) const;

    bool getUnexpiredContactsInstrument(
        const std::string& instrument,
        int timeNow,
        Bindings& bindings) const;

    bool getAllOldBindings(int timeNow, Bindings& binding);

    bool getAllExpiredBindings(Bindings& bindings);

    bool getAllBindings(Bindings& binding);

    bool cleanAndPersist(int currentExpireTime, bool nodeFetch);

    bool cleanAndPersist(int currentExpireTime, const std::string& json, bool nodeFetch);

    bool clearAllBindings();

    static std::string& defaultNamespace();

    void setLocalAddress(const std::string& localAddress);

    const std::string& getLocalAddress() const;

    static MongoDB::Collection<RegDB>& defaultCollection();
protected:
    void updateReplicationTimeStamp();
    void replicate();
    void fetchNodesFromJson(const std::string& nodeConfig);
    void fetchNodesFromMongo();

public:
    bool addReplicationNode(const std::string& nodeAddress);
    bool addReplicationNode(const std::string& nodeAddress, const std::string& internalAddress, const std::string& ns);
    void disableNode(const std::string& nodeId);
    void enableNode(const std::string& nodeId);
    bool isNodeDisabled(const std::string& nodeId) const;
private:
    MongoDB::DBInterfaceSet _replicationNodes;
    std::string _localAddress;
    std::map<std::string, int> _nodeTimeStamps;
    std::set<std::string> _disabledNodes;
    mutable MongoDB::Mutex _disabledNodesMutex;
    int _st_mtime;
    MongoDB::Collection<NodesDb>* _pNodesDb;
    static std::string _defaultNamespace;
};


//
// Inlines
//

inline void RegDB::setLocalAddress(const std::string& localAddress)
{
    _localAddress = localAddress;
}

inline const std::string& RegDB::getLocalAddress() const
{
    return _localAddress;
}

inline void RegDB::updateBinding(const RegBinding::Ptr& pBinding)
{
    updateBinding(*(pBinding.get()));
}


#endif	/* RegDB_H */


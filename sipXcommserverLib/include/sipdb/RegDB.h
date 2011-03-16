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


#include "sipdb/RegBinding.h"
#include "sipdb/NodeDB.h"
#include "boost/noncopyable.hpp"
#include "NodeDB.h"
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

    bool cleanAndPersist(int currentExpireTime);

    static std::string& defaultNamespace();

    void setLocalAddress(const std::string& localAddress);

    const std::string& getLocalAddress() const;

    static MongoDB::Collection<RegDB>& defaultCollection();
protected:
    void updateReplicationTimeStamp();
    void replicate();
    void fetchNodes();
    bool addReplicationNode(const std::string& nodeAddress);
private:
    mutable Mutex _mutex;
    MongoDB::DBInterfaceSet _replicationNodes;
    std::string _localAddress;
    std::map<std::string, int> _nodeTimeStamps;
    bool _firstIncrement;
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


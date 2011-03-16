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

#ifndef NODEDB_H
#define	NODEDB_H

#include "utl/UtlString.h"
#include "net/Url.h"
#include "sipdb/NodeRecord.h"
#include <set>


class NodeDB : public MongoDB::DBInterface
{
public:
    typedef std::vector<NodeRecord> Nodes;

    enum Algo
    {
        AlgoNone,
        AlgoSweep,
        AlgoPush,
        AlgoPushSweep
    };

    NodeDB(
        MongoDB& db,
        const std::string& ns = NodeDB::_defaultNamespace);

    ~NodeDB();

    std::string& localAddress();
    bool getNodes(Nodes& nodes, Algo algo = AlgoNone) const;
    static std::string& defaultNamespace();
    static MongoDB::Collection<NodeDB>& defaultCollection();
    static std::string _defaultNamespace;
    
private:
    std::string _localAddress;
};

//
// Inline
//

inline std::string& NodeDB::localAddress()
{
    return _localAddress;
}

#endif	/* NODEDB_H */


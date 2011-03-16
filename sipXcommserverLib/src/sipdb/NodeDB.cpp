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

#include <sipdb/NodeDB.h>
#include "os/OsSysLog.h"


std::string NodeDB::_defaultNamespace = "imdb.node";
std::string& NodeDB::defaultNamespace()
{
    return NodeDB::_defaultNamespace;
}

MongoDB::Collection<NodeDB>& NodeDB::defaultCollection()
{
    static MongoDB::Collection<NodeDB> collection(NodeDB::_defaultNamespace);
    return collection;
}


NodeDB::NodeDB(
    MongoDB& db,
    const std::string& ns ) :
        MongoDB::DBInterface(db, ns)
{
}

NodeDB::~NodeDB()
{

}

bool NodeDB::getNodes(Nodes& nodes, Algo algo) const
{
    MongoDB::BSONObj query;

    if (algo == AlgoSweep && !_localAddress.empty())
        query = BSON(NodeRecord::ipAddress_fld() << BSON_NOT_EQUAL( _localAddress));

    std::string error;
    MongoDB::Cursor pCursor = _db.find(_ns, query, error);
    while (pCursor->more())
        nodes.push_back(pCursor->next());
    return pCursor->itcount() != 0;
}

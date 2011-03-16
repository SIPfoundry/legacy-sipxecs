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

#ifndef NODERECORD_H
#define	NODERECORD_H

#include <string>
#include <vector>
#include <set>
#include <boost/shared_ptr.hpp>
#include "sipdb/MongoDB.h"


class NodeRecord
{
public:
    NodeRecord();
    NodeRecord(const NodeRecord& node);
    NodeRecord(const MongoDB::BSONObj& bsonObj);
    NodeRecord& operator =(const MongoDB::BSONObj& bsonObj);
    NodeRecord& operator=(const NodeRecord& node);
    void swap(NodeRecord& node);

    std::string& ipAddress();
    static const char* ipAddress_fld();
    std::string& description();
    static const char* description_fld();
    bool& master();
    static const char* master_fld();
    std::string oid();
private:
    std::string _ipAddress;
    std::string _description;
    std::string _oid;
    bool _master;
};

//
// Inlines
//

inline std::string& NodeRecord::ipAddress()
{
    return _ipAddress;
}

inline std::string& NodeRecord::description()
{
    return _description;
}

inline bool& NodeRecord::master()
{
    return _master;
}

inline std::string NodeRecord::oid()
{
    return _oid;
}

#endif	/* NODERECORD_H */


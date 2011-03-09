/* 
 * File:   NodeRecord.h
 * Author: joegen
 *
 * Created on March 8, 2011, 4:00 PM
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


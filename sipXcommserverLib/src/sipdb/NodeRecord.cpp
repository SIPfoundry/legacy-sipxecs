#include <sipdb/NodeRecord.h>

const char* NodeRecord::ipAddress_fld()
{
    static std::string fld = "ip";
    return fld.c_str();
}

const char* NodeRecord::description_fld()
{
    static std::string fld = "dsc";
    return fld.c_str();
}

const char* NodeRecord::master_fld()
{
    static std::string fld = "mstr";
    return fld.c_str();
}


NodeRecord::NodeRecord() :
    _master(false)
{

}

NodeRecord::NodeRecord(const NodeRecord& node)
{
    _ipAddress = node._ipAddress;
    _description = node._description;
    _master = node._master;
}

NodeRecord::NodeRecord(const MongoDB::BSONObj& bsonObj)
{
    try
    {
        _oid = bsonObj.getStringField("_id");

        if (bsonObj.hasField(NodeRecord::ipAddress_fld()))
        {
            _ipAddress = bsonObj.getStringField(NodeRecord::ipAddress_fld());
        }

        if (bsonObj.hasField(NodeRecord::description_fld()))
        {
            _description = bsonObj.getStringField(NodeRecord::description_fld());
        }

        if (bsonObj.hasField(NodeRecord::description_fld()))
        {
            _master = bsonObj.getBoolField(NodeRecord::master_fld());
        }
    }
    catch(const std::exception& e)
    {
        SYSLOG_ERROR("MongoDB Exception: (Entity::operator =(const MongoDB::BSONObj& bsonObj))" << e.what());
    }
}

NodeRecord& NodeRecord::operator =(const MongoDB::BSONObj& bsonObj)
{
    try
    {
        _oid = bsonObj.getStringField("_id");

        if (bsonObj.hasField(NodeRecord::ipAddress_fld()))
        {
            _ipAddress = bsonObj.getStringField(NodeRecord::ipAddress_fld());
        }

        if (bsonObj.hasField(NodeRecord::description_fld()))
        {
            _description = bsonObj.getStringField(NodeRecord::description_fld());
        }

        if (bsonObj.hasField(NodeRecord::description_fld()))
        {
            _master = bsonObj.getBoolField(NodeRecord::master_fld());
        }
    }
    catch(const std::exception& e)
    {
        SYSLOG_ERROR("MongoDB Exception: (Entity::operator =(const MongoDB::BSONObj& bsonObj))" << e.what());
    }
    return *this;
}

NodeRecord& NodeRecord::operator=(const NodeRecord& node)
{
    NodeRecord clonable(node);
    swap(clonable);
    return *this;
}

void NodeRecord::swap(NodeRecord& node)
{
    std::swap(_ipAddress, node._ipAddress);
    std::swap(_description, node._description);
    std::swap(_master, node._master);
}



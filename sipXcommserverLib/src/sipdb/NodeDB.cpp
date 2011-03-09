
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
    bool ok = true;
    while (pCursor->more())
        nodes.push_back(pCursor->next());
    return pCursor->itcount() != 0;
}

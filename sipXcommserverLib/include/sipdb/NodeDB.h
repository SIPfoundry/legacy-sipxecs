/* 
 * File:   NodeDB.h
 * Author: joegen
 *
 * Created on March 8, 2011, 3:59 PM
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


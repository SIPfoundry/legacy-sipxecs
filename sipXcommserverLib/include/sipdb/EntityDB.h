
#ifndef ENTITYDB_H
#define	ENTITYDB_H

#include "sipdb/EntityRecord.h"

class EntityDB : MongoDB::DBInterface
{
public:
    typedef std::vector<EntityRecord> Entities;
    typedef std::map<std::string, EntityRecord> EntitiesByIdentity;

    EntityDB(
        MongoDB& db,
        const std::string& ns);

    ~EntityDB();

    bool findByIdentity(const std::string& identity, EntityRecord& entity);
    bool findByUserId(const std::string& userId, EntityRecord& entity);
};


#endif	/* ENTITYDB_H */


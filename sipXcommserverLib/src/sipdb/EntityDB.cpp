#include "sipdb/EntityDB.h"

EntityDB::EntityDB(
    MongoDB& db,
    const std::string& ns) :
    MongoDB::DBInterface(db, ns)
{
}

EntityDB::~EntityDB()
{
}

bool EntityDB::findByIdentity(const std::string& identity, EntityRecord& entity)
{
    MongoDB::BSONObj query = BSON(EntityRecord::identity_fld() << identity);
    std::string error;
    MongoDB::Cursor pCursor = _db.find(_ns, query, error);
    if (pCursor->more())
    {
        entity = pCursor->next();
        return true;
    }
    return false;
}

bool EntityDB::findByUserId(const std::string& userId, EntityRecord& entity)
{
    MongoDB::BSONObj query = BSON(EntityRecord::userId_fld() << userId);
    std::string error;
    MongoDB::Cursor pCursor = _db.find(_ns, query, error);
    if (pCursor->more())
    {
        entity = pCursor->next();
        return true;
    }
    return false;
}

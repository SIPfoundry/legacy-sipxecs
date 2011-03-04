#include "sipdb/EntityDB.h"
#include "os/OsSysLog.h"

std::string EntityDB::_defaultNamespace = "imdb.entity";
std::string& EntityDB::defaultNamespace()
{
    return EntityDB::_defaultNamespace;
}

MongoDB::Collection<EntityDB>& EntityDB::defaultCollection()
{
    static MongoDB::Collection<EntityDB> collection(EntityDB::_defaultNamespace);
    return collection;
}

EntityDB::EntityDB(
    MongoDB& db,
    const std::string& ns) :
    MongoDB::DBInterface(db, ns)
{
}

EntityDB::~EntityDB()
{
}

bool EntityDB::findByIdentity(const std::string& identity, EntityRecord& entity) const
{
    MongoDB::BSONObj query = BSON(EntityRecord::identity_fld() << identity);
    SYSLOG_INFO("EntityDB::findByIdentity - Finding entity record for " << identity << " from namespace " << _ns);
    std::string error;
    MongoDB::Cursor pCursor = _db.find(_ns, query, error);
    if (pCursor->more())
    {
      SYSLOG_DEBUG( identity << " is present in namespace " << _ns);
        entity = pCursor->next();
        return true;
    }
    SYSLOG_DEBUG( identity << " is NOT present in namespace " << _ns);
    if (!error.empty())
    {
        SYSLOG_ERROR("MongoDB Exception: (EntityDB::findByIdentity)" << error);
    }

    SYSLOG_INFO("EntityDB::findByIdentity - Unable to find entity record for " << identity << " from namespace " << _ns);
    return false;
}

bool EntityDB::findByUserId(const std::string& userId, EntityRecord& entity) const
{
    MongoDB::BSONObj query = BSON(EntityRecord::userId_fld() << userId);
    std::string error;
    MongoDB::Cursor pCursor = _db.find(_ns, query, error);

    SYSLOG_INFO("EntityDB::findByUserId - Finding entity record for " << userId << " from namespace " << _ns);

    if (!error.empty())
    {
        SYSLOG_ERROR("MongoDB Exception: (EntityDB::findByIdentity)" << error);
    }
    if (pCursor->more())
    {
        entity = pCursor->next();
        return true;
    }
    SYSLOG_INFO("EntityDB::findByUserId - Unable to find entity record for " << userId << " from namespace " << _ns);
    return false;
}

/// Retrieve the SIP credential check values for a given identity and realm
bool EntityDB::getCredential (
   const Url& uri,
   const UtlString& realm,
   UtlString& userid,
   UtlString& passtoken,
   UtlString& authType) const
{
    UtlString identity;
    uri.getIdentity(identity);


    EntityRecord entity;
    if (!findByIdentity(identity.str(), entity))
        return false;

    if (entity.realm() != realm.str())
        return false;

    userid = entity.userId();
    passtoken = entity.password();
    authType = entity.authType();

    return true;
}

/// Retrieve the SIP credential check values for a given userid and realm
bool EntityDB::getCredential (
   const UtlString& userid,
   const UtlString& realm,
   Url& uri,
   UtlString& passtoken,
   UtlString& authType) const
{
    EntityRecord entity;
    if (!findByUserId(userid.str(), entity))
        return false;

    if (entity.realm() != realm.str())
        return false;

    uri = entity.identity().c_str();
    passtoken = entity.password();
    authType = entity.authType();

    return true;
}


void EntityDB::getAliasContacts (
    const Url& aliasIdentity,
    Aliases& aliases,
    bool& isUserIdentity) const
{
    UtlString identity;
    aliasIdentity.getIdentity(identity);

     EntityRecord entity;
     if (findByIdentity(identity.str(), entity))
     {
         aliases = entity.aliases();
         isUserIdentity = !entity.realm().empty() && !entity.password().empty();
     }
}